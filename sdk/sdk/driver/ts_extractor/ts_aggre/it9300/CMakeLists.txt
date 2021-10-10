
if (DEFINED CFG_AGGRE_IT9300)
    if (CMAKE_BUILD_TYPE STREQUAL Debug)
        add_definitions(
            -DDEBUG
            )
    endif()

    include_directories(
        .
        ${PROJECT_SOURCE_DIR}/sdk/include/iic
        )

    add_library(aggre STATIC
        brCmd.cpp
        brCmd.h
        brError.h
        brfirmware.h
        brRegister.h
        brTiming.h
        brType.h
        brUser.cpp
        brUser.h
        brVariable.h
        brVersion.h
        IT9300.cpp
        IT9300.h
        )

endif()
