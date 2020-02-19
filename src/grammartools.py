def remove_duplicate_rules_in_a_key(g):
    g_ = {}
    for k in g:
        s = {str(r):r for r in g[k]}
        g_[k] = list(sorted(list(s.values())))
    return g_

def len_rule(r): return len(r)
def len_definition(d): return sum([len_rule(r) for r in d])
def len_grammar(g): return sum([len_definition(g[k]) for k in g])

