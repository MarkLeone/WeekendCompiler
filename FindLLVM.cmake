# - Find LLVM library
# Find the native LLVM includes and library
# This module defines
#  LLVM_INCLUDE_DIRS, where to find LLVM.h, Set when LLVM_INCLUDE_DIR is found.
#  LLVM_LIBRARIES, libraries to link against to use LLVM.
#  LLVM_ROOT_DIR, The base directory to search for LLVM.
#                This can also be an environment variable.
#  LLVM_FOUND, If false, do not try to use LLVM.
#
# also defined, but not for general use are
#  LLVM_LIBRARY, where to find the LLVM library.

#=============================================================================
# Copyright 2015 Blender Foundation.
#
# Distributed under the OSI-approved BSD License (the "License");
# see accompanying file Copyright.txt for details.
#
# This software is distributed WITHOUT ANY WARRANTY; without even the
# implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
# See the License for more information.
#=============================================================================

if(LLVM_ROOT_DIR)
  if(DEFINED LLVM_VERSION)
    find_program(LLVM_CONFIG llvm-config-${LLVM_VERSION} HINTS ${LLVM_ROOT_DIR}/bin)
  endif()
  if(NOT LLVM_CONFIG)
    find_program(LLVM_CONFIG llvm-config HINTS ${LLVM_ROOT_DIR}/bin)
  endif()
else()
  if(DEFINED LLVM_VERSION)
        message(running llvm-config-${LLVM_VERSION})
    find_program(LLVM_CONFIG llvm-config-${LLVM_VERSION})
  endif()
  if(NOT LLVM_CONFIG)
    find_program(LLVM_CONFIG llvm-config)
  endif()
endif()

if(NOT DEFINED LLVM_VERSION)
  execute_process(COMMAND ${LLVM_CONFIG} --version
          OUTPUT_VARIABLE LLVM_VERSION
          OUTPUT_STRIP_TRAILING_WHITESPACE)
  set(LLVM_VERSION ${LLVM_VERSION} CACHE STRING "Version of LLVM to use")
endif()
if(NOT LLVM_ROOT_DIR)
  execute_process(COMMAND ${LLVM_CONFIG} --prefix
          OUTPUT_VARIABLE LLVM_ROOT_DIR
          OUTPUT_STRIP_TRAILING_WHITESPACE)
  set(LLVM_ROOT_DIR ${LLVM_ROOT_DIR} CACHE PATH "Path to the LLVM installation")
endif()
if(NOT LLVM_LIBPATH)
  execute_process(COMMAND ${LLVM_CONFIG} --libdir
          OUTPUT_VARIABLE LLVM_LIBPATH
          OUTPUT_STRIP_TRAILING_WHITESPACE)
  set(LLVM_LIBPATH ${LLVM_LIBPATH} CACHE PATH "Path to the LLVM library path")
  mark_as_advanced(LLVM_LIBPATH)
endif()

if(LLVM_STATIC)
  # For static builds, try to find any core LLVM library
  find_library(LLVM_LIBRARY
               NAMES 
                 LLVMCore
                 LLVMAnalysis
                 LLVMSupport
               PATHS ${LLVM_LIBPATH})
else()
  find_library(LLVM_LIBRARY
               NAMES
                 LLVM-${LLVM_VERSION}
                 LLVMCore
                 LLVMAnalysis  # check for the static library as a fall-back
               PATHS ${LLVM_LIBPATH})
endif()


if(LLVM_LIBRARY AND LLVM_ROOT_DIR AND LLVM_LIBPATH)
  execute_process(COMMAND ${LLVM_CONFIG} --includedir
    OUTPUT_VARIABLE LLVM_INCLUDE_DIRS
    OUTPUT_STRIP_TRAILING_WHITESPACE)
  if(LLVM_STATIC)
    # if static LLVM libraries were requested, try to use llvm-config to generate
    # the list of what libraries we need, but fall back to basic libraries if it fails
    execute_process(COMMAND ${LLVM_CONFIG} --libfiles
                    OUTPUT_VARIABLE LLVM_LIBRARY
                    ERROR_VARIABLE LLVM_CONFIG_ERROR
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    RESULT_VARIABLE LLVM_CONFIG_RESULT)
    
    if(NOT LLVM_CONFIG_RESULT EQUAL 0)
      message(STATUS "llvm-config --libfiles failed, trying --libs all")
      # Try using --libs all instead
      execute_process(COMMAND ${LLVM_CONFIG} --libs all
                      OUTPUT_VARIABLE LLVM_LIBRARY
                      ERROR_VARIABLE LLVM_CONFIG_ERROR
                      OUTPUT_STRIP_TRAILING_WHITESPACE
                      RESULT_VARIABLE LLVM_CONFIG_RESULT)
      
      if(NOT LLVM_CONFIG_RESULT EQUAL 0)
        message(FATAL_ERROR "Both llvm-config --libfiles and --libs all failed")
      else()
        # Use -l flags directly, let the linker resolve them
        string(REPLACE " " ";" LLVM_LIBRARY "${LLVM_LIBRARY}")
      endif()
    else()
      string(REPLACE " " ";" LLVM_LIBRARY "${LLVM_LIBRARY}")
    endif()
    
    # Also get system libraries that LLVM depends on
    execute_process(COMMAND ${LLVM_CONFIG} --system-libs
                    OUTPUT_VARIABLE LLVM_SYSTEM_LIBS
                    ERROR_VARIABLE LLVM_CONFIG_ERROR
                    OUTPUT_STRIP_TRAILING_WHITESPACE
                    RESULT_VARIABLE LLVM_CONFIG_RESULT)
    
    if(LLVM_CONFIG_RESULT EQUAL 0)
      string(REPLACE " " ";" LLVM_SYSTEM_LIBS "${LLVM_SYSTEM_LIBS}")
      list(APPEND LLVM_LIBRARY ${LLVM_SYSTEM_LIBS})
    endif()
  endif()
endif()


# handle the QUIETLY and REQUIRED arguments and set SDL2_FOUND to TRUE if
# all listed variables are TRUE
INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(LLVM DEFAULT_MSG
    LLVM_LIBRARY)

MARK_AS_ADVANCED(
  LLVM_LIBRARY
)
