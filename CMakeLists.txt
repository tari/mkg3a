cmake_minimum_required (VERSION 2.6)
project (mkg3a)
add_definitions(-Wall -Wextra)

## Feature checks
include (TestBigEndian)
test_big_endian (IS_BIG_ENDIAN)

include (CheckIncludeFiles)
check_include_files (unistd.h HAVE_UNISTD_H)

option(USE_MAGICKCORE "Use ImageMagick for icon support.")
if (USE_MAGICKCORE)
	message (STATUS "Using MagickCore for icon support")
	find_package (ImageMagick REQUIRED MagickCore)
	include_directories ("${ImageMagick_INCLUDE_DIRS}")
	target_link_libraries (icon ${ImageMagick_MagickCore_LIBRARIES})
else ()
    message (STATUS "Using built-in bmp handler for icon support")
endif ()

execute_process( COMMAND hg parent --template "{tags} {node}"
                 WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
                 OUTPUT_VARIABLE HG_PARENT )
string( REGEX MATCH "^ ?[^ ]+"
        HG_TAG "${HG_PARENT}" )
message( STATUS "Version: ${HG_TAG}" )

## Configuration
set (mkg3a_VERSION_TAG ${HG_TAG})

configure_file (
	"${PROJECT_SOURCE_DIR}/config.h.in"
	"${PROJECT_BINARY_DIR}/config.h"
)
include_directories ("${PROJECT_BINARY_DIR}")

add_definitions (-Wall)
add_executable (mkg3a main.c g3a.c icon.c util.c)

if (NOT HAVE_UNISTD_H)
	message (STATUS "unistd.h unavailable; using included getopt")
    add_library (getopt getopt.c)
    target_link_libraries (mkg3a getopt)
endif ()

add_executable (g3a-icodump icodump.c icon.c util.c)

install (TARGETS mkg3a g3a-icodump DESTINATION bin)
