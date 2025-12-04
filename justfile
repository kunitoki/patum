
default:
    @just --list

update:
    cmake -G Xcode -B build .

test TARGET="Debug":
    @just update
    cmake --build build --config {{TARGET}} --target patum_tests
    ./build/{{TARGET}}/patum_tests
