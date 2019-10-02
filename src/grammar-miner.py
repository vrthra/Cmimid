import sys
import json
import subprocess

Epsilon = '-'
NoEpsilon = '='

from operator import itemgetter
from fuzzingbook.GrammarFuzzer import tree_to_string
import itertools as it

class O:
    def __init__(self, **keys): self.__dict__.update(keys)
    def __repr__(self): return str(self.__dict__)

def do(command, env=None, shell=False, log=False, **args):
    result = subprocess.Popen(command,
        stdout = subprocess.PIPE,
        stderr = subprocess.STDOUT,
    )
    stdout, stderr = result.communicate()
    if log:
        with open('build/do.log', 'a+') as f:
            print(json.dumps({'cmd':command, 'env':env, 'exitcode':result.returncode}), env, file=f)
    return O(returncode=result.returncode, stdout=stdout, stderr=stderr)

#def do(command, env=None, shell=False, log=False, **args):
#    result = run(command, universal_newlines=True, shell=shell,
#                  env=dict(os.environ, **({} if env is None else env)),**args)
#    return result

def reconstruct_method_tree(method_map):
    first_id = None
    tree_map = {}
    for key in method_map:
        m_id, m_name, m_children = method_map[key]
        children = []
        if m_id in tree_map:
            # just update the name and children
            assert not tree_map[m_id]
            tree_map[m_id]['id'] = m_id
            tree_map[m_id]['name'] = m_name
            tree_map[m_id]['indexes'] = []
            tree_map[m_id]['children'] = children
        else:
            assert first_id is None
            tree_map[m_id] = {'id': m_id, 'name': m_name, 'children': children, 'indexes': []}
            first_id = m_id

        for c in m_children:
            assert c not in tree_map
            val = {}
            tree_map[c] = val
            children.append(val)
    return first_id, tree_map



def last_comparisons(comparisons):
    HEURISTIC = True
    last_cmp_only = {}
    last_idx = {}

    # get the last indexes compared in methods.
    for idx, char, mid in comparisons:
        if mid in last_idx:
            if idx > last_idx[mid]:
                last_idx[mid] = idx
        else:
            last_idx[mid] = idx

    for idx, char, mid in comparisons:
        if HEURISTIC:
            if idx in last_cmp_only:
                if last_cmp_only[idx] > mid:
                    # do not clobber children unless it was the last character
                    # for that child.
                    if last_idx[mid] > idx:
                        # if it was the last index, may be the child used it
                        # as a boundary check.
                        continue
        last_cmp_only[idx] = mid
    return last_cmp_only


def attach_comparisons(method_tree, comparisons):
    for idx in comparisons:
        mid = comparisons[idx]
        method_tree[mid]['indexes'].append(idx)


def to_node(idxes, my_str):
    assert len(idxes) == idxes[-1] - idxes[0] + 1
    assert min(idxes) == idxes[0]
    assert max(idxes) == idxes[-1]
    return my_str[idxes[0]:idxes[-1] + 1], [], idxes[0], idxes[-1]


def indexes_to_children(indexes, my_str):
    lst = [
        list(map(itemgetter(1), g))
        for k, g in it.groupby(enumerate(indexes), lambda x: x[0] - x[1])
    ]

    return [to_node(n, my_str) for n in lst]

def does_item_overlap(s, e, s_, e_):
    return (s_ >= s and s_ <= e) or (e_ >= s and e_ <= e) or (s_ <= s and e_ >= e)

def is_second_item_included(s, e, s_, e_):
    return (s_ >= s and e_ <= e)

def has_overlap(ranges, s_, e_):
    return {(s, e) for (s, e) in ranges if does_item_overlap(s, e, s_, e_)}

def is_included(ranges, s_, e_):
    return {(s, e) for (s, e) in ranges if is_second_item_included(s, e, s_, e_)}

def remove_overlap_from(original_node, orange):
    node, children, start, end = original_node
    new_children = []
    if not children:
        return None
    start = -1
    end = -1
    for child in children:
        if does_item_overlap(*child[2:4], *orange):
            new_child = remove_overlap_from(child, orange)
            if new_child: # and new_child[1]:
                if start == -1: start = new_child[2]
                new_children.append(new_child)
                end = new_child[3]
        else:
            new_children.append(child)
            if start == -1: start = child[2]
            end = child[3]
    if not new_children:
        return None
    assert start != -1
    assert end != -1
    return (node, new_children, start, end)

def no_overlap(arr):
    my_ranges = {}
    for a in arr:
        _, _, s, e = a
        included = is_included(my_ranges, s, e)
        if included:
            continue  # we will fill up the blanks later.
        else:
            overlaps = has_overlap(my_ranges, s, e)
            if overlaps:
                # unlike include which can happen only once in a set of
                # non-overlapping ranges, overlaps can happen on multiple parts.
                # The rule is, the later child gets the say. So, we recursively
                # remove any ranges that overlap with the current one from the
                # overlapped range.
                # assert len(overlaps) == 1
                #oitem = list(overlaps)[0]
                for oitem in overlaps:
                    v = remove_overlap_from(my_ranges[oitem], (s,e))
                    del my_ranges[oitem]
                    if v:
                        my_ranges[v[2:4]] = v
                    my_ranges[(s, e)] = a
            else:
                my_ranges[(s, e)] = a
    res = my_ranges.values()
    # assert no overlap, and order by starting index
    s = sorted(res, key=lambda x: x[2])
    return s

def to_tree(node, my_str):
    method_name = ("<%s>" % node['name']) if node['name'] is not None else '<START>'
    indexes = node['indexes']
    node_children = []
    for c in node.get('children', []):
        t = to_tree(c, my_str)
        if t is None: continue
        node_children.append(t)
    idx_children = indexes_to_children(indexes, my_str)
    children = no_overlap(node_children + idx_children)
    if not children:
        return None
    start_idx = children[0][2]
    end_idx = children[-1][3]
    si = start_idx
    my_children = []
    # FILL IN chars that we did not compare. This is likely due to an i + n
    # instruction.
    for c in children:
        if c[2] != si:
            sbs = my_str[si: c[2]]
            my_children.append((sbs, [], si, c[2] - 1))
        my_children.append(c)
        si = c[3] + 1

    m = (method_name, my_children, start_idx, end_idx)
    return m

def miner(call_traces):
    my_trees = []
    for call_trace in call_traces:
        method_map = call_trace['method_map']

        first, method_tree = reconstruct_method_tree(method_map)
        comparisons = call_trace['comparisons']
        attach_comparisons(method_tree, last_comparisons(comparisons))

        my_str = call_trace['inputstr']

        #print("INPUT:", my_str, file=sys.stderr)
        tree = to_tree(method_tree[first], my_str)
        #print("RECONSTRUCTED INPUT:", tree_to_string(tree), file=sys.stderr)
        my_tree = {'tree': tree, 'original': call_trace['original'], 'arg': call_trace['arg']}
        assert tree_to_string(tree) == my_str
        my_trees.append(my_tree)
    return my_trees

import os.path, copy, random
random.seed(0)


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
    return check(my_string, module)

EXEC_MAP = {}
NODE_REGISTER = {}
TREE = None
FILE = None
def reset_generalizer():
    global NODE_REGISTER, TREE, FILE, EXEC_MAP
    NODE_REGISTER={}
    TREE = None
    FILE = None
    EXEC_MAP = {}


def check(s, module):
    if s in EXEC_MAP: return EXEC_MAP[s]
    result = do([module, s])
    with open('%s.log' % module, 'a+') as f:
        print(s, file=f)
        print(' '.join([module, '"%s"' % s]), file=f)
        print(":=", result.returncode, file=f)
        print("\n", file=f)
    v = (result.returncode == 0)
    EXEC_MAP[s] = v
    return v

def to_modifiable(derivation_tree):
    node, children, *rest = derivation_tree
    return [node, [to_modifiable(c) for c in children], *rest]

def parse_name(name):
    assert name[0] + name[-1] == '<>'
    name = name[1:-1]
    method, rest = name.split(':')
    ctrl_name, space, rest = rest.partition(' ')
    can_empty, space, stack = rest.partition(' ')
    ctrl, cname = ctrl_name.split('_')
    if ':while_' in name or ':for_' in name:
        method_stack = json.loads(stack)
        return method, ctrl, int(cname), 0, can_empty, method_stack
    elif ':if_' in name or ':switch_' in name:
        num, mstack = stack.split('#')
        method_stack = json.loads(mstack)
        return method, ctrl, int(cname), num, can_empty, method_stack

def unparse_name(method, ctrl, name, num, can_empty, cstack):
    if ctrl == 'while' or ctrl == 'for':
        return "<%s:%s_%s %s %s>" % (method, ctrl, name, can_empty, json.dumps(cstack))
    else:
        return "<%s:%s_%s %s %s#%s>" % (method, ctrl, name, can_empty, num, json.dumps(cstack))



def update_stack(node, at, new_name):
    nname, children, *rest = node
    if not (':if_' in nname or ':switch_' in nname or ':while_' in nname or ':for_' in nname):
        return
    method, ctrl, cname, num, can_empty, cstack = parse_name(nname)
    cstack[at] = new_name
    name = unparse_name(method, ctrl, cname, num, can_empty, cstack)
    #assert '?' not in name
    node[0] = name
    for c in children:
        update_stack(c, at, new_name)

def update_name(k_m, my_id, seen):
    # fixup k_m with what is in my_id, and update seen.
    original = k_m[0]
    method, ctrl, cname, num, can_empty, cstack = parse_name(original)
    #assert can_empty != '?'
    cstack[-1] = float('%d.0' % my_id)
    name = unparse_name(method, ctrl, cname, num, can_empty, cstack)
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
        method1, ctrl1, cname1, num1, can_empty1, cstack1 = parse_name(i[0])
        method2, ctrl2, cname2, num2, can_empty2, cstack2 = parse_name(j[0])
        assert method1 == method2
        assert ctrl1 == ctrl2
        assert cname1 == cname2
        #assert can_empty2 != '?'

        # fixup the can_empty
        new_name = unparse_name(method1, ctrl1, cname1, num1, can_empty2, cstack1)
        i[0] = new_name
        assert len(cstack1) == len(cstack2)
        update_stack(i, len(cstack2)-1, cstack2[-1])
    to_replace.clear()

def node_include(i, j):
    name_i, children_i, s_i, e_i = i
    name_j, children_j, s_j, e_j = j
    return s_i <= s_j and e_i >= e_j

def num_tokens(v, s):
    name, child, *rest = v
    s.add(name)
    [num_tokens(i, s) for i in child]
    return len(s)

def s_fn(v):
    return num_tokens(v[0], set())

MAX_SAMPLES=1

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
            if len(my_values) > MAX_SAMPLES:
                lst = [v for v in my_values if not node_include(v[0], k_m)]
                values = sorted(lst, key=s_fn, reverse=True)[0:MAX_SAMPLES]
            else:
                values = my_values

            # all values in v should be tried.
            replace = 0
            for v in values:
                assert v[0][0] == v_[0][0]
                if f != FILE or not node_include(v[0], k_m): # if not k_m includes v
                    a = is_compatible((k_m, FILE, TREE), v, module)
                    if not a:
                        replace = 0
                        break
                    else:
                        replace += 1
                if f != FILE or not node_include(k_m, v[0]):
                    b = is_compatible(v, (k_m, FILE, TREE), module)
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
        a = is_compatible((i_m, FILE, TREE), (['', [], 0, 0], FILE, TREE), module)
        method1, ctrl1, cname1, num1, can_empty, cstack1 = parse_name(i_m[0])
        name = unparse_name(method1, ctrl1, cname1, num1, Epsilon if a else NoEpsilon, cstack1)
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
            if not node_include(j_m, i_m):
                a = is_compatible((i_m, FILE, TREE), (j_m, FILE, TREE), module)
                if not a: continue
                replace = True
            if not node_include(i_m, j_m):
                b = is_compatible((j_m, FILE, TREE), (i_m, FILE, TREE), module)
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
                method1, ctrl1, cname1, num1, can_empty1, cstack1 = parse_name(k_m[0])
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

def generalize_loop(idx_map, while_register, module):
    # First we check the previous while loops
    check_registered_loops_for_compatibility(idx_map, while_register, module)

    # Check whether any of these can be deleted.
    can_the_loop_be_deleted(idx_map, while_register, module)

    # then we check he current while iterations
    check_current_loops_for_compatibility(idx_map, while_register, module)

    # lastly, update all while names.
    register_new_loops(idx_map, while_register)

def generalize(tree, module):
    node, children, *_rest = tree
    if node not in NODE_REGISTER:
        NODE_REGISTER[node] = {}
    register = NODE_REGISTER[node]

    for child in children:
        generalize(child, module)

    idxs = {}
    last_while = None
    for i,child in enumerate(children):
        # now we need to map the while_name here to the ones in node
        # register. Essentially, we try to replace each.
        if ':while_' not in child[0] and ':for_' not in child[0]:
            continue
        while_name = child[0].split(' ')[0]
        if last_while is None:
            last_while = while_name
            if while_name not in register:
                register[while_name] = [{}, 0]
        else:
            if last_while != while_name:
                # a new while! Generalize the last
                last_while = while_name
                generalize_loop(idxs, register[last_while])
        idxs[i] = child
    if last_while is not None:
        generalize_loop(idxs, register[last_while], module)

def generalize_iter(jtrees, log=False):
    global TREE, FILE
    new_trees = []
    for j in jtrees:
        FILE = j['arg']
        if log: print(FILE, file=sys.stderr)
        sys.stderr.flush()
        TREE = to_modifiable(j['tree'])
        generalize(TREE, j['original'])
        j['tree'] = TREE
        new_trees.append(copy.deepcopy(j))
    return new_trees


import re
RE_NONTERMINAL = re.compile(r'(<[^<> ]*>)')

def recurse_grammar(grammar, key, order, canonical):
    rules = sorted(grammar[key])
    old_len = len(order)
    for rule in rules:
        if not canonical:
            res =  re.findall(RE_NONTERMINAL, rule)
        else:
            res = rule
        for token in res:
            if token.startswith('<') and token.endswith('>'):
                if token not in order:
                    order.append(token)
    new = order[old_len:]
    for ckey in new:
        recurse_grammar(grammar, ckey, order, canonical)

def show_grammar(grammar, start_symbol='<START>', canonical=True):
    order = [start_symbol]
    recurse_grammar(grammar, start_symbol, order, canonical)
    assert len(order) == len(grammar.keys())
    return {k: sorted(grammar[k]) for k in order}

def to_grammar(tree, grammar):
    node, children, _, _ = tree
    tokens = []
    if node not in grammar:
        grammar[node] = list()
    for c in children:
        if c[1] == []:
            tokens.append(c[0])
        else:
            tokens.append(c[0])
            to_grammar(c, grammar)
    grammar[node].append(tuple(tokens))
    return grammar

def merge_grammar(g1, g2):
    all_keys = set(list(g1.keys()) + list(g2.keys()))
    merged = {}
    for k in all_keys:
        alts = set(g1.get(k, []) + g2.get(k, []))
        merged[k] = alts
    return {k:[l for l in merged[k]] for k in merged}

def convert_to_grammar(my_trees):
    grammar = {}
    for my_tree in my_trees:
        tree = my_tree['tree']
        src = my_tree['original']
        g = to_grammar(tree, grammar)
        grammar = merge_grammar(grammar, g)
    return grammar

def to_fuzzable_grammar(grammar):
    def escape(t):
        if ((t[0]+t[-1]) == '<>'):
            return t.replace(' ', '_')
        else:
            return t
    new_g = {}
    for k in grammar:
        new_alt = []
        for rule in grammar[k]:
            new_alt.append(''.join([escape(t) for t in rule]))
        new_g[k.replace(' ', '_')] = new_alt
    return new_g


def check_empty_rules(grammar):
    new_grammar = {}
    for k in grammar:
        if k in ':if_' or k in ':switch_':
            name, marker = k.split('#')
            if name.endswith(' *'):
                new_grammar[k] = grammar[k].add(('',))
            else:
                new_grammar[k] = grammar[k]
        elif k in ':while_' or k in ':for_':
            # TODO -- we have to check the rules for sequences of whiles.
            # for now, ignore.
            new_grammar[k] = grammar[k]
        else:
            new_grammar[k] = grammar[k]
    return new_grammar

import json
class Buf:
    def __init__(self, size):
        self.size = size
        self.items = [None] * self.size

class Buf(Buf):
    def add1(self, items):
        self.items.append(items.pop(0))
        return self.items.pop(0)

class Buf(Buf):
    def __eq__(self, items):
        if any(isinstance(i, dict) for i in self.items): return False
        if any(isinstance(i, dict) for i in items): return False
        return items == self.items

def detect_chunks(n, lst_):
    lst = list(lst_)
    chunks = set()
    last = Buf(n)
    # check if the next_n elements are repeated.
    for _ in range(len(lst) - n):
        lnext_n = lst[0:n]
        if last == lnext_n:
            # found a repetition.
            chunks.add(tuple(last.items))
        else:
            pass
        last.add1(lst)
    return chunks

def chunkify(lst_,n , chunks):
    lst = list(lst_)
    chunked_lst = []
    while len(lst) >= n:
        lnext_n = lst[0:n]
        if (not any(isinstance(i, dict) for i in lnext_n)) and tuple(lnext_n) in chunks:
            chunked_lst.append({'_':lnext_n})
            lst = lst[n:]
        else:
            chunked_lst.append(lst.pop(0))
    chunked_lst.extend(lst)
    return chunked_lst


def identify_chunks(my_lsts):
    # initialize
    all_chunks = {}
    maximum = max(len(lst) for lst in my_lsts)
    for i in range(1, maximum//2+1):
        all_chunks[i] = set()

    # First, identify chunks in each list.
    for lst in my_lsts:
        for i in range(1,maximum//2+1):
            chunks = detect_chunks(i, lst)
            all_chunks[i] |= chunks

    # Then, chunkify
    new_lsts = []
    for lst in my_lsts:
        for i in range(1,maximum//2+1):
            chunks = all_chunks[i]
            lst = chunkify(lst, i, chunks)
        new_lsts.append(lst)
    return new_lsts

class Node:
    # Each tree node gets its unique id.
    _uid = 0
    def __init__(self, item):
        # self.repeats = False
        self.count = 1 # how many repetitions.
        self.counters = set()
        self.last = False
        self.children = []
        self.item = item
        self.uid = Node._uid
        Node._uid += 1

    def update_counters(self):
        self.counters.add(self.count)
        self.count = 0
        for c in self.children:
            c.update_counters()

    def __repr__(self):
        return str(self.to_json())

    def __str__(self):
        return str("(%s, [%s])", (self.item, ' '.join([str(i) for i in self.children])))

    def to_json(self):
        s = ("(%s)" % ' '.join(self.item['_'])) if isinstance(self.item, dict) else str(self.item)
        return (s, tuple(self.counters), [i.to_json() for i in self.children])

    def inc_count(self):
        self.count += 1

    def add_ref(self):
        self.count = 1

    def get_child(self, c):
        for i in self.children:
            if i.item == c: return i
        return None

    def add_child(self, c):
        # first check if it is the current node. If it is, increment
        # count, and return ourselves.
        if c == self.item:
            self.inc_count()
            return self
        else:
            # check if it is one of the children. If it is a child, then
            # preserve its original count.
            nc = self.get_child(c)
            if nc is None:
                nc = Node(c)
                self.children.append(nc)
            else:
                nc.add_ref()
            return nc

def update_tree(lst_, root):
    lst = list(lst_)
    branch = root
    while lst:
        first, *lst = lst
        branch = branch.add_child(first)
    branch.last = True
    return root

def create_tree_with_lsts(lsts):
    Node._uid = 0
    root =  Node(None)
    for lst in lsts:
        root.count = 1 # there is at least one element.
        update_tree(lst, root)
        root.update_counters()
    return root

def get_star(node, key):
    if node.item is None:
        return ''
    if isinstance(node.item, dict):
        # take care of counters
        elements = node.item['_']
        my_key = "<%s-%d-s>" % (key, node.uid)
        alts = [elements]
        if len(node.counters) > 1: # repetition
            alts.append(elements + [my_key])
        return [my_key], {my_key:alts}
    else:
        return [str(node.item)], {}

def node_to_grammar(node, grammar, key):
    rule = []
    alts = [rule]
    if node.uid == 0:
        my_key = "<%s>" % key
    else:
        my_key = "<%s-%d>" % (key, node.uid)
    grammar[my_key] = alts
    if node.item is not None:
        mk, g = get_star(node, key)
        rule.extend(mk)
        grammar.update(g)
    # is the node last?
    if node.last:
        assert node.item is not None
        # add a duplicate rule that ends here.
        ending_rule = list(rule)
        # if there are no children, the current rule is
        # any way ending.
        if node.children:
            alts.append(ending_rule)

    if node.children:
        if len(node.children) > 1:
            my_ckey = "<%s-%d-c>" % (key, node.uid)
            rule.append(my_ckey)
            grammar[my_ckey] = [ ["<%s-%d>" % (key, c.uid)] for c in node.children]
        else:
            my_ckey = "<%s-%d>" % (key, node.children[0].uid)
            rule.append(my_ckey)
    else:
        pass
    for c in node.children:
        node_to_grammar(c, grammar, key)
    return grammar

def generate_grammar(lists, key):
    lsts = identify_chunks(lists)
    tree = create_tree_with_lsts(lsts)
    grammar = {}
    node_to_grammar(tree, grammar, key)
    return grammar

def collapse_alts(rules, k):
    ss = [[str(r) for r in rule] for rule in rules]
    x = generate_grammar(ss, k[1:-1])
    return x

def collapse_rules(grammar):
    r_grammar = {}
    for k in grammar:
        new_grammar = collapse_alts(grammar[k], k)
        # merge the new_grammar with r_grammar
        # we know none of the keys exist in r_grammar because
        # new keys are k prefixed.
        for k_ in new_grammar:
            r_grammar[k_] = new_grammar[k_]
    return r_grammar

def convert_spaces(grammar):
    keys = {key: key.replace(' ', '_') for key in grammar}
    new_grammar = {}
    for key in grammar:
        new_alt = []
        for rule in grammar[key]:
            new_rule = []
            for t in rule:
                for k in keys:
                    t = t.replace(k, keys[k])
                new_rule.append(t)
            new_alt.append(''.join(new_rule))
        new_grammar[keys[key]] = new_alt
    return new_grammar

def first_in_chain(token, chain):
    while True:
        if token in chain:
            token = chain[token]
        else:
            break
    return token

def new_symbol(grammar, symbol_name="<symbol>"):
    if symbol_name not in grammar:
        return symbol_name

    count = 1
    while True:
        tentative_symbol_name = symbol_name[:-1] + "-" + repr(count) + ">"
        if tentative_symbol_name not in grammar:
            return tentative_symbol_name
        count += 1

def replacement_candidates(grammar):
    to_replace = {}
    for k in grammar:
        if len(grammar[k]) != 1: continue
        if k in {'<START>', '<main>'}: continue
        rule = grammar[k][0]
        res =  re.findall(RE_NONTERMINAL, rule)
        if len(res) == 1:
            if len(res[0]) != len(rule): continue
            to_replace[k] = first_in_chain(res[0], to_replace)
        elif len(res) == 0:
            to_replace[k] = first_in_chain(rule, to_replace)
        else:
            continue # more than one.
    return to_replace

def replace_key_by_new_key(grammar, keys_to_replace):
    new_grammar = {}
    for key in grammar:
        new_rules = []
        for rule in grammar[key]:
            for k in keys_to_replace:
                new_key = keys_to_replace[k]
                rule = rule.replace(k, keys_to_replace[k])
            new_rules.append(rule)
        new_grammar[keys_to_replace.get(key, key)] = new_rules
    assert len(grammar) == len(new_grammar)
    return new_grammar

def replace_key_by_key(grammar, keys_to_replace):
    new_grammar = {}
    for key in grammar:
        if key in keys_to_replace:
            continue
        new_rules = []
        for rule in grammar[key]:
            for k in keys_to_replace:
                new_key = keys_to_replace[k]
                rule = rule.replace(k, keys_to_replace[k])
            new_rules.append(rule)
        new_grammar[key] = new_rules
    return new_grammar


def remove_single_entries(grammar):
    keys_to_replace = replacement_candidates(grammar)
    return replace_key_by_key(grammar, keys_to_replace)

def collect_duplicate_rule_keys(grammar):
    collect = {}
    for k in grammar:
        salt = str(sorted(grammar[k]))
        if salt not in collect:
            collect[salt] = (k, set())
        else:
            collect[salt][1].add(k)
    return collect

def remove_duplicate_rule_keys(grammar):
    g = grammar
    while True:
        collect = collect_duplicate_rule_keys(g)
        keys_to_replace = {}
        for salt in collect:
            k, st = collect[salt]
            for s in st:
                keys_to_replace[s] = k
        if not keys_to_replace:
            break
        g = replace_key_by_key(g, keys_to_replace)
    return g

def collect_replacement_keys(grammar):
    g = copy.deepcopy(grammar)
    to_replace = {}
    for k in grammar:
        if ':' in k:
            first, rest = k.split(':')
            sym = new_symbol(g, symbol_name=first + '>')
            assert sym not in g
            g[sym] = None
            to_replace[k] = sym
        else:
            continue
    return to_replace

def cleanup_tokens(grammar):
    keys_to_replace = collect_replacement_keys(grammar)
    g = replace_key_by_new_key(grammar, keys_to_replace)
    return g

def replaceAngular(grammar):
    new_g = {}
    replaced = False
    for k in grammar:
        new_rules = []
        for rule in grammar[k]:
            new_rule = rule.replace('<>', '<openA><closeA>').replace('</>', '<openA>/<closeA>')
            if rule != new_rule:
                replaced = True
            new_rules.append(new_rule)
        new_g[k] = new_rules
    if replaced:
        new_g['<openA>'] = ['<']
        new_g['<closeA>'] = ['<']
    return new_g

import math

def len_to_start(item, parents, seen=None):
    if seen is None: seen = set()
    if item in seen:
        return math.inf
    seen.add(item)
    if item == '<START>':
        return 0
    else:
        return 1 + min(len_to_start(p, parents, seen) for p in parents[item])

def order_by_length_to_start(items, parents):
    return sorted(items, key=lambda i: len_to_start(i, parents))

def id_parents(grammar, key, seen=None, parents=None):
    if parents is None:
        parents = {}
        seen = set()
    if key in seen: return
    seen.add(key)
    for rule in grammar[key]:
        res = re.findall(RE_NONTERMINAL, rule)
        for token in res:
            if token.startswith('<') and token.endswith('>'):
                if token not in parents: parents[token] = list()
                parents[token].append(key)
    for ckey in {i for i in  grammar if i not in seen}:
        id_parents(grammar, ckey, seen, parents)
    return parents

def remove_single_alts(grammar, start_symbol='<START>'):
    single_alts = {p for p in grammar if len(grammar[p]) == 1 and p != start_symbol}

    child_parent_map = id_parents(grammar, start_symbol)

    single_refs = {p:child_parent_map[p] for p in single_alts if len(child_parent_map[p]) <= 1}

    keys_to_replace = {p:grammar[p][0] for p in order_by_length_to_start(single_refs, child_parent_map)}
    g =  replace_key_by_key(grammar, keys_to_replace)
    return g

def main(tracefile):
    with open(tracefile) as f:
        my_trace = json.load(f)
    mined_trees = miner(my_trace)
    with open('build/t1.json', 'w+') as f: json.dump(mined_trees, f)
    generalized_trees = generalize_iter(mined_trees)
    g = convert_to_grammar(generalized_trees)
    with open('build/g1_.json', 'w+') as f: json.dump(g, f)
    g = check_empty_rules(g)
    with open('build/g2_.json', 'w+') as f: json.dump(g, f)
    g = collapse_rules(g)
    with open('build/g3_.json', 'w+') as f: json.dump(g, f)
    g = convert_spaces(g)
    with open('build/g4_.json', 'w+') as f: json.dump(g, f)
    e = remove_single_entries(g)
    with open('build/g5_.json', 'w+') as f: json.dump(e, f)
    e = remove_duplicate_rule_keys(e)
    with open('build/g6_.json', 'w+') as f: json.dump(e, f)
    e = cleanup_tokens(e)
    with open('build/g7_.json', 'w+') as f: json.dump(e, f)
    e = remove_single_alts(e)
    with open('build/g8_.json', 'w+') as f: json.dump(e, f)
    e = show_grammar(e, canonical=False)
    with open('build/g.json', 'w+') as f: json.dump(e, f)

main(sys.argv[1])


