import sys
from clang.cindex import Index, Config, CursorKind, TokenKind
import os

# Notes: We need both STACK_EXIT and SCOPE_EXIT because
# Break causes both STACK and SCOPE exits while
# Continue only causes SCOPE exit.

# Notes: We need to sanitize C because our parser is
# essentially held together with duct tape
# 1. No macros -- translate all macros to expansions
# 2. No enums -- enums with case statements fail
# 3. Absolutely no C++ comments
# 4. Put braces around individual case statements in a switch
# 5. Braces initialization of struct members does not work as expected in many places
# 6. Defaults in switch should not contain breaks.
# 7. Really no macros, even for things like isdigit or strncmp
# 8. No labels. If labels are necessary, remember to ensure processing on statement.

LIBCLANG_PATH = os.environ['LIBCLANG_PATH']
Config.set_library_file(LIBCLANG_PATH)

CURRENT_STACK = [(0, '0')]

counter = 0;
def compound_body_with_cb(node):
    #assert node.extent.start.line != node.extent.end.line
    rep = ""
    if node.kind == CursorKind.COMPOUND_STMT:
        src = to_string(node)
        assert (src[0], src[-1]) == ('{', '}')
        assert (src[1], src[-2]) == ('\n', '\n')
        rep = src[2:-2]
    else:
        rep = to_string(node)
        if rep[-1] != "}" and rep[-1] != "\n" and rep[-1] != ";":
            rep += " ;"

    return '''\
{
%s
}''' % (rep)


class AstNode:
    def __init__(self, node):
        self.node = node


    def check_children_not_macro(self):
        for c in self.node.get_children():
            if c.extent.start.line != c.extent.end.line:
                continue
            if c.extent.start.column != c.extent.end.column:
                continue
            if c.kind == CursorKind.CASE_STMT: # I have no idea why I dont get this.
                continue
            raise Exception("We do not know how to handle macros in code")

    def to_src(self):
        src = ''.join(SRC[self.node.extent.begin_int_data-2:self.node.extent.end_int_data-2])
        if self.node.extent.start.line == self.node.extent.end.line:
            lines = [SRC[self.node.extent.start.line-1]]
            lines[0] = lines[0][self.node.extent.start.column-1:self.node.extent.end.column-1]
        else:
            lines = SRC[self.node.extent.start.line-1: self.node.extent.end.line] # last line not included
            lines[0] = lines[0][self.node.extent.start.column-1:]
            lines[-1] = lines[-1][:self.node.extent.end.column-1]
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
class CharacterLiteral(SrcNode): pass
class CXXUnaryExpr(SrcNode): pass
class CStyleCastexpr(SrcNode): pass

class ParmDecl(AstNode): pass
class DeclStmt(AstNode): pass
class CompoundAssignmentOperator(AstNode): pass
class TypeRef(AstNode): pass
class UnaryOperator(AstNode): pass
class BinaryOperator(AstNode): pass
class DefaultStmt(AstNode): pass
class NullStmt(AstNode):
    def __repr__(self):
        return ';'

class ReturnStmt(StmtNode): pass
class BreakStmt(StmtNode): pass
class ContinueStmt(StmtNode): pass

class GotoStmt(AstNode):
    def __repr__(self): assert False
class LabelStmt(AstNode):
    def __repr__(self): assert False


def extent(node):
    return range(node.extent.start.line,node.extent.end.line+1)


class CaseStmt(AstNode):
    def __repr__(self):
       children = list(self.node.get_children())
       if len(children) == 2:
           label = to_string(children[0])
           body = compound_body_with_cb(children[1])
           return '''case %s: %s;''' % (label, body)
       src = []
       for child in children:
           c = to_string(child)
           src.append(c)
       return '\n'.join(src)


class ForStmt(AstNode):
    def __repr__(self):
        outer_range = extent(self.node)
        tokens = [t for t in self.node.get_tokens()]
        lst = list(self.node.get_children()) # this is OK because of assert
        decl, cond, incr, body = None, None, None, None
        assert len(lst) != 0
        body = lst[-1]
        lst.pop()
        assert len(lst) <= 3
        if lst:
            semicolons = [t for t in self.node.get_tokens() if t.spelling == ';']
            pos1, pos2, *_ = [t for t in semicolons]
            if lst[-1].extent.begin_int_data > pos2.extent.begin_int_data:
                incr = lst.pop()
            else:
                incr = None
            assert len(lst) <= 2

            if lst:
                if lst[-1].extent.begin_int_data > pos1.extent.begin_int_data:
                    cond = lst.pop()
                else:
                    cond = None
                assert len(lst) <= 1

                if lst:
                    decl = lst[-1]

        # now, ensure that all children are within the range.
        self.check_children_not_macro()

        assert len(CURRENT_STACK) > 0
        CURRENT_STACK.append(('CMIMID_FOR'))
        sdecl = to_string(decl) if decl is not None else ''
        scond = to_string(cond) if cond is not None else ''
        sincr = to_string(incr) if incr is not None else ''
        sbody = compound_body_with_cb(body) if body is not None else ''
        CURRENT_STACK.pop()
        assert len(CURRENT_STACK) > 0
        return '''\
{
%s /*decl*/
while(1) {
__cmimid__res = (%s); /*cond*/
if (!__cmimid__res) break;
%s
%s; /*incr*/
}
}
''' % (sdecl,scond, sbody, sincr)


class DoStmt(AstNode):
    def __repr__(self):
        outer_range = extent(self.node)
        children = list(self.node.get_children())
        self.check_children_not_macro()
        assert(len(children) == 2)

        cond = to_string(children[1])
        assert len(CURRENT_STACK) > 0
        CURRENT_STACK.append(('CMIMID_WHILE'))
        body = compound_body_with_cb(children[0])
        CURRENT_STACK.pop()
        assert len(CURRENT_STACK) > 0

        return '''\
while (1) {
%s
__cmimid__res = (%s);
if (!__cmimid__res) break;
}
''' % (body, cond)


class WhileStmt(AstNode):
    def __repr__(self):
        outer_range = extent(self.node)
        children = list(self.node.get_children())
        self.check_children_not_macro()
        assert(len(children) == 2)

        cond = to_string(children[0])
        assert len(CURRENT_STACK) > 0
        CURRENT_STACK.append(('CMIMID_WHILE'))
        body = compound_body_with_cb(children[1])
        CURRENT_STACK.pop()
        assert len(CURRENT_STACK) > 0

        return '''\
while (1) {
__cmimid__res = (%s);
if (!__cmimid__res) break;
%s
}
''' % (cond, body)


class IfStmt(AstNode):
    def __init__(self, node, with_cb=True):
        super().__init__(node)
        self.with_cb = with_cb

    def __repr__(self):
        cond =  ""
        if_body = ""
        else_body = ""

        for i, child in enumerate(self.node.get_children()):
            if i == 0:   # if condition
                cond = "%s" % to_string(child)
            elif i == 1: # if body
                assert len(CURRENT_STACK) > 0
                CURRENT_STACK.append(('CMIMID_IF'))
                if_body = compound_body_with_cb(child)
                CURRENT_STACK.pop()
                assert len(CURRENT_STACK) > 0
            elif i == 2: # else body (exists if there is an else)
                if child.kind == CursorKind.IF_STMT:
                    # else if -> no before/after if callbacks
                    else_body = "%s" % repr(IfStmt(child, with_cb=False))
                else:
                    CURRENT_STACK.append(('CMIMID_IF'))
                    else_body = compound_body_with_cb(child)
                    CURRENT_STACK.pop()
                    assert len(CURRENT_STACK) > 0

        block = '''
__cmimid__res = (%s);
if ( __cmimid__res )
%s
''' % (cond, if_body)
        if else_body != "":
            block += " else {%s}" % else_body

        if self.with_cb:
            return '''\
%s
''' % (block)

        return block


class SwitchStmt(AstNode):
    def __repr__(self):
        children = list(self.node.get_children())
        assert(len(children) == 2)
        assert(children[1].kind == CursorKind.COMPOUND_STMT)
        assert len(CURRENT_STACK) > 0
        CURRENT_STACK.append(('CMIMID_SWITCH'))
        switch_expr = to_string(children[0])
        body = to_string(children[1])
        CURRENT_STACK.pop()
        assert len(CURRENT_STACK) > 0
        return '''\
__cmimid__res = (%s);
switch (__cmimid__res) %s
''' % (switch_expr, body)


class CompoundStmt(AstNode):
    def __repr__(self):
        outer_range = extent(self.node)
        stmts = []
        children = list(self.node.get_children())
        self.check_children_not_macro()
        label = None
        ilabel = 0
        seen_default = False
        for i,child in enumerate(children):
            rep = to_string(child)
            if child.kind == CursorKind.LABEL_STMT:
                # what is the outer scope here? Hopefully it is
                # in CURRENT_STACK[-1]
                pass
            elif child.kind == CursorKind.CASE_STMT:
                assert not seen_default
                gchildren = list(child.get_children())
                label = to_string(gchildren[0])
                assert len(gchildren) == 2
                fall_through = [label]
                while(gchildren[1].kind == CursorKind.CASE_STMT):
                    # fall through. We do not count it as a separate option.
                    child = gchildren[1]
                    gchildren = list(child.get_children())
                    label = to_string(gchildren[0])
                    fall_through.append(label)
                    assert len(gchildren) == 2
                ilabel += 1
                body = to_string(gchildren[1])
                rep = '''\
%s
%s
''' % ("\n".join(["case %s:" % c for c in fall_through]), body)

            elif child.kind == CursorKind.DEFAULT_STMT:
                seen_default = True
                gchildren = list(child.get_children())
                assert len(gchildren) == 1
                body = to_string(gchildren[0])
                ilabel += 1 # We dont expect any more after default
                rep = '''\
default:
%s;
''' % (body.strip())
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
        cparams = [p for p in children if p.kind == CursorKind.PARM_DECL]
        params = ", ".join([to_string(c) for c in cparams])

        if '...' in self.node.type.spelling:
            params = params + ", ..."
        if self.node.is_definition():
        #if children and children[-1].kind == CursorKind.COMPOUND_STMT:
            body = to_string(children[-1])
            return '''\
%s
%s(%s) {
int __cmimid__res = 0;
%s
}''' % (return_type, function_name, params, body)
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
        CursorKind.LABEL_STMT: LabelStmt,
        CursorKind.DEFAULT_STMT: DefaultStmt,
        CursorKind.NULL_STMT: NullStmt,
        CursorKind.FOR_STMT: ForStmt,
        CursorKind.WHILE_STMT: WhileStmt,
        CursorKind.DO_STMT: DoStmt,
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
        CursorKind.CXX_UNARY_EXPR: CXXUnaryExpr,
        CursorKind.CSTYLE_CAST_EXPR: CStyleCastexpr,
        CursorKind.TYPE_REF: TypeRef,
        CursorKind.PAREN_EXPR: ParenExpr,
        CursorKind.COMPOUND_ASSIGNMENT_OPERATOR: CompoundAssignmentOperator,
        CursorKind.UNEXPOSED_EXPR: UnexposedExpr,
        CursorKind.CHARACTER_LITERAL: CharacterLiteral,
        CursorKind.GOTO_STMT: GotoStmt,
        }


def to_ast(node):
    if node.kind in FN_HASH:
        return FN_HASH[node.kind](node)
    else:
        print(node.kind, file=sys.stderr)
        return AstNode(node)

STOPPED = False
def to_string(node):
    global STOPPED
    stp = os.environ.get('STOP')
    if stp is not None and not STOPPED:
        line = node.location.line
        if int(line) > int(stp):
            STOPPED = True

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
    CFLAGS = os.environ.get('CFLAGS', '') #'-xc++ std=c++14')
    translation_unit = idx.parse(arg, args =  CFLAGS.split(' '))
    # IMPORTANT: If you change values here, remember to change in src/events.py too
    print('''\
''')
    for i in translation_unit.cursor.get_children():
        if i.location.file.name == sys.argv[1]:
            display_till(i.extent.start.line-1)
            print(to_string(i), file=sys.stdout)
            displayed_till = i.extent.end.line
        else:
            pass
           #skipped.append(to_string(i))
    display_till(len(SRC))

store(sys.argv[1])
parse(sys.argv[1])
#for i in skipped:
#    print(repr(i), file=sys.stderr)
