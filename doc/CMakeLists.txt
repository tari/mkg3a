set(INSTALL_MANDIR "/share/man/"
    CACHE PATH "Directory containing manpages")

set(MAN_SOURCE "${PROJECT_SOURCE_DIR}/doc/mkg3a.1")
add_custom_command(
    OUTPUT mkg3a.1.gz
    COMMAND gzip ${MAN_SOURCE} -c > mkg3a.1.gz
    DEPENDS ${MAN_SOURCE}
)
add_custom_target(doc ALL DEPENDS mkg3a.1.gz)
install_files("${INSTALL_MANDIR}/man1" FILES mkg3a.1.gz)
