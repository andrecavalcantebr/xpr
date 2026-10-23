# Diário de progresso: xpr

Atualizado ao fim de cada sessão e sempre que uma etapa for concluída. Plano em `@docs/plano.md`, spec em `@docs/xpr-specs.md`.

## Situação atual

- **Etapa atual:** 0 (Infraestrutura e lexer), em andamento.
- **Feito:** spec documentada; lexer (`lexer.c`); `loader`; `tgc` em `vendor/`; 25 casos léxicos; 21 casos positivos e 22 negativos de execução preparados como contratos futuros; executor de testes com comparação exata e validação de diagnósticos.
- **Em andamento pelo aluno:** loader e driver; transição do loader para `LoadedSource`, organização das opções da CLI e início do prompt. Ler as anotações pessoais ao final deste documento.
- **Pendente na etapa 0:** concluir e compatibilizar as alterações de `main.c`, `loader.c` e `loader.h`; completar `diagnostic.h/.c`; resolver `AST` indefinido em `parser.c`; incluir dependências `.d`; ASan no Makefile. O parâmetro incompleto do parser já foi removido pelo aluno. O alvo `make test` já existe. A suíte de execução ainda depende da implementação do parser e avaliador.
- **Próximo foco informado por André:** estudar carga e recursão; desenvolver prompt, stdin por pipe e `-x`; organizar pastas e Makefile; depois compilar e testar as opções do driver. Não há nova validação de construção dessas alterações nesta revisão.
- **Horas acumuladas:** não medidas.

## Decisões de linguagem

Estão em `docs/xpr-specs.md`. Resumo dos pontos centrais:

| Ponto | Decisão |
|---|---|
| Execução | Duas fases: o arquivo inteiro vira AST, depois é executado |
| Tipos | número (`double`), string UTF-8, booleano, lista, função |
| Função | `fn(x){...}`, 1 argumento (lista para vários), captura congelada na definição |
| Valores | Sem referências: toda atribuição e passagem de parâmetro copia |
| Memória | AST em `tgc`; valores em arenas hierárquicas (ver `AGENTS.md`) |
| Listas | Operações elemento a elemento, recursivas; sem broadcast de escalar |
| Erros | Tipo + arquivo + linha + coluna (codepoints); 3 categorias de parse e 7 categorias de execução na tabela da spec |
| Saída | `print` não acrescenta newline; `\n` deve ser explícito |
| Terminadores | Statements dentro de blocos exigem `;`; a expressão que constitui o arquivo termina sem `;` |
| Unário numérico | Sinal adjacente ao literal: `2 * -3` válido; `2 *- 3` inválido |

## Dúvidas em aberto

Memória (já levantadas, sem decisão):

1. **Ponteiros de variável guardados na AST × recursão.** Um `NODE_VAR` serve a todos os frames ativos. Alternativa a avaliar: o parser guarda (profundidade de escopo, índice) e o frame resolve em tempo de execução. Decidir antes da etapa 5.
2. **`for` com `mark/restore` × variáveis externas.** Em `s = cat(s, [i])` dentro do `for`, o `restore` libera o valor atribuído à variável de fora, que fica pendurada. Lembrar: o destino decide a arena (`AGENTS.md`). Decidir antes da etapa 8.

Spec (inconsistências encontradas na análise):

3. `int_num` está definido duas vezes na EBNF; revisar também a exigência de dígitos na produção de números iniciados por ponto.
4. Conciliar os exemplos de `cat(a,b)` e `at(lista,i)` com a regra de um argumento por função.
5. O lexer tem `TOKEN_PERCENT`, mas a spec não define `%`. Remover o token ou especificar o operador.
6. Definir os contratos de conversão de `bool`, a disponibilidade de `sqrt` e os detalhes de `fmt`, incluindo zero. Os casos correspondentes ainda não integram os testes ativos quando dependem dessas decisões.
7. A busca padrão `(bindir)/../base` (spec 4.1) ainda não corresponde a nenhum diretório.

Driver e integração, para estudo a partir das anotações pessoais:

8. Distinguir prompt de stdin por pipe quando não houver argumentos e definir o comportamento ao chegar a EOF.
9. Separar análise sintática, controle de carregamento e profundidade de chamadas: parsear todo o fonte antes de executar detecta erros sintáticos, mas não garante o término de uma recursão. O avaliador deve aplicar o limite de chamadas da spec; o loader/resolvedor controla cargas e nomes pendentes.
10. Confirmar a convenção da CLI sobre o valor final do script. Os testes atuais de execução pressupõem somente a saída explícita de `print`.

## Registro de sessões

### 2026-10-23: análise e plano
- **Feito:** comparação com o projeto `claude/xpr`; `plano.md` e `PROGRESSO.md` criados a partir dele; README e Makefile corrigidos.
- **Horas gastas:** (preencher)
- **Testes:** os 24 léxicos existentes.
- **Próximo passo:** o aluno corrige `diagnostic.h/.c` e `parser.c` para o projeto voltar a compilar, e decide a dúvida 5 (`%`).

### 2026-10-23: adequação dos testes
- **Decisões confirmadas:** `print` sem newline automático; terminadores internos obrigatórios e ausência de `;` após o bloco do programa; sinal unário adjacente ao literal numérico.
- **Feito:** revisão dos 24 `.expected` léxicos anteriores; caso adicional de posições de sinais; adaptação dos cenários de execução; novos negativos com código, arquivo, linha e coluna esperados. Detalhes em `tests/README.md`.
- **Separação das suítes:** `bash tests/run_tests.sh` usa o binário existente para os casos léxicos. `--suite runtime` exige o futuro avaliador; `--suite runtime --list` verifica somente o catálogo.
- **Limite da alteração:** fontes do compilador e interpretador preservados. A regra de adjacência ainda precisa ser implementada pelo aluno no reconhecimento do unário.
- **Validação:** 25/25 testes léxicos passaram com comparação exata; os 21 fontes positivos novos chegaram ao EOF sem erro léxico. O executor de testes passou por 15 verificações de saída, status e diagnósticos simulados. Isso não valida a execução semântica dos 43 contratos futuros.
- **Pendências de testes:** decisão sobre `%`, contratos de conversão de `bool`, disponibilidade de `sqrt` e formatação de zero. A suíte assume saída explícita por `print`, sem eco automático do resultado do arquivo.


### 2026-10-23: revisão do progresso e memória local
- **Lido:** registro pessoal de André sobre loader, driver, recursão, prompt, stdin, `-x`, organização de arquivos e Makefile. A seção pessoal foi preservada sem alterações.
- **Atualizado:** trabalho em andamento e próximos focos; removida a pendência já resolvida do parâmetro incompleto do parser. As alterações do aluno ainda contêm trechos incompletos e interfaces em transição; não foram compiladas ou corrigidas nesta revisão.
- **Ajustado:** número de categorias de erro e lista de dúvidas da spec. Não há referência a `sin`/`range` na versão atual; um nome da linguagem como `exp` também não implica, por si só, colisão com um símbolo C da libc.
- **Continuidade:** criado `docs/MEMORIA.md` e referenciado no `AGENTS.md`, incluindo a regra de ler e preservar as anotações pessoais. Trata-se de memória local do projeto.

## Registro do André 23/10/2026

- Desenvolvendo o loader e o driver
  - cuidado aqui porque o parser pode chamar recursivamente o loader
  - se já tiver carregado, simplemeste executa
  - problema: pode haver recursão infinita na execução (quem pega? penso que o parser que deve parsear toda a expressão antes da execução - estudar o problema e a solução)
- Desenvolver o modo prompt
- Desenvolver o modo stdin: `cat script.x | xpr` (verificar se não conflita com o modo prompt - acho que não)
- Desenvolver o modo exec: `xpr -x "2 + 5"`
- Mudar os arquivos para as respectivas pastas
- Alterar o Makefile para usar as respectivas pastas
- Compilar e testar as opções do driver
