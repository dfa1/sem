cmake_minimum_required(VERSION 3.10)
project(sem C)

# Set C standard to C23
set(CMAKE_C_STANDARD 23)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS ON)

# Enable maximum warnings
if (CMAKE_C_COMPILER_ID MATCHES "Clang" OR CMAKE_C_COMPILER_ID MATCHES "AppleClang")
    add_compile_options(
            -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wundef
            -Wcast-align -Wstrict-prototypes -Wwrite-strings
            -Wmissing-prototypes -Wold-style-definition
            -Wfloat-equal -Waggregate-return -Wswitch-enum
            -Wunreachable-code -Wformat=2 -Wnull-dereference
            -Wdouble-promotion -Wvla -Wno-unused-parameter
    )
elseif (CMAKE_C_COMPILER_ID STREQUAL "GNU")
    add_compile_options(
            -Wall -Wextra -Wpedantic -Wconversion -Wshadow -Wundef
            -Wcast-align -Wstrict-prototypes -Wwrite-strings
            -Wmissing-prototypes -Wold-style-definition
            -Wfloat-equal -Waggregate-return -Wswitch-enum
            -Wunreachable-code -Wformat=2 -Wnull-dereference
            -Wdouble-promotion -Wvla -Wno-unused-parameter
    )
endif()
set(CMAKE_VERBOSE_MAKEFILE ON)

# Find Flex and Bison
find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

# Generate parser and lexer
FLEX_TARGET(Scanner src/scanner.l ${CMAKE_CURRENT_BINARY_DIR}/lexer.yy.c)
BISON_TARGET(Compiler src/compiler.y ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.c COMPILE_FLAGS "--defines=${CMAKE_CURRENT_BINARY_DIR}/parser.tab.h")
ADD_FLEX_BISON_DEPENDENCY(Scanner Compiler)

# Add executable
add_executable(sem
        ${FLEX_Scanner_OUTPUTS}
        ${BISON_Compiler_OUTPUTS}
        src/debugger.c
        src/io.c
        src/memory.c
        src/vm.c
        src/main.c
)

# Include directories
target_include_directories(sem PRIVATE include)

# testing support
enable_testing()
include(CTest)

add_executable(UnitTests tests/test_memory.c src/memory.c)
target_include_directories(UnitTests PRIVATE include)

add_test(NAME RunUnitTests COMMAND UnitTests)

