import sys
import random
import json
import re
RE_NONTERMINAL = re.compile(r'(<[^<> ]*>)')

class Fuzzer:
    def __init__(self, grammar):
        self.grammar = grammar

    def print(self):
        for k in self.grammar:
            print(k)
            for rule in self.grammar[k]:
                print("\t", rule)

    def fuzz_key(self, key='<START>'):
        #print(":", key)
        if key not in self.grammar: return key
        choice = random.choice(self.grammar[key])
        return self.fuzz_rule(list(re.split(RE_NONTERMINAL, choice)))

    def fuzz_rule(self, rule):
        #print("\t",rule)
        fuzzed = [self.fuzz_key(token) for token in rule]
        return ''.join(fuzzed)


def main(args):
    with open(args[0]) as f:
        s = json.load(f)
        f = Fuzzer(s)
    for i in range(100):
        try:
            v = f.fuzz_key()
            print(v)
        except RecursionError:
            pass

main(sys.argv[1:])
