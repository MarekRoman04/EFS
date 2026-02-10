#!/bin/bash
source "$(dirname "$0")/test.sh"

help() {
    cat <<EOF
Pattern file test mode (-f)
--------------------------
Each *_patterns.txt file is passed using -f.

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

        printf "\rChecking $FLAGS: %-40s [%d/%d]" "$FILE" "$COUNT" "$TOTAL"
        run_test "$FLAGS" "$PATTERN_FILE" "$FILE"
    done
}

run_test() {
    local FLAGS="$1"
    local PATTERN_FILE="$2"
    local FILE="$3"
    local TMP_ID=$RANDOM

    ./efs $FLAGS -f -- "$PATTERN_FILE" "$FILE" > /dev/shm/efs_$TMP_ID
    EFS_RV=$?

    LC_ALL=C grep $FLAGS -FH -f "$PATTERN_FILE" -- "$FILE" > /dev/shm/grep_$TMP_ID
    GREP_RV=$?

    if [ "$EFS_RV" -ne "$GREP_RV" ]; then
        ((FAIL_COUNT++))
        echo "$FILE | $FLAGS | INVALID RV: EFS=$EFS_RV GREP=$GREP_RV" >> "$FAILED_PATTERNS_FILE"
    elif ! diff -q /dev/shm/efs_$TMP_ID /dev/shm/grep_$TMP_ID >/dev/null; then
        ((FAIL_COUNT++))
        echo "$FILE | $FLAGS | INVALID OUTPUT" >> "$FAILED_PATTERNS_FILE"
    fi

    rm -f /dev/shm/efs_$TMP_ID /dev/shm/grep_$TMP_ID
}

parse_args "$0" "$@"
run_flag_set run_tests
print_summary
