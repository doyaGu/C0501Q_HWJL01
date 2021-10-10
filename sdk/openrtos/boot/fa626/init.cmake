function(from_hex HEX DEC)
    string(TOUPPER "${HEX}" HEX)
    string(REGEX REPLACE "^0X" "" HEX ${HEX})
    set(_res 0)
    string(LENGTH "${HEX}" _strlen)
    while (_strlen GREATER 0)
        math(EXPR _res "${_res} * 16")
        string(SUBSTRING "${HEX}" 0 1 NIBBLE)
        string(SUBSTRING "${HEX}" 1 -1 HEX)
        if (NIBBLE STREQUAL "A")
            math(EXPR _res "${_res} + 10")
        elseif (NIBBLE STREQUAL "B")
            math(EXPR _res "${_res} + 11")
        elseif (NIBBLE STREQUAL "C")
            math(EXPR _res "${_res} + 12")
        elseif (NIBBLE STREQUAL "D")
            math(EXPR _res "${_res} + 13")
        elseif (NIBBLE STREQUAL "E")
            math(EXPR _res "${_res} + 14")
        elseif (NIBBLE STREQUAL "F")
            math(EXPR _res "${_res} + 15")
        else()
            math(EXPR _res "${_res} + ${NIBBLE}")
        endif()
        string(LENGTH "${HEX}" _strlen)
    endwhile()
    set(${DEC} ${_res} PARENT_SCOPE)
endfunction()

function(toDec STR DEC)
    string(TOUPPER "${STR}" STR)
    if (STR MATCHES "^0X[0-9A-F]+")
        from_hex("${STR}" _res)
    else()
        set(_res "${STR}")
    endif()
    set(${DEC} ${_res} PARENT_SCOPE)
endfunction()

file(READ ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_PROJECT_NAME}.symbol SYMBOL_FILE)
string(REGEX MATCH
    "[0-9a-f]+[ \t]+[0-9a-f]+[ \t]+NOTYPE[ \t]+GLOBAL[ \t]+DEFAULT[ \t]+ABS[ \t]+__lcd_base_a"
    LCDA_ADDR
    ${SYMBOL_FILE}
    )
#message("LCDA_ADDR=${LCDA_ADDR}")

string(REGEX MATCH
    "^[0-9a-f]+"
    LCDA_ADDR
    ${LCDA_ADDR}
    )
#message("LCDA_ADDR=${LCDA_ADDR}")

from_hex("${LCDA_ADDR}" CFG_LCDA_ADDR)
#message("CFG_LCDA_ADDR=${CFG_LCDA_ADDR}")

if (CMAKE_HOST_WIN32)
    execute_process(COMMAND cmd /c for %I in (lcd_clear.bin) do @echo %~zI OUTPUT_VARIABLE CFG_LCD_CLEAR_FILESIZE)
else (CMAKE_HOST_WIN32)
    execute_process(COMMAND stat --format=%s lcd_clear.bin OUTPUT_VARIABLE CFG_LCD_CLEAR_FILESIZE)
endif (CMAKE_HOST_WIN32)
#message("CFG_LCD_CLEAR_FILESIZE=${CFG_LCD_CLEAR_FILESIZE}")

if (CFG_LCD_INIT_ON_BOOTING)

    execute_process(COMMAND convert -define bmp:format=bmp3 -define bmp3:alpha=true ${PROJECT_SOURCE_DIR}/data/bitmap/${CFG_LCD_BOOT_BITMAP} ${CMAKE_CURRENT_BINARY_DIR}/bitmap.bmp)

    if (CFG_LCD_BPP STREQUAL "4")        
        execute_process(COMMAND dataconv --format=ARGB8888 -r -q -b ${CMAKE_CURRENT_BINARY_DIR}/bitmap.bmp -o ${CMAKE_CURRENT_BINARY_DIR}/bitmap.raw OUTPUT_VARIABLE DATACONV_OUTPUT)
    else()        
        execute_process(COMMAND dataconv --format=RGB565 -r -q -b ${CMAKE_CURRENT_BINARY_DIR}/bitmap.bmp -o ${CMAKE_CURRENT_BINARY_DIR}/bitmap.raw OUTPUT_VARIABLE DATACONV_OUTPUT)
    endif()
    
    #message("DATACONV_OUTPUT=${DATACONV_OUTPUT}")

    string(REGEX REPLACE "([0-9]+) ([0-9]+) ([0-9]+) ([0-9]+)" "\\2" BITMAP_WIDTH ${DATACONV_OUTPUT})
    string(REGEX REPLACE "([0-9]+) ([0-9]+) ([0-9]+) ([0-9]+)" "\\3" CFG_LCD_BOOT_BITMAP_HEIGHT ${DATACONV_OUTPUT})
    #message("CFG_LCDA_ADDR=${CFG_LCDA_ADDR}")
    #message("BITMAP_WIDTH=${BITMAP_WIDTH}")
    #message("BITMAP_HEIGHT=${CFG_LCD_BOOT_BITMAP_HEIGHT}")

    math(EXPR CFG_LCD_BOOT_BITMAP_WIDTH "${BITMAP_WIDTH} * ${CFG_LCD_BPP}")
    #message("CFG_LCD_BOOT_BITMAP_WIDTH=${CFG_LCD_BOOT_BITMAP_WIDTH}")

    math(EXPR CFG_LCD_BOOT_BITMAP_ADDR "${CFG_LCD_PITCH} * ((${CFG_LCD_HEIGHT} - ${CFG_LCD_BOOT_BITMAP_HEIGHT}) >> 1) + ((${CFG_LCD_WIDTH} * ${CFG_LCD_BPP} - ${CFG_LCD_BOOT_BITMAP_WIDTH}) >> 1)")
    math(EXPR CFG_LCD_BOOT_BITMAP_ADDR "((${CFG_LCDA_ADDR} + ${CFG_LCD_BOOT_BITMAP_ADDR}) >> 2) << 2")
    #message("CFG_LCD_BOOT_BITMAP_ADDR=${CFG_LCD_BOOT_BITMAP_ADDR}")
else()
    if (CMAKE_HOST_WIN32)
        execute_process(COMMAND cmd /c type nul OUTPUT_FILE bitmap.raw)
    else (CMAKE_HOST_WIN32)
        execute_process(COMMAND touch bitmap.raw)
    endif (CMAKE_HOST_WIN32)
    set(CFG_LCD_BOOT_BITMAP_ADDR 0)
    set(CFG_LCD_BOOT_BITMAP_HEIGHT 0)
    set(CFG_LCD_BOOT_BITMAP_WIDTH 0)
endif()

if (NOT CFG_LCD_MULTIPLE)
    configure_file(${PROJECT_SOURCE_DIR}/sdk/target/lcd/${CFG_LCD_INIT_SCRIPT} ${CMAKE_CURRENT_BINARY_DIR}/lcd.txt)
endif()

configure_file(${PROJECT_SOURCE_DIR}/sdk/target/ram/${CFG_RAM_INIT_SCRIPT} ${CMAKE_CURRENT_BINARY_DIR}/ram.txt)

if (CFG_BACKLIGHT_ENABLE AND CFG_LCD_INIT_ON_BOOTING)
    if (CMAKE_HOST_WIN32)
        execute_process(COMMAND cmd /c for %I in (backlight.bin) do @echo %~zI OUTPUT_VARIABLE CFG_BACKLIGHT_FILESIZE)
    else (CMAKE_HOST_WIN32)
        execute_process(COMMAND stat --format=%s backlight.bin OUTPUT_VARIABLE CFG_BACKLIGHT_FILESIZE)
    endif (CMAKE_HOST_WIN32)
    #message("CFG_BACKLIGHT_FILESIZE=${CFG_BACKLIGHT_FILESIZE}")
    
    file(READ ${CMAKE_CURRENT_BINARY_DIR}/backlight.symbol SYMBOL_FILE)
    string(REGEX MATCH
        "[0-9a-f]+[ \t]+[0-9a-f]+[ \t]+FUNC[ \t]+GLOBAL[ \t]+DEFAULT[ \t]+[0-9a-f]+[ \t]+_start"
        ENTRY_ADDR
        ${SYMBOL_FILE}
        )
    #message("ENTRY_ADDR=${ENTRY_ADDR}")

    string(REGEX MATCH
        "^[0-9a-f]+"
        CFG_BACKLIGHT_ENTRY_ADDR
        ${ENTRY_ADDR}
        )
    #message("ENTRY_ADDR=${CFG_BACKLIGHT_ENTRY_ADDR}")

    execute_process(COMMAND ${CPP} -DCFG_BACKLIGHT_ENTRY_ADDR=0x${CFG_BACKLIGHT_ENTRY_ADDR} -DCFG_BACKLIGHT_FILESIZE=${CFG_BACKLIGHT_FILESIZE} -DCFG_BACKLIGHT_BOOTLODER_DELAY=${CFG_BACKLIGHT_BOOTLODER_DELAY} -I${CMAKE_CURRENT_BINARY_DIR} -E -P -o ${CMAKE_CURRENT_BINARY_DIR}/init.scr ${PROJECT_SOURCE_DIR}/$ENV{CFG_PLATFORM}/boot/fa626/init.scr.in)
else()
    execute_process(COMMAND ${CPP}  -I${CMAKE_CURRENT_BINARY_DIR} -E -P -o ${CMAKE_CURRENT_BINARY_DIR}/init.scr ${PROJECT_SOURCE_DIR}/$ENV{CFG_PLATFORM}/boot/fa626/init.scr.in)
endif()
