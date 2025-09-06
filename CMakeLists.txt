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

# Include ExternalProject module
include(ExternalProject)

# Select the appropriate re2c preset based on the operating system
if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  set(RE2C_PRESET "linux-gcc-release-ootree-skeleton-fast")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
  set(RE2C_PRESET "macos-clang-release-ootree-skeleton-fast")
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  set(RE2C_PRESET "windows-msvc-release-ootree-fast")
else()
  message(FATAL_ERROR "Unsupported operating system: ${CMAKE_SYSTEM_NAME}. Please add support for this platform.")
endif()

message(STATUS "Using re2c preset: ${RE2C_PRESET}")

# Build re2c from source using ExternalProject
ExternalProject_Add(
  re2c
  GIT_REPOSITORY https://github.com/skvadrik/re2c.git
  GIT_TAG 4.3
  PREFIX ${CMAKE_CURRENT_BINARY_DIR}/external/re2c
  INSTALL_DIR ${CMAKE_CURRENT_BINARY_DIR}/external/re2c/install
  CMAKE_ARGS
    -DCMAKE_INSTALL_PREFIX=${CMAKE_CURRENT_BINARY_DIR}/external/re2c/install
    -DRE2C_BUILD_TESTS=OFF
    -DRE2C_BUILD_DOCS=OFF
    -DCMAKE_PRESET_NAME=${RE2C_PRESET}
  BUILD_BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/external/re2c/install/bin/re2c
)

# Get the re2c executable path
set(RE2C_EXE ${CMAKE_CURRENT_BINARY_DIR}/external/re2c/install/bin/re2c)

# Generate Lexer.cpp from Lexer.re using re2c
add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Lexer.cpp
  COMMAND ${RE2C_EXE} -o ${CMAKE_CURRENT_BINARY_DIR}/Lexer.cpp ${CMAKE_SOURCE_DIR}/Lexer.re
  DEPENDS ${CMAKE_SOURCE_DIR}/Lexer.re re2c
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

