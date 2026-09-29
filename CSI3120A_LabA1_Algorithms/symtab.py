"""
symtab.py -- Symbol table with STATIC (lexical) scoping for MiniScope.

STARTER CODE -- Symbol and Scope are complete; finish SymbolTable (TODO).

Concepts from Lecture 4 implemented here:
  * every declaration creates a BINDING between a name and its attributes
    (kind, type, declaring scope, line, storage category);
  * scopes are nested; each scope knows its STATIC PARENT;
  * a reference is resolved by searching the current scope, then its static
    parent, then the static ancestors up to the global scope;
  * an inner declaration HIDES an outer one with the same name;
  * the REFERENCING ENVIRONMENT of a statement = every visible (non-hidden)
    name in the current scope and its static ancestors.
"""


class Symbol:
    def __init__(self, name, kind, typ, line, scope, params=None):
        self.name = name
        self.kind = kind            # 'var', 'const', 'param' or 'func'
        self.type = typ             # 'int', 'float', 'bool', 'string' (return type for funcs, may be 'void')
        self.line = line            # line of the declaration
        self.scope = scope          # Scope object that declares the name
        self.params = params or []  # list of parameter types (functions only)

    @property
    def storage(self):
        """Storage category of the entity (Lecture 4, Part 5)."""
        if self.kind == "func":
            return "-"
        return "static" if self.scope.level == 0 else "stack-dynamic"

    def describe(self):
        if self.kind == "func":
            sig = f"({', '.join(self.params)}) : {self.type}"
            return f"func {self.name}{sig}"
        return f"{self.kind} {self.name} : {self.type}"


class Scope:
    def __init__(self, name, parent):
        self.name = name
        self.parent = parent                          # static parent
        self.level = 0 if parent is None else parent.level + 1
        self.symbols = {}                             # insertion-ordered

    def label(self):
        return f"'{self.name}' (level {self.level})"


class SymbolTable:
    def __init__(self):
        self.current = None

    # -- scope management ---------------------------------------------------
    def open_scope(self, name):
        self.current = Scope(name, self.current)
        return self.current

    def close_scope(self):
        self.current = self.current.parent

    # -- declarations ---------------------------------------------------------
    def declare(self, sym):
        """Bind sym.name in the CURRENT scope.
        Returns (ok, redeclared_symbol_or_None, hidden_symbol_or_None):
          * (False, previous, None) if the name is already declared in this same scope;
          * otherwise add it and return (True, None, hidden) where `hidden` is the
            outer declaration (found in a static ancestor) that the new one hides, or None.
        TODO
        """
        raise NotImplementedError("declare")

    # -- lookup (static scoping) ------------------------------------------------
    @staticmethod
    def lookup_from(scope, name):
        """STATIC scoping: search `scope`, then its static parent, then the
        static ancestors, up to the global scope. Return the Symbol or None.
        TODO
        """
        raise NotImplementedError("lookup_from")

    def lookup(self, name):
        return self.lookup_from(self.current, name)

    # -- referencing environment -----------------------------------------------
    def referencing_environment(self):
        """Returns (visible, hidden): lists of Symbols, innermost scope first,
        in declaration order inside each scope. A symbol is hidden when a
        scope closer to the current one declares the same name.
        TODO
        """
        raise NotImplementedError("referencing_environment")
