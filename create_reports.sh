#!/bin/sh

lcov_report() {
    TITLE=$1

    echo "lcov report ($TITLE) ..."
    lcov -t $TITLE -o measurements/lcov/life_test.info -c -d .
    genhtml -o measurements/lcov/html measurements/lcov/life_test.info
}

perf_report() {
    GENERATIONS=$1
    FILE=$2

    echo "perf report for $FILE with $GENERATIONS generations ..."
    (perf stat -e cycles ./life $GENERATIONS < $FILE | sort > /dev/null) > measurements/perf/${FILE}_${GENERATIONS}.log 2>&1
}

#### Main
echo "Cleaning up previous builds ..."
make clean
echo "Building ..."
make life

FILES=*.l
for f in $FILES
do
    perf_report $1 $f
done

echo "Generating lcov report ..."
lcov_report Life
echo "Done."

echo "Cleaning up ..."
make clean
