#!/bin/bash

# --- CONFIG ---
DEFAULT_TEST_PATH="./test_files"
DEFAULT_PATTERNS_PATH="./patterns.txt"
TEST_MODE=("random" "words")
TEST_SIZE=("small" "medium" "large")

run_tests()
{
    TEST_FUNCTION=$1
    TEST_PATTERNS=${2:-$DEFAULT_PATTERNS_PATH}
    TEST_FILES=${3:-$DEFAULT_TEST_PATH}

    #File input tests
    for MODE in "${TEST_MODE[@]}"; do
        echo "Starting: $MODE tests!"
        for SIZE in "${TEST_SIZE[@]}"; do
            COUNT=0
            TOTAL=$(find "$TEST_FILES/$MODE/$SIZE" -type f | wc -l)
            for FILE in "$TEST_FILES/$MODE/$SIZE"/*; do
                [ -f "$FILE" ] || continue
                ((COUNT++))

                printf "\rChecking: %-60s [%d/%d]" "$FILE" "$COUNT" "$TOTAL"
                
                while IFS= read -r PATTERN; do
                    "$TEST_FUNCTION" "$PATTERN" "$FILE"
                done < "$TEST_PATTERNS"
            done
            echo -e "\n$SIZE tests passed!"
        done
        echo "$MODE tests passed!"
    done

    #Expansion tests
    for MODE in "${TEST_MODE[@]}"; do
        for SIZE in "${TEST_SIZE[@]}"; do
            FILES="$TEST_FILES/$MODE/$SIZE/*"
            echo "Checking: $MODE/$SIZE/*"
            while IFS= read -r PATTERN; do
                "$TEST_FUNCTION" "$PATTERN" $FILES
            done < "$TEST_PATTERNS"
        done
        echo "$MODE test finished!"
    done
}

string_search_test()
{
    mkdir -p ./temp

    #EFS test
    EFS_RESULT="./temp/efs_print.txt"
    GREP_RESULT="./temp/grep_print.txt"
    ./efs -n -- "$1" "$2" > "$EFS_RESULT"
    grep -nFH -- "$1" "$2" > "$GREP_RESULT"

    if ! diff -q "$EFS_RESULT" "$GREP_RESULT" >/dev/null; then
        echo "Error invalid line match $1 in $2" >&2
        diff "$EFS_RESULT" "$GREP_RESULT" >&2
        exit 1
    fi

    #EFS -c flag test
    EFS_COUNT=$(./efs -c -- "$1" "$2")
    GREP_COUNT=$(grep -FHc -- "$1" "$2")

    if [ $EFS_COUNT != $GREP_COUNT ]; then
        echo "Error invalid full count $1 in $2" >&2
        echo "Grep: $GREP_COUNT" >&2
        echo "EFS: $EFS_COUNT" >&2
    fi

    # EFS -l flag test
    EFS_RESULT="./temp/efs_list.txt"
    GREP_RESULT="./temp/grep_list.txt"
    ./efs -l -- "$1" "$2" > "$EFS_RESULT"
    grep -Fl -- "$1" "$2" > "$GREP_RESULT"

    if ! diff -q "$EFS_RESULT" "$GREP_RESULT" >/dev/null; then
        echo "Error invalid list match in $1" >&2
        diff "$EFS_RESULT" "$GREP_RESULT" >&2
        exit 1
    fi

    #EFS -q flag test
    ./efs -q -- "$1" "$2"
    EFS_STATUS=$?

    grep -Fq -- "$1" "$2"
    GREP_STATUS=$?

    if [ $EFS_STATUS -ne $GREP_STATUS ]; then
        echo "Invalid quiet return $1, in $2" >&2
        exit 1
    fi

    # EFS_WORD_RESULT=...
    # GREP_WORD_RESULT="./temp/test_w_str_temp.txt"
    # grep -Fw "$1" "$2" > "$GREP_WORD_RESULT"

    # RESULT_DIFF=$(diff $EFS_WORD_RESULT $GREP_WORD_RESULT)

    # if [ $RESULT_DIFF ]; then
    #     echo "Error invalid lines matched in $2" >&2
    #     echo $RESULT_DIFF >&2
    #     exit 1
    # fi

    # EFS_WORD_COUNT=...
    # GREP_WORD_COUNT=$(grep -Fcw "$1" "$2")
    # echo $GREP_WORD_COUNT

    # if [ $EFS_WORD_COUNT != $GREP_WORD_COUNT ]; then
    #     echo "Error invalid pattern count in $2" >&2
    #     echo "Grep: $GREP_WORD_COUNT" >&2
    #     echo "EFS: $EFS_WORD_COUNT" >&2
    #     exit 1
    # fi
}

case "$1" in
    all)
        echo "Running all tests!"
        ;;
    string)
        echo "Running string search tests!"
        run_tests "string_search_test" $2 $3
        ;;
    regex)
        echo "Running regex matching tests!"
        ;;
    *)
        echo "Invalid arg given! Try --help for more information"
        ;;
esac


