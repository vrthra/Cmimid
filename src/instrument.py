import sys
from clang.cindex import Index, Config, CursorKind
Config.set_library_file('/usr/lib/llvm-8/lib/libclang-8.0.0.so')

class MyAst:
    def __init__(self, node):
        self.node = node

    def __repr__(self):
        return " ".join([t.spelling for t in self.node.get_tokens()])

class IntegerLiteral(MyAst):
    def __repr__(self):
        return super().__repr__()
        #return "%s %s" % (self.node.type.spelling, self.node.spelling)

class ParmDecl(MyAst):
    def __repr__(self):
        return super().__repr__()
        assert not list(self.node.get_children())
        #return "%s %s" % (self.node.type.spelling, self.node.spelling)

class _VarDecl(MyAst):
    def __repr__(self):
        return "%s;" % super().__repr__()
        #children = "\n".join([repr(show(c)) for c in self.node.get_children()])
        #return "%s %s" % (self.node.type.spelling, children)

class DeclStmt(MyAst):
    def __repr__(self):
        return "%s" % super().__repr__()
        #return "\n".join([repr(show(c)) for c in self.node.get_children()])

class ReturnStmt(MyAst):
    def __repr__(self):
        return "%s;" % super().__repr__()
        #return "\n".join([repr(show(c)) for c in self.node.get_children()])

class WhileStmt(MyAst):
    def __repr__(self):
        children = list(self.node.get_children())
        assert(len(children) == 2)
        return "while(%s) {\n%s\n}" % (repr(show(children[0])), repr(show(children[1])))
        return "\n".join([repr(show(c)) for c in self.node.get_children()])


class CompoundStmt(MyAst):
    def __repr__(self):
        return "\n".join([repr(show(c)) for c in self.node.get_children()])


class FunctionDecl(MyAst):
    def __repr__(self):
        #body = '\n'.join([repr(show(c)) for c in self.node.get_children()])
        params = [c for c in self.node.get_children() if c.kind == CursorKind.PARM_DECL]
        stmt = [c for c in self.node.get_children() if c.kind != CursorKind.PARM_DECL]
        args = ', '.join([repr(show(c)) for c in params])
        body = '\n'.join([repr(show(c)) for c in stmt])
        buf = "%s %s(%s) {\n%s}" % (self.node.result_type.spelling, self.node.spelling, args, body)
        return buf


def show(node):
    if node.kind == CursorKind.FUNCTION_DECL:
        return FunctionDecl(node)
    #if node.kind == CursorKind.PARM_DECL:
    #    return ParmDecl(node)
    if node.kind == CursorKind.COMPOUND_STMT:
        return CompoundStmt(node)
    if node.kind == CursorKind.DECL_STMT:
        return DeclStmt(node)
    if node.kind == CursorKind.VAR_DECL:
        return VarDecl(node)
    if node.kind == CursorKind.WHILE_STMT:
        return WhileStmt(node)
    if node.kind == CursorKind.RETURN_STMT:
        return ReturnStmt(node) # need the ;
    if node.kind == CursorKind.INTEGER_LITERAL:
        return IntegerLiteral(node)
    else:
        return MyAst(node)

def parse(arg):
    idx = Index.create()
    translation_unit = idx.parse(arg)
    for i in translation_unit.cursor.get_children():
        if i.location.file.name == sys.argv[1]:
            print(repr(show(i)))


parse(sys.argv[1])
