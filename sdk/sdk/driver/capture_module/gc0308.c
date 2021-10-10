#include <unistd.h>
#include "iic/mmp_iic.h"
#include "capture_module/gc0308.h"
#include "capture_module.h" 

//=============================================================================
//                Constant Definition
//=============================================================================
static uint8_t GC0308_IICADDR = 0x42 >> 1;  
#define POWER_MANAGEMENT 0x1A

#define RESET_MASK       (1 << 7)
#define TRI_STATE_ENABLE (1 << 7)


//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
typedef struct _REGPAIR
{
    uint8_t  addr;
    uint16_t value;
} REGPAIR;

typedef struct GC0308CaptureModuleDriverStruct
{
    CaptureModuleDriverStruct base;
} GC0308CaptureModuleDriverStruct;

//=============================================================================
//                Global Data Definition
//=============================================================================
static uint16_t gtGC0308CurDev   = 0xFF;
static MMP_BOOL gtGC0308InitDone = MMP_FALSE;

/* 32-Lead LFCSP , page 108*/
static REGPAIR  INPUT_REG[]      =
{
    #if 1
    {0xfe,  0x00},  
    {0x0f,  0x00},   //[7:4]VB_High_4bit, [3:0]HB_High_4bit
    {0x01,  0x6a},   //hb[7:0]
    {0x02,  0x70},   //vb[7:0]

    {0x03,  0x01},   //0x01f4 = 500 xposure //Benson
    {0x04,  0xF4},

    {0xe2,  0x00},  
    {0xe3,  0x96},   //anti-flicker step [7:0]
    {0xe4,  0x02},    //exp level 1
    {0xe5,  0x58},    
    {0xe6,  0x03},    //exp level 2
    {0xe7,  0x84},    
    {0xe8,  0x07},    //exp level 3
    {0xe9,  0x08},    
    {0xea,  0x0d},    //exp level 4
    {0xeb,  0x7a},  
    {0xec,  0x20},    //[5:4]exp_level  [3:0]minimum exposure high 4 bits
    {0x05,  0x00},  // row_start_high
    {0x06,  0x00},  // row_start_low
    {0x07,  0x00},  // col_start_high
    {0x08,  0x00},  // col_start_low
    {0x09,  0x01},  //[8]cis_win_height  488
    {0x0a,  0xe8},  //[7:0]cis_win_height
    {0x0b,  0x02},  //[9:8]cis_win_width 648
    {0x0c,  0x88},  //[7:0]cis_win_width
    {0x0d,  0x02},  //vs_st
    {0x0e,  0x02},  //vs_et
    {0x10,  0x26},  ////[7:4]restg_width, [3:0]sh_width
    {0x11,  0x0d},  //fd//[7:4]tx_width, [3:0]space width,*2
    {0x12,  0x2a},  //sh_delay
    {0x13,  0x00},  //[3:0] row_tail_width
    {0x14,  0x10},  //[7]hsync_always ,[6] NA,  [5:4] CFA sequence// [3:2]NA,  [1]upside_down,  [0] mirror 
    {0x15,  0x0a},  //[7:6]output_mode,,[5:4]restg_mode,[3:2]sdark_mode, [1]new exposure,[0]badframe_en
    {0x16,  0x05},  //[7:5]NA, [4]capture_ad_data_edge, [3:0]Number of A/D pipe stages 
    {0x17,  0x01},  //[7:6]analog_opa_r,[5]coltest_en, [4]ad_test_enable, 
                    //[3]tx_allow,[2]black sun correction,[1:0]black sun control reg
    {0x18,  0x44},  //[7]NA,  [6:4]column gain ee, [3]NA, [2:0]column gain eo
    {0x19,  0x44},  //[7]NA,  [6:4]column gain oe, [3]NA, [2:0]column gain oo
    {0x1a,  0x2a},  //1e//[7]rsv1,[6]rsv0, [5:4]coln_r, 
                    //[3:2]colg_r column gain opa bias current, [1]clk_delay, [0] apwd
    {0x1b,  0x00},  //[7:2]NA, [1:0]BIN4 AND BIN2
    {0x1c,  0x49},  //c1//[7]hrst_enbale, [6:4]da_rsg, [3]tx high enable, [2]NA, [1:0]da18_r 
    {0x1d,  0x9a},  //08//[7]vref_en, [6:4]da_vef, [3]da25_en, [2]NA, [1:0]da25_r,set da25 voltage
    {0x1e,  0x61},  //60//[7]LP_MTD,[6:5]opa_r,ADC's operating current,  [4:2]NA, [1:0]sref
    {0x1f,  0x36},//0x16},  //[7:6]NA, [5:4]sync_drv, [3:2]data_drv, [1:0]pclk_drv

    {0x20,  0xff},  //[7]bks[6]gamma[5]cc[4]ee[3]intp[2]dn[1]dd[0]lsc
    {0x21,  0xfa},  //[7]na[6]na[5]skin_ee[4]cbcr_hue_en[3]y_as_en[2]auto_gray_en[1]y_gamma_en[0]na
    {0x22,  0x57},  //[7]na [6]auto_dndd [5]auto_ee [4]auto_sa [3]na [2]abs [1]awb  [0]na
    {0x24,  0xa2},  //
    {0x25,  0x0f},  
                    //output sync_mode
    {0x26,  0x02},  //
    {0x2f,  0x01},  //debug mode3  
                    /////////////////////////////////////////////////////////////////
                    /////////////////////// grab     ////////////////////////////////
                    /////////////////////////////////////////////////////////////////
    {0x30,  0xf7},  //blk mode [7]dark current mode:1 use exp rated dark ,0 use ndark row calculated  
                    //[1]dark_current_en
                    //[0]offset_en
    {0x31,  0x50},  //blk_value limit.64 low align to 11bits;8 for 256 range
    {0x32,  0x00},  //global offset  
    {0x39,  0x04},  // exp_ate_darkc 
    {0x3a,  0x20},  //{7:6}offset submode {5:0}offset ratio
    {0x3b,  0x20},  //{7:6}darkc submode {5:0}dark current ratio
    {0x3c,  0x00},  //manual g1 offset
    {0x3d,  0x00},  //manual r offset
    {0x3e,  0x00},  //manual b offset
    {0x3f,  0x00},  //manual g2 offset
                    //gain
    {0x50,  0x14},  //10  //global gain

    {0x53,  0x80},  //G 
    {0x54,  0x87},  //R channel gain
    {0x55,  0x87},  //B channel gain
    {0x56,  0x80},  
                    /////////////////////////////////////////////////////////////////////
                    /////////////////////////// LSC_t    ////////////////////////////////
                    /////////////////////////////////////////////////////////////////////
    {0x8b,  0x10},  ////r2
    {0x8c,  0x10},  ////g2
    {0x8d,  0x10},  ////b2
    {0x8e,  0x10},  ////r4
    {0x8f,  0x10},  ////g4
    {0x90,  0x10},  ////b4
    {0x91,  0x3c},  ////[7]singed4 [6:0]row_cneter
    {0x92,  0x50},  ////col_center
    {0x5d,  0x12},  ///decrease 1
    {0x5e,  0x1a},  ///decrease 2
    {0x5f,  0x24},  ///decrease 3
                    /////////////////////////////////////////////////////////////////////
                    /////////////////////////// DNDD_t    ///////////////////////////////
                    /////////////////////////////////////////////////////////////////////
    {0x60,  0x07},  //[4]zero weight mode
                    //[3]share mode
                    //[2]c weight mode
                    //[1]lsc decrease mode
                    //[0]b mode
    {0x61,  0x15},  //[7:6]na
                    //[5:4]c weight adap ratio
                    //[3:2]dn lsc ratio
                    //[1:0]b ratio
    {0x62,  0x08},  //b base
                    //0x63,0x02 ,//b increase RO
    {0x64,  0x03},  //[7:4]n base [3:0]c weight
                    //0x65     //[7:4]n increase [3:0]c coeff
    {0x66,  0xe8},  //dark_th ,bright_th
    {0x67,  0x86},  //flat high, flat low
    {0x68,  0xa2},  //[7:4]dd limit [1:0]dd ratio

                    /////////////////////////////////////////////////////////////////////
                    /////////////////////////// asde_t    ///////////////////////////////
                    /////////////////////////////////////////////////////////////////////
    {0x069,  0x18},  //gain high th
    {0x06a,  0x0f},  //[7:4]dn_c slop  //[3]use post_gain [2]use pre_gain [1]use global gain [0]use col gain
    {0x06b,  0x00},  //[7:4]dn_b slop [3:0]dn_n slop
    {0x06c,  0x5f},  //[7:4]bright_th start [3:0]bright_th slop
    {0x06d,  0x8f},  //[7:4]dd_limit_start[3:0]dd_limit slop
    {0x06e,  0x55},  //[7:4]ee1 effect start [3:0]slope  broad
    {0x06f,  0x38},  //[7:4]ee2 effect start [3:0]slope  narrow
    {0x070,  0x15},  //saturation dec slope
    {0x071,  0x33},  //[7:4]low limit,[3:0]saturation slope
                     /////////////////////////////////////////////////////////////////////
                     /////////////////////////// eeintp_t    ///////////////////////////////
                     /////////////////////////////////////////////////////////////////////
    {0x72,  0xdc},   //[7]edge_add_mode [6]new edge mode [5]edge2_mode [4]HP_mode
                     //[3]lp intp en [2]lp edge en [1:0]lp edge mode
          
    {0x73,  0x80},   //[7]edge_add_mode2 [6]NA [5]only 2direction [4]fixed direction th
                     //[3]only defect map [2]intp_map dir [1]HP_acc [0]only edge map

                    //for high resolution in light scene
    {0x74,  0x02},  //direction th1
    {0x75,  0x3f},  //direction th2
    {0x76,  0x02},  //direction diff th      h>v+diff ; h>th1 ; v<th2
    {0x77,  0x47},  //[7:4]edge1_effect [3:0]edge2_effect
    {0x78,  0x88},  //[7:4]edge_pos_ratio  [3:0]edge neg ratio
    {0x79,  0x81},  //edge1_max,edge1_min
    {0x7a,  0x81},  //edge2_max,edge2_min
    {0x7b,  0x22},  //edge1_th,edge2_th
    {0x7c,  0xff},  //pos_edge_max,neg_edge_max
                    //for high resolution in light scene

                    //CCT
    {0x93,  0x46},  // <--40
    {0x94,  0x00},
    {0x95,  0x03},
    {0x96,  0xd0},
    {0x97,  0x40},  //
    {0x98,  0xf0},

    //YCPT
    {0xb1,  0x3c}, //manual cb
    {0xb2,  0x3c}, //manual cr
    {0xb3,  0x40},
    {0xb6,  0xe0},
    {0xbd,  0x3C},
    {0xbe,  0x36},  // [5:4]gray mode 00:4&8  01:4&12 10:4&20  11:8$16   [3:0] auto_gray
                    //=================
                    //AECT
    {0xd0,  0xC9},  // exp is gc mode
    {0xd1,  0x10},  //every N
    {0xd2,  0x90},  // 7 aec enable 5 clore y mode 4skin weight 3 weight drop mode
    {0xd3,  0x88},  //Y_target and low pixel thd high X4 low X2
    {0xd5,  0xF2},  //lhig
    {0xd6,  0x10},  // ignore mode
    {0xdb,  0x92},
    {0xdc,  0xA5},  //fast_margin  fast_ratio
    {0xdf,  0x23},  // I_fram D_ratio

    {0xd9,  0x00},  // colore offset in CAL ,now is too dark so set zero
    {0xda,  0x00},  // GB offset
    {0xe0,  0x09},

    {0xed,  0x04},  //minimum exposure low 8  bits
    {0xee,  0xa0},  //max_post_dg_gain
    {0xef,  0x40},  //max_pre_dg_gain

                    //ABBT
    {0x80,  0x03},
                    //RGB_gamma_m5
    {0x9F,  0x0E},
    {0xA0,  0x1C},
    {0xA1,  0x34},
    {0xA2,  0x48},
    {0xA3,  0x5A},
    {0xA4,  0x6B},
    {0xA5,  0x7B},
    {0xA6,  0x95},
    {0xA7,  0xAB},
    {0xA8,  0xBF},
    {0xA9,  0xCE},
    {0xAA,  0xD9},
    {0xAB,  0xE4},
    {0xAC,  0xEC},
    {0xAD,  0xF7},
    {0xAE,  0xFD},
    {0xAF,  0xFF},
                    //wint
                    //Y_gamma
    {0xc0,  0x00},  //Y_gamma_0 
    {0xc1,  0x14},  //Y_gamma_1 
    {0xc2,  0x21},  //Y_gamma_2 
    {0xc3,  0x36},  //Y_gamma_3 
    {0xc4,  0x49},  //Y_gamma_4 
    {0xc5,  0x5B},  //Y_gamma_5 
    {0xc6,  0x6B},  //Y_gamma_6 
    {0xc7,  0x7B},  //Y_gamma_7 
    {0xc8,  0x98},  //Y_gamma_8 
    {0xc9,  0xB4},  //Y_gamma_9 
    {0xca,  0xCE},  //Y_gamma_10
    {0xcb,  0xE8},  //Y_gamma_11
    {0xcc,  0xFF},  //Y_gamma_12
                    //ABS
    {0xf0,  0x02},
    {0xf1,  0x01},
    {0xf2,  0x04},  //manual stretch K
    {0xf3,  0x30},  //the limit of Y_stretch
    {0xf9,  0x9f},
    {0xfa,  0x78},
                    //################
                    //AWBp
                    //##############
    {0xfe,  0x01}, 
    {0x00,  0xf5},  //high_low limit
    {0x02,  0x20},  //y2c 
    {0x04,  0x10}, 
    {0x05,  0x10}, 
    {0x06,  0x20}, 
    {0x08,  0x15}, 
    {0x0a,  0xa0},  // number limit
    {0x0b,  0x64},  // skip_mode
    {0x0c,  0x08}, 
    {0x0e,  0x4C},  // width step
    {0x0f,  0x39},  // height step
    {0x10,  0x41}, 
    {0x11,  0x37},  // 0x3f
    {0x12,  0x24}, 
    {0x13,  0x39},  //13//smooth 2
    {0x14,  0x45},  //R_5k_gain_base
    {0x15,  0x45},  //B_5k_gain_base
    {0x16,  0xc2},  //sinT
    {0x17,  0xA8},  //ac//a8//a8//a8//cosT
    {0x18,  0x18},  //X1 thd
    {0x19,  0x55},  //X2 thd
    {0x1a,  0xd8},  //e4//d0//Y1 thd
    {0x1b,  0xf5},  //Y2 thd
    {0x70,  0x40},  // A R2G low	
    {0x71,  0x58},  // A R2G high
    {0x72,  0x30},  // A B2G low
    {0x73,  0x48},  // A B2G high
    {0x74,  0x20},  // A G low
    {0x75,  0x60},  // A G high  
    {0x77,  0x20}, 
    {0x78,  0x32},  
                    //HSP 
    {0x30,  0x03}, //[1]HSP_en [0]sa_curve_en
    {0x31,  0x40},
    {0x32,  0x10},
    {0x33,  0xe0},
    {0x34,  0xe0},
    {0x35,  0x00},
    {0x36,  0x80},
    {0x37,  0x00},
    {0x38,  0x04}, //sat1, at8   
    {0x39,  0x09},              
    {0x3a,  0x12},              
    {0x3b,  0x1C},              
    {0x3c,  0x28},              
    {0x3d,  0x31},              
    {0x3e,  0x44},              
    {0x3f,  0x57},              
    {0x40,  0x6C},              
    {0x41,  0x81},              
    {0x42,  0x94},              
    {0x43,  0xA7},              
    {0x44,  0xB8},              
    {0x45,  0xD6},              
    {0x46,  0xEE}, //sat15,at224
    {0x47,  0x0d}, //blue_edge_dec_ratio
                   //OUT
    {0xfe,  0x00},

    #else
    //24Mhz 10fps
    {0xfe, 0x80},//{0xde, 0x80},
    {0xfe, 0x00}, //MCLK=24MHz 10fps
    {0x0f, 0x05},	 //0x00
    {0x01, 0xe1},   //0x6a
    {0x02, 0x70},   //0x70
    {0xe2, 0x00},
    {0xe3, 0x96},
    {0xe4, 0x02},
    {0xe5, 0x58},
    {0xe6, 0x02},
    {0xe7, 0x58},
    {0xe8, 0x02},
    {0xe9, 0x58},
    {0xea, 0x0e},
    {0xeb, 0xa6},

    //sensor default register
    {0xfe, 0x00},
    {0xec, 0x20},
    {0x05, 0x00},
    {0x06, 0x00},
    {0x07, 0x00},
    {0x08, 0x00},
    {0x09, 0x01},//1e8 = 488 , change to 480
    {0x0a, 0xe8},//e8
    {0x0b, 0x02},
    {0x0c, 0x88},//0x88
    {0x0d, 0x02},
    {0x0e, 0x02},
    {0x10, 0x26},
    {0x11, 0x0d},
    {0x12, 0x2a},
    {0x13, 0x00},
    {0x14, 0x10}, //0x11 //Benson  Hsync always
    {0x15, 0x0a},
    {0x16, 0x05},
    {0x17, 0x01},
    {0x18, 0x44},
    {0x19, 0x44},
    {0x1a, 0x2a},
    {0x1b, 0x00},
    {0x1c, 0x49},
    {0x1d, 0x9a},
    {0x1e, 0x61},
    {0x1f, 0x16},
    {0x20, 0x7f},
    {0x21, 0xfa},
    {0x22, 0x57},
    {0x24, 0xa2},	//YCbYCr
    {0x25, 0x0f},
    {0x26, 0x03}, //benson  //0x03 // 0x01
    {0x28, 0x00},
    {0x2d, 0x0a},
    {0x2f, 0x01},
    {0x30, 0xf7},
    {0x31, 0x50},
    {0x32, 0x00},
    {0x33, 0x28},
    {0x34, 0x2a},
    {0x35, 0x28},
    {0x39, 0x04},
    {0x3a, 0x20},
    {0x3b, 0x20},
    {0x3c, 0x00},
    {0x3d, 0x00},
    {0x3e, 0x00},
    {0x3f, 0x00},
    {0x50, 0x14}, // 0x14
    {0x52, 0x41},
    {0x53, 0x80},
    {0x54, 0x80},
    {0x55, 0x80},
    {0x56, 0x80},
    {0x8b, 0x20},
    {0x8c, 0x20},
    {0x8d, 0x20},
    {0x8e, 0x14},
    {0x8f, 0x10},
    {0x90, 0x14},
    {0x91, 0x3c},
    {0x92, 0x50},
    //{{0x8b},{0x10}},
    //{{0x8c},{0x10}},
    //{{0x8d},{0x10}},
    //{{0x8e},{0x10}},
    //{{0x8f},{0x10}},
    //{{0x90},{0x10}},
    //{{0x91},{0x3c}},
    //{{0x92},{0x50}},
    {0x5d, 0x12},
    {0x5e, 0x1a},
    {0x5f, 0x24},
    {0x60, 0x07},
    {0x61, 0x15},
    {0x62, 0x08}, // 0x08
    {0x64, 0x03},  // 0x03
    {0x66, 0xe8},
    {0x67, 0x86},
    {0x68, 0x82},
    {0x69, 0x18},
    {0x6a, 0x0f},
    {0x6b, 0x00},
    {0x6c, 0x5f},
    {0x6d, 0x8f},
    {0x6e, 0x55},
    {0x6f, 0x38},
    {0x70, 0x15},
    {0x71, 0x33},
    {0x72, 0xdc},
    {0x73, 0x00},
    {0x74, 0x02},
    {0x75, 0x3f},
    {0x76, 0x02},
    {0x77, 0x38}, // 0x47
    {0x78, 0x88},
    {0x79, 0x81},
    {0x7a, 0x81},
    {0x7b, 0x22},
    {0x7c, 0xff},
    {0x93, 0x48},  //color matrix default
    {0x94, 0x02},
    {0x95, 0x07},
    {0x96, 0xe0},
    {0x97, 0x40},
    {0x98, 0xf0},
    {0xb1, 0x40},
    {0xb2, 0x40},
    {0xb3, 0x40}, //0x40
    {0xb6, 0xe0},
    {0xbd, 0x38},
    {0xbe, 0x36},
    {0xd0, 0xCB},
    {0xd1, 0x10},
    {0xd2, 0x90},
    {0xd3, 0x48},
    {0xd5, 0xF2},
    {0xd6, 0x16},
    {0xdb, 0x92},
    {0xdc, 0xA5},
    {0xdf, 0x23},
    {0xd9, 0x00},
    {0xda, 0x00},
    {0xe0, 0x09},
    {0xed, 0x04},
    {0xee, 0xa0},
    {0xef, 0x40},
    {0x80, 0x03},
                
    {0x9F, 0x10},
    {0xA0, 0x20},
    {0xA1, 0x38},
    {0xA2, 0x4e},
    {0xA3, 0x63},
    {0xA4, 0x76},
    {0xA5, 0x87},
    {0xA6, 0xa2},
    {0xA7, 0xb8},
    {0xA8, 0xca},
    {0xA9, 0xd8},
    {0xAA, 0xe3},
    {0xAB, 0xeb},
    {0xAC, 0xf0},
    {0xAD, 0xF8},
    {0xAE, 0xFd},
    {0xAF, 0xFF},
                
    {0xc0, 0x00},
    {0xc1, 0x10},
    {0xc2, 0x1c},
    {0xc3, 0x30},
    {0xc4, 0x43},
    {0xc5, 0x54},
    {0xc6, 0x65},
    {0xc7, 0x75},
    {0xc8, 0x93},
    {0xc9, 0xB0},
    {0xca, 0xCB},
    {0xcb, 0xE6},
    {0xcc, 0xFF},
    {0xf0, 0x02},
    {0xf1, 0x01},
    {0xf2, 0x02},
    {0xf3, 0x30},
    {0xf7, 0x12},
    {0xf8, 0x0a},
    {0xf9, 0x9f},
    {0xfa, 0x78},
    {0xfe, 0x01},
    {0x00, 0xf5},
    {0x02, 0x20},
    {0x04, 0x10},
    {0x05, 0x08},
    {0x06, 0x20},
    {0x08, 0x0a},
    {0x0a, 0xa0},
    {0x0b, 0x60},
    {0x0c, 0x08},
    {0x0e, 0x44},
    {0x0f, 0x32},
    {0x10, 0x41},
    {0x11, 0x37},
    {0x12, 0x22},
    {0x13, 0x19},
    {0x14, 0x44},
    {0x15, 0x44},
    {0x16, 0xc2},
    {0x17, 0xA8},
    {0x18, 0x18},
    {0x19, 0x50},
    {0x1a, 0xd8},
    {0x1b, 0xf5},
    {0x70, 0x40},
    {0x71, 0x58},
    {0x72, 0x30},
    {0x73, 0x48},
    {0x74, 0x20},
    {0x75, 0x60},
    {0x77, 0x20},
    {0x78, 0x32},
    {0x30, 0x03},
    {0x31, 0x40},
    {0x32, 0x10},
    {0x33, 0xe0},
    {0x34, 0xe0},
    {0x35, 0x00},
    {0x36, 0x80},
    {0x37, 0x00},
    {0x38, 0x04},
    {0x39, 0x09},
    {0x3a, 0x12},
    {0x3b, 0x1C},
    {0x3c, 0x28},
    {0x3d, 0x31},
    {0x3e, 0x44},
    {0x3f, 0x57},
    {0x40, 0x6C},
    {0x41, 0x81},
    {0x42, 0x94},
    {0x43, 0xA7},
    {0x44, 0xB8},
    {0x45, 0xD6},
    {0x46, 0xEE},
    {0x47, 0x0d},
    {0x62, 0xf7},
    {0x63, 0x68},
    {0x64, 0xd3},
    {0x65, 0xd3},
    {0x66, 0x60},
    {0xfe, 0x00},
    #endif   
};

static CAP_TIMINFO_TABLE GC0308_TABLE [] = {
    //Index, HActive, VActive,  Rate,            FrameRate,                 Hpor,   Vpor,  HStar,     HEnd,   VStar1,   VEnd1,  VStar2,   VEnd2,
    {0,     640,    480,        1000,   CAP_FRAMERATE_10HZ,                   0,      0,      0,        0,        0,      0,   0,       0   }, //480i60     // Benson
};      

//=============================================================================
//                Private Function Definition
//=============================================================================
static uint8_t _GC0308_ReadI2c_Byte(uint8_t RegAddr)
{
    uint8_t    data;
    MMP_RESULT flag;

    //mmpIicLockModule(IIC_PORT_0);
    if (0 != (flag = mmpIicReceiveData(IIC_PORT_0, IIC_MASTER_MODE, GC0308_IICADDR, &RegAddr, 1, &data, sizeof(data))))
    {
        printf("GC0308 I2C Read error, reg = %02x\n", RegAddr);
        mmpIicGenStop(IIC_PORT_0);
    }
    //mmpIicReleaseModule(IIC_PORT_0);
    return data;
}

static MMP_RESULT _GC0308_WriteI2c_Byte(uint8_t RegAddr, uint8_t data)
{
    MMP_RESULT flag;

    //mmpIicLockModule(IIC_PORT_0);
    if (0 != (flag = mmpIicSendData(IIC_PORT_0, IIC_MASTER_MODE, GC0308_IICADDR, RegAddr, &data, sizeof(data))))
    {
        printf("GC0308 I2c write error, reg = %02x val =%02x\n", RegAddr, data);
        mmpIicGenStop(IIC_PORT_0);
    }
    //mmpIicReleaseModule(IIC_PORT_0);
    return flag;
}

static MMP_RESULT _GC0308_WriteI2c_ByteMask(uint8_t RegAddr, uint8_t data, uint8_t mask)
{
    uint8_t    value;
    MMP_RESULT flag;

    //mmpIicLockModule(IIC_PORT_0);
    if (0 != (flag = mmpIicReceiveData(IIC_PORT_0, IIC_MASTER_MODE, GC0308_IICADDR, &RegAddr, 1, &value, 1)))
    {
        printf("GC0308 I2C Read error, reg = %02x\n", RegAddr);
        mmpIicGenStop(IIC_PORT_0);
    }

    value = ((value & ~mask) | (data & mask));

    if (0 != (flag = mmpIicSendData(IIC_PORT_0, IIC_MASTER_MODE, GC0308_IICADDR, RegAddr, &value, 1)))
    {
        printf("GC0308 I2c write error, reg = %02x val =%02x\n", RegAddr, value);
        mmpIicGenStop(IIC_PORT_0);
    }
    //mmpIicReleaseModule(IIC_PORT_0);
    return flag;
}

static void _GC0308_SWReset()
{
    _GC0308_WriteI2c_Byte(0xFE,0x70);
}

static void _Set_GC0308_Input_Setting(void)
{
    uint16_t i;

    for (i = 0; i < (sizeof(INPUT_REG) / sizeof(REGPAIR)); i++)
    {
        _GC0308_WriteI2c_Byte(INPUT_REG[i].addr, INPUT_REG[i].value);
    }
}

//=============================================================================
//                Public Function Definition
//=============================================================================

void GC0308Initial()
{
    uint8_t    data;
    printf("GC0308Initial\n");

    gtGC0308InitDone = MMP_FALSE;
    _GC0308_SWReset();

    usleep(1000* 10);
    _Set_GC0308_Input_Setting();
    gtGC0308InitDone = MMP_TRUE;
}


void GC0308_PowerDown(
    MMP_BOOL enable)
{
    //When PDBP is set to 1, setting the PWRDWN bit switches the GC0308 to a chip-wide power-down mode.
    if (enable)
    {
        gtGC0308InitDone = MMP_FALSE;
        _GC0308_WriteI2c_ByteMask(POWER_MANAGEMENT, 0x01, 0x01);
    }
    else
        _GC0308_WriteI2c_ByteMask(POWER_MANAGEMENT, 0x00, 0x01);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////

//X10LightDriver_t1.c
void GC0308Initialize(void)
{
    printf("GC0308Initialize here\n");
    GC0308Initial();
}

void GC0308Terminate(void)
{
    printf("GC0308Terminate\n");
}

void GC0308OutputPinTriState(unsigned char flag)
{
}

unsigned char GC0308IsSignalStable(void)
{
	return true;
}

void GC0308GetProperty(CAP_GET_PROPERTY * pGetProperty)
{
    uint16_t i;
    pGetProperty->GetTopFieldPolarity = MMP_FALSE;
    pGetProperty->GetHeight = 480;
    pGetProperty->GetWidth  = 640;
    pGetProperty->Rate = 1000;
    pGetProperty->GetModuleIsInterlace = 0;

    for (i = 0; i < (sizeof(GC0308_TABLE) / sizeof(CAP_TIMINFO_TABLE)); ++i)
    {
       if ((pGetProperty->GetWidth == GC0308_TABLE[i].HActive) && (pGetProperty->GetHeight == GC0308_TABLE[i].VActive) && pGetProperty->Rate == GC0308_TABLE[i].Rate)
        {
            pGetProperty->HPolarity       = GC0308_TABLE[i].HPolarity; 
            pGetProperty->VPolarity       = GC0308_TABLE[i].VPolarity;
            pGetProperty->FrameRate       = GC0308_TABLE[i].FrameRate; 
            pGetProperty->matchResolution = MMP_TRUE;

            pGetProperty->HStar  = GC0308_TABLE[i].HStar;
            pGetProperty->HEnd   = GC0308_TABLE[i].HEnd;
            pGetProperty->VStar1 = GC0308_TABLE[i].VStar1;
            pGetProperty->VEnd1  = GC0308_TABLE[i].VEnd1;
            pGetProperty->VStar2 = GC0308_TABLE[i].VStar2; 
            pGetProperty->VEnd2  = GC0308_TABLE[i].VEnd2; 
        }
    }
}

void GC0308PowerDown(unsigned char enable)
{
    GC0308_PowerDown(enable);
}

void GC0308ForCaptureDriverSetting(CAP_CONTEXT *Capctxt )
{
    printf("GC0308ForCaptureDriverSetting\n");
    /* Input Format default Setting */
    Capctxt->Interleave   = Progressive;

    /* Input Data Format Setting */
    // 8bit bus
    Capctxt->YUV422Format  = CAP_IN_YUV422_YUYV;
    Capctxt->EnDEMode             = MMP_TRUE;
    Capctxt->input_protocol       = BT_601;
    
    Capctxt->input_video_source.HSYNCDE =   MMP_TRUE;
}

static void GC0308CaptureModuleDriver_Destory(CaptureModuleDriver base)
{
    GC0308CaptureModuleDriver self = (GC0308CaptureModuleDriver)base;
    if(self)
        free(self);
}

static CaptureModuleDriverInterfaceStruct interface =
{
    GC0308Initialize,
    GC0308Terminate,
    GC0308OutputPinTriState,
    GC0308IsSignalStable,
    GC0308GetProperty,
    GC0308PowerDown,
    GC0308ForCaptureDriverSetting,
    GC0308CaptureModuleDriver_Destory
};

CaptureModuleDriver GC0308CaptureModuleDriver_Create()
{
    GC0308CaptureModuleDriver self = calloc(1, sizeof(GC0308CaptureModuleDriverStruct));
    self->base.vtable = &interface;
    self->base.type = "GC0308";
    return (GC0308CaptureModuleDriver)self;
}
//end of X10LightDriver_t1.c
