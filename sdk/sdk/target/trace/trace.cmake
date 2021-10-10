file(READ ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.symbol SYMBOL_FILE)
set(TRACE_DATA_SYM "")

# parsing the symbol table for LTO build
string(REGEX MATCH
    ": [0-9a-f]+[ \t]+[0-9a-fx]+[ \t]+OBJECT[ \t]+LOCAL[ \t]+DEFAULT[ \t]+[0-9]+[ \t]+RecorderData.[0-9]+\n"
    TRACE_DATA_SYM
    ${SYMBOL_FILE}
    )

# parsing the symbol table for normal build
string(COMPARE EQUAL "${TRACE_DATA_SYM}" "" result)
if (result)
    string(REGEX MATCH
        ": [0-9a-f]+[ \t]+[0-9a-fx]+[ \t]+OBJECT[ \t]+GLOBAL[ \t]+DEFAULT[ \t]+[0-9]+[ \t]+RecorderData\n"
        TRACE_DATA_SYM
        ${SYMBOL_FILE}
        )
endif()
#message("TRACE_DATA_SYM=${TRACE_DATA_SYM}")

string(COMPARE EQUAL "${TRACE_DATA_SYM}" "" result)
if (NOT result)
    string(REGEX REPLACE ": ([0-9a-f]+)[ \t]+([0-9a-fx]+)[ \t]+OBJECT[ \t]+[GL].+" "\\1" TRACE_DATA_ADDR ${TRACE_DATA_SYM})
    string(REGEX REPLACE ": ([0-9a-f]+)[ \t]+([0-9a-fx]+)[ \t]+OBJECT[ \t]+[GL].+" "\\2" TRACE_DATA_SIZE ${TRACE_DATA_SYM})

    file(GLOB files ${PROJECT_SOURCE_DIR}/sdk/target/trace/*.in)

    foreach (src ${files})
        string(REPLACE "${PROJECT_SOURCE_DIR}/sdk/target/trace/" "${CMAKE_CURRENT_BINARY_DIR}/" tmp ${src})
        string(REPLACE ".in" "" dest ${tmp})
        configure_file(${src} ${dest})
    endforeach()
    message("Generate dumptrace.cmd.")
else()
    message(FATAL_ERROR "Can not find the symbol for trace data.")
endif ()
#message("TRACE_DATA_ADDR=${TRACE_DATA_ADDR}")
#message("TRACE_DATA_SIZE=${TRACE_DATA_SIZE}")
