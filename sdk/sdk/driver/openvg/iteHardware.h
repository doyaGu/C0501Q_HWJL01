/*
 */

#ifndef __ITEHARDWARE_H
#define __ITEHARDWARE_H

#include "openvg.h"
#include "iteDefs.h"
#include "iteVectors.h"
#include "itePath.h"

/* ======================================== */ 
/*              ITE Register                */
/* ======================================== */

//======================================
// Base address
//======================================
#define ITE_VG_AHB_REG_BASE  0xD0600000
#define ITE_VG_HOST_REG_BASE 0x6C00

//======================================
//Tessellation Control Register
//======================================
#define ITE_VG_REG_TCR_BASE     0x000
//
// D[31:12] Minter limit length (12.8)
//
#define ITE_VG_CMDSHIFT_MITERLIMIT  12
#define ITE_VG_CMDMASK_MITERLIMIT   0xFFFFF000

//
// D[15:14] Join Type 
// 00: Miter 
// 01: Round 
// 10: Bevel
//
#define ITE_VG_CMD_JOINTYPE_MITER   0x00000000
#define ITE_VG_CMD_JOINTYPE_ROUND   0x00004000
#define ITE_VG_CMD_JOINTYPE_BEVEL   0x00008000
#define ITE_VG_CMDSHIFT_JOINTYPE    14
#define ITE_VG_CMDMASK_JOINTYPE     0x0000C000
//
// D[13:12] Cap Type 
// 00: Square 
// 01: Round 
// 10: BUTT
//
#define ITE_VG_CMD_CAPTYPE_SQUARE   0x00000000
#define ITE_VG_CMD_CAPTYPE_ROUND    0x00001000
#define ITE_VG_CMD_CAPTYPE_BUTT     0x00002000
#define ITE_VG_CMDSHIFT_CAPTYPE     12
#define ITE_VG_CMDMASK_CAPTYPE      0x00003000

//
// D[11] Enable Perspective transform
//
#define ITE_VG_CMD_PERSPECTIVE_EN   0x00000800

//
// D[10] Enable Transform
//
#define ITE_VG_CMD_TRANSFORM_EN     0x00000400

//
// D[9] Stroked line dash phase reset
//
#define ITE_VG_CMD_DASHPHASERESET   0x00000200

//
// D[8] Enable Stroked path
//
#define ITE_VG_CMD_STROKEPATH       0x00000100

//
// D[3] Tessellation Debug bit
// 0: Disable
// 1: Enable
//
#define ITE_VG_CMD_TELDEBUGEN		0x00000008

//
// D[2] Caculate boundary bit
// 0: Disable
// 1: Enable
//
#define ITE_VG_CMD_CALBOUNDARY		0x00000004

//
// D[1] Read memory bit
// Data of tesselation from memory or not
// 0: Disable
// 1: Enable
//
#define ITE_VG_CMD_READMEM		    0x00000002

//
// D[1] Tesseleation engine enable bit
// 0: Disable
// 1: Enable
//
#define ITE_VG_CMD_TELWORK		    0x00000001

//======================================
//Tessellation Control Register End
//======================================


//======================================
//Line Width Register
//======================================
#define ITE_VG_REG_LWR_BASE     0x004
//
// D[24:0] The Width of Stroked line (S12.12)
//
#define ITE_VG_CMDSHIFT_LINEWIDTH   0
#define ITE_VG_CMDMASK_LINEWIDTH    0x01FFFFFF
//======================================
//Line Width Register End
//======================================


//======================================
//Stroke Round Number Register
//======================================
#define ITE_VG_REG_SRNR_BASE    0x008
//
// D[20:16] Total Dash line pattern count
//
#define ITE_VG_CMDSHIFT_DASHCOUNT   24
#define ITE_VG_CMDMASK_DASHCOUNT    0x1F000000
//
// D[11:0] Divide the stroke round to the number
//
#define ITE_VG_CMDSHIFT_STROKEROUNDNUM   0
#define ITE_VG_CMDMASK_STROKEROUNDNUM    0x00000FFF

//
// D[6] Shift rounding: Enable rounding with right shift in TL shift ALU.
//
#define ITE_VG_CMD_SHIFTROUNDING	0x00000040

//
// D[4] Join Round 0: Bevel join in Bezier and arc curves
//                 1: Round join in Bezier and arc curves
#define ITE_VG_CMD_JOINROUND		0x00000030

//======================================
//Stroke Round Number Register End
//======================================


//======================================
//Frame Number Register
//======================================
#define ITE_VG_REG_FNR_BASE     0x00C
//
// D[31]
//
#define ITE_VG_CMD_PROBE_NOT    0x80000000

//
// D[29:24]
//
#define ITE_VG_CMDSHIFT_PROBEMUX   24
#define ITE_VG_CMDMASK_PROBEMUX    0x3F000000

//
// D[21:20]
//
#define ITE_VG_CMDSHIFT_DEBUGBDY   20
#define ITE_VG_CMDMASK_DEBUGBDY    0x00300000

//
// D[18]
//
#define ITE_VG_CMD_FRAM_END    0x00040000

//
// D[17]
//
#define ITE_VG_CMD_FRAM_START    0x00020000

//
// D[16]
//
#define ITE_VG_CMD_FRAM_EN    0x00010000

//
// D[15:0] OpenVG state is busy at least of No. of HCLK.
//
#define ITE_VG_CMDSHIFT_FRAMENUM   0
#define ITE_VG_CMDMASK_FRAMENUM    0x0000FFFF
//======================================
//Frame Number Register End
//======================================


//======================================
//Path Length Register
//======================================
#define ITE_VG_REG_PLR_BASE     0x010
//
// D[31:0] Path Length in Byte. (32 bits alignment)
//
#define ITE_VG_CMDSHIFT_PATHLENGTH   0
#define ITE_VG_CMDMASK_PATHLENGTH    0xFFFFFFFF
//======================================
//Path Length Register End
//======================================


//======================================
//Path Base Register
//======================================
#define ITE_VG_REG_PBR_BASE     0x014
//
// D[31:0] Path Base Address. (32 bits alignment)
//
#define ITE_VG_CMDSHIFT_PATHBASE   0
#define ITE_VG_CMDMASK_PATHBASE    0xFFFFFFFF
//======================================
//Path Base Register End
//======================================


//======================================
//Tessellation Length Register
//======================================
#define ITE_VG_REG_TLR_BASE     0x018
//
// D[31:0] Tessellation Length in Byte. (32 bits alignment)
//
#define ITE_VG_CMDSHIFT_TELLENGTH   0
#define ITE_VG_CMDMASK_TELLENGTH    0xFFFFFFFF
//======================================
//Tessellation Length Register End
//======================================


//======================================
//Tessellation Base Register
//======================================
#define ITE_VG_REG_TBR_BASE     0x01C
//
// D[31:0] Tessellation Base Address. (32 bits alignment)
//
#define ITE_VG_CMDSHIFT_TELBASE   0
#define ITE_VG_CMDMASK_TELBASE    0xFFFFFFFF
//======================================
//Tessellation Base Register End
//======================================


//======================================
//User Transform 00 Register
//======================================
#define ITE_VG_REG_UTR00_BASE   0x020
//
// D[28:0] User Transform Parameter 00(s12.16)
//
#define ITE_VG_CMDSHIFT_USRXFMS00   0
#define ITE_VG_CMDMASK_USRXFMS00    0x1FFFFFFF
//======================================
//User Transform 00 Register End
//======================================


//======================================
//User Transform 01 Register
//======================================
#define ITE_VG_REG_UTR01_BASE   0x024
//
// D[28:0] User Transform Parameter 01(s12.16)
//
#define ITE_VG_CMDSHIFT_USRXFMS01   0
#define ITE_VG_CMDMASK_USRXFMS01    0x1FFFFFFF
//======================================
//User Transform 01 Register End
//======================================


//======================================
//User Transform 02 Register
//======================================
#define ITE_VG_REG_UTR02_BASE   0x028
//
// D[28:0] User Transform Parameter 02(s12.16)
//
#define ITE_VG_CMDSHIFT_USRXFMS02   0
#define ITE_VG_CMDMASK_USRXFMS02    0x1FFFFFFF
//======================================
//User Transform 02 Register End
//======================================


//======================================
//User Transform 10 Register
//======================================
#define ITE_VG_REG_UTR10_BASE   0x02C
//
// D[28:0] User Transform Parameter 10(s12.16)
//
#define ITE_VG_CMDSHIFT_USRXFMS10   0
#define ITE_VG_CMDMASK_USRXFMS10    0x1FFFFFFF
//======================================
//User Transform 10 Register End
//======================================


//======================================
//User Transform 11 Register
//======================================
#define ITE_VG_REG_UTR11_BASE   0x030
//
// D[28:0] User Transform Parameter 11(s12.16)
//
#define ITE_VG_CMDSHIFT_USRXFMS11   0
#define ITE_VG_CMDMASK_USRXFMS11    0x1FFFFFFF
//======================================
//User Transform 11 Register End
//======================================


//======================================
//User Transform 12 Register
//======================================
#define ITE_VG_REG_UTR12_BASE   0x034
//
// D[28:0] User Transform Parameter 12(s12.16)
//
#define ITE_VG_CMDSHIFT_USRXFMS12   0
#define ITE_VG_CMDMASK_USRXFMS12    0x1FFFFFFF
//======================================
//User Transform 12 Register End
//======================================


//======================================
//User Transform 20 Register
//======================================
#define ITE_VG_REG_UTR20_BASE   0x038
//
// D[28:0] User Transform Parameter 20(s12.16)
//
#define ITE_VG_CMDSHIFT_USRXFMS20   0
#define ITE_VG_CMDMASK_USRXFMS20    0x1FFFFFFF
//======================================
//User Transform 20 Register End
//======================================


//======================================
//User Transform 21 Register
//======================================
#define ITE_VG_REG_UTR21_BASE   0x03C
//
// D[28:0] User Transform Parameter 21(s12.16)
//
#define ITE_VG_CMDSHIFT_USRXFMS21   0
#define ITE_VG_CMDMASK_USRXFMS21    0x1FFFFFFF
//======================================
//User Transform 21 Register End
//======================================


//======================================
//User Transform 22 Register
//======================================
#define ITE_VG_REG_UTR22_BASE   0x040
//
// D[28:0] User Transform Parameter 22(s12.16)
//
#define ITE_VG_CMDSHIFT_USRXFMS22   0
#define ITE_VG_CMDMASK_USRXFMS22    0x1FFFFFFF
//======================================
//User Transform 22 Register End
//======================================


//======================================
//Dash Pattern 00 Register
//======================================
#define ITE_VG_REG_DPR00_BASE   0x044
//
// D[23:0] Dash Line Pattern 00 (MSB: Enable bit, s12.11)
//
#define ITE_VG_CMDSHIFT_DASHPAT00   0
#define ITE_VG_CMDMASK_DASHPAT00    0x00FFFFFF
//======================================
//Dash Pattern 00 Register End
//======================================


//======================================
//Dash Pattern 01 Register
//======================================
#define ITE_VG_REG_DPR01_BASE   0x048
//
// D[23:0] Dash Line Pattern 01 (MSB: Enable bit, s12.11)
//
#define ITE_VG_CMDSHIFT_DASHPAT01   0
#define ITE_VG_CMDMASK_DASHPAT01    0x00FFFFFF
//======================================
//Dash Pattern 01 Register End
//======================================


//======================================
//Dash Pattern 02 Register
//======================================
#define ITE_VG_REG_DPR02_BASE   0x04C
//
// D[23:0] Dash Line Pattern 02 (MSB: Enable bit, s12.11)
//
#define ITE_VG_CMDSHIFT_DASHPAT02   0
#define ITE_VG_CMDMASK_DASHPAT02    0x00FFFFFF
//======================================
//Dash Pattern 02 Register End
//======================================


//======================================
//Dash Pattern 03 Register
//======================================
#define ITE_VG_REG_DPR03_BASE   0x050
//
// D[23:0] Dash Line Pattern 03 (MSB: Enable bit, s12.11)
//
#define ITE_VG_CMDSHIFT_DASHPAT03   0
#define ITE_VG_CMDMASK_DASHPAT03    0x00FFFFFF
//======================================
//Dash Pattern 03 Register End
//======================================


//======================================
//Dash Pattern 04 Register
//======================================
#define ITE_VG_REG_DPR04_BASE   0x054
//
// D[23:0] Dash Line Pattern 04 (MSB: Enable bit, s12.11)
//
#define ITE_VG_CMDSHIFT_DASHPAT04   0
#define ITE_VG_CMDMASK_DASHPAT04    0x00FFFFFF
//======================================
//Dash Pattern 04 Register End
//======================================


//======================================
//Dash Pattern 05 Register
//======================================
#define ITE_VG_REG_DPR05_BASE   0x058
//
// D[23:0] Dash Line Pattern 05 (MSB: Enable bit, s12.11)
//
#define ITE_VG_CMDSHIFT_DASHPAT05   0
#define ITE_VG_CMDMASK_DASHPAT05    0x00FFFFFF
//======================================
//Dash Pattern 05 Register End
//======================================


//======================================
//Dash Pattern 06 Register
//======================================
#define ITE_VG_REG_DPR06_BASE   0x05C
//
// D[23:0] Dash Line Pattern 06 (MSB: Enable bit, s12.11)
//
#define ITE_VG_CMDSHIFT_DASHPAT06   0
#define ITE_VG_CMDMASK_DASHPAT06    0x00FFFFFF
//======================================
//Dash Pattern 06 Register End
//======================================


//======================================
//Dash Pattern 07 Register
//======================================
#define ITE_VG_REG_DPR07_BASE   0x060
//
// D[23:0] Dash Line Pattern 07 (MSB: Enable bit, s12.11)
//
#define ITE_VG_CMDSHIFT_DASHPAT07   0
#define ITE_VG_CMDMASK_DASHPAT07    0x00FFFFFF
//======================================
//Dash Pattern 07 Register End
//======================================


//======================================
//Dash Pattern 08 Register
//======================================
#define ITE_VG_REG_DPR08_BASE   0x064
//
// D[23:0] Dash Line Pattern 08 (MSB: Enable bit, s12.11)
//
#define ITE_VG_CMDSHIFT_DASHPAT08   0
#define ITE_VG_CMDMASK_DASHPAT08    0x00FFFFFF
//======================================
//Dash Pattern 08 Register End
//======================================


//======================================
//Dash Pattern 09 Register
//======================================
#define ITE_VG_REG_DPR09_BASE   0x068
//
// D[23:0] Dash Line Pattern 09 (MSB: Enable bit, s12.11)
//
#define ITE_VG_CMDSHIFT_DASHPAT09   0
#define ITE_VG_CMDMASK_DASHPAT09    0x00FFFFFF
//======================================
//Dash Pattern 09 Register End
//======================================


//======================================
//Dash Pattern 10 Register
//======================================
#define ITE_VG_REG_DPR10_BASE   0x06C
//
// D[23:0] Dash Line Pattern 10 (MSB: Enable bit, s12.11)
//
#define ITE_VG_CMDSHIFT_DASHPAT10   0
#define ITE_VG_CMDMASK_DASHPAT10    0x00FFFFFF
//======================================
//Dash Pattern 10 Register End
//======================================


//======================================
//Dash Pattern 11 Register
//======================================
#define ITE_VG_REG_DPR11_BASE   0x070
//
// D[23:0] Dash Line Pattern 11 (MSB: Enable bit, s12.11)
//
#define ITE_VG_CMDSHIFT_DASHPAT11   0
#define ITE_VG_CMDMASK_DASHPAT11    0x00FFFFFF
//======================================
//Dash Pattern 11 Register End
//======================================


//======================================
//Dash Pattern 12 Register
//======================================
#define ITE_VG_REG_DPR12_BASE   0x074
//
// D[23:0] Dash Line Pattern 12 (MSB: Enable bit, s12.11)
//
#define ITE_VG_CMDSHIFT_DASHPAT12   0
#define ITE_VG_CMDMASK_DASHPAT12    0x00FFFFFF
//======================================
//Dash Pattern 12 Register End
//======================================


//======================================
//Dash Pattern 13 Register
//======================================
#define ITE_VG_REG_DPR13_BASE   0x078
//
// D[23:0] Dash Line Pattern 13 (MSB: Enable bit, s12.11)
//
#define ITE_VG_CMDSHIFT_DASHPAT13   0
#define ITE_VG_CMDMASK_DASHPAT13    0x00FFFFFF
//======================================
//Dash Pattern 13 Register End
//======================================


//======================================
//Dash Pattern 14 Register
//======================================
#define ITE_VG_REG_DPR14_BASE   0x07C
//
// D[23:0] Dash Line Pattern 14 (MSB: Enable bit, s12.11)
//
#define ITE_VG_CMDSHIFT_DASHPAT14   0
#define ITE_VG_CMDMASK_DASHPAT14    0x00FFFFFF
//======================================
//Dash Pattern 14 Register End
//======================================


//======================================
//Dash Pattern 15 Register
//======================================
#define ITE_VG_REG_DPR15_BASE   0x080
//
// D[23:0] Dash Line Pattern 15 (MSB: Enable bit, s12.11)
//
#define ITE_VG_CMDSHIFT_DASHPAT15   0
#define ITE_VG_CMDMASK_DASHPAT15    0x00FFFFFF
//======================================
//Dash Pattern 15 Register End
//======================================


//======================================
//Dash Pattern 16 Register
//======================================
#define ITE_VG_REG_DPR16_BASE   0x084
//
// D[23:0] Dash Line Pattern 16 (MSB: Enable bit, s12.11)
//
#define ITE_VG_CMDSHIFT_DASHPAT16   0
#define ITE_VG_CMDMASK_DASHPAT16    0x00FFFFFF
//======================================
//Dash Pattern 16 Register End
//======================================


//Reserved Register 0x088
//Reserved Register 0x08C


//======================================
//Coverage Control Register
//======================================
#define ITE_VG_REG_CCR_BASE     0x090

//
// D[31]
//
#define ITE_VG_CMD_CLIPFLIP_EN	0x80000000

//
// D[30]
//
#define ITE_VG_CMD_DSTFLIP_EN	0x40000000

//
// D[29]
//
#define ITE_VG_CMD_SRCFLIP_EN	0x20000000

//
// D[28]
//
#define ITE_VG_CMD_SCREENFLIP_EN	0x10000000

//
// D[7]
//
#define ITE_VG_CMD_TIMERDY_EN   0x00000080

//
// D[6]
//
#define ITE_VG_CMD_FULLRDY_EN   0x00000040

//
// D[5] No clear valid plane
//
#define ITE_VG_CMD_NOCLRVALID   0x00000020

//
// D[4] Enable plan clipping
//
#define ITE_VG_CMD_CLIPPING     0x00000010

//
// D[3:2] Render quality selection 
// 00: Non-anti aliasing (Nearest) 
// 01: Faster (4xAA) 
// 10: Better (16xAA) 
// Note: When the quality is better, the coverage plane data format MUST be two bytes (s11.4).
//
#define ITE_VG_CMD_RENDERQ_NONAA    0x00000000
#define ITE_VG_CMD_RENDERQ_FASTER   0x00000004
#define ITE_VG_CMD_RENDERQ_BETTER   0x00000008
#define ITE_VG_CMDSHIFT_RENDERQ     2
#define ITE_VG_CMDMASK_RENDERQ      0x0000000C

//
// D[0] Coverage plane data format
// 0: one byte (s5.2) 
// 1: two bytes (s11.4)
//
#define ITE_VG_CMD_PLANFORMAT_ONEBYTE      0x00000000
#define ITE_VG_CMD_PLANFORMAT_TWOBYTES     0x00000001
//======================================
//Coverage Control Register End
//======================================


//======================================
//Coverage Plan Base Register
//======================================
#define ITE_VG_REG_CPBR_BASE    0x094
//
// D[31:0] Coverage Plane Base Address. (64 bits alignment)
//
#define ITE_VG_CMDSHIFT_PLANBASE   0
#define ITE_VG_CMDMASK_PLANBASE    0xFFFFFFFF
//======================================
//Coverage Plan Base Register End
//======================================


//======================================
//Coverage/Valid Plan Pitch Register
//======================================
#define ITE_VG_REG_CVPPR_BASE   0x098
//
// D[24:16] Valid Plane Pitch in bytes. (64 bits alignment)
//
#define ITE_VG_CMDSHIFT_VALIDPITCH   16
#define ITE_VG_CMDMASK_VALIDPITCH    0x01FF0000

//
// D[13:0] Coverage Plane Pitch in bytes. (64 bits alignment)
//
#define ITE_VG_CMDSHIFT_PLANPITCH   0
#define ITE_VG_CMDMASK_PLANPITCH    0x00003FFF
//======================================
//Coverage/Valid Plan Pitch Register End
//======================================


//======================================
//Valid Plan Base Register
//======================================
#define ITE_VG_REG_VPBR_BASE    0x09C
//
// D[31:0] Valid Plane Base Address. (64 bits alignment)
//
#define ITE_VG_CMDSHIFT_VALIDBASE   0
#define ITE_VG_CMDMASK_VALIDBASE    0xFFFFFFFF
//======================================
//Valid Plan Base Register End
//======================================


//======================================
//Plan X Clipping Register
//======================================
#define ITE_VG_REG_PXCR_BASE    0x0A0
//
// D[28:16] Clipping x direction after XEND (s12)
//
#define ITE_VG_CMDSHIFT_PXCR_CLIPXEND   16
#define ITE_VG_CMDMASK_PXCR_CLIPXEND    0x1FFF0000

//
// D[12:0] Clipping x direction before XSTART (s12)
//
#define ITE_VG_CMDSHIFT_PXCR_CLIPXSTART   0
#define ITE_VG_CMDMASK_PXCR_CLIPXSTART    0x00001FFF
//======================================
//Plan X Clipping Register End
//======================================


//======================================
//Plan Y Clipping Register
//======================================
#define ITE_VG_REG_PYCR_BASE    0x0A4
//
// D[28:16] Clipping y direction after YEND (s12)
//
#define ITE_VG_CMDSHIFT_PYCR_CLIPXEND   16
#define ITE_VG_CMDMASK_PYCR_CLIPXEND    0x1FFF0000

//
// D[12:0] Clipping y direction before YSTART (s12)
//
#define ITE_VG_CMDSHIFT_PYCR_CLIPXSTART   0
#define ITE_VG_CMDMASK_PYCR_CLIPXSTART    0x00001FFF
//======================================
//Plan Y Clipping Register End
//======================================
#define ITE_VG_REG_SFR_BASE    0x0A8

//
// D[28:16]
//
#define ITE_VG_CMDSHIFT_DSTFLIP_H	16
#define ITE_VG_CMDMASK_DSTFLIP_H	0x1FFF0000

//
// D[12:0]
//
#define ITE_VG_CMDSHIFT_SRCFLIP_H	0
#define ITE_VG_CMDMASK_SRCFLIP_H	0x1FFF

//======================================
//Arbiter Control Register
//======================================
#define ITE_VG_REG_ACR_BASE     0x0AC

//
// D[24] Enable Last between Rd and Wr Rq to Memory
//
#define ITE_VG_CMD_RDWRLAST     0x01000000

//
// D[23:22] Read priority 3 of arbiter. 
// 00: FrontEnd NO cache reads. 
// 01: FrontEnd cache reads. 
// 10: BackEnd NO cache reads. 
// 11: BackEnd cache reads.
//
#define ITE_VG_CMD_RDPRIORITY3_FE_NOCACHE   0x00000000
#define ITE_VG_CMD_RDPRIORITY3_FE_CACHE     0x00400000
#define ITE_VG_CMD_RDPRIORITY3_BE_NOCACHE   0x00800000
#define ITE_VG_CMD_RDPRIORITY3_BE_CACHE     0x00C00000
#define ITE_VG_CMDSHIFT_RDPRIORITY3         22
#define ITE_VG_CMDMASK_RDPRIORITY3          0x00C00000

//
// D[21:20] Write priority 2 of arbiter. 
// 00: FrontEnd NO cache writes. 
// 01: FrontEnd cache writes. 
// 10: BackEnd NO cache writes.
// 11: BackEnd cache reads.
//
#define ITE_VG_CMD_WRPRIORITY2_FE_NOCACHE   0x00000000
#define ITE_VG_CMD_WRPRIORITY2_FE_CACHE     0x00100000
#define ITE_VG_CMD_WRPRIORITY2_BE_NOCACHE   0x00200000
#define ITE_VG_CMD_WRPRIORITY2_BE_CACHE     0x00300000
#define ITE_VG_CMDSHIFT_WRPRIORITY2         20
#define ITE_VG_CMDMASK_WRPRIORITY2          0x00300000

//
// D[19:18] Write priority 1 of arbiter. 
// 00: FrontEnd NO cache writes. 
// 01: FrontEnd cache writes. 
// 10: BackEnd NO cache writes.
// 11: BackEnd cache reads.
//
#define ITE_VG_CMD_WRPRIORITY1_FE_NOCACHE   0x00000000
#define ITE_VG_CMD_WRPRIORITY1_FE_CACHE     0x00040000
#define ITE_VG_CMD_WRPRIORITY1_BE_NOCACHE   0x00080000
#define ITE_VG_CMD_WRPRIORITY1_BE_CACHE     0x000C0000
#define ITE_VG_CMDSHIFT_WRPRIORITY1         18
#define ITE_VG_CMDMASK_WRPRIORITY1          0x000C0000


//
// D[17:16] Write priority 0 of arbiter. 
// 00: FrontEnd NO cache writes. 
// 01: FrontEnd cache writes. 
// 10: BackEnd NO cache writes.
// 11: BackEnd cache reads.
//
#define ITE_VG_CMD_WRPRIORITY0_FE_NOCACHE   0x00000000
#define ITE_VG_CMD_WRPRIORITY0_FE_CACHE     0x00010000
#define ITE_VG_CMD_WRPRIORITY0_BE_NOCACHE   0x00020000
#define ITE_VG_CMD_WRPRIORITY0_BE_CACHE     0x00030000
#define ITE_VG_CMDSHIFT_WRPRIORITY0         16
#define ITE_VG_CMDMASK_WRPRIORITY0          0x00030000

//
// D[15:12] Control Max buffer size (For debug)
//
#define ITE_VG_CMDSHIFT_BUFBOUNDARY   12
#define ITE_VG_CMDMASK_BUFBOUNDARY    0x0000F000

//
// D[8] Enable buffer boundary
//
#define ITE_VG_CMD_ENBUFBOUND     0x00000100

//
// D[3] Enable clear render cache at first
//
#define ITE_VG_CMD_CLRRDRCACHE     0x00000008

//
// D[2] Enable clear coverage cache at first
//
#define ITE_VG_CMD_CLRCOVCACHE     0x00000004

//
// D[1] Enable render cache
//
#define ITE_VG_CMD_RDRCACHE_EN     0x00000002

//
// D[0] Enable coverage cache
//
#define ITE_VG_CMD_COVCACHE_EN     0x00000001
//======================================
//Arbiter Control Register End
//======================================

//======================================
//Render Control Register
//======================================
#define ITE_VG_REG_RCR_BASE     0x0B0
//
// D[31] Enable pre-multiplied for Destination Image
//
#define ITE_VG_CMD_DST_PRE_EN     0x80000000

//
// D[30] Enable Nonpre-multiplied before Color Transform
//
#define ITE_VG_CMD_SRC_NONPRE_EN     0x40000000

//
// D[29] Enable pre-multiplied before Blending
//
#define ITE_VG_CMD_SRC_PRE_EN     0x20000000

//
// D[28] Enable pre-multiplied for Texture Image
//
#define ITE_VG_CMD_TEX_PRE_EN     0x10000000

//
// D[27] Enable nonpre-multipled for writing image
//
#define ITE_VG_CMD_WR_NONPRE_EN     0x08000000

//
// D[26] Enable Lookup Table. (Image filter) (Only work at RENDMODE=11)
//
#define ITE_VG_CMD_LOOKUP_EN     0x04000000

//
// D[25] Enable Color combination. (Image filter)
//
#define ITE_VG_CMD_COLCOMBIN_EN     0x02000000

//
// D[24] Enable dither. (For output is not RGBA8888)
//
#define ITE_VG_CMD_DITHER_EN     0x01000000

//
// D[23] Paint color for valid pixels in Transparent BitBlt
//
#define ITE_VG_CMD_TBITBLT_PAINT     0x00800000

//
// D[22] Enable Transparent BitBlt. Only work at Blend disable
//
#define ITE_VG_CMD_TBITBLT_EN     0x00400000

//
// D[21] Enable ROP3. Only work at Blend disable
//
#define ITE_VG_CMD_ROP3_EN     0x00200000

//
// D[20] Enable blend. (Only work at destination enable)
//
#define ITE_VG_CMD_BLEND_EN     0x00100000

//
// D[19] Enable gamma or inverse gamma
//
#define ITE_VG_CMD_GAMMA_EN     0x00080000

//
// D[18] Enable color transform
//
#define ITE_VG_CMD_COLORCLIP_EN     0x00040000

//
// D[17] Enable clipping value to 8 bits per channel in color transform.
// If disable, you need to determine which 16 bits of 25 bits 
// are chosen at COLLSBSHIFT.
//
#define ITE_VG_CMD_COLORXFM_EN     0x00020000

//
// D[16] Enable mask read
//
#define ITE_VG_CMD_MASK_EN			0x00010000

//
// D[15] Enable destination paint color. Only work at Destination read enable.
//
#define ITE_VG_CMD_DSTCOLOR_EN		0x00008000

//
// D[14] Enable destination read
//
#define ITE_VG_CMD_DESTINATION_EN     0x00004000

//
// D[13] Enable texture cache
//
#define ITE_VG_CMD_TEXCACHE_EN     0x00002000

//
// D[12] Enable texture read
//
#define ITE_VG_CMD_TEXTURE_EN     0x00001000

//
// D[11] Enable separate input data for texture & paint,
//       only at 16 bits per channel
//
#define ITE_VG_CMD_TEXPATIN_EN		0x00000800

//
// D[10] Enable Enable merge texture and paint to destination,
//       only at 16 bits per channel and blend disable.
//
#define ITE_VG_CMD_BLDMERGE_EN		0x00000400

//
// D[9] Enable scissor read
//
#define ITE_VG_CMD_SCISSOR_EN     0x00000200

//
// D[8] Enable coverage read
//
#define ITE_VG_CMD_COVERAGE_EN     0x00000100

//
// D[7:6] Texture image quality selection: 
// 00: Non-anti aliasing (Nearest) 
// 01: Faster (bilinear) 
// 10: Better (like-trilinear)
//
#define ITE_VG_CMD_IMAGEQ_NONAA      0x00000000
#define ITE_VG_CMD_IMAGEQ_FASTER     0x00000040
#define ITE_VG_CMD_IMAGEQ_BETTER     0x00000080
#define ITE_VG_CMDSHIFT_IMAGEQ       6
#define ITE_VG_CMDMASK_IMAGEQ        0x000000C0

//
// D[4] Render fill rule selection: 
// 0: odd-even 
// 1: non-zero
//
#define ITE_VG_CMD_FILLRULE_ODDEVEN      0x00000000
#define ITE_VG_CMD_FILLRULE_NONZERO      0x00000010

//
// D[3:2] Render Mode Selection: 
// 00: vgDrawPath vgDrawImage (Paint Pattern, 3pass-Step1) 
// 01: vgDrawImage (Except: Paint Pattern) 
//     vgDrawImage (Paint Pattern, 2pass-Step2) 
//     vgDrawImage (Paint Pattern, 3pass-Step3) 
// 10: vgDrawImage (Paint Pattern, 2pass-Step1) 
// 11: vgDrawImage (Paint Pattern, 3pass-Step2)
//
#define ITE_VG_CMD_RENDERMODE_0     0x00000000
#define ITE_VG_CMD_RENDERMODE_1     0x00000004
#define ITE_VG_CMD_RENDERMODE_2     0x00000008
#define ITE_VG_CMD_RENDERMODE_3     0x0000000C
#define ITE_VG_CMDSHIFT_RENDERMODE  2
#define ITE_VG_CMDMASK_RENDERMODE   0x0000000C

//
// D[1] Enable Coordinate operation at valid.
//
#define ITE_VG_CMD_RDPLN_VLD_EN		0x00000002

//
// D[0] Enable Fast Source Copy. (Byte alignment)
//
#define ITE_VG_CMD_SRCCOPY_EN		0x00000001
//======================================
//Render Control Register End
//======================================


//======================================
//Render Mode Register
//======================================
#define ITE_VG_REG_RMR_BASE     0x0B4
//
// D[31] Enable clipping premultiple at [0, a]
//
#define ITE_VG_CMD_CLIPPRE_EN     0x80000000

//
// D[30] Enable scan reversed X direction
//
#define ITE_VG_CMD_SCANREXDIR_EN     0x40000000

//
// D[29] Enable scan reversed Y direction
//
#define ITE_VG_CMD_SCANREYDIR_EN     0x20000000

//
// D[28] Enable auto to detect the direction of scan
//
#define ITE_VG_CMD_AUTOSCAN_EN     0x10000000

//
// D[27:24] Color transformation LSB shift selection. 
// Note: Only work at COLORXFM enable and COLORCLIP disable.
//
#define ITE_VG_CMDSHIFT_COLLSBSHIFT   24
#define ITE_VG_CMDMASK_COLLSBSHIFT    0x0F000000

//
// D[22:20] texture transformation LSB shift selection. 
// Note: Only work at COLORXFM enable and COLORCLIP disable.
//
#define ITE_VG_CMDSHIFT_TEXLSBSHIFT   20
#define ITE_VG_CMDMASK_TEXLSBSHIFT    0x00700000

//
// D[17:16] Lookup Mode (Color Combination Mode) 
// 00: R channel 
// 01: G channel 
// 10: B channel 
// 11: A channel
//
#define ITE_VG_CMD_LOOKUPMODE_R     0x00000000
#define ITE_VG_CMD_LOOKUPMODE_G     0x00010000
#define ITE_VG_CMD_LOOKUPMODE_B     0x00020000
#define ITE_VG_CMD_LOOKUPMODE_A     0x00030000
#define ITE_VG_CMDSHIFT_LOOKUPMODE  16
#define ITE_VG_CMDMASK_LOOKUPMODE   0x00030000

//
// D[15:14] Masking Mode 
// 00: Union 
// 01: Intersect (Normal) 
// 10: Subtract 
// 11: Intersect (Normal)
//
#define ITE_VG_CMD_MASKMODE_UNION         0x00000000
#define ITE_VG_CMD_MASKMODE_INTERSECT     0x00004000
#define ITE_VG_CMD_MASKMODE_SUBTRACT      0x00008000
#define ITE_VG_CMD_MASKMODE_INTERSECT2    0x0000C000
#define ITE_VG_CMDSHIFT_MASKMODE          14
#define ITE_VG_CMDMASK_MASKMODE           0x0000C000

//
// D[13] Gamma Mode 
// 0: gamma 
// 1: inverse gamma
//
#define ITE_VG_CMD_GAMMAMODE_GAMMA      0x00000000
#define ITE_VG_CMD_GAMMAMODE_INVERSE    0x00002000

//
// D[12:8] Blending Mode. 
// 0x00: Src 
// 0x01: Src over Dst 
// 0x02: Dst over Src 
// 0x03: Src in Dst 
// 0x04: Dst in Src 
// 0x05: Multiply 
// 0x06: Screen 
// 0x07: Darken 
// 0x08: Lighten 
// 0x09: Additive 
// 0x0A: Constant Alpha 
// 0x10: Union 
// 0x11: Intersect 
// 0x12: Subtract
//
#define ITE_VG_CMD_BLENDMODE_SRC              0x00000000
#define ITE_VG_CMD_BLENDMODE_SRC_OVER_DST     0x00000100
#define ITE_VG_CMD_BLENDMODE_DST_OVER_SRC     0x00000200
#define ITE_VG_CMD_BLENDMODE_SRC_IN_DST       0x00000300
#define ITE_VG_CMD_BLENDMODE_DST_IN_SRC       0x00000400
#define ITE_VG_CMD_BLENDMODE_MULTIPLY         0x00000500
#define ITE_VG_CMD_BLENDMODE_SCREEN           0x00000600
#define ITE_VG_CMD_BLENDMODE_DARKEN           0x00000700
#define ITE_VG_CMD_BLENDMODE_LIGHTEN          0x00000800
#define ITE_VG_CMD_BLENDMODE_ADDITIVE         0x00000900
#define ITE_VG_CMD_BLENDMODE_CONSTANT_ALPHA   0x00000A00
#define ITE_VG_CMD_BLENDMODE_UNION            0x00001000
#define ITE_VG_CMD_BLENDMODE_INTERSECT        0x00001100
#define ITE_VG_CMD_BLENDMODE_SUBTRACT         0x00001200
#define ITE_VG_CMDSHIFT_BLENDMODE             8
#define ITE_VG_CMDMASK_BLENDMODE              0x00001F00

//
// D[7:6] Pattern Paint Tile Mode
// 00: Fill 
// 01: Pad 
// 10: Repeat 
// 11: Reflect
//
#define ITE_VG_CMD_TILEMODE_FILL         0x00000000
#define ITE_VG_CMD_TILEMODE_PAD          0x00000040
#define ITE_VG_CMD_TILEMODE_REPEAT       0x00000080
#define ITE_VG_CMD_TILEMODE_REFLECT      0x000000C0
#define ITE_VG_CMDSHIFT_TILEMODE         6
#define ITE_VG_CMDMASK_TILEMODE          0x000000C0

//
// D[5:4] Gradient Ramp Mode 
// 00: Pad 
// 01: Repeat 
// 10: Reflect
//
#define ITE_VG_CMD_RAMPMODE_PAD          0x00000000
#define ITE_VG_CMD_RAMPMODE_REPEAT       0x00000010
#define ITE_VG_CMD_RAMPMODE_REFLECT      0x00000020
#define ITE_VG_CMDSHIFT_RAMPMODE         4
#define ITE_VG_CMDMASK_RAMPMODE          0x00000030

//
// D[3:2] Paint Type 
// 00: Color 
// 01: Linear gradient 
// 10: Radial gradient 
// 11: Pattern
//
#define ITE_VG_CMD_PAINTTYPE_COLOR         0x00000000
#define ITE_VG_CMD_PAINTTYPE_LINEAR        0x00000004
#define ITE_VG_CMD_PAINTTYPE_RADIAL        0x00000008
#define ITE_VG_CMD_PAINTTYPE_PATTERN       0x0000000C
#define ITE_VG_CMDSHIFT_PAINTTYPE          2
#define ITE_VG_CMDMASK_PAINTTYPE           0x0000000C

//
// D[1:0] Image Mode 
// 00: Normal 
// 01: Multiply 
// 10: Stencil
//
#define ITE_VG_CMD_IMAGEMODE_NORMAL         0x00000000
#define ITE_VG_CMD_IMAGEMODE_MULTIPLY       0x00000001
#define ITE_VG_CMD_IMAGEMODE_STENCIL        0x00000002
#define ITE_VG_CMDSHIFT_IMAGEMODE           0
#define ITE_VG_CMDMASK_IMAGEMODE            0x00000003

//======================================
//Render Mode Register End
//======================================


//======================================
//Render Format Register
//======================================
#define ITE_VG_REG_RFR_BASE     0x0B8
//
// D[26] Enable Mask extend bits. 
// 0: extend 0. 
// 1: Let MSB be extended bits. 
// Note: As read destination extend at lookup table.
//
#define ITE_VG_CMD_MASKEXTEND_EN    0x04000000

//
// D[25] Enable Destination extend bits. 
// 0: extend 0. 
// 1: Let MSB be extended bits.
//
#define ITE_VG_CMD_DSTEXTEND_EN    0x02000000

//
// D[24] Enable Source (Texture) extend bits. 
// 0: extend 0. 
// 1: Let MSB be extended bits.
//
#define ITE_VG_CMD_SRCEXTEND_EN    0x01000000

//
// D[23:16] Mask data format
/* RGB{A,X} channel ordering */
//  HW_sRGBX_8888                               =  0,
//  HW_sRGBA_8888                               =  1,
//  HW_sRGBA_8888_PRE                           =  2,
//  HW_sRGB_565                                 =  3,
//  HW_sRGBA_5551                               =  4,
//  HW_sRGBA_4444                               =  5,
//  HW_sL_8                                     =  6,
//  HW_lRGBX_8888                               =  7,
//  HW_lRGBA_8888                               =  8,
//  HW_lRGBA_8888_PRE                           =  9,
//  HW_lL_8                                     = 10,
//  HW_A_8                                      = 11,
//  HW_BW_1                                     = 12,
//  HW_A_1                                      = 13,
//  HW_A_4                                      = 14,
//  HW_RGBA_16								    = 15,
//
/* {A,X}RGB channel ordering */
//  HW_sXRGB_8888                               =  0 | (1 << 6),
//  HW_sARGB_8888                               =  1 | (1 << 6),
//  HW_sARGB_8888_PRE                           =  2 | (1 << 6),
//  HW_sARGB_1555                               =  4 | (1 << 6),
//  HW_sARGB_4444                               =  5 | (1 << 6),
//  HW_lXRGB_8888                               =  7 | (1 << 6),
//  HW_lARGB_8888                               =  8 | (1 << 6),
//  HW_lARGB_8888_PRE                           =  9 | (1 << 6),
//
/* BGR{A,X} channel ordering */
//  HW_sBGRX_8888                               =  0 | (1 << 7),
//  HW_sBGRA_8888                               =  1 | (1 << 7),
//  HW_sBGRA_8888_PRE                           =  2 | (1 << 7),
//  HW_sBGR_565                                 =  3 | (1 << 7),
//  HW_sBGRA_5551                               =  4 | (1 << 7),
//  HW_sBGRA_4444                               =  5 | (1 << 7),
//  HW_lBGRX_8888                               =  7 | (1 << 7),
//  HW_lBGRA_8888                               =  8 | (1 << 7),
//  HW_lBGRA_8888_PRE                           =  9 | (1 << 7),
//
/* {A,X}BGR channel ordering */
//  HW_sXBGR_8888                               =  0 | (1 << 6) | (1 << 7),
//  HW_sABGR_8888                               =  1 | (1 << 6) | (1 << 7),
//  HW_sABGR_8888_PRE                           =  2 | (1 << 6) | (1 << 7),
//  HW_sABGR_1555                               =  4 | (1 << 6) | (1 << 7),
//  HW_sABGR_4444                               =  5 | (1 << 6) | (1 << 7),
//  HW_lXBGR_8888                               =  7 | (1 << 6) | (1 << 7),
//  HW_lABGR_8888                               =  8 | (1 << 6) | (1 << 7),
//  HW_lABGR_8888_PRE                           =  9 | (1 << 6) | (1 << 7),
//
#define ITE_VG_CMD_MASKFORMAT_sRGBX_8888         0x00000000
#define ITE_VG_CMD_MASKFORMAT_sRGBA_8888         1 << 16
#define ITE_VG_CMD_MASKFORMAT_sRGBA_8888_PRE     2 << 16
#define ITE_VG_CMD_MASKFORMAT_sRGB_565           3 << 16
#define ITE_VG_CMD_MASKFORMAT_sRGBA_5551         4 << 16
#define ITE_VG_CMD_MASKFORMAT_sRGBA_4444         5 << 16
#define ITE_VG_CMD_MASKFORMAT_sL_8               6 << 16
#define ITE_VG_CMD_MASKFORMAT_lRGBX_8888         7 << 16
#define ITE_VG_CMD_MASKFORMAT_lRGBA_8888         8 << 16
#define ITE_VG_CMD_MASKFORMAT_lRGBA_8888_PRE     9 << 16
#define ITE_VG_CMD_MASKFORMAT_lL_8               10 << 16
#define ITE_VG_CMD_MASKFORMAT_A_8                11 << 16
#define ITE_VG_CMD_MASKFORMAT_BW_1               12 << 16
#define ITE_VG_CMD_MASKFORMAT_A_1                13 << 16
#define ITE_VG_CMD_MASKFORMAT_A_4                14 << 16
#define ITE_VG_CMD_MASKFORMAT_RGBA_16            15 << 16

#define ITE_VG_CMD_MASKFORMAT_sXRGB_8888         (0 | (1 << 6)) << 16
#define ITE_VG_CMD_MASKFORMAT_sARGB_8888         (1 | (1 << 6)) << 16
#define ITE_VG_CMD_MASKFORMAT_sARGB_8888_PRE     (2 | (1 << 6)) << 16
#define ITE_VG_CMD_MASKFORMAT_sARGB_1555         (4 | (1 << 6)) << 16
#define ITE_VG_CMD_MASKFORMAT_sARGB_4444         (5 | (1 << 6)) << 16
#define ITE_VG_CMD_MASKFORMAT_lXRGB_8888         (7 | (1 << 6)) << 16
#define ITE_VG_CMD_MASKFORMAT_lARGB_8888         (8 | (1 << 6)) << 16
#define ITE_VG_CMD_MASKFORMAT_lARGB_8888_PRE     (9 | (1 << 6)) << 16

#define ITE_VG_CMD_MASKFORMAT_sBGRX_8888         (0 | (1 << 7)) << 16
#define ITE_VG_CMD_MASKFORMAT_sBGRA_8888         (1 | (1 << 7)) << 16
#define ITE_VG_CMD_MASKFORMAT_sBGRA_8888_PRE     (2 | (1 << 7)) << 16
#define ITE_VG_CMD_MASKFORMAT_sBGR_565           (4 | (1 << 7)) << 16
#define ITE_VG_CMD_MASKFORMAT_sBGRA_5551         (5 | (1 << 7)) << 16
#define ITE_VG_CMD_MASKFORMAT_lBGRX_8888         (7 | (1 << 7)) << 16
#define ITE_VG_CMD_MASKFORMAT_lBGRA_8888         (8 | (1 << 7)) << 16
#define ITE_VG_CMD_MASKFORMAT_lBGRA_8888_PRE     (9 | (1 << 7)) << 16

#define ITE_VG_CMD_MASKFORMAT_sXBGR_8888         (0 | (1 << 6) | (1 << 7)) << 16
#define ITE_VG_CMD_MASKFORMAT_sABGR_8888         (1 | (1 << 6) | (1 << 7)) << 16
#define ITE_VG_CMD_MASKFORMAT_sABGR_8888_PRE     (2 | (1 << 6) | (1 << 7)) << 16
#define ITE_VG_CMD_MASKFORMAT_sABGR_1555         (4 | (1 << 6) | (1 << 7)) << 16
#define ITE_VG_CMD_MASKFORMAT_sABGR_4444         (5 | (1 << 6) | (1 << 7)) << 16
#define ITE_VG_CMD_MASKFORMAT_lXBGR_8888         (7 | (1 << 6) | (1 << 7)) << 16
#define ITE_VG_CMD_MASKFORMAT_lABGR_8888         (8 | (1 << 6) | (1 << 7)) << 16
#define ITE_VG_CMD_MASKFORMAT_lABGR_8888_PRE     (9 | (1 << 6) | (1 << 7)) << 16

#define ITE_VG_CMDSHIFT_MASKFORMAT               16
#define ITE_VG_CMDMASK_MASKFORMAT                0x00FF0000


//
// D[15:8] Dst data format
/* RGB{A,X} channel ordering */
//  HW_sRGBX_8888                               =  0,
//  HW_sRGBA_8888                               =  1,
//  HW_sRGBA_8888_PRE                           =  2,
//  HW_sRGB_565                                 =  3,
//  HW_sRGBA_5551                               =  4,
//  HW_sRGBA_4444                               =  5,
//  HW_sL_8                                     =  6,
//  HW_lRGBX_8888                               =  7,
//  HW_lRGBA_8888                               =  8,
//  HW_lRGBA_8888_PRE                           =  9,
//  HW_lL_8                                     = 10,
//  HW_A_8                                      = 11,
//  HW_BW_1                                     = 12,
//  HW_A_1                                      = 13,
//  HW_A_4                                      = 14,
//  HW_RGBA_16								    = 15,
//
/* {A,X}RGB channel ordering */
//  HW_sXRGB_8888                               =  0 | (1 << 6),
//  HW_sARGB_8888                               =  1 | (1 << 6),
//  HW_sARGB_8888_PRE                           =  2 | (1 << 6),
//  HW_sARGB_1555                               =  4 | (1 << 6),
//  HW_sARGB_4444                               =  5 | (1 << 6),
//  HW_lXRGB_8888                               =  7 | (1 << 6),
//  HW_lARGB_8888                               =  8 | (1 << 6),
//  HW_lARGB_8888_PRE                           =  9 | (1 << 6),
//
/* BGR{A,X} channel ordering */
//  HW_sBGRX_8888                               =  0 | (1 << 7),
//  HW_sBGRA_8888                               =  1 | (1 << 7),
//  HW_sBGRA_8888_PRE                           =  2 | (1 << 7),
//  HW_sBGR_565                                 =  3 | (1 << 7),
//  HW_sBGRA_5551                               =  4 | (1 << 7),
//  HW_sBGRA_4444                               =  5 | (1 << 7),
//  HW_lBGRX_8888                               =  7 | (1 << 7),
//  HW_lBGRA_8888                               =  8 | (1 << 7),
//  HW_lBGRA_8888_PRE                           =  9 | (1 << 7),
//
/* {A,X}BGR channel ordering */
//  HW_sXBGR_8888                               =  0 | (1 << 6) | (1 << 7),
//  HW_sABGR_8888                               =  1 | (1 << 6) | (1 << 7),
//  HW_sABGR_8888_PRE                           =  2 | (1 << 6) | (1 << 7),
//  HW_sABGR_1555                               =  4 | (1 << 6) | (1 << 7),
//  HW_sABGR_4444                               =  5 | (1 << 6) | (1 << 7),
//  HW_lXBGR_8888                               =  7 | (1 << 6) | (1 << 7),
//  HW_lABGR_8888                               =  8 | (1 << 6) | (1 << 7),
//  HW_lABGR_8888_PRE                           =  9 | (1 << 6) | (1 << 7),
//
#define ITE_VG_CMD_DSTFORMAT_sRGBX_8888         0x00000000
#define ITE_VG_CMD_DSTFORMAT_sRGBA_8888         1 << 8
#define ITE_VG_CMD_DSTFORMAT_sRGBA_8888_PRE     2 << 8
#define ITE_VG_CMD_DSTFORMAT_sRGB_565           3 << 8
#define ITE_VG_CMD_DSTFORMAT_sRGBA_5551         4 << 8
#define ITE_VG_CMD_DSTFORMAT_sRGBA_4444         5 << 8
#define ITE_VG_CMD_DSTFORMAT_sL_8               6 << 8
#define ITE_VG_CMD_DSTFORMAT_lRGBX_8888         7 << 8
#define ITE_VG_CMD_DSTFORMAT_lRGBA_8888         8 << 8
#define ITE_VG_CMD_DSTFORMAT_lRGBA_8888_PRE     9 << 8
#define ITE_VG_CMD_DSTFORMAT_lL_8               10 << 8
#define ITE_VG_CMD_DSTFORMAT_A_8                11 << 8
#define ITE_VG_CMD_DSTFORMAT_BW_1               12 << 8
#define ITE_VG_CMD_DSTFORMAT_A_1                13 << 8
#define ITE_VG_CMD_DSTFORMAT_A_4                14 << 8
#define ITE_VG_CMD_DSTFORMAT_RGBA_16            15 << 8

#define ITE_VG_CMD_DSTFORMAT_sXRGB_8888         (0 | (1 << 6)) << 8
#define ITE_VG_CMD_DSTFORMAT_sARGB_8888         (1 | (1 << 6)) << 8
#define ITE_VG_CMD_DSTFORMAT_sARGB_8888_PRE     (2 | (1 << 6)) << 8
#define ITE_VG_CMD_DSTFORMAT_sARGB_1555         (4 | (1 << 6)) << 8
#define ITE_VG_CMD_DSTFORMAT_sARGB_4444         (5 | (1 << 6)) << 8
#define ITE_VG_CMD_DSTFORMAT_lXRGB_8888         (7 | (1 << 6)) << 8
#define ITE_VG_CMD_DSTFORMAT_lARGB_8888         (8 | (1 << 6)) << 8
#define ITE_VG_CMD_DSTFORMAT_lARGB_8888_PRE     (9 | (1 << 6)) << 8

#define ITE_VG_CMD_DSTFORMAT_sBGRX_8888         (0 | (1 << 7)) << 8
#define ITE_VG_CMD_DSTFORMAT_sBGRA_8888         (1 | (1 << 7)) << 8
#define ITE_VG_CMD_DSTFORMAT_sBGRA_8888_PRE     (2 | (1 << 7)) << 8
#define ITE_VG_CMD_DSTFORMAT_sBGR_565           (4 | (1 << 7)) << 8
#define ITE_VG_CMD_DSTFORMAT_sBGRA_5551         (5 | (1 << 7)) << 8
#define ITE_VG_CMD_DSTFORMAT_lBGRX_8888         (7 | (1 << 7)) << 8
#define ITE_VG_CMD_DSTFORMAT_lBGRA_8888         (8 | (1 << 7)) << 8
#define ITE_VG_CMD_DSTFORMAT_lBGRA_8888_PRE     (9 | (1 << 7)) << 8

#define ITE_VG_CMD_DSTFORMAT_sXBGR_8888         (0 | (1 << 6) | (1 << 7)) << 8
#define ITE_VG_CMD_DSTFORMAT_sABGR_8888         (1 | (1 << 6) | (1 << 7)) << 8
#define ITE_VG_CMD_DSTFORMAT_sABGR_8888_PRE     (2 | (1 << 6) | (1 << 7)) << 8
#define ITE_VG_CMD_DSTFORMAT_sABGR_1555         (4 | (1 << 6) | (1 << 7)) << 8
#define ITE_VG_CMD_DSTFORMAT_sABGR_4444         (5 | (1 << 6) | (1 << 7)) << 8
#define ITE_VG_CMD_DSTFORMAT_lXBGR_8888         (7 | (1 << 6) | (1 << 7)) << 8
#define ITE_VG_CMD_DSTFORMAT_lABGR_8888         (8 | (1 << 6) | (1 << 7)) << 8
#define ITE_VG_CMD_DSTFORMAT_lABGR_8888_PRE     (9 | (1 << 6) | (1 << 7)) << 8

#define ITE_VG_CMDSHIFT_DSTFORMAT               8
#define ITE_VG_CMDMASK_DSTFORMAT                0x0000FF00

//
// D[7:0] Src data format
/* RGB{A,X} channel ordering */
//  HW_sRGBX_8888                               =  0,
//  HW_sRGBA_8888                               =  1,
//  HW_sRGBA_8888_PRE                           =  2,
//  HW_sRGB_565                                 =  3,
//  HW_sRGBA_5551                               =  4,
//  HW_sRGBA_4444                               =  5,
//  HW_sL_8                                     =  6,
//  HW_lRGBX_8888                               =  7,
//  HW_lRGBA_8888                               =  8,
//  HW_lRGBA_8888_PRE                           =  9,
//  HW_lL_8                                     = 10,
//  HW_A_8                                      = 11,
//  HW_BW_1                                     = 12,
//  HW_A_1                                      = 13,
//  HW_A_4                                      = 14,
//  HW_RGBA_16								    = 15,
//
/* {A,X}RGB channel ordering */
//  HW_sXRGB_8888                               =  0 | (1 << 6),
//  HW_sARGB_8888                               =  1 | (1 << 6),
//  HW_sARGB_8888_PRE                           =  2 | (1 << 6),
//  HW_sARGB_1555                               =  4 | (1 << 6),
//  HW_sARGB_4444                               =  5 | (1 << 6),
//  HW_lXRGB_8888                               =  7 | (1 << 6),
//  HW_lARGB_8888                               =  8 | (1 << 6),
//  HW_lARGB_8888_PRE                           =  9 | (1 << 6),
//
/* BGR{A,X} channel ordering */
//  HW_sBGRX_8888                               =  0 | (1 << 7),
//  HW_sBGRA_8888                               =  1 | (1 << 7),
//  HW_sBGRA_8888_PRE                           =  2 | (1 << 7),
//  HW_sBGR_565                                 =  3 | (1 << 7),
//  HW_sBGRA_5551                               =  4 | (1 << 7),
//  HW_sBGRA_4444                               =  5 | (1 << 7),
//  HW_lBGRX_8888                               =  7 | (1 << 7),
//  HW_lBGRA_8888                               =  8 | (1 << 7),
//  HW_lBGRA_8888_PRE                           =  9 | (1 << 7),
//
/* {A,X}BGR channel ordering */
//  HW_sXBGR_8888                               =  0 | (1 << 6) | (1 << 7),
//  HW_sABGR_8888                               =  1 | (1 << 6) | (1 << 7),
//  HW_sABGR_8888_PRE                           =  2 | (1 << 6) | (1 << 7),
//  HW_sABGR_1555                               =  4 | (1 << 6) | (1 << 7),
//  HW_sABGR_4444                               =  5 | (1 << 6) | (1 << 7),
//  HW_lXBGR_8888                               =  7 | (1 << 6) | (1 << 7),
//  HW_lABGR_8888                               =  8 | (1 << 6) | (1 << 7),
//  HW_lABGR_8888_PRE                           =  9 | (1 << 6) | (1 << 7),
//
#define ITE_VG_CMD_SRCFORMAT_sRGBX_8888         0x00000000
#define ITE_VG_CMD_SRCFORMAT_sRGBA_8888         1
#define ITE_VG_CMD_SRCFORMAT_sRGBA_8888_PRE     2
#define ITE_VG_CMD_SRCFORMAT_sRGB_565           3
#define ITE_VG_CMD_SRCFORMAT_sRGBA_5551         4
#define ITE_VG_CMD_SRCFORMAT_sRGBA_4444         5
#define ITE_VG_CMD_SRCFORMAT_sL_8               6
#define ITE_VG_CMD_SRCFORMAT_lRGBX_8888         7
#define ITE_VG_CMD_SRCFORMAT_lRGBA_8888         8
#define ITE_VG_CMD_SRCFORMAT_lRGBA_8888_PRE     9
#define ITE_VG_CMD_SRCFORMAT_lL_8               10
#define ITE_VG_CMD_SRCFORMAT_A_8                11
#define ITE_VG_CMD_SRCFORMAT_BW_1               12
#define ITE_VG_CMD_SRCFORMAT_A_1                13
#define ITE_VG_CMD_SRCFORMAT_A_4                14 
#define ITE_VG_CMD_SRCFORMAT_RGBA_16            15 

#define ITE_VG_CMD_SRCFORMAT_sXRGB_8888         (0 | (1 << 6))
#define ITE_VG_CMD_SRCFORMAT_sARGB_8888         (1 | (1 << 6))
#define ITE_VG_CMD_SRCFORMAT_sARGB_8888_PRE     (2 | (1 << 6))
#define ITE_VG_CMD_SRCFORMAT_sARGB_1555         (4 | (1 << 6))
#define ITE_VG_CMD_SRCFORMAT_sARGB_4444         (5 | (1 << 6))
#define ITE_VG_CMD_SRCFORMAT_lXRGB_8888         (7 | (1 << 6))
#define ITE_VG_CMD_SRCFORMAT_lARGB_8888         (8 | (1 << 6))
#define ITE_VG_CMD_SRCFORMAT_lARGB_8888_PRE     (9 | (1 << 6))

#define ITE_VG_CMD_SRCFORMAT_sBGRX_8888         (0 | (1 << 7))
#define ITE_VG_CMD_SRCFORMAT_sBGRA_8888         (1 | (1 << 7))
#define ITE_VG_CMD_SRCFORMAT_sBGRA_8888_PRE     (2 | (1 << 7))
#define ITE_VG_CMD_SRCFORMAT_sBGR_565           (4 | (1 << 7))
#define ITE_VG_CMD_SRCFORMAT_sBGRA_5551         (5 | (1 << 7))
#define ITE_VG_CMD_SRCFORMAT_lBGRX_8888         (7 | (1 << 7))
#define ITE_VG_CMD_SRCFORMAT_lBGRA_8888         (8 | (1 << 7))
#define ITE_VG_CMD_SRCFORMAT_lBGRA_8888_PRE     (9 | (1 << 7))

#define ITE_VG_CMD_SRCFORMAT_sXBGR_8888         (0 | (1 << 6) | (1 << 7))
#define ITE_VG_CMD_SRCFORMAT_sABGR_8888         (1 | (1 << 6) | (1 << 7))
#define ITE_VG_CMD_SRCFORMAT_sABGR_8888_PRE     (2 | (1 << 6) | (1 << 7))
#define ITE_VG_CMD_SRCFORMAT_sABGR_1555         (4 | (1 << 6) | (1 << 7))
#define ITE_VG_CMD_SRCFORMAT_sABGR_4444         (5 | (1 << 6) | (1 << 7))
#define ITE_VG_CMD_SRCFORMAT_lXBGR_8888         (7 | (1 << 6) | (1 << 7))
#define ITE_VG_CMD_SRCFORMAT_lABGR_8888         (8 | (1 << 6) | (1 << 7))
#define ITE_VG_CMD_SRCFORMAT_lABGR_8888_PRE     (9 | (1 << 6) | (1 << 7))

#define ITE_VG_CMDSHIFT_SRCFORMAT               0
#define ITE_VG_CMDMASK_SRCFORMAT                0x000000FF

//======================================
//Render Format Register End
//======================================


//======================================
//Ramp Stop Register 01
//======================================
#define ITE_VG_REG_RSR01_BASE   0x0BC
//
// D[31] Enable stop 1 valid 
//
#define ITE_VG_CMD_RAMPSTOP1VLD    0x80000000

//
// D[28:16] Ramp stop g value 1 (1.12)
//
#define ITE_VG_CMDSHIFT_RAMPSTOP1   16
#define ITE_VG_CMDMASK_RAMPSTOP1    0x1FFF0000

//
// D[15] Enable stop 0 valid 
//
#define ITE_VG_CMD_RAMPSTOP0VLD    0x00008000

//
// D[14] Enable gradient valid when g = stop 0 
//
#define ITE_VG_CMD_RAMPSTOP0EQ    0x00004000

//
// D[12:0] Ramp stop g value 0 (1.12)
//
#define ITE_VG_CMDSHIFT_RAMPSTOP0   0
#define ITE_VG_CMDMASK_RAMPSTOP0    0x00001FFF
//======================================
//Ramp Stop Register 01 End
//======================================

//======================================
//Ramp Stop Register 23
//======================================
#define ITE_VG_REG_RSR23_BASE   0x0C0
//
// D[31] Enable stop 3 valid
//
#define ITE_VG_CMD_RAMPSTOP3VLD    0x80000000

//
// D[28:16] Ramp stop g value 3 (1.12)
//
#define ITE_VG_CMDSHIFT_RAMPSTOP3   16
#define ITE_VG_CMDMASK_RAMPSTOP3    0x1FFF0000

//
// D[15] Enable stop 2 valid
//
#define ITE_VG_CMD_RAMPSTOP2VLD    0x00008000

//
// D[12:0] Ramp stop g value 2 (1.12)
//
#define ITE_VG_CMDSHIFT_RAMPSTOP2   0
#define ITE_VG_CMDMASK_RAMPSTOP2    0x00001FFF
//======================================
//Ramp Stop Register 23 End
//======================================

//======================================
//Ramp Stop Register 45
//======================================
#define ITE_VG_REG_RSR45_BASE   0x0C4
//
// D[31] Enable stop 5 valid
//
#define ITE_VG_CMD_RAMPSTOP5VLD    0x80000000

//
// D[28:16] Ramp stop g value 5 (1.12)
//
#define ITE_VG_CMDSHIFT_RAMPSTOP5   16
#define ITE_VG_CMDMASK_RAMPSTOP5    0x1FFF0000

//
// D[15] Enable stop 4 valid
//
#define ITE_VG_CMD_RAMPSTOP4VLD    0x00008000

//
// D[12:0] Ramp stop g value 4 (1.12)
//
#define ITE_VG_CMDSHIFT_RAMPSTOP4   0
#define ITE_VG_CMDMASK_RAMPSTOP4    0x00001FFF
//======================================
//Ramp Stop Register 45 End
//======================================


//======================================
//Ramp Stop Register 67
//======================================
#define ITE_VG_REG_RSR67_BASE   0x0C8
//
// D[31] Enable stop 7 valid
//
#define ITE_VG_CMD_RAMPSTOP7VLD    0x80000000

//
// D[28:16] Ramp stop g value 7 (1.12)
//
#define ITE_VG_CMDSHIFT_RAMPSTOP7   16
#define ITE_VG_CMDMASK_RAMPSTOP7    0x1FFF0000

//
// D[15] Enable stop 6 valid
//
#define ITE_VG_CMD_RAMPSTOP6VLD    0x00008000

//
// D[12:0] Ramp stop g value 6 (1.12)
//
#define ITE_VG_CMDSHIFT_RAMPSTOP6   0
#define ITE_VG_CMDMASK_RAMPSTOP6    0x00001FFF
//======================================
//Ramp Stop Register 67 End
//======================================

//======================================
//Ramp Color Register 00
//======================================
#define ITE_VG_REG_RCR00_BASE   0x0CC
//
// D[27:16] Ramp stop color 0 B (8.4 pre-multiplied) 
//
#define ITE_VG_CMDSHIFT_RAMPCOLOR0B   16
#define ITE_VG_CMDMASK_RAMPCOLOR0B    0x0FFF0000

//
// D[7:0] Ramp stop color 0 A (8) 
//
#define ITE_VG_CMDSHIFT_RAMPCOLOR0A   0
#define ITE_VG_CMDMASK_RAMPCOLOR0A    0x000000FF
//======================================
//Ramp Color Register 00 End
//======================================


//======================================
//Ramp Color Register 01
//======================================
#define ITE_VG_REG_RCR01_BASE   0x0D0
//
// D[27:16] Ramp stop color 0 R (8.4 pre-multiplied)
//
#define ITE_VG_CMDSHIFT_RAMPCOLOR0R   16
#define ITE_VG_CMDMASK_RAMPCOLOR0R    0x0FFF0000

//
// D[11:0] Ramp stop color 0 G (8.4 pre-multiplied)
//
#define ITE_VG_CMDSHIFT_RAMPCOLOR0G   0
#define ITE_VG_CMDMASK_RAMPCOLOR0G    0x00000FFF
//======================================
//Ramp Color Register 01 End
//======================================


//======================================
//Ramp Color Register 10
//======================================
#define ITE_VG_REG_RCR10_BASE   0x0D4
//
// D[27:16] Ramp stop color 1 B (8.4 pre-multiplied) 
//
#define ITE_VG_CMDSHIFT_RAMPCOLOR1B   16
#define ITE_VG_CMDMASK_RAMPCOLOR1B    0x0FFF0000

//
// D[7:0] Ramp stop color 1 A (8) 
//
#define ITE_VG_CMDSHIFT_RAMPCOLOR1A   0
#define ITE_VG_CMDMASK_RAMPCOLOR1A    0x000000FF
//======================================
//Ramp Color Register 10 End
//======================================


//======================================
//Ramp Color Register 11
//======================================
#define ITE_VG_REG_RCR11_BASE   0x0D8
//
// D[27:16] Ramp stop color 1 R (8.4 pre-multiplied)
//
#define ITE_VG_CMDSHIFT_RAMPCOLOR1R   16
#define ITE_VG_CMDMASK_RAMPCOLOR1R    0x0FFF0000

//
// D[11:0] Ramp stop color 1 G (8.4 pre-multiplied)
//
#define ITE_VG_CMDSHIFT_RAMPCOLOR1G   0
#define ITE_VG_CMDMASK_RAMPCOLOR1G    0x00000FFF
//======================================
//Ramp Color Register 11 End
//======================================


//======================================
//Ramp Color Register 20
//======================================
#define ITE_VG_REG_RCR20_BASE   0x0DC
//
// D[27:16] Ramp stop color 2 B (8.4 pre-multiplied) 
//
#define ITE_VG_CMDSHIFT_RAMPCOLOR2B   16
#define ITE_VG_CMDMASK_RAMPCOLOR2B    0x0FFF0000

//
// D[7:0] Ramp stop color 2 A (8) 
//
#define ITE_VG_CMDSHIFT_RAMPCOLOR2A   0
#define ITE_VG_CMDMASK_RAMPCOLOR2A    0x000000FF
//======================================
//Ramp Color Register 20 End
//======================================


//======================================
//Ramp Color Register 21
//======================================
#define ITE_VG_REG_RCR21_BASE   0x0E0
//
// D[27:16] Ramp stop color 2 R (8.4 pre-multiplied)
//
#define ITE_VG_CMDSHIFT_RAMPCOLOR2R   16
#define ITE_VG_CMDMASK_RAMPCOLOR2R    0x0FFF0000

//
// D[11:0] Ramp stop color 2 G (8.4 pre-multiplied)
//
#define ITE_VG_CMDSHIFT_RAMPCOLOR2G   0
#define ITE_VG_CMDMASK_RAMPCOLOR2G    0x00000FFF
//======================================
//Ramp Color Register 21 End
//======================================


//======================================
//Ramp Color Register 30
//======================================
#define ITE_VG_REG_RCR30_BASE   0x0E4
//
// D[27:16] Ramp stop color 3 B (8.4 pre-multiplied) 
//
#define ITE_VG_CMDSHIFT_RAMPCOLOR3B   16
#define ITE_VG_CMDMASK_RAMPCOLOR3B    0x0FFF0000

//
// D[7:0] Ramp stop color 3 A (8) 
//
#define ITE_VG_CMDSHIFT_RAMPCOLOR3A   0
#define ITE_VG_CMDMASK_RAMPCOLOR3A    0x000000FF
//======================================
//Ramp Color Register 30 End
//======================================


//======================================
//Ramp Color Register 31
//======================================
#define ITE_VG_REG_RCR31_BASE   0x0E8
//
// D[27:16] Ramp stop color 3 R (8.4 pre-multiplied)
//
#define ITE_VG_CMDSHIFT_RAMPCOLOR3R   16
#define ITE_VG_CMDMASK_RAMPCOLOR3R    0x0FFF0000

//
// D[11:0] Ramp stop color 3 G (8.4 pre-multiplied)
//
#define ITE_VG_CMDSHIFT_RAMPCOLOR3G   0
#define ITE_VG_CMDMASK_RAMPCOLOR3G    0x00000FFF
//======================================
//Ramp Color Register 31 End
//======================================


//======================================
//Ramp Color Register 40
//======================================
#define ITE_VG_REG_RCR40_BASE   0x0EC
//
// D[27:16] Ramp stop color 4 B (8.4 pre-multiplied) 
//
#define ITE_VG_CMDSHIFT_RAMPCOLOR4B   16
#define ITE_VG_CMDMASK_RAMPCOLOR4B    0x0FFF0000

//
// D[7:0] Ramp stop color 4 A (8) 
//
#define ITE_VG_CMDSHIFT_RAMPCOLOR4A   0
#define ITE_VG_CMDMASK_RAMPCOLOR4A    0x000000FF
//======================================
//Ramp Color Register 40 End
//======================================


//======================================
//Ramp Color Register 41
//======================================
#define ITE_VG_REG_RCR41_BASE   0x0F0
//
// D[27:16] Ramp stop color 4 R (8.4 pre-multiplied)
//
#define ITE_VG_CMDSHIFT_RAMPCOLOR4R   16
#define ITE_VG_CMDMASK_RAMPCOLOR4R    0x0FFF0000

//
// D[11:0] Ramp stop color 4 G (8.4 pre-multiplied)
//
#define ITE_VG_CMDSHIFT_RAMPCOLOR4G   0
#define ITE_VG_CMDMASK_RAMPCOLOR4G    0x00000FFF
//======================================
//Ramp Color Register 41 End
//======================================


//======================================
//Ramp Color Register 50
//======================================
#define ITE_VG_REG_RCR50_BASE   0x0F4
//
// D[27:16] Ramp stop color 5 B (8.4 pre-multiplied) 
//
#define ITE_VG_CMDSHIFT_RAMPCOLOR5B   16
#define ITE_VG_CMDMASK_RAMPCOLOR5B    0x0FFF0000

//
// D[7:0] Ramp stop color 5 A (8) 
//
#define ITE_VG_CMDSHIFT_RAMPCOLOR5A   0
#define ITE_VG_CMDMASK_RAMPCOLOR5A    0x000000FF
//======================================
//Ramp Color Register 50 End
//======================================


//======================================
//Ramp Color Register 51
//======================================
#define ITE_VG_REG_RCR51_BASE   0x0F8
//
// D[27:16] Ramp stop color 5 R (8.4 pre-multiplied)
//
#define ITE_VG_CMDSHIFT_RAMPCOLOR5R   16
#define ITE_VG_CMDMASK_RAMPCOLOR5R    0x0FFF0000

//
// D[11:0] Ramp stop color 5 G (8.4 pre-multiplied)
//
#define ITE_VG_CMDSHIFT_RAMPCOLOR5G   0
#define ITE_VG_CMDMASK_RAMPCOLOR5G    0x00000FFF
//======================================
//Ramp Color Register 51 End
//======================================


//======================================
//Ramp Color Register 60
//======================================
#define ITE_VG_REG_RCR60_BASE   0x0FC
//
// D[27:16] Ramp stop color 6 B (8.4 pre-multiplied) 
//
#define ITE_VG_CMDSHIFT_RAMPCOLOR6B   16
#define ITE_VG_CMDMASK_RAMPCOLOR6B    0x0FFF0000

//
// D[7:0] Ramp stop color 6 A (8) 
//
#define ITE_VG_CMDSHIFT_RAMPCOLOR6A   0
#define ITE_VG_CMDMASK_RAMPCOLOR6A    0x000000FF
//======================================
//Ramp Color Register 60 End
//======================================


//======================================
//Ramp Color Register 61
//======================================
#define ITE_VG_REG_RCR61_BASE   0x100
//
// D[27:16] Ramp stop color 6 R (8.4 pre-multiplied)
//
#define ITE_VG_CMDSHIFT_RAMPCOLOR6R   16
#define ITE_VG_CMDMASK_RAMPCOLOR6R    0x0FFF0000

//
// D[11:0] Ramp stop color 6 G (8.4 pre-multiplied)
//
#define ITE_VG_CMDSHIFT_RAMPCOLOR6G   0
#define ITE_VG_CMDMASK_RAMPCOLOR6G    0x00000FFF
//======================================
//Ramp Color Register 61 End
//======================================


//======================================
//Ramp Color Register 70
//======================================
#define ITE_VG_REG_RCR70_BASE   0x104
//
// D[27:16] Ramp stop color 7 B (8.4 pre-multiplied) 
//
#define ITE_VG_CMDSHIFT_RAMPCOLOR7B   16
#define ITE_VG_CMDMASK_RAMPCOLOR7B    0x0FFF0000

//
// D[7:0] Ramp stop color 7 A (8) 
//
#define ITE_VG_CMDSHIFT_RAMPCOLOR7A   0
#define ITE_VG_CMDMASK_RAMPCOLOR7A    0x000000FF
//======================================
//Ramp Color Register 70 End
//======================================


//======================================
//Ramp Color Register 71
//======================================
#define ITE_VG_REG_RCR71_BASE   0x108
//
// D[27:16] Ramp stop color 7 R (8.4 pre-multiplied)
//
#define ITE_VG_CMDSHIFT_RAMPCOLOR7R   16
#define ITE_VG_CMDMASK_RAMPCOLOR7R    0x0FFF0000

//
// D[11:0] Ramp stop color 7 G (8.4 pre-multiplied)
//
#define ITE_VG_CMDSHIFT_RAMPCOLOR7G   0
#define ITE_VG_CMDMASK_RAMPCOLOR7G    0x00000FFF
//======================================
//Ramp Color Register 71 End
//======================================


//======================================
//Ramp Divider Register 01
//======================================
#define ITE_VG_REG_RDR01_BASE   0x10C
//
// D[24:0] 2^24(1.24) / distant between Stop0 and Stop1 (1.12) 
// Result: (13.12)
//
#define ITE_VG_CMDSHIFT_RAMPDIVIDER01   0
#define ITE_VG_CMDMASK_RAMPDIVIDER01    0x01FFFFFF
//======================================
//Ramp Divider Register 01 End
//======================================


//======================================
//Ramp Divider Register 12
//======================================
#define ITE_VG_REG_RCR12_BASE   0x110
//
// D[24:0] 2^24(1.24) / distant between Stop1 and Stop2 (1.12) 
// Result: (13.12)
//
#define ITE_VG_CMDSHIFT_RAMPDIVIDER12   0
#define ITE_VG_CMDMASK_RAMPDIVIDER12    0x01FFFFFF
//======================================
//Ramp Divider Register 12 End
//======================================


//======================================
//Ramp Divider Register 23
//======================================
#define ITE_VG_REG_RDR23_BASE   0x114
//
// D[24:0] 2^24(1.24) / distant between Stop2 and Stop3 (1.12) 
// Result: (13.12)
//
#define ITE_VG_CMDSHIFT_RAMPDIVIDER23   0
#define ITE_VG_CMDMASK_RAMPDIVIDER23    0x01FFFFFF
//======================================
//Ramp Divider Register 23 End
//======================================


//======================================
//Ramp Divider Register 34
//======================================
#define ITE_VG_REG_RCR34_BASE   0x118
//
// D[24:0] 2^24(1.24) / distant between Stop3 and Stop4 (1.12) 
// Result: (13.12)
//
#define ITE_VG_CMDSHIFT_RAMPDIVIDER34   0
#define ITE_VG_CMDMASK_RAMPDIVIDER34    0x01FFFFFF
//======================================
//Ramp Divider Register 34 End
//======================================


//======================================
//Ramp Divider Register 45
//======================================
#define ITE_VG_REG_RDR45_BASE   0x11C
//
// D[24:0] 2^24(1.24) / distant between Stop4 and Stop5 (1.12) 
// Result: (13.12)
//
#define ITE_VG_CMDSHIFT_RAMPDIVIDER45   0
#define ITE_VG_CMDMASK_RAMPDIVIDER45    0x01FFFFFF
//======================================
//Ramp Divider Register 45 End
//======================================


//======================================
//Ramp Divider Register 56
//======================================
#define ITE_VG_REG_RCR56_BASE   0x120
//
// D[24:0] 2^24(1.24) / distant between Stop5 and Stop6 (1.12) 
// Result: (13.12)
//
#define ITE_VG_CMDSHIFT_RAMPDIVIDER56   0
#define ITE_VG_CMDMASK_RAMPDIVIDER56    0x01FFFFFF
//======================================
//Ramp Divider Register 56 End
//======================================


//======================================
//Ramp Divider Register 67
//======================================
#define ITE_VG_REG_RDR67_BASE   0x124
//
// D[24:0] 2^24(1.24) / distant between Stop6 and Stop7 (1.12) 
// Result: (13.12)
//
#define ITE_VG_CMDSHIFT_RAMPDIVIDER67   0
#define ITE_VG_CMDMASK_RAMPDIVIDER67    0x01FFFFFF
//======================================
//Ramp Divider Register 67 End
//======================================


//======================================
//Gradient Parameter Register A
//======================================
#define ITE_VG_REG_GPRA_BASE    0x128
//
// D[23:0]Linear & Radial gradient parameter A (s7.16)
//
#define ITE_VG_CMDSHIFT_GRADIENT0   0
#define ITE_VG_CMDMASK_GRADIENT0    0x00FFFFFF
//======================================
//Gradient Parameter Register A End
//======================================


//======================================
//Gradient Parameter Register B
//======================================
#define ITE_VG_REG_GPRB_BASE    0x12C
//
// D[23:0]Linear & Radial gradient parameter B (s7.16)
//
#define ITE_VG_CMDSHIFT_GRADIENT1   0
#define ITE_VG_CMDMASK_GRADIENT1    0x00FFFFFF
//======================================
//Gradient Parameter Register B End
//======================================


//======================================
//Gradient Parameter Register C
//======================================
#define ITE_VG_REG_GPRC_BASE    0x130
//
// D[23:0]Linear & Radial gradient parameter C (s7.16)
//
#define ITE_VG_CMDSHIFT_GRADIENT2   0
#define ITE_VG_CMDMASK_GRADIENT2    0x00FFFFFF
//======================================
//Gradient Parameter Register C End
//======================================


//======================================
//Gradient Parameter Register D0
//======================================
#define ITE_VG_REG_GPRD0_BASE   0x134
//
// D[13:0]Radial gradient parameter D, sign bit & integer part. (s13)
//
#define ITE_VG_CMDSHIFT_GRADIENT3A   0
#define ITE_VG_CMDMASK_GRADIENT3A    0x00003FFF
//======================================
//Gradient Parameter Register D0 End
//======================================


//======================================
//Gradient Parameter Register D1
//======================================
#define ITE_VG_REG_GPRD1_BASE   0x138
//
// D[23:0]Radial gradient parameter D, fix point part. (.24)
//
#define ITE_VG_CMDSHIFT_GRADIENT3B   0
#define ITE_VG_CMDMASK_GRADIENT3B    0x00FFFFFF
//======================================
//Gradient Parameter Register D1 End
//======================================


//======================================
//Gradient Parameter Register E0
//======================================
#define ITE_VG_REG_GPRE0_BASE   0x13C
//
// D[13:0]Radial gradient parameter E, sign bit & integer part. (s13)
//
#define ITE_VG_CMDSHIFT_GRADIENT4A   0
#define ITE_VG_CMDMASK_GRADIENT4A    0x00003FFF
//======================================
//Gradient Parameter Register E0 End
//======================================


//======================================
//Gradient Parameter Register E1
//======================================
#define ITE_VG_REG_GPRE1_BASE   0x140
//
// D[23:0]Radial gradient parameter E, fix point part. (.24)
//
#define ITE_VG_CMDSHIFT_GRADIENT4B   0
#define ITE_VG_CMDMASK_GRADIENT4B    0x00FFFFFF
//======================================
//Gradient Parameter Register E1 End
//======================================


//======================================
//Gradient Parameter Register F0
//======================================
#define ITE_VG_REG_GPRF0_BASE   0x144
//
// D[13:0]Radial gradient parameter F, sign bit & integer part. (s13)
//
#define ITE_VG_CMDSHIFT_GRADIENT5A   0
#define ITE_VG_CMDMASK_GRADIENT5A    0x00003FFF
//======================================
//Gradient Parameter Register F0 End
//======================================


//======================================
//Gradient Parameter Register F1
//======================================
#define ITE_VG_REG_GPRF1_BASE   0x148
//
// D[23:0]Radial gradient parameter F, fix point part. (.24)
//
#define ITE_VG_CMDSHIFT_GRADIENT5B   0
#define ITE_VG_CMDMASK_GRADIENT5B    0x00FFFFFF
//======================================
//Gradient Parameter Register F1 End
//======================================


//======================================
//Gradient Parameter Register G0
//======================================
#define ITE_VG_REG_GPRG0_BASE   0x14C
//
// D[13:0]Radial gradient parameter G, sign bit & integer part. (s13)
//
#define ITE_VG_CMDSHIFT_GRADIENT6A   0
#define ITE_VG_CMDMASK_GRADIENT6A    0x00003FFF
//======================================
//Gradient Parameter Register G0 End
//======================================


//======================================
//Gradient Parameter Register G1
//======================================
#define ITE_VG_REG_GPRG1_BASE   0x150
//
// D[23:0]Radial gradient parameter G, fix point part. (.24)
//
#define ITE_VG_CMDSHIFT_GRADIENT6B   0
#define ITE_VG_CMDMASK_GRADIENT6B    0x00FFFFFF
//======================================
//Gradient Parameter Register G1 End
//======================================


//======================================
//Gradient Parameter Register H0
//======================================
#define ITE_VG_REG_GPRH0_BASE   0x154
//
// D[13:0]Radial gradient parameter H, sign bit & integer part. (s13)
//
#define ITE_VG_CMDSHIFT_GRADIENT7A   0
#define ITE_VG_CMDMASK_GRADIENT7A    0x00003FFF
//======================================
//Gradient Parameter Register H0 End
//======================================


//======================================
//Gradient Parameter Register H1
//======================================
#define ITE_VG_REG_GPRH1_BASE   0x158
//
// D[23:0]Radial gradient parameter H, fix point part. (.24)
//
#define ITE_VG_CMDSHIFT_GRADIENT7B   0
#define ITE_VG_CMDMASK_GRADIENT7B    0x00FFFFFF
//======================================
//Gradient Parameter Register H1 End
//======================================


//======================================
//Gradient Parameter Register I0
//======================================
#define ITE_VG_REG_GPRI0_BASE   0x15C
//
// D[13:0]Radial gradient parameter I, sign bit & integer part. (s13)
//
#define ITE_VG_CMDSHIFT_GRADIENT8A   0
#define ITE_VG_CMDMASK_GRADIENT8A    0x00003FFF
//======================================
//Gradient Parameter Register I0 End
//======================================


//======================================
//Gradient Parameter Register I1
//======================================
#define ITE_VG_REG_GPRI1_BASE   0x160
//
// D[23:0]Radial gradient parameter I, fix point part. (.24)
//
#define ITE_VG_CMDSHIFT_GRADIENT8B   0
#define ITE_VG_CMDMASK_GRADIENT8B    0x00FFFFFF
//======================================
//Gradient Parameter Register I1 End
//======================================


//======================================
//Color Transform Register 0
//======================================
#define ITE_VG_REG_CTBR0_BASE    0x164

//
// D[23:0]Color transform parameter Br (s15.8)
//
#define ITE_VG_CMDSHIFT_COLXFM00   0
#define ITE_VG_CMDMASK_COLXFM00    0x00FFFFFF
//======================================
//Color Transform Register 0 End
//======================================


//======================================
//Color Transform Register 1
//======================================
#define ITE_VG_REG_CTBR1_BASE    0x168
//
// D[23:0]Color transform parameter Bg (s15.8)
//

#define ITE_VG_CMDSHIFT_COLXFM10   0
#define ITE_VG_CMDMASK_COLXFM10    0x00FFFFFF
//======================================
//Color Transform Register 1 End
//======================================


//======================================
//Color Transform Register 2
//======================================
#define ITE_VG_REG_CTBR2_BASE    0x16C
//
// D[23:0]Color transform parameter Bb (s15.8)
//

#define ITE_VG_CMDSHIFT_COLXFM20   0
#define ITE_VG_CMDMASK_COLXFM20    0x00FFFFFF
//======================================
//Color Transform Register 2 End
//======================================


//======================================
//Color Transform Register 3
//======================================
#define ITE_VG_REG_CTBR3_BASE    0x170
//
// D[23:0]Color transform parameter Ba (s15.8)
//

#define ITE_VG_CMDSHIFT_COLXFM30   0
#define ITE_VG_CMDMASK_COLXFM30    0x00FFFFFF
//======================================
//Color Transform Register 3 End
//======================================


//======================================
//Destination Coordinate Register
//======================================
#define ITE_VG_REG_DCR_BASE     0x174
//
// D[28:16]Destination Y Coordinate (s12)
//
#define ITE_VG_CMDSHIFT_DSTY   16
#define ITE_VG_CMDMASK_DSTY    0x1FFF0000

//
// D[12:0]Destination Y Coordinate (s12)
//
#define ITE_VG_CMDSHIFT_DSTX   0
#define ITE_VG_CMDMASK_DSTX    0x00001FFF
//======================================
//Destination Coordinate Register End
//======================================


//======================================
//Destination Height/Width Register
//======================================
#define ITE_VG_REG_DHWR_BASE    0x178
//
// D[29:16]Destination Height (Max: 8192)
//
#define ITE_VG_CMDSHIFT_DSTHEIGHT   16
#define ITE_VG_CMDMASK_DSTHEIGHT    0x3FFF0000

//
// D[13:0]Destination Width (Max: 8192)
//
#define ITE_VG_CMDSHIFT_DSTWIDTH   0
#define ITE_VG_CMDMASK_DSTWIDTH    0x00003FFF
//======================================
//Destination Height/Width Register End
//======================================


//======================================
//Destination Base Register
//======================================
#define ITE_VG_REG_DBR_BASE     0x17C
//
// D[31:0]Destination Base Address
//
#define ITE_VG_CMDSHIFT_DSTBASE   0
#define ITE_VG_CMDMASK_DSTBASE   0xFFFFFFFF
//======================================
//Destination Base Register End
//======================================


//======================================
//Src/Dst Pitch Register
//======================================
#define ITE_VG_REG_SDPR_BASE    0x180
//
// D[30:16]Source Pitch Width in Byte
//
#define ITE_VG_CMDSHIFT_SRCPITCH0   16
#define ITE_VG_CMDMASK_SRCPITCH0    0x7FFF0000

//
// D[14:0]Destination Pitch Width in Byte
//
#define ITE_VG_CMDSHIFT_DSTPITCH   0
#define ITE_VG_CMDMASK_DSTPITCH    0x00007FFF
//======================================
//Src/Dst Pitch Register End
//======================================


//======================================
//Source Coordinate Register
//======================================
#define ITE_VG_REG_SCR_BASE     0x184
//
// D[28:16]Source Y Coordinate (s12)
//
#define ITE_VG_CMDSHIFT_SRCY   16
#define ITE_VG_CMDMASK_SRCY    0x1FFF0000

//
// D[12:0]Source Y Coordinate (s12)
//
#define ITE_VG_CMDSHIFT_SRCX   0
#define ITE_VG_CMDMASK_SRCX    0x00001FFF
//======================================
//Source Coordinate Register End
//======================================


//======================================
//Source Height/Width Register
//======================================
#define ITE_VG_REG_SHWR_BASE    0x188
//
// D[29:16]Source Height (Max: 8192)
//
#define ITE_VG_CMDSHIFT_SRCHEIGHT   16
#define ITE_VG_CMDMASK_SRCHEIGHT    0x3FFF0000

//
// D[13:0]Source Width (Max: 8192)
//
#define ITE_VG_CMDSHIFT_SRCWIDTH   0
#define ITE_VG_CMDMASK_SRCWIDTH    0x00003FFF
//======================================
//Source Height/Width Register End
//======================================


//======================================
//Source Base Register
//======================================
#define ITE_VG_REG_SBR_BASE     0x18C
//
// D[31:0]Source Base Address
//
#define ITE_VG_CMDSHIFT_SRCBASE   0
#define ITE_VG_CMDMASK_SRCBASE   0xFFFFFFFF
//======================================
//Source Base Register End
//======================================


//======================================
//Source 1/2 Pitch Register
//======================================
#define ITE_VG_REG_SPR12_BASE   0x190
//
// D[30:16]Source 2 Pitch for better quality. (4x Source Image)
//
#define ITE_VG_CMDSHIFT_SRCPITCH2   16
#define ITE_VG_CMDMASK_SRCPITCH2    0x7FFF0000

//
// D[14:0]Source 1 Pitch for better quality. (1/4x Source Image) 
// Note: As read destination pitch at lookup table.
//
#define ITE_VG_CMDSHIFT_SRCPITCH1   0
#define ITE_VG_CMDMASK_SRCPITCH1    0x00007FFF

//
// D[24] Enable Src1 extend bits.
//
#define ITE_VG_SRC1EXTEND_EN		0x01000000

//
// D[23:16] Src1 data format. Please see OpenVG 1.1 Spec. Page 136
//
#define ITE_VG_SRC1FORMAT           0x00FF0000
#define ITE_VG_SRC1FORMAT_SHIFT     16

//======================================
//Source 1/2 Pitch Register End
//======================================


//======================================
//Source 1 Base Register
//======================================
#define ITE_VG_REG_SBR1_BASE    0x194
//
// D[31:0]Source 1 Base Address for better quality. 
// (1/4x Source Image) 
// Note: As read destination base address at lookup table.
//
#define ITE_VG_CMDSHIFT_SRCBASE1   0
#define ITE_VG_CMDMASK_SRCBASE1    0xFFFFFFFF
//======================================
//Source 1 Base Register End
//======================================


//======================================
//Source 2 Base Register
//======================================
#define ITE_VG_REG_SBR2_BASE    0x198
//
// D[31:0]Source 2 Base Address for better quality. 
// (1/4x Source Image) 
// Note: As read destination base address at lookup table.
//
#define ITE_VG_CMDSHIFT_SRCBASE2   0
#define ITE_VG_CMDMASK_SRCBASE2    0xFFFFFFFF

//
// D[28:16] Source1 Y Coordinate (s12) at RENDERMODE=2 b10
//
#define ITE_VG_CMDSHIFT_SRCY1		16
//
// D[12:0] Source1 X Coordinate (s12) at RENDERMODE=2 b10
//
#define ITE_VG_CMDSHIFT_SRCX1		0

//======================================
//Source 2 Base Register End
//======================================


//======================================
//Mask Base Register
//======================================
#define ITE_VG_REG_MBR_BASE     0x19C
//
// D[31:0]Mask Base Address
//
#define ITE_VG_CMDSHIFT_MASKBASE   0
#define ITE_VG_CMDMASK_MASKBASE    0xFFFFFFFF
//======================================
//Mask Base Register End
//======================================


//======================================
//Scissor/Mask Pitch Register
//======================================
#define ITE_VG_REG_SMPR_BASE    0x1A0
//
// D[25:16]Scissoring Pitch (64 bits alignment)
//
#define ITE_VG_CMDSHIFT_SCISPITCH   16
#define ITE_VG_CMDMASK_SCISPITCH    0x03FF0000

//
// D[12:0]Masking Pitch
//
#define ITE_VG_CMDSHIFT_MASKPITCH   0
#define ITE_VG_CMDMASK_MASKPITCH    0x00000FFF
//======================================
//Scissor/Mask Pitch Register End
//======================================


//======================================
//Scissor Base Register
//======================================
#define ITE_VG_REG_SCBR_BASE    0x1A4
//
// D[31:0]Scissoring Base Address (64 bits alignment)
//
#define ITE_VG_CMDSHIFT_SCISBASE   0
#define ITE_VG_CMDMASK_SCISBASE    0xFFFFFFFF
//======================================
//Scissor Base Register End
//======================================


//======================================
//User Inverse Transform Register 00
//======================================
#define ITE_VG_REG_UITR00_BASE  0x1A8
//
// D[28:0]User Inverse Transform Parameter (0,0) (s12.16)
//
#define ITE_VG_CMDSHIFT_USRINV00   0
#define ITE_VG_CMDMASK_USRINV00    0x1FFFFFFF
//======================================
//User Inverse Transform Register 00 End
//======================================


//======================================
//User Inverse Transform Register 01
//======================================
#define ITE_VG_REG_UITR01_BASE  0x1AC
//
// D[28:0]User Inverse Transform Parameter (0,1) (s12.16)
//
#define ITE_VG_CMDSHIFT_USRINV01   0
#define ITE_VG_CMDMASK_USRINV01    0x1FFFFFFF
//======================================
//User Inverse Transform Register 01 End
//======================================


//======================================
//User Inverse Transform Register 02
//======================================
#define ITE_VG_REG_UITR02_BASE  0x1B0
//
// D[28:0]User Inverse Transform Parameter (0,2) (s12.16)
//
#define ITE_VG_CMDSHIFT_USRINV02   0
#define ITE_VG_CMDMASK_USRINV02    0x1FFFFFFF
//======================================
//User Inverse Transform Register 02 End
//======================================


//======================================
//User Inverse Transform Register 10
//======================================
#define ITE_VG_REG_UITR10_BASE  0x1B4
//
// D[28:0]User Inverse Transform Parameter (1,0) (s12.16)
//
#define ITE_VG_CMDSHIFT_USRINV10   0
#define ITE_VG_CMDMASK_USRINV10    0x1FFFFFFF
//======================================
//User Inverse Transform Register 10 End
//======================================


//======================================
//User Inverse Transform Register 11
//======================================
#define ITE_VG_REG_UITR11_BASE  0x1B8
//
// D[28:0]User Inverse Transform Parameter (1,1) (s12.16)
//
#define ITE_VG_CMDSHIFT_USRINV11   0
#define ITE_VG_CMDMASK_USRINV11    0x1FFFFFFF
//======================================
//User Inverse Transform Register 11 End
//======================================


//======================================
//User Inverse Transform Register 12
//======================================
#define ITE_VG_REG_UITR12_BASE  0x1BC
//
// D[28:0]User Inverse Transform Parameter (1,2) (s12.16)
//
#define ITE_VG_CMDSHIFT_USRINV12   0
#define ITE_VG_CMDMASK_USRINV12    0x1FFFFFFF
//======================================
//User Inverse Transform Register 12 End
//======================================


//======================================
//User Inverse Transform Register 20
//======================================
#define ITE_VG_REG_UITR20_BASE  0x1C0
//
// D[28:0]User Inverse Transform Parameter (2,0) (s12.16)
//
#define ITE_VG_CMDSHIFT_USRINV20   0
#define ITE_VG_CMDMASK_USRINV20    0x1FFFFFFF
//======================================
//User Inverse Transform Register 20 End
//======================================


//======================================
//User Inverse Transform Register 21
//======================================
#define ITE_VG_REG_UITR21_BASE  0x1C4
//
// D[28:0]User Inverse Transform Parameter (2,1) (s12.16)
//
#define ITE_VG_CMDSHIFT_USRINV21   0
#define ITE_VG_CMDMASK_USRINV21    0x1FFFFFFF
//======================================
//User Inverse Transform Register 21 End
//======================================


//======================================
//User Inverse Transform Register 22
//======================================
#define ITE_VG_REG_UITR22_BASE  0x1C8
//
// D[28:0]User Inverse Transform Parameter (2,2) (s12.16)
//
#define ITE_VG_CMDSHIFT_USRINV22   0
#define ITE_VG_CMDMASK_USRINV22    0x1FFFFFFF
//======================================
//User Inverse Transform Register 22 End
//======================================


//======================================
//Paint Inverse Transform Register 00
//======================================
#define ITE_VG_REG_PITR00_BASE  0x1CC
//
// D[28:0]Paint Inverse Transform Parameter (0,0) (s12.16)
//
#define ITE_VG_CMDSHIFT_PATINV00   0
#define ITE_VG_CMDMASK_PATINV00    0x1FFFFFFF
//======================================
//Paint Inverse Transform Register 00 End
//======================================


//======================================
//Paint Inverse Transform Register 01
//======================================
#define ITE_VG_REG_PITR01_BASE  0x1D0
//
// D[28:0]Paint Inverse Transform Parameter (0,1) (s12.16)
//
#define ITE_VG_CMDSHIFT_PATINV01   0
#define ITE_VG_CMDMASK_PATINV01    0x1FFFFFFF
//======================================
//Paint Inverse Transform Register 01 End
//======================================


//======================================
//Paint Inverse Transform Register 02
//======================================
#define ITE_VG_REG_PITR02_BASE  0x1D4
//
// D[28:0]Paint Inverse Transform Parameter (0,2) (s12.16)
//
#define ITE_VG_CMDSHIFT_PATINV02   0
#define ITE_VG_CMDMASK_PATINV02    0x1FFFFFFF
//======================================
//Paint Inverse Transform Register 02 End
//======================================


//======================================
//Paint Inverse Transform Register 10
//======================================
#define ITE_VG_REG_PITR10_BASE  0x1D8
//
// D[28:0]Paint Inverse Transform Parameter (1,0) (s12.16)
//
#define ITE_VG_CMDSHIFT_PATINV10   0
#define ITE_VG_CMDMASK_PATINV10    0x1FFFFFFF
//======================================
//Paint Inverse Transform Register 10 End
//======================================


//======================================
//Paint Inverse Transform Register 11
//======================================
#define ITE_VG_REG_PITR11_BASE  0x1DC
//
// D[28:0]Paint Inverse Transform Parameter (1,1) (s12.16)
//
#define ITE_VG_CMDSHIFT_PATINV11   0
#define ITE_VG_CMDMASK_PATINV11    0x1FFFFFFF
//======================================
//Paint Inverse Transform Register 11 End
//======================================


//======================================
//Paint Inverse Transform Register 12
//======================================
#define ITE_VG_REG_PITR12_BASE  0x1E0
//
// D[28:0]Paint Inverse Transform Parameter (1,2) (s12.16)
//
#define ITE_VG_CMDSHIFT_PATINV12   0
#define ITE_VG_CMDMASK_PATINV12    0x1FFFFFFF
//======================================
//Paint Inverse Transform Register 12 End
//======================================

//======================================
//Color Transform Scale Register 0
//======================================
#define ITE_VG_REG_CTSR0_BASE    0x1E4

//
// D[31:16]Color transform parameter Sg (s7.8)
//
#define ITE_VG_CMDSHIFT_SCOLXFM10   16
#define ITE_VG_CMDMASK_SCOLXFM10    0xFFFF0000

//
// D[15:0]Color transform parameter Sr (s7.8)
//
#define ITE_VG_CMDSHIFT_SCOLXFM00   0
#define ITE_VG_CMDMASK_SCOLXFM00    0x0000FFFF
//======================================
//Color Transform Scale Register 0 End
//======================================

//======================================
//Color Transform Scale Register 1
//======================================
#define ITE_VG_REG_CTSR1_BASE    0x1E8

//
// D[31:16]Color transform parameter Sa (s7.8)
//
#define ITE_VG_CMDSHIFT_SCOLXFM30   16
#define ITE_VG_CMDMASK_SCOLXFM30    0xFFFF0000

//
// D[15:0]Color transform parameter Sb (s7.8)
//
#define ITE_VG_CMDSHIFT_SCOLXFM20   0
#define ITE_VG_CMDMASK_SCOLXFM20    0x0000FFFF
//======================================
//Color Transform Scale Register 1 End
//======================================

//======================================
//Texture Quality Scalar Register
//======================================
#define ITE_VG_REG_TQSR_BASE    0x1EC

//
// D[23:0]Texture quality scalar ratio. (12.12)
//
#define ITE_VG_CMDSHIFT_TexScalar   0
#define ITE_VG_CMDMASK_TexScalar   0x00FFFFFF

//
// D[28:16] Source1 Height (Max: 8192) at RENDERMODE=2 b10
//
#define ITE_VG_CMDMASK_SRC1HEIGHT	0x1FFF0000
#define ITE_VG_CMDSHIFT_SRC1HEIGHT	16

//
// D[12:0] Source1 Height (Max: 8192) at RENDERMODE=2 b10
//
#define ITE_VG_CMDMASK_SRC1WIDTH	0x00001FFF

//======================================
//Texture Quality Scalar Register End
//======================================

//======================================
//Constant Alpha Register
//======================================
#define ITE_VG_REG_CAR_BASE     0x1F0
//
// D[31:24]Constant Alpha of Red (8)
//
#define ITE_VG_CMDSHIFT_CONSTALPHA_R   24
#define ITE_VG_CMDMASK_CONSTALPHA_R    0xFF000000

//
// D[23:16]Constant Alpha of Green (8)
//
#define ITE_VG_CMDSHIFT_CONSTALPHA_G   16
#define ITE_VG_CMDMASK_CONSTALPHA_G    0x00FF0000

//
// D[15:8]Constant Alpha of Blue (8)
//
#define ITE_VG_CMDSHIFT_CONSTALPHA_B   8
#define ITE_VG_CMDMASK_CONSTALPHA_B    0x0000FF00

//
// D[7:0]Constant Alpha of Alpha (8)
//
#define ITE_VG_CMDSHIFT_CONSTALPHA_A   0
#define ITE_VG_CMDMASK_CONSTALPHA_A    0x000000FF
//======================================
//Constant Alpha Register End
//======================================

//======================================
//Destination Paint Color Register 0
//======================================
#define ITE_VG_REG_DPCR0_BASE     0x1F4
//
// D[23:16]Destination paint color Blue, tBitBlt High color key
//
#define ITE_VG_CMDSHIFT_DSTCOLOR_B   16
#define ITE_VG_CMDMASK_DSTCOLOR_B    0x00FF0000

//
// D[7:0]Destination paint color Alpha, tBitBlt High color key
//
#define ITE_VG_CMDSHIFT_DSTCOLOR_A   0
#define ITE_VG_CMDMASK_DSTCOLOR_A    0x000000FF

//======================================
//Destination Paint Color Register 0 End
//======================================


//======================================
//Destination Paint Color Register 1
//======================================
#define ITE_VG_REG_DPCR1_BASE     0x1F8
//
// D[23:16]Destination paint color Blue, tBitBlt High color key
//
#define ITE_VG_CMDSHIFT_DSTCOLOR_R   16
#define ITE_VG_CMDMASK_DSTCOLOR_R    0x00FF0000

//
// D[11:0]Destination paint color Alpha, tBitBlt High color key
//
#define ITE_VG_CMDSHIFT_DSTCOLOR_G   0
#define ITE_VG_CMDMASK_DSTCOLOR_G    0x00000FFF

//======================================
//Destination Paint Color Register 1 End
//======================================

//======================================
//Pattern Paint Color Register 0
//======================================
#define ITE_VG_REG_PPCR0_BASE     0x1FC

//
// D[23:16] ROP3 pattern paint color Blue, tBitBlt High color key
//
#define ITE_VG_CMDSHIFT_PATCOLOR_B		16
#define ITE_VG_CMDMASK_PATCOLOR_B		0x00FF0000

//
// D[7:0] ROP3 pattern paint color Blue, tBitBlt High color key
//
#define ITE_VG_CMDSHIFT_PATCOLOR_A		0
#define ITE_VG_CMDMASK_PATCOLOR_A		0x000000FF

//======================================
//Pattern Paint Color Register 0 end
//======================================

//======================================
//Pattern Paint Color Register 1
//======================================
#define ITE_VG_REG_PPCR1_BASE     0x200
//
// D[23:16]ROP3 pattern paint color Red, tBitBlt High color key
//
#define ITE_VG_CMDSHIFT_DSTCOLOR_R   16
#define ITE_VG_CMDMASK_DSTCOLOR_R    0x00FF0000

//
// D[11:0]ROP3 pattern paint color Red, tBitBlt High color key
//
#define ITE_VG_CMDSHIFT_DSTCOLOR_G   0
#define ITE_VG_CMDMASK_DSTCOLOR_G    0x00000FFF

//======================================
//Pattern Paint Color Register 1 End
//======================================


//======================================
//BIST State Register
//======================================
#define ITE_VG_REG_BISTR_BASE   0x20C
//
// D[11:8]R
//
#define ITE_VG_CMDSHIFT_BIST_DONE   8
#define ITE_VG_CMDMASK_BIST_DONE    0x00000F00

//
// D[3:0]R
//
#define ITE_VG_CMDSHIFT_BIST_FAULT   0
#define ITE_VG_CMDMASK_BIST_FAULT    0x0000000F
//======================================
//BIST State Register End
//======================================


//======================================
//Command Register
//======================================
#define ITE_VG_REG_CMDR_BASE    0x210
//
// D[0]ALL Engine busy enable. 
// 0: HVGBusy = Only Front Engine busy. 
// 1: HVGBusy = All Engine busy.
//
#define ITE_VG_CMD_HVGBUSY   0x00000001
//======================================
//Command Register
//======================================

#define ITE_VG_REG_ESR1_BASE    0x214
#define ITE_VG_REG_ESR2_BASE    0x218
#define ITE_VG_REG_ESR3_BASE    0x21C

//======================================
// Interrupt Control Register
//======================================
#define ITE_VG_REG_ICR_BASE		0x224

#define ITE_VG_CMDSHIFT_BID6INT	13
#define ITE_VG_CMDMASK_BID6INT	0x00002000

#define ITE_VG_CMDSHIFT_BID5INT	12
#define ITE_VG_CMDMASK_BID5INT	0x00001000

#define ITE_VG_CMDSHIFT_BID4INT	11
#define ITE_VG_CMDMASK_BID4INT	0x00000800

#define ITE_VG_CMDSHIFT_BID3INT	10
#define ITE_VG_CMDMASK_BID3INT	0x00000400

#define ITE_VG_CMDSHIFT_BID2INT	9
#define ITE_VG_CMDMASK_BID2INT	0x00000200

#define ITE_VG_CMDSHIFT_BID1INT	8
#define ITE_VG_CMDMASK_BID1INT	0x00000100

#define ITE_VG_CMDSHIFT_BID6INT_EN	5
#define ITE_VG_CMDMASK_BID6INT_EN	0x00000020

#define ITE_VG_CMDSHIFT_BID5INT_EN	4
#define ITE_VG_CMDMASK_BID5INT_EN	0x00000010

#define ITE_VG_CMDSHIFT_BID4INT_EN	3
#define ITE_VG_CMDMASK_BID4INT_EN	0x00000008

#define ITE_VG_CMDSHIFT_BID3INT_EN	2
#define ITE_VG_CMDMASK_BID3INT_EN	0x00000004

#define ITE_VG_CMDSHIFT_BID2INT_EN	1
#define ITE_VG_CMDMASK_BID2INT_EN	0x00000002

#define ITE_VG_CMDSHIFT_BID1INT_EN	0
#define ITE_VG_CMDMASK_BID1INT_EN	0x00000001

//======================================
// BitBlt ID 1 Register
//======================================
#define ITE_VG_REG_BID1_BASE		0x228

//======================================
// BitBlt ID 2 Register
//======================================
#define ITE_VG_REG_BID2_BASE		0x22C

//======================================
// BitBlt ID 3 Register
//======================================
#define ITE_VG_REG_BID3_BASE		0x230

//======================================
// BitBlt ID 4 Register
//======================================
#define ITE_VG_REG_BID4_BASE		0x234

//======================================
// BitBlt ID 5 Register
//======================================
#define ITE_VG_REG_BID5_BASE		0x238

//======================================
// BitBlt ID 6 Register
//======================================
#define ITE_VG_REG_BID6_BASE		0x23C

//======================================
//Revision Register
//======================================
#define ITE_VG_REG_REV_BASE     0x240
//
// D[23:16]Major revision number
//
#define ITE_VG_CMDSHIFT_MAJOR_REV   16
#define ITE_VG_CMDMASK_MAJOR_REV    0x00FF0000

//
// D[15:8]Minor revision number
//
#define ITE_VG_CMDSHIFT_MINOR_REV   8
#define ITE_VG_CMDMASK_MINOR_REV    0x0000FF00

//
// D[7:0]Release number
//
#define ITE_VG_CMDSHIFT_REL_REV   0
#define ITE_VG_CMDMASK_REL_REV    0x000000FF
//======================================
//Revision Register End
//======================================

/* ======================================== */ 
/*              ITE Register End            */
/* ======================================== */


#define POINTPREC		11

#define CACHE			0
#define COVERAGEDRAW	0
#define RENDERDRAW		0
#define ADDRESSBIT		12
#define CACHESIZE		4096	// 2^8 = 256
#define CACHESET		1
#define DUMPDATA		1
#define DUMPWIDTH		73
#define DUMPHEIGHT		65
#define DUMPFORMAT		1
#define DEBUGLENGTH		1

typedef enum {
  HW_FALSE = 0,
  HW_TRUE  = 1
} HWboolean;

typedef enum {
  HW_EVEN_ODD                                 = 0,
  HW_NON_ZERO                                 = 1
} HWFillRule;

typedef enum {
  HW_IMAGE_QUALITY_NONANTIALIASED             = 0,
  HW_IMAGE_QUALITY_FASTER                     = 1,
  HW_IMAGE_QUALITY_BETTER                     = 2
} HWImageQuality;

typedef enum {
  HW_RENDERING_QUALITY_NONANTIALIASED         = 0,
  HW_RENDERING_QUALITY_FASTER                 = 1,
  HW_RENDERING_QUALITY_BETTER                 = 2  /* Default */
} HWRenderingQuality;

typedef enum {
  HW_BLEND_SRC                                = 0x0,
  HW_BLEND_SRC_OVER                           = 0x1,
  HW_BLEND_DST_OVER                           = 0x2,
  HW_BLEND_SRC_IN                             = 0x3,
  HW_BLEND_DST_IN                             = 0x4,
  HW_BLEND_MULTIPLY                           = 0x5,
  HW_BLEND_SCREEN                             = 0x6,
  HW_BLEND_DARKEN                             = 0x7,
  HW_BLEND_LIGHTEN                            = 0x8,
  HW_BLEND_ADDITIVE                           = 0x9,
  HW_UNION_MASK								  = 0x10,
  HW_INTERSECT_MASK							  = 0x11,
  HW_SUBTRACT_MASK							  = 0x12
} HWBlendMode;

typedef enum {
  HW_UNION_RENDERMASK						  = 0x0,
  HW_INTERSECT_RENDERMASK					  = 0x1,
  HW_SUBTRACT_RENDERMASK					  = 0x2
} HWMaskMode;

typedef enum {
  HW_DRAW_IMAGE_NORMAL                        = 0x0,
  HW_DRAW_IMAGE_MULTIPLY                      = 0x1,
  HW_DRAW_IMAGE_STENCIL                       = 0x2
} HWImageMode;

typedef enum {
  HW_FILL_PATH                                = 0,
  HW_STROKE_PATH                              = 1
} HWPaintMode;

typedef enum {
  HW_CAP_BUTT                                 = 0x0,
  HW_CAP_ROUND                                = 0x1,
  HW_CAP_SQUARE                               = 0x2
} HWCapStyle;

typedef enum {
  HW_JOIN_MITER                               = 0x0,
  HW_JOIN_ROUND                               = 0x1,
  HW_JOIN_BEVEL                               = 0x2
} HWJoinStyle;

typedef enum {
  HW_COLOR_RAMP_SPREAD_PAD                    = 0,
  HW_COLOR_RAMP_SPREAD_REPEAT                 = 1,
  HW_COLOR_RAMP_SPREAD_REFLECT                = 2
} HWColorRampSpreadMode;

typedef enum {
  HW_PAINT_TYPE_COLOR                         = 0,
  HW_PAINT_TYPE_LINEAR_GRADIENT               = 1,
  HW_PAINT_TYPE_RADIAL_GRADIENT               = 2,
  HW_PAINT_TYPE_PATTERN                       = 3
} HWPaintType;

typedef enum {
  HW_TILE_FILL                                = 0,
  HW_TILE_PAD                                 = 1,
  HW_TILE_REPEAT                              = 2,
  HW_TILE_REFLECT                             = 3
} HWTilingMode;

typedef enum {
  /* RGB{A,X} channel ordering */
  HW_sRGBX_8888                               =  0,
  HW_sRGBA_8888                               =  1,
  HW_sRGBA_8888_PRE                           =  2,
  HW_sRGB_565                                 =  3,
  HW_sRGBA_5551                               =  4,
  HW_sRGBA_4444                               =  5,
  HW_sL_8                                     =  6,
  HW_lRGBX_8888                               =  7,
  HW_lRGBA_8888                               =  8,
  HW_lRGBA_8888_PRE                           =  9,
  HW_lL_8                                     = 10,
  HW_A_8                                      = 11,
  HW_BW_1                                     = 12,
  HW_A_1                                      = 13,
  HW_A_4                                      = 14,
  HW_RGBA_16								  = 15,

  /* {A,X}RGB channel ordering */
  HW_sXRGB_8888                               =  0 | (1 << 6),
  HW_sARGB_8888                               =  1 | (1 << 6),
  HW_sARGB_8888_PRE                           =  2 | (1 << 6),
  HW_sARGB_1555                               =  4 | (1 << 6),
  HW_sARGB_4444                               =  5 | (1 << 6),
  HW_lXRGB_8888                               =  7 | (1 << 6),
  HW_lARGB_8888                               =  8 | (1 << 6),
  HW_lARGB_8888_PRE                           =  9 | (1 << 6),

  /* BGR{A,X} channel ordering */
  HW_sBGRX_8888                               =  0 | (1 << 7),
  HW_sBGRA_8888                               =  1 | (1 << 7),
  HW_sBGRA_8888_PRE                           =  2 | (1 << 7),
  HW_sBGR_565                                 =  3 | (1 << 7),
  HW_sBGRA_5551                               =  4 | (1 << 7),
  HW_sBGRA_4444                               =  5 | (1 << 7),
  HW_lBGRX_8888                               =  7 | (1 << 7),
  HW_lBGRA_8888                               =  8 | (1 << 7),
  HW_lBGRA_8888_PRE                           =  9 | (1 << 7),

  /* {A,X}BGR channel ordering */
  HW_sXBGR_8888                               =  0 | (1 << 6) | (1 << 7),
  HW_sABGR_8888                               =  1 | (1 << 6) | (1 << 7),
  HW_sABGR_8888_PRE                           =  2 | (1 << 6) | (1 << 7),
  HW_sABGR_1555                               =  4 | (1 << 6) | (1 << 7),
  HW_sABGR_4444                               =  5 | (1 << 6) | (1 << 7),
  HW_lXBGR_8888                               =  7 | (1 << 6) | (1 << 7),
  HW_lABGR_8888                               =  8 | (1 << 6) | (1 << 7),
  HW_lABGR_8888_PRE                           =  9 | (1 << 6) | (1 << 7),

} HWImageFormat;

typedef struct
{
  ITEs12p3 x,y;			//s12.3
} HWVector2;

typedef struct
{
  ITEs15p16 x,y;		//s15.16
} HWTXVector2;

typedef struct
{
  ITEs15p16 x;			//s15.16
  ITEs12p3  y;			//s12.3
} HWPXVector2;

typedef struct
{
  ITEs12p3 x,y,z;		//s12.3
} HWVector3;

typedef struct
{
  ITEs15p16 x,y,z;		//s15.16
} HWTXVector3;

typedef struct
{
  ITEs15p16 m[3][3];	//s15.16
} HWMatrix3x3;

// (v, mat, vout) = (s12.3, s15.16, s12.16)
#define HW0TRANSFORM3TO(v, mat, vout) { \
vout.x = (ITEint32)(((ITEint)v.x*mat.m[0][0] + (ITEint)v.y*mat.m[0][1] + (ITEint)v.z*mat.m[0][2] + (1<<2) )>>3); \
vout.y = (ITEint32)(((ITEint)v.x*mat.m[1][0] + (ITEint)v.y*mat.m[1][1] + (ITEint)v.z*mat.m[1][2] + (1<<2) )>>3); \
vout.z = (ITEint32)(((ITEint)v.x*mat.m[2][0] + (ITEint)v.y*mat.m[2][1] + (ITEint)v.z*mat.m[2][2] + (1<<2) )>>3); }

// (v, mat, vout) = (s12.3, s15.16, s12.3)
#define HW1TRANSFORM3TO(v, mat, vout) { \
vout.x = (ITEint16)(((ITEint)v.x*mat.m[0][0] + (ITEint)v.y*mat.m[0][1] + (ITEint)v.z*mat.m[0][2] + (1<<15) )>>16); \
vout.y = (ITEint16)(((ITEint)v.x*mat.m[1][0] + (ITEint)v.y*mat.m[1][1] + (ITEint)v.z*mat.m[1][2] + (1<<15) )>>16); \
vout.z = (ITEint16)(((ITEint)v.x*mat.m[2][0] + (ITEint)v.y*mat.m[2][1] + (ITEint)v.z*mat.m[2][2] + (1<<15) )>>16); }

// (v, mat, vout) = (s12.3, s15.16, s15.16)
#define HW0TRANSFORM2TO(v, mat, vout) { \
vout.x = (ITEint32)(((ITEint)v.x*mat.m[0][0] + (ITEint)v.y*mat.m[0][1] + 8*mat.m[0][2] + (1<<2) )>>3); \
vout.y = (ITEint32)(((ITEint)v.x*mat.m[1][0] + (ITEint)v.y*mat.m[1][1] + 8*mat.m[1][2] + (1<<2) )>>3); }

// (v, mat, vout) = (s15.16, s15.16, s12.3)
#define HW1TRANSFORM2TO(v, mat, vout) { \
vout.x = (ITEint16)(((ITEint64)v.x*mat.m[0][0] + (ITEint64)v.y*mat.m[0][1] + (ITEint64)0x10000*mat.m[0][2] + (1<<28) )>>29); \
vout.y = (ITEint16)(((ITEint64)v.x*mat.m[1][0] + (ITEint64)v.y*mat.m[1][1] + (ITEint64)0x10000*mat.m[1][2] + (1<<28) )>>29); }

// (v, mat, vout) = (s12.11, s15.16, s12.3)
#define HWAFFINETRANSFORM2TO(v, mat, vout) { \
vout.x = (ITEint32)(((ITEint64)v.x*mat.m[0][0] + (ITEint64)v.y*mat.m[0][1] + (ITEint64)(1<<POINTPREC)*mat.m[0][2] + (1<<((POINTPREC+16-3)-1)) )>>(POINTPREC+16-3)); \
vout.y = (ITEint32)(((ITEint64)v.x*mat.m[1][0] + (ITEint64)v.y*mat.m[1][1] + (ITEint64)(1<<POINTPREC)*mat.m[1][2] + (1<<((POINTPREC+16-3)-1)) )>>(POINTPREC+16-3)); }

// (v, mat, vout) = (s12.11, s15.16, s12.3 = s12.19 / s12.16)
#define HWPERSPECTIVETRANSFORM3TO(v, mat, vout) { \
vout.x = (ITEint32)( (((ITEint64)v.x*mat.m[0][0] + (ITEint64)v.y*mat.m[0][1] + (ITEint64)(1<<POINTPREC)*mat.m[0][2] + (1<<(POINTPREC+16-19-1)) )>>(POINTPREC+16-19)) / \
	                 (((ITEint64)v.x*mat.m[2][0] + (ITEint64)v.y*mat.m[2][1] + (ITEint64)(1<<POINTPREC)*mat.m[2][2] + (1<<(POINTPREC+16-19-1)) )>>(POINTPREC+16-16)) ); \
vout.y = (ITEint32)( (((ITEint64)v.x*mat.m[1][0] + (ITEint64)v.y*mat.m[1][1] + (ITEint64)(1<<POINTPREC)*mat.m[1][2] + (1<<(POINTPREC+16-19-1)) )>>(POINTPREC+16-19)) / \
					 (((ITEint64)v.x*mat.m[2][0] + (ITEint64)v.y*mat.m[2][1] + (ITEint64)(1<<POINTPREC)*mat.m[2][2] + (1<<(POINTPREC+16-19-1)) )>>(POINTPREC+16-16)) ); }

// (v, mat, vout) = (s15.16, s15.16, s12.3)
#define HW1TRANSFORM2DIR(v, mat, vout) { \
vout.x = (ITEint16)(((ITEint64)v.x*mat.m[0][0] + (ITEint64)v.y*mat.m[0][1] + (1<<28) )>>29); \
vout.y = (ITEint16)(((ITEint64)v.x*mat.m[1][0] + (ITEint64)v.y*mat.m[1][1] + (1<<28) )>>29); }

#define HW_MOD(a,b) ( a%(b)>=0 ? a%(b) : (a%(b))+(b) ) 
#define HW_FLOOR(x) ((x>>3)<<3)
#define HW_CEIL(x) (((x+7)>>3)<<3)
#define HW_SQRT(x) ( (ITEuint32)(sqrt((float)(x)/0x10000)*0x10000) )

#define TESSELLATION_CMD_LENGTH (65536)
#define INVALID_OBJECT_ID			0xFFFFFFFF
#define INVALID_FRAME_ID			0xFFFFFFFF

typedef struct _ITEHardwareRegister
{	
	ITEuint32 REG_TCR_BASE;			// 0x000
	ITEuint32 REG_LWR_BASE;			// 0x004
	ITEuint32 REG_SRNR_BASE;		// 0x008
	ITEuint32 REG_FNR_BASE;			// 0x00C
	ITEuint32 REG_PBR_BASE;			// 0x010
	ITEuint32 REG_PLR_BASE;			// 0x014
	ITEuint32 REG_TLR_BASE;			// 0x018
	ITEuint32 REG_TBR_BASE;			// 0x01C
	ITEuint32 REG_UTR00_BASE;		// 0x020
	ITEuint32 REG_UTR01_BASE;		// 0x024
	ITEuint32 REG_UTR02_BASE;		// 0x028
	ITEuint32 REG_UTR10_BASE;		// 0x02C
	ITEuint32 REG_UTR11_BASE;		// 0x030
	ITEuint32 REG_UTR12_BASE;		// 0x034
	ITEuint32 REG_UTR20_BASE;		// 0x038
	ITEuint32 REG_UTR21_BASE;		// 0x03C
	ITEuint32 REG_UTR22_BASE;		// 0x040
	ITEuint32 REG_DPR00_BASE;		// 0x044
	ITEuint32 REG_DPR01_BASE;		// 0x048
	ITEuint32 REG_DPR02_BASE;		// 0x04C
	ITEuint32 REG_DPR03_BASE;		// 0x050
	ITEuint32 REG_DPR04_BASE;		// 0x054
	ITEuint32 REG_DPR05_BASE;		// 0x058
	ITEuint32 REG_DPR06_BASE;		// 0x05C
	ITEuint32 REG_DPR07_BASE;		// 0x060
	ITEuint32 REG_DPR08_BASE;		// 0x064
	ITEuint32 REG_DPR09_BASE;		// 0x068
	ITEuint32 REG_DPR10_BASE;		// 0x06C
	ITEuint32 REG_DPR11_BASE;		// 0x070
	ITEuint32 REG_DPR12_BASE;		// 0x074
	ITEuint32 REG_DPR13_BASE;		// 0x078
	ITEuint32 REG_DPR14_BASE;		// 0x07C
	ITEuint32 REG_DPR15_BASE;		// 0x080
	ITEuint32 REG_DPR16_BASE;		// 0x084
	ITEuint32 REG_88_BASE;		    // 0x088
	ITEuint32 REG_8C_BASE;		    // 0x08C	
	ITEuint32 REG_CCR_BASE;			// 0x090
	ITEuint32 REG_CPBR_BASE;		// 0x094
	ITEuint32 REG_CVPPR_BASE;		// 0x098
	ITEuint32 REG_VPBR_BASE;		// 0x09C
	ITEuint32 REG_PXCR_BASE;		// 0x0A0
	ITEuint32 REG_PYCR_BASE;		// 0x0A4
	ITEuint32 REG_SFR_BASE;		    // 0x0A8
	ITEuint32 REG_ACR_BASE;			// 0x0AC
	ITEuint32 REG_RCR_BASE;			// 0x0B0
	ITEuint32 REG_RMR_BASE;			// 0x0B4
	ITEuint32 REG_RFR_BASE;			// 0x0B8
	ITEuint32 REG_RSR01_BASE;		// 0x0BC
	ITEuint32 REG_RSR23_BASE;		// 0x0C0
	ITEuint32 REG_RSR45_BASE;		// 0x0C4
	ITEuint32 REG_RSR67_BASE;		// 0x0C8
	ITEuint32 REG_RCR00_BASE;		// 0x0CC
	ITEuint32 REG_RCR01_BASE;		// 0x0D0
	ITEuint32 REG_RCR10_BASE;		// 0x0D4
	ITEuint32 REG_RCR11_BASE;		// 0x0D8
	ITEuint32 REG_RCR20_BASE;		// 0x0DC
	ITEuint32 REG_RCR21_BASE;		// 0x0E0
	ITEuint32 REG_RCR30_BASE;		// 0x0E4
	ITEuint32 REG_RCR31_BASE;		// 0x0E8
	ITEuint32 REG_RCR40_BASE;		// 0x0EC
	ITEuint32 REG_RCR41_BASE;		// 0x0F0
	ITEuint32 REG_RCR50_BASE;		// 0x0F4
	ITEuint32 REG_RCR51_BASE;		// 0x0F8
	ITEuint32 REG_RCR60_BASE;		// 0x0FC
	ITEuint32 REG_RCR61_BASE;		// 0x100
	ITEuint32 REG_RCR70_BASE;		// 0x104
	ITEuint32 REG_RCR71_BASE;		// 0x108
	ITEuint32 REG_RDR01_BASE;		// 0x10C
	ITEuint32 REG_RCR12_BASE;		// 0x110
	ITEuint32 REG_RDR23_BASE;		// 0x114
	ITEuint32 REG_RCR34_BASE;		// 0x118
	ITEuint32 REG_RDR45_BASE;		// 0x11C
	ITEuint32 REG_RCR56_BASE;		// 0x120
	ITEuint32 REG_RDR67_BASE;		// 0x124
	ITEuint32 REG_GPRA_BASE;		// 0x128
	ITEuint32 REG_GPRB_BASE;		// 0x12C
	ITEuint32 REG_GPRC_BASE;		// 0x130
	ITEuint32 REG_GPRD0_BASE;		// 0x134
	ITEuint32 REG_GPRD1_BASE;		// 0x138
	ITEuint32 REG_GPRE0_BASE;		// 0x13C
	ITEuint32 REG_GPRE1_BASE;		// 0x140
	ITEuint32 REG_GPRF0_BASE;		// 0x144
	ITEuint32 REG_GPRF1_BASE;		// 0x148
	ITEuint32 REG_GPRG0_BASE;		// 0x14C
	ITEuint32 REG_GPRG1_BASE;		// 0x150
	ITEuint32 REG_GPRH0_BASE;		// 0x154
	ITEuint32 REG_GPRH1_BASE;		// 0x158
	ITEuint32 REG_GPRI0_BASE;		// 0x15C
	ITEuint32 REG_GPRI1_BASE;		// 0x160
	ITEuint32 REG_CTBR0_BASE;		// 0x164
	ITEuint32 REG_CTBR1_BASE;		// 0x168
	ITEuint32 REG_CTBR2_BASE;		// 0x16C
	ITEuint32 REG_CTBR3_BASE;		// 0x170
	ITEuint32 REG_DCR_BASE;			// 0x174
	ITEuint32 REG_DHWR_BASE;		// 0x178
	ITEuint32 REG_DBR_BASE;			// 0x17C
	ITEuint32 REG_SDPR_BASE;		// 0x180
	ITEuint32 REG_SCR_BASE;			// 0x184
	ITEuint32 REG_SHWR_BASE;		// 0x188
	ITEuint32 REG_SBR_BASE;			// 0x18C
	ITEuint32 REG_SPR12_BASE;		// 0x190
	ITEuint32 REG_SBR1_BASE;		// 0x194
	ITEuint32 REG_SBR2_BASE;		// 0x198
	ITEuint32 REG_MBR_BASE;			// 0x19C
	ITEuint32 REG_SMPR_BASE;		// 0x1A0
	ITEuint32 REG_SCBR_BASE;		// 0x1A4
	ITEuint32 REG_UITR00_BASE;		// 0x1A8
	ITEuint32 REG_UITR01_BASE;		// 0x1AC
	ITEuint32 REG_UITR02_BASE;		// 0x1B0
	ITEuint32 REG_UITR10_BASE;		// 0x1B4
	ITEuint32 REG_UITR11_BASE;		// 0x1B8
	ITEuint32 REG_UITR12_BASE;		// 0x1BC
	ITEuint32 REG_UITR20_BASE;		// 0x1C0
	ITEuint32 REG_UITR21_BASE;		// 0x1C4
	ITEuint32 REG_UITR22_BASE;		// 0x1C8
	ITEuint32 REG_PITR00_BASE;		// 0x1CC
	ITEuint32 REG_PITR01_BASE;		// 0x1D0
	ITEuint32 REG_PITR02_BASE;		// 0x1D4
	ITEuint32 REG_PITR10_BASE;		// 0x1D8
	ITEuint32 REG_PITR11_BASE;		// 0x1DC
	ITEuint32 REG_PITR12_BASE;		// 0x1E0
	ITEuint32 REG_CTSR0_BASE;		// 0x1E4
	ITEuint32 REG_CTSR1_BASE;		// 0x1E8
	ITEuint32 REG_TQSR_BASE;		// 0x1EC
	ITEuint32 REG_CAR_BASE;			// 0x1F0
	ITEuint32 REG_DPCR0_BASE;		// 0x1F4
	ITEuint32 REG_DPCR1_BASE;		// 0x1F8
	ITEuint32 REG_PPCR0_BASE;		// 0x1FC
	ITEuint32 REG_PPCR1_BASE;		// 0x200
	ITEuint32 REG_204_BASE;			// 0x204
	ITEuint32 REG_208_BASE;			// 0x208
	ITEuint32 REG_BISTR_BASE;		// 0x20C
	ITEuint32 REG_CMDR_BASE;		// 0x210
	ITEuint32 REG_ESR1_BASE;		// 0x214
	ITEuint32 REG_ESR2_BASE;		// 0x218
	ITEuint32 REG_ESR3_BASE;		// 0x21C
	ITEuint32 REG_ESR4_BASE;		// 0x220
	ITEuint32 REG_ICR_BASE;			// 0x224
	ITEuint32 REG_BID1_BASE;		// 0x228
	ITEuint32 REG_BID2_BASE;		// 0x22C
	ITEuint32 REG_BID3_BASE;		// 0x230
	ITEuint32 REG_BID4_BASE;		// 0x234
	ITEuint32 REG_BID5_BASE;		// 0x238
	ITEuint32 REG_BID6_BASE;		// 0x23C
	ITEuint32 REG_REV_BASE;			// 0x240

	//ITEuint32 tessellateCmd[TESSELLATION_CMD_LENGTH];
    //ITEuint32 cmdLength; // Used command number
}ITEHardwareRegister;

typedef struct
{
	ITEuint				tessellateCmd[1<<14];	// tessellate Command buffer
	ITEuint				cmdData[1<<16];			// command buffer

	/* Tessellation Engine */
	HWPaintMode			paintMode;
	ITEs15p16			lineWidth;
	HWVector2			min;
	HWVector2			max;
	HWMatrix3x3			pathTransform;		//s15.16
	
	HWCapStyle			strokeCapStyle;
	HWJoinStyle			strokeJoinStyle;
	ITEs12p3			strokeMiterLimit;

	HWboolean           enDashLine;
	HWboolean           dashPhaseReset;
	ITEint				dashMaxCount;
	ITEs15p16			dashPattern[17];	// s12.11
	ITEs15p16			dashRLength;
	ITEuint8			dashCount;

  /* Mode settings */
	HWFillRule          fillRule;
	HWImageQuality      imageQuality;
	HWRenderingQuality  renderingQuality;
	HWBlendMode			blendMode;
	HWMaskMode			maskMode;
	HWImageMode         imageMode;

  /* enabling */
	HWboolean			enCoverage;			// enable coverage
	HWboolean           enScissor;
	HWboolean			enMask;  
	HWboolean			enTexture; 
	HWboolean			enColorTransform;
	HWboolean			enLookup;
	HWboolean           enBlend;
	HWboolean           enSrcMultiply;
	HWboolean           enSrcUnMultiply;
	HWboolean           enDstMultiply;
	HWboolean			enPerspective;

	/* color */
	ITEColor			tileFillColor;		//Edge fill color for vgConvolve and pattern paint  
	ITEColor			clearColor;			// vgClear

	/* Matrices */
	HWMatrix3x3			fillTransform;		//s15.16
	HWMatrix3x3			strokeTransform;	//s15.16
	HWMatrix3x3			imageTransform;		//s15.16

	/* Paint */
	HWColorRampSpreadMode spreadMode;
	HWPaintType			paintType;
	ITEColor			paintColor;
	HWTilingMode		tilingMode;
	ITEuint8			*gradientData;
	ITEuint8            gradientLen;

	ITEs15p16			linearGradientA;		//s15.16
	ITEs15p16			linearGradientB;		//s15.16
	ITEs15p16			linearGradientC;		//s15.16
	ITEs15p16			radialGradientA;		//s15.16
	ITEs15p16			radialGradientB;		//s15.16
	ITEs15p16			radialGradientC;		//s15.16
	ITEs15p16			radialGradientD;		//s15.16
	ITEs15p16			radialGradientE;		//s15.16
	ITEs15p16			radialGradientF;		//s15.16
	ITEs15p16			radialGradientG;		//s15.16
	ITEs15p16			radialGradientH;		//s15.16
	ITEs15p16			radialGradientI;		//s15.16

	/* Image */
	ITEs12p3			coverageX;				// S12.3
	ITEs12p3			coverageY;				// S12.3
	ITEint16			coverageWidth;			// 12
	ITEint16			coverageHeight;			// 12
	ITEint16			*coverageData;			// S10.5
	ITEint16			coveragevalidpitch;		// 6   byte aligment
	ITEuint8			*coverageValid;			// 8 valid bits


	ITEuint8			*textureData;
	ITEuint16			texturePitch;	// 14
	ITEint16			textureWidth;
	ITEint16			textureHeight;
	ITEuint8			textureFormat;

	ITEuint8			*patternData;
	ITEuint16			patternPitch;
	ITEint16			patternWidth;
	ITEint16			patternHeight;
	ITEuint8			patternFormat;

	ITEuint8			*maskData;
	ITEs12p3			maskX;		
	ITEs12p3			maskY;		
	ITEuint16			maskPitch;
	ITEint16			maskWidth;
	ITEint16			maskHeight;
	ITEuint8			maskFormat;

	ITEuint8			*surfaceData;
	ITEuint16			surfacePitch;
	ITEint16			surfaceWidth;
	ITEint16			surfaceHeight;
	ITEuint8			surfaceFormat;

	ITEs12p3            dstX;
	ITEs12p3			dstY;
	ITEint16			dstWidth;
	ITEint16			dstHeight;

  /* four point of the stroke path to do cap and join */ 
  //	recm0 |-----------------------| recm1
  //		  |						  |
  //		  |---------------------->|
  //		  |		    UnitV		  |
  //	recm3 |-----------------------| recm2
	ITEuint16			strokeRoundLines;
	HWTXVector2			strokeRoundC;		// s12.11

	ITEint8				LastisMove;
	HWTXVector2			strokestartrecm0;	// s12.11
	HWTXVector2			strokestartrecm3;	// s12.11
	HWTXVector2			strokestartUnitV0;	// s1.22
	HWTXVector2			strokestartVec0;	// s12.11
	HWboolean			strokestartdash0;

	HWTXVector2			strokelastrecm1;	// s12.11
	HWTXVector2			strokelastrecm2;	// s12.11
	HWTXVector2			strokelastVec1;		// s12.11
	HWTXVector2			strokelastUnitV1;	// s1.22		
	HWboolean           strokelastdash1;

	HWTXVector2			strokerecm0;		// s12.11
	HWTXVector2			strokerecm1;		// s12.11
	HWTXVector2			strokerecm2;		// s12.11
	HWTXVector2			strokerecm3;		// s12.11
	HWTXVector2			strokeVec0;			// s12.11
	HWTXVector2			strokeVec1;			// s12.11
	HWTXVector2			strokeUnitV0;		// s1.22	
	HWTXVector2			strokeUnitV1;		// s1.22
	HWboolean			strokedash0;
	HWboolean			strokedash1;

  /* color transform */
	ITEs7p8            colorTransform[4][4];	// s7.8
	ITEint16           colorBias[4];			// s8

  /* Look up table */
	ITEuint8		   lookupTable[4][256];		// use SRAM


} ITEHardware;

typedef struct
{
	HWboolean	valid[CACHESET];
	ITEuint32	tag[CACHESET];
	ITEint16	word[CACHESET];
	ITEint8		replaceMark;
} ITECache16;

typedef struct
{
	HWboolean   valid[CACHESET];
	ITEuint32	tag[CACHESET];
	ITEint8 	word[CACHESET];
	ITEint8		replaceMark;
} ITECache8;


void ITEHardware_dtor(ITEHardwareRegister* hw);
void ITEHardware_ctor(ITEHardwareRegister* hw);
//void iteHardwareRender();
void iteHardwareFire(ITEHardwareRegister* hwReg);
void iteHardwareNullFire(ITEuint32 frameID);
//ITEHardware* iteCreateHardware();
void iteCreateHardware(ITEHardwareRegister* hwReg);
void iteDestroyHardware();
void iteStoreColor(ITEColor *c, ITEHColor *hc, void *data, VGImageFormat vg, ITEuint8 bit);
void iteLoadColor(ITEColor *c, ITEHColor *hc, const void *data, VGImageFormat vg, ITEuint8 bit);
void iteTessllationEngine();
void iteCoverageEngine();
void iteRenderEngine();

ITEuint32 iteHardwareGenFrameID();
ITEuint32 iteHardwareGenObjectID();
ITEuint32 iteHardwareGetCurrentFrameID();
ITEuint32 iteHardwareGetCurrentObjectID();
ITEuint32 iteHardwareGetNextFrameID();
ITEuint32 iteHardwareGetNextObjectID();
void iteHardwareWaitObjID(ITEuint32 objID);
void iteHardwareWaitFrameID(ITEuint32 frameID);

#endif /* __ITEHARDWARE_H */

