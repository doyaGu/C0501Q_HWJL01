#ifndef __CS8556_H__
#define __CS8556_H__

#include "ite/itp.h"
#include "ite/ith_defs.h"
#include "ite/ith.h"

#ifdef __cplusplus
extern "C" {
#endif


//====================================================================
/*
 * 0x203A
 *    H/W Test Register 
 */
//====================================================================
//#define REG_TESTHW					(REG_CAP_BASE + 0x003A)
#define REG_00_00    0x0000
#define REG_00_01    0x0001
#define REG_00_02    0x0002
#define REG_00_04    0x0004
#define REG_00_05    0x0005
#define REG_00_06    0x0006
#define REG_00_07    0x0007
#define REG_00_08    0x0008
#define REG_00_09    0x0009
#define REG_00_0A    0x000A
#define REG_00_0B    0x000B
#define REG_00_0C    0x000C
#define REG_00_0D    0x000D
#define REG_00_0E    0x000E
#define REG_00_0F    0x000F
#define REG_00_10    0x0010
#define REG_00_11    0x0011
#define REG_00_30    0x0030
#define REG_00_31    0x0031
#define REG_00_32    0x0032
#define REG_00_33    0x0033
#define REG_00_34    0x0034
#define REG_00_35    0x0035
#define REG_00_36    0x0036
#define REG_00_37    0x0037
#define REG_00_38    0x0038
#define REG_00_39    0x0039
#define REG_00_3A    0x003A
#define REG_00_3B    0x003B
#define REG_00_3C    0x003C

#define REG_00_3D    0x003D
#define REG_00_3E    0x003E
#define REG_00_3F    0x003F
#define REG_00_40    0x0040
#define REG_00_41    0x0041
#define REG_00_42    0x0042
#define REG_00_43    0x0043
#define REG_00_44    0x0044
#define REG_00_45    0x0045
#define REG_00_46    0x0046
#define REG_00_47    0x0047
#define REG_00_48    0x0048
#define REG_00_49    0x0049
#define REG_00_4A    0x004A
#define REG_00_4B    0x004B
#define REG_00_4C    0x004C
#define REG_00_4D    0x004D
#define REG_00_4E    0x004E
#define REG_00_50    0x0050
#define REG_00_51    0x0051
#define REG_00_52    0x0052
#define REG_00_53    0x0053
#define REG_00_54    0x0054
#define REG_00_55    0x0055
#define REG_00_56    0x0056
#define REG_00_57    0x0057
#define REG_00_58    0x0058
#define REG_00_59    0x0059
#define REG_00_5A    0x005A
#define REG_00_5B    0x005B
#define REG_00_5C    0x005C
#define REG_00_5E    0x005E
#define REG_00_5F    0x005F
#define REG_00_60    0x0060
#define REG_00_61    0x0061
#define REG_00_62    0x0062
#define REG_00_63    0x0063
#define REG_00_64    0x0064



void CS8556Initial(void);

#ifdef __cplusplus
}
#endif

#endif