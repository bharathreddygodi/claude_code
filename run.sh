#!/bin/sh
# Compile and run solutions.
#   sh run.sh                 -> every folder
#   sh run.sh bits            -> one folder
#   sh run.sh bits BIT-05     -> one problem
cd "$(dirname "$0")" || exit 1
dirs=${1:-"memory bits strings pointers linked_lists rtos interrupts drivers concurrency timers"}
for d in $dirs; do
    [ -d "$d" ] || continue
    mkdir -p "$d/bin"
    for f in "$d"/${2:-}*.c; do
        [ -f "$f" ] || continue
        name=$(basename "$f" .c)
        echo "===================== $d/$name ====================="
        gcc -std=c11 -Wall -Wextra -O2 -pthread "$f" -o "$d/bin/$name.exe" -lm && "./$d/bin/$name.exe"
        echo
    done
done
