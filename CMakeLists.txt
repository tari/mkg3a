cmake_minimum_required (VERSION 2.6)
project (mkg3a)
add_definitions(-Wall)
if (MSVC)
    # Always static linking on windows
    add_definitions(/MT)
else ()
    add_definitions(-Wextra)
endif ()

## Feature checks
include (TestBigEndian)
test_big_endian (IS_BIG_ENDIAN)

include (CheckIncludeFiles)
check_include_files (unistd.h HAVE_UNISTD_H)

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

execute_process( COMMAND hg id -t
                 WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                 OUTPUT_VARIABLE HG_TAG OUTPUT_STRIP_TRAILING_WHITESPACE )
string( LENGTH "${HG_TAG}" HG_TAGLEN )
if ( HG_TAG STREQUAL tip OR NOT HG_TAGLEN )
    execute_process( COMMAND hg id -i
                     WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                     OUTPUT_VARIABLE HG_TAG OUTPUT_STRIP_TRAILING_WHITESPACE )
    set( HG_TAG "hg ${HG_TAG}" )
endif()
message( STATUS "Version: ${HG_TAG}" )

## Configuration
set (mkg3a_VERSION_TAG ${HG_TAG})

configure_file (
	"${PROJECT_SOURCE_DIR}/config.h.in"
	"${PROJECT_BINARY_DIR}/config.h"
)
include_directories ("${PROJECT_BINARY_DIR}")

add_library (icon STATIC icon.c)
if (USE_PNG)
    include_directories (${PNG_INCLUDE_DIRS})
    target_link_libraries (icon ${PNG_LIBRARY} ${ZLIB_LIBRARY})
endif ()

add_executable (mkg3a main.c g3a.c util.c)
target_link_libraries(mkg3a icon)

if (NOT HAVE_UNISTD_H)
    message (STATUS "unistd.h unavailable; using included getopt")
    add_library (getopt getopt.c)
    target_link_libraries (mkg3a getopt)
endif ()

add_executable (g3a-icodump icodump.c icon.c util.c)
target_link_libraries (g3a-icodump icon)
#add_executable (convert565 convert565.c icon.c util.c)

install (TARGETS mkg3a DESTINATION bin)
