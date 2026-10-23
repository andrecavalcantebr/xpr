# XPR Specification

XPR is an expression-oriented language where every construct returns a value.

## 1. Core Concepts
- **Expression-Oriented**: Every statement is an expression. Blocks, `iif`, and loops return values.
- **Dynamic Loading**: Undefined identifiers in function calls trigger a search for `{identifier}.x` in the search path (see 4.1).
- **First-Class Functions**: Functions can be passed as arguments and returned as values.
- **Two-phase execution**: the whole file is parsed into an AST before execution; errors are split into parse-time and run-time (see Section 7).

## 2. Types
- **Number**: Double-precision floating point (supports scientific notation).
- **String**: UTF-8 strings, holding any Unicode codepoint, with escape sequences (`\n`, `\t`, `\r`, `\0`, `\"`, `\'`, `\\`). 
- **Boolean**: `true` and `false`.
- **List**: Ordered collection of values `[val1, val2, ...]`, homogeneous or heterogeneous.
- **Function**: Executable blocks of code.

## 3. Grammar (EBNF)

```ebnf
program        = expression ;

expression     = assignment | fn_def | logical_or | block ;

assignment     = ( identifier | indexed_target | list_expr ) "=" expression ;
indexed_target = identifier { "[" expression "]" } ;
fn_def         = "fn" "(" ( identifier | params ) ")" block ;
params         = "[" [ identifier { "," identifier } ] "]" ;

logical_or     = logical_and { "or" logical_and } ;
logical_and    = logical_not { "and" logical_not } ;
logical_not    = "not" logical_not | equality ;
equality       = relational { ( "==" | "!=" ) relational } ;
relational     = arithmetic { ( "<" | ">" | "<=" | ">=" ) arithmetic } ;
arithmetic     = term { ( "+" | "-" ) term } ;
term           = factor { ( "*" | "/" ) factor } ;
factor         = [ "+" | "-" ] indexed ;

indexed        = primary { "[" expression "]" } ;

primary        = literal 
               | fn_call 
               | block 
               | iif_expr 
               | for_expr 
               | list_expr
               | "(" expression ")" ;

literal        = number | string | boolean | identifier ;
fn_call        = identifier "(" [ expression ] ")" ;
block          = "{" { statement } "}" ;
statement      = expression ";" ;
iif_expr       = "iif" "(" expression "," expression "," expression ")" ;
for_expr       = "for" "(" expression ";" expression ";" expression ")" expression ;
list_expr      = "[" [ expression { "," expression } ] "]" ;

// detected by lexer
number    = ( int_num [ "." { digit } ] [ exp_part ] )
          | ( "." { int_num } [ exp_part ] ) ;
int_num   = digit { digit } ;
exp_part  = ( "e" | "E" ) [ "+" | "-" ] int_num ;
int_num        = digit { digit } ;
digit          = ( "0"-"9" ) ;

string         = '"' { chars | escaped } '"' | "'" { chars | escaped } "'" ;
chars          = ? any Unicode codepoint except the surrounding quote and
                 U+005C ( \ ); control codepoints are accepted, so a string
                 may span several lines ? ;
// The lexer only delimits strings and does not validate their content. Control
// codepoints are sanitized when text is shown in diagnostics (see diagnostic.c).
escaped        = "\" ( "n" | "t" | "r" | "0" | '"' | "'" | "\" ) ;

identifier     = ( alfa | "_" ) { alfa | digit | "_" } ;
alfa           = ( "a"-"z" | "A"-"Z" ) ;
// Identifiers are ASCII-only (they also name the {identifier}.x files that are
// loaded dynamically). Non-ASCII characters are accepted only in strings and comments.

keywords       = ( "fn", "iif", "for", "not", "and", "or", "true", "false" ) ;

boolean        = "true" | "false" ; 
operators      = ( "+", "-", "*", "/", 
                   "<", ">", "=", ">=", "<=", "==", "!=" ) ;

// ignored by lexer
comments          = line_comment | multiline_comment ;
line_comment      = "//" { chars } ( "\n" | EOF ) ;
multiline_comment = "/*" { chars } "*/" ;
```

## 4. Semantics

### Numeric unary signs and whitespace

When unary `+` or `-` precedes a numeric literal, the sign must be directly
adjacent to that literal in the source. Whitespace, a newline or a comment
between the sign and the number is a `syntax-error`.

- `2 * -3` and `2 *-3` are valid.
- `2 *- 3`, `- 3` and `+ 3` are invalid.
- Binary subtraction still allows whitespace: `2 - 3` is valid.

This source-adjacency constraint supplements the EBNF. A separate sign token
is compatible with the rule: the parser can distinguish unary and binary
use and check the source spans. Token columns alone must not be confused
with byte offsets in UTF-8 text.

The existing grammar also admits unary operands such as `-x` and `-(2 + 3)`;
the numeric-literal adjacency rule above does not redefine those forms.

### 4.1 Dynamic Loading

When the evaluator encounters a function call `identifier(...)` and the identifier
is not defined in the current scope:

1. It searches for a file named `identifier.x` in the search path (below).
2. If found, the file is parsed and evaluated as a single expression. The result
   must be a function, which is then called with the argument — exactly as if it
   had been defined locally. If the value is not a function, a `type-mismatch`
   occurs at the call.
3. If not found, an `undefined-identifier` error is raised.

Search path, in order (first hit wins):

1. the directory of the main script;
2. each directory in the `XPRPATH` environment variable (`:`-separated, in order);
3. the standard library directory `(bindir)/../base`.

External functions follow the same rules as local ones, including frozen
globals and recursion:

- The file's expression is evaluated at the **first call** (load time); free
  variables are resolved and frozen at that moment.
- During the evaluation of `identifier.x`, the name `identifier` counts as
  defined (pending), so a file can recurse on itself.
- Each file is loaded once; after the first load, the function is bound to the
  global scope and later calls skip the search.
- The file's expression may be a block, which gives the file private names
  via block scope.
- Consequence: mutual recursion between files fails at load time (the other
  name is undefined there), and cyclic loads cannot loop forever.
- Builtins are predefined globals and are never shadowed by files: loading
  only fires for identifiers that are not defined.

Running an external function is nothing more than:
- load the source of the function from the file:
  `const unsigned char *src = load(filename);`
- call the parser recursively: `parser_parse(src);`

Parse errors inside a loaded file are reported with the loaded file's name,
line and column.

### 4.2 Blocks
A block `{ expr1; expr2; ... exprN; }` evaluates all expressions sequentially and returns the value of `exprN`.

The value of an empty block is the empty list `[]`.

Statements demand `;` at the end. Then:

```c
{3;} // is ok
{3} // is error
```

And:

```c
{
  f = fn(x){ x + 9; };
}
```

Needs 2 ';': one after the expression `(x+9)` and one after the `fn` expression definition.

A program is an expression, then:

`3`
`3+1`
`fn(x){...}`
`{statement1; statement2;}`

are all valid, but...

`3;`
`3+1;`
`fn(x){...};`
`{statement1; statement2;};`

are syntax errors. Likewise: `3; 3+1;` and `{3}`


### 4.3 iif (Ternary)

`iif(cond, then_expr, else_expr)`:

- Evaluates `cond`.
- If `true`, evaluates and returns `then_expr`.
- If `false`, evaluates and returns `else_expr`.
- Branches are evaluated lazily

### 4.4 for loop

`for(init; cond; step) expr` — where `expr` is usually a block:

- Executes `init`.
- While `cond` is true:
  - Executes `expr` (the loop body).
  - Executes `step`.
- Returns the number of executions of the body. If the condition is initially false, returns 0.

### 4.5 functions

- All functions are in the form `y = f(x)`. Only one argument for input and one argument for output. However, `x` and/or `y` can be list, then: `[a, b] = f([c,d,e]);`

- A call with none parameter (`y = f()`) is equivalent to a call with a empty list (`y=f([])`).

- Note that `y = f(a,b)` is a syntax error. xpr functions have just one argument. Use: `y = f([a,b])` instead. 

- `fn([a, b])` declares a function of one list argument, destructured into
  `a` and `b` inside the body (`f([1, 2])` binds `a=1`, `b=2`).

- Variables can be assigned by functions. Function variables can be passed in parameter like any other variables. Just pay attention if `f` is variable assigned to a function or evaluated:

```c
f = fn(x){x + 9;};
use_func(f); // passes the function
use_value(f(1)); // passes the value 10
```

- There is no saved variables:

```c
c = 9;
f9 = fn(x){x + c;}; // defines a function
f19 = fn(x){x + c;}; // defines same function
c = 19;   // doesn't change f9 or f19
f19 = fn(x){x + c;}; // re-defines a function
print(f19(1));   //prints 20.00
```

- Free variables are resolved and frozen at definition time. A free variable
  that is not defined at that moment raises `undefined-identifier` at the
  definition:

```c
g = fn(x){ x + c; };   // error: c is not defined yet
```

- A function variable is just a "pointer" to the AST node of the function definition: reassigning the variable later does not change functions already created.

```c
fat = fn(x){ iif(x<=1, 1, fat(x-1)); };   // recursive body
g = fat;
fat = fn(x){ 999; };   //this is a new function
g(5);                  // 120, because g is point to first function
```

- Exception: while `name = fn(...){ ... }` is being evaluated, name counts as defined inside the body — like a forward declaration in C. The self-reference binds to the function being created, not to the variable.

- Parameters shadow globals: in `fn(c){ c + 1; }` the c in the body is the
parameter.

- Mutual recursion is not possible: in `even = fn(x){ ... odd(...) ...; };`
the name odd is undefined at that moment and is not the pending name, so
it errors.

### 4.6. Scope

Blocks define scope. A program runs in a single global scope, and the program itself is one expression. Variables only exist within the scope of their definitions. If a function is assigned to a variable and that variable goes out of scope, the definition goes out too — unless other variables hold the same function.

Variables and functions can be used only **after** their definition.

```c
fat = fn(x){ iif(int(x) == 0 or int(x) == 1, 1, fat(int(int(x)-1))); };

c = fat(5);  //the variable fat is already defined
```

### 4.7 Assignments

- An assignment is an expression, so it returns the assigned value.
- Chains of assignments are evaluated right to left.
- The type of a variable is defined at the first assignment; reassigning a
  value of a different type is a `type-mismatch`.

Example (with `a` and `b` undefined before):
```c
a = [a, b] = [1, 2]; //error, a is a number, not a list
a = [a, b] = [[1, 2], 3];  //ok. After the assignment a == [[1,2], 3] and b == 3; 
                           //(a is a list with 2 elements)
```
### 4.8 Values and copies

Everything is a value; there are no references. Every assignment — including
parameter passing — **copies** the value:

```c
a = [1, 2];
b = a;        // b is an independent copy
b[0] = 9;     // a is still [1, 2]

f = fn(x){ x[0] = 9; };   // never mutates the caller's list
f(a);                     // a is still [1, 2]
```

This is the same principle behind frozen variables in functions (4.5): a function captures the values that existed at its definition.

Indexed assignment writes into the variable’s own list:

```c
a = [[1, 2], 3];
a[0] = [3, 4];   // a == [[3, 4], 3]
a[1][0] = 9;     // error: a[1] is a number (type-mismatch)
```

- `v[i] = expr` assigns to element i and returns the assigned value.

- Writing out of range raises index-out-of-range (lists never grow
implicitly; use cat).

- Strings are immutable: indexing a string is read-only (`at`/`s[i]` returns
a one-codepoint string) and `s[i] = expr` is a `type-mismatch`.

## 5. Built-in Functions

- `print(x)`: Print the argument to stdout. Returns the number of **codepoints**
  printed. Use `print([a,b])` for more variables (separated by a single space;
  strings are printed unquoted).
  No trailing newline is added automatically: use an explicit `print("\n")`
  or include `\n` in a string when needed. Explicit newline characters count
  toward the returned codepoint count. `println` is not currently a builtin.

- `len(list/string)`: Returns length (for strings, in codepoints).
  `len(number)` is a `type-mismatch`.

- `input(prompt)`: Reads string from stdin.

- `string(val)`, `number(val)`, `bool(val)`: Type conversion.

- `int(num)`, `frac(num)`, `ceil(num)`, `floor(num)`, `round(num)`: rounding.
  `int` truncates toward zero; `floor`/`ceil` round down/up; `round` rounds
  half away from zero; `frac` returns the signed fractional part
  (`frac(x) == x - int(x)`).

| x | int | floor | ceil | round | frac |
|---|---|---|---|---|---|
| 3.99 | 3 | 3 | 4 | 4 | 0.99 |
| -3.7 | -3 | -4 | -3 | -4 | -0.7 |
| 5.5 | 5 | 5 | 6 | 6 | 0.5 |

Infinity and NaN propagate (`int(INF) == INF`, `int(NaN) == NaN`), following
`trunc`, `floor`, `ceil`, `round` from C `math.h`.

- `any(list)`: true if at least one element is true.

- `all(list)`: true only if every element is true.

- `cat(a, b)`: concatenates two lists (`cat([1,2],[3])` is `[1,2,3]`).

- `at(list/string, i)`: returns element `i` (0-based). For strings, returns
  a one-codepoint string. `v[i]` is sugar for `at(v, i)`.

- Builtins are predefined global symbols and are never shadowed by external
  `.x` functions: dynamic loading only fires for identifiers that are not
  defined. A user definition binds an anonymous function value to a variable.

- Builtins hold function values and can be passed as arguments like any
  function (`use_func(print)` is valid).

### 5.1 Number formatting

`fmt` is a predefined global variable holding `[decimals, sigdigits]`
(default `[2, 15]`). It controls how `print` and `string` convert numbers to
text:

- `NaN`, `INF` and `-INF` print literally, ignoring `fmt`.
- If `10^-decimals <= |x| < 10^sigdigits`: fixed notation with `decimals`
  digits after the point — `20` prints as `20.00`.
- Otherwise: scientific notation with `sigdigits` significant digits —
  `0.001` prints as `1.00e-3`; `1234567890123456789` as `1.23456789012346e+18`.
- Rounding follows C `printf` (round half to even).
- `number(string(x)) == x` is not guaranteed; use `fmt = [17, 17]` for exact
  round-trips.

### 5.2 Operator semantics

| operands | `+` | `-` `*` `/` | `< <= > >=` | `== !=` | `not`/`and`/`or` |
|---|---|---|---|---|---|
| number ⊗ number | sum | arithmetic | boolean | boolean | — |
| string ⊗ string | concatenation | type-mismatch | boolean, lexicographic by codepoint ("A" < "a") | boolean | — |
| list ⊗ list, same size | element-wise | element-wise | element-wise | element-wise | — |
| list ⊗ list, different size | size-mismatch | size-mismatch | size-mismatch | size-mismatch | — |
| scalar ⊗ list | type-mismatch (no broadcast) | type-mismatch | type-mismatch | type-mismatch | — |
| boolean ⊗ boolean | type-mismatch | type-mismatch | type-mismatch | boolean | boolean; `and`/`or` short-circuit |
| function operands | type-mismatch | type-mismatch | type-mismatch | type-mismatch | — |

Lists use one single rule, applied recursively at every level: the operation
is applied element by element, and when both elements of a pair are lists,
the same rule applies again.

- `+`: number leaves sum; string leaves concatenate; list pairs recurse
  (`[[1],[2]] + [[3],[4]]` is `[[1]+[3], [2]+[4]]` is `[[4],[6]]`).

- `-`, `*`, `/`: number leaves only; list pairs recurse.

- comparisons: number/string leaves produce a boolean, collected into a
  list (`[a,b] == [c,d]` is `[a==c, b==d]`); fold with `any`/`all`.

- List concatenation is never done with `+`; use `cat(a, b)`.

- NaN follows IEEE-754: every comparison involving NaN is false.


## 6. Math Constants

- `NaN` : Not a Number
- `INF` : Infinity - can be used unary "+" or "-" with it
- `pi` or `PI` : the pi;
- `exp` or `EXP` : the Euler number

## 7. Errors

xpr is an interpreter, but the whole file is parsed into an AST before
execution, so errors exist in two phases: **parse-time** and **run-time**.
Syntax errors are detected even inside function bodies that are never called.
In any error, the file, line and column (counted in codepoints) and the error
type are shown with a message.

**Parse-time errors**

| Error | Example | Description |
| -- | -- | -- |
| `syntax-error` | `3;` · `{3}` · `f(a, b)` | trailing `;` at program level; missing `;` in block; two arguments |
| `wrong-assignment` | `int(a) = 10.6` | left side must be an identifier or a list of identifiers |
| `empty-program` | loaded `.x` with no expression | the call requires a value |

**Run-time errors**

| Error | Example | Description |
| -- | -- | -- |
| `size-mismatch` | `[a, b] = [1, 2, 3]` | Lists need to be the same size to apply operations |
| `type-mismatch` | `[0, "hello", 2.0] + [0, 2.0, " world"]` | Operations on lists are element by element (Notes 1) |
| `type-mismatch` | `a = 0; b = "10"; c = a + b;` | There is no implicit conversion |
| `type-mismatch` | `[a, b] = 5` (Notes 2) | Left side must be the same type of the right side |
| `undefined-identifier` | `a = b + ident(x);` | `b` is not defined; or `ident` was not found as a file |
| `undefined-identifier` | `g = fn(x){ x + c; };` | free variable not defined at the definition (see 4.5) |
| `div-by-zero` | `a = 10.0/0.0` | not possible |
| `not-a-function` | `a = 5; a(1);` | the call target is not a function |
| `stack-overflow` | `f = fn(x){ f(x); }; f(1);` | recursion depth limit exceeded (see Notes 4) |
| `index-out-of-range` | `a = [1,2]; a[5];` | index outside 0..len-1 |


> Notes:<br>
> (1) The expression `[0, "hello", 2.0] + [0, " world", 2]` produces `[0, "hello world", 4.0]` and it is valid.<br>
> (2) Note that `a = [c, d] = [1, 2]` is valid, but `a = [a, b] = [1, 2]` doesn't (`type-mismatch`: `a` is a number, not a list). A third form is valid:  `a = [a, b] = [[1,2], 2]`.<br>
> (3) `a = int(10.6)` results in `a == 10.0` exactly.<br>
> (4) Stack depth is fixed at 10,000.
