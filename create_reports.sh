#!/bin/bash

lcov_report() {
    TITLE=$1
    TAG=$2

    printf "lcov report ($TITLE) ...\n"
    mkdir measurements/lcov/${TAG}
    lcov -t $TITLE -o measurements/lcov/${TAG}/life_test.info -c -d .
    genhtml -o measurements/lcov/${TAG}/html measurements/lcov/${TAG}/life_test.info
}

perf_report() {
    GENERATIONS=$1
    FILE=$2
    TAG=$3

    printf "perf report for $FILE with $GENERATIONS generations ...\n"
    mkdir measurements/perf/${TAG}
    (perf stat -e cycles:u -e cycles:k -e instructions:u -e branch-misses -e L1-dcache-load-misses:u -e L1-dcache-store-misses:u -e LLC-loads:u -e LLC-stores:u -e LLC-load-misses:u -e LLC-store-misses:u ./life $GENERATIONS < $FILE | sort > /dev/null) > measurements/perf/${TAG}/${FILE}_${GENERATIONS}.log 2>&1
}

oprofile_report() {
    GENERATIONS=$1
    FILE=$2
    TAG=$3

    printf "oprofile report for $FILE with $GENERATIONS generations ...\n"
    mkdir measurements/oprofile/${TAG}
    mkdir measurements/oprofile/${TAG}/${FILE}_${GENERATIONS}
    operf --session-dir ./measurements/oprofile/${TAG}/${FILE}_${GENERATIONS} ./life $GENERATIONS < $FILE | sort > /dev/null
    opreport --session-dir ./measurements/oprofile/${TAG}/${FILE}_${GENERATIONS} > ./measurements/oprofile/${TAG}/${FILE}_${GENERATIONS}/report.log
}

reports() {
    GENERATIONS=$1
    TAG=$2

    FILES=*.l
    for f in $FILES
    do
        perf_report $GENERATIONS $f $TAG
        oprofile_report $GENERATIONS $f $TAG
    done
}

print_heading() {
    TEXT="$1"
    LENGTH=$((${#TEXT} + 2))
    BORDER_TOP_BOTTOM="-"
    BORDER_LEFT_RIGHT="|"

    printf -vch "%${LENGTH}s" ""
    printf "+%s+\n" "${ch// /${BORDER_TOP_BOTTOM}}"

    printf "${BORDER_LEFT_RIGHT} %s ${BORDER_LEFT_RIGHT}\n" "$TEXT"

    printf "+%s+\n" "${ch// /${BORDER_TOP_BOTTOM}}"
}

#### Main
print_heading "Report Generation"

cd ./effprog

TAGS=`git tag`
for tag in $TAGS
do
    print_heading "Generating reports for $tag ..."

    # switch to tag
    git checkout -b ${tag}_report $tag

    # clean up and prepare for coverage
    printf "Cleaning up previous builds ...\n"
    make clean
    printf "Building ...\n"
    make coverage

    # generate reports

    reports $1 $tag
    lcov_report $tag $tag

    # merge reports into master and delete report branch
    git checkout master
    git merge ${tag}_report --strategy-option theirs
    git branch -d ${tag}_report
    exit
done

print_heading "Cleaning up"
make clean

exit
