
default:
    @just --list

update:
    cmake -G Xcode -B build .

open: update
    -open build/patum.xcodeproj

test TARGET="Debug": update
    cmake --build build --config {{TARGET}} --target patum_tests
    ./build/tests/{{TARGET}}/patum_tests

coverage:
    cmake -S . -B build-coverage -G Ninja -DCMAKE_BUILD_TYPE=Debug '-DCMAKE_CXX_FLAGS=-fprofile-instr-generate -fcoverage-mapping' '-DCMAKE_EXE_LINKER_FLAGS=-fprofile-instr-generate'
    cmake --build build-coverage --target patum_tests
    LLVM_PROFILE_FILE=build-coverage/patum_tests.profraw ./build-coverage/tests/patum_tests
    llvm-profdata merge -sparse build-coverage/patum_tests.profraw -o build-coverage/patum_tests.profdata
    llvm-cov report ./build-coverage/tests/patum_tests -arch=$(uname -m) -instr-profile=build-coverage/patum_tests.profdata include/patum
    llvm-cov show ./build-coverage/tests/patum_tests -arch=$(uname -m) -instr-profile=build-coverage/patum_tests.profdata -format=html -output-dir=build-coverage/coverage-html include/patum
    @echo "HTML coverage report: build-coverage/coverage-html/index.html"

amalgamate: test
    python3 scripts/amalgamate.py

clean:
    rm -rf build/*
