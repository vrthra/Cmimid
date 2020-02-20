import sys
import random
import json
import re
import fuzzingbook.Parser as P
from fuzzingbook.GrammarFuzzer import tree_to_string
RE_NONTERMINAL = re.compile(r'(<[^<> ]*>)')

class Fuzzer:
    def __init__(self, grammar):
        self.grammar = grammar

    def print(self):
        for k in self.grammar:
            print(k)
            for rule in self.grammar[k]:
                print("\t", rule)

    def fuzz_key(self, key='<START>', path=[]):
        path.append(key)
        #print(":", key)
        if key not in self.grammar: return key
        choice = random.choice(self.grammar[key])
        if isinstance(choice, list):
            return self.fuzz_rule(choice, path)
        else:
            return self.fuzz_rule(list(re.split(RE_NONTERMINAL, choice)), path)

    def fuzz_rule(self, rule, path):
        #print("\t",rule)
        fuzzed = [self.fuzz_key(token, path) for token in rule]
        return ''.join(fuzzed)


import string
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

class Fuzzer:
    def __init__(self, grammar):
        self.grammar = grammar

    def fuzz(self, key='<start>', max_num=None, max_depth=None):
        raise NotImplemented()

class LimitFuzzer(Fuzzer):
    def symbol_cost(self, grammar, symbol, seen):
        if symbol in self.key_cost: return self.key_cost[symbol]
        if symbol in seen:
            self.key_cost[symbol] = float('inf')
            return float('inf')
        v = min((self.expansion_cost(grammar, rule, seen | {symbol})
                    for rule in grammar.get(symbol, [])), default=0)
        self.key_cost[symbol] = v
        return v

    def expansion_cost(self, grammar, tokens, seen):
        return max((self.symbol_cost(grammar, token, seen)
                    for token in tokens if token in grammar), default=0) + 1

    def gen_key(self, key, depth, max_depth):
        if key in ASCII_MAP:
            return (random.choice(ASCII_MAP[key]), [])
        if key not in self.grammar: return (key, [])
        if depth > max_depth:
            clst = sorted([(self.cost[key][str(rule)], rule) for rule in self.grammar[key]])
            rules = [r for c,r in clst if c == clst[0][0]]
        else:
            rules = self.grammar[key]
        return (key, self.gen_rule(random.choice(rules), depth+1, max_depth))

    def gen_rule(self, rule, depth, max_depth):
        return [self.gen_key(token, depth, max_depth) for token in rule]

    def fuzz(self, key='<start>', max_depth=10):
        return tree_to_string(self.gen_key(key=key, depth=0, max_depth=max_depth))

    def __init__(self, grammar):
        super().__init__(grammar)
        self.key_cost = {}
        self.cost = self.compute_cost(grammar)

    def compute_cost(self, grammar):
        cost = {}
        for k in grammar:
            cost[k] = {}
            for rule in grammar[k]:
                cost[k][str(rule)] = self.expansion_cost(grammar, rule, set())
            if len(grammar[k]):
                assert len([v for v in cost[k] if v != float('inf')]) > 0
        return cost


import subprocess
def main(args):
    errors = []
    with open(args[0]) as f:
        s = json.load(f)
    grammar = s['[grammar]']
    command = s['[command]']
    if len(args) > 1:
        command = args[1]
    f = LimitFuzzer(grammar)
    key = args[2] if len(args)> 2 else s['[start]']
    for i in range(100):
        try:
            v = f.fuzz(key)
            print(repr(v))
            p = subprocess.Popen(command, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            data, err = p.communicate(input=v.encode())
            #print(p.returncode)
            if p.returncode != 0:
                errors.append(v)
        except RecursionError:
            pass
    return errors

def process_token(i):
    if i and i[0] == '<' and ' ' in  i:
        return i.split(' ')[0] + '>'
    elif i and i[0] == '<':
        return i
    else:
        return repr(i)

if __name__ == '__main__':
    errors = main(sys.argv[1:])
    print()
    for e in errors:
        print(repr(e))
        print()

    print(len(errors))
