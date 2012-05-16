set(INSTALL_MANDIR share/man
    CACHE PATH)
add_custom_command(OUTPUT mkg3a.1.gz
                   COMMAND gzip mkg3a.1 -c > mkg3a.1.gz
                   DEPENDS mkg3a.1)
install(TARGETS mkg3a.1.gz DESTINATION ${INSTALL_MANDIR}/man1)
