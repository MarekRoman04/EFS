#!/bin/bash

# --- CONFIG ---
DEFAULT_TEST_PATH="./test_files"
DEFAULT_PATTERN_PATH="./patterns.txt"
DEFAULT_TEST_MODE=("random" "words")
DEFAULT_TEST_SIZE=("small" "medium" "large")
DEFAULT_FILE_INPUT_MODE=("single" "multiple")
DEFAULT_OUTPUT_PATH="./temp"
FLAGS="qlcnviw"
INVALID_COMBINATIONS=("-qv" "-lv")

help()
{
    echo "Bash script comparing EFS and grep results on given patterns"
    echo "Supports test file and pattern generation using python scripts"
    echo "Usage: $0 -[OPTION..] -- [EFS_FLAG_STRING...]"
    echo "Options:"
    echo "-a, --all             Runs all tests"
    echo "-h, --help            Display help message"
    echo "-o, --output          Sets ouput path on test failure"
    echo "-f, --files           Sets test file path"
    echo "-p, --pattern         Sets test pattern file path"
    echo "-m, --mode            Sets test mode "random,words", default: all modes"
    echo "-i, --input-mode      Sets input mode for files (single, multiple), default: all modes"
    echo "-s, --size            Sets file size to test on "small,mediun,large", default: all sizes"
    echo "    --no-print        Disables test run without flags"
    echo "EFS flag string:"
    echo "Sets flags used in efs tests, default: no flag"
    echo "When used with -a, --all flag string may contain only -v, -i, -w,"
    echo "only first flag string is used, other are ignored!,"
    echo "-q and -l tests run only with -i, -w flags"
}

run_tests()
{
    FLAGS=$1

    for FILE_MODE in ${FILE_INPUT_MODE[@]}; do
        case "$FILE_MODE" in
            single) single_file_tests $FLAGS ;;
            multiple) multiple_file_tests $FLAGS ;;
            *) echo "Invalid file mode: $FILE_MODE" ;;
        esac
    done
}

single_file_tests()
{
    FLAGS=$1

    for MODE in "${TEST_MODE[@]}"; do
        for SIZE in "${TEST_SIZE[@]}"; do
            COUNT=0
            TOTAL=$(find "$TEST_PATH/$MODE/$SIZE" -type f | wc -l)
            for FILE in "$TEST_PATH/$MODE/$SIZE"/*; do
                [ -f "$FILE" ] || continue
                ((COUNT++))
                printf "\rChecking: %-50s [%d/%d]" "$FILE" "$COUNT" "$TOTAL"
                while IFS= read -r PATTERN; do
                    string_search_test "$FLAGS" "$PATTERN" "$FILE"
                done < "$PATTERN_PATH"
            done
            echo "Single file tests: $FLAGS passed!"
        done
    done
}

multiple_file_tests()
{
    FLAGS=$1

    for MODE in "${TEST_MODE[@]}"; do
        for SIZE in "${TEST_SIZE[@]}"; do
            FILES=("$TEST_PATH/$MODE/$SIZE"/*)
            [ -e "${FILES[0]}" ] || continue
            printf "Checking: %-55s " "$MODE/$SIZE/*"
            while IFS= read -r PATTERN; do
                string_search_test "$FLAGS" "$PATTERN" "${FILES[@]}"
            done < "$PATTERN_PATH"
            echo "Multiple file tests: $FLAGS passed!"
        done
    done
}


string_search_test()
{
    FLAGS="$1"
    shift
    PATTERN="$1"
    shift
    FILES=("$@")

    EFS_RESULT="/dev/shm/efs${FLAGS}_${TMP_ID}.txt"
    GREP_RESULT="/dev/shm/grep${FLAGS}_${TMP_ID}.txt"
    ./efs $FLAGS -- "$PATTERN" ${FILES[@]} > "$EFS_RESULT"
    grep $FLAGS -FH -- "$PATTERN" ${FILES[@]} > "$GREP_RESULT"

    if ! diff -q "$EFS_RESULT" "$GREP_RESULT" >/dev/null; then
        echo "Error invalid results with flags: ${FLAGS}, pattern: '$PATTERN' in file: ${FILES[*]}" >&2
        diff "$EFS_RESULT" "$GREP_RESULT" >&2
        echo "Program results saved to ouput location"
        cp $EFS_RESULT ./efs.txt
        cp $GREP_RESULT ./grep.txt
        exit 1
    fi
}

run_all_tests()
{
    CLI_FLAGS="${1:1}"

    if [[ $CLI_FLAGS != *v* ]]; then
        #Quiet search test
        run_tests -q$CLI_FLAGS
        #List search test
        run_tests -l$CLI_FLAGS
    fi
    #Count search test
    run_tests -c$CLI_FLAGS
    #Line number search test
    run_tests -n$CLI_FLAGS
    #Print search test
    run_tests

    exit 0
}

RUN_ALL=false
PRINT=true
OUTPUT_PATH=""
TEST_PATH=""
PATTERN_PATH=""
TEST_MODE=""
FILE_INPUT_MODE=""
TEST_SIZE=""
EFS_FLAGS=()

#Parse options
TEMP=$(getopt -o aho:f:p:m:i:s: --long all,help,output:,files:,pattern:,mode:,input-mode:,size:,no-print -n "$0" -- "$@")
if [ $? != 0 ]; then
    echo "Error parsing arguments" >&2
    exit 1
fi
eval set -- "$TEMP"

#Process options
while true; do
    case "$1" in
        -a|--all) RUN_ALL=true; shift ;;
        -h|--help) help; exit 0 ;;
        -o|--output) OUTPUT_PATH="$2"; shift 2 ;;
        -f|--files) TEST_PATH="$2"; shift 2 ;;
        -p|--pattern) PATTERN_PATH="$2"; shift 2 ;;
        -m|--mode) TEST_MODE="$2"; shift 2 ;;
        -i|--input-mode) FILE_INPUT_MODE="$2"; shift 2 ;;
        -s|--size) TEST_SIZE="$2"; shift 2 ;;
        --no-print) PRINT=false; shift ;;
        --) shift; break ;;  # end of options, start of EFS flags
        *) echo "Unknown option: $1"; exit 1 ;;
    esac
done

EFS_FLAG_STRINGS=("$@")

#Default values if no value given
: "${TEST_PATH:=$DEFAULT_TEST_PATH}"
: "${PATTERN_PATH:=$DEFAULT_PATTERN_PATH}"
: "${OUTPUT_PATH:=$DEFAULT_OUTPUT_PATH}"

#Default values for arrays, or values for all option
if [ -z "$TEST_MODE" ]; then
    TEST_MODE=("${DEFAULT_TEST_MODE[@]}")
else
    IFS=',' read -ra TEST_MODE <<< "$TEST_MODE"
fi

if [ -z "$FILE_INPUT_MODE" ]; then
    FILE_INPUT_MODE=("${DEFAULT_FILE_INPUT_MODE[@]}")
else
    IFS=',' read -ra FILE_INPUT_MODE <<< "$FILE_INPUT_MODE"
fi

if [ -z "$TEST_SIZE" ]; then
    TEST_SIZE=("${DEFAULT_TEST_SIZE[@]}")
else
    IFS=',' read -ra TEST_SIZE <<< "$TEST_SIZE"
fi

#Runs EFS tests
if $RUN_ALL; then
    run_all_tests ${EFS_FLAG_STRINGS[0]}
fi

if $PRINT; then
    run_tests    
fi

#Runs tests for given valid flag combinations
for FLAG in "${EFS_FLAG_STRINGS[@]}"; do
    INVALID=false
    for INVALID_FLAG in "${INVALID_COMBINATIONS[@]}"; do
        if [[ "$FLAG" == "$INVALID_FLAG" ]]; then
            INVALID=true
            break
        fi
    done

    if $INVALID; then
        echo "Skipping invalid flag combination: $FLAG"
    else
        run_tests "$FLAG"
    fi
done

exit 0