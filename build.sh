#!/bin/bash

set -e

curDir="$(pwd)"
codeDir="$curDir/src"
buildDir="$curDir/gebouw"
compiler=tcc

cflags="-O3 -g -ggdb -Wall -Werror -pedantic -lm"
cppflags="-O0 -g -ggdb -Wall -Werror -pedantic -std=c++11 -lm"

exceptions="-Wno-unused-function -Wno-writable-strings -Wno-gnu-zero-variadic-macro-arguments -Wno-gnu-anonymous-struct -Wno-nested-anon-types -Wno-missing-braces -Wno-vla-extension"

mkdir -p "$buildDir"

echo Building "xfloat"

pushd "$buildDir" > /dev/null
    $compiler $cflags $exceptions "$codeDir/xfloat_gen_constants.c" -o xfloat-const-gen
    ./xfloat-const-gen
    cp xfloat_constants_*.cpp "$codeDir/"

    $compiler $cflags $exceptions "$codeDir/xfloat_test.c" -o xfloat-test
    $compiler $cflags $exceptions "$codeDir/xfloat_test_math.c" -o xfloat-math-test
    clang++ $cppflags $exceptions "$codeDir/float512_test.cpp" -o f512-test
popd > /dev/null

