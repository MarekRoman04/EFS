#!/bin/bash
set -euo pipefail

usage() {
    cat <<'EOF'
Usage:
    traversal_subtest.sh <depth> <dirs_per_level> <files_per_dir>
EOF
    exit 1
}

[[ $# -eq 3 ]] || usage
DEPTH="$1"
DIRS="$2"
FILES="$3"

[[ "$DEPTH" =~ ^[0-9]+$ ]] && (( DEPTH >= 1 )) || { echo "depth must be >= 1" >&2; exit 2; }
[[ "$DIRS"  =~ ^[0-9]+$ ]] && (( DIRS  >= 1 )) || { echo "dirs_per_level must be >= 1" >&2; exit 2; }
[[ "$FILES" =~ ^[0-9]+$ ]] && (( FILES >= 0 )) || { echo "files_per_dir must be >= 0" >&2; exit 2; }

ROOT="$(mktemp -d "/dev/shm/efs-traversal.XXXXXX")"

PHASE="gen"
TEST_FAILED=0
PATTERN="TEST"

TMP_ID="$RANDOM"
EFS_RESULT="/dev/shm/efs_${TMP_ID}.txt"
GREP_RESULT="/dev/shm/grep_${TMP_ID}.txt"

cleanup() {
    if [[ "$PHASE" == "test" ]] && (( TEST_FAILED != 0 )); then
        mkdir -p -- "./temp"
        cp -a -- "$ROOT" "./temp/"
        echo "[CLEANUP] Saved failing tree to ./temp/$(basename "$ROOT")" >&2
    fi

    rm -rf -- "$ROOT"
    rm -f -- "$EFS_RESULT" "$GREP_RESULT"
}

trap cleanup EXIT INT TERM

echo "[GEN] Generating directory tree" >&2
PARENTS=("$ROOT")

for ((LEVEL=1; LEVEL<=DEPTH; LEVEL++)); do
    NEW_PARENTS=()

    for PARENT in "${PARENTS[@]}"; do
        for ((I=1; I<=DIRS; I++)); do
        CHILD="$PARENT/dir_${LEVEL}_${I}"
        mkdir -p -- "$CHILD"

        for ((F=1; F<=FILES; F++)); do
            printf '%s\n' "$PATTERN" > "$CHILD/file_${LEVEL}_${I}_${F}.txt"
        done

        NEW_PARENTS+=("$CHILD")
        done
    done

    PARENTS=("${NEW_PARENTS[@]}")
done

PHASE="test"
echo "[TEST] Starting traversal test" >&2


./efs -r -- "$PATTERN" "$ROOT" | sort > "$EFS_RESULT"
grep -r -FH -- "$PATTERN" "$ROOT" | sort > "$GREP_RESULT"

if ! diff -q "$EFS_RESULT" "$GREP_RESULT" >/dev/null; then
    TEST_FAILED=1
    mkdir -p -- "./temp"
    cp -f -- "$EFS_RESULT" "./temp/efs.txt"
    cp -f -- "$GREP_RESULT" "./temp/grep.txt"
    echo "[FAIL] Traversal output differs" >&2
    exit 1
fi

echo "[PASS] Traversal outputs match" >&2