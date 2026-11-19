#!/bin/sh
set -e
ROOT="$1"; GRAMMAR="$2"; TOKENS="$3"; AST="$4"; INPUT="$5"
"$ROOT/build/bin/lexer" -g "$GRAMMAR" -t "$TOKENS" "$INPUT"
"$ROOT/build/bin/parser" -g "$GRAMMAR" -a "$AST" "$TOKENS"