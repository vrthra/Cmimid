import sys
from clang.cindex import Index, Config, CursorKind, TokenKind
import os

# Notes: We need both STACK_EXIT and SCOPE_EXIT because
# Break causes both STACK and SCOPE exits while
# Continue only causes SCOPE exit.


LIBCLANG_PATH = os.environ['LIBCLANG_PATH']
Config.set_library_file(LIBCLANG_PATH)

counter = 0;
def get_id():
    global counter
    counter += 1
    return str(counter)

def compound_body_with_cb(node, c):
    #assert node.extent.start.line != node.extent.end.line
    c1 = get_id()
    rep = ""
    if node.kind == CursorKind.COMPOUND_STMT:
        src = to_src(node)
        assert (src[0], src[-1]) == ('{', '}')
        assert (src[1], src[-2]) == ('\n', '\n')
        rep = src[2:-2]
    else:
        rep = to_src(node)
        if rep[-1] != "}" and rep[-1] != "\n" and rep[-1] != ";":
            rep += " ;"

    return '''\
{
cmimid__scope_enter(%s);
%s
cmimid__scope_exit(CMIMID_EXIT);
}''' % (c1, rep)


class AstNode:
    def __init__(self, node):
        self.node = node


    def check_children_not_macro(self):
        for c in self.node.get_children():
            if c.extent.start.line != c.extent.end.line:
                continue
            if c.extent.start.column != c.extent.end.column:
                continue
            raise Exception("We do not know how to handle macros in code")

    def to_src(self):
        src = ''.join(SRC[self.node.extent.begin_int_data-2:self.node.extent.end_int_data-2])
        if self.node.extent.start.line == self.node.extent.end.line:
            lines = [SRC[self.node.extent.start.line-1]]
        else:
            lines = SRC[self.node.extent.start.line-1: self.node.extent.end.line-1]
        lines[0] = lines[0][self.node.extent.start.column-1:self.node.extent.end.column-1]
        return ''.join(lines)

    def __repr__(self):
        return " ".join([t.spelling for t in self.node.get_tokens()])

class SpellingNode(AstNode):
    def __repr__(self): return self.node.spelling

class SrcNode(AstNode):
    def __repr__(self):
        s = self.to_src()
        if not s: bp()
        return s

class StmtNode(AstNode):
    def __repr__(self): return "%s;" % super().__repr__()



class EnumDecl(StmtNode): pass
class TypedefDecl(StmtNode): pass
class IntegerLiteral(AstNode): pass
class StructDecl(StmtNode): pass
class VarDecl(StmtNode): pass
class UnionDecl(StmtNode): pass
class TypeDecl(StmtNode): pass

class CallExpr(SrcNode): pass
class ParenExpr(SrcNode): pass
class UnexposedExpr(SrcNode): pass
class DeclRefExpr(SrcNode): pass

class ParmDecl(AstNode): pass
class DeclStmt(AstNode): pass
class CompoundAssignmentOperator(AstNode): pass
class TypeRef(AstNode): pass
class UnaryOperator(AstNode): pass
class BinaryOperator(AstNode): pass
class CaseStmt(AstNode): pass
class DefaultStmt(AstNode): pass

class ReturnStmt(AstNode):
    def __repr__(self):
        return '''\
cmimid__return(CMIMID_RETURN);
%s ;''' % super().__repr__()

class BreakStmt(AstNode):
    def __repr__(self):
        return '''\
cmimid__break(CMIMID_BREAK);
%s ;''' % super().__repr__()

class ContinueStmt(AstNode):
    def __repr__(self):
        return '''\
cmimid__continue(CMIMID_CONTINUE);
%s ;''' % super().__repr__()

def extent(node):
    return range(node.extent.start.line,node.extent.end.line+1)

class ForStmt(AstNode):
    def __repr__(self):
        outer_range = extent(self.node)
        tokens = [t for t in self.node.get_tokens()]
        children = list(self.node.get_children()) # this is OK because of assert
        # now, ensure that all children are within the range.
        self.check_children_not_macro()

        body_child = children[-1]
        #assert body_child.kind is CursorKind.COMPOUND_STMT

        for_part_tokens = [t for t in tokens if
                t.extent.end_int_data <= body_child.extent.begin_int_data]
        for_part = ' '.join([t.spelling for t in for_part_tokens])

        c = get_id()
        body = compound_body_with_cb(children[-1], c)
        return '''\
cmimid__stack_enter(CMIMID_FOR, %s);
%s %s
cmimid__stack_exit(CMIMID_EXIT);''' % (c, for_part, body)


class WhileStmt(AstNode):
    def __repr__(self):
        outer_range = extent(self.node)
        children = list(self.node.get_children())
        self.check_children_not_macro()
        assert(len(children) == 2)

        cond = to_src(children[0])
        c = get_id()
        body = compound_body_with_cb(children[1], c)

        return '''\
cmimid__stack_enter(CMIMID_WHILE, %s);
while (%s) %s
cmimid__stack_exit(CMIMID_EXIT);''' % (c, cond, body)


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
                cond = "%s" % to_src(cmp)
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
cmimid__stack_enter(CMIMID_IF, %s);
%s
cmimid__stack_exit(CMIMID_EXIT);''' % (c, block)

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
        body = to_src(children[1])

        return '''\
cmimid__stack_enter(CMIMID_SWITCH, %s);
%s %s
cmimid__stack_exit(CMIMID_EXIT)''' % (c, switch_part, body)


class CompoundStmt(AstNode):
    def __repr__(self):
        outer_range = extent(self.node)
        stmts = []
        children = self.node.get_children()
        self.check_children_not_macro()
        label = None
        seen_default = False
        for child in children:
            rep = to_src(child)
            if child.kind == CursorKind.CASE_STMT:
                assert not seen_default
                literal, *_ = child.get_children()
                label = int(to_src(literal))
                assert rep.startswith('case')
                colon = rep.find(':')
                init = rep[:colon]
                rest = rep[colon+1:]
                rep = '''\
%s:
cmimid__scope_enter(%d);
%s
''' % (init, label, rest)

            elif child.kind == CursorKind.DEFAULT_STMT:
                seen_default = True
                label += 1 # We dont expect any more after default
                assert rep.startswith('default')
                colon = rep.find(':')
                init = rep[:colon]
                rest = rep[colon+1:]
                rep = '''\
%s:
cmimid__scope_enter(%d);
%s
''' % (init, label, rest)
            if not rep:
               print(child.kind, child.extent, file=sys.stderr)
               continue

            # handle missing semicolons
            if rep.strip()[-1] not in {'}', ';'}:
                rep += ";"

            stmts.append(rep)

        body = "\n".join(stmts)
        return '''\
{
%s
}''' % body


class FunctionDecl(AstNode):
    # method context wrapper
    def __repr__(self):
        children = list(self.node.get_children())
        return_type = self.node.result_type.spelling
        function_name = self.node.spelling
        c = get_id()
        cparams = [p for p in children if p.kind == CursorKind.PARM_DECL]
        params = ", ".join([to_src(c) for c in cparams])
        if children[-1].kind == CursorKind.COMPOUND_STMT:
            body = to_src(children[-1])
            return '''\
%s
%s(%s) {
cmimid__method_enter(%s);
%s
cmimid__method_exit();
}''' % (return_type, function_name, params, c, body)
        else:
            # function declaration.
            return '''\
%s
%s(%s);''' % (return_type, function_name, params)

import pudb
bp = pudb.set_trace

FN_HASH = {
        CursorKind.FUNCTION_DECL: FunctionDecl,
        CursorKind.PARM_DECL: ParmDecl,
        CursorKind.VAR_DECL: VarDecl,
        CursorKind.TYPEDEF_DECL: TypedefDecl,
        CursorKind.INTEGER_LITERAL: IntegerLiteral,
        CursorKind.COMPOUND_STMT: CompoundStmt,
        CursorKind.DECL_STMT: DeclStmt,
        CursorKind.DECL_REF_EXPR: DeclRefExpr,
        CursorKind.IF_STMT: IfStmt,
        CursorKind.SWITCH_STMT: SwitchStmt,
        CursorKind.CASE_STMT: CaseStmt,
        CursorKind.DEFAULT_STMT: DefaultStmt,
        CursorKind.FOR_STMT: ForStmt,
        CursorKind.WHILE_STMT: WhileStmt,
        CursorKind.RETURN_STMT: ReturnStmt,
        CursorKind.BREAK_STMT: BreakStmt,
        CursorKind.CONTINUE_STMT: ContinueStmt,
        CursorKind.STRUCT_DECL: StructDecl,
        CursorKind.TYPEDEF_DECL: TypeDecl,
        CursorKind.ENUM_DECL: EnumDecl,
        CursorKind.UNION_DECL: UnionDecl,
        CursorKind.CALL_EXPR: CallExpr,
        CursorKind.BINARY_OPERATOR: BinaryOperator,
        CursorKind.UNARY_OPERATOR: UnaryOperator,
        CursorKind.TYPE_REF: TypeRef,
        CursorKind.PAREN_EXPR: ParenExpr,
        CursorKind.COMPOUND_ASSIGNMENT_OPERATOR: CompoundAssignmentOperator,
        CursorKind.UNEXPOSED_EXPR: UnexposedExpr,
        }


def to_ast(node):
    if node.kind in FN_HASH:
        return FN_HASH[node.kind](node)
    else:
        print(node.kind, file=sys.stderr)
        return AstNode(node)

def to_src(node):
    v = to_ast(node)
    return repr(v)

# DEBUG
def traverse(node, level):
    print('%s %-35s %-20s %-10s [%-6s:%s - %-6s:%s] %s %s ' % (' ' * level,
    node.kind, node.spelling, node.type.spelling, node.extent.start.line, node.extent.start.column,
    node.extent.end.line, node.extent.end.column, node.location.file, node.mangled_name))
    if node.kind == clang.cindex.CursorKind.CALL_EXPR:
        for arg in node.get_arguments():
            print("ARG=%s %s" % (arg.kind, arg.spelling))

    for child in node.get_children():
        traverse(child, level+1)
# DEBUG

skipped = []
parsed_extent = []
SRC = []

def store(arg):
    with open(arg) as f:
        SRC.extend(f.readlines())

displayed_till = 0
def display_till(last):
    for i in range(displayed_till, last):
        print(SRC[i], end='')

def parse(arg):
    global displayed_till
    idx = Index.create()
    translation_unit = idx.parse(arg)
    print('''\
#define CMIMID_EXIT 0
#define CMIMID_BREAK 1
#define CMIMID_CONTINUE 2
#define CMIMID_FOR 3
#define CMIMID_WHILE 4
#define CMIMID_IF 5
#define CMIMID_SWITCH 6
#define CMIMID_RETURN 7

void cmimid__method_enter(int i) {}
void cmimid__method_exit() {}
void cmimid__stack_enter(int i, int j) {}
void cmimid__stack_exit(int i) {}
void cmimid__scope_enter(int i) {}
void cmimid__scope_exit(int i) {}
void cmimid__break(int i) {}
void cmimid__continue(int i) {}
void cmimid__return(int i) {}
''')
    for i in translation_unit.cursor.get_children():
        if i.location.file.name == sys.argv[1]:
            display_till(i.location.line-1)
            print(to_src(i), file=sys.stdout)
            displayed_till = i.extent.end.line
        else:
            pass
           #skipped.append(to_src(i))
    display_till(len(SRC))

store(sys.argv[1])
parse(sys.argv[1])
#for i in skipped:
#    print(repr(i), file=sys.stderr)
