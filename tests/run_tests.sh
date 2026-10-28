#!/bin/bash
# /*
#    FILE: run_tests.sh
#    DESC: Run XPR lexer, parser and runtime contract tests using bash and coreutils only.
#    AUTHOR: Andre Cavalcante with Sonnet 5.5 partner
#    DATE: October, 2026
#    LICENSE: GPL-v3
# */

set -u
export LC_ALL=C.UTF-8

ROOT=$(cd -- "$(dirname -- "$0")/.." && pwd)
TESTS=$ROOT/tests
RUNTIME=$TESTS/runtime
BINARY=${XPR:-$ROOT/xpr}
SUITE=lexer
FILTER=
LIST=0
TIMEOUT=5
LEXER_ARGS=()
CASES=

die() {
    printf 'error: %s\n' "$*" >&2
    exit 2
}

usage() {
    cat <<'EOF'
usage: run_tests.sh [--suite lexer|parser|runtime|all] [--binary PATH] [--case STEM]
                    [--lexer-arg ARG]... [--list] [--timeout SECONDS]
EOF
}

while [ $# -gt 0 ]; do
    case $1 in
        --suite)       [ $# -ge 2 ] || die "--suite requires a value"; SUITE=$2; shift 2 ;;
        --suite=*)     SUITE=${1#*=}; shift ;;
        --binary)      [ $# -ge 2 ] || die "--binary requires a value"; BINARY=$2; shift 2 ;;
        --binary=*)    BINARY=${1#*=}; shift ;;
        --case)        [ $# -ge 2 ] || die "--case requires a value"; FILTER=$2; shift 2 ;;
        --case=*)      FILTER=${1#*=}; shift ;;
        --lexer-arg)   [ $# -ge 2 ] || die "--lexer-arg requires a value"; LEXER_ARGS+=("$2"); shift 2 ;;
        --lexer-arg=*) LEXER_ARGS+=("${1#*=}"); shift ;;
        --list)        LIST=1; shift ;;
        --timeout)     [ $# -ge 2 ] || die "--timeout requires a value"; TIMEOUT=$2; shift 2 ;;
        --timeout=*)   TIMEOUT=${1#*=}; shift ;;
        -h|--help)     usage; exit 0 ;;
        *)             usage >&2; die "unknown argument: $1" ;;
    esac
done

case $SUITE in
    lexer|parser|runtime|all) ;;
    *) die "invalid --suite '$SUITE' (expected lexer, parser, runtime or all)" ;;
esac

awk -v t="$TIMEOUT" 'BEGIN { exit !(t > 0) }' || die "--timeout must be positive"

cd -- "$ROOT" || die "cannot enter $ROOT"

# ---- case collection ---------------------------------------------------------

add_case() {
    # $1 suite, $2 source relative to ROOT, $3 expected relative or empty,
    # $4 negative spec "file:line:column:code" or empty for positive cases
    CASES+="$1|$2|$3|$4"$'\n'
}

collect_lexer() {
    local src exp
    for src in "$TESTS"/lexer/*.x; do
        [ -e "$src" ] || die "no lexer fixtures in tests/lexer/"
        exp=${src%.x}.expected
        [ -f "$exp" ] || die "missing expected output for $(basename -- "$src")"
        add_case lexer "tests/lexer/$(basename -- "$src")" "tests/lexer/$(basename -- "$exp")" ""
    done
}

collect_parser() {
    # Reserved for --ast dumps and pure-syntax fixtures; empty for now.
    local src exp
    for src in "$TESTS"/parser/*.x; do
        [ -e "$src" ] || return 0
        exp=${src%.x}.expected
        [ -f "$exp" ] || die "missing expected output for $(basename -- "$src")"
        add_case parser "tests/parser/$(basename -- "$src")" "tests/parser/$(basename -- "$exp")" ""
    done
}

collect_runtime() {
    local src exp
    for src in "$RUNTIME"/cases/*.x; do
        [ -e "$src" ] || die "no runtime cases in tests/runtime/cases/"
        exp=$RUNTIME/expected/$(basename -- "${src%.x}").expected
        [ -f "$exp" ] || die "missing expected output for $(basename -- "$src")"
        add_case runtime "tests/runtime/cases/$(basename -- "$src")" \
                         "tests/runtime/expected/$(basename -- "$exp")" ""
    done
    collect_runtime_errors
}

collect_runtime_errors() {
    [ -f "$RUNTIME/errors.toml" ] || die "missing tests/runtime/errors.toml"

    # The manifest is a trivial TOML subset ([[case]] tables with scalar keys),
    # so a line-oriented awk reader is enough.
    local records file line column code src total_lines target width
    records=$(awk '
        function flush() { if (file != "") print file "|" line "|" column "|" error }
        /^[[:space:]]*\[\[case\]\][[:space:]]*$/ { flush(); file = ""; line = ""; column = ""; error = ""; next }
        /^[[:space:]]*file[[:space:]]*=/   { v = $0; sub(/^[^=]*=[[:space:]]*/, "", v); gsub(/["[:space:]]/, "", v); file = v; next }
        /^[[:space:]]*error[[:space:]]*=/  { v = $0; sub(/^[^=]*=[[:space:]]*/, "", v); gsub(/["[:space:]]/, "", v); error = v; next }
        /^[[:space:]]*line[[:space:]]*=/   { v = $0; sub(/^[^=]*=[[:space:]]*/, "", v); gsub(/[^0-9]/, "", v); line = v; next }
        /^[[:space:]]*column[[:space:]]*=/ { v = $0; sub(/^[^=]*=[[:space:]]*/, "", v); gsub(/[^0-9]/, "", v); column = v; next }
        END { flush() }
    ' "$RUNTIME/errors.toml") || die "failed to parse errors.toml"
    [ -n "$records" ] || die "errors.toml contains no cases"

    # Manifest entries and negative sources must match exactly: no dups, no orphans.
    if printf '%s\n' "$records" | awk -F'|' '{print $1}' | sort | uniq -d | grep -q .; then
        die "negative sources and manifest entries must match exactly"
    fi
    if ! diff <(printf '%s\n' "$records" | awk -F'|' '{print $1}' | sort -u) \
              <(cd "$RUNTIME/errors" && ls -1 -- *.x | sort -u) >/dev/null; then
        die "negative sources and manifest entries must match exactly"
    fi

    while IFS='|' read -r file line column code; do
        [ -n "$file" ] || continue
        [[ "$line" =~ ^[0-9]+$ ]] || die "invalid diagnostic line for $file"
        [[ "$column" =~ ^[0-9]+$ ]] || die "invalid diagnostic column for $file"
        src=$RUNTIME/errors/$file
        [ -f "$src" ] || die "missing negative source $file"
        total_lines=$(awk 'END { print NR }' "$src")
        [ "$line" -ge 1 ] && [ "$line" -le "$total_lines" ] || die "invalid diagnostic line for $file"
        target=$(awk -v n="$line" 'NR == n' "$src")
        width=$(( $(printf '%s' "$target" | wc -m) ))
        [ "$column" -ge 1 ] && [ "$column" -le $((width + 1)) ] || die "invalid diagnostic column for $file"
        add_case runtime "tests/runtime/errors/$file" "" "$file:$line:$column:$code"
    done <<EOF
$records
EOF
}

case $SUITE in
    lexer)   collect_lexer ;;
    parser)  collect_parser ;;
    runtime) collect_runtime ;;
    all)     collect_lexer
             collect_parser
             collect_runtime ;;
esac

[ "$LIST" -eq 1 ] || [ -x "$BINARY" ] || die "test binary not found or not executable: $BINARY"

TMP=$(mktemp -d) || die "mktemp failed"
trap 'rm -rf "$TMP"' EXIT

CODES_RE='\[(syntax-error|wrong-assignment|empty-program|size-mismatch|type-mismatch|undefined-identifier|div-by-zero|not-a-function|stack-overflow|index-out-of-range)\]'

total=0
failed=0

while IFS='|' read -r suite src_rel exp_rel spec; do
    [ -n "$suite" ] || continue
    stem=$(basename -- "$src_rel" .x)
    if [ -n "$FILTER" ] && [ "$stem" != "$FILTER" ]; then continue; fi
    total=$((total + 1))

    if [ "$LIST" -eq 1 ]; then
        kind="exact stdout"
        [ -n "$spec" ] && kind=${spec##*:}
        printf '%s: %s (%s)\n' "$suite" "$src_rel" "$kind"
        continue
    fi

    status=0
    if [ "$suite" = lexer ]; then
        timeout "$TIMEOUT" "$BINARY" ${LEXER_ARGS[@]+"${LEXER_ARGS[@]}"} "$ROOT/$src_rel" \
            >"$TMP/out" 2>"$TMP/err" </dev/null || status=$?
    else
        timeout "$TIMEOUT" "$BINARY" "$ROOT/$src_rel" \
            >"$TMP/out" 2>"$TMP/err" </dev/null || status=$?
    fi

    problems=()
    if [ -z "$spec" ]; then
        # Positive contract: byte-exact stdout, exit 0, silent stderr.
        if ! cmp -s "$TMP/out" "$ROOT/$exp_rel"; then
            problems+=("stdout mismatch (expected vs got):")
        fi
        if [ "$status" -eq 124 ]; then
            problems+=("timeout after ${TIMEOUT}s")
        elif [ "$status" -ne 0 ]; then
            problems+=("expected exit 0, got $status")
        fi
        if [ -s "$TMP/err" ]; then
            problems+=("unexpected stderr:")
        fi
    else
        # Negative contract: ordinary error exit and one located diagnostic.
        IFS=':' read -r efile eline ecolumn ecode <<<"$spec"
        if [ "$status" -eq 124 ]; then
            problems+=("timeout after ${TIMEOUT}s")
        fi
        if [ "$status" -lt 1 ] || [ "$status" -gt 127 ]; then
            problems+=("expected an ordinary error exit (1..127), got $status")
        fi
        found=$(grep -oE "$CODES_RE" "$TMP/err" || true)
        count=$(printf '%s\n' "$found" | grep -c .)
        if [ "$count" -ne 1 ] || [ "$found" != "[$ecode]" ]; then
            problems+=("expected one [$ecode] diagnostic, got: $(printf '%s' "$found" | tr '\n' ' ')")
        fi
        loc_name=${efile//./\\.}
        loc_re="(^|[[:space:]/]|\\\\)$loc_name:$eline:$ecolumn:"
        if ! grep -E "$loc_re" "$TMP/err" | grep -qF "[$ecode]"; then
            problems+=("expected $efile:$eline:$ecolumn: and [$ecode] on the same diagnostic line")
        fi
    fi

    if [ "${#problems[@]}" -gt 0 ]; then
        failed=$((failed + 1))
        printf 'FAIL %s/%s\n' "$suite" "$stem"
        for problem in "${problems[@]}"; do
            printf '  %s\n' "$problem"
        done
        if [ "${problems[0]}" = "stdout mismatch (expected vs got):" ]; then
            diff -u --label expected --label got "$ROOT/$exp_rel" "$TMP/out" 2>/dev/null | sed -n '3,17s/^/  /p'
        fi
        if [ -s "$TMP/err" ]; then
            printf '  stderr:\n'
            sed 's/^/  | /' "$TMP/err" | head -n 10
        fi
    else
        printf 'PASS %s/%s\n' "$suite" "$stem"
    fi
done <<<"$CASES"

[ "$total" -gt 0 ] || die "no matching tests"

if [ "$LIST" -eq 1 ]; then
    printf 'Total: %d\n' "$total"
    exit 0
fi

printf 'Total: %d; passed: %d; failed: %d\n' "$total" $((total - failed)) "$failed"
[ "$failed" -eq 0 ] || exit 1
exit 0
