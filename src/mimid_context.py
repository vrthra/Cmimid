import taints
import urllib.parse

class method__:
    def __init__(self, name, args):
        self.args = '_'.join([urllib.parse.quote(i) for i in args if type(i) == str])
        if not self.args:
            self.name = name
        else:
            self.name = "%s__%s" % (name, self.args) # <- not for now #TODO
        if args and hasattr(args[0], 'tag'):
            self.name = "%s:%s" % (args[0].tag, self.name)
        taints.trace_call(self.name)

    def __enter__(self):
        taints.trace_set_method(self.name)
        self.stack = []
        return self

    def __exit__(self, *args):
        taints.trace_return()
        taints.trace_set_method(self.name)

class stack__:
    def __init__(self, name, num, method_i, can_empty):
        self.stack = method_i.stack
        self.can_empty = can_empty # * means yes. + means no, ? means to be determined
        self.name, self.num, self.method = name, num, method_i.name

    def __enter__(self):
        if self.name in {'while', 'for'}:
            self.stack.append(0)
        elif self.name in {'if', 'switch'}:
            self.stack.append(-1)
        else:
            assert False
        return self

    def __exit__(self, *args):
        self.stack.pop()

import json
class scope__:
    def __init__(self, alt, stack_i):
        self.name, self.num, self.method, self.alt = stack_i.name, stack_i.num, stack_i.method, alt
        self.stack = stack_i.stack
        self.can_empty = stack_i.can_empty

    def __enter__(self):
        if self.name in {'while', 'for'}:
            self.stack[-1] += 1
        elif self.name in {'if', 'switch'}:
            pass
        else:
            assert False, self.name
        uid = json.dumps(self.stack)
        if self.name in {'while'}:
            taints.trace_call('%s:%s_%s %s %s' % (self.method, self.name, self.num, self.can_empty, uid))
        else:
            taints.trace_call('%s:%s_%s %s %s#%s' % (self.method, self.name, self.num, self.can_empty, self.alt, uid))
        taints.trace_set_method(self.name)
        return self

    def __exit__(self, *args):
        taints.trace_return()
        taints.trace_set_method(self.name)
