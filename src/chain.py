import os.path
import json
import subprocess
import string
import enum
import sys

import config

import random
random.seed(config.RandomSeed)
TIMEOUT = 100

COMPARE_OPERATORS = {'==': lambda x, y: x == y}

All_Characters = list(string.ascii_letters + string.digits + string.punctuation) \
        if config.No_CTRL else list(string.printable)
All_Characters = [i for i in All_Characters if i not in {"\n"}]

class Op:
    EQ = 1
    NE = 2
    IN = 3
    NOT_IN = 4

def log(var, i=1):
    if config.Debug >= i: print(repr(var), file=sys.stderr, flush=True)

import pudb
brk = pudb.set_trace

def compile_src(executable):
    with open('build/exec_file', 'w+') as f:
        print('''
pfuzzer=../checksum-repair
cp examples/*.h ./build
cp -r build/* $pfuzzer/build
{
cd ../checksum-repair;
./install/bin/trace-instr build/%s.c ./samples/excluded_functions 2>err >out
}
''' % executable, file=f)
    do(["bash", "./build/exec_file"], shell=False, input='')

def strsearch(Y, x):
    comparisons = []
    N = len(Y)
    n = len(x)
    i = 0
    while i+n <= N:
        found=True
        j = 0
        while found and j < n:
            comparisons.append((Y[i+j], x[j], i+j, j))
            if (Y[i+j] == x[j]):
                found = True
            else:
                found = False
            j += 1
        if found:
            return comparisons
        i +=1
    return comparisons

class EState(enum.Enum):
    # A char comparison made using a previous character
    Trim = enum.auto()
    # End of string as found using tainting or a comparison with the
    # empty string
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
        # should be overridden in child classes
        raise NotImplemnted

    def continue_valid(self):
        return []

class Search(Prefix):

    def continue_valid(self):
        if  random.uniform(0,1) > config.Return_Probability:
            return [self.create_prefix(self.my_arg + random.choice(All_Characters))]

    def parsing_state(self, h, arg_prefix):
        # If the any goes beyond the current official input, then it
        # is reasonable to assume that an EOF check was made.
        if h.x == len(arg_prefix):
            return EState.Append
        # We could ideally assume that anything else is a trim, since we no longer
        # need to detect EOF.
        return EState.Trim

    def comparisons_at(self, x, cmp_traces):
        return [(i,t) for i,t in enumerate(cmp_traces) if x == t.x]

    def get_previous_fixes(self, h, sprefix, seen):
        end = h.x
        similar = [i for i in seen if sprefix[:end] in i and
                   len(i) > len(sprefix[:end])]
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
            lst_solutions = All_Characters
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
        if not cmp_stack or config.Dumb_Search:
            return [[l] for l in All_Characters if constraints(l)]

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
        # add the prefix to seen.
        # we are assuming a character by character comparison.
        # so get the comparison with the last element.
        while traces:
            h, *ltrace = traces
            k = self.parsing_state(h, arg_prefix)
            end =  h.x
            new_prefix = arg_prefix[:end]
            fixes = self.get_previous_fixes(h, arg_prefix, seen)

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
                # check for line cov here.
                chars = sorted(set(sum(corr, [])))

            elif k == EState.Append:
                assert new_prefix == arg_prefix
                #assert len(fixes) == 0
                # An empty comparison at the EOF
                chars = All_Characters
            else:
                assert k == EState.Unknown
                # Unknown what exactly happened. Strip the last and try again
                # try again.
                traces = ltrace
                continue

            return [self.create_prefix("%s%s" % (new_prefix, new_char))
                    for new_char in chars]

        return []

class O:
    def __init__(self, **keys): self.__dict__.update(keys)
    def __repr__(self): return str(self.__dict__)

def do(command, env=None, shell=False, log=False,input=None, **args):
    result = subprocess.Popen(command,
        stdin = subprocess.PIPE,
        stdout = subprocess.PIPE,
        stderr = subprocess.PIPE,
        shell=shell
    )
    stdout, stderr = result.communicate(timeout=TIMEOUT, input=input.encode('ascii'))
    return O(returncode=result.returncode, stdout=stdout, stderr=stderr)

class Chain:

    def __init__(self, executable):
        self._my_arg = None
        self.seen = set()
        self.executable = executable
        self.eof_char = chr(126)

    def add_sys_arg(self, v):
        self._my_arg = v

    def sys_arg(self):
        return self._my_arg

    def sys_full_arg(self):
        return self._my_arg + self.eof_char

    def prune(self, solutions):
        return [s for s in solutions if s.my_arg not in self.seen]

    def choose(self, solutions):
        # never retry an argument.
        return [random.choice(self.prune(solutions))]

    def get_comparisons(self):
        input_comparisons = []
        with open('build/pygmalion.json') as pg:
            lines = pg.readlines()
        for sline in lines:
            if sline.startswith('#'): continue
            line = json.loads(sline)
            if line['type'] == 'INPUT_COMPARISON':
                if 'strip_input' in line['stack']: continue
                if line['operator'] in {'tokenstore', 'tokencomp', 'eof'}:
                    continue
                if line['operator'] == 'switch':
                    assert len(line['index']) == 1
                    for k in line['operand']:
                        input_comparisons.append(O(**{'x': line['index'][0], 'op': '==', 'op_B': k, 'op_A': line['value']}))
                elif line['operator'] == '==':
                    assert len(line['index']) == 1
                    input_comparisons.append(O(**{'x': line['index'][0], 'op': line['operator'], 'op_B':line['operand'][0], 'op_A': line['value']}))
                elif line['operator'] == 'conversion':
                    continue
                elif line['operator'] == 'strcmp':
                    # the index indicates which items in the input were touched.
                    # so for strcmp, length of index array corresponds to the length
                    # of operand
                    Bchars = line['operand'][0]
                    Achars = line['value']
                    idxs = line['index']
                    assert len(idxs) <= len(Bchars)

                    # now, for each element in the index array, there is a corresponding
                    # element of operand
                    for i,k in enumerate(idxs):
                        op_B = Bchars[i]
                        if i >= len(line['value']): break
                        op_A = Achars[i]
                        input_comparisons.append(O(**{'x': k, 'op': '==', 'op_B': op_B, 'op_A': op_A}))
                        if op_A != op_B: break

                elif line['operator'] == 'strsearch':
                    # the index indicates which items in the input were touched.
                    # so for strcmp, length of index array corresponds to the length
                    # of operand
                    Bchars = line['operand'][0]
                    Achars = line['value']
                    idxs = line['index']
                    comparisons = strsearch(Achars, Bchars)
                    for (y, x, i_j, j) in comparisons:
                        op_A, op_B = y, x
                        k = idxs[i_j]
                        assert self.sys_full_arg()[k] == op_A
                        input_comparisons.append(O(**{'x': k, 'op': '==', 'op_B': op_B, 'op_A': op_A}))

                else:
                    assert False
        for i in input_comparisons:
            if i.x >= len(self.sys_arg()):
                continue
            assert self.sys_arg()[i.x] == i.op_A
        return input_comparisons

    def execute(self, my_input):
        # first write the input in checksum-repair build
        with open('../checksum-repair/build/%s.input' % self.executable, 'w+') as f:
            print(my_input, end='', file=f)
        with open('build/exec_file', 'w+') as f:
            print('''
exec ../checksum-repair/build/%(program)s.c.uninstrumented < ../checksum-repair/build/%(program)s.input
''' % {'program':self.executable}, file=f)
        result1 = do(["bash", "./build/exec_file"], shell=False, input=my_input)
        if result1.returncode == 0:
            return result1

        # try to identify if we have an EOF
        with open('build/exec_file', 'w+') as f:
            print('''
../checksum-repair/build/%(program)s.c.instrumented < ../checksum-repair/build/%(program)s.input
gzip -c output > build/output.gz
exec ../checksum-repair/install/bin/trace-taint -me build/metadata -po build/pygmalion.json -t build/output.gz
''' % {'program':self.executable}, file=f)

        with open('../checksum-repair/build/%s.input' % self.executable, 'w+') as f:
            print(my_input + self.eof_char, end='', file=f)
        result2 = do(["bash", "./build/exec_file"], shell=False, input=my_input)
        raise Exception(result2)

    def links(self):
        self.start_i = 0
        # replace interesting things
        arg = config.MyPrefix if config.MyPrefix else random.choice(All_Characters)
        solution_stack = [DeepSearch(arg)]

        compile_src(self.executable)
        for i in range(self.start_i, config.MaxIter):
            my_prefix, *solution_stack = solution_stack
            self.current_prefix = my_prefix
            self.add_sys_arg(my_prefix.my_arg)

            self.start_i = i
            try:
                log(">> %s" % self.sys_arg(), 1)
                v = self.execute(self.sys_arg())
                solution_stack = my_prefix.continue_valid()
                if not solution_stack:
                    return (self.sys_arg(), v)
            except Exception as e:
                self.seen.add(self.current_prefix.my_arg)
                #log('Exception %s' % e)
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
    chain.links()

main(*sys.argv[1:])
