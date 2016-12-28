make clean
make
./life 100 < f0.l | sort > f100.l
lcov -t 'Life' -o life_test.info -c -d .
genhtml -o lcov_results life_test.info
