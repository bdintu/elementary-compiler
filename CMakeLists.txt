cmake_minimum_required(VERSION 3.5)
project (ElementaryCompiler)

find_package(FLEX)
find_package(BISON)

FLEX_TARGET(Scanner src/simple.l ${CMAKE_CURRENT_BINARY_DIR}/lex.yy.c)
BISON_TARGET(Parser src/simple.y ${CMAKE_CURRENT_BINARY_DIR}/simple.tab.c)
ADD_FLEX_BISON_DEPENDENCY(Scanner Parser)

include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_executable(simple ${BISON_Parser_OUTPUTS} ${FLEX_Scanner_OUTPUTS} src/main.c)
target_link_libraries(simple ${FLEX_LIBRARIES})
