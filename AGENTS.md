# AI Agent Guidelines - XPR Project

## Goals
- This project is for learning compilers using C language;
- Do not build the solution/software but list all the steps that resolve the points
- When requested by the user, you may create support tools, but never code intended for a compiler or interpreter.
- Answer with clarity and objectvely

## Coding Standards
- **Language**: All code (variables, constants, functions) and comments must be in **English**.
- **Documentation**: May be in Portuguese or English.
- **Comments**: Focus on the "why" (intent/logic), not the "how" (syntax).
- **Header**: Every file must start with the following header:

```c
/*
   FILE: filename.c (or filename.h)
   DESC: short description of the file
   AUTHOR: Andre Cavalcante with Sonnet 5.5 partner
   DATE: October, 2026
   LICENSE: GPL-v3
*/
```

- **Header Guards**: All `.h` files must have `#ifndef`, `#define`, and `#endif` guards immediately following the header.

## Implementation Details
- **Memory Management**: Two allocators with different lifetimes:
    - **AST → `tgc`**: The AST is built once, grows during parsing, and lives for
      the whole run: function values point to `NODE_FN_DEF` nodes, and loaded `.x`
      files stay bound to the global scope. Every AST allocation (nodes, names,
      decoded strings, `NodeList` arrays) uses `tgc`.
    - **Runtime values → hierarchical arenas**: Variables and values created
      during execution live in scope arenas, so allocation and release are O(1)
      (bump pointer, mark/restore, reset). This fits the language semantics:
      everything is a value and every assignment copies (spec 4.8).
        - Each scope (global, function call, block) owns an arena; inner arenas
          are allocated inside outer ones. The arena pointer is kept in the
          variable control structure of that scope.
        - The destination decides the arena: a value assigned to a variable is
          allocated in the arena of the scope that owns the variable, not in the
          current one.
        - A value leaving a scope (block/function result) is deep-copied to the
          parent arena before the inner arena is reset.
        - `for` uses the enclosing function arena: `mark` at the start of the
          loop, `restore` at the end; the loop result is copied out before the
          last `restore`.
        - Function values carry their frozen variables (spec 4.5); copying a
          function value copies its captures too.
        - Runtime pointers cached in the AST (e.g. variable addresses) must be
          cleared when the variable leaves scope: an arena reset invalidates
          every pointer into it.
- **Architecture**:
    - `lexer.c`: Tokenization.
    - `parser.c`: Recursive descent AST generation.
    - `exec.c`: Evaluator with dynamic `.x` file resolution.
    - `builtins.c`: Native functions.
    - `diagnostic.c`: Error reporting.

## Session Continuity
- Read `docs/MEMORIA.md` and `docs/PROGRESSO.md` when resuming project work.
- Read Andre's personal progress notes for context, but never edit the sections titled `Registro do André` unless he explicitly asks to change them.
- Distinguish confirmed language decisions, test-suite assumptions, work in progress, and verified results. Existing binaries do not prove that the current sources build.
