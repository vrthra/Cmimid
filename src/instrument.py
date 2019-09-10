import sys
from clang.cindex import Index, Config, CursorKind
import os

LIBCLANG_PATH = os.environ['LIBCLANG_PATH']
Config.set_library_file(LIBCLANG_PATH)

counter = 0;
def create_cb_name():
    global counter
    counter += 1
    return "__fn%s" % counter

def compound_body_with_cb(node):
    rep = ""
    if node.kind == CursorKind.COMPOUND_STMT:
        rep = repr(to_ast(node))[2:-2]
    else:
        rep = repr(to_ast(node))
        if rep[-1] != "}" and rep[-1] != "\n" and rep[-1] != ";":
            rep += " ;"

    return "{\n%s() ;\n%s\n%s() ;\n}" % (create_cb_name(),
                                        rep,
                                        create_cb_name())

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


class ForStmt(AstNode):
    def __repr__(self):
        before_cb = create_cb_name()
        after_cb = create_cb_name()

        children = list(self.node.get_children())
        body_token_len = len(list(children[-1].get_tokens()))

        for_part_tokens = list(self.node.get_tokens())[:-body_token_len]
        for_part = " ".join([t.spelling for t in for_part_tokens])

        body = compound_body_with_cb(children[-1])
        return "%s() ;\n%s %s\n%s() ;" % (
                before_cb, for_part, body, after_cb)


class WhileStmt(AstNode):
    def __repr__(self):
        before_cb = create_cb_name()
        after_cb = create_cb_name()

        children = list(self.node.get_children())
        assert(len(children) == 2)

        cond = repr(to_ast(children[0]))
        body = compound_body_with_cb(children[1])

        return "%s();\nwhile ( %s ) %s\n%s();" % (
                before_cb, cond, body, after_cb)


class IfStmt(AstNode):
    def __init__(self, node, with_cb=True):
        super().__init__(node)
        self.with_cb = with_cb

    def __repr__(self):
        if self.with_cb:
            before_cb = create_cb_name()
            after_cb = create_cb_name()

        cond =  ""
        if_body = ""
        else_body = ""

        for i, c in enumerate(self.node.get_children()):
            if i == 0:   # if condition
                cond = "%s" % repr(to_ast(c))
            elif i == 1: # if body
                if_body = compound_body_with_cb(c)
            elif i == 2: # else body (exists if there is an else)
                if c.kind == CursorKind.IF_STMT:
                    # else if -> no before/after if callbacks
                    else_body = "%s" % repr(IfStmt(c, with_cb=False))
                else:
                    else_body = compound_body_with_cb(c)

        block = "if ( %s ) %s" % (cond, if_body)
        if else_body != "":
            block += " else %s" % else_body

        if self.with_cb:
            return "%s() ;\n%s\n%s() ;" % (before_cb, block, after_cb)

        return block


class SwitchStmt(AstNode):
    def __repr__(self):
        before_cb = create_cb_name()
        after_cb = create_cb_name()

        children = list(self.node.get_children())
        assert(len(children) == 2)

        body_tokens_len = len(list(children[1].get_tokens()))

        switch_part_tokens = list(self.node.get_tokens())[:-body_tokens_len]
        switch_part = " ".join([t.spelling for t in switch_part_tokens])

        assert(children[1].kind == CursorKind.COMPOUND_STMT)
        body_compound_stmt = CompoundStmt(children[1])
        check_cases_have_break(body_compound_stmt)
        body = repr(body_compound_stmt)

        return "%s() ;\n%s %s\n%s();" % (
                before_cb, switch_part, body, after_cb)

class CaseStmt(AstNode):
    def __repr__(self):
        return super().__repr__()


class DefaultStmt(AstNode):
    def __repr__(self):
        return super().__repr__()


class CompoundStmt(AstNode):
    def __repr__(self):
        case_seen = False
        case_entry_cb = None
        case_exit_cb = None

        stmts = [];
        for c in self.node.get_children():
            is_case_stmt = (c.kind == CursorKind.CASE_STMT
                            or c.kind == CursorKind.DEFAULT_STMT)
            is_break_stmt = c.kind == CursorKind.BREAK_STMT

            if is_case_stmt:
                # create cb names beforehand for number ordering
                assert(case_seen == False)
                case_seen = True
                case_entry_cb = create_cb_name() + "() ;\n"
                case_exit_cb = create_cb_name() + "() ;\n"

            rep = repr(to_ast(c))

            # handle missing semicolons
            if rep[-1] != "}" and rep[-1] != "\n" and rep[-1] != ";":
                rep += " ;"

            if is_case_stmt:
                words = rep.split(":")
                case_part = words[0]
                others = ":".join(words[1:])
                others = others[1:] if others[0] == " " else others
                case_part += ":\n" + case_entry_cb
                rep = case_part + others
            elif is_break_stmt and case_seen:
                case_seen = False
                rep = case_exit_cb + rep

            stmts.append(rep)

        body = "\n".join(stmts)
        return "{\n%s\n}" % body


class FunctionDecl(AstNode):
    def __repr__(self):
        children = list(self.node.get_children())
        return_type = self.node.result_type.spelling
        function_name = self.node.spelling
        params = ", ".join([repr(to_ast(c)) for c in children[:-1]])
        body = repr(to_ast(children[-1]))
        return "%s %s(%s) %s" % (return_type, function_name, params, body)


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

    else:
        return AstNode(node)

def parse(arg, fn):
    with open(fn, 'w+') as f:
        idx = Index.create()
        translation_unit = idx.parse(arg)
        for i in translation_unit.cursor.get_children():
            if i.location.file.name == sys.argv[1]:
                print(repr(to_ast(i)), file=f)


parse(sys.argv[1], sys.argv[2])
