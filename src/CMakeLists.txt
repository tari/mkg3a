cmake_minimum_required (VERSION 2.6)

## Basic build configuration
if (MSVC)
    # Always static linking on windows
    add_definitions(/MT)
else ()
    add_definitions(-Wextra)
endif ()


## Platform compatibility
include (TestBigEndian)
test_big_endian (IS_BIG_ENDIAN)

include (CheckIncludeFiles)
check_include_files (unistd.h HAVE_UNISTD_H)
if (NOT HAVE_UNISTD_H)
    message (STATUS "unistd.h unavailable; using included getopt")
    set (EXTRA_LIBS getopt)
    add_library (getopt STATIC getopt.c)
endif ()


## PNG loader
option (USE_PNG "Enable png icon support via libpng" ON)
if (USE_PNG)
    message (STATUS "libpng icon loading support enabled.")
    find_package (PNG REQUIRED)
    include_directories (${PNG_INCLUDE_DIR})
    set (USE_PNG "1")
else ()
    # Lazy kludge for config.h.in
    set (USE_PNG "0")
endif ()

add_library (images STATIC images.c)
if (USE_PNG)
    include_directories (${PNG_INCLUDE_DIRS})
    target_link_libraries (images ${PNG_LIBRARY} ${ZLIB_LIBRARY})
endif ()


## Build configuration
execute_process(
    COMMAND git describe --tags
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_DESCRIPTION
    ERROR_VARIABLE GIT_ERROR
    RESULT_VARIABLE GIT_RESULT
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
if (NOT GIT_RESULT EQUAL 0)
    message(WARNING "Unable to determine program version from git")
    set(GIT_DESCRIPTION "<unknown version>")
endif()
message(STATUS "Version: ${GIT_DESCRIPTION}")


set (mkg3a_VERSION_TAG ${GIT_DESCRIPTION})
configure_file (
    config.h.in
    config.h
)
include_directories ("${PROJECT_BINARY_DIR}/src")


## Targets
add_library (g3a-util STATIC g3a.c util.c)

add_executable (mkg3a mkg3a.c)
target_link_libraries (mkg3a g3a-util images ${EXTRA_LIBS})

add_executable (g3a-icondump g3a-icondump.c)
target_link_libraries (g3a-icondump images g3a-util)

add_executable (g3a-updateicon g3a-updateicon.c)
target_link_libraries (g3a-updateicon images g3a-util)

add_executable (convert565 convert565.c)
target_link_libraries (convert565 images g3a-util)

# Install generally useful tools
install (TARGETS mkg3a g3a-updateicon
         DESTINATION bin)
