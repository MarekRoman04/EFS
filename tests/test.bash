#!/bin/bash

# --- CONFIG ---
DEFAULT_TEST_PATH="./test_files"
DEFAULT_PATTERNS_PATH="./patterns.txt"
TEST_MODE=("random" "words")
TEST_SIZE=("small" "medium")

run_tests()
{
    TEST_FUNCTION=$1
    TEST_PATTERNS=${2:-$DEFAULT_PATTERNS_PATH}
    TEST_FILES=${3:-$DEFAULT_TEST_PATH}

    for MODE in "${TEST_MODE[@]}"; do
        for SIZE in "${TEST_SIZE[@]}"; do
            for FILE in "$TEST_FILES/$MODE"/$SIZE/*; do
                echo "Checking: $FILE"
                [ -f "$FILE" ] || continue
                while IFS= read -r PATTERN; do
                    "$TEST_FUNCTION" "$PATTERN" "$FILE"
                done < "$TEST_PATTERNS"
            done
        done
        echo "$MODE file test finished!"
    done
}

string_search_test()
{
    mkdir -p ./temp
    # EFS_RESULT=...
    # GREP_RESULT="./temp/test_str_temp.txt"
    # grep -F "$1" "$2" > "$GREP_RESULT"

    # if ! diff -q "$EFS_RESULT" "$GREP_RESULT" >/dev/null; then
    #     echo "Error invalid line match in $FILE" >&2
    #     diff "$EFS_RESULT" "$GREP_RESULT" >&2
    #     exit 1
    # fi

    # EFS_COUNT=...
    # GREP_COUNT=$(grep -Fc "$1" "$2")
    # echo $GREP_COUNT

    # if [ $EFS_COUNT != $GREP_COUNT ]; then
    #     echo "Error invalid pattern count in $2" >&2
    #     echo "Grep: $GREP_COUNT" >&2
    #     echo "EFS: $EFS_COUNT" >&2
    #     exit 1
    # else
    #     echo "$2: $GREP_COUNT"
    # fi

    EFS_FULL_COUNT=$(./efs -c "$1" "$2")
    GREP_FULL_COUNT=$(grep -Fo "$1" "$2" | wc -l)

    if [ $EFS_FULL_COUNT != $GREP_FULL_COUNT ]; then
        echo "Error invalid full count $1 in $2" >&2
        echo "Grep | wc: $GREP_FULL_COUNT" >&2
        echo "EFS: $EFS_FULL_COUNT" >&2
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

    # EFS_WORD_FULL_COUNT=...
    # GREP_WORD_FULL_COUNT=$(grep -Fow "$1" "$2" | wc -l)
    # echo $GREP_WORD_FULL_COUNT

    # if [ $EFS_WORD_FULL_COUNT != $GREP_WORD_FULL_COUNT ]; then
    #     echo "Error invalid full pattern count in $2" >&2
    #     echo "Grep | wc: $GREP_WORD_FULL_COUNT" >&2
    #     echo "EFS: $EFS_WORD_FULL_COUNT" >&2
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


