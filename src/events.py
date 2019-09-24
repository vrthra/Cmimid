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
        _mid, *args = e.info
        mname = METHOD_PREFIX[-1]
        cmimid_stack.append(('method', mname))
        gen_events.append(('method_enter', mname))

    elif e.fun in {'cmimid__method_exit'}:
        method, mname = cmimid_stack.pop()
        assert method == 'method'
        gen_events.append(('method_exit', mname))

    elif e.fun in {'cmimid__stack_enter'}:
        stack_kind, stack_id, *_args = e.info
        str_skind = kind_map[stack_kind]

        key = to_key(METHOD_PREFIX[-1], str_skind, stack_id)

        pseudo_method_stack.append(key)
        cmimid_stack.append(('stack', stack_id, str_skind))
        gen_events.append(('stack_enter', str_skind, stack_id))

    elif e.fun in {'cmimid__stack_exit'}:
        stack, stack_id, str_skind = cmimid_stack.pop()
        assert stack == 'stack'
        gen_events.append(('stack_exit', stack_id))
        pseudo_method_stack.pop()

    elif e.fun in {'cmimid__scope_enter'}:
        scope_alt, is_default_or_else, *args = e.info
        cmimid_stack.append(('scope', scope_alt, args))
        gen_events.append(('scope_enter', scope_alt))

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
                gen_events.append(('stack_exit', stack_id))
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
                stack, stack_id, str_skind = t
                gen_events.append(('stack_exit', stack_id))
                if str_skind in {'for', 'while', 'switch'}:
                    # this should not happen.
                    assert False
            elif t[0] == 'scope':
                scope, scope_kind, args = t
                gen_events.append(('scope_exit', scope_kind))
                stack, stack_id, str_skind = cmimid_stack[-1]
                if str_skind in {'for', 'while', 'switch'}:
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
                stack, stack_id, str_skind = t
                # we should exit before the first _loop_ or _switch_
                # which is the parent for _continue_
                assert str_skind not in {'for', 'while', 'switch'}
                gen_events.append(('stack_exit', stack_id))
            elif t[0] == 'scope':
                scope, scope_kind, args = t
                gen_events.append(('scope_exit', scope_kind))
                stack, stack_id, skind = cmimid_stack[-1]
                if stack_kind in {'for', 'while', 'switch'}:
                    # stop unwinding
                    break
    else:
        # TODO need goto
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
            _, mname = e
            method.append(mimid_context.method__(name=mname, args=[]))
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
                can_empty = '-' # or = TODO
            _, name, num = e
            method.append(mimid_context.stack__(name=name, num=num, method_i=method[-1], can_empty=can_empty))
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
def process_events(events):
    global METHOD_PREFIX

    assert not cmimid_stack
    for e in events:
        if e['type'] == 'CMIMID_EVENT':
            track_stack(O(**e))
        elif e['type'] == 'INPUT_COMPARISON':
            track_comparison(e)
        elif e['type'] == 'STACK_EVENT':
            # this only gets us the top level methods
            # i.e no pseudo methods.
            METHOD_PREFIX = e['stack']
    assert not cmimid_stack
    fire_events(gen_events)

events = read_json(sys.argv[1])
with open(sys.argv[2]) as f: inputstr = f.read()
process_events(events)
