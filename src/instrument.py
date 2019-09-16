import sys
from clang.cindex import Index, Config, CursorKind
import os

LIBCLANG_PATH = os.environ['LIBCLANG_PATH']
Config.set_library_file(LIBCLANG_PATH)

counter = 0;
def create_cb_name():
    global counter
    counter += 1
    return "__fn%s()" % counter

def get_id():
    global counter
    counter += 1
    return str(counter)

def compound_body_with_cb(node, c):
    c1 = get_id()
    rep = ""
    if node.kind == CursorKind.COMPOUND_STMT:
        rep = repr(to_ast(node))[2:-2]
    else:
        rep = repr(to_ast(node))
        if rep[-1] != "}" and rep[-1] != "\n" and rep[-1] != ";":
            rep += " ;"

    return '''\
{
scope__enter(%s);
%s
scope__exit("exit");
}''' % (c1, rep)

def check_cases_have_break(compound_stmt):
    node = compound_stmt.node
    case_indexes = [i for i, c in enumerate(node.get_children())
                    if c.kind == CursorKind.CASE_STMT or
                       c.kind == CursorKind.DEFAULT_STMT]
    break_indexes = [i for i, c in enumerate(node.get_children())
                     if c.kind == CursorKind.BREAK_STMT]

    if len(break_indexes) < len(case_indexes):
        raise Exception("case or default stmt does not have break")

    for i, ci in enumerate(case_indexes):
        ci_next = (break_indexes[-1] + 1 if i == len(case_indexes) - 1
                   else case_indexes[i+1])
        has_break = any(bi > ci and bi < ci_next for bi in break_indexes)
        if not has_break:
            raise Exception("case or default stmt does not have break")


class AstNode:
    def __init__(self, node):
        self.node = node

    def __repr__(self):
        return " ".join([t.spelling for t in self.node.get_tokens()])


class IntegerLiteral(AstNode):
    def __repr__(self):
        return super().__repr__()
        #return "%s %s" % (self.node.type.spelling, self.node.spelling)


class ParmDecl(AstNode):
    def __repr__(self):
        return super().__repr__()
        assert not list(self.node.get_children())
        #return "%s %s" % (self.node.type.spelling, self.node.spelling)


class VarDecl(AstNode):
    def __repr__(self):
        return super().__repr__()


class DeclStmt(AstNode):
    def __repr__(self):
        return super().__repr__()
        #return "\n".join([repr(to_ast(c)) for c in self.node.get_children()])


class ReturnStmt(AstNode):
    def __repr__(self):
        return "%s ;" % super().__repr__()
        #return "\n".join([repr(to_ast(c)) for c in self.node.get_children()])

class BreakStmt(AstNode):
    def __repr__(self):
        return '''\
scope__exit("break");
stack__exit("break");
%s ;''' % super().__repr__()

class ContinueStmt(AstNode):
    def __repr__(self):
        return '''\
scope__exit("continue");
%s ;''' % super().__repr__()


class ForStmt(AstNode):
    def __repr__(self):
        children = list(self.node.get_children())
        body_token_len = len(list(children[-1].get_tokens()))

        for_part_tokens = list(self.node.get_tokens())[:-body_token_len]
        for_part = " ".join([t.spelling for t in for_part_tokens])

        c = get_id()
        body = compound_body_with_cb(children[-1], c)
        return '''\
stack__enter("for", %s);
%s %s
stack__exit("exit");''' % (c, for_part, body)


class WhileStmt(AstNode):
    def __repr__(self):
        children = list(self.node.get_children())
        assert(len(children) == 2)

        cond = repr(to_ast(children[0]))
        c = get_id()
        body = compound_body_with_cb(children[1], c)

        return '''\
stack__enter("while", %s);
while (%s) %s
stack__exit("exit");''' % (c, cond, body)


class IfStmt(AstNode):
    def __init__(self, node, with_cb=True):
        super().__init__(node)
        self.with_cb = with_cb

    def __repr__(self):
        c = get_id()
        cond =  ""
        if_body = ""
        else_body = ""

        for i, cmp in enumerate(self.node.get_children()):
            if i == 0:   # if condition
                cond = "%s" % repr(to_ast(cmp))
            elif i == 1: # if body
                if_body = compound_body_with_cb(cmp, c)
            elif i == 2: # else body (exists if there is an else)
                if cmp.kind == CursorKind.IF_STMT:
                    # else if -> no before/after if callbacks
                    else_body = "%s" % repr(IfStmt(cmp, with_cb=False))
                else:
                    else_body = compound_body_with_cb(cmp, c)

        block = "if ( %s ) %s" % (cond, if_body)
        if else_body != "":
            block += " else %s" % else_body

        if self.with_cb:
            return '''\
stack__enter("if", %s);
%s
stack__exit("exit");''' % (c, block)

        return block


class SwitchStmt(AstNode):
    def __repr__(self):
        c = get_id()
        children = list(self.node.get_children())
        assert(len(children) == 2)

        body_tokens_len = len(list(children[1].get_tokens()))

        switch_part_tokens = list(self.node.get_tokens())[:-body_tokens_len]
        switch_part = " ".join([t.spelling for t in switch_part_tokens])

        assert(children[1].kind == CursorKind.COMPOUND_STMT)
        body_compound_stmt = CompoundStmt(children[1])
        check_cases_have_break(body_compound_stmt)
        body = repr(body_compound_stmt)

        return '''\
stack__enter("switch", %s);
%s %s
stack__exit("exit")''' % (c, switch_part, body)

class CaseStmt(AstNode):
    def __repr__(self):
        return super().__repr__()


class DefaultStmt(AstNode):
    def __repr__(self):
        return super().__repr__()


class CompoundStmt(AstNode):
    def __repr__(self):
        case_seen = False
        #case_entry_cb = None
        #case_exit_cb = None
        c = get_id()

        stmts = [];
        for c in self.node.get_children():
            is_case_stmt = (c.kind == CursorKind.CASE_STMT
                            or c.kind == CursorKind.DEFAULT_STMT)
            is_break_stmt = c.kind == CursorKind.BREAK_STMT

            if is_case_stmt:
                # create cb names beforehand for number ordering
                assert(case_seen == False)
                case_seen = True
                #case_entry_cb = create_cb_name() + "() ;\n"
                #case_exit_cb = create_cb_name() + "() ;\n"

            rep = repr(to_ast(c))

            # handle missing semicolons
            if rep[-1] != "}" and rep[-1] != "\n" and rep[-1] != ";":
                rep += " ;"

            if is_case_stmt:
                words = rep.split(":")
                case_part = words[0]
                others = ":".join(words[1:])
                others = others[1:] if others[0] == " " else others
                case_part += ":\n" #+ case_entry_cb
                rep = case_part + others
            elif is_break_stmt and case_seen:
                case_seen = False
                #rep = case_exit_cb + rep

            stmts.append(rep)

        body = "\n".join(stmts)
        return '''\
{
%s
}''' % body


class StructDecl(AstNode):
    def __repr__(self):
        return '/*StructDecl (%s) */' % super().__repr__()

class EnumDecl(AstNode):
    pass

class TypeDecl(AstNode):
    pass

class UnionDecl(AstNode):
    pass

class CallExpr(AstNode):
    pass

class ParenExpr(AstNode):
    pass

class CompoundAssignmentOperator(AstNode):
    pass


class TypeRef(AstNode):
    pass

class UnaryOperator(AstNode):
    pass

class BinaryOperator(AstNode):
    pass



class FunctionDecl(AstNode):
    # method context wrapper
    def __repr__(self):
        children = list(self.node.get_children())
        return_type = self.node.result_type.spelling
        function_name = self.node.spelling
        c = get_id()
        params = ", ".join([repr(to_ast(c)) for c in children[:-1]])
        if children:
            body = repr(to_ast(children[-1]))
        else:
            body = ''
        return  '''\
%s
%s(%s) {
method__enter(%s);
%s
method__exit();
}''' % (return_type, function_name, params, c, body)


def to_ast(node):
    #print(node.kind, repr(AstNode(node)))

    # declarations
    if node.kind == CursorKind.FUNCTION_DECL:
        return FunctionDecl(node)
    elif node.kind == CursorKind.PARM_DECL:
        return ParmDecl(node)
    elif node.kind == CursorKind.VAR_DECL:
        return VarDecl(node)

    # literals
    elif node.kind == CursorKind.INTEGER_LITERAL:
        return IntegerLiteral(node)

    # statements
    elif node.kind == CursorKind.COMPOUND_STMT:
        return CompoundStmt(node)
    elif node.kind == CursorKind.DECL_STMT:
        return DeclStmt(node)
    elif node.kind == CursorKind.IF_STMT:
        return IfStmt(node)
    elif node.kind == CursorKind.SWITCH_STMT:
        return SwitchStmt(node)
    elif node.kind == CursorKind.CASE_STMT:
        return CaseStmt(node)
    elif node.kind == CursorKind.DEFAULT_STMT:
        return DefaultStmt(node)
    elif node.kind == CursorKind.FOR_STMT:
        return ForStmt(node)
    elif node.kind == CursorKind.WHILE_STMT:
        return WhileStmt(node)
    elif node.kind == CursorKind.RETURN_STMT:
        return ReturnStmt(node) # need the ;
    elif node.kind == CursorKind.BREAK_STMT:
        return BreakStmt(node)
    elif node.kind == CursorKind.CONTINUE_STMT:
        return ConinuetStmt(node)
    elif node.kind == CursorKind.STRUCT_DECL:
        return StructDecl(node)
    elif node.kind == CursorKind.TYPEDEF_DECL:
        return TypeDecl(node)
    elif node.kind == CursorKind.ENUM_DECL:
        return EnumDecl(node)
    elif node.kind == CursorKind.UNION_DECL:
        return UnionDecl(node)
    elif node.kind == CursorKind.CALL_EXPR:
        return CallExpr(node)
    elif node.kind == CursorKind.BINARY_OPERATOR:
        return BinaryOperator(node)
    elif node.kind == CursorKind.UNARY_OPERATOR:
        return UnaryOperator(node)
    elif node.kind == CursorKind.TYPE_REF:
        return TypeRef(node)
    elif node.kind == CursorKind.PAREN_EXPR:
        return ParenExpr(node)
    elif node.kind == CursorKind.COMPOUND_ASSIGNMENT_OPERATOR:
        return CompoundAssignmentOperator(node)
    else:
        print(node.kind, file=sys.stderr)
        return AstNode(node)

skipped = []

def parse(arg):
    idx = Index.create()
    translation_unit = idx.parse(arg)
    #assert translation_unit.cursor.displayname == 'calc_parse.c'
    for i in translation_unit.cursor.get_children():
        if i.location.file.name == sys.argv[1]:
            print(i.location.line,  i.extent.end.line, file=sys.stderr)
            print(repr(to_ast(i)), file=sys.stdout)
        else:
           skipped.append(to_ast(i))

parse(sys.argv[1])
#for i in skipped:
#    print(repr(i), file=sys.stderr)
