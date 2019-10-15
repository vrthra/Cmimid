import sys
import json

import itertools as it

from operator import itemgetter
from fuzzingbook.GrammarFuzzer import tree_to_string

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
    # first, for each method, find the index that
    # was accessed in that method invocation last.
    for idx, char, mid in comparisons:
        if mid in last_idx:
            if idx > last_idx[mid]:
                last_idx[mid] = idx
        else:
            last_idx[mid] = idx

    # next, for each index, find the method that
    # accessed it last.
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

import os.path, copy, random
random.seed(0)

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

def main(tracefile):
    with open(tracefile) as f:
        my_trace = json.load(f)
    mined_trees = miner(my_trace)
    json.dump(mined_trees, sys.stdout)
main(sys.argv[1])
