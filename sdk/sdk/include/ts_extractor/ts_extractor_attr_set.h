
// external define example
// #define TSE_AGGRE_ATTR_SET(aggre_chip,aggre_bus_type,aggre_index,aggre_i2c_addr,linked_tsi_index)\
//                 {aggre_chip, aggre_bus_type, aggre_index, aggre_i2c_addr, linked_tsi_index},

// #define TSE_DEMOD_ATTR_SET(demod_chip,attached_aggre_id,demod_index,demod_bus_type,demod_i2c_addr,linked_tsi_index)\
//                 {attached_aggre_id, {demod_index, demod_bus_type, demod_chip, demod_i2c_addr}},


#if (CFG_DEMOD_SUPPORT_COUNT > 2) && (CFG_DEMOD_SWITCH_GPIO_0 < 0)
    #error "Wrong demod count and switch_GPIO !!"
#endif

#ifndef TSE_DEMOD_SWITCH_GPIO_0
    #define TSE_DEMOD_SWITCH_GPIO_0             CFG_DEMOD_SWITCH_GPIO_0
#endif
#ifndef TSE_DEMOD_SWITCH_GPIO_0_SHIFT
    #define TSE_DEMOD_SWITCH_GPIO_0_SHIFT       24
#endif
#ifndef TSE_DEMOD_SWITCH_GPIO_MASK
    #define TSE_DEMOD_SWITCH_GPIO_MASK          0xFF
#endif

// just reserve code
//#ifndef TSE_DEMOD_SWITCH_GPIO_1
//    #define TSE_DEMOD_SWITCH_GPIO_1             CFG_DEMOD_SWITCH_GPIO_1
//#endif
//#ifndef TSE_DEMOD_SWITCH_GPIO_1_SHIFT
//    #define TSE_DEMOD_SWITCH_GPIO_1_SHIFT       16
//#endif
//#ifndef TSE_DEMOD_SWITCH_GPIO_2
//    #define TSE_DEMOD_SWITCH_GPIO_2             CFG_DEMOD_SWITCH_GPIO_2
//#endif
//#ifndef TSE_DEMOD_SWITCH_GPIO_2_SHIFT
//    #define TSE_DEMOD_SWITCH_GPIO_2_SHIFT       8
//#endif


#ifndef TSE_DEMOD_I2C_ADDR_0
    #define TSE_DEMOD_I2C_ADDR_0        (0x38 | ((0xFF & TSE_DEMOD_SWITCH_GPIO_MASK) << TSE_DEMOD_SWITCH_GPIO_0_SHIFT))
#endif

#ifndef TSE_DEMOD_I2C_ADDR_1
    #define TSE_DEMOD_I2C_ADDR_1        (0x3c | ((0xFF & TSE_DEMOD_SWITCH_GPIO_MASK) << TSE_DEMOD_SWITCH_GPIO_0_SHIFT))
#endif

#ifndef TSE_DEMOD_I2C_ADDR_2
    #define TSE_DEMOD_I2C_ADDR_2        (0x38 | ((TSE_DEMOD_SWITCH_GPIO_0 & TSE_DEMOD_SWITCH_GPIO_MASK) << TSE_DEMOD_SWITCH_GPIO_0_SHIFT))
#endif

#ifndef TSE_DEMOD_I2C_ADDR_3
    #define TSE_DEMOD_I2C_ADDR_3        (0x3c | ((TSE_DEMOD_SWITCH_GPIO_0 & TSE_DEMOD_SWITCH_GPIO_MASK) << TSE_DEMOD_SWITCH_GPIO_0_SHIFT))
#endif

#ifndef TSE_DEMOD_I2C_ADDR_4
    #define TSE_DEMOD_I2C_ADDR_4        0x0
#endif

#ifndef TSE_DEMOD_I2C_ADDR_5
    #define TSE_DEMOD_I2C_ADDR_5        0x0
#endif

#ifndef TSE_DEMOD_I2C_ADDR_6
    #define TSE_DEMOD_I2C_ADDR_6        0x0
#endif

#ifndef TSE_DEMOD_I2C_ADDR_7
    #define TSE_DEMOD_I2C_ADDR_7        0x0
#endif

//////////////////////////////////////////////////////
#ifndef TSE_AGGRE_ATTR_SET
    #define TSE_AGGRE_ATTR_SET(aggre_chip, aggre_bus_type, aggre_index, aggre_i2c_addr, linked_tsi_index)
#endif

#ifndef TSE_DEMOD_ATTR_SET
    #define TSE_DEMOD_ATTR_SET(demod_chip, attached_aggre_id, demod_index, demod_bus_type, demod_i2c_addr, linked_aggre_port_idx)
#endif

/**
 * aggre module attribute setting
 **/
#if defined(CFG_AGGRE_IT9300)
    // AGGRE_IT9300
    #if defined(CFG_BOARD_ITE_9079_EVB)
        #if (CFG_AGGRE_SUPPORT_COUNT > 0)
        TSE_AGGRE_ATTR_SET(TSE_AGGRE_TYPE_ENDEAVOUR, TSE_AGGRE_BUS_I2C, 1, 0x68, 1)
        #endif

        #if (CFG_AGGRE_SUPPORT_COUNT > 1)
        TSE_AGGRE_ATTR_SET(TSE_AGGRE_TYPE_ENDEAVOUR, TSE_AGGRE_BUS_UNKNOW, 0, 0x6A, 0)
        #endif
    #else /* #if defined(CFG_BOARD_ITE_9079_EVB) */
        #if (CFG_AGGRE_SUPPORT_COUNT > 0)
        TSE_AGGRE_ATTR_SET(TSE_AGGRE_TYPE_ENDEAVOUR, TSE_AGGRE_BUS_I2C, 0, 0x68, 0)
        #endif

        #if (CFG_AGGRE_SUPPORT_COUNT > 1)
        TSE_AGGRE_ATTR_SET(TSE_AGGRE_TYPE_ENDEAVOUR, TSE_AGGRE_BUS_UNKNOW, 1, 0x6A, 1)
        #endif
    #endif /* #if defined(CFG_BOARD_ITE_9079_EVB) */
#else
    // all unknow
    #if (CFG_AGGRE_SUPPORT_COUNT == -1)
    TSE_AGGRE_ATTR_SET(TSE_AGGRE_TYPE_UNKNOW, TSE_AGGRE_BUS_UNKNOW, -1, 0x0, 0)
    #endif
#endif

/**
 * demod module attribute setting
 **/
#if defined(CFG_DEMOD_IT9135)
    // DEMOD_IT9135
    #if (CFG_DEMOD_SUPPORT_COUNT > 0)
        #if (CFG_AGGRE_SUPPORT_COUNT == -1)
            TSE_DEMOD_ATTR_SET(TSE_DEMOD_TYPE_IT9135, -1, 0, TSE_DEMOD_BUS_I2C, TSE_DEMOD_I2C_ADDR_0, -1)
        #else
            TSE_DEMOD_ATTR_SET(TSE_DEMOD_TYPE_IT9135, 0, 0, TSE_DEMOD_BUS_I2C, TSE_DEMOD_I2C_ADDR_0, CFG_DEMOD_0_LINK_AGGRE_PORT)
        #endif
    #endif

    #if (CFG_DEMOD_SUPPORT_COUNT > 1)
        #if (CFG_AGGRE_SUPPORT_COUNT == -1)
            TSE_DEMOD_ATTR_SET(TSE_DEMOD_TYPE_IT9135, -1, 1, TSE_DEMOD_BUS_I2C, TSE_DEMOD_I2C_ADDR_1, -1)
        #else
            TSE_DEMOD_ATTR_SET(TSE_DEMOD_TYPE_IT9135, 0, 1, TSE_DEMOD_BUS_I2C, TSE_DEMOD_I2C_ADDR_1, CFG_DEMOD_1_LINK_AGGRE_PORT)
        #endif
    #endif

    #if (CFG_DEMOD_SUPPORT_COUNT > 2)
        TSE_DEMOD_ATTR_SET(TSE_DEMOD_TYPE_IT9135, 0, 2, TSE_DEMOD_BUS_UNKNOW, TSE_DEMOD_I2C_ADDR_2, CFG_DEMOD_2_LINK_AGGRE_PORT)
    #endif

    #if (CFG_DEMOD_SUPPORT_COUNT > 3)
        TSE_DEMOD_ATTR_SET(TSE_DEMOD_TYPE_IT9135, 0, 3, TSE_DEMOD_BUS_UNKNOW, TSE_DEMOD_I2C_ADDR_3, CFG_DEMOD_3_LINK_AGGRE_PORT)
    #endif

    #if (CFG_DEMOD_SUPPORT_COUNT > 4)
        TSE_DEMOD_ATTR_SET(TSE_DEMOD_TYPE_IT9135, 1, 0, TSE_DEMOD_BUS_UNKNOW, TSE_DEMOD_I2C_ADDR_4, -1)
    #endif

    #if (CFG_DEMOD_SUPPORT_COUNT > 5)
        TSE_DEMOD_ATTR_SET(TSE_DEMOD_TYPE_IT9135, 1, 1, TSE_DEMOD_BUS_UNKNOW, TSE_DEMOD_I2C_ADDR_5, -1)
    #endif

    #if (CFG_DEMOD_SUPPORT_COUNT > 6)
        TSE_DEMOD_ATTR_SET(TSE_DEMOD_TYPE_IT9135, 1, 2, TSE_DEMOD_BUS_UNKNOW, TSE_DEMOD_I2C_ADDR_6, -1)
    #endif

    #if (CFG_DEMOD_SUPPORT_COUNT > 7)
        TSE_DEMOD_ATTR_SET(TSE_DEMOD_TYPE_IT9135, 1, 3, TSE_DEMOD_BUS_UNKNOW, TSE_DEMOD_I2C_ADDR_7, -1)
    #endif
#elif defined(CFG_DEMOD_IT9137)
    // DEMOD_IT9137
    #if (CFG_DEMOD_SUPPORT_COUNT > 0)
        #if (CFG_AGGRE_SUPPORT_COUNT == -1)
            TSE_DEMOD_ATTR_SET(TSE_DEMOD_TYPE_IT9137, -1, 0, TSE_DEMOD_BUS_I2C, TSE_DEMOD_I2C_ADDR_0, -1)
        #else
            TSE_DEMOD_ATTR_SET(TSE_DEMOD_TYPE_IT9137, 0, 0, TSE_DEMOD_BUS_I2C, TSE_DEMOD_I2C_ADDR_0, CFG_DEMOD_0_LINK_AGGRE_PORT)
        #endif
    #endif

    #if (CFG_DEMOD_SUPPORT_COUNT > 1)
        #if (CFG_AGGRE_SUPPORT_COUNT == -1)
            TSE_DEMOD_ATTR_SET(TSE_DEMOD_TYPE_IT9137, -1, 1, TSE_DEMOD_BUS_I2C, TSE_DEMOD_I2C_ADDR_1, -1)
        #else
            TSE_DEMOD_ATTR_SET(TSE_DEMOD_TYPE_IT9137, 0, 1, TSE_DEMOD_BUS_I2C, TSE_DEMOD_I2C_ADDR_1, CFG_DEMOD_1_LINK_AGGRE_PORT)
        #endif
    #endif

    #if (CFG_DEMOD_SUPPORT_COUNT > 2)
        TSE_DEMOD_ATTR_SET(TSE_DEMOD_TYPE_IT9137, 0, 2, TSE_DEMOD_BUS_I2C, TSE_DEMOD_I2C_ADDR_2, CFG_DEMOD_2_LINK_AGGRE_PORT)
    #endif

    #if (CFG_DEMOD_SUPPORT_COUNT > 3)
        TSE_DEMOD_ATTR_SET(TSE_DEMOD_TYPE_IT9137, 0, 3, TSE_DEMOD_BUS_I2C, TSE_DEMOD_I2C_ADDR_3, CFG_DEMOD_3_LINK_AGGRE_PORT)
    #endif

    #if (CFG_DEMOD_SUPPORT_COUNT > 4)
        TSE_DEMOD_ATTR_SET(TSE_DEMOD_TYPE_IT9137, 1, 0, TSE_DEMOD_BUS_UNKNOW, TSE_DEMOD_I2C_ADDR_4, -1)
    #endif

    #if (CFG_DEMOD_SUPPORT_COUNT > 5)
        TSE_DEMOD_ATTR_SET(TSE_DEMOD_TYPE_IT9137, 1, 1, TSE_DEMOD_BUS_UNKNOW, TSE_DEMOD_I2C_ADDR_5, -1)
    #endif

    #if (CFG_DEMOD_SUPPORT_COUNT > 6)
        TSE_DEMOD_ATTR_SET(TSE_DEMOD_TYPE_IT9137, 1, 2, TSE_DEMOD_BUS_UNKNOW, TSE_DEMOD_I2C_ADDR_6, -1)
    #endif

    #if (CFG_DEMOD_SUPPORT_COUNT > 7)
        TSE_DEMOD_ATTR_SET(TSE_DEMOD_TYPE_IT9137, 1, 3, TSE_DEMOD_BUS_UNKNOW, TSE_DEMOD_I2C_ADDR_7, -1)
    #endif
#else
    // all unknow
    #if (CFG_DEMOD_SUPPORT_COUNT == -1)
        TSE_DEMOD_ATTR_SET(TSE_DEMOD_TYPE_UNKNOW, 0, 0, TSE_DEMOD_BUS_UNKNOW, 0x0, -1)
    #endif
#endif




#undef TSE_AGGRE_ATTR_SET
#undef TSE_DEMOD_ATTR_SET

