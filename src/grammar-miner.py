import sys
import util
import pudb
bp = pudb.set_trace
import json
import subprocess

import copy, random
random.seed(0)

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
    if not children: return grammar
    tokens = []
    if node not in grammar:
        grammar[node] = list()
    for c in children:
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
    ret = []
    for my_tree in my_trees:
        tree = my_tree['tree']
        src_file = my_tree['original']
        arg_file = my_tree['arg']
        ret.append((src_file, arg_file))
        g = to_grammar(tree, grammar)
        grammar = merge_grammar(grammar, g)
    return ret, grammar

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
        if ':if_' in k:
            name, marker = k.split('#')
            if name.endswith(' *'):
                new_grammar[k] = grammar[k].add(('',))
            else:
                new_grammar[k] = grammar[k]
        elif k in ':while_': # or k in ':for_':
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

def convert_spaces_in_keys(grammar):
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
            new_alt.append(new_rule)
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

def len_grammar(g):
    return sum([len(g[k]) for k in g])


def shrink_rules_cf(g_):
    g = dict(g_)
    for k in g:
        if len(g[k]) != len(set(g[k])):
            v = list(sorted(list(set(g[k]))))
            g[k] = v
    return g

def remove_redundant_tokens_f(g):
    g_ = {}
    for k in g:
        rs_ = []
        for r in g[k]:
            assert isinstance(r, str)
            if r == k:
                continue
            rs_.append(r)
        g_[k] = rs_
    return g_



def remove_redundant_tokens_c(g):
    g_ = {}
    for k in g:
        rs_ = []
        for r in g[k]:
            assert not isinstance(r, str)
            r_ = []
            for t in r:
                if t == k:
                    continue
                r_.append(t)
            rs_.append(r_)
        g_[k] = rs_
    return g_


def non_canonical(grammar):
    new_grammar = {}
    for k in grammar:
        rules = grammar[k]
        new_rules = []
        for rule in rules:
            new_rules.append(''.join(rule))
        new_grammar[k] = new_rules
    return new_grammar

def main(tracefile):
    with open(tracefile) as f:
        generalized_trees  = json.load(f)
    ret, g = convert_to_grammar(generalized_trees)
    cmds = {src for src,arg in ret}
    assert len(cmds) == 1
    cmd = list(cmds)[0]

    with open('build/g1_.json', 'w+') as f: json.dump(g, f)
    g = check_empty_rules(g) # add optional rules
    with open('build/g2_.json', 'w+') as f: json.dump(g, f)
    g = collapse_rules(g) # learn regex
    with open('build/g3_.json', 'w+') as f: json.dump(g, f)
    g = convert_spaces_in_keys(g) # fuzzable grammar
    with open('build/g4_.json', 'w+') as f: json.dump(g, f)
    g = non_canonical(g)
    with open('build/g5_.json', 'w+') as f: json.dump(g, f)

    l = len_grammar(g)
    diff = 1
    while diff > 0:
        e = remove_single_entries(g)
        e = remove_duplicate_rule_keys(e)
        e = cleanup_tokens(e)
        e = remove_single_alts(e)

        e = shrink_rules_cf(e)
        e = remove_redundant_tokens_f(e)
        g = e
        l_ = len_grammar(g)
        diff = l - l_
        l = l_
    e = show_grammar(e, canonical=False)
    with open('build/g.json', 'w+') as f: json.dump({'[start]': '<START>', '[grammar]':e, '[command]':cmd}, fp=f)

main(sys.argv[1])

