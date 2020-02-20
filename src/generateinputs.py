import sys
import os
import random
import json
import re
import fuzz as F
import fuzzingbook.Parser as P

import subprocess
def main(args):
    errors = []
    with open(args[0]) as f:
        s = json.load(f)
    grammar = s['[grammar]']
    start = s['[start]']
    key = args[1]
    command = args[2]
    directory = args[3]
    os.makedirs(directory, exist_ok=True)
    f = F.LimitFuzzer(grammar)
    for i in range(10):
        try:
            v = f.fuzz(start)
            print(repr(v))
            p = subprocess.Popen(command, stdin=subprocess.PIPE, stdout=subprocess.PIPE, stderr=subprocess.PIPE)
            data, err = p.communicate(input=v.encode())
            #print(p.returncode)
            if p.returncode != 0:
                errors.append(v)
            else:
                with open('%s/%s.input.%d' % (directory, key, i), 'w+') as fn:
                    print(v, end='', file=fn)
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
    main(sys.argv[1:])

