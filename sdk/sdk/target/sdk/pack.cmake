file(MAKE_DIRECTORY 
    ${CMAKE_BINARY_DIR}/${CFG_PACK_NAME}/bootloader
    ${CMAKE_BINARY_DIR}/${CFG_PACK_NAME}/${CMAKE_PROJECT_NAME}
    )

file(COPY
    ${bootloader_path}
    DESTINATION ${CMAKE_BINARY_DIR}/${CFG_PACK_NAME}/bootloader
    )

file(COPY
    ${CMAKE_BINARY_DIR}/kproc.sys
    DESTINATION ${CMAKE_BINARY_DIR}/${CFG_PACK_NAME}/${CMAKE_PROJECT_NAME}
    )

file(COPY
    ${CMAKE_BINARY_DIR}/../../data
    ${PROJECT_SOURCE_DIR}/tool/bin/pkgtool.exe
    DESTINATION ${CMAKE_BINARY_DIR}/${CFG_PACK_NAME}
    )

configure_file(${PROJECT_SOURCE_DIR}/sdk/target/sdk/pack.cmd.in ${CMAKE_BINARY_DIR}/${CFG_PACK_NAME}/pack.cmd)

execute_process(COMMAND 7za a -t7z -mx=9 -ms=4G -mf -mhc -mhcf -m1=LZMA:a=2:d=25 -mmt4 ${CFG_PACK_NAME}.7z ${CMAKE_BINARY_DIR}/${CFG_PACK_NAME})
