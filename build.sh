#!/bin/bash

set -e

curDir="$(pwd)"
codeDir="$curDir/src"
buildDir="$curDir/gebouw"

flags="-O0 -g -ggdb -Wall -Werror -pedantic -std=c++11"

exceptions="-Wno-unused-function -Wno-writable-strings -Wno-gnu-zero-variadic-macro-arguments -Wno-gnu-anonymous-struct -Wno-nested-anon-types -Wno-missing-braces -Wno-vla-extension"

mkdir -p "$buildDir"

echo Building "core-math"

pushd "$buildDir" > /dev/null
    clang++ $flags $exceptions "$codeDir/xfloat_gen_constants.cpp" -o xfloat-const-gen
    ./xfloat-const-gen
    cp xfloat_constants_*.cpp "$codeDir/"

    clang++ $flags $exceptions "$codeDir/xfloat_test.cpp" -o xfloat-test
    clang++ $flags $exceptions "$codeDir/float512_test.cpp" -o f512-test
popd > /dev/null

