"""
parser.py -- Recursive-descent parser + static-scope checker for MiniScope.

STARTER CODE -- functions marked TODO must be completed.
The functions that are already written are worked examples: follow their style.

One parsing function per non-terminal of the grammar (Sebesta, Ch. 4).
The left-recursive expression rules of the BNF were rewritten in EBNF
(iteration instead of left recursion) before being coded, e.g.

    <add_expr> -> <term> { ( + | - ) <term> }

The parser stops at the first syntax error.  Semantic actions (declarations,
name resolution, type checks) are executed while parsing, in a single pass,
so a name is only visible AFTER its declaration.
"""

from symtab import Symbol, SymbolTable

TYPE_TOKENS = {"KW_INT": "int", "KW_FLOAT": "float", "KW_BOOL": "bool", "KW_STRING": "string"}
REL_OPS = {"EQ_OP", "NEQ_OP", "LT_OP", "LE_OP", "GT_OP", "GE_OP"}
NUMERIC = {"int", "float"}

# Human-readable spelling of token codes, for error messages
SPELLING = {
    "SEMICOLON": "';'", "COLON": "':'", "COMMA": "','", "LEFT_PAREN": "'('",
    "RIGHT_PAREN": "')'", "LEFT_BRACE": "'{'", "RIGHT_BRACE": "'}'",
    "ASSIGN_OP": "'='", "IDENT": "identifier", "KW_PROGRAM": "'program'",
    "EOF": "end of file",
}


class MiniScopeSyntaxError(Exception):
    def __init__(self, line, msg):
        super().__init__(msg)
        self.line = line
        self.msg = msg


def assignable(target, source):
    """Can a value of type `source` be stored in a variable of type `target`?"""
    if target is None or source is None:          # unknown type: error already reported
        return True
    return target == source or (target == "float" and source == "int")


class Parser:
    def __init__(self, tokens, trace=False, out=print):
        self.tokens = tokens
        self.i = 0
        self.trace = trace
        self.depth = 0
        self.out = out
        self.st = SymbolTable()
        self.func_stack = []          # enclosing function symbols (for 'return')
        self.errors = 0
        self.warnings = 0

    # ======================================================================
    # Token helpers
    # ======================================================================
    @property
    def tok(self):
        return self.tokens[self.i]

    def at(self, *kinds):
        return self.tok.kind in kinds

    def advance(self):
        t = self.tok
        if t.kind != "EOF":
            self.i += 1
        return t

    def expect(self, kind, what=None):
        if self.tok.kind != kind:
            self.syntax_error(what or SPELLING.get(kind, kind))
        return self.advance()

    def syntax_error(self, expected):
        found = "end of file" if self.tok.kind == "EOF" else f"'{self.tok.lexeme}'"
        raise MiniScopeSyntaxError(self.tok.line, f"expected {expected} but found {found}")

    # ======================================================================
    # Tracing and diagnostics
    # ======================================================================
    def enter(self, nt):
        if self.trace:
            self.out("  " * self.depth + f"Enter <{nt}>")
        self.depth += 1

    def exit(self, nt):
        self.depth -= 1
        if self.trace:
            self.out("  " * self.depth + f"Exit <{nt}>")

    def report(self, tag, line, msg):
        self.out(f"[{tag}] line {line}: {msg}")

    def sem_error(self, line, msg):
        self.errors += 1
        self.report("error", line, msg)

    def warn(self, line, msg):
        self.warnings += 1
        self.report("warn", line, msg)

    # ======================================================================
    # Semantic helpers
    # ======================================================================
    def declare(self, sym):
        ok, previous, hidden = self.st.declare(sym)
        if not ok:
            self.sem_error(sym.line, f"'{sym.name}' is already declared in scope "
                                     f"{self.st.current.label()} at line {previous.line}")
            return
        self.report("decl", sym.line, f"{sym.describe()} in scope {sym.scope.label()} [{sym.storage}]")
        if hidden is not None:
            self.warn(sym.line, f"'{sym.name}' hides {hidden.kind} '{hidden.name}' "
                                f"declared at line {hidden.line} in scope {hidden.scope.label()}")

    def resolve(self, name, line):
        sym = self.st.lookup(name)
        if sym is None:
            self.sem_error(line, f"'{name}' is not declared (no binding visible in scope "
                                 f"{self.st.current.label()})")
        else:
            self.report("bind", line, f"'{name}' -> {sym.describe()}, declared at line {sym.line} "
                                      f"in scope {sym.scope.label()}")
        return sym

    # ======================================================================
    # <program> -> program IDENT <block>
    # ======================================================================
    def parse_program(self):
        self.enter("program")
        self.expect("KW_PROGRAM")
        self.expect("IDENT", "program name")
        self.st.open_scope("global")
        self.parse_block(new_scope=False)
        self.st.close_scope()
        if not self.at("EOF"):
            self.syntax_error("end of file")
        self.exit("program")

    # <block> -> { <item_list> }
    # <item_list> -> <item> <item_list> | epsilon
    def parse_block(self, new_scope=True):
        self.enter("block")
        lbrace = self.expect("LEFT_BRACE")
        if new_scope:
            self.st.open_scope(f"block@{lbrace.line}")
        while not self.at("RIGHT_BRACE", "EOF"):
            self.parse_item()
        self.expect("RIGHT_BRACE")
        if new_scope:
            self.st.close_scope()
        self.exit("block")

    # <item> -> <decl> | <stmt>
    def parse_item(self):
        self.enter("item")
        if self.at("KW_VAR"):
            self.parse_var_decl()
        elif self.at("KW_CONST"):
            self.parse_const_decl()
        elif self.at("KW_FUNC"):
            self.parse_func_decl()
        else:
            self.parse_stmt()
        self.exit("item")

    # <type> -> int | float | bool | string
    def parse_type(self):
        self.enter("type")
        if not self.at(*TYPE_TOKENS):
            self.syntax_error("a type (int, float, bool or string)")
        t = TYPE_TOKENS[self.advance().kind]
        self.exit("type")
        return t

    # <var_decl> -> var IDENT : <type> <init> ;
    # <init>     -> = <expr> | epsilon
    def parse_var_decl(self):
        self.enter("var_decl")
        self.expect("KW_VAR")
        name = self.expect("IDENT")
        self.expect("COLON")
        typ = self.parse_type()
        if self.at("ASSIGN_OP"):
            self.advance()
            etype = self.parse_expr()        # initializer is evaluated BEFORE the name is bound
            if not assignable(typ, etype):
                self.sem_error(name.line, f"type mismatch: cannot initialize '{name.lexeme}' ({typ}) with a {etype} value")
        self.expect("SEMICOLON")
        self.declare(Symbol(name.lexeme, "var", typ, name.line, self.st.current))
        self.exit("var_decl")

    # <const_decl> -> const IDENT : <type> = <expr> ;
    def parse_const_decl(self):
        """TODO: Like parse_var_decl, but the initializer is mandatory and the Symbol kind is 'const'.
        """
        raise NotImplementedError("parse_const_decl")

    # <func_decl>  -> func IDENT ( <params> ) : <ret_type> <block>
    # <params>     -> <param_list> | epsilon
    # <param_list> -> <param> | <param> , <param_list>
    # <param>      -> IDENT : <type>
    # <ret_type>   -> <type> | void
    def parse_func_decl(self):
        """TODO: Parse the header, then: (1) declare the function name in the ENCLOSING scope
        (so it can call itself), (2) open a scope named after the function, (3) declare the
        parameters there (kind 'param'), (4) push the function on self.func_stack, parse the
        body with new_scope=False, pop, and close the scope.
        """
        raise NotImplementedError("parse_func_decl")

    def parse_param(self):
        """TODO: Return a tuple (name, line, type).
        """
        raise NotImplementedError("parse_param")

    # <stmt> -> <ident_stmt> | <if_stmt> | <while_stmt> | <print_stmt>
    #         | <return_stmt> | <env_stmt> | <block>
    def parse_stmt(self):
        self.enter("stmt")
        if self.at("IDENT"):
            self.parse_ident_stmt()
        elif self.at("KW_IF"):
            self.parse_if_stmt()
        elif self.at("KW_WHILE"):
            self.parse_while_stmt()
        elif self.at("KW_PRINT"):
            self.parse_print_stmt()
        elif self.at("KW_RETURN"):
            self.parse_return_stmt()
        elif self.at("KW_SHOW_ENV"):
            self.parse_env_stmt()
        elif self.at("LEFT_BRACE"):
            self.parse_block()
        else:
            self.syntax_error("a declaration or a statement")
        self.exit("stmt")

    # <ident_stmt> -> IDENT <ident_tail> ;
    # <ident_tail> -> = <expr> | ( <args> )
    def parse_ident_stmt(self):
        """TODO: Assignment or call statement (left-factored on IDENT).
        Assignment: resolve the name; report an error if it is a const or a func, or if the
        expression type is not assignable to the variable type.
        """
        raise NotImplementedError("parse_ident_stmt")

    # <if_stmt>   -> if ( <expr> ) <block> <else_part>
    # <else_part> -> else <block> | epsilon
    def parse_if_stmt(self):
        """TODO: Use self.check_condition(...) on the condition's type.
        """
        raise NotImplementedError("parse_if_stmt")

    # <while_stmt> -> while ( <expr> ) <block>
    def parse_while_stmt(self):
        """TODO: Use self.check_condition(...) on the condition's type.
        """
        raise NotImplementedError("parse_while_stmt")

    def check_condition(self, t, line, what):
        if t is not None and t != "bool":
            self.sem_error(line, f"condition of '{what}' must be bool, found {t}")

    # <print_stmt> -> print ( <expr> ) ;
    def parse_print_stmt(self):
        self.enter("print_stmt")
        self.expect("KW_PRINT")
        self.expect("LEFT_PAREN")
        self.parse_expr()
        self.expect("RIGHT_PAREN")
        self.expect("SEMICOLON")
        self.exit("print_stmt")

    # <return_stmt> -> return <ret_val> ;
    # <ret_val>     -> <expr> | epsilon
    def parse_return_stmt(self):
        """TODO: Check: return outside a function; a void function returning a value;
        a non-void function returning nothing; a returned value of the wrong type.
        """
        raise NotImplementedError("parse_return_stmt")

    # <env_stmt> -> show_env ;
    def parse_env_stmt(self):
        self.enter("env_stmt")
        kw = self.expect("KW_SHOW_ENV")
        self.expect("SEMICOLON")
        self.print_environment(kw.line)
        self.exit("env_stmt")

    def print_environment(self, line):
        visible, hidden = self.st.referencing_environment()
        self.out(f"----- referencing environment at line {line}, scope {self.st.current.label()} -----")
        self.out(f"  {'name':<10}{'kind':<7}{'type':<8}{'declared in':<22}{'line':<6}storage")
        for s in visible:
            self.out(f"  {s.name:<10}{s.kind:<7}{s.type:<8}{s.scope.label():<22}{s.line:<6}{s.storage}")
        if hidden:
            self.out("  hidden: " + ", ".join(f"{s.name} ({s.scope.label()}, line {s.line})" for s in hidden))
        else:
            self.out("  hidden: none")
        self.out("-" * 72)

    # <call> -> IDENT ( <args> )      (IDENT already consumed)
    # <args> -> <arg_list> | epsilon
    # <arg_list> -> <expr> | <expr> , <arg_list>
    def parse_call(self, name):
        """TODO: Resolve the name, parse the argument list, then check that the name is a
        function, the number of arguments, and each argument type. Return the function's
        return type (or None if unknown).
        """
        raise NotImplementedError("parse_call")

    # ======================================================================
    # Expressions (EBNF form of the left-recursive BNF rules)
    # ======================================================================
    # <expr> -> <and_expr> { || <and_expr> }
    def parse_expr(self):
        """TODO: EBNF iteration, like parse_add_expr. Use self.logical(...) for the type.
        """
        raise NotImplementedError("parse_expr")

    # <and_expr> -> <rel_expr> { && <rel_expr> }
    def parse_and_expr(self):
        """TODO: EBNF iteration, like parse_add_expr. Use self.logical(...) for the type.
        """
        raise NotImplementedError("parse_and_expr")

    def logical(self, a, b, op):
        if a is None or b is None:
            return "bool"
        if a != "bool" or b != "bool":
            self.sem_error(op.line, f"operator '{op.lexeme}' needs bool operands, found {a} and {b}")
        return "bool"

    # <rel_expr> -> <add_expr> [ <rel_op> <add_expr> ]
    def parse_rel_expr(self):
        """TODO: At most ONE relational operator (non-associative). The result type is bool.
        == and != accept two values of the same type (or two numbers);
        < <= > >= accept numbers only.
        """
        raise NotImplementedError("parse_rel_expr")

    # <add_expr> -> <term> { ( + | - ) <term> }
    def parse_add_expr(self):
        self.enter("add_expr")
        t = self.parse_term()
        while self.at("ADD_OP", "SUB_OP"):
            op = self.advance()
            t = self.arith(t, self.parse_term(), op)
        self.exit("add_expr")
        return t

    # <term> -> <factor> { ( * | / | % ) <factor> }
    def parse_term(self):
        """TODO: EBNF iteration, like parse_add_expr. Use self.arith(...) for the type.
        """
        raise NotImplementedError("parse_term")

    def arith(self, a, b, op):
        if a is None or b is None:
            return None
        if op.kind == "ADD_OP" and a == "string" and b == "string":
            return "string"                                   # concatenation
        if op.kind == "MOD_OP":
            if a == "int" and b == "int":
                return "int"
        elif a in NUMERIC and b in NUMERIC:
            return "float" if "float" in (a, b) else "int"
        self.sem_error(op.line, f"operator '{op.lexeme}' cannot be applied to {a} and {b}")
        return None

    # <factor> -> ! <factor> | - <factor> | <primary>
    def parse_factor(self):
        """TODO: '!' needs a bool operand, unary '-' a numeric one. Return the type.
        """
        raise NotImplementedError("parse_factor")

    # <primary> -> INT_LIT | FLOAT_LIT | STRING_LIT | true | false
    #            | IDENT | IDENT ( <args> ) | ( <expr> )
    def parse_primary(self):
        """TODO: Literals, true/false, IDENT (resolve it: it must not be a func),
        IDENT ( <args> ) (a call; a void function cannot be used in an expression),
        and ( <expr> ). Anything else: self.syntax_error("an expression"). Return the type.
        """
        raise NotImplementedError("parse_primary")
