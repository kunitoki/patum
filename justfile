
default:
    @just --list

update:
    cmake -G Xcode -B build .

open: update
    -open build/patum.xcodeproj

test TARGET="Debug": update
    cmake --build build --config {{TARGET}} --target patum_tests
    ./build/tests/{{TARGET}}/patum_tests

amalgamate: test
    python3 scripts/amalgamate.py

clean:
    rm -rf build/*
