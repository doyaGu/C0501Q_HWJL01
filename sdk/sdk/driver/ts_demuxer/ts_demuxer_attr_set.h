
// external define example
// #define TSD_DEMOD_ATTR_SET(demod_chip,demod_index,demod_bus_type,demod_i2c_addr,linked_tsi_index,privData)\
//                 {demod_index, demod_bus_type, demod_chip, demod_i2c_addr, privData},



//////////////////////////////////////////////////////


/**
 * demod module attribute setting
 **/
#if defined(CFG_DEMOD_IT9135)
    // DEMOD_IT9135
    #if (CFG_DEMOD_SUPPORT_COUNT > 0)
        TSD_DEMOD_ATTR_SET(DEMOD_ID_IT9135, 0, DEMOD_BUS_I2C, 0x38, 0, 0)
    #endif

    #if (CFG_DEMOD_SUPPORT_COUNT > 1)
        TSD_DEMOD_ATTR_SET(DEMOD_ID_IT9135, 1, DEMOD_BUS_I2C, 0x3C, 1, 0)
    #endif

    #if (CFG_DEMOD_SUPPORT_COUNT > 2)
        TSD_DEMOD_ATTR_SET(DEMOD_ID_IT9135, 2, DEMOD_BUS_USB, 0x0, -1, 0)
    #endif

    #if (CFG_DEMOD_SUPPORT_COUNT > 3)
        TSD_DEMOD_ATTR_SET(DEMOD_ID_IT9135, 3, DEMOD_BUS_USB, 0x0, -1, 0)
    #endif
#elif defined(CFG_DEMOD_IT9137)
    // DEMOD_IT9137
    #if (CFG_DEMOD_SUPPORT_COUNT > 0)
        TSD_DEMOD_ATTR_SET(DEMOD_ID_IT9137, 0, DEMOD_BUS_I2C, 0x38, 0, 0)
    #endif

    #if (CFG_DEMOD_SUPPORT_COUNT > 1)
        TSD_DEMOD_ATTR_SET(DEMOD_ID_IT9137, 1, DEMOD_BUS_I2C, 0x3C, 1, 0)
    #endif

    #if (CFG_DEMOD_SUPPORT_COUNT > 2)
        TSD_DEMOD_ATTR_SET(DEMOD_ID_IT9137, 2, DEMOD_BUS_UNKNOW, 0x0, -1, 0)
    #endif

    #if (CFG_DEMOD_SUPPORT_COUNT > 3)
        TSD_DEMOD_ATTR_SET(DEMOD_ID_IT9137, 3, DEMOD_BUS_UNKNOW, 0x0, -1, 0)
    #endif    
#elif defined(CFG_DEMOD_IT9137_USB)
    // DEMOD_IT9137_USB
    #if (CFG_DEMOD_SUPPORT_COUNT > 0)
        TSD_DEMOD_ATTR_SET(DEMOD_ID_IT9137_USB, 0, DEMOD_BUS_I2C, 0x38, 0, 0)
    #endif

    #if (CFG_DEMOD_SUPPORT_COUNT > 1)
        TSD_DEMOD_ATTR_SET(DEMOD_ID_IT9137_USB, 1, DEMOD_BUS_I2C, 0x3C, 1, 0)
    #endif

    #if (CFG_DEMOD_SUPPORT_COUNT > 2)
        TSD_DEMOD_ATTR_SET(DEMOD_ID_IT9137_USB, 2, DEMOD_BUS_USB, 0x0, -1, 0)
    #endif

    #if (CFG_DEMOD_SUPPORT_COUNT > 3)
        TSD_DEMOD_ATTR_SET(DEMOD_ID_IT9137_USB, 3, DEMOD_BUS_USB, 0x0, -1, 0)
    #endif
#else
    // all unknow
    #if (CFG_DEMOD_SUPPORT_COUNT == -1)
        TSD_DEMOD_ATTR_SET(DEMOD_ID_UNKNOW, 0, DEMOD_BUS_UNKNOW, 0x0, -1, 0)
    #endif
#endif

#undef TSD_DEMOD_ATTR_SET

