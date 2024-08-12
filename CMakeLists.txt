cmake_minimum_required(VERSION 3.16)

#####################################################
#                   project setup                   #
#####################################################
# ~ version
set(ARCH_VERSION_MAJOR 0)
set(ARCH_VERSION_MINOR 0)
set(ARCH_VERSION_PATCH 1)
set(ARCH_VERSION ${ARCH_VERSION_MAJOR}.${ARCH_VERSION_MINOR}.${ARCH_VERSION_PATCH})

# ~ project creation
project(archivio VERSION ${ARCH_VERSION} LANGUAGES C)

# ~ setting cmake variables
set(CMAKE_C_STANDARD 99)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON) # for clangd

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# ~ detecting standalone build
set(ARCH_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR})
string(COMPARE EQUAL "${CMAKE_CURRENT_SOURCE_DIR}" "${ARCH_SOURCE_DIR}" ARCH_STANDALONE)

# ~ defining options
option(ARCH_BUILD_EXAMPLES "Build the arch example programs." ${ARCH_STANDALONE})

# ~ printing info
message(STATUS "==== Archivio info ====")
message(STATUS "lib type: static")
message(STATUS "version:  ${ARCH_VERSION}")
message(STATUS "ARCH_BUILD_EXAMPLES:  ${ARCH_BUILD_EXAMPLES}")

#####################################################
#                    target setup                   #
#####################################################
# ~ target set up
add_library(archivio STATIC ./src/arch.c)
target_include_directories(archivio PUBLIC include)

# ~ platform-specific source files
if (UNIX)
  target_sources(archivio PRIVATE ./src/unix.c)
endif()

# ~ example programm
if (ARCH_BUILD_EXAMPLES)
  add_subdirectory(examples)
endif()
