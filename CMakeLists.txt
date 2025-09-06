cmake_minimum_required(VERSION 3.16)

project(WeekendCompiler 
  VERSION 1.0
  DESCRIPTION "A weekend compiler project"
  LANGUAGES CXX
  HOMEPAGE_URL "https://github.com/yourusername/WeekendCompiler"
)

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Compiler-specific options
if(MSVC)
  add_compile_options(/W4)
else()
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# Find re2c executable
find_program(RE2C_EXE 
  NAMES re2c
  PATHS ${CMAKE_PREFIX_PATH}
  PATH_SUFFIXES bin
  DOC "Path to re2c executable"
)

if(NOT RE2C_EXE)
  message(FATAL_ERROR "re2c executable not found. Please install re2c or specify RE2C_EXE")
endif()

# Generate Lexer.cpp from Lexer.re using re2c
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Lexer.cpp
  COMMAND ${RE2C_EXE} -o ${CMAKE_CURRENT_BINARY_DIR}/Lexer.cpp ${CMAKE_SOURCE_DIR}/Lexer.re
  DEPENDS ${CMAKE_SOURCE_DIR}/Lexer.re
  COMMENT "Generating Lexer.cpp from Lexer.re"
  VERBATIM
)

set(LLVM_STATIC TRUE)

# Try modern LLVM config first
find_package(LLVM QUIET CONFIG)

# If modern config failed, use custom FindLLVM.cmake
if(NOT LLVM_FOUND)
  message(STATUS "Modern LLVM config not found, falling back to custom FindLLVM.cmake")
  list(INSERT CMAKE_MODULE_PATH 0 "${CMAKE_SOURCE_DIR}")
  find_package(LLVM REQUIRED)
endif()

# Create the main executable
add_executable(weekend
  main.cpp
  Codegen.cpp
  Parser.cpp
  Printer.cpp
  Token.cpp
  Typechecker.cpp
  ${CMAKE_CURRENT_BINARY_DIR}/Lexer.cpp
)

# Set target properties
target_include_directories(weekend 
  PRIVATE 
    ${CMAKE_SOURCE_DIR}
)

# Handle both modern and legacy LLVM configurations
if(TARGET LLVM::LLVM)
  # Modern LLVM config provides imported targets
  target_link_libraries(weekend PRIVATE LLVM::LLVM)
else()
  # Legacy FindLLVM.cmake provides variables
  target_include_directories(weekend PRIVATE ${LLVM_INCLUDE_DIRS})
  target_link_libraries(weekend PRIVATE ${LLVM_LIBRARY})
  if(LLVM_DEFINITIONS)
    target_compile_definitions(weekend PRIVATE ${LLVM_DEFINITIONS})
  endif()
  # Link against C++ standard library for RTTI support
  target_link_libraries(weekend PRIVATE stdc++)
endif()

