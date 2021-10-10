if (DEFINED CFG_UI_ENC_ENABLE)

    if (CMAKE_BUILD_TYPE STREQUAL Debug)
        ADD_DEFINITIONS(
            -DDEBUG
            )
    endif()

    INCLUDE_DIRECTORIES(
        .
        ${PROJECT_SOURCE_DIR}/sdk/include/uiEnc
        ${SMEDIA_INC_DIRS}
        )

    add_definitions(
        -DCFG_ALPHA_ENABLE
        )    

    ADD_LIBRARY(uiEnc STATIC
        mmp_uiEnc.c
        uiEnc_hw.c
        
        # include file
        ${PROJECT_SOURCE_DIR}/sdk/include/uiEnc/mmp_uiEnc.h
        ${PROJECT_SOURCE_DIR}/sdk/include/uiEnc/uiEnc_err.h
        uiEnc_config.h
        uiEnc_defs.h
        uiEnc_hw.h
        uiEnc_types.h
        )                        

endif()
