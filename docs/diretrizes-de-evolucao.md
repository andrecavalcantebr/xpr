/*
   FILE: diretrizes-de-evolucao.md
   DESC: Findings and recommended steps for the evolution of the XPR project.
   AUTHOR: Andre Cavalcante with Sonnet 5.5 partner
   DATE: October, 2026
   LICENSE: GPL-v3
*/

# Diretrizes para a evolução do XPR

Relatório de 23/10/2026, elaborado a partir da análise do projeto e de uma tentativa anterior. O foco é definir o que devemos ter nesta implementação, quais decisões precisam ser fechadas e em que ordem avançar.

Este documento complementa o [plano de estudos](plano.md) e o [registro de progresso](PROGRESSO.md). A [especificação da linguagem](xpr-specs.md) e o [AGENTS.md](../AGENTS.md) continuam sendo as referências do projeto. Recomendações aqui registradas não alteram automaticamente esses contratos.

## 1. Direção do projeto

Manter a implementação atual como base de aprendizado. Aproveitar referências para compreender algoritmos, organizar responsabilidades e formular testes, com cada recurso implementado pelo aluno e validado contra a especificação.

Os principais objetivos são:

- Um parser de descida recursiva que produza a AST completa antes da execução.
- Uma representação de valores independente da representação sintática.
- Funções com capturas congeladas, parâmetros locais e recursão correta.
- AST em `tgc` e valores de execução em arenas hierárquicas.
- Diagnósticos consistentes, com tipo do erro e origem no fonte.
- Testes que verifiquem semântica, erros e duração dos valores.

## 2. Estado verificado

Na análise de 23/10/2026:

- Os 24 testes léxicos passaram usando o executável existente.
- A verificação com `gcc -fsyntax-only` encontrou `AST` indefinido e um parâmetro incompleto em `src/parser.c`.
- `src/diagnostic.c` estava vazio; `src/diagnostic.h` continha uma tabela global definida no próprio cabeçalho.
- O lexer já contava colunas em codepoints UTF-8 e aceitava strings multilinha, conforme a especificação.
- A estrutura da AST já previa comprimento explícito para strings, indexação, atribuição a alvos e `for`.
- O loader existia como infraestrutura inicial, ainda sem integração com um executor.
- O Makefile já tinha o alvo `test`, mas gerava arquivos de dependências `.d` sem incluí-los.

Não houve recompilação ou execução de ASan/Valgrind durante a análise. Os testes no binário existente não demonstram que os fontes atuais possam gerar esse mesmo executável. Esses registros descrevem a situação observada naquela data.

## 3. Arquitetura que devemos construir

| Componente | Responsabilidade |
|---|---|
| `lexer.c` | Delimitar tokens e preservar sua localização no fonte. |
| `parser.c` | Aplicar a gramática, validar a estrutura sintática e construir a AST com `tgc`. |
| `exec.c` | Avaliar a AST, coordenar escopos e chamadas e acionar a resolução dinâmica de funções. |
| `builtins.c` | Implementar funções nativas segundo os mesmos contratos de valores e chamadas. |
| `diagnostic.c` | Formatar e emitir os erros de todas as fases. |
| `loader.c` | Ler fontes e controlar seu carregamento, em integração com a resolução dinâmica. |
| Camada de valores | Representar, copiar, comparar e operar sobre valores; `value.c/.h` são nomes sugeridos. |
| Camada de arenas | Controlar a duração das alocações de execução; `arena.c/.h` são nomes sugeridos. |

### Parser

Organizar o parser em funções pequenas correspondentes aos níveis da EBNF. Centralizar o avanço de tokens, a exigência de delimitadores e a criação de nós.

Preservar as decisões da gramática atual:

- O programa contém uma expressão.
- Cada expressão dentro de um bloco exige `;`.
- A atribuição é associativa à direita.
- Igualdade e comparação relacional ocupam níveis distintos.
- `not` segue a precedência definida na EBNF, separada dos sinais aritméticos unários.
- Indexação, atribuição indexada, desestruturação e `for` fazem parte do escopo planejado.

Manter informações suficientes para identificar arquivo, linha e coluna de cada erro, inclusive em funções definidas em arquivos carregados posteriormente. A origem pode ser representada por um identificador de fonte compartilhado, sem duplicar o nome do arquivo em cada nó.

### Valores e operações

Usar uma união etiquetada para número, string, booleano, lista e função. Toda atribuição e passagem de parâmetro deve produzir uma cópia independente, incluindo listas aninhadas e capturas de funções.

Centralizar as operações sobre valores fora do fluxo principal do avaliador. A mesma regra deve atender escalares e descer recursivamente por pares de listas. Isso evita duplicar regras de tipos e operadores no executor.

Preservar comprimento explícito nas strings da AST e de execução: `\0` é conteúdo válido, e o comprimento em bytes precisa ser distinguido da quantidade de codepoints usada por `len`, `print` e indexação.

### Funções e escopos

Separar a declaração de uma variável local da atualização de uma variável existente. A entrada em uma função deve criar seus parâmetros no novo escopo, mesmo quando há nomes iguais nos escopos externos.

Resolver e congelar as variáveis livres na definição da função. Uma chamada posterior não deve buscar essas variáveis no ambiente do chamador.

Vincular a autorreferência à função criada. Se essa função for copiada e a variável original receber outra função, a cópia deve continuar recursando sobre a definição original.

Preservar também:

- Erro na definição quando uma variável livre não existe, exceto o nome pendente da própria função.
- Tipo da variável fixado na primeira atribuição.
- Chamada sem argumento equivalente à passagem de `[]`.
- Limite de profundidade de chamadas e diagnóstico `stack-overflow` conforme a especificação.

### Memória

Aplicar os dois tempos de vida definidos no `AGENTS.md`:

1. Nós, nomes, strings decodificadas e arrays da AST são alocados com `tgc` e permanecem disponíveis durante a execução.
2. Valores de execução pertencem às arenas dos escopos. O escopo proprietário da variável determina a arena de destino de uma atribuição.

Antes de liberar uma arena interna, copiar profundamente o resultado que precisa sobreviver para a arena de destino. A cópia de uma função inclui suas capturas.

Resolver antes das etapas dependentes:

- **AST compartilhada entre chamadas:** um endereço de variável guardado diretamente em um nó não identifica corretamente todos os frames recursivos. Avaliar descritores de resolução, como profundidade e índice, e garantir que nenhum endereço sobreviva à arena correspondente.
- **`for` e valores persistentes:** um `restore` não pode invalidar valores atribuídos a variáveis externas. A regra de arena determinada pelo destino precisa ser compatibilizada com a região restaurada, especialmente quando o destino está na própria arena da função.

As operações da arena não tornam a cópia profunda constante: copiar listas, strings e capturas continua tendo custo proporcional ao conteúdo copiado.

## 4. Infraestrutura que devemos completar

### Diagnósticos

Ter uma interface comum para erros léxicos, sintáticos e de execução, contendo código, mensagem, arquivo, linha e coluna. Exibir o trecho do fonte e um marcador de posição quando houver localização válida.

Contar colunas em codepoints, tratar a apresentação de tabulações e sanitizar caracteres de controle. Evitar um contexto global único que perca a origem de erros ao alternar entre arquivos carregados.

Manter definições de tabelas no arquivo de implementação e declarações no cabeçalho. Distinguir erro de avaliação, função inexistente e falha de carregamento para evitar mensagens duplicadas ou enganosas.

### Leitura e carregamento

Verificar os resultados de leitura e alocação. A leitura incremental até EOF é uma referência útil, especialmente para futura entrada via `stdin`, pois dispensa a exigência de um fluxo reposicionável.

Na integração do loader, distinguir arquivo já carregado, arquivo inexistente, falha de leitura e falha de parse. Implementar a busca na ordem prevista: diretório do script principal, `XPRPATH` e biblioteca padrão.

Após a primeira carga bem-sucedida, vincular a função ao escopo global e preservar seu fonte e sua AST. Respeitar o nome pendente para autorrecursão e informar erros com a origem do arquivo carregado.

### Builtins

Representar builtins como valores de função no ambiente, permitindo atribuição e passagem como argumento. Avaliar seus argumentos como valores; a chamada não deve depender de o argumento estar escrito como um literal na AST.

Implementar os contratos especificados, incluindo `int` truncando em direção a zero, operações de texto em codepoints, retorno de `print` e formatação por `fmt`. Centralizar a conversão de valores para texto para evitar divergências entre `print` e `string`.

### Construção e acompanhamento

- Corrigir os bloqueios de sintaxe antes de usar o binário existente como evidência de avanço dos fontes.
- Incluir as dependências `.d` geradas pelo Makefile.
- Acrescentar uma configuração de diagnóstico de memória nas etapas apropriadas.
- Manter modos separados para inspecionar tokens e, futuramente, a AST.
- Atualizar o registro de progresso quando cada pendência for resolvida.

## 5. Decisões de linguagem a fechar

Antes de transformar exemplos em testes normativos:

1. Resolver a presença de `%` no lexer, já que o operador não está especificado.
2. Remover a duplicação de `int_num` e revisar a produção de números iniciados por ponto para exigir dígitos.
3. Explicitar a convenção de argumentos de `cat` e `at`, conciliando os exemplos com a regra de um argumento por função.
4. Revisar exemplos e resultados esperados de formatação numérica para que correspondam às regras de `fmt`.
5. Definir a localização efetiva da biblioteca padrão e sua relação com o executável.

Recursos ausentes do contrato, como `nil`, `in`, `<>`, multiplicação preguiçosa por zero e mudança livre de tipo, devem permanecer fora do escopo até uma decisão explícita de linguagem. O bloco vazio deve continuar produzindo `[]`.

## 6. Testes que devemos ter

Preservar os testes léxicos e acrescentar casos sintáticos e de execução conforme as etapas. Ao adaptar um cenário, derivar seu resultado esperado da especificação atual.

| Área | Critério de aceitação |
|---|---|
| Programa e blocos | `3` e `{3;}` são válidos; `3;` e `{3}` são erros; `{}` produz `[]`. |
| Precedência | Aritmética, comparação, igualdade e `not` seguem os níveis da EBNF. |
| Avaliação preguiçosa | `and`, `or` e `iif` avaliam apenas os operandos ou ramos necessários. |
| Capturas | Uma função definida com `c = 9` continua usando esse valor depois de `c` mudar. |
| Parâmetros | Um parâmetro chamado `x` não altera uma variável externa chamada `x`. |
| Recursão | Fatorial e Fibonacci produzem os resultados esperados; `fib(10)` vale `55`. |
| Identidade da função | Copiar uma função recursiva e redefinir a variável original preserva a recursão da cópia. |
| Tipos | Reatribuir string a uma variável numérica produz `type-mismatch`. |
| Cópias | Alterar uma lista copiada ou passada como argumento não modifica a original. |
| Listas aninhadas | `[[1],[2]] + [[3],[4]]` produz `[[4],[6]]`. |
| Indexação | Leitura e escrita válidas funcionam; índices fora dos limites produzem o erro previsto. |
| Strings | `len("olá")` e `len("a\0b")` valem `3`; strings multilinha são aceitas. |
| Conversões | `int(-3.7)` vale `-3`; demais arredondamentos seguem sua tabela. |
| Builtins | Funções nativas podem ser copiadas e passadas como valores. |
| Escopos e arenas | Resultados sobrevivem ao fim do escopo; atribuições persistentes sobrevivem ao término do laço. |
| Carregamento | Busca, carga única, autorrecursão e origem dos erros obedecem ao contrato. |
| Diagnósticos | Comentário sem fechamento é rejeitado; erro esperado tem código e posição corretos. |

O executor de testes deve verificar saída padrão, saída de erro e código de retorno. Uma falha qualquer não comprova o erro esperado. Testes positivos devem falhar quando o programa termina com erro, mesmo que parte da saída corresponda ao esperado.

Verificações de memória devem incluir listas vazias, expansão de capacidade, listas aninhadas, saídas por erro, funções retornadas e valores que atravessam limites de escopo.

## 7. Ordem recomendada

1. Fechar as ambiguidades da especificação e registrar exemplos normativos.
2. Corrigir o esqueleto do parser, completar diagnósticos e regularizar as dependências de construção.
3. Preservar a suíte léxica em um modo próprio de inspeção.
4. Implementar AST, valores numéricos e avaliação aritmética de ponta a ponta.
5. Acrescentar programa, blocos, variáveis e regras de tipos, conforme o plano de estudos.
6. Consolidar arenas, cópias e operações sobre valores antes de avançar para os recursos que dependem deles.
7. Implementar funções e validar capturas, parâmetros, recursão e autorreferência.
8. Completar listas, indexação, desestruturação, `for` e builtins.
9. Integrar carregamento dinâmico e localização de erros entre arquivos.
10. Validar os exemplos da especificação, os caminhos de erro e a memória; registrar os resultados de cada etapa.

Cada etapa deve permanecer pequena o suficiente para que o aluno explique as decisões, implemente o recurso e demonstre seu comportamento com testes. Este relatório orienta o trabalho futuro e não implementa componentes do compilador ou interpretador.

## 8. Adequação dos testes após a revisão

Foram preparados 21 testes positivos e 22 negativos em `tests/runtime/`, com
resultados esperados derivados do contrato deste projeto. A organização, os
casos pendentes e as regras de diagnóstico estão em [tests/README.md](../tests/README.md).

Decisões confirmadas pelo autor:

- `print` não acrescenta newline. Cada quebra desejada deve estar explícita no
  argumento; o retorno continua sendo a quantidade de codepoints emitidos.
- `{ {z = 5;}; print(z); }` deve produzir `undefined-identifier`. Os terminadores
  internos são obrigatórios e o bloco que constitui o arquivo não recebe `;`.
- Em um unário sobre literal numérico, o sinal deve estar colado ao número:
  `2 *- 3` é inválido e `2 * -3` é válido.

Os 24 resultados léxicos anteriores foram auditados e preservados. Um novo caso
léxico registra as posições dos sinais e números; os testes negativos registram
a rejeição que o parser ainda deverá implementar. O lexer atual preserva os
trechos e as posições, mas não rejeita sozinho o unário separado.

A suíte de execução é um conjunto de critérios de aceitação futuros. Sua criação
não significa que o interpretador já execute esses recursos. Nenhum arquivo de
`src/` foi modificado durante essa adequação.
