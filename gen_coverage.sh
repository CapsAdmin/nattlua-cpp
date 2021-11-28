#!/bin/bash

rm -rf coverage
mkdir coverage
pushd build/CMakeFiles/tests.dir/tests/
    lcov --capture --directory ../../../../ --output-file ../../../../coverage/lcov_output.info
popd
genhtml coverage/lcov_output.info --output-directory coverage/