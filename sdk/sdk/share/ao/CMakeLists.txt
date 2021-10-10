add_definitions(
    -DAO_BUILDING_LIBAO
    )

include_directories(
    ${PROJECT_SOURCE_DIR}/sdk/include/ao
    )
    
add_library(ao STATIC
    ao_aixs.c
    ao_au.c
    ao_null.c
    ao_raw.c
    ao_wav.c
    audio_out.c
    config.c
    )

if (DEFINED CFG_WIN32_SIMULATOR)
    add_definitions(
        -DHAVE_WMM
        )

    add_library(ao STATIC
        ao_wmm.c
        )
endif()