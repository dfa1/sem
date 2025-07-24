cmake_minimum_required(VERSION 3.10)
project(sem C)

# Find Flex and Bison
find_package(FLEX REQUIRED)
find_package(BISON REQUIRED)

# Generate parser and lexer
FLEX_TARGET(Scanner src/scanner.l ${CMAKE_CURRENT_BINARY_DIR}/lexer.yy.c)
BISON_TARGET(Compiler src/compiler.y ${CMAKE_CURRENT_BINARY_DIR}/parser.tab.c COMPILE_FLAGS "--defines=${CMAKE_CURRENT_BINARY_DIR}/parser.tab.h")
ADD_FLEX_BISON_DEPENDENCY(Scanner Compiler)

# Add executable
add_executable(myproject_exec
        ${FLEX_Lexer_OUTPUTS}
        ${BISON_Parser_OUTPUTS}
        src/debugger.c
        src/io.c
        src/memory.c
        src/vm.c
        src/main.c
)

# Include directories
target_include_directories(myproject_exec PRIVATE
        ${CMAKE_CURRENT_BINARY_DIR}  # for generated .h files
        ${CMAKE_SOURCE_DIR}/include  # for defs.h or others
)