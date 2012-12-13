cmake_minimum_required (VERSION 2.6)
project (mkg3a)

add_definitions (-Wall)
add_subdirectory(doc)
add_subdirectory(src)

# Packaging
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A tool to create Casio FX-CG (Prizm) addon (.g3a) files")
set(CPACK_PACKAGE_VENDOR "Peter Marheine")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/README")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING")
set(CPACK_PACKAGE_VERSION_MAJOR "0")
set(CPACK_PACKAGE_VERSION_MINOR "3")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Peter Marheine <peter@taricorp.net>")
include(CPack)
