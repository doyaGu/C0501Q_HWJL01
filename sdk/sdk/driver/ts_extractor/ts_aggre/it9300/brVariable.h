#ifndef __MODULATOR_VARIABLE_H__
#define __MODULATOR_VARIABLE_H__

// ----- LL variables -----
#define OVA_BASE                        0x4C00                      // omega variable address base

//#define OVA_PRECHIP_VERSION_7_0
#define OVA_LINK_VERSION                (OVA_BASE-4)                // 4 byte
#define OVA_LINK_VERSION_31_24          (OVA_LINK_VERSION+0)
#define OVA_LINK_VERSION_23_16          (OVA_LINK_VERSION+1)
#define OVA_LINK_VERSION_15_8           (OVA_LINK_VERSION+2)
#define OVA_LINK_VERSION_7_0            (OVA_LINK_VERSION+3)
#define OVA_SECOND_DEMOD_I2C_ADDR       (OVA_BASE-5)                    

#define OVA_IR_TABLE                    (OVA_BASE-361)              // 7 * 50 + 6 bytes
#define OVA_HID_TABLE                   (OVA_IR_TABLE)              // 7 * 50 bytes
#define OVA_IR_TOGGLE_MASK              (OVA_HID_TABLE+7*50)        // 2 bytes
#define OVA_IR_nKEYS                    (OVA_IR_TOGGLE_MASK+2)      // 1 byte
#define OVA_IR_FN_EXPIRE_TIME           (OVA_IR_nKEYS+1)            // 1 byte
#define OVA_IR_REPEAT_PERIOD            (OVA_IR_FN_EXPIRE_TIME+1)   // 1 byte
#define OVA_IR_RESERVED_PARAM           (OVA_IR_REPEAT_PERIOD+1)    // 1 byte

#define OVA_IR_TABLE_ADDR               (OVA_BASE-363)              // 2 bytes pointer point to IR_TABLE
#define OVA_IR_TABLE_ADDR_15_18         (OVA_IR_TABLE_ADDR+0)
#define OVA_IR_TABLE_ADDR_7_0           (OVA_IR_TABLE_ADDR+1)
#define OVA_HOST_REQ_IR_MODE            (OVA_BASE-364)
#define OVA_EEPROM_CFG                  (OVA_BASE-620)              // 256bytes
#define OVA_XC4000_PKTCNT               (OVA_BASE-621)
#define OVA_XC4000_CLKCNT1              (OVA_BASE-623)              // 2 bytes
#define OVA_XC4000_CLKCNT2              (OVA_BASE-625)              // 2 bytes
#define OVA_I2C_NO_STOPBIT_PKTCNT       (OVA_BASE-626)
#define OVA_CLK_STRETCH                 (OVA_BASE-643)
#define OVA_DUMMY0XX                    (OVA_BASE-644)
#define OVA_HW_VERSION                  (OVA_BASE-645)
#define OVA_TMP_HW_VERSION              (OVA_BASE-646)
#define OVA_EEPROM_CFG_VALID            (OVA_BASE-647)

#define OVA_THIRD_DEMOD_I2C_ADDR        (OVA_BASE-648)  
#define OVA_FOURTH_DEMOD_I2C_ADDR       (OVA_BASE-649)  
#define OVA_SECOND_DEMOD_I2C_BUS        (OVA_BASE-650)  
#define OVA_NEXT_LEVEL_FIRST_I2C_ADDR   (OVA_BASE-651)
#define OVA_NEXT_LEVEL_SECOND_I2C_ADDR  (OVA_BASE-652)
#define OVA_NEXT_LEVEL_THIRD_I2C_ADDR   (OVA_BASE-653)
#define OVA_NEXT_LEVEL_FOURTH_I2C_ADDR  (OVA_BASE-654)
#define OVA_NEXT_LEVEL_FIRST_I2C_BUS    (OVA_BASE-655)
#define OVA_NEXT_LEVEL_SECOND_I2C_BUS   (OVA_BASE-656)
#define OVA_NEXT_LEVEL_THIRD_I2C_BUS    (OVA_BASE-657)
#define OVA_NEXT_LEVEL_FOURTH_I2C_BUS   (OVA_BASE-658)
#define OVA_EEPROM_I2C_ADD              (OVA_BASE-659)              //0x496D

#define OVA_EEPROM_BOOT_SEGMENT_SIZE    (OVA_BASE-761)              //0x4907-0x4946
#define OVA_EEPROM_BOOT_FW_OFFSET       (OVA_BASE-763)              //0x4905-0x4906
#define OVA_EEPROM_BOOT_FW_SIZE         (OVA_BASE-765)              //0x4903-0x4904
#define OVA_EEPROM_BOOT_FW_SEGMENT_NUM  (OVA_BASE-766)              //0x4902
#define OVA_EEPROM_BOOT_ERROR_CODE      (OVA_BASE-767)              //0x4901
#define OVA_TEST                        (OVA_BASE-768)              //0x4900

// for API variable name, not use in firmware
#define second_i2c_address              OVA_SECOND_DEMOD_I2C_ADDR       //0x4BFB
#define third_i2c_address               OVA_THIRD_DEMOD_I2C_ADDR        //0x4978
#define fourth_i2c_address              OVA_FOURTH_DEMOD_I2C_ADDR       //0x4977
#define second_i2c_bus                  OVA_SECOND_DEMOD_I2C_BUS        //0x4976
#define next_level_first_i2c_address    OVA_NEXT_LEVEL_FIRST_I2C_ADDR   //0x4975    
#define next_level_second_i2c_address   OVA_NEXT_LEVEL_SECOND_I2C_ADDR  //0x4974
#define next_level_third_i2c_address    OVA_NEXT_LEVEL_THIRD_I2C_ADDR   //0x4973
#define next_level_fourth_i2c_address   OVA_NEXT_LEVEL_FOURTH_I2C_ADDR  //0x4972
#define next_level_first_i2c_bus        OVA_NEXT_LEVEL_FIRST_I2C_BUS    //0x4971    
#define next_level_second_i2c_bus       OVA_NEXT_LEVEL_SECOND_I2C_BUS   //0x4970
#define next_level_third_i2c_bus        OVA_NEXT_LEVEL_THIRD_I2C_BUS    //0x496F
#define next_level_fourth_i2c_bus       OVA_NEXT_LEVEL_FOURTH_I2C_BUS   //0x496E

#define ir_table_start_15_8             OVA_IR_TABLE_ADDR_15_18         //0x417F
#define ir_table_start_7_0              OVA_IR_TABLE_ADDR_7_0           //0x4180
#define prechip_version_7_0             0x384F
#define chip_version_7_0                0x1222
#define link_version_31_24              OVA_LINK_VERSION_31_24          //0x83E9
#define link_version_23_16              OVA_LINK_VERSION_23_16          //0x83EA
#define link_version_15_8               OVA_LINK_VERSION_15_8           //0x83EB
#define link_version_7_0                OVA_LINK_VERSION_7_0            //0x83EC

#define modulator_var_end               0x0153
#endif

