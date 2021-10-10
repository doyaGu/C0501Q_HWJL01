if (DEFINED CFG_ALT_CPU_ENABLE)

    if (DEFINED CFG_RSL_MASTER)
        add_subdirectory(rslMaster)
    endif()

    if (DEFINED CFG_RSL_SLAVE)
        add_subdirectory(rslSlave)
    endif()

    if (DEFINED CFG_SW_PWM)
        add_subdirectory(swPwm)
    endif()
endif()
