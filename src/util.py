import urllib.parse
import copy
import json
import subprocess
from fuzzingbook.GrammarFuzzer import tree_to_string
PARSE_SUCCEEDED = 10
MAX_SAMPLES = 100
MAX_PROC_SAMPLES = 100

Epsilon = '-'
NoEpsilon = '='

def tree_to_str(tree):
    return tree_to_string(tree)

class O:
    def __init__(self, **keys): self.__dict__.update(keys)
    def __repr__(self): return str(self.__dict__)

def init_log(prefix, var, module):
    with open('%s.log' % module, 'a+') as f:
        print(prefix, ':==============',var, file=f)

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

def check(o, e, s, module, sa1, sa2):
    if s in EXEC_MAP: return EXEC_MAP[s]
    result = do([module, s])
    with open('%s.log' % module, 'a+') as f:
        print('------------------', file=f)
        print('original:', repr(o), file=f)
        print('updated:', repr(s), file=f)
        print('Checking:',e, file=f)
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

def get_ref(node, node_name):
    name, children, *rest = node
    if name == node_name:
        return node
    for child in children:
        res = get_ref(child, node_name)
        if res is not None: return res
    return None


# replace the given node in a2 by the node in a1
def replace_nodes(a2, a1):
    node2, _, t2 = a2
    node1, _, t1 = a1
    str2_old = tree_to_str(t2)

    # first change the name of the node, then copy the tree.
    tmpl_name = '___cmimid___'
    old_name = node2[0]
    node2[0] = tmpl_name
    t2_new = copy.deepcopy(t2)
    node2[0] = old_name

    # now find the reference to tmpl_name in t2_new
    node2 = get_ref(t2_new, tmpl_name)
    node2.clear()
    for n in node1:
        node2.append(n)
    str2_new = tree_to_str(t2_new)
    assert str2_old != str2_new
    return str2_new

def is_compatible(a1, a2, module):
    t1 = is_a_replaceable_with_b(a1, a2, module)
    if not t1: return False
    t2 = is_a_replaceable_with_b(a2, a1, module)
    return t2

def is_a_replaceable_with_b(a1, a2, module):
    n1, f1, t1 = a1
    n2, f2, t2 = a2
    if tree_to_str(n1) == tree_to_str(n2): return True
    my_string = replace_nodes(a1, a2)
    o = tree_to_str(t1)
    return check(o, n1[0], my_string, module, tree_to_str(a1[0]), tree_to_str(a2[0]))

def parse_pseudo_name(node_name):
    assert (node_name[0], node_name[-1]) == ('<','>')
    return decode_name(node_name[1:-1])

def decode_name(node_name_stack):
    node_name, mstack = node_name_stack.split('#')
    method_stack = json.loads(mstack)
    method_ctrl_alt_name, can_empty = node_name.split(' ')
    method, ctrl_cid_altid = method_ctrl_alt_name.split(':')
    ctrl, cid_altid = ctrl_cid_altid.split('_')
    assert ctrl in {'while', 'if'}
    cid, altid = cid_altid.split(',')

    if 'while' == ctrl:
        assert altid == '0'
    return method, ctrl, int(cid), altid, can_empty, method_stack

def unparse_pseudo_name(method, ctrl, ctrl_id, alt_num, can_empty, cstack):
    return "<%s>" % encode_name(method, ctrl, ctrl_id, alt_num, can_empty, cstack)

def encode_name(method, ctrl, ctrl_id, alt_num, can_empty, stack):
    assert ctrl in {'while', 'if'}
    return '%s:%s_%s,%s %s#%s' % (method, ctrl, ctrl_id, alt_num, can_empty, json.dumps(stack))

def encode_method_name(name, my_args):
    # trick to convert args that are not of type str for later.
    #if args and hasattr(args[0], 'tag'):
    #    self.name = "%s:%s" % (args[0].tag, self.name)

    if not my_args:
        return name
    else:
        return "%s(%s)" % (name, urllib.parse.quote('_'.join([str(i) for i in my_args])))

def parse_method_name(mname):
    assert (mname[0], mname[-1]) == ('<', '>')
    name = mname[1:-1]
    if '.' in name:
        nname, my_id = name.split('.')
        return nname, my_id
    else:
        return name, '0'

def unparse_method_name(mname, my_id):
    return '<%s.%s>' % (mname, my_id)
