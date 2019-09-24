#!/usr/bin/env python
import sys
import pudb
bp = pudb.set_trace
import json

import mimid_context
import taints

CMIMID_EXIT=0
CMIMID_BREAK=1
CMIMID_CONTINUE=2
CMIMID_FOR=3
CMIMID_WHILE=4
CMIMID_IF=5
CMIMID_SWITCH=6

kind_map={CMIMID_FOR:'for', CMIMID_WHILE:'while', CMIMID_IF:'if', CMIMID_SWITCH:'switch'}

class O:
    def __init__(self, **keys): self.__dict__.update(keys)
    def __repr__(self): return str(self.__dict__)

def read_json(json_file):
    json_arr = []
    with open(json_file) as f:
        arr =  f.readlines()
    for a in arr:
        json_arr.append(json.loads(a))
    return json_arr

cmimid_stack =  []
pseudo_method_stack = []
gen_events = []

def to_key(method, name, num): return '%s:%s_%s' % (method, name, num)

def track_stack(e):
    if e.fun in {'cmimid__method_enter'}:
        mid, *args = e.info
        cmimid_stack.append(('method', mid))
        method_name = METHOD_PREFIX[-1]
        gen_events.append(('method_enter', mid, method_name))

    elif e.fun in {'cmimid__method_exit'}:
        method, mid = cmimid_stack.pop()
        assert method == 'method'
        gen_events.append(('method_exit', mid))
        method_name = METHOD_PREFIX[-1]


    elif e.fun in {'cmimid__stack_enter'}:
        stack_kind, stack_id, *args = e.info
        pseudo_method_stack.append((stack_id, kind_map[stack_kind]))
        cmimid_stack.append(('stack', stack_kind, stack_id, args))
        gen_events.append(('stack_enter', kind_map[stack_kind], stack_id))

    elif e.fun in {'cmimid__stack_exit'}:
        stack, stack_kind, stack_id, args = cmimid_stack.pop()
        assert stack == 'stack'
        gen_events.append(('stack_exit', stack_kind, stack_id))
        pseudo_method_stack.pop()

    elif e.fun in {'cmimid__scope_enter'}:
        scope_alt, *args = e.info

        key = to_key(METHOD_PREFIX[-1], pseudo_method_stack[-1][1], pseudo_method_stack[-1][0])

        name = "%(key)s %(alt)s" % { 'key': key, 'alt': scope_alt}


        cmimid_stack.append(('scope', scope_alt, args))
        gen_events.append(('scope_enter', scope_alt, pseudo_method_stack[-1][1], name))

    elif e.fun in {'cmimid__scope_exit'}:
        scope, scope_alt, args = cmimid_stack.pop()
        assert scope == 'scope'
        gen_events.append(('scope_exit', scope_alt))


    elif e.fun in {'cmimid__return'}:
        # For return, unwind all until the method.
        assert cmimid_stack
        while True:
            t = cmimid_stack.pop()
            if t[0] == 'method':
                method, mid = t
                gen_events.append(('method_exit', mid))
                # stop unwinding
                break
            elif t[0] == 'stack':
                stack, stack_kind, stack_id, args = t
                gen_events.append(('stack_exit', stack_kind, stack_id))
            elif t[0] == 'scope':
                scope, scope_kind, args = t
                gen_events.append(('scope_exit', scope_kind))

    elif e.fun in {'cmimid__break'}:
        # break is a little return. Unwind until the next
        # stack that is a loop.
        assert cmimid_stack
        while True:
            t = cmimid_stack.pop()
            if t[0] == 'method':
                # this should never happen.
                assert False
            elif t[0] == 'stack':
                stack, stack_kind, stack_id, args = t
                gen_events.append(('stack_exit', stack_kind, stack_id))
                if stack_kind in {CMIMID_FOR, CMIMID_WHILE, CMIMID_SWITCH}:
                    # this should not happen.
                    assert False
            elif t[0] == 'scope':
                scope, scope_kind, args = t
                gen_events.append(('scope_exit', scope_kind))

                stack, stack_kind, stack_id, args = cmimid_stack[-1]
                if stack_kind in {CMIMID_FOR, CMIMID_WHILE, CMIMID_SWITCH}:
                    # stop unwinding. The stack would get popped next.
                    break


    elif e.fun in {'cmimid__continue'}:
        # continue is a little break. Unwind until the next
        # scope that is scope of a loop.
        assert cmimid_stack
        while True:
            t = cmimid_stack.pop()
            if t[0] == 'method':
                # this should never happen.
                assert False
            elif t[0] == 'stack':
                stack, stack_kind, stack_id, args = t
                assert scope_kind not in {CMIMID_FOR, CMIMID_WHILE, CMIMID_SWITCH}
                gen_events.append(('stack_exit', stack_kind, stack_id))
            elif t[0] == 'scope':
                scope, scope_kind, args = t
                gen_events.append(('scope_exit', scope_kind))
                stack, stack_kind, stack_id, args = cmimid_stack[-1]
                if stack_kind in {CMIMID_FOR, CMIMID_WHILE, CMIMID_SWITCH}:
                    # stop unwinding
                    break
    else:
        assert False

def track_comparison(e):
    # {'type': 'INPUT_COMPARISON', 'index': [3],
    # 'length': 4, 'value': '\n',
    # 'operator': 'strcmp',
    # 'operand': ['\n'],
    # 'id': 1, 'stack': ['_real_program_main']}
    indexes = e['index']
    for i in indexes:
        # we need only the accessed indexes
        gen_events.append(('comparison', i))

def show_nested(gen_events):
    indent = 0
    for e in gen_events:
        if '_enter' in e[0]:
            print("|\t" * indent, e)
            indent += 1
        elif '_exit' in e[0]:
            indent -= 1
            print("|\t" * indent, e)

def fire_events(gen_events):
    comparisons = []
    taints.trace_init()
    method = []
    for e in gen_events:
        if 'method_enter' == e[0]:
            method.append(mimid_context.method__(name=e[2], args=[]))
            method[-1].__enter__()
        elif 'method_exit' == e[0]:
            method[-1].__exit__()
            method.pop()

        elif 'stack_enter' == e[0]:
            can_empty = '?'
            if e[1] in {'while', 'for'}:
                can_empty = '?'
            elif e[1] in {'switch', 'if'}:
                # first need to check if the switch has default an if has else
                can_empty = '-' # or =
            method.append(mimid_context.stack__(name=e[1], num=e[2], method_i=method[-1], can_empty=can_empty))
            method[-1].__enter__()
        elif 'stack_exit' == e[0]:
            method[-1].__exit__()
            method.pop()

        elif 'scope_enter' == e[0]:
            method.append(mimid_context.scope__(alt=e[1], stack_i=method[-1]))
            method[-1].__enter__()

        elif 'scope_exit' == e[0]:
            method[-1].__exit__()
            method.pop()

        elif 'comparison' == e[0]:
            idx = e[1]
            method_, stackdepth_, mid = taints.get_current_method()
            comparisons.append((idx, inputstr[idx], mid))


    j = { 'comparisons_fmt': 'idx, char, method_call_id',
          'comparisons':comparisons,
                'method_map_fmt': 'method_call_id, method_name, children',
                'method_map': taints.convert_method_map(taints.METHOD_MAP),
                'inputstr': inputstr,
                'original': sys.argv[2],
                'arg': sys.argv[2]}
    print(json.dumps([j]))


METHOD_PREFIX = None
METHOD_NAME = None
def process_events(events):
    comparisons_fmt = 'idx, char, method_call_id'
    method_map_fmt = 'method_call_id, method_name, children'
    global METHOD_PREFIX

    assert not cmimid_stack
    for e in events:
        if e['type'] == 'CMIMID_EVENT':
            track_stack(O(**e))
        elif e['type'] == 'INPUT_COMPARISON':
            track_comparison(e)
        elif e['type'] == 'STACK_EVENT':
            METHOD_PREFIX = e['stack']
    assert not cmimid_stack
    fire_events(gen_events)
    #show_nested(gen_events)

events = read_json(sys.argv[1])
with open(sys.argv[2]) as f:
    inputstr = f.read()
process_events(events)
