# Plano de estudos: xpr

Disciplina prática de Compiladores. O aluno implementa, em C, um interpretador para a linguagem **xpr** (ver `@docs/xpr-specs.md`). A IA é tutor: explica conceitos, faz perguntas e revisa; o aluno escreve o código e os testes.

Ritmo de referência: **~5 h/semana**.

## Etapas

| # | Etapa | Conteúdo | Horas |
|---|-------|----------|------:|
| 0 | Infraestrutura e lexer | Pipeline lexer → parser → AST → exec; EBNF; `tgc`; `loader`; `diagnostic` (tabela de erros); `tests/run_tests.sh`; ASan. **Lexer já existe**: revisar e fechar | 4 |
| 1 | AST e aritmética | `Node` (união etiquetada); descida recursiva `arithmetic → term → factor → indexed → primary`; unário e parênteses; modo `--ast`; `exec` devolvendo `Value` numérico; `div-by-zero` | 10 |
| 2 | Programa e blocos | `program = expression`; regra do `;` (`3;` e `{3}` são erros, `{3;}` não); bloco vazio vale `[]`; erros de sintaxe com arquivo, linha e coluna | 6 |
| 3 | Variáveis e atribuição | Escopo global (arena global); atribuição como expressão, associativa à direita; tipo fixado na 1ª atribuição; `undefined-identifier`; constantes (`pi`, `INF`, `NaN`) | 6 |
| 4 | Booleanos, strings e comparações | `Value` com tags; `== != < > <= >=`; `and`/`or` com curto-circuito; `not`; strings UTF-8 com escapes; `+` em strings; `iif` preguiçoso; tabela de operadores (spec 5.2) numa função central | 8 |
| 5 | Escopo de bloco | Arena por bloco, `mark`/`restore`; valor do bloco copiado para a arena pai; sem vazamento de variáveis. **Resolver antes a questão de memória 1 (ver PROGRESSO)** | 6 |
| 6 | Funções | `fn(x){...}`; chamada `f(x)` com 1 argumento; `f()` ≡ `f([])`; captura congelada na definição; nome pendente para recursão; parâmetros que sombreiam globais; `not-a-function`; `stack-overflow` (10 000) | 12 |
| 7 | Listas | Literal; indexação `v[i]` e `v[i] = x`; `[a, b] = ...` (destructuring); operações elemento a elemento recursivas; `size-mismatch`; `index-out-of-range`; cópia por valor (spec 4.8) | 10 |
| 8 | `for` e builtins | `for(init; cond; step) expr` devolvendo nº de execuções (**resolver antes a questão de memória 2**); `print`, `len`, `input`, `string`, `number`, `bool`, arredondamentos, `any`, `all`, `cat`, `at`; `fmt` (spec 5.1) | 8 |
| 9 | Carregamento dinâmico | `identifier.x`; caminho de busca (dir do script, `XPRPATH`, stdlib); carga única; nome pendente; erros de parse no arquivo carregado; `empty-program` | 6 |
| 10 | Integração e acabamento | Exemplos da spec rodando; vazamentos (ASan/valgrind); retrospectiva escrita | 4 |
| | **Total** | | **80 h** |

**Estimativa:** 80 h (faixa realista 65–100 h). A 5 h/semana, são **~16 semanas**.

Hipóteses: o aluno conhece C razoavelmente, mas é novo em compiladores. A carga se divide em ~25% leitura e teoria, ~45% código e ~30% testes e depuração.

## Ordem de implementação: fatias verticais

Cada etapa entrega um programa que roda de ponta a ponta, mesmo com poucos recursos: um recurso por vez, passando por parser e avaliação (o lexer já está pronto).

Regras que valem em todas as etapas:

- **A spec é o contrato.** Se a implementação pedir uma mudança, atualize `docs/xpr-specs.md` antes de codar.
- **Teste junto com o código.** Uma etapa só fecha com os testes que ela introduziu passando.
- **`exec` devolve `Value` desde a etapa 1**, mesmo só com número. A assinatura não muda quando entram as outras tags.
- **Operações sobre valores numa função central** (tabela 5.2 da spec), não num `switch` dentro de `exec`. Isso permite o comportamento elemento a elemento das listas (etapa 7) sem reescrever o avaliador.
- **Erros com arquivo, linha e coluna** (em codepoints) propagados do lexer até a mensagem, sempre via `diagnostic.c`.
- **Memória:** AST em `tgc`; valores em arenas (ver `AGENTS.md`).

## Testes por etapa

O aluno escreve os casos na suíte correspondente: `tests/lexer/NOME.x` com `tests/lexer/NOME.expected`, ou contratos de execução em `tests/runtime/` (`cases/`, `expected/`, `errors/` e `errors.toml`); `make test` roda `tests/run_tests.sh`. Sugestão de cobertura mínima:

| Etapa | O que testar |
|---|---|
| 0 | Os testes léxicos que já existem passam; erros léxicos (`error_*.x`) com linha e coluna |
| 1 | Precedência e associatividade (`1+2*3`, `10-4-3`, `-2*-3`); parênteses; `1/0`; `--ast` |
| 2 | `3` ok; `3;` `3; 3+1;` `{3}` erro; `{3;}` ok; `{}` vale `[]`; posição do erro |
| 3 | `a = b = 3`; `a = 1; a = "x"` (`type-mismatch`); variável indefinida |
| 4 | Cada linha da tabela 5.2; curto-circuito de `and`/`or`; `iif` só avalia o ramo escolhido; escapes e UTF-8 |
| 5 | Variável de bloco não vaza; valor do bloco sobrevive ao `restore`; blocos aninhados |
| 6 | Congelamento (`c = 9; f = fn(x){x+c;}; c = 19; f(1)` → 10); recursão (fatorial); `g = fat` mantém a função antiga; mutual recursion falha; `stack-overflow` |
| 7 | Mutação não afeta a cópia (`b = a; b[0] = 9`); destructuring; `[a,b] = [1,2,3]` (`size-mismatch`); lista aninhada `+` |
| 8 | `for` devolve nº de execuções (inclusive 0); cada builtin; tabela de arredondamento; `fmt` |
| 9 | Arquivo `.x` simples; recursão em arquivo; carga única; erro de parse em arquivo carregado |
| 10 | Todos os exemplos da spec; ASan sem erros |

Observação: hoje os `.expected` léxicos comparam com o despejo de tokens de `debug_parse`. Quando o parser assumir `main`, mantenha esse despejo atrás de uma opção (`--lex`) ou regenere os `.expected`, para os testes do lexer não se perderem.

## Critério de conclusão de cada etapa

1. Os testes da etapa passam (`make test`), sem avisos do `-Wall -Wextra` e sem erros do ASan.
2. As perguntas de verificação da etapa foram respondidas com as palavras do próprio aluno.
3. `docs/PROGRESSO.md` foi atualizado com a data, as horas gastas e as dúvidas em aberto.

## Arquivos produzidos pelo aluno

- `src/lexer.c`, `src/lexer.h` (existem)
- `src/loader.c`, `src/loader.h` (existem)
- `src/diagnostic.c`, `src/diagnostic.h` (esqueleto a corrigir)
- `src/parser.c`, `src/parser.h`
- `src/value.c`, `src/value.h`, `src/arena.c`, `src/arena.h` (nomes sugeridos)
- `src/exec.c`, `src/builtins.c`
- `src/main.c`
- `tests/lexer/*.x` e `tests/lexer/*.expected` (contratos de execução em `tests/runtime/`)
