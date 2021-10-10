add_definitions(

    )

include_directories(
    .    
    ${PROJECT_SOURCE_DIR}/sdk/driver/
    )
    
add_library(sxa_dmx STATIC
    sxa_dmx_3gpp.c
    sxa_dmx_3gpp_stbl_entry.c
    sxa_dmx_demuxer.c
    file.c
    )
