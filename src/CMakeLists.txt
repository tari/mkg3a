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
if (EXISTS "${PROJECT_SOURCE_DIR}/.hg/")
    # We're in a repo
    execute_process( COMMAND hg id -t
                     WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                     OUTPUT_VARIABLE HG_TAG OUTPUT_STRIP_TRAILING_WHITESPACE )
    execute_process( COMMAND hg id -i
                     WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                     OUTPUT_VARIABLE HG_NODE OUTPUT_STRIP_TRAILING_WHITESPACE )
 elseif (EXISTS "${PROJECT_SOURCE_DIR}/.hg_archival.txt")
    # Auto-archived tag snapshot of repository
    file(STRINGS ../.hg_archival.txt HG_TAG
         REGEX "tag: ")
    string(SUBSTRING "${HG_TAG}" 5 -1 HG_TAG)
    file(STRINGS ../.hg_archival.txt HG_NODE
         REGEX "node: ")
    string(SUBSTRING "${HG_NODE}" 5 -1 HG_NODE)
else ()
    # No idea
    set (HG_TAG "<unknown version>")
endif()

# If not tagged or this revision is tip, use the node ID instead of tag
string( LENGTH "${HG_TAG}" HG_TAGLEN )
if ( HG_TAG STREQUAL tip OR NOT HG_TAGLEN )
    set( HG_TAG "hg ${HG_NODE}" )
endif ()
message( STATUS "Version: ${HG_TAG}" )

set (mkg3a_VERSION_TAG ${HG_TAG})
configure_file (
    config.h.in
    config.h
)
include_directories ("${PROJECT_BINARY_DIR}/src")


## Targets
add_library (g3a-util STATIC g3a.c util.c)

add_executable (mkg3a mkg3a.c)
target_link_libraries (mkg3a g3a-util images ${EXTRA_LIBS})
install (TARGETS mkg3a
         DESTINATION bin)

add_executable (g3a-icondump g3a-icondump.c)
target_link_libraries (g3a-icondump images g3a-util)

add_executable (convert565 convert565.c)
target_link_libraries (convert565 images g3a-util)
