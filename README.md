# XPR Interpreter

XPR is an expression-oriented language where every construct returns a value. It features dynamic loading of external expressions and first-class functions.

This is a learning project: the goal is to study compilers by building the interpreter in C (see `docs/plano.md`).

## Build and test
- `make`: builds the `xpr` binary.
- `make test`: builds first, then runs the lexer suite through `tests/run_tests.sh`.
- `bash tests/run_tests.sh`: checks the existing binary without building (Python 3.11+).
- `bash tests/run_tests.sh --suite runtime --list`: lists and validates the future runtime fixtures.
- `bash tests/run_tests.sh --suite runtime`: runs positive and negative runtime contracts; these require the parser and evaluator to be implemented.
- `make clean`: removes `obj/` and the binary.

## Usage
- `xpr -x "expression"`: Evaluates a string expression directly from the command line.
- `xpr file.x`: Evaluates the XPR script contained in the specified file.

## Project Structure
- `src/`: Source code and headers.
- `vendor/tgc/`: `tgc` garbage collector, used for the AST.
- `tests/`: Test cases (`.x` programs and `.expected` output).
- `docs/`: `xpr-specs.md` (language specification), `plano.md` (study plan), `PROGRESSO.md` (progress log).
- `Makefile`: Build system.

See [tests/README.md](tests/README.md) for exact-output checks, negative
diagnostics and the distinction between lexer fixtures and runtime contracts.

## License
GPL-v3
