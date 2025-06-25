#!/bin/bash

check_args()
{
    ARGS=$1
    EXPECTED_ARGS=$2
    if [ $ARGS != $EXPECTED_ARGS ]; then
            echo "Expected $EXPECTED_ARGS argument" >&2
            exit 1
    fi
}

string_search_test()
{
    check_args $# 2
    echo "Checking $2"
    EFS_RESULT=$(grep "$1" $2)
    GREP_RESULT=$(grep "$1" $2)

    # RESULT_DIFF=$(diff $EFS_RESULT $GREP_RESULT)

    # if [ $RESULT_DIFF ]; then
    #     echo "Error invalid lines matched" >&2
    #     echo $RESULT_DIFF >&2
    #     exit 1
    # fi

    EFS_COUNT=$(grep -c "$1" $2)
    GREP_COUNT=$(grep -c "$1" $2)

    if [ $EFS_COUNT != $GREP_COUNT ]; then
        echo "Error invalid pattern count" >&2
        echo "Grep: $GREP_COUNT" >&2
        echo "TEST: $EFS_COUNT" >&2
        exit 1
    else
        echo "$2: $GREP_COUNT"
    fi
}

TEST_PATH="output_tests"
TEST_MODE=("binary" "text_random" "text_words")
TEST_SIZE=("small" "medium" "large")


echo "Running Tests..."

for MODE in ${TEST_MODE[@]}; do
    if [ $MODE != "binary" ]; then 
        for SIZE in ${TEST_SIZE[@]}; do
        FILE_PATH=$TEST_PATH/$MODE/$SIZE
            for FILE in $FILE_PATH/*; do
                string_search_test "unwilling" "$FILE"
            done
        done
    fi
    echo "$MODE passed!"
done

