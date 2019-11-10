import string
import json
import enum
import sys
import os.path

import config
import chainutils

import random
random.seed(config.RandomSeed)
import numpy as np
np.random.seed(config.RandomSeed)

import events

import pudb
brk = lambda : () #pudb.set_trace

COMPARE_OPERATORS = {'==': lambda x, y: x == y}

from functools import reduce

class EState(enum.Enum):
    Trim = enum.auto()
    Append = enum.auto()
    Unknown = enum.auto()

class Prefix:
    def __init__(self):
        pass

    def __repr__(self):
        return repr(self.my_arg)

    def solve(self, prefix, my_traces, seen):
        raise NotImplemnted

    def continue_valid(self, prefix):
        raise NotImplemnted

class Search(Prefix):

    def continue_valid(self, prefix):
        if  random.uniform(0,1) > config.Return_Probability:
            return [prefix + random.choice(config.All_Characters)]
        return []

    def parsing_state(self, h, limit_len):
        # If the any goes beyond the current official input, then it
        # is reasonable to assume that an EOF check was made.
        if h.x == limit_len:
            return EState.Append
        # We could ideally assume that anything else is a trim, since we no longer
        # need to detect EOF.
        return EState.Trim

    def comparisons_at(self, x, cmp_traces):
        return [(i,t) for i,t in enumerate(cmp_traces) if x == t.x]

    def get_previous_fixes(self, end, sprefix, seen):
        similar = [i for i in seen
                       if sprefix[:end] in i and len(i) > len(sprefix[:end])]
        return [i[end] for i in similar]

class DeepSearch(Search):

    def extract_solutions(self, elt, lst_solutions, flip=False):
        original = elt.op_B
        fn = COMPARE_OPERATORS[elt.op]
        result = fn(elt.op_A, elt.op_B)
        myfn = fn if not flip else lambda a, b: not fn(a, b)
        fres = lambda x: x if result else not x
        return [c for c in lst_solutions if fres(myfn(c, original))]

    def get_lst_solutions_at_divergence(self, cmp_stack, v):
        # if we dont get a solution by inverting the last comparison, go one
        # step back and try inverting it again.
        stack_size = len(cmp_stack)
        while v < stack_size:
            # now, we need to skip everything till v. That is, our cmp_stack
            # is in reversed form. So, we want to diverge at the end of a
            # chain starting from index at -1.
            assert cmp_stack[-1] is cmp_stack[v:][-1]
            diverge, *satisfy = cmp_stack[v:]
            lst_solutions = config.All_Characters
            for i,elt in reversed(satisfy):
                lst_solutions = self.extract_solutions(elt, lst_solutions, False)

            # now we need to diverge here
            i, elt = diverge
            lst_solutions = self.extract_solutions(elt, lst_solutions, True)
            if lst_solutions:
                return lst_solutions
            v += 1
        return []

    def get_corrections(self, cmp_stack, constraints):
        """
        cmp_stack contains a set of comparions, with the last comparison made
        at the top of the stack, and first at the bottom. Choose a point
        somewhere and generate a character that conforms to everything until
        then.
        """
        if not cmp_stack:
            return [[l] for l in config.All_Characters if constraints(l)]

        stack_size = len(cmp_stack)
        lst_positions = list(range(stack_size-1,-1,-1))
        solutions = []

        for point_of_divergence in lst_positions:
            lst_solutions = self.get_lst_solutions_at_divergence(cmp_stack,
                    point_of_divergence)
            lst = [l for l in lst_solutions if constraints(l)]
            if lst:
                solutions.append(lst)
        return solutions

    def solve(self, arg_prefix, traces, seen):
        # we are assuming a character by character comparison.
        # so get the comparison with the last element.
        while traces:
            h, *ltrace = traces
            end =  h.x
            k = self.parsing_state(h, limit_len=len(arg_prefix))
            new_prefix = arg_prefix[:end]
            fixes = self.get_previous_fixes(end, arg_prefix, seen)
            kind = None

            if k == EState.Trim:
                # A character comparison of the *last* char.
                # This was a character comparison. So collect all
                # comparisons made using this character. until the
                # first comparison that was made otherwise.
                # Now, try to fix the last failure
                cmp_stack = self.comparisons_at(h.x, traces)
                # Now, try to fix the last failure
                corr = self.get_corrections(cmp_stack, lambda i: i not in fixes)
                if not corr: raise Exception('Exhausted attempts: %s' % fixes)
                chars = sorted(set(sum(corr, [])))
                kind = EState.Trim

            elif k == EState.Append:
                assert new_prefix == arg_prefix
                chars = config.All_Characters
                kind = EState.Append
            else:
                assert k == EState.Unknown
                # Unknown what exactly happened. Strip the last and try again
                traces = ltrace
                continue

            return kind, ["%s%s" % (new_prefix, new_char) for new_char in chars]

        return []

class Chain:

    def __init__(self, executable, learner):
        self.learner = learner
        self.executable = executable
        self.reset()
        self.solver = DeepSearch()
        if not os.path.exists('./build/metadata'):
            chainutils.compile_src(self.executable)

    def reset(self):
        self._my_arg = None
        self.seen = set()
        self.starting_fn = '<start>'
        self.last_fn = self.starting_fn
        self.traces = []
        return self.last_fn

    def prune(self, solutions):
        # never retry an argument.
        return [s for s in solutions if s not in self.seen]

    def choose(self, solutions):
        return [random.choice(self.prune(solutions))]

    def get_comparisons(self, prefix):
        return chainutils.get_comparisons()

    def execute(self, my_input):
        return chainutils.execute(self.executable, my_input)


    def evaluate(self, arg):
        self.current_prefix = arg

        print(">", repr(self.current_prefix), "(%s: %d %s %d)" % (
            self.learner.cur_state,
            self.learner.env.last_stack_len,
            self.learner.env.e, self.learner.env.i)) #, end="\r")
        done, v = self.execute(self.current_prefix)
        if done:
            print(">",repr(self.current_prefix))
            return None, [self.starting_fn], self.starting_fn, True, []

        self.seen.add(self.current_prefix)
        self.traces = self.get_comparisons(self.current_prefix)
        sol_kind, new_solutions = self.solver.solve(self.current_prefix, list(reversed(self.traces)), self.seen)
        # new_state is the function seen in last comparison

        close_idx = len(self.current_prefix)
        true_trace = [i for i in self.traces if i.x < close_idx]
        if not true_trace:
            brk()
        state = "%s@%d" %(true_trace[-1].stack[-1],true_trace[-1].id)
        #state = true_trace[-1].stack[-1]
        return sol_kind, true_trace[-1].stack, state, False, self.prune(new_solutions)

class Env:
    def __init__(self, chain):
        self.chain = chain
        self.prefix = ''
        self.kind = EState.Append
        self.solutions = []
        self.last_stack_len = -1
        self.e = '_'
        self.i = 0

    def reset(self):
        self.prefix = ''
        self.kind = EState.Append
        self.solutions = []
        self.last_stack_len = -1
        return self.chain.reset()

    def step(self, action, cur_state):
        if self.kind == EState.Append:
            self.prefix = self.prefix + action
        else:
            self.prefix = self.prefix[0:-1] + action
        kind, stack, state, done, solutions = self.chain.evaluate(self.prefix)
        self.solutions = solutions
        # what should the next state be?
        # if solutions given is prefix + ... then we have an append
        if done:
            self.last_stack_len = -1 #len(stack)
            return stack, stack[-1], 100, True
        else:
            self.kind = kind
            reward = -1
            if kind == EState.Trim:
                next_state = cur_state
                reward = -10 # * len(stack)
            elif kind == EState.Append:
                next_state = state #stack[-1]

                # We maintain the stack. For example, true, false
                # The problem is isctrl, isspace etc has little incentive to stop
                if len(stack)  == self.last_stack_len:
                    if next_state == cur_state:
                        reward = 1
                    else:
                        reward = -10

                # A reduction in stack length. We give it a reward
                elif len(stack) < self.last_stack_len:
                    reward = 10
                else:
                    # An increase in stack length. We penalize it
                    reward = -10
                self.last_stack_len = len(stack)
            else:
                assert False

            #if next_state != self.chain.starting_fn:
            #    if kind == EState.Append and next_state not in stack:
            #        brk()

            return stack, next_state, reward, False


class Learner: pass
class QLearner(Learner):
    def __init__(self, program, qarr=None):
        self.program = program
        self.actions = config.All_Characters
        starting_state = '<start>'
        self.cur_state = starting_state
        self.env = Env(Chain(program, self))
        self.states = [starting_state] + chainutils.get_functions()
        self.action_prefixes = {}

        self.alpha = 0.5 # learning rate
        self.gamma = 0.90 # discount factor -- 0.8 - 0.99
        self.epsilon_init = 1.0 # explore/exploit factor
        self.epsilon = self.epsilon_init
        self.epsilon_min = 0.001
        self.Q = chainutils.loadQ()

    def update_epsilon(self):
        if self.epsilon > self.epsilon_min:
            self.epsilon = (100.0 - len(self.env.prefix))/100.0
        else:
            self.epsilon = self.epsilon_min


    def sidx(self, state):
        return next(i for i,a in enumerate(self.states) if a == state)

    def aidx(self, action):
        return next(i for i,a in enumerate(self.actions) if a == action)

    def update_Q(self, state, action, new_state, reward):
        if state not in self.Q: self.Q[state] = dict(self.Q['_'])
        if new_state not in self.Q: self.Q[new_state] = dict(self.Q['_'])

        self.Q[state][action] = ((1 - self.alpha) * self.Q[state][action]) + \
                self.alpha * (reward + self.gamma * max(self.Q[new_state].values()))
        chainutils.dumpQ(self.Q)
        return True

    def randargmax(self, hm, actions):
        items = [(k,v) for k,v in hm.items() if k in actions]
        max_val = max([v for k,v in items])
        return random.choice([k for k,v in items if v == max_val])

    def next_action(self):
        if not self.env.solutions:
            self.env.e = 'r'
            return random.choice(config.All_Characters)
        chars = [c[-1] for c in self.env.solutions] if self.env.solutions else config.All_Characters
        if random.uniform(0,1) < self.epsilon:
            self.env.e = 'R(%s)' % self.epsilon
            return random.choice(chars)
        else:
            self.env.e = '^(%s)' % self.epsilon
            maxval = max(self.Q[self.cur_state].values())
            if maxval == 0:
                self.env.e = '.(%s)' % self.epsilon
            act = self.randargmax(self.Q[self.cur_state], chars)
            return act

    def manage_stack(self, program_stack, state=''):
        if not self.managed_stack:
            self.skip_stack = program_stack[0:-1]
            self.managed_stack = list(self.skip_stack)
            return

        # first, process the managed_stack
        managed_stack_prefix = [prog.split('@')[0] for prog in self.managed_stack]
        if ' '.join(managed_stack_prefix) in ' '.join(program_stack):
            if len(program_stack) > len(managed_stack_prefix):
                # program added a few more stacks.
                suffix = program_stack[len(managed_stack_prefix):-1]
                self.managed_stack.extend(suffix)
                if state:
                    assert program_stack[-1] == state.split('@')[0]
                self.managed_stack.append(state)
            else:
                assert len(program_stack) == len(managed_stack_prefix)
        elif ' '.join(program_stack) in ' '.join(managed_stack_prefix):
            assert len(program_stack) < len(managed_stack_prefix)
            self.managed_stack = self.managed_stack[0:len(program_stack)]
        return


    def learn(self, episodes):
        # start over each time.
        # What is reset:
        #   - the prefix
        #   - list of seen arguments
        #   - the last function
        #   - traces
        self.cur_state = self.env.reset()
        done = False

        for _ in range(episodes):
            # start with full exploration
            self.epsilon = self.epsilon_init

            self.cur_state = self.env.reset()
            self.managed_stack = None
            for i in range(config.MaxIter):
                self.env.i = i
                chainutils.check_debug()
                action = self.next_action()
                program_stack, next_state, reward, done = self.env.step(action, self.cur_state)

                # We can only trust appends
                # TODO: maintain our own stack, and verify that the given stack is using prefix strings.

                if next_state == self.env.chain.starting_fn:
                    # We do not want ot learn anything about the starting state.
                    self.manage_stack(program_stack)
                else:
                    if self.env.kind == EState.Trim:
                        pass
                        # No learning here.
                    elif self.env.kind == EState.Append:
                        self.manage_stack(program_stack, next_state)

                        key = next_state
                        # update only when it is append. We do not need help
                        # in rejection.
                        new_prefix_cmp = [(i.x, i.op_B) for i in self.env.chain.traces if i.stack == program_stack] # TODO; should we filter close_char
                        idxes = [i for i,m in new_prefix_cmp]
                        min_idx, max_idx = min(idxes), max(idxes)
                        s_prefix = ''.join([m for i,m in new_prefix_cmp])
                        str_prefix = ' '.join(["%d:%s" % (i - min_idx,m) for i,m in new_prefix_cmp])

                        if key not in self.action_prefixes:
                            self.action_prefixes[key] = {str_prefix: None } #{str_prefix}}

                        old_prefixes = self.action_prefixes[key]
                        fn = key.split('@')[0]
                        found = 0

                        for old_prefix in list(old_prefixes.keys()):
                            if len(str_prefix) > len(old_prefix):
                                if str_prefix.startswith(old_prefix):
                                    found += 1
                                    # remove the old prefix first, and put the more complete version
                                    # in its place
                                    #prev_set = self.action_prefixes[key][old_prefix]
                                    del self.action_prefixes[key][old_prefix]
                                    self.action_prefixes[key][str_prefix] = None #prev_set | {str_prefix}
                                else:
                                    # not found
                                    pass
                            else:
                                if old_prefix.startswith(str_prefix):
                                    found += 1
                                else:
                                    # not found
                                    pass

                        if found == 0:
                            # time to blacklist.
                            self.action_prefixes[key][str_prefix] = None

                        if len(old_prefixes.keys()) > 1:
                            v = repr(s_prefix)[1:-1]
                            next_state = next_state + ':' + chainutils.summary(v)

                        self.update_Q(self.cur_state, action, next_state, reward)
                        self.cur_state = next_state
                    else:
                        assert False, 'Unknown state'
                if done: break

                # reduce the exploration during each step.
                self.update_epsilon()

def main(program, *rest):
    learner = QLearner(program)
    learner.learn(100)

main(*sys.argv[1:])
