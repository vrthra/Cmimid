import copy
import json
import subprocess
from fuzzingbook.GrammarFuzzer import tree_to_string
PARSE_SUCCEEDED = 10
MAX_SAMPLES = 1

Epsilon = '-'
NoEpsilon = '='


class O:
    def __init__(self, **keys): self.__dict__.update(keys)
    def __repr__(self): return str(self.__dict__)

def do(command, env=None, shell=False, log=False, **args):
    result = subprocess.Popen(command,
        stdout = subprocess.PIPE,
        stderr = subprocess.STDOUT,
    )
    stdout, stderr = result.communicate(timeout=PARSE_SUCCEEDED)
    if log:
        with open('build/do.log', 'a+') as f:
            print(json.dumps({'cmd':command, 'env':env, 'exitcode':result.returncode}), env, file=f)
    return O(returncode=result.returncode, stdout=stdout, stderr=stderr)

#def do(command, env=None, shell=False, log=False, **args):
#    result = run(command, universal_newlines=True, shell=shell,
#                  env=dict(os.environ, **({} if env is None else env)),**args)
#    return result
EXEC_MAP = {}

def check(s, module, sa1, sa2):
    if s in EXEC_MAP: return EXEC_MAP[s]
    result = do([module, s])
    with open('%s.log' % module, 'a+') as f:
        print(s, file=f)
        print('Checking:', file=f)
        print('1:', repr(sa1), file=f)
        print('2:', repr(sa2), file=f)
        print(' '.join([module, repr(s)]), file=f)
        print(":=", result.returncode, file=f)
        print("\n", file=f)
    v = (result.returncode == 0)
    EXEC_MAP[s] = v
    return v

def to_modifiable(derivation_tree):
    node, children, *rest = derivation_tree
    return [node, [to_modifiable(c) for c in children], *rest]


def node_include(i, j):
    name_i, children_i, s_i, e_i = i
    name_j, children_j, s_j, e_j = j
    return s_i <= s_j and e_i >= e_j

def replace_nodes(a2, a1):
    node2, _, t2 = a2
    node1, _, t1 = a1
    str2_old = tree_to_string(t2)
    old = copy.copy(node2)
    node2.clear()
    for n in node1:
        node2.append(n)
    str2_new = tree_to_string(t2)
    assert str2_old != str2_new
    node2.clear()
    for n in old:
        node2.append(n)
    str2_last = tree_to_string(t2)
    assert str2_old == str2_last
    return str2_new

def is_compatible(a1, a2, module):
    if tree_to_string(a1[0]) == tree_to_string(a2[0]):
        return True
    my_string = replace_nodes(a1, a2)
    return check(my_string, module, tree_to_string(a1[0]), tree_to_string(a2[0]))



def parse_pseudo_name(name):
    assert name[0] + name[-1] == '<>'
    name = name[1:-1]
    method, rest = name.split(':')
    ctrl_name, space, rest = rest.partition(' ')
    can_empty, space, stack = rest.partition(' ')
    ctrl, cname = ctrl_name.split('_')
    assert ':for_' not in name
    assert ':switch_' not in name
    if ':while_' in name:
        method_stack = json.loads(stack)
        return method, ctrl, int(cname), 0, can_empty, method_stack
    elif ':if_' in name:
        num, mstack = stack.split('#')
        method_stack = json.loads(mstack)
        return method, ctrl, int(cname), num, can_empty, method_stack

def unparse_pseudo_name(method, ctrl, name, num, can_empty, cstack):
    assert ctrl != 'for'  and ctrl != 'switch'
    if ctrl == 'while':
        return "<%s:%s_%s %s %s>" % (method, ctrl, name, can_empty, json.dumps(cstack))
    else:
        return "<%s:%s_%s %s %s#%s>" % (method, ctrl, name, can_empty, num, json.dumps(cstack))
