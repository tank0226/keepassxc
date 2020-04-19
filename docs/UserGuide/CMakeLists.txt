if(WITH_XC_DOCS)
    add_custom_command(OUTPUT KeePassXC_GettingStarted.html
        COMMAND ${ASCIIDOCTOR_EXE} -D ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/KeePassXC_GettingStarted.adoc
        DEPENDS KeePassXC_GettingStarted.adoc
        VERBATIM
    )
    add_custom_command(OUTPUT KeePassXC_UserGuide.html
        COMMAND ${ASCIIDOCTOR_EXE} -D ${CMAKE_CURRENT_BINARY_DIR} ${CMAKE_CURRENT_SOURCE_DIR}/KeePassXC_UserGuide.adoc
        DEPENDS KeePassXC_UserGuide.adoc
        VERBATIM
    )
    add_custom_target(userguide DEPENDS KeePassXC_GettingStarted.html KeePassXC_UserGuide.html)
endif()
