find_program(ASCIIDOCTOR_EXE asciidoctor)
if(NOT ASCIIDOCTOR_EXE)
    message(FATAL_ERROR "asciidoctor is required to build documentation")
else()
    message(STATUS "Using asciidoctor: ${ASCIIDOCTOR_EXE}")
endif()

add_custom_command(OUTPUT KeePassXC_GettingStarted.html
    COMMAND ${ASCIIDOCTOR_EXE} -D ${CMAKE_CURRENT_BINARY_DIR} -o KeePassXC_GettingStarted.html ${CMAKE_CURRENT_SOURCE_DIR}/GettingStarted/index.adoc
    DEPENDS ./GettingStarted/*
    VERBATIM
)
add_custom_command(OUTPUT KeePassXC_UserGuide.html
    COMMAND ${ASCIIDOCTOR_EXE} -D ${CMAKE_CURRENT_BINARY_DIR} -o KeePassXC_UserGuide.html ${CMAKE_CURRENT_SOURCE_DIR}/UserGuide/index.adoc
    DEPENDS ./UserGuide/*
    VERBATIM
)
add_custom_target(userguide DEPENDS KeePassXC_GettingStarted.html KeePassXC_UserGuide.html)
