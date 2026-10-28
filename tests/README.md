/*
   FILE: README.md
   DESC: Explain lexer fixtures, runtime contracts and diagnostic expectations.
   AUTHOR: Andre Cavalcante with Sonnet 5.5 partner
   DATE: October, 2026
   LICENSE: GPL-v3
*/

# Testes do XPR

Os arquivos em `lexer/` verificam o despejo de tokens. `parser/` é reservada
para os futuros despejos da árvore (`--ast`) e casos de sintaxe pura. Os
arquivos em `runtime/` descrevem o comportamento desejado do futuro
interpretador. O executável atual ainda não executa essa última suíte
corretamente.

## Executar

Requisito do executor de testes: bash, awk e coreutils, sem pacotes externos.

```sh
bash tests/run_tests.sh
bash tests/run_tests.sh --suite runtime --list
bash tests/run_tests.sh --suite runtime
bash tests/run_tests.sh --suite runtime --case e05_escopo_vaza
bash tests/run_tests.sh --suite all
```

Esses comandos não compilam o projeto. `make test` continua dependendo da
construção do executável; o esqueleto atual do parser ainda impede essa construção.
`--binary /caminho/xpr` ou a variável `XPR` permitem selecionar outro executável.
Quando existir um modo léxico explícito, usar `--lexer-arg=--lex`. A suíte `parser`
ainda não tem casos: `--suite parser` devolve `no matching tests` até a etapa 1
do plano popular `parser/`.

`--list` verifica o catálogo e os arquivos esperados, sem comprovar a sintaxe
ou a semântica dos programas XPR. O limite padrão é cinco segundos por caso;
`--timeout` permite ajustá-lo. Timeout, falha ao iniciar e encerramento por sinal
são falhas do teste, nunca erros de linguagem aceitos.

## Saídas exatas

`stdout` é comparado byte a byte, incluindo espaços, UTF-8 e quebras de linha.
O executor não remove espaços finais nem acrescenta ou elimina `\n`.
Casos positivos também exigem status zero e `stderr` vazio.

Cada `.expected` contém apenas os bytes da saída esperada. Esses arquivos de
dados não possuem cabeçalho ou comentários, pois seriam confundidos com saída.

Na suíte de execução, cada separador de linha é emitido explicitamente por
`print("\n")`. Os casos `16_print_sem_newline` e `17_print_retorno` terminam
intencionalmente sem newline no `.expected`, exercitando o contrato confirmado.
Os valores numéricos usam o `fmt` padrão `[2, 15]`; listas simples são impressas
com espaços entre elementos e strings sem aspas.

A suíte pressupõe que a execução de um arquivo escreva somente a saída explícita
de `print`, sem eco automático do valor final do script. Essa é uma convenção
do teste para a futura CLI; eventual modo de eco deve ser separado.

## Testes negativos

`runtime/errors.toml` define o código e a posição esperados para cada arquivo
de `runtime/errors/`. Todos esses casos exigem `stdout` vazio, inclusive o caso
que contém um `print` antes de um erro sintático em uma função não chamada:
o arquivo inteiro precisa ser analisado antes de começar a execução.

O contrato de diagnóstico adotado pela suíte é:

```text
arquivo.x:linha:coluna: ... [codigo-do-erro] ...
```

O caminho pode ser absoluto ou relativo. Código e posição devem estar na mesma
linha; texto explicativo e contexto visual não são fixados. Deve existir um único
código de erro reconhecido. Uma falha de sintaxe não satisfaz, por exemplo, um
teste que espera `undefined-identifier`. O status deve ser um erro normal entre
1 e 127; sinais e status convencionais de crash são rejeitados.

As posições contam codepoints e incluem o cabeçalho de cada fonte. Para tornar
os casos determinísticos, o catálogo adota as seguintes âncoras:

- Operador responsável, para divisão por zero e incompatibilidade de listas.
- Identificador, para variável indefinida, chamada de não função e reatribuição
  incompatível; `[` da indexação inválida, para índice fora dos limites.
- Sinal unário, quando há separação do literal numérico.
- Abertura da string ou comentário sem fechamento.
- Token inesperado, para os demais erros sintáticos.

Essas âncoras e a apresentação dos códigos são contratos da nova suíte a seguir
ao implementar os diagnósticos; o compilador não foi alterado para produzi-los.
Ao editar fontes negativos, revisar também linha e coluna no catálogo.

O caso `e05_escopo_vaza` contém exatamente a expressão
`{ {z = 5;}; print(z); }`: os dois statements internos têm terminador, enquanto
o bloco externo termina o programa sem `;`. O erro esperado é a leitura de `z`
fora do seu escopo, nunca um erro de ponto e vírgula.

## Auditoria dos testes léxicos

Na reorganização de 23/10/2026, os fixtures da raiz foram movidos para `lexer/`
com `git mv`, sem nenhuma alteração de conteúdo; a auditoria abaixo se aplica
aos arquivos dessa pasta.

Os 24 `.expected` preexistentes foram comparados byte a byte com o executável
existente e preservados. Eles são expectativas de tokenização, não provas de
validade sintática: por isso podem conter múltiplos argumentos, `%` ou sequências
que o futuro parser rejeitará.

Foi acrescentado `unary_spacing.x`, com posições esperadas revisadas para:

| Fonte | Coluna de `-` | Coluna de `3` | Decisão futura do parser |
|---|---:|---:|---|
| `2 *- 3` | 4 | 6 | Erro: sinal unário separado do número. |
| `2 * -3` | 5 | 6 | Válido. |
| `2 *-3` | 4 | 5 | Válido. |
| `- 3` | 1 | 3 | Erro: sinal unário separado do número. |
| `2 - 3` | 3 | 5 | Válido: subtração binária. |

O lexer atual produz `MINUS` e `NUMBER` separados e preserva os trechos no fonte.
O parser poderá comparar o fim do trecho do sinal com o começo do número ao
reconhecer um uso unário. Não basta rejeitar toda ocorrência de `MINUS` seguida
de espaço, porque isso também rejeitaria subtrações válidas.

Os testes léxicos que contêm `TOKEN_ERROR` ainda esperam status zero do despejo
atual. Isso não se aplica aos testes negativos do interpretador.

## Adaptações e pendências

- `01` a `15` preservam os cenários aproveitáveis da análise anterior, com bloco
  externo, terminadores internos e resultados revisados.
- `<>` foi trocado por `!=`; os casos de `in` foram retirados.
- `07_listas_valores` substitui operações fora do contrato por cópia independente,
  indexação e reduções booleanas. Não afirma equivalência entre essas operações
  e os antigos builtins `push`, `pop`, `head` e `tail`.
- `fn()` foi substituído por `fn(unused)`; a chamada `constant()` foi preservada.
- `0 * (1/0)` passou para os negativos, com `div-by-zero`.
- `int(-3.7)` agora espera `-3.00`. Conversões ambíguas de `bool` e chamadas de
  `sqrt` ficaram fora até que seus contratos sejam definidos.
- O antigo caso `e8` de `%` permanece pendente: o token existe, mas o operador
  ainda não está especificado. Não há um teste ativo com resultado inventado.
- Casos que precisam provar resultado numérico zero usam comparação booleana,
  pois a representação textual do zero nas regras de `fmt` ainda merece decisão.
- A sintaxe existente de `-x` e `-(...)` foi mantida; a nova restrição explícita
  trata da adjacência entre sinal e literal numérico.

Os casos adicionais cobrem ausência de newline implícito, retorno de `print`,
UTF-8, NUL embutido, sinais unários, capturas, sombreamento de parâmetros,
autorreferência e listas aninhadas. O catálogo negativo também verifica que a
análise sintática antecede a execução e que colunas UTF-8 contam codepoints.
