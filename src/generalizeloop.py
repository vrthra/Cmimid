import sys, json, copy, random
import util
from fuzzingbook.GrammarFuzzer import tree_to_string

NODE_REGISTER = {}
TREE = None
FILE = None
def reset_generalizer():
    global NODE_REGISTER, TREE, FILE
    NODE_REGISTER={}
    TREE = None
    FILE = None
    util.EXEC_MAP.clear()

def parse_name(name):
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

def update_stack(node, at, new_name):
    nname, children, *rest = node
    assert ':for_' not in nname
    assert ':switch_' not in nname
    if not (':if_' in nname or ':while_' in nname):
        return
    method, ctrl, cname, num, can_empty, cstack = util.parse_pseudo_name(nname)
    cstack[at] = new_name
    name = util.unparse_pseudo_name(method, ctrl, cname, num, can_empty, cstack)
    #assert '?' not in name
    node[0] = name
    for c in children:
        update_stack(c, at, new_name)

def update_name(k_m, my_id, seen):
    # fixup k_m with what is in my_id, and update seen.
    original = k_m[0]
    method, ctrl, cname, num, can_empty, cstack = util.parse_pseudo_name(original)
    #assert can_empty != '?'
    cstack[-1] = float('%d.0' % my_id)
    name = util.unparse_pseudo_name(method, ctrl, cname, num, can_empty, cstack)
    seen[k_m[0]] = name
    k_m[0] = name

    # only replace it at the len(cstack) -1 the
    # until the first non-cf token
    children = []
    for c in k_m[1]:
        update_stack(c, len(cstack)-1, cstack[-1])
    return name, k_m

def replace_stack_and_mark_star(to_replace):
    # remember, we only replace whiles.
    for (i, j) in to_replace:
        method1, ctrl1, cname1, num1, can_empty1, cstack1 = util.parse_pseudo_name(i[0])
        method2, ctrl2, cname2, num2, can_empty2, cstack2 = util.parse_pseudo_name(j[0])
        assert method1.split('.')[0] == method2.split('.')[0] # not necessary.
        assert ctrl1 == ctrl2
        assert cname1 == cname2
        #assert can_empty2 != '?'

        # fixup the can_empty
        new_name = util.unparse_pseudo_name(method2, ctrl1, cname1, num1, can_empty2, cstack1)
        i[0] = new_name
        assert len(cstack1) == len(cstack2)
        update_stack(i, len(cstack2)-1, cstack2[-1])
    to_replace.clear()

def num_tokens(v, s):
    name, child, *rest = v
    s.add(name)
    [num_tokens(i, s) for i in child]
    return len(s)

def s_fn(v):
    return num_tokens(v[0], set())

def check_registered_loops_for_compatibility(idx_map, while_register, module):
    seen = {}
    to_replace = []
    idx_keys = sorted(idx_map.keys())
    for while_key, f in while_register[0]:
        # try sampling here.
        my_values = while_register[0][(while_key, f)]
        v_ = random.choice(my_values)
        for k in idx_keys:
            k_m = idx_map[k]
            if k_m[0] in seen: continue
            if len(my_values) > util.MAX_SAMPLES:
                lst = [v for v in my_values if not util.node_include(v[0], k_m)]
                values = sorted(lst, key=s_fn, reverse=True)[0:util.MAX_SAMPLES]
            else:
                values = my_values

            # all values in v should be tried.
            replace = 0
            for v in values:
                #assert v[0][0] == v_[0][0]
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
    replace_stack_and_mark_star(to_replace)

def can_the_loop_be_deleted(idx_map, while_register, module):
    idx_keys = sorted(idx_map.keys())
    for i in idx_keys:
        i_m = idx_map[i]
        if '.0' in i_m[0]:
            # assert '?' not in i_m[0]
            continue
        a = util.is_compatible((i_m, FILE, TREE), (['', [], 0, 0], FILE, TREE), module)
        method1, ctrl1, cname1, num1, can_empty, cstack1 = util.parse_pseudo_name(i_m[0])
        name = util.unparse_pseudo_name(method1, ctrl1, cname1, num1, util.Epsilon if a else util.NoEpsilon, cstack1)
        i_m[0] = name

def check_current_loops_for_compatibility(idx_map, while_register, module):
    to_replace = []
    rkeys = sorted(idx_map.keys(), reverse=True)
    for i in rkeys: # <- nodes to check for replacement -- started from the back
        i_m = idx_map[i]
        # assert '?' not in i_m[0]
        if '.0' in i_m[0]: continue
        j_keys = sorted([j for j in idx_map.keys() if j < i])
        for j in j_keys: # <- nodes that we can replace i_m with -- starting from front.
            j_m = idx_map[j]
            # assert '?' not in j_m[0]
            if i_m[0] == j_m[0]: break
            # previous whiles worked.
            replace = False
            if not util.node_include(j_m, i_m):
                a = util.is_compatible((i_m, FILE, TREE), (j_m, FILE, TREE), module)
                if not a: continue
                replace = True
            if not util.node_include(i_m, j_m):
                b = util.is_compatible((j_m, FILE, TREE), (i_m, FILE, TREE), module)
                if not b: continue
                replace = True
            if replace:
                to_replace.append((i_m, j_m)) # <- replace i_m by j_m
            break
    replace_stack_and_mark_star(to_replace)

def register_new_loops(idx_map, while_register):
    idx_keys = sorted(idx_map.keys())
    seen = {}
    for k in idx_keys:
        k_m = idx_map[k]
        if ".0" not in k_m[0]:
            if k_m[0] in seen:
                k_m[0] = seen[k_m[0]]
                # and update
                method1, ctrl1, cname1, num1, can_empty1, cstack1 = util.parse_pseudo_name(k_m[0])
                update_name(k_m, cstack1[-1], seen)
                continue
            # new! get a brand new name!
            while_register[1] += 1
            my_id = while_register[1]

            original_name = k_m[0]
            #assert '?' not in original_name
            name, new_km = update_name(k_m, my_id, seen)
            #assert '?' not in name
            while_register[0][(name, FILE)] = [(new_km, FILE, TREE)]
        else:
            name = k_m[0]
            if (name, FILE) not in while_register[0]:
                while_register[0][(name, FILE)] = []
            while_register[0][(name, FILE)].append((k_m, FILE, TREE))

def generalize_while(idx_map, while_register, module):
    # First we check the previous while loops
    check_registered_loops_for_compatibility(idx_map, while_register, module)

    # Check whether any of these can be deleted.
    can_the_loop_be_deleted(idx_map, while_register, module)

    # then we check he current while iterations
    check_current_loops_for_compatibility(idx_map, while_register, module)

    # lastly, update all while names.
    register_new_loops(idx_map, while_register)

def generalize_while_node(tree, module):
    node, children, *_rest = tree
    if node not in NODE_REGISTER:
        NODE_REGISTER[node] = {}
    register = NODE_REGISTER[node]

    for child in children:
        generalize_while_node(child, module)

    # Generalize while.
    # IMPORTANT: If there are multiple loops, split out the idxs
    # correspondingly so that only those idxs belonging to a particular loop are
    # sent to generalize_while
    idxs = {}
    last_while = None
    for i,child in enumerate(children):
        # now we need to map the while_name here to the ones in node
        # register. Essentially, we try to replace each.
        if ':while_' not in child[0]:
            assert ':for_' not in child[0]
            continue
        while_name = child[0].split(' ')[0]
        if last_while is None:
            last_while = while_name
            if while_name not in register:
                register[while_name] = [{}, 0]
        else:
            if last_while != while_name:
                # a new while! Generalize the last
                generalize_while(idxs, register[last_while], module)
                last_while = while_name
                idxs = {}
                if last_while  not in register:
                    register[last_while] = [{}, 0]
        idxs[i] = child
    if last_while is not None:
        generalize_while(idxs, register[last_while], module)

def generalize_while_trees(jtrees, log=False):
    global TREE, FILE
    new_trees = []
    for j in jtrees:
        FILE = j['arg']
        if log: print(FILE, file=sys.stderr)
        sys.stderr.flush()
        TREE = util.to_modifiable(j['tree'])
        generalize_while_node(TREE, j['original'])
        j['tree'] = TREE
        new_trees.append(copy.deepcopy(j))
    return new_trees

def main(tracefile):
    with open(tracefile) as f:
        gmethod_trees = json.load(f)
    generalized_trees = generalize_while_trees(gmethod_trees)
    with open('build/t3.json', 'w+') as f: json.dump(generalized_trees, f)

main(sys.argv[1])


