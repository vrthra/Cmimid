import sys
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


def method_replace_stack(to_replace):
    # remember, we only replace methods.
    for (i, j) in to_replace:
        old_name = i[0]
        method1, my_id1 = parse_method_name(i[0])
        method2, my_id2 = parse_method_name(j[0])
        assert method1 == method2
        #assert can_empty2 != '?'

        new_name = unparse_method_name(method1, my_id2)
        i[0] = new_name

        for c in i[1]:
            update_method_stack(c, old_name[1:-1], new_name[1:-1])
    to_replace.clear()

def update_method_name(k_m, my_id, seen):
    # fixup k_m with what is in my_id, and update seen.
    original = k_m[0]
    method, old_id = parse_method_name(original)
    name = unparse_method_name(method, my_id)
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
            method1, my_id = parse_method_name(k_m[0])
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
        if len(my_values) > util.MAX_SAMPLES:
            lst = [v for v in my_values if not util.node_include(v[0], k_m)]
            values = sorted(lst, key=s_fn, reverse=True)[0:util.MAX_SAMPLES]
        else:
            values = my_values

        # all values in v should be tried.
        replace = 0
        for v in values:
            assert v[0][0] == v_[0][0]
            if f != FILE or not util.node_include(v[0], k_m): # if not k_m includes v
                a = util.is_compatible((k_m, FILE, TREE), v, module)
                if not a:
                    replace = 0
                    break
                else:
                    replace += 1
            if f != FILE or not util.node_include(k_m, v[0]):
                b = util.is_compatible(v, (k_m, FILE, TREE), module)
                if not b:
                    replace = 0
                    break
                else:
                    replace += 1
        # at least one needs to vouch, and all capable needs to agree.
        if replace:
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
    if node not in NODE_REGISTER:
        NODE_REGISTER[node] = {}
    register = NODE_REGISTER[node]

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
        else:
            # a new method! Generalize the last
            generalize_method(child, register[method_name], module)

def generalize_method_trees(jtrees, log=False):
    global TREE, FILE
    new_trees = []
    for j in jtrees:
        FILE = j['arg']
        if log: print(FILE, file=sys.stderr)
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


