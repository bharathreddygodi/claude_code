#!/bin/sh
# Compile and run every .c file in this folder. Usage: sh build_and_run.sh [MEM-05 | leak]
cd "$(dirname "$0")" || exit 1
mkdir -p bin
for f in ${1:-}*.c; do
    [ -f "$f" ] || continue
    exe="bin/${f%.c}.exe"
    echo "===================== $f ====================="
    gcc -std=c11 -Wall -Wextra -O2 "$f" -o "$exe" && "./$exe"
    echo
done
