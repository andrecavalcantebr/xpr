#!/usr/bin/env sh

# Emits the compilation databases that clangd (Zed, VSCode, vim) reads to
# resolve the `#include`s and, through them, the types.
#

set -eu
root=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
cd "$root"

STD=c2x
sep=""

entry() {  # $1 = relative file, $2... = flags
  file=$1; shift
  printf '%s  {"directory":"%s","file":"%s","command":"cc -std=%s %s -c %s"}\n' \
         "$sep" "$root" "$file" "$STD" "$*" "$file"
  sep=","
}

{
  printf '[\n'

  for c in $(find vendor/tgc -name '*.c' 2>/dev/null | sort); do
    entry "$c" "-I vendor/tgc -I vendor/tgc"
  done

  for c in $(find src -name '*.c' 2>/dev/null | sort); do
    entry "$c" "-I src"
  done

  for tree in vendor vendor; do
    [ -d "$tree" ] || continue
    for h in $(find "$tree" -name '*.h' | sort); do
      entry "$h" "-I $tree"
    done
  done

  printf ']\n'
} > compile_commands.json

echo "compile_commands.json: $(grep -c '"file"' compile_commands.json) entries"
[ -x golden/gen-ccjson.sh ] && ./golden/gen-ccjson.sh
