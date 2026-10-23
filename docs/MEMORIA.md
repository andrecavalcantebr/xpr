/*
   FILE: MEMORIA.md
   DESC: Preserve project decisions and collaboration context across sessions.
   AUTHOR: Andre Cavalcante with Sonnet 5.5 partner
   DATE: October, 2026
   LICENSE: GPL-v3
*/

# Memória de trabalho do XPR

Atualizada em 23/10/2026. Registro local de continuidade; não pressupõe memória
automática fora do repositório. Ler junto com [PROGRESSO.md](PROGRESSO.md),
[AGENTS.md](../AGENTS.md) e a [especificação](xpr-specs.md).

## Forma de colaboração

- O projeto é um exercício de aprendizado de compiladores em C. André escreve
  o compilador e o interpretador; a IA explica, revisa e lista os passos.
- Ferramentas de apoio, documentação e adequações dos testes foram autorizadas.
  Isso não autoriza implementar ou corrigir o compilador pelo aluno.
- Ler as seções pessoais `Registro do André` no progresso, sem modificá-las.
  Preservar também alterações em andamento nos fontes.
- Responder com clareza, em português; código e comentários novos em inglês,
  seguindo o cabeçalho e as demais regras do `AGENTS.md`.

## Decisões confirmadas pelo autor

- `print(x)` não acrescenta `\n`. A quebra precisa estar explícita na string;
  `println` é somente uma possibilidade futura.
- `{ {z = 5;}; print(z); }` deve falhar por `undefined-identifier`. Statements
  internos exigem `;`; o bloco que constitui o arquivo termina sem `;`.
- Sinal unário sobre literal numérico deve estar colado ao número:
  `2 *- 3` é erro e `2 * -3` é válido. A subtração binária aceita espaços.
- O contrato de memória continua sendo AST em `tgc` e valores em arenas
  hierárquicas. O destino decide a arena; atribuições e parâmetros copiam valores.
- Funções congelam variáveis livres, parâmetros sombreiam nomes externos e
  cópias de funções recursivas preservam a autorreferência original.

## Hipóteses e pendências que não devem virar decisões implícitas

- `-x` e `-(...)` foram preservados conforme a gramática existente; não houve
  confirmação adicional do autor sobre restringir esses operandos.
- A suíte de execução pressupõe saída explícita por `print`, sem eco automático
  do valor final do arquivo. Confirmar essa convenção ao fechar o driver.
- O formato `arquivo:linha:coluna: ... [codigo] ...` e as âncoras de localização
  são contratos propostos pela suíte, descritos em `tests/README.md`.
- Permanecem decisões sobre `%`, assinatura de `cat`/`at`, conversões de `bool`,
  disponibilidade de `sqrt`, formatação numérica de zero e biblioteca padrão.
- Antes de funções e laços, resolver endereços de variáveis na AST sob recursão
  e valores persistentes que não podem ser invalidados por `restore` no `for`.

## Estado dos testes

- 25 testes léxicos: os 24 originais foram auditados e preservados; foi adicionado
  `unary_spacing`, que documenta as posições dos sinais e números.
- Última execução registrada: 25/25 passaram no binário existente. Isso não
  comprova a construção dos fontes atuais, que estão sendo alterados pelo aluno.
- `tests/runtime/` tem 21 positivos e 22 negativos preparados para o futuro
  avaliador. Não foram aprovados semanticamente; os positivos apenas passaram
  pela tokenização e o catálogo foi conferido.
- O executor de testes compara bytes exatos e rejeita erro de código ou posição
  diferente, saída indevida, status de sucesso em negativo, timeout e crash.
- Comandos: `bash tests/run_tests.sh` para o lexer;
  `bash tests/run_tests.sh --suite runtime --list` para conferir o catálogo;
  `--suite runtime` para executar os contratos quando o avaliador estiver pronto.

## Foco atual de André

Segundo suas anotações de 23/10/2026, André está trabalhando no loader e no driver.
Quer estudar a interação com carregamentos recursivos, reaproveitar arquivos já
carregados, desenvolver prompt, entrada por pipe (`cat script.x | xpr`) e `-x`,
organizar arquivos/pastas, ajustar o Makefile e então compilar e testar o driver.

A leitura dos fontes confirmou alterações em andamento em `main.c`, `loader.c`
e `loader.h`, incluindo a transição para `LoadedSource`. Não registrar essas
etapas como concluídas nem substituir o trabalho do aluno.

Para a dúvida sobre recursão, distinguir três assuntos ao orientar:

1. O parser verifica a sintaxe completa antes de executar cada fonte.
2. O carregamento precisa controlar o estado dos arquivos e respeitar os nomes
   pendentes e as regras de resolução da linguagem.
3. O avaliador controla a profundidade das chamadas e emite `stack-overflow`.
   Analisar a sintaxe antes não demonstra que uma execução recursiva terminará.

## Referências de continuidade

- [Progresso e anotações pessoais](PROGRESSO.md).
- [Plano de estudos](plano.md).
- [Diretrizes de evolução](diretrizes-de-evolucao.md).
- [Organização, contratos e pendências dos testes](../tests/README.md).

A tentativa comparada está em `/home/andre/code/cprojects/qwen3.8/xpr`.
Foi usada como referência para cenários e organização; a especificação deste
projeto é a referência para os resultados esperados.
