#!/bin/bash

TEST_PATH="./test_files/"
OUTPUT_PATH="./temp"
FAILED_PATTERNS_FILE=""
FAILED_OUTPUTS_FILE=""
SAVE_FAILED_OUTPUTS=false
RUN_ALL=false
RUN_DEFAULT=true
EFS_FLAG_STRINGS=()

FLAG_STRINGS=(
    "-q" "-l" "-c" "-n"
    "-qi" "-qw" "-li" "-lw"
    "-ci" "-cw" "-cv"
    "-ni" "-nw" "-nv"
    "-i" "-w" "-v"
    "-iw" "-iv" "-wv" "-ivw"
)

print_common_help() {
    cat <<EOF
Usage:
  SCRIPT [OPTIONS] -- [FLAGS]

Options:
  -a, --all            Run all predefined flag combinations
  -f, --files PATH     Root directory with test files (default: ./test_files)
  -o, --output PATH    Directory for failed tests (default: ./temp)
      --no-print       Disable progress output
  -h, --help           Show this help message
  -n, --no-print       Disable default search test with no flags

Flags:
  Flags after '--' are passed to efs/grep.

Examples:
  SCRIPT -a
  SCRIPT -- -iw
EOF
}

parse_args() {
    local SCRIPT_NAME="$1"
    shift

    local TEMP
    TEMP=$(getopt -o ahno:f: --long all,help,output:,files:,no-print -n "$SCRIPT_NAME" -- "$@") || exit 1
    eval set -- "$TEMP"

    while true; do
        case "$1" in
            -a|--all) RUN_ALL=true; shift ;;
            -h|--help) help; exit 0 ;;
            -o|--output)
                OUTPUT_PATH="$2"
                shift 2
                ;;
            -f|--files)
                TEST_PATH="$2"
                shift 2
                ;;
            -n|--no-print) RUN_DEFAULT=false; shift ;;
            --) shift; break ;;
            *) echo "Unknown option: $1"; exit 1 ;;
        esac
    done

    EFS_FLAG_STRINGS=("$@")

    FAILED_PATTERNS_FILE="$OUTPUT_PATH/failed_patterns.txt"
    FAILED_OUTPUTS_FILE="$OUTPUT_PATH/failed_results.txt"

    mkdir -p "$OUTPUT_PATH"
    rm -f "$FAILED_PATTERNS_FILE" "$FAILED_OUTPUTS_FILE"
}

is_valid_flag() {
    local FLAG="$1"
    for VALID in "${FLAG_STRINGS[@]}"; do
        [[ "$FLAG" == "$VALID" ]] && return 0
    done
    return 1
}

run_flag() {
    local FLAG="$1"
    local FAILURE_FILE="$OUTPUT_PATH/job_${JOB_ID}.failed"

    ((JOB_ID++))

    echo -e "=== Running tests with flags: $FLAG ==="

    FAILED_PATTERNS_FILE="$FAILURE_FILE" \
        "$RUNNER" "$FLAG" &

    PIDS+=("$!")

    while (( $(jobs -rp | wc -l) >= MAX_JOBS )); do
        wait -n
    done
}

run_flag_set() {
    local RUNNER="$1"
    local MAX_JOBS=8
    local JOB_ID=0
    local -a PIDS=()

    if $RUN_DEFAULT; then
        echo -e "=== Running default test ==="
        "$RUNNER" &
        PIDS+=("$!")
    fi

    if $RUN_ALL; then
        for FLAG in "${FLAG_STRINGS[@]}"; do
            run_flag "$FLAG"
        done
    fi

    for FLAG in "${EFS_FLAG_STRINGS[@]}"; do
        if ! is_valid_flag "$FLAG"; then
            echo "Skipping invalid flag combination: $FLAG"
            continue
        fi

        run_flag "$FLAG"
    done

    local STATUS=0

    for PID in "${PIDS[@]}"; do
        wait "$PID" || STATUS=1
    done

    # Merge all per-flag failure files
    : > "$FAILED_PATTERNS_FILE"

    for FILE in "$OUTPUT_PATH"/job_*.failed; do
        [ -f "$FILE" ] || continue
        cat "$FILE" >> "$FAILED_PATTERNS_FILE"
        rm "$FILE"
    done

    if [ ! -s "$FAILED_PATTERNS_FILE" ]; then
        rm "$FAILED_PATTERNS_FILE"
        echo "All tests passed!"
    fi

    return "$STATUS"
}
