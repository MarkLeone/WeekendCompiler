cmake_minimum_required(VERSION 3.1)

project(WeekendCompiler VERSION 1.0
  LANGUAGES CXX)

find_program(RE2C_EXE re2c)
if(NOT RE2C_EXE)
  message(WARNING "re2c executable not found.  Please specify RE2C_EXE.")
endif()

add_custom_command(
    OUTPUT  Lexer.cpp
    COMMAND "${RE2C_EXE}" -o Lexer.cpp "${CMAKE_SOURCE_DIR}/Lexer.re"
    DEPENDS Lexer.re
    )

list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_SOURCE_DIR}")
set(LLVM_STATIC TRUE)
find_package(LLVM REQUIRED)

add_executable(weekend
  main.cpp
  Codegen.cpp
  Parser.cpp
  Printer.cpp
  Token.cpp
  Typechecker.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/Lexer.cpp
  )

target_include_directories(weekend PUBLIC "${CMAKE_SOURCE_DIR}" "${LLVM_INCLUDE_DIRS}")
target_link_libraries(weekend PUBLIC "${LLVM_LDFLAGS}" ${LLVM_LIBRARY})
