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

import subprocess
errors = []
def main(args):
    with open(args[0]) as f:
        s = json.load(f)
        f = Fuzzer(s)
    for i in range(100):
        try:
            path = []
            v = f.fuzz_key(args[2] if len(args)> 2 else '<START>', path=path)
            print(repr(v))
            p = subprocess.Popen(args[1], stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            data, err = p.communicate(input=v.encode())
            #print(p.returncode)
            if p.returncode != 0:
                errors.append((v, path))
        except RecursionError:
            pass

main(sys.argv[1:])
print()
def process_token(i):
    if i and i[0] == '<' and ' ' in  i:
        return i.split(' ')[0] + '>'
    elif i and i[0] == '<':
        return i
    else:
        return repr(i)
for e,p in errors:
    print(repr(e))
    print(' '.join([process_token(i) for i in p]))
    print()

print(len(errors))
