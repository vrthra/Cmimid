import sys
import pudb
bp = pudb.set_trace
import util
import json
import os.path, copy, random
random.seed(0)

NODE_REGISTER = {}
TREE = None
FILE = None
def reset_generalizer():
    global NODE_REGISTER, TREE, FILE
    NODE_REGISTER={}
    TREE = None
    FILE = None
    util.EXEC_MAP.clear()

def update_method_stack(node, old_name, new_name):
    nname, children, *rest = node
    if not (':if_' in nname or ':while_' in nname):
        return
    method, ctrl, cname, num, can_empty, cstack = util.parse_pseudo_name(nname)
    assert method == old_name
    name = util.unparse_pseudo_name(new_name, ctrl, cname, num, can_empty, cstack)
    #assert '?' not in name
    node[0] = name
    for c in node[1]:
        update_method_stack(c, old_name, new_name)

def method_replace_stack(to_replace):
    # remember, we only replace methods.
    for (i, j) in to_replace:
        old_name = i[0]
        method1, my_id1 = util.parse_method_name(i[0])
        method2, my_id2 = util.parse_method_name(j[0])
        assert method1 == method2
        #assert can_empty2 != '?'

        new_name = util.unparse_method_name(method1, my_id2)
        i[0] = new_name

        for c in i[1]:
            update_method_stack(c, old_name[1:-1], new_name[1:-1])
    to_replace.clear()

def update_method_name(k_m, my_id, seen):
    # fixup k_m with what is in my_id, and update seen.
    original = k_m[0]
    method, old_id = util.parse_method_name(original)
    name = util.unparse_method_name(method, my_id)
    seen[k_m[0]] = name
    k_m[0] = name

    for c in k_m[1]:
        update_method_stack(c, original[1:-1], name[1:-1])

    return name, k_m

def register_new_methods(child, method_register):
    seen = {}
    k_m = child
    if "." not in k_m[0]:
        if k_m[0] in seen:
            k_m[0] = seen[k_m[0]]
            # and update
            method1, my_id = util.parse_method_name(k_m[0])
            update_method_name(k_m, my_id, seen)
            return
        # new! get a brand new name!
        method_register[1] += 1
        my_id = method_register[1]

        original_name = k_m[0]
        name, new_km = update_method_name(k_m, my_id, seen)
        method_register[0][(name, FILE)] = [(new_km, FILE, TREE)]
    else:
        name = k_m[0]
        if (name, FILE) not in method_register[0]:
            method_register[0][(name, FILE)] = []
        method_register[0][(name, FILE)].append((k_m, FILE, TREE))

def num_tokens(v, s):
    name, child, *rest = v
    s.add(name)
    [num_tokens(i, s) for i in child]
    return len(s)

def s_fn(v):
    return num_tokens(v[0], set())

def check_registered_methods_for_compatibility(child, method_register, module):
    seen = {}
    to_replace = []
    for method_key, f in method_register[0]:
        # try sampling here.
        my_values = method_register[0][(method_key, f)]
        v_ = random.choice(my_values)

        k_m = child
        if k_m[0] in seen: continue
        if len(my_values) > util.MAX_PROC_SAMPLES:
            values = sorted(my_values, key=s_fn, reverse=True)[0:util.MAX_PROC_SAMPLES]
        else:
            values = my_values

        # all values in v should be tried.
        replace = 0
        for v in values:
            assert v[0][0] == v_[0][0]
            a = util.is_compatible((k_m, FILE, TREE), v, module)
            if a:
                replace += 1
        if replace == 0: continue
        assert len(values) == replace, 'Not all values agreed'
        to_replace.append((k_m, v_[0])) # <- replace k_m by v
        seen[k_m[0]] = True

    method_replace_stack(to_replace)


def generalize_method(child, method_register, module):
    # First we check the previous methods
    check_registered_methods_for_compatibility(child, method_register, module)

    # lastly, update all method names.
    register_new_methods(child, method_register)

def generalize_method_node(tree, module):
    node, children, *_rest = tree
    global NODE_REGISTER
    register = NODE_REGISTER

    for i,child in enumerate(children):
        generalize_method_node(child, module)

    # Generalize methods
    for i,child in enumerate(children):
        if (child[0][0], child[0][-1]) != ('<','>'): continue
        if ':while_' in child[0] or ':if' in child[0]: continue
        method_name = child[0]
        if method_name not in register:
            register[method_name] = [{}, 0]
        generalize_method(child, register[method_name], module)

def generalize_method_trees(jtrees, log=False):
    global TREE, FILE
    new_trees = []
    for j in jtrees:
        FILE = j['arg']
        util.init_log('generalize_method', FILE, j['original'])
        if log:
            print(FILE, file=sys.stderr)
        sys.stderr.flush()
        TREE = util.to_modifiable(j['tree'])
        generalize_method_node(TREE, j['original'])
        j['tree'] = TREE
        new_trees.append(copy.deepcopy(j))
    return new_trees

def main(tracefile):
    with open(tracefile) as f:
        mined_trees = json.load(f)
    gmethod_trees = generalize_method_trees(mined_trees)
    json.dump(gmethod_trees, sys.stdout)

main(sys.argv[1])


