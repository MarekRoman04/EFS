#!/bin/bash

TEST_PATH="./test_files"
OUTPUT_PATH="./temp"
FAILED_PATTERNS_FILE=""
FAILED_OUTPUTS_FILE=""
SAVE_FAILED_OUTPUTS=false
RUN_ALL=false
PRINT=true
FAIL_COUNT=0
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
  --no-print           Disable default search test with no flags

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
    TEMP=$(getopt -o aho:f: --long all,help,output:,files:,no-print -n "$SCRIPT_NAME" -- "$@") || exit 1
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
            --no-print) PRINT=false; shift ;;
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

run_flag_set() {
    local RUNNER="$1"

    if $PRINT; then
        "$RUNNER"
        echo
    fi

    if $RUN_ALL; then
        for FLAG in "${FLAG_STRINGS[@]}"; do
            echo -e "\n=== Running tests with flags: $FLAG ==="
            "$RUNNER" "$FLAG"
        done
    fi

    for FLAG in "${EFS_FLAG_STRINGS[@]}"; do
        is_valid_flag "$FLAG" || {
            echo "Skipping invalid flag combination: $FLAG"
            continue
        }
        "$RUNNER" "$FLAG"
        echo
    done
}

print_summary() {
    echo
    if (( FAIL_COUNT > 0 )); then
        echo "$FAIL_COUNT test(s) failed."
        echo "Failed patterns: $FAILED_PATTERNS_FILE"
        $SAVE_FAILED_OUTPUTS && echo "Failed outputs: $FAILED_OUTPUTS_FILE"
        exit 1
    else
        echo "All tests passed!"
        [ -f "$FAILED_PATTERNS_FILE" ] || rmdir "$OUTPUT_PATH" 2>/dev/null
        exit 0
    fi
}
