#!/bin/bash
source "$(dirname "$0")/test.sh"

help() {
    cat <<EOF
Single pattern test mode
-----------------------
Each line in *_patterns.txt is treated as a separate pattern.

EOF
    print_common_help
}

run_tests() {
    local FLAGS="$1"
    local FILES
    IFS=$'\n' read -d '' -r -a FILES < <(
        find "$TEST_PATH" -type f ! -name "*_patterns.txt" && printf '\0'
    )

    local COUNT=0
    local TOTAL=${#FILES[@]}

    for FILE in "${FILES[@]}"; do
        ((COUNT++))

        local PATTERN_FILE="${FILE%.*}_patterns.txt"
        [ -f "$PATTERN_FILE" ] || continue

        printf "\rChecking $FLAGS: %-60s [%d/%d]" "$FILE" "$COUNT" "$TOTAL"

        while IFS= read -r PATTERN; do
            run_test "$FLAGS" "$PATTERN" "$FILE"
        done < "$PATTERN_FILE"
    done
}


run_test() {
    local FLAGS="$1"
    local PATTERN="$2"
    local FILE="$3"
    local TMP_ID=$RANDOM

    ./efs $FLAGS -- "$PATTERN" "$FILE" > /dev/shm/efs_$TMP_ID
    grep $FLAGS -FH -- "$PATTERN" "$FILE" > /dev/shm/grep_$TMP_ID

    if ! diff -q /dev/shm/efs_$TMP_ID /dev/shm/grep_$TMP_ID >/dev/null; then
        ((FAIL_COUNT++))
        echo "$PATTERN | $FILE | $FLAGS" >> "$FAILED_PATTERNS_FILE"
    fi

    rm -f /dev/shm/efs_$TMP_ID /dev/shm/grep_$TMP_ID
}

parse_args "$0" "$@"
run_flag_set run_tests
print_summary
