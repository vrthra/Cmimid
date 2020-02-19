#!/usr/bin/env python
import sys
import sys
sys.setrecursionlimit(3000)
import random
import string
import util
import copy
import json
import re
import fuzz as F
import subprocess
def is_nt(token):
    return token.startswith('<') and token.endswith('>')

def generalize_tokens(grammar):
    g_ = {}
    for k in grammar:
        new_rules = []
        for rule in grammar[k]:
            new_rule = []
            for token in rule:
                if not is_nt(token):
                    new_rule.extend(list(token))
                else:
                    new_rule.append(token)
            new_rules.append(new_rule)
        g_[k]  = new_rules
    return g_

def get_list_of_single_chars(grammar):
    lst = []
    for p,k in enumerate(grammar):
        for q,rule in enumerate(grammar[k]):
            for r,token in enumerate(rule):
                if is_nt(token): continue
                if len(token) == 1:
                    lst.append((k, q, r))
    return lst

def reachable_dict(grammar):
    reachable = {}
    for key in grammar:
        keys = find_reachable_keys(grammar, key, reachable)
        reachable[key] = keys
    return reachable

def find_reachable_keys(grammar, key, reachable_keys=None, found_so_far=None):
    if reachable_keys is None: reachable_keys = {}
    if found_so_far is None: found_so_far = set()

    for rule in grammar[key]:
        for token in rule:
            if not is_nt(token): continue
            if token in found_so_far: continue
            found_so_far.add(token)
            if token in reachable_keys:
                for k in reachable_keys[token]:
                    found_so_far.add(k)
            else:
                keys = find_reachable_keys(grammar, token, reachable_keys, found_so_far)
                # reachable_keys[token] = keys <- found_so_far contains results from earlier
    return found_so_far

def get_reachable_positions(rule, fkey, reachable):
    positions = []
    for i, token in enumerate(rule):
        if not is_nt(token): continue
        if fkey == token or fkey in reachable[token]:
            positions.append(i)
    return positions

#def generate_focus_grammar(g_, focus):
#    val = g_[focus]
#    reachable_keys = reachable_dict(g_)
#    my_grammar = {}
#    for k in g_:
#        rs = []
#        for r in g_[k]:
#            positions = get_reachable_positions(r, focus, reachable_keys)
#            if positions:
#                rs.append(r)
#        if rs:
#            my_grammar[k] = rs
#        else:
#            my_grammar[k] = g_[k]
#
#    my_grammar[focus] = val
#    return my_grammar

def key_reachable_from_rule(key, rule, reachable_keys):
    for token in rule:
        if not is_nt(token): continue
        if token == key: return True
        if key in reachable_keys[token]:
            return True
    return False


def find_path_rule(g_, rule, gk, reachable_keys):
    assert key_reachable_from_rule(gk, rule, reachable_keys)
    ret = []
    choices = []
    for i,token in enumerate(rule):
        if not is_nt(token):
            ret.append((token, []))
        elif gk in reachable_keys[token]:
            choices.append(i)
            #tree = find_path_key(g_, token, gk, reachable_keys)
            ret.append((token, None))
        elif gk == token:
            choices.append(i)
            ret.append((token, None))
        else:
            ret.append((token, None))

    choice = random.choice(choices)
    token, val = ret[choice]
    assert val is None
    if token == gk:
        return ret
    else:
        ret[choice] = find_path_key(g_, token, gk, reachable_keys)
    return ret

def find_path_key(g_, key, gk, reachable_keys):
    assert gk in reachable_keys[key]
    #which rule can we choose?
    choices = []
    for i, rule in enumerate(g_[key]):
        if key_reachable_from_rule(gk, rule, reachable_keys):
            choices.append(i)
    # pick one
    rule_i = random.choice(choices)
    return (key, find_path_rule(g_, g_[key][rule_i], gk, reachable_keys))


def remove_recursion(d):
    new_d = {}
    for k in d:
        new_rs = []
        for t in d[k]:
            if t != k:
                new_rs.append(t)
        new_d[k] = new_rs
    return new_d

def flush_tree(stree, fuzzer, gk):
    key, children = stree
    if key == gk:
        return (gk, [])
    if children is None:
        return fuzzer.gen_key(key, depth=0, max_depth=1)
    else:
        return (key, [flush_tree(c, fuzzer, gk) for c in children])

def replaceable_with_kind(stree, orig, parent, gk, command):
    my_node = None
    def fill_tree(node):
        nonlocal my_node
        name, children = node
        if name == gk:
            my_node = [name, [[parent, []]]]
            return my_node
        elif not children:
            if name in ASCII_MAP:
                return (random.choice(ASCII_MAP[name]), [])
            return (name, [])
        else:
            return (name, [fill_tree(c) for c in children])

    tree0 = fill_tree(stree)
    sval = util.tree_to_str(tree0)
    assert my_node is not None
    a1 = my_node, '', tree0
    for pval in ASCII_MAP[parent]:
        aX = ((gk, [[pval, []]]), '', tree0)
        val = util.is_a_replaceable_with_b(a1, aX, command)
        if val:
            continue
        else:
            return False
    return True


# string.ascii_letters The concatenation of the ascii_lowercase and ascii_uppercase constants described below. This value is not locale-dependent.
# string.ascii_lowercase The lowercase letters 'abcdefghijklmnopqrstuvwxyz'. This value is not locale-dependent and will not change.
# string.ascii_uppercase The uppercase letters 'ABCDEFGHIJKLMNOPQRSTUVWXYZ'. This value is not locale-dependent and will not change.
# string.digits The string '0123456789'.
# string.hexdigits The string '0123456789abcdefABCDEF'.
# string.octdigits The string '01234567'.
# string.punctuation String of ASCII characters which are considered punctuation characters in the C locale: !"#$%&'()*+,-./:;<=>?@[\]^_`{|}~.
# string.printable String of ASCII characters which are considered printable. This is a combination of digits, ascii_letters, punctuation, and whitespace.
# string.whitespace A string containing all ASCII characters that are considered whitespace. This includes the characters space, tab, linefeed, return, formfeed, and vertical tab.

def parent_map():
    parent = {}
    for sp in string.whitespace:
        parent[sp] = '[__WHITESPACE__]'
    for digit in string.digits:
        parent[digit] = '[__DIGIT__]'
    for ll in string.ascii_lowercase:
        parent[ll] = '[__ASCII_LOWER__]'
    for ul in string.ascii_uppercase:
        parent[ul] = '[__ASCII_UPPER__]'
    for p in string.punctuation:
        parent[p] = '[__ASCII_PUNCT__]'

    parent['[__WHITESPACE__]'] = '[__ASCII_PRINTABLE__]'

    parent['[__DIGIT__]']      = '[__ASCII_ALPHANUM__]'
    parent['[__ASCII_LOWER__]']      = '[__ASCII_LETTER__]'
    parent['[__ASCII_UPPER__]']      = '[__ASCII_LETTER__]'
    parent['[__ASCII_LETTER__]']      = '[__ASCII_ALPHANUM__]'
    parent['[__ASCII_ALPHANUM__]']      = '[__ASCII_PRINTABLE__]'
    parent['[__PUNCT__]']               = '[__ASCII_PRINTABLE__]'
    return parent

ASCII_MAP = {
        '[__WHITESPACE__]': string.whitespace,
        '[__DIGIT__]': string.digits,
        '[__ASCII_LOWER__]': string.ascii_lowercase,
        '[__ASCII_UPPER__]': string.ascii_uppercase,
        '[__ASCII_PUNCT__]': string.punctuation,
        '[__ASCII_LETTER__]': string.ascii_letters,
        '[__ASCII_ALPHANUM__]': string.ascii_letters + string.digits,
        '[__ASCII_PRINTABLE__]': string.printable
        }
PARENT_MAP = parent_map()
def find_max_generalized(tree, kind, gk, command):
    if kind not in PARENT_MAP: return kind
    parent = PARENT_MAP[kind]
    if replaceable_with_kind(tree, kind, parent, gk, command):
        return find_max_generalized(tree, parent, gk, command)
    else:
        return kind

GK = '<__GENERALIZE__>'
def generalize_single_token(grammar, start, k, q, r, command):
    # first we replace the token with a temporary key
    gk = GK
    g_ = copy.deepcopy(grammar)
    char = g_[k][q][r]
    g_[k][q][r] = gk
    g_[gk] = [[char]]
    reachable_keys = remove_recursion(reachable_dict(g_))
    # now, we need a path to reach this.
    skel_tree = find_path_key(g_, start, gk, reachable_keys)
    tree = flush_tree(skel_tree, F.LimitFuzzer(grammar), gk)
    gen_token = find_max_generalized(tree, char, gk, command)
    del g_[gk]
    g_[k][q][r] = gen_token
    return g_

def main(args):
    gfname = args[0]
    with open(gfname) as f:
        gf = json.load(fp=f)
    grammar = gf['[grammar]']
    start = gf['[start]']
    command = gf['[command]']


    # now, what we want to do is first regularize the grammar by splitting each
    # multi-character tokens into single characters.
    generalized_grammar = generalize_tokens(grammar)

    # next, we want to get the list of all such instances

    list_of_things_to_generalize = get_list_of_single_chars(generalized_grammar)
    print(len(list_of_things_to_generalize), file=sys.stderr)

    # next, we want to generalie each in turn
    # finally, we want to generalize the length.
    reachable_keys = reachable_dict(grammar)
    g_ = generalized_grammar
    for k, q, r in list_of_things_to_generalize:
        g_ = generalize_single_token(g_, start, k, q, r, command)

    # finally, we want to generalize the length.
    #g = generalize_size(g_)
    print(json.dumps({'[start]': start, '[grammar]':g_, '[command]': command}))

if __name__ == '__main__':
    main(sys.argv[1:])
