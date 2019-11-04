import string
import enum
import sys

import config
import chainutils

import random
random.seed(config.RandomSeed)

import pudb
brk = pudb.set_trace

COMPARE_OPERATORS = {'==': lambda x, y: x == y}

def log(var, i=1):
    if config.Debug >= i: print(repr(var), file=sys.stderr, flush=True)

class EState(enum.Enum):
    Trim = enum.auto()
    Append = enum.auto()
    Unknown = enum.auto()

class Prefix:
    def __init__(self, myarg):
        self.my_arg = myarg

    def __repr__(self):
        return repr(self.my_arg)

    def solve(self, my_traces, seen):
        raise NotImplemnted

    def create_prefix(self, myarg):
        raise NotImplemnted

    def continue_valid(self):
        return []

class Search(Prefix):

    def continue_valid(self):
        if  random.uniform(0,1) > config.Return_Probability:
            return [self.create_prefix(self.my_arg + random.choice(config.All_Characters))]
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

    def create_prefix(self, myarg): return DeepSearch(myarg)

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

    def solve(self, traces, seen):
        arg_prefix = self.my_arg
        # we are assuming a character by character comparison.
        # so get the comparison with the last element.
        while traces:
            h, *ltrace = traces
            end =  h.x
            k = self.parsing_state(h, limit_len=len(arg_prefix))
            new_prefix = arg_prefix[:end]
            fixes = self.get_previous_fixes(end, arg_prefix, seen)

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

            elif k == EState.Append:
                assert new_prefix == arg_prefix
                chars = config.All_Characters
            else:
                assert k == EState.Unknown
                # Unknown what exactly happened. Strip the last and try again
                traces = ltrace
                continue

            return [self.create_prefix("%s%s" % (new_prefix, new_char))
                    for new_char in chars]

        return []

class Chain:

    def __init__(self, executable):
        self._my_arg = None
        self.seen = set()
        self.executable = executable

    def add_sys_arg(self, v):
        self._my_arg = v

    def sys_arg(self):
        return self._my_arg

    def prune(self, solutions):
        # never retry an argument.
        return [s for s in solutions if s.my_arg not in self.seen]

    def choose(self, solutions):
        return [random.choice(self.prune(solutions))]

    def get_comparisons(self):
        return chainutils.get_comparisons()

    def execute(self, my_input):
        return chainutils.execute(self.executable, my_input)

    def gen_links(self):
        # replace interesting things
        arg = config.MyPrefix if config.MyPrefix else random.choice(config.All_Characters)
        solution_stack = [DeepSearch(arg)]

        chainutils.compile_src(self.executable)
        for _ in range(config.MaxIter):
            my_prefix, *solution_stack = solution_stack
            self.current_prefix = my_prefix
            self.add_sys_arg(my_prefix.my_arg)

            try:
                log(">> %s" % self.sys_arg(), 1)
                v = self.execute(self.sys_arg())
                solution_stack = my_prefix.continue_valid()
                if not solution_stack:
                    return (self.sys_arg(), v)
            except Exception as e:
                self.seen.add(self.current_prefix.my_arg)
                self.traces = self.get_comparisons()

                self.current_prefix = DeepSearch(self.current_prefix.my_arg)

                new_solutions = self.current_prefix.solve(list(reversed(self.traces)), self.seen)
                solution_stack = self.choose(new_solutions)

                if not solution_stack:
                    # remove one character and try again.
                    new_arg = self.sys_arg()[:-1]
                    if not new_arg:
                        raise Exception('DFS: No suitable continuation found')
                    solution_stack = [self.current_prefix.create_prefix(new_arg)]

def main(program, *rest):
    chain = Chain(program)
    chain.gen_links()

main(*sys.argv[1:])
