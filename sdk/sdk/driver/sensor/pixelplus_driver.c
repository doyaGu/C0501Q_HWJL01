#include "encoder/encoder_types.h"
#include "sensor/pixelplus_io.h"
#include "sensor/pixelplus_driver.h"

//=============================================================================
//                Constant Definition
//=============================================================================
#define AntiFlicker_60Hz_MODE 8
#define AntiFlicker_50Hz_MODE 4
#define GPIO_BASE               0xDE000000
//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================


//=============================================================================
//                Global Data Definition
//=============================================================================
static bool gbPixelPlusInit = MMP_FALSE;
static uint8_t gFlickerMode = 0;

//=============================================================================
//                Private Function Definition
//=============================================================================
static void
_PixelPlus_PO3100K_20bit_SMPTE_30fps(
    void)
{
    //////////////////////////////////////// start up////////////////////////////////////////
    PixelPlus_WriteI2C_Byte(0x03, 0x00);
    PixelPlus_WriteI2C_Byte(0x2D, 0x01);
    //PixelPlus_WriteI2C_Byte(0x29, 0x9D);	// output Hi-z release 20bit

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    PixelPlus_WriteI2C_Byte(0x03, 0x00);
    PixelPlus_WriteI2C_Byte(0x04, 0x00);	// chip mode (00 : smpte, 01 : sampling, 02 : DIGITAL)
    PixelPlus_WriteI2C_Byte(0x05, 0x00);	// mirror

    //PixelPlus_WriteI2C_Byte(0x41, 0x02);	// pll_ms
    //PixelPlus_WriteI2C_Byte(0x42, 0x0B);	// pll_ns

    //PixelPlus_WriteI2C_Byte(0x40, 0x3C);// pll_control
    //$0500////////////////////////////////////////////// need to delay about 500ms
    //PixelPlus_WriteI2C_Byte(0x40, 0x2C);// pll_control

    PixelPlus_WriteI2C_Byte(0x06, 0x06);// framewidth_h
    PixelPlus_WriteI2C_Byte(0x07, 0x71);// framewidth_l
    PixelPlus_WriteI2C_Byte(0x08, 0x02);// fd_fheight_a_h
    PixelPlus_WriteI2C_Byte(0x09, 0xED);// fd_fheight_a_l
    PixelPlus_WriteI2C_Byte(0x0A, 0x02);// fd_fheight_b_h
    PixelPlus_WriteI2C_Byte(0x0B, 0xED);// fd_fheight_b_l

    PixelPlus_WriteI2C_Byte(0x0C, 0x00);// windowx1_h
    PixelPlus_WriteI2C_Byte(0x0D, 0x05);// windowx1_l
    PixelPlus_WriteI2C_Byte(0x0E, 0x00);// windowy1_h
    PixelPlus_WriteI2C_Byte(0x0F, 0x05);// windowy1_l
    PixelPlus_WriteI2C_Byte(0x10, 0x05);// windowx2_h
    PixelPlus_WriteI2C_Byte(0x11, 0x04);// windowx2_l
    PixelPlus_WriteI2C_Byte(0x12, 0x02);// windowy2_h
    PixelPlus_WriteI2C_Byte(0x13, 0xD4);// windowy2_l

    PixelPlus_WriteI2C_Byte(0x14, 0x00);// vsyncstartrow_f0_h
    PixelPlus_WriteI2C_Byte(0x15, 0x15);// vsyncstartrow_f0_l
    PixelPlus_WriteI2C_Byte(0x16, 0x02);// vsyncstoprow_f0_h
    PixelPlus_WriteI2C_Byte(0x17, 0xE9);// vsyncstoprow_f0_l

    ////////////////////////// Start Settings////////////////////////////////

    PixelPlus_WriteI2C_Byte(0x03, 0x00);
    PixelPlus_WriteI2C_Byte(0x33, 0x05);// pixelbias (130703 khpark)
    PixelPlus_WriteI2C_Byte(0x34, 0x02);// compbias
    PixelPlus_WriteI2C_Byte(0x36, 0x80);// tx_bais, recommended by design 1

    PixelPlus_WriteI2C_Byte(0x03, 0x00);
    PixelPlus_WriteI2C_Byte(0x38, 0x58);// black_bias '011', rangesel "000", recommended by design 1

    PixelPlus_WriteI2C_Byte(0x03, 0x01);
    PixelPlus_WriteI2C_Byte(0x1E, 0x0E);// bsmode '0'
    PixelPlus_WriteI2C_Byte(0x26, 0x03);// blacksunth_h

    ////////////////////////// BLACK////////////////////////////////
    PixelPlus_WriteI2C_Byte(0x03, 0x01);
    PixelPlus_WriteI2C_Byte(0xB1, 0x30);// adcoffset

    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x06, 0x98);// front_black_fitting[4],
     	// ycont/ybright[3],
     	// adcoffset_fitting[2]

    PixelPlus_WriteI2C_Byte(0x03, 0x01);
    PixelPlus_WriteI2C_Byte(0xA4, 0x88);	//front_black_ref0
    PixelPlus_WriteI2C_Byte(0xA5, 0x88);	//front_black_ref1
    PixelPlus_WriteI2C_Byte(0xA6, 0x88);	//front_black_ref2
    PixelPlus_WriteI2C_Byte(0xA7, 0x00);	//front_black_ref3
    PixelPlus_WriteI2C_Byte(0xA8, 0x00);	//front_black_ref4
    PixelPlus_WriteI2C_Byte(0xA9, 0x08);	//front_black_ref5

    ////////////////////////// AWB////////////////////////////////

    //AWB gain control
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x06, 0xB8);

    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x75, 0x28);
    PixelPlus_WriteI2C_Byte(0x76, 0x28);
    PixelPlus_WriteI2C_Byte(0x77, 0x78);
    PixelPlus_WriteI2C_Byte(0x78, 0x78);
    PixelPlus_WriteI2C_Byte(0x79, 0x48);
    PixelPlus_WriteI2C_Byte(0x7A, 0x48);
    PixelPlus_WriteI2C_Byte(0x7B, 0xB8);
    PixelPlus_WriteI2C_Byte(0x7C, 0xB8);
    PixelPlus_WriteI2C_Byte(0x7D, 0x01);
    PixelPlus_WriteI2C_Byte(0x7E, 0x00);
    PixelPlus_WriteI2C_Byte(0x7F, 0x02);
    PixelPlus_WriteI2C_Byte(0x80, 0x07);

    //AWB option
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x73, 0x08);
    PixelPlus_WriteI2C_Byte(0x74, 0x04);

    // Set AWB Sampling Boundary
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x51, 0x10);
    PixelPlus_WriteI2C_Byte(0x52, 0xE0);
    PixelPlus_WriteI2C_Byte(0x53, 0x02);
    PixelPlus_WriteI2C_Byte(0x54, 0x02);
    PixelPlus_WriteI2C_Byte(0x55, 0x40);
    PixelPlus_WriteI2C_Byte(0x56, 0xC0);
    PixelPlus_WriteI2C_Byte(0x57, 0x04);
    PixelPlus_WriteI2C_Byte(0x58, 0x6E);
    PixelPlus_WriteI2C_Byte(0x59, 0x45);

    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x5A, 0x23);
    PixelPlus_WriteI2C_Byte(0x5B, 0x4B);
    PixelPlus_WriteI2C_Byte(0x5C, 0x64);
    PixelPlus_WriteI2C_Byte(0x5D, 0xAA);
    PixelPlus_WriteI2C_Byte(0x5E, 0x23);
    PixelPlus_WriteI2C_Byte(0x5F, 0x28);
    PixelPlus_WriteI2C_Byte(0x60, 0x4B);
    PixelPlus_WriteI2C_Byte(0x61, 0x73);
    PixelPlus_WriteI2C_Byte(0x62, 0x3C);
    PixelPlus_WriteI2C_Byte(0x63, 0x87);
    PixelPlus_WriteI2C_Byte(0x64, 0x2D);
    PixelPlus_WriteI2C_Byte(0x65, 0x2D);

    //awb rg/bg ratio axis
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x6E, 0x3A);
    PixelPlus_WriteI2C_Byte(0x6F, 0x50);
    PixelPlus_WriteI2C_Byte(0x70, 0x60);

    //lens / cs axis
    PixelPlus_WriteI2C_Byte(0x03, 0x03);
    PixelPlus_WriteI2C_Byte(0x16, 0x3A);
    PixelPlus_WriteI2C_Byte(0x17, 0x50);
    PixelPlus_WriteI2C_Byte(0x18, 0x60);

    ////////////////////////// AE////////////////////////////////
    //Y target contrl
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x05, 0x64);
    PixelPlus_WriteI2C_Byte(0x3B, 0x90);
    PixelPlus_WriteI2C_Byte(0x3C, 0x78);
    PixelPlus_WriteI2C_Byte(0x3D, 0x70);
    PixelPlus_WriteI2C_Byte(0x3E, 0x78);
    PixelPlus_WriteI2C_Byte(0x3F, 0x24);
    PixelPlus_WriteI2C_Byte(0x40, 0x4B);

    //Auto exposure option
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x48, 0x08);
    PixelPlus_WriteI2C_Byte(0x49, 0x08);
    PixelPlus_WriteI2C_Byte(0x4A, 0x08);

    //saturation level th
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x2C, 0xBB);//66

    //saturation ratio fitting
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x41, 0x04);
    PixelPlus_WriteI2C_Byte(0x42, 0x08);
    PixelPlus_WriteI2C_Byte(0x43, 0x10);
    PixelPlus_WriteI2C_Byte(0x44, 0x20);
    PixelPlus_WriteI2C_Byte(0x2E, 0x04);//05

    //Flicker canceling mode - manual 60hz
    PixelPlus_WriteI2C_Byte(0x03, 0x00);
    PixelPlus_WriteI2C_Byte(0x4F, 0x08);

    PixelPlus_WriteI2C_Byte(0x03, 0x00);
    PixelPlus_WriteI2C_Byte(0x59, 0x00);
    PixelPlus_WriteI2C_Byte(0x5A, 0xBA);
    PixelPlus_WriteI2C_Byte(0x5B, 0x00);

    ////////////////////////////// COLOR////////////////////////////////
    //Color correction
    PixelPlus_WriteI2C_Byte(0x03, 0x02);
    PixelPlus_WriteI2C_Byte(0x33, 0x07);
    PixelPlus_WriteI2C_Byte(0x34, 0x00);
    PixelPlus_WriteI2C_Byte(0x35, 0x07);
    PixelPlus_WriteI2C_Byte(0x36, 0x0E);
    PixelPlus_WriteI2C_Byte(0x37, 0x0B);
    PixelPlus_WriteI2C_Byte(0x38, 0x0C);
    PixelPlus_WriteI2C_Byte(0x39, 0x02);
    PixelPlus_WriteI2C_Byte(0x3A, 0x08);
    PixelPlus_WriteI2C_Byte(0x3B, 0x0A);

    //Color saturation
    PixelPlus_WriteI2C_Byte(0x03, 0x03);
    PixelPlus_WriteI2C_Byte(0x0C, 0x25);
    PixelPlus_WriteI2C_Byte(0x0D, 0x88);
    PixelPlus_WriteI2C_Byte(0x0E, 0x00);
    PixelPlus_WriteI2C_Byte(0x0F, 0x25);

    ////////////////////////////// GAMMA////////////////////////////////
    //gamma curve fitting (0.45)
    PixelPlus_WriteI2C_Byte(0x03, 0x02);
    PixelPlus_WriteI2C_Byte(0x3D, 0x00);
    PixelPlus_WriteI2C_Byte(0x3E, 0x27);
    PixelPlus_WriteI2C_Byte(0x3F, 0x36);
    PixelPlus_WriteI2C_Byte(0x40, 0x40);
    PixelPlus_WriteI2C_Byte(0x41, 0x49);
    PixelPlus_WriteI2C_Byte(0x42, 0x58);
    PixelPlus_WriteI2C_Byte(0x43, 0x64);
    PixelPlus_WriteI2C_Byte(0x44, 0x78);
    PixelPlus_WriteI2C_Byte(0x45, 0x89);
    PixelPlus_WriteI2C_Byte(0x46, 0xA4);
    PixelPlus_WriteI2C_Byte(0x47, 0xBB);
    PixelPlus_WriteI2C_Byte(0x48, 0xCF);
    PixelPlus_WriteI2C_Byte(0x49, 0xE0);
    PixelPlus_WriteI2C_Byte(0x4A, 0xF1);
    PixelPlus_WriteI2C_Byte(0x4B, 0xFF);

    //gamma curve fitting (0.75)
    PixelPlus_WriteI2C_Byte(0x03, 0x02);
    PixelPlus_WriteI2C_Byte(0x5B, 0x00);
    PixelPlus_WriteI2C_Byte(0x5C, 0x0B);
    PixelPlus_WriteI2C_Byte(0x5D, 0x13);
    PixelPlus_WriteI2C_Byte(0x5E, 0x1A);
    PixelPlus_WriteI2C_Byte(0x5F, 0x20);
    PixelPlus_WriteI2C_Byte(0x60, 0x2B);
    PixelPlus_WriteI2C_Byte(0x61, 0x36);
    PixelPlus_WriteI2C_Byte(0x62, 0x49);
    PixelPlus_WriteI2C_Byte(0x63, 0x5A);
    PixelPlus_WriteI2C_Byte(0x64, 0x7B);
    PixelPlus_WriteI2C_Byte(0x65, 0x98);
    PixelPlus_WriteI2C_Byte(0x66, 0xB4);
    PixelPlus_WriteI2C_Byte(0x67, 0xCE);
    PixelPlus_WriteI2C_Byte(0x68, 0xE7);
    PixelPlus_WriteI2C_Byte(0x69, 0xFF);

    //gamma curve fitting (0.75)
    PixelPlus_WriteI2C_Byte(0x03, 0x02);
    PixelPlus_WriteI2C_Byte(0x6A, 0x00);
    PixelPlus_WriteI2C_Byte(0x6B, 0x0B);
    PixelPlus_WriteI2C_Byte(0x6C, 0x13);
    PixelPlus_WriteI2C_Byte(0x6D, 0x1A);
    PixelPlus_WriteI2C_Byte(0x6E, 0x20);
    PixelPlus_WriteI2C_Byte(0x6F, 0x2B);
    PixelPlus_WriteI2C_Byte(0x70, 0x36);
    PixelPlus_WriteI2C_Byte(0x71, 0x49);
    PixelPlus_WriteI2C_Byte(0x72, 0x5A);
    PixelPlus_WriteI2C_Byte(0x73, 0x7B);
    PixelPlus_WriteI2C_Byte(0x74, 0x98);
    PixelPlus_WriteI2C_Byte(0x75, 0xB4);
    PixelPlus_WriteI2C_Byte(0x76, 0xCE);
    PixelPlus_WriteI2C_Byte(0x77, 0xE7);
    PixelPlus_WriteI2C_Byte(0x78, 0xFF);

    //Y weight control
    PixelPlus_WriteI2C_Byte(0x03, 0x02);
    PixelPlus_WriteI2C_Byte(0x8D, 0x30);

    ////////////////////////////// DARK////////////////////////////////
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x09, 0x00);// 01->00

    // dark_dpc_p
    PixelPlus_WriteI2C_Byte(0x03, 0x03);
    PixelPlus_WriteI2C_Byte(0x26, 0x00);
    PixelPlus_WriteI2C_Byte(0x27, 0x10);
    PixelPlus_WriteI2C_Byte(0x28, 0x20);

    // dark_blf
    PixelPlus_WriteI2C_Byte(0x03, 0x03);
    PixelPlus_WriteI2C_Byte(0x2E, 0x7F);
    PixelPlus_WriteI2C_Byte(0x2F, 0x7F);
    PixelPlus_WriteI2C_Byte(0x30, 0x7F);

    // dark_nf
    PixelPlus_WriteI2C_Byte(0x03, 0x03);
    PixelPlus_WriteI2C_Byte(0x32, 0x00);
    PixelPlus_WriteI2C_Byte(0x33, 0x00);
    PixelPlus_WriteI2C_Byte(0x34, 0x00);

    PixelPlus_WriteI2C_Byte(0x03, 0x03);
    PixelPlus_WriteI2C_Byte(0x77, 0x00);// dark_dc
    PixelPlus_WriteI2C_Byte(0x78, 0x00);
    PixelPlus_WriteI2C_Byte(0x79, 0x00);

    // dark_e_blf
    PixelPlus_WriteI2C_Byte(0x03, 0x03);
    PixelPlus_WriteI2C_Byte(0xA1, 0x30);
    PixelPlus_WriteI2C_Byte(0xA2, 0x7F);
    PixelPlus_WriteI2C_Byte(0xA3, 0x7F);

    //////////////////////////////////////// CDS710 delay////////////////////////////////////////
    PixelPlus_WriteI2C_Byte(0x03, 0x01);// bank B

    PixelPlus_WriteI2C_Byte(0x19, 0xC4);// default C0   bayer_control_04 '1' ramp pclk count '0' ramp 1/2 pclk count
    //PixelPlus_WriteI2C_Byte(0x1C, 0x22);// default 11   bayer_control_07 rampspeed, adcspeed

    PixelPlus_WriteI2C_Byte(0x5F, 0x02);
    PixelPlus_WriteI2C_Byte(0x60, 0xC8);
    PixelPlus_WriteI2C_Byte(0x61, 0x03);
    PixelPlus_WriteI2C_Byte(0x62, 0x66);
    PixelPlus_WriteI2C_Byte(0x63, 0x00);
    PixelPlus_WriteI2C_Byte(0x64, 0x00);
    PixelPlus_WriteI2C_Byte(0x65, 0x00);
    PixelPlus_WriteI2C_Byte(0x66, 0x00);
    PixelPlus_WriteI2C_Byte(0x67, 0x03);
    PixelPlus_WriteI2C_Byte(0x68, 0x79);
    PixelPlus_WriteI2C_Byte(0x69, 0x03);
    PixelPlus_WriteI2C_Byte(0x6A, 0x8C);
    PixelPlus_WriteI2C_Byte(0x5B, 0x00);
    PixelPlus_WriteI2C_Byte(0x5C, 0x02);
    PixelPlus_WriteI2C_Byte(0x5D, 0x06);
    PixelPlus_WriteI2C_Byte(0x5E, 0x1C);
    PixelPlus_WriteI2C_Byte(0x57, 0x06);
    PixelPlus_WriteI2C_Byte(0x58, 0x24);
    PixelPlus_WriteI2C_Byte(0x59, 0x06);
    PixelPlus_WriteI2C_Byte(0x5A, 0x53);
    PixelPlus_WriteI2C_Byte(0x53, 0x02);
    PixelPlus_WriteI2C_Byte(0x54, 0xD0);
    PixelPlus_WriteI2C_Byte(0x55, 0x06);
    PixelPlus_WriteI2C_Byte(0x56, 0x24);
    PixelPlus_WriteI2C_Byte(0x99, 0x03);
    PixelPlus_WriteI2C_Byte(0x9A, 0x79);
    PixelPlus_WriteI2C_Byte(0x9B, 0x06);
    PixelPlus_WriteI2C_Byte(0x9C, 0x24);
    PixelPlus_WriteI2C_Byte(0x6F, 0x03);
    PixelPlus_WriteI2C_Byte(0x70, 0x96);
    PixelPlus_WriteI2C_Byte(0x71, 0x06);
    PixelPlus_WriteI2C_Byte(0x72, 0x1A);
    PixelPlus_WriteI2C_Byte(0x73, 0x03);
    PixelPlus_WriteI2C_Byte(0x74, 0x9C);
    PixelPlus_WriteI2C_Byte(0x75, 0x06);
    PixelPlus_WriteI2C_Byte(0x76, 0x18);
    PixelPlus_WriteI2C_Byte(0x77, 0x06);
    PixelPlus_WriteI2C_Byte(0x78, 0x46);
    PixelPlus_WriteI2C_Byte(0x79, 0x06);
    PixelPlus_WriteI2C_Byte(0x7A, 0x50);
    PixelPlus_WriteI2C_Byte(0x7B, 0x03);
    PixelPlus_WriteI2C_Byte(0x7C, 0x9C);
    PixelPlus_WriteI2C_Byte(0x7D, 0x06);
    PixelPlus_WriteI2C_Byte(0x7E, 0x18);
    PixelPlus_WriteI2C_Byte(0x8F, 0x03);
    PixelPlus_WriteI2C_Byte(0x90, 0x73);
    PixelPlus_WriteI2C_Byte(0xAE, 0x20);
    PixelPlus_WriteI2C_Byte(0xAF, 0x20);
    PixelPlus_WriteI2C_Byte(0xB0, 0x20);
    PixelPlus_WriteI2C_Byte(0xB1, 0x20);
    PixelPlus_WriteI2C_Byte(0x8B, 0x03);
    PixelPlus_WriteI2C_Byte(0x8C, 0x9E);
    PixelPlus_WriteI2C_Byte(0x8D, 0x06);
    PixelPlus_WriteI2C_Byte(0x8E, 0x30);
    PixelPlus_WriteI2C_Byte(0x0C, 0x00);
    PixelPlus_WriteI2C_Byte(0x87, 0x06);
    PixelPlus_WriteI2C_Byte(0x88, 0x28);
    PixelPlus_WriteI2C_Byte(0x89, 0x06);
    PixelPlus_WriteI2C_Byte(0x8A, 0x2D);
    PixelPlus_WriteI2C_Byte(0x95, 0x06);
    PixelPlus_WriteI2C_Byte(0x96, 0x30);
    PixelPlus_WriteI2C_Byte(0x97, 0x06);
    PixelPlus_WriteI2C_Byte(0x98, 0x41);
    PixelPlus_WriteI2C_Byte(0x91, 0x06);
    PixelPlus_WriteI2C_Byte(0x92, 0x30);
    PixelPlus_WriteI2C_Byte(0x93, 0x06);
    PixelPlus_WriteI2C_Byte(0x94, 0x71);
    PixelPlus_WriteI2C_Byte(0x7F, 0x06);
    PixelPlus_WriteI2C_Byte(0x80, 0x30);
    PixelPlus_WriteI2C_Byte(0x81, 0x06);
    PixelPlus_WriteI2C_Byte(0x82, 0x67);
    PixelPlus_WriteI2C_Byte(0x83, 0x06);
    PixelPlus_WriteI2C_Byte(0x84, 0x30);
    PixelPlus_WriteI2C_Byte(0x85, 0x06);
    PixelPlus_WriteI2C_Byte(0x86, 0x67);
    PixelPlus_WriteI2C_Byte(0x83, 0x06);
    PixelPlus_WriteI2C_Byte(0x84, 0x30);
    PixelPlus_WriteI2C_Byte(0x85, 0x06);
    PixelPlus_WriteI2C_Byte(0x86, 0x67);
    PixelPlus_WriteI2C_Byte(0xA1, 0x04);
    PixelPlus_WriteI2C_Byte(0xA2, 0x6E);
    PixelPlus_WriteI2C_Byte(0x36, 0x01);
    PixelPlus_WriteI2C_Byte(0x37, 0x12);
    PixelPlus_WriteI2C_Byte(0x38, 0x06);
    PixelPlus_WriteI2C_Byte(0x39, 0x22);

    //Variable frame rate control
    PixelPlus_WriteI2C_Byte(0x03, 0x01);
    PixelPlus_WriteI2C_Byte(0x16, 0x05);

    //Auto exposure control (ag 8x, dg 1x)
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x12, 0x02);
    PixelPlus_WriteI2C_Byte(0x13, 0x70);
    PixelPlus_WriteI2C_Byte(0x14, 0x02);
    PixelPlus_WriteI2C_Byte(0x15, 0x70);
    PixelPlus_WriteI2C_Byte(0x16, 0x02);
    PixelPlus_WriteI2C_Byte(0x17, 0x70);
    PixelPlus_WriteI2C_Byte(0x1B, 0x00);
    PixelPlus_WriteI2C_Byte(0x1C, 0x17);
    PixelPlus_WriteI2C_Byte(0x1D, 0x40);
    PixelPlus_WriteI2C_Byte(0x1E, 0x00);
    PixelPlus_WriteI2C_Byte(0x1F, 0x17);
    PixelPlus_WriteI2C_Byte(0x20, 0x40);

    //Darkness Group Control
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x07, 0xA1);

    //Darkness X reference
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x82, 0x0C);
    PixelPlus_WriteI2C_Byte(0x83, 0x13);
    PixelPlus_WriteI2C_Byte(0x84, 0x15);

    ////////////////////////////////////LED SETTING//////////////////////////////////////////
    PixelPlus_WriteI2C_Byte(0x03, 0x00);//A-bank
    PixelPlus_WriteI2C_Byte(0x2A, 0x80);//Pad control4 LED pad
    PixelPlus_WriteI2C_Byte(0x2C, 0x80);//Pad control6 MIRS pad

    PixelPlus_WriteI2C_Byte(0x03, 0x04);//E-bank
    //PixelPlus_WriteI2C_Byte(0x07, 0xA0);//auto control4 - auto led control enable without cds mode
    PixelPlus_WriteI2C_Byte(0x07, 0xA1);//auto control4 - auto led control enable with cds mode

     ///////////////////////CDS START/////////////////////
    PixelPlus_WriteI2C_Byte(0x03, 0x00);//A-bank
    //PixelPlus_WriteI2C_Byte(0x8E, 0x98);//led_control1
    PixelPlus_WriteI2C_Byte(0x8E, 0x88);//led_control1, [4]=0, disable B&W when LED on

    PixelPlus_WriteI2C_Byte(0x8F, 0x09);//led_control2 Led control mode selection & exrom
      //01b : led control with CdS
      //05b : auto led control without CdS
      //09b : auto led control with CdS

    PixelPlus_WriteI2C_Byte(0x90, 0x50);//led_lvth1
    PixelPlus_WriteI2C_Byte(0x91, 0xB8);//led_lvth2
    PixelPlus_WriteI2C_Byte(0x92, 0x10);//led_frame
    PixelPlus_WriteI2C_Byte(0x93, 0xFF);//mirs_pw

    PixelPlus_WriteI2C_Byte(0x03, 0x01);//B-bank
    PixelPlus_WriteI2C_Byte(0x16, 0x04);//01 = 7:0 , 10 = 8:1 11 or 00 = 9:2
    PixelPlus_WriteI2C_Byte(0x17, 0xFA);//led invertig disable

    ////////////////////////////////// LED off th(without CDS)
    PixelPlus_WriteI2C_Byte(0x03, 0x00);
    PixelPlus_WriteI2C_Byte(0x99, 0x00);
    PixelPlus_WriteI2C_Byte(0x9A, 0x00);
    PixelPlus_WriteI2C_Byte(0x9B, 0x00);
    PixelPlus_WriteI2C_Byte(0x9C, 0x00);
    PixelPlus_WriteI2C_Byte(0x9D, 0x00);
    PixelPlus_WriteI2C_Byte(0x9E, 0x25);

    ////////////////////////////// LED////////////////////////////////
    PixelPlus_WriteI2C_Byte(0x03, 0x00);//A-bank
    PixelPlus_WriteI2C_Byte(0x7F, 0x00);//led_exposure_t   //LS -1X
    PixelPlus_WriteI2C_Byte(0x80, 0x04);//led_exposure_h
    PixelPlus_WriteI2C_Byte(0x81, 0x0E);//led_exposure_m
    PixelPlus_WriteI2C_Byte(0x82, 0x00);//led_exposure_l

    PixelPlus_WriteI2C_Byte(0x03, 0x00);//A-bank      //48x
    PixelPlus_WriteI2C_Byte(0x8A, 0x02);//led_on_th_t
    PixelPlus_WriteI2C_Byte(0x8B, 0x47);//led_on_th_h
    PixelPlus_WriteI2C_Byte(0x8C, 0xE0);//led_on_th_m

    PixelPlus_WriteI2C_Byte(0x03, 0x00);//A-Bank
    PixelPlus_WriteI2C_Byte(0x7C, 0x04);//max_ledlight_h
    PixelPlus_WriteI2C_Byte(0x7D, 0x77);//max_ledlight_m

    PixelPlus_WriteI2C_Byte(0x97, 0x04);//max_led_pp_h
    PixelPlus_WriteI2C_Byte(0x98, 0x77);//max_led_pp_i

    // Set AWB Sampling Boundary
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x51, 0x10);
    PixelPlus_WriteI2C_Byte(0x52, 0xE0);
    PixelPlus_WriteI2C_Byte(0x53, 0x02);
    PixelPlus_WriteI2C_Byte(0x54, 0x02);
    PixelPlus_WriteI2C_Byte(0x55, 0x40);
    PixelPlus_WriteI2C_Byte(0x56, 0xC0);
    PixelPlus_WriteI2C_Byte(0x57, 0x04);
    PixelPlus_WriteI2C_Byte(0x58, 0x6E);
    PixelPlus_WriteI2C_Byte(0x59, 0x45);

    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x5A, 0x23);
    PixelPlus_WriteI2C_Byte(0x5B, 0x4B);
    PixelPlus_WriteI2C_Byte(0x5C, 0x82);
    PixelPlus_WriteI2C_Byte(0x5D, 0xAA);
    PixelPlus_WriteI2C_Byte(0x5E, 0x23);
    PixelPlus_WriteI2C_Byte(0x5F, 0x28);
    PixelPlus_WriteI2C_Byte(0x60, 0x4B);
    PixelPlus_WriteI2C_Byte(0x61, 0x73);
    PixelPlus_WriteI2C_Byte(0x62, 0x3C);
    PixelPlus_WriteI2C_Byte(0x63, 0x87);
    PixelPlus_WriteI2C_Byte(0x64, 0x2D);
    PixelPlus_WriteI2C_Byte(0x65, 0x2D);

    //Auto exposure option
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x48, 0x08);
    PixelPlus_WriteI2C_Byte(0x49, 0x08);
    PixelPlus_WriteI2C_Byte(0x4A, 0x0A);

    //refgain4
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0xA1, 0x10);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////// 2013.12.20 Tune
    //gamma curve fitting y1 gamma 0.40 2%
    PixelPlus_WriteI2C_Byte(0x03, 0x02);
    PixelPlus_WriteI2C_Byte(0x3D, 0x00);
    PixelPlus_WriteI2C_Byte(0x3E, 0x06);
    PixelPlus_WriteI2C_Byte(0x3F, 0x18);
    PixelPlus_WriteI2C_Byte(0x40, 0x2C);
    PixelPlus_WriteI2C_Byte(0x41, 0x3C);
    PixelPlus_WriteI2C_Byte(0x42, 0x54);
    PixelPlus_WriteI2C_Byte(0x43, 0x65);
    PixelPlus_WriteI2C_Byte(0x44, 0x7D);
    PixelPlus_WriteI2C_Byte(0x45, 0x8F);
    PixelPlus_WriteI2C_Byte(0x46, 0xAB);
    PixelPlus_WriteI2C_Byte(0x47, 0xC1);
    PixelPlus_WriteI2C_Byte(0x48, 0xD3);
    PixelPlus_WriteI2C_Byte(0x49, 0xE3);
    PixelPlus_WriteI2C_Byte(0x4A, 0xF2);
    PixelPlus_WriteI2C_Byte(0x4B, 0xFF);

    //Y weight control
    PixelPlus_WriteI2C_Byte(0x03, 0x02);
    PixelPlus_WriteI2C_Byte(0x8D, 0x50);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////// New Contrast
    PixelPlus_WriteI2C_Byte(0x03, 0x02);
    PixelPlus_WriteI2C_Byte(0x0A, 0x95);

    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x0A, 0x20);

    //Darkness Y reference cont. slope 2
    PixelPlus_WriteI2C_Byte(0x03, 0x03);
    PixelPlus_WriteI2C_Byte(0x9B, 0x45);
    PixelPlus_WriteI2C_Byte(0x9C, 0x45);
    PixelPlus_WriteI2C_Byte(0x9D, 0x45);

    //Darkness Y reference cont. th2
    PixelPlus_WriteI2C_Byte(0x03, 0x03);
    PixelPlus_WriteI2C_Byte(0x97, 0x90);
    PixelPlus_WriteI2C_Byte(0x98, 0x90);
    PixelPlus_WriteI2C_Byte(0x99, 0x90);

    //Color correction
    PixelPlus_WriteI2C_Byte(0x03, 0x02);
    PixelPlus_WriteI2C_Byte(0x33, 0x37);
    PixelPlus_WriteI2C_Byte(0x34, 0x8A);
    PixelPlus_WriteI2C_Byte(0x35, 0x8D);
    PixelPlus_WriteI2C_Byte(0x36, 0x8B);
    PixelPlus_WriteI2C_Byte(0x37, 0x3E);
    PixelPlus_WriteI2C_Byte(0x38, 0x90);
    PixelPlus_WriteI2C_Byte(0x39, 0x84);
    PixelPlus_WriteI2C_Byte(0x3A, 0x98);
    PixelPlus_WriteI2C_Byte(0x3B, 0x3C);

    //Color saturation
    PixelPlus_WriteI2C_Byte(0x03, 0x03);
    PixelPlus_WriteI2C_Byte(0x08, 0x25);
    PixelPlus_WriteI2C_Byte(0x09, 0x86);
    PixelPlus_WriteI2C_Byte(0x0A, 0x00);
    PixelPlus_WriteI2C_Byte(0x0B, 0x25);
    PixelPlus_WriteI2C_Byte(0x0C, 0x25);
    PixelPlus_WriteI2C_Byte(0x0D, 0x88);
    PixelPlus_WriteI2C_Byte(0x0E, 0x00);
    PixelPlus_WriteI2C_Byte(0x0F, 0x25);

    //Color saturation weight
    PixelPlus_WriteI2C_Byte(0x03, 0x03);
    PixelPlus_WriteI2C_Byte(0x19, 0x2A);

    //////////////////////////////////////////////////////////// for Day Mode
    //Darkness Y reference
    PixelPlus_WriteI2C_Byte(0x03, 0x03);
    PixelPlus_WriteI2C_Byte(0x56, 0x00);
    PixelPlus_WriteI2C_Byte(0x57, 0x04);
    PixelPlus_WriteI2C_Byte(0x58, 0x08);

    //Y target contrl
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x05, 0x64);
    PixelPlus_WriteI2C_Byte(0x3B, 0x66);
    PixelPlus_WriteI2C_Byte(0x3C, 0x66);
    PixelPlus_WriteI2C_Byte(0x3D, 0x66);
    PixelPlus_WriteI2C_Byte(0x3E, 0x66);
    PixelPlus_WriteI2C_Byte(0x3F, 0x24);
    PixelPlus_WriteI2C_Byte(0x40, 0x4B);

    //Center window weight
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x34, 0x08);

    //PG_enable//////20140115
    //PixelPlus_WriteI2C_Byte(0x03, 0x05);
    //PixelPlus_WriteI2C_Byte(0x04, 0x00);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////// MV1100
    PixelPlus_WriteI2C_Byte(0x03, 0x00);
    PixelPlus_WriteI2C_Byte(0x40, 0x2C);// pll normal
    //PixelPlus_WriteI2C_Byte(0x40, 0x1C)// Bypass
    //PixelPlus_WriteI2C_Byte(0x25, 0x00)

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////// X2 Input
    PixelPlus_WriteI2C_Byte(0x03, 0x00);
    //PixelPlus_WriteI2C_Byte(0x27, 0x64);// osc_pad_drv up

    //PixelPlus_WriteI2C_Byte(0x27, 0x60);// X2 Input
    PixelPlus_WriteI2C_Byte(0x27, 0x66);// X1 Input

    //PixelPlus_WriteI2C_Byte(0x28, 0xC0);// MV1100
    //PixelPlus_WriteI2C_Byte(0x28, 0x50);// PP-DEB-008

    ////////////////////////////clk div
    PixelPlus_WriteI2C_Byte(0x03, 0x00);
    PixelPlus_WriteI2C_Byte(0x25, 0x08);
    PixelPlus_WriteI2C_Byte(0x26, 0x41);
    PixelPlus_WriteI2C_Byte(0x28, 0x03);

    //off parking guide
    PixelPlus_WriteI2C_Byte(0x03, 0x05);
    PixelPlus_WriteI2C_Byte(0x04, 0x08);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////// OSD Button
    //PixelPlus_WriteI2C_Byte(0x03, 0x05)
    //PixelPlus_WriteI2C_Byte(0xB2, 0x04)//sw_continue_cnt
    PixelPlus_WriteI2C_Byte(0x03, 0x00);
    PixelPlus_WriteI2C_Byte(0x29, 0x0D);	// output Hi-z release 20bit

    //[SMPTE30F]

    //[SMPTE60F]

    //[MIRROR00]

    //[MIRROR01]

    //[MIRROR10]

    //[MIRROR11]

    //[FLICKER00]

    //[FLICKER01]

    //[FLICKER10]

    //[FLICKER11]

    //[PLLON]

    //[PLLOFF]

    //[GENERAL000]

    //[GENERAL001]

    //[GENERAL010]

    //[GENERAL011]

    //[GENERAL100]

    //[GENERAL101]

    //[GENERAL110]

    //[GENERAL111]

    //[LEDON]
    //////////////////////////////////////////////////////////// for Night Mode
    //Darkness Y reference
    PixelPlus_WriteI2C_Byte(0x03, 0x03);
    PixelPlus_WriteI2C_Byte(0x56, 0xFF);
    PixelPlus_WriteI2C_Byte(0x57, 0xFF);
    PixelPlus_WriteI2C_Byte(0x58, 0xFF);

    //Y target contrl
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x05, 0x64);
    PixelPlus_WriteI2C_Byte(0x3B, 0x60);
    PixelPlus_WriteI2C_Byte(0x3C, 0x60);
    PixelPlus_WriteI2C_Byte(0x3D, 0x60);
    PixelPlus_WriteI2C_Byte(0x3E, 0x60);
    PixelPlus_WriteI2C_Byte(0x3F, 0x24);
    PixelPlus_WriteI2C_Byte(0x40, 0x4B);

    //Center window weight
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x34, 0x0A);

    //[LEDOFF]
    //////////////////////////////////////////////////////////// for Day Mode
    //Darkness Y reference
    PixelPlus_WriteI2C_Byte(0x03, 0x03);
    PixelPlus_WriteI2C_Byte(0x56, 0x00);
    PixelPlus_WriteI2C_Byte(0x57, 0x04);
    PixelPlus_WriteI2C_Byte(0x58, 0x08);

    //Y target contrl
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x05, 0x64);
    PixelPlus_WriteI2C_Byte(0x3B, 0x66);
    PixelPlus_WriteI2C_Byte(0x3C, 0x66);
    PixelPlus_WriteI2C_Byte(0x3D, 0x66);
    PixelPlus_WriteI2C_Byte(0x3E, 0x66);
    PixelPlus_WriteI2C_Byte(0x3F, 0x24);
    PixelPlus_WriteI2C_Byte(0x40, 0x4B);

    //Center window weight
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x34, 0x08);
}

static void
_PixelPlus_PO3100K_20bit_SMPTE_25fps(
    void)
{
    //////////////////////////////////////// start up////////////////////////////////////////
    PixelPlus_WriteI2C_Byte(0x03, 0x00);
    PixelPlus_WriteI2C_Byte(0x2D, 0x01);
    //PixelPlus_WriteI2C_Byte(0x29, 0x9D);	// output Hi-z release 20bit

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////
    PixelPlus_WriteI2C_Byte(0x03, 0x00);
    PixelPlus_WriteI2C_Byte(0x04, 0x00);	// chip mode (00 : smpte, 01 : sampling, 02 : DIGITAL)
    PixelPlus_WriteI2C_Byte(0x05, 0x00);	// mirror

    //PixelPlus_WriteI2C_Byte(0x41, 0x02);	// pll_ms
    //PixelPlus_WriteI2C_Byte(0x42, 0x0B);	// pll_ns

    //PixelPlus_WriteI2C_Byte(0x40, 0x3C);// pll_control
    //$0500////////////////////////////////////////////// need to delay about 500ms
    //PixelPlus_WriteI2C_Byte(0x40, 0x2C);// pll_control

    PixelPlus_WriteI2C_Byte(0x06, 0x07);// framewidth_h
    PixelPlus_WriteI2C_Byte(0x07, 0xBB);// framewidth_l
    PixelPlus_WriteI2C_Byte(0x08, 0x02);// fd_fheight_a_h
    PixelPlus_WriteI2C_Byte(0x09, 0xED);// fd_fheight_a_l
    PixelPlus_WriteI2C_Byte(0x0A, 0x02);// fd_fheight_b_h
    PixelPlus_WriteI2C_Byte(0x0B, 0xED);// fd_fheight_b_l

    PixelPlus_WriteI2C_Byte(0x0C, 0x00);// windowx1_h
    PixelPlus_WriteI2C_Byte(0x0D, 0x05);// windowx1_l
    PixelPlus_WriteI2C_Byte(0x0E, 0x00);// windowy1_h
    PixelPlus_WriteI2C_Byte(0x0F, 0x05);// windowy1_l
    PixelPlus_WriteI2C_Byte(0x10, 0x05);// windowx2_h
    PixelPlus_WriteI2C_Byte(0x11, 0x04);// windowx2_l
    PixelPlus_WriteI2C_Byte(0x12, 0x02);// windowy2_h
    PixelPlus_WriteI2C_Byte(0x13, 0xD4);// windowy2_l

    PixelPlus_WriteI2C_Byte(0x14, 0x00);// vsyncstartrow_f0_h
    PixelPlus_WriteI2C_Byte(0x15, 0x15);// vsyncstartrow_f0_l
    PixelPlus_WriteI2C_Byte(0x16, 0x02);// vsyncstoprow_f0_h
    PixelPlus_WriteI2C_Byte(0x17, 0xE9);// vsyncstoprow_f0_l

    PixelPlus_WriteI2C_Byte(0xDA, 0x01);
    PixelPlus_WriteI2C_Byte(0xDB, 0x4A);

    ////////////////////////// Start Settings////////////////////////////////

    PixelPlus_WriteI2C_Byte(0x03, 0x00);
    PixelPlus_WriteI2C_Byte(0x33, 0x05);// pixelbias (130703 khpark)
    PixelPlus_WriteI2C_Byte(0x34, 0x02);// compbias
    PixelPlus_WriteI2C_Byte(0x36, 0x80);// tx_bais, recommended by design 1

    PixelPlus_WriteI2C_Byte(0x03, 0x00);
    PixelPlus_WriteI2C_Byte(0x38, 0x58);// black_bias '011', rangesel "000", recommended by design 1

    PixelPlus_WriteI2C_Byte(0x03, 0x01);
    PixelPlus_WriteI2C_Byte(0x1E, 0x0E);// bsmode '0'
    PixelPlus_WriteI2C_Byte(0x26, 0x03);// blacksunth_h

    ////////////////////////// BLACK////////////////////////////////
    PixelPlus_WriteI2C_Byte(0x03, 0x01);
    PixelPlus_WriteI2C_Byte(0xB1, 0x30);// adcoffset

    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x06, 0x98);// front_black_fitting[4],
     	// ycont/ybright[3],
     	// adcoffset_fitting[2]

    PixelPlus_WriteI2C_Byte(0x03, 0x01);
    PixelPlus_WriteI2C_Byte(0xA4, 0x88);	//front_black_ref0
    PixelPlus_WriteI2C_Byte(0xA5, 0x88);	//front_black_ref1
    PixelPlus_WriteI2C_Byte(0xA6, 0x88);	//front_black_ref2
    PixelPlus_WriteI2C_Byte(0xA7, 0x00);	//front_black_ref3
    PixelPlus_WriteI2C_Byte(0xA8, 0x00);	//front_black_ref4
    PixelPlus_WriteI2C_Byte(0xA9, 0x08);	//front_black_ref5

    ////////////////////////// AWB////////////////////////////////

    //AWB gain control
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x06, 0xB8);

    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x75, 0x28);
    PixelPlus_WriteI2C_Byte(0x76, 0x28);
    PixelPlus_WriteI2C_Byte(0x77, 0x78);
    PixelPlus_WriteI2C_Byte(0x78, 0x78);
    PixelPlus_WriteI2C_Byte(0x79, 0x48);
    PixelPlus_WriteI2C_Byte(0x7A, 0x48);
    PixelPlus_WriteI2C_Byte(0x7B, 0xB8);
    PixelPlus_WriteI2C_Byte(0x7C, 0xB8);
    PixelPlus_WriteI2C_Byte(0x7D, 0x01);
    PixelPlus_WriteI2C_Byte(0x7E, 0x00);
    PixelPlus_WriteI2C_Byte(0x7F, 0x02);
    PixelPlus_WriteI2C_Byte(0x80, 0x07);

    //AWB option
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x73, 0x08);
    PixelPlus_WriteI2C_Byte(0x74, 0x04);

    // Set AWB Sampling Boundary
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x51, 0x10);
    PixelPlus_WriteI2C_Byte(0x52, 0xE0);
    PixelPlus_WriteI2C_Byte(0x53, 0x02);
    PixelPlus_WriteI2C_Byte(0x54, 0x02);
    PixelPlus_WriteI2C_Byte(0x55, 0x40);
    PixelPlus_WriteI2C_Byte(0x56, 0xC0);
    PixelPlus_WriteI2C_Byte(0x57, 0x04);
    PixelPlus_WriteI2C_Byte(0x58, 0x6E);
    PixelPlus_WriteI2C_Byte(0x59, 0x45);

    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x5A, 0x23);
    PixelPlus_WriteI2C_Byte(0x5B, 0x4B);
    PixelPlus_WriteI2C_Byte(0x5C, 0x64);
    PixelPlus_WriteI2C_Byte(0x5D, 0xAA);
    PixelPlus_WriteI2C_Byte(0x5E, 0x23);
    PixelPlus_WriteI2C_Byte(0x5F, 0x28);
    PixelPlus_WriteI2C_Byte(0x60, 0x4B);
    PixelPlus_WriteI2C_Byte(0x61, 0x73);
    PixelPlus_WriteI2C_Byte(0x62, 0x3C);
    PixelPlus_WriteI2C_Byte(0x63, 0x87);
    PixelPlus_WriteI2C_Byte(0x64, 0x2D);
    PixelPlus_WriteI2C_Byte(0x65, 0x2D);

    //awb rg/bg ratio axis
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x6E, 0x3A);
    PixelPlus_WriteI2C_Byte(0x6F, 0x50);
    PixelPlus_WriteI2C_Byte(0x70, 0x60);

    //lens / cs axis
    PixelPlus_WriteI2C_Byte(0x03, 0x03);
    PixelPlus_WriteI2C_Byte(0x16, 0x3A);
    PixelPlus_WriteI2C_Byte(0x17, 0x50);
    PixelPlus_WriteI2C_Byte(0x18, 0x60);

    ////////////////////////// AE////////////////////////////////
    //Y target contrl
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x05, 0x64);
    PixelPlus_WriteI2C_Byte(0x3B, 0x90);
    PixelPlus_WriteI2C_Byte(0x3C, 0x78);
    PixelPlus_WriteI2C_Byte(0x3D, 0x70);
    PixelPlus_WriteI2C_Byte(0x3E, 0x78);
    PixelPlus_WriteI2C_Byte(0x3F, 0x24);
    PixelPlus_WriteI2C_Byte(0x40, 0x4B);

    //Auto exposure option
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x48, 0x08);
    PixelPlus_WriteI2C_Byte(0x49, 0x08);
    PixelPlus_WriteI2C_Byte(0x4A, 0x08);

    //saturation level th
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x2C, 0xBB);//66

    //saturation ratio fitting
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x41, 0x04);
    PixelPlus_WriteI2C_Byte(0x42, 0x08);
    PixelPlus_WriteI2C_Byte(0x43, 0x10);
    PixelPlus_WriteI2C_Byte(0x44, 0x20);
    PixelPlus_WriteI2C_Byte(0x2E, 0x04);//05

    //Flicker canceling mode - manual 60hz
    PixelPlus_WriteI2C_Byte(0x03, 0x00);
    PixelPlus_WriteI2C_Byte(0x4F, 0x08);

    PixelPlus_WriteI2C_Byte(0x03, 0x00);
    PixelPlus_WriteI2C_Byte(0x59, 0x00);
    PixelPlus_WriteI2C_Byte(0x5A, 0xBA);
    PixelPlus_WriteI2C_Byte(0x5B, 0x00);

    ////////////////////////////// COLOR////////////////////////////////
    //Color correction
    PixelPlus_WriteI2C_Byte(0x03, 0x02);
    PixelPlus_WriteI2C_Byte(0x33, 0x07);
    PixelPlus_WriteI2C_Byte(0x34, 0x00);
    PixelPlus_WriteI2C_Byte(0x35, 0x07);
    PixelPlus_WriteI2C_Byte(0x36, 0x0E);
    PixelPlus_WriteI2C_Byte(0x37, 0x0B);
    PixelPlus_WriteI2C_Byte(0x38, 0x0C);
    PixelPlus_WriteI2C_Byte(0x39, 0x02);
    PixelPlus_WriteI2C_Byte(0x3A, 0x08);
    PixelPlus_WriteI2C_Byte(0x3B, 0x0A);

    //Color saturation
    PixelPlus_WriteI2C_Byte(0x03, 0x03);
    PixelPlus_WriteI2C_Byte(0x0C, 0x25);
    PixelPlus_WriteI2C_Byte(0x0D, 0x88);
    PixelPlus_WriteI2C_Byte(0x0E, 0x00);
    PixelPlus_WriteI2C_Byte(0x0F, 0x25);

    ////////////////////////////// GAMMA////////////////////////////////
    //gamma curve fitting (0.45)
    PixelPlus_WriteI2C_Byte(0x03, 0x02);
    PixelPlus_WriteI2C_Byte(0x3D, 0x00);
    PixelPlus_WriteI2C_Byte(0x3E, 0x27);
    PixelPlus_WriteI2C_Byte(0x3F, 0x36);
    PixelPlus_WriteI2C_Byte(0x40, 0x40);
    PixelPlus_WriteI2C_Byte(0x41, 0x49);
    PixelPlus_WriteI2C_Byte(0x42, 0x58);
    PixelPlus_WriteI2C_Byte(0x43, 0x64);
    PixelPlus_WriteI2C_Byte(0x44, 0x78);
    PixelPlus_WriteI2C_Byte(0x45, 0x89);
    PixelPlus_WriteI2C_Byte(0x46, 0xA4);
    PixelPlus_WriteI2C_Byte(0x47, 0xBB);
    PixelPlus_WriteI2C_Byte(0x48, 0xCF);
    PixelPlus_WriteI2C_Byte(0x49, 0xE0);
    PixelPlus_WriteI2C_Byte(0x4A, 0xF1);
    PixelPlus_WriteI2C_Byte(0x4B, 0xFF);

    //gamma curve fitting (0.75)
    PixelPlus_WriteI2C_Byte(0x03, 0x02);
    PixelPlus_WriteI2C_Byte(0x5B, 0x00);
    PixelPlus_WriteI2C_Byte(0x5C, 0x0B);
    PixelPlus_WriteI2C_Byte(0x5D, 0x13);
    PixelPlus_WriteI2C_Byte(0x5E, 0x1A);
    PixelPlus_WriteI2C_Byte(0x5F, 0x20);
    PixelPlus_WriteI2C_Byte(0x60, 0x2B);
    PixelPlus_WriteI2C_Byte(0x61, 0x36);
    PixelPlus_WriteI2C_Byte(0x62, 0x49);
    PixelPlus_WriteI2C_Byte(0x63, 0x5A);
    PixelPlus_WriteI2C_Byte(0x64, 0x7B);
    PixelPlus_WriteI2C_Byte(0x65, 0x98);
    PixelPlus_WriteI2C_Byte(0x66, 0xB4);
    PixelPlus_WriteI2C_Byte(0x67, 0xCE);
    PixelPlus_WriteI2C_Byte(0x68, 0xE7);
    PixelPlus_WriteI2C_Byte(0x69, 0xFF);

    //gamma curve fitting (0.75)
    PixelPlus_WriteI2C_Byte(0x03, 0x02);
    PixelPlus_WriteI2C_Byte(0x6A, 0x00);
    PixelPlus_WriteI2C_Byte(0x6B, 0x0B);
    PixelPlus_WriteI2C_Byte(0x6C, 0x13);
    PixelPlus_WriteI2C_Byte(0x6D, 0x1A);
    PixelPlus_WriteI2C_Byte(0x6E, 0x20);
    PixelPlus_WriteI2C_Byte(0x6F, 0x2B);
    PixelPlus_WriteI2C_Byte(0x70, 0x36);
    PixelPlus_WriteI2C_Byte(0x71, 0x49);
    PixelPlus_WriteI2C_Byte(0x72, 0x5A);
    PixelPlus_WriteI2C_Byte(0x73, 0x7B);
    PixelPlus_WriteI2C_Byte(0x74, 0x98);
    PixelPlus_WriteI2C_Byte(0x75, 0xB4);
    PixelPlus_WriteI2C_Byte(0x76, 0xCE);
    PixelPlus_WriteI2C_Byte(0x77, 0xE7);
    PixelPlus_WriteI2C_Byte(0x78, 0xFF);

    //Y weight control
    PixelPlus_WriteI2C_Byte(0x03, 0x02);
    PixelPlus_WriteI2C_Byte(0x8D, 0x30);

    ////////////////////////////// DARK////////////////////////////////
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x09, 0x00);// 01->00

    // dark_dpc_p
    PixelPlus_WriteI2C_Byte(0x03, 0x03);
    PixelPlus_WriteI2C_Byte(0x26, 0x00);
    PixelPlus_WriteI2C_Byte(0x27, 0x10);
    PixelPlus_WriteI2C_Byte(0x28, 0x20);

    // dark_blf
    PixelPlus_WriteI2C_Byte(0x03, 0x03);
    PixelPlus_WriteI2C_Byte(0x2E, 0x7F);
    PixelPlus_WriteI2C_Byte(0x2F, 0x7F);
    PixelPlus_WriteI2C_Byte(0x30, 0x7F);

    // dark_nf
    PixelPlus_WriteI2C_Byte(0x03, 0x03);
    PixelPlus_WriteI2C_Byte(0x32, 0x00);
    PixelPlus_WriteI2C_Byte(0x33, 0x00);
    PixelPlus_WriteI2C_Byte(0x34, 0x00);

    PixelPlus_WriteI2C_Byte(0x03, 0x03);
    PixelPlus_WriteI2C_Byte(0x77, 0x00);// dark_dc
    PixelPlus_WriteI2C_Byte(0x78, 0x00);
    PixelPlus_WriteI2C_Byte(0x79, 0x00);

    // dark_e_blf
    PixelPlus_WriteI2C_Byte(0x03, 0x03);
    PixelPlus_WriteI2C_Byte(0xA1, 0x30);
    PixelPlus_WriteI2C_Byte(0xA2, 0x7F);
    PixelPlus_WriteI2C_Byte(0xA3, 0x7F);

    //////////////////////////////////////// CDS710 delay////////////////////////////////////////
    PixelPlus_WriteI2C_Byte(0x03, 0x01);// bank B

    PixelPlus_WriteI2C_Byte(0x19, 0xC4);// default C0   bayer_control_04 '1' ramp pclk count '0' ramp 1/2 pclk count
    //PixelPlus_WriteI2C_Byte(0x1C, 0x22);// default 11   bayer_control_07 rampspeed, adcspeed

    PixelPlus_WriteI2C_Byte(0x5F, 0x02);
    PixelPlus_WriteI2C_Byte(0x60, 0xC8);
    PixelPlus_WriteI2C_Byte(0x61, 0x03);
    PixelPlus_WriteI2C_Byte(0x62, 0x66);
    PixelPlus_WriteI2C_Byte(0x63, 0x00);
    PixelPlus_WriteI2C_Byte(0x64, 0x00);
    PixelPlus_WriteI2C_Byte(0x65, 0x00);
    PixelPlus_WriteI2C_Byte(0x66, 0x00);
    PixelPlus_WriteI2C_Byte(0x67, 0x03);
    PixelPlus_WriteI2C_Byte(0x68, 0x79);
    PixelPlus_WriteI2C_Byte(0x69, 0x03);
    PixelPlus_WriteI2C_Byte(0x6A, 0x8C);
    PixelPlus_WriteI2C_Byte(0x5B, 0x00);
    PixelPlus_WriteI2C_Byte(0x5C, 0x02);
    PixelPlus_WriteI2C_Byte(0x5D, 0x06);
    PixelPlus_WriteI2C_Byte(0x5E, 0x1C);
    PixelPlus_WriteI2C_Byte(0x57, 0x06);
    PixelPlus_WriteI2C_Byte(0x58, 0x24);
    PixelPlus_WriteI2C_Byte(0x59, 0x06);
    PixelPlus_WriteI2C_Byte(0x5A, 0x53);
    PixelPlus_WriteI2C_Byte(0x53, 0x02);
    PixelPlus_WriteI2C_Byte(0x54, 0xD0);
    PixelPlus_WriteI2C_Byte(0x55, 0x06);
    PixelPlus_WriteI2C_Byte(0x56, 0x24);
    PixelPlus_WriteI2C_Byte(0x99, 0x03);
    PixelPlus_WriteI2C_Byte(0x9A, 0x79);
    PixelPlus_WriteI2C_Byte(0x9B, 0x06);
    PixelPlus_WriteI2C_Byte(0x9C, 0x24);
    PixelPlus_WriteI2C_Byte(0x6F, 0x03);
    PixelPlus_WriteI2C_Byte(0x70, 0x96);
    PixelPlus_WriteI2C_Byte(0x71, 0x06);
    PixelPlus_WriteI2C_Byte(0x72, 0x1A);
    PixelPlus_WriteI2C_Byte(0x73, 0x03);
    PixelPlus_WriteI2C_Byte(0x74, 0x9C);
    PixelPlus_WriteI2C_Byte(0x75, 0x06);
    PixelPlus_WriteI2C_Byte(0x76, 0x18);
    PixelPlus_WriteI2C_Byte(0x77, 0x06);
    PixelPlus_WriteI2C_Byte(0x78, 0x46);
    PixelPlus_WriteI2C_Byte(0x79, 0x06);
    PixelPlus_WriteI2C_Byte(0x7A, 0x50);
    PixelPlus_WriteI2C_Byte(0x7B, 0x03);
    PixelPlus_WriteI2C_Byte(0x7C, 0x9C);
    PixelPlus_WriteI2C_Byte(0x7D, 0x06);
    PixelPlus_WriteI2C_Byte(0x7E, 0x18);
    PixelPlus_WriteI2C_Byte(0x8F, 0x03);
    PixelPlus_WriteI2C_Byte(0x90, 0x73);
    PixelPlus_WriteI2C_Byte(0xAE, 0x20);
    PixelPlus_WriteI2C_Byte(0xAF, 0x20);
    PixelPlus_WriteI2C_Byte(0xB0, 0x20);
    PixelPlus_WriteI2C_Byte(0xB1, 0x20);
    PixelPlus_WriteI2C_Byte(0x8B, 0x03);
    PixelPlus_WriteI2C_Byte(0x8C, 0x9E);
    PixelPlus_WriteI2C_Byte(0x8D, 0x06);
    PixelPlus_WriteI2C_Byte(0x8E, 0x30);
    PixelPlus_WriteI2C_Byte(0x0C, 0x00);
    PixelPlus_WriteI2C_Byte(0x87, 0x06);
    PixelPlus_WriteI2C_Byte(0x88, 0x28);
    PixelPlus_WriteI2C_Byte(0x89, 0x06);
    PixelPlus_WriteI2C_Byte(0x8A, 0x2D);
    PixelPlus_WriteI2C_Byte(0x95, 0x06);
    PixelPlus_WriteI2C_Byte(0x96, 0x30);
    PixelPlus_WriteI2C_Byte(0x97, 0x06);
    PixelPlus_WriteI2C_Byte(0x98, 0x41);
    PixelPlus_WriteI2C_Byte(0x91, 0x06);
    PixelPlus_WriteI2C_Byte(0x92, 0x30);
    PixelPlus_WriteI2C_Byte(0x93, 0x06);
    PixelPlus_WriteI2C_Byte(0x94, 0x71);
    PixelPlus_WriteI2C_Byte(0x7F, 0x06);
    PixelPlus_WriteI2C_Byte(0x80, 0x30);
    PixelPlus_WriteI2C_Byte(0x81, 0x06);
    PixelPlus_WriteI2C_Byte(0x82, 0x67);
    PixelPlus_WriteI2C_Byte(0x83, 0x06);
    PixelPlus_WriteI2C_Byte(0x84, 0x30);
    PixelPlus_WriteI2C_Byte(0x85, 0x06);
    PixelPlus_WriteI2C_Byte(0x86, 0x67);
    PixelPlus_WriteI2C_Byte(0x83, 0x06);
    PixelPlus_WriteI2C_Byte(0x84, 0x30);
    PixelPlus_WriteI2C_Byte(0x85, 0x06);
    PixelPlus_WriteI2C_Byte(0x86, 0x67);
    PixelPlus_WriteI2C_Byte(0xA1, 0x04);
    PixelPlus_WriteI2C_Byte(0xA2, 0x6E);
    PixelPlus_WriteI2C_Byte(0x36, 0x01);
    PixelPlus_WriteI2C_Byte(0x37, 0x12);
    PixelPlus_WriteI2C_Byte(0x38, 0x06);
    PixelPlus_WriteI2C_Byte(0x39, 0x22);

    //Variable frame rate control
    PixelPlus_WriteI2C_Byte(0x03, 0x01);
    PixelPlus_WriteI2C_Byte(0x16, 0x05);

    //Auto exposure control (ag 8x, dg 1x)
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x12, 0x02);
    PixelPlus_WriteI2C_Byte(0x13, 0x70);
    PixelPlus_WriteI2C_Byte(0x14, 0x02);
    PixelPlus_WriteI2C_Byte(0x15, 0x70);
    PixelPlus_WriteI2C_Byte(0x16, 0x02);
    PixelPlus_WriteI2C_Byte(0x17, 0x70);
    PixelPlus_WriteI2C_Byte(0x1B, 0x00);
    PixelPlus_WriteI2C_Byte(0x1C, 0x17);
    PixelPlus_WriteI2C_Byte(0x1D, 0x40);
    PixelPlus_WriteI2C_Byte(0x1E, 0x00);
    PixelPlus_WriteI2C_Byte(0x1F, 0x17);
    PixelPlus_WriteI2C_Byte(0x20, 0x40);

    //Darkness Group Control
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x07, 0xA1);

    //Darkness X reference
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x82, 0x0C);
    PixelPlus_WriteI2C_Byte(0x83, 0x13);
    PixelPlus_WriteI2C_Byte(0x84, 0x15);

    ////////////////////////////////////LED SETTING//////////////////////////////////////////
    PixelPlus_WriteI2C_Byte(0x03, 0x00);//A-bank
    PixelPlus_WriteI2C_Byte(0x2A, 0x80);//Pad control4 LED pad
    PixelPlus_WriteI2C_Byte(0x2C, 0x80);//Pad control6 MIRS pad

    PixelPlus_WriteI2C_Byte(0x03, 0x04);//E-bank
    //PixelPlus_WriteI2C_Byte(0x07, 0xA0);//auto control4 - auto led control enable without cds mode
    PixelPlus_WriteI2C_Byte(0x07, 0xA1);//auto control4 - auto led control enable with cds mode

     ///////////////////////CDS START/////////////////////
    PixelPlus_WriteI2C_Byte(0x03, 0x00);//A-bank
    //PixelPlus_WriteI2C_Byte(0x8E, 0x98);//led_control1
    PixelPlus_WriteI2C_Byte(0x8E, 0x88);//led_control1, [4]=0, disable B&W when LED on

    PixelPlus_WriteI2C_Byte(0x8F, 0x09);//led_control2 Led control mode selection & exrom
      //01b : led control with CdS
      //05b : auto led control without CdS
      //09b : auto led control with CdS

    PixelPlus_WriteI2C_Byte(0x90, 0x50);//led_lvth1
    PixelPlus_WriteI2C_Byte(0x91, 0xB8);//led_lvth2
    PixelPlus_WriteI2C_Byte(0x92, 0x10);//led_frame
    PixelPlus_WriteI2C_Byte(0x93, 0xFF);//mirs_pw

    PixelPlus_WriteI2C_Byte(0x03, 0x01);//B-bank
    PixelPlus_WriteI2C_Byte(0x16, 0x04);//01 = 7:0 , 10 = 8:1 11 or 00 = 9:2
    PixelPlus_WriteI2C_Byte(0x17, 0xFA);//led invertig disable

    ////////////////////////////////// LED off th(without CDS)
    PixelPlus_WriteI2C_Byte(0x03, 0x00);
    PixelPlus_WriteI2C_Byte(0x99, 0x00);
    PixelPlus_WriteI2C_Byte(0x9A, 0x00);
    PixelPlus_WriteI2C_Byte(0x9B, 0x00);
    PixelPlus_WriteI2C_Byte(0x9C, 0x00);
    PixelPlus_WriteI2C_Byte(0x9D, 0x00);
    PixelPlus_WriteI2C_Byte(0x9E, 0x25);

    ////////////////////////////// LED////////////////////////////////
    PixelPlus_WriteI2C_Byte(0x03, 0x00);//A-bank
    PixelPlus_WriteI2C_Byte(0x7F, 0x00);//led_exposure_t   //LS -1X
    PixelPlus_WriteI2C_Byte(0x80, 0x04);//led_exposure_h
    PixelPlus_WriteI2C_Byte(0x81, 0x0E);//led_exposure_m
    PixelPlus_WriteI2C_Byte(0x82, 0x00);//led_exposure_l

    PixelPlus_WriteI2C_Byte(0x03, 0x00);//A-bank      //48x
    PixelPlus_WriteI2C_Byte(0x8A, 0x02);//led_on_th_t
    PixelPlus_WriteI2C_Byte(0x8B, 0x47);//led_on_th_h
    PixelPlus_WriteI2C_Byte(0x8C, 0xE0);//led_on_th_m

    PixelPlus_WriteI2C_Byte(0x03, 0x00);//A-Bank
    PixelPlus_WriteI2C_Byte(0x7C, 0x04);//max_ledlight_h
    PixelPlus_WriteI2C_Byte(0x7D, 0x77);//max_ledlight_m

    PixelPlus_WriteI2C_Byte(0x97, 0x04);//max_led_pp_h
    PixelPlus_WriteI2C_Byte(0x98, 0x77);//max_led_pp_i

    // Set AWB Sampling Boundary
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x51, 0x10);
    PixelPlus_WriteI2C_Byte(0x52, 0xE0);
    PixelPlus_WriteI2C_Byte(0x53, 0x02);
    PixelPlus_WriteI2C_Byte(0x54, 0x02);
    PixelPlus_WriteI2C_Byte(0x55, 0x40);
    PixelPlus_WriteI2C_Byte(0x56, 0xC0);
    PixelPlus_WriteI2C_Byte(0x57, 0x04);
    PixelPlus_WriteI2C_Byte(0x58, 0x6E);
    PixelPlus_WriteI2C_Byte(0x59, 0x45);

    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x5A, 0x23);
    PixelPlus_WriteI2C_Byte(0x5B, 0x4B);
    PixelPlus_WriteI2C_Byte(0x5C, 0x82);
    PixelPlus_WriteI2C_Byte(0x5D, 0xAA);
    PixelPlus_WriteI2C_Byte(0x5E, 0x23);
    PixelPlus_WriteI2C_Byte(0x5F, 0x28);
    PixelPlus_WriteI2C_Byte(0x60, 0x4B);
    PixelPlus_WriteI2C_Byte(0x61, 0x73);
    PixelPlus_WriteI2C_Byte(0x62, 0x3C);
    PixelPlus_WriteI2C_Byte(0x63, 0x87);
    PixelPlus_WriteI2C_Byte(0x64, 0x2D);
    PixelPlus_WriteI2C_Byte(0x65, 0x2D);

    //Auto exposure option
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x48, 0x08);
    PixelPlus_WriteI2C_Byte(0x49, 0x08);
    PixelPlus_WriteI2C_Byte(0x4A, 0x0A);

    //refgain4
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0xA1, 0x10);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////// 2013.12.20 Tune
    //gamma curve fitting y1 gamma 0.40 2%
    PixelPlus_WriteI2C_Byte(0x03, 0x02);
    PixelPlus_WriteI2C_Byte(0x3D, 0x00);
    PixelPlus_WriteI2C_Byte(0x3E, 0x06);
    PixelPlus_WriteI2C_Byte(0x3F, 0x18);
    PixelPlus_WriteI2C_Byte(0x40, 0x2C);
    PixelPlus_WriteI2C_Byte(0x41, 0x3C);
    PixelPlus_WriteI2C_Byte(0x42, 0x54);
    PixelPlus_WriteI2C_Byte(0x43, 0x65);
    PixelPlus_WriteI2C_Byte(0x44, 0x7D);
    PixelPlus_WriteI2C_Byte(0x45, 0x8F);
    PixelPlus_WriteI2C_Byte(0x46, 0xAB);
    PixelPlus_WriteI2C_Byte(0x47, 0xC1);
    PixelPlus_WriteI2C_Byte(0x48, 0xD3);
    PixelPlus_WriteI2C_Byte(0x49, 0xE3);
    PixelPlus_WriteI2C_Byte(0x4A, 0xF2);
    PixelPlus_WriteI2C_Byte(0x4B, 0xFF);

    //Y weight control
    PixelPlus_WriteI2C_Byte(0x03, 0x02);
    PixelPlus_WriteI2C_Byte(0x8D, 0x50);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////// New Contrast
    PixelPlus_WriteI2C_Byte(0x03, 0x02);
    PixelPlus_WriteI2C_Byte(0x0A, 0x95);

    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x0A, 0x20);

    //Darkness Y reference cont. slope 2
    PixelPlus_WriteI2C_Byte(0x03, 0x03);
    PixelPlus_WriteI2C_Byte(0x9B, 0x45);
    PixelPlus_WriteI2C_Byte(0x9C, 0x45);
    PixelPlus_WriteI2C_Byte(0x9D, 0x45);

    //Darkness Y reference cont. th2
    PixelPlus_WriteI2C_Byte(0x03, 0x03);
    PixelPlus_WriteI2C_Byte(0x97, 0x90);
    PixelPlus_WriteI2C_Byte(0x98, 0x90);
    PixelPlus_WriteI2C_Byte(0x99, 0x90);

    //Color correction
    PixelPlus_WriteI2C_Byte(0x03, 0x02);
    PixelPlus_WriteI2C_Byte(0x33, 0x37);
    PixelPlus_WriteI2C_Byte(0x34, 0x8A);
    PixelPlus_WriteI2C_Byte(0x35, 0x8D);
    PixelPlus_WriteI2C_Byte(0x36, 0x8B);
    PixelPlus_WriteI2C_Byte(0x37, 0x3E);
    PixelPlus_WriteI2C_Byte(0x38, 0x90);
    PixelPlus_WriteI2C_Byte(0x39, 0x84);
    PixelPlus_WriteI2C_Byte(0x3A, 0x98);
    PixelPlus_WriteI2C_Byte(0x3B, 0x3C);

    //Color saturation
    PixelPlus_WriteI2C_Byte(0x03, 0x03);
    PixelPlus_WriteI2C_Byte(0x08, 0x25);
    PixelPlus_WriteI2C_Byte(0x09, 0x86);
    PixelPlus_WriteI2C_Byte(0x0A, 0x00);
    PixelPlus_WriteI2C_Byte(0x0B, 0x25);
    PixelPlus_WriteI2C_Byte(0x0C, 0x25);
    PixelPlus_WriteI2C_Byte(0x0D, 0x88);
    PixelPlus_WriteI2C_Byte(0x0E, 0x00);
    PixelPlus_WriteI2C_Byte(0x0F, 0x25);

    //Color saturation weight
    PixelPlus_WriteI2C_Byte(0x03, 0x03);
    PixelPlus_WriteI2C_Byte(0x19, 0x2A);

    //////////////////////////////////////////////////////////// for Day Mode
    //Darkness Y reference
    PixelPlus_WriteI2C_Byte(0x03, 0x03);
    PixelPlus_WriteI2C_Byte(0x56, 0x00);
    PixelPlus_WriteI2C_Byte(0x57, 0x04);
    PixelPlus_WriteI2C_Byte(0x58, 0x08);

    //Y target contrl
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x05, 0x64);
    PixelPlus_WriteI2C_Byte(0x3B, 0x66);
    PixelPlus_WriteI2C_Byte(0x3C, 0x66);
    PixelPlus_WriteI2C_Byte(0x3D, 0x66);
    PixelPlus_WriteI2C_Byte(0x3E, 0x66);
    PixelPlus_WriteI2C_Byte(0x3F, 0x24);
    PixelPlus_WriteI2C_Byte(0x40, 0x4B);

    //Center window weight
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x34, 0x08);

    //PG_enable//////20140115
    //PixelPlus_WriteI2C_Byte(0x03, 0x05);
    //PixelPlus_WriteI2C_Byte(0x04, 0x00);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////// MV1100
    PixelPlus_WriteI2C_Byte(0x03, 0x00);
    PixelPlus_WriteI2C_Byte(0x40, 0x2C);// pll normal
    //PixelPlus_WriteI2C_Byte(0x40, 0x1C)// Bypass
    //PixelPlus_WriteI2C_Byte(0x25, 0x00)

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////// X2 Input
    PixelPlus_WriteI2C_Byte(0x03, 0x00);
    //PixelPlus_WriteI2C_Byte(0x27, 0x64);// osc_pad_drv up

    //PixelPlus_WriteI2C_Byte(0x27, 0x60);// X2 Input
    PixelPlus_WriteI2C_Byte(0x27, 0x66);// X1 Input

    //PixelPlus_WriteI2C_Byte(0x28, 0xC0);// MV1100
    //PixelPlus_WriteI2C_Byte(0x28, 0x50);// PP-DEB-008

    ////////////////////////////clk div
    PixelPlus_WriteI2C_Byte(0x03, 0x00);
    PixelPlus_WriteI2C_Byte(0x25, 0x08);
    PixelPlus_WriteI2C_Byte(0x26, 0x41);
    PixelPlus_WriteI2C_Byte(0x28, 0x03);

    //off parking guide
    PixelPlus_WriteI2C_Byte(0x03, 0x05);
    PixelPlus_WriteI2C_Byte(0x04, 0x08);

    ////////////////////////////////////////////////////////////////////////////////////////////////////////////// OSD Button
    //PixelPlus_WriteI2C_Byte(0x03, 0x05)
    //PixelPlus_WriteI2C_Byte(0xB2, 0x04)//sw_continue_cnt
    PixelPlus_WriteI2C_Byte(0x03, 0x00);
    PixelPlus_WriteI2C_Byte(0x29, 0x0D);	// output Hi-z release 20bit

    //[SMPTE30F]

    //[SMPTE60F]

    //[MIRROR00]

    //[MIRROR01]

    //[MIRROR10]

    //[MIRROR11]

    //[FLICKER00]

    //[FLICKER01]

    //[FLICKER10]

    //[FLICKER11]

    //[PLLON]

    //[PLLOFF]

    //[GENERAL000]

    //[GENERAL001]

    //[GENERAL010]

    //[GENERAL011]

    //[GENERAL100]

    //[GENERAL101]

    //[GENERAL110]

    //[GENERAL111]

    //[LEDON]
    //////////////////////////////////////////////////////////// for Night Mode
    //Darkness Y reference
    PixelPlus_WriteI2C_Byte(0x03, 0x03);
    PixelPlus_WriteI2C_Byte(0x56, 0xFF);
    PixelPlus_WriteI2C_Byte(0x57, 0xFF);
    PixelPlus_WriteI2C_Byte(0x58, 0xFF);

    //Y target contrl
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x05, 0x64);
    PixelPlus_WriteI2C_Byte(0x3B, 0x60);
    PixelPlus_WriteI2C_Byte(0x3C, 0x60);
    PixelPlus_WriteI2C_Byte(0x3D, 0x60);
    PixelPlus_WriteI2C_Byte(0x3E, 0x60);
    PixelPlus_WriteI2C_Byte(0x3F, 0x24);
    PixelPlus_WriteI2C_Byte(0x40, 0x4B);

    //Center window weight
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x34, 0x0A);

    //[LEDOFF]
    //////////////////////////////////////////////////////////// for Day Mode
    //Darkness Y reference
    PixelPlus_WriteI2C_Byte(0x03, 0x03);
    PixelPlus_WriteI2C_Byte(0x56, 0x00);
    PixelPlus_WriteI2C_Byte(0x57, 0x04);
    PixelPlus_WriteI2C_Byte(0x58, 0x08);

    //Y target contrl
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x05, 0x64);
    PixelPlus_WriteI2C_Byte(0x3B, 0x66);
    PixelPlus_WriteI2C_Byte(0x3C, 0x66);
    PixelPlus_WriteI2C_Byte(0x3D, 0x66);
    PixelPlus_WriteI2C_Byte(0x3E, 0x66);
    PixelPlus_WriteI2C_Byte(0x3F, 0x24);
    PixelPlus_WriteI2C_Byte(0x40, 0x4B);

    //Center window weight
    PixelPlus_WriteI2C_Byte(0x03, 0x04);
    PixelPlus_WriteI2C_Byte(0x34, 0x08);
}

static void
_PixelPlus_Internal_Rom_Setting(
    void)
{
    PixelPlus_WriteI2C_Byte(0x03, 0x00); //GroupA
    //PixelPlus_WriteI2C_Byte(0x04, 0x00); //0x02
    //PixelPlus_WriteI2C_Byte(0x29, 0x9D); //0x00
    //PixelPlus_WriteI2C_Byte(0x2D, 0x01); //0x00
    //PixelPlus_WriteI2C_Byte(0x40, 0x2C); //0x1C
    //PixelPlus_WriteI2C_Byte(0x50, 0x01); //0x00

    //don't remove for disable green line
    PixelPlus_WriteI2C_Byte(0xAC, 0x01); //0x00

//    PixelPlus_WriteI2C_Byte(0x03, 0x01); //Group B
//    PixelPlus_WriteI2C_Byte(0xC0, 0x01); //0x00
//    PixelPlus_WriteI2C_Byte(0xC1, 0x40); //0x39
//    PixelPlus_WriteI2C_Byte(0xC2, 0x00); //0x99
//
//    PixelPlus_WriteI2C_Byte(0x03, 0x03); //Group D
//    PixelPlus_WriteI2C_Byte(0x20, 0x65); //0x67
//    PixelPlus_WriteI2C_Byte(0x83, 0x5D); //0x5C
//    PixelPlus_WriteI2C_Byte(0x84, 0x07); //0x04
//    PixelPlus_WriteI2C_Byte(0x8F, 0x02); //0x00
//    PixelPlus_WriteI2C_Byte(0x90, 0x1E); //0x05
//    PixelPlus_WriteI2C_Byte(0x92, 0xE2); //0xD0
//    PixelPlus_WriteI2C_Byte(0x93, 0xA0); //0x09
//    PixelPlus_WriteI2C_Byte(0x94, 0x99); //0x9F
//
//    PixelPlus_WriteI2C_Byte(0x03, 0x04); //Group E
//    PixelPlus_WriteI2C_Byte(0x28, 0x01); //0x00
//    PixelPlus_WriteI2C_Byte(0x29, 0x40); //0x39
//    PixelPlus_WriteI2C_Byte(0x2A, 0x00); //0x99
//    PixelPlus_WriteI2C_Byte(0x30, 0x02); //0x08
//    PixelPlus_WriteI2C_Byte(0x31, 0x02); //0x08
//    PixelPlus_WriteI2C_Byte(0x32, 0x02); //0x08
//    PixelPlus_WriteI2C_Byte(0x33, 0x02); //0x08
//    PixelPlus_WriteI2C_Byte(0x34, 0x01); //0x08
//    PixelPlus_WriteI2C_Byte(0x3A, 0xA0); //0x97
//    PixelPlus_WriteI2C_Byte(0x3C, 0xA0); //0x78
//    PixelPlus_WriteI2C_Byte(0x3D, 0xA0); //0x80
//    PixelPlus_WriteI2C_Byte(0x3E, 0xA0); //0x78
}

static void
_PixelPlus_MIRS_Output_Polarity(
    bool flag)
{
    uint8_t value;

    PixelPlus_WriteI2C_Byte(0x03, 0x00);//A-bank
    value = PixelPlus_ReadI2C_Byte(0x8E); //Led control

    if (flag)
        value = value | 0x1;
    else
        value = value & 0xFE;

    PixelPlus_WriteI2C_Byte(0x8E, value);
}


//=============================================================================
//                Public Function Definition
//=============================================================================
void
mmpPixelPlusPowerOn(
    void)
{
    //GPIO30 PWDN
    //Set GPIO30 to Low
    AHB_WriteRegisterMask(GPIO_BASE + 0x10, (1 << 30), (1 << 30));
    //Set GPIO30 Output Mode
    AHB_WriteRegisterMask(GPIO_BASE + 0x8, (1 << 30), (1 << 30));
    //Set GPIO30 Mode0
    AHB_WriteRegisterMask(GPIO_BASE + 0x94, (0x0 << (14 * 2)), (0x3 << (14 * 2)));
}

void
mmpPixelPlusPowerOff(
	void)
{
	  AHB_WriteRegisterMask(GPIO_BASE + 0xC, (1 << 30), (1 << 30));
}

void
mmpPixelPlusLedOn(
    void)
{
    //GPIO31 LED_EN
    //Set GPIO31 to High
    AHB_WriteRegisterMask(GPIO_BASE + 0xC, (1 << 31), (1 << 31));
    //Set GPIO31 Output Mode
    AHB_WriteRegisterMask(GPIO_BASE + 0x8, (1 << 31), (1 << 31));
    //Set GPIO31 Mode0
    AHB_WriteRegisterMask(GPIO_BASE + 0x94, (0x0 << (15 * 2)), (0x3 << (15 * 2)));
}

void
mmpPixelPlusLedOff(
    void)
{
	  AHB_WriteRegisterMask(GPIO_BASE + 0x10, (1 << 31), (1 << 31));
}

void
mmpSensorLoadConfig(
    MMP_UINT32 configLen,
    uint8_t *configData)
{
    uint8_t   addr;
    uint8_t   data;
    uint32_t  i;
    uint8_t   *pConfigData = configData;

    for (i = 0; i < configLen; i++)
    {
        addr = *pConfigData;
        pConfigData++;
        data = *pConfigData;
        pConfigData++;

        PixelPlus_WriteI2C_Byte(addr, data);
    }
}

void
mmpPixelPlusInitialize(
    void)
{
    //_PixelPlus_ISP_Reset();
    //_PixelPlus_Internal_Rom_Setting();
    //_PixelPlus_ISP_Initialize_IRCUT();
    //_PixelPlus_PO3100K_20bit_SMPTE_30fps();
    //_PixelPlus_MIRS_Output_Polarity(MMP_FALSE);
    //mmpPixelPlusSetMirror(1, 1);
    //mmpPixelPlusSetAntiFlickerAuto();
    
    gbPixelPlusInit = MMP_FALSE;
}

void
mmpPixelPlusSetEffectDefault(
    void)
{
    mmpPixelPlusSetBrightness(0x0);
    mmpPixelPlusSetContrast(0x45);
    mmpPixelPlusSetSaturation(0x2a);
    mmpPixelPlusSetSharpness(0x14);
}

void
mmpPixelPlusGetEffectDefault(
    uint8_t *brightness,
    uint8_t *contrast,
    uint8_t *saturation,
    uint8_t *sharpness)
{
#if defined (PIXELPLUS_PO3100)
    *brightness = 0x0 + 0x80;
    *contrast = 0x45;
    *saturation = 0x2a;
    *sharpness = 0x14;
#endif
}

void
mmpPixelPlusGetContrast(
    uint8_t *value)
{
    PixelPlus_WriteI2C_Byte(0x03, 0x03);
    *value = PixelPlus_ReadI2C_Byte(0x9b);
};

void
mmpPixelPlusGetBrightness(
    uint8_t *value)
{
    uint8_t data;
    PixelPlus_WriteI2C_Byte(0x03, 0x02);
    data = PixelPlus_ReadI2C_Byte(0x95);
    *value  = data + 0x80;
};

void
mmpPixelPlusGetSaturation(
    uint8_t *value)
{
    PixelPlus_WriteI2C_Byte(0x03, 0x03);
    *value = PixelPlus_ReadI2C_Byte(0x19);
};

void
mmpPixelPlusGetSharpness(
    uint8_t *value)
{
    PixelPlus_WriteI2C_Byte(0x03, 0x02);
    *value = PixelPlus_ReadI2C_Byte(0x2F);
};

void
mmpPixelPlusSetContrast(
    uint8_t value)
{
    PixelPlus_WriteI2C_Byte(0x03, 0x03);
    PixelPlus_WriteI2C_Byte(0x9b, value);
    PixelPlus_WriteI2C_Byte(0x9c, value);
    PixelPlus_WriteI2C_Byte(0x9d, value);
    PixelPlus_WriteI2C_Byte(0x9e, value);
};

void
mmpPixelPlusSetBrightness(
    uint8_t value)
{
    int data = value - 0x80;
    uint8_t regVaule = data & 0xFF;
    PixelPlus_WriteI2C_Byte(0x03, 0x02);
    PixelPlus_WriteI2C_Byte(0x95, regVaule);
    PixelPlus_WriteI2C_Byte(0x96, regVaule);
    PixelPlus_WriteI2C_Byte(0x97, regVaule);
    PixelPlus_WriteI2C_Byte(0x98, regVaule);
};

void
mmpPixelPlusSetSaturation(
    uint8_t value)
{
    PixelPlus_WriteI2C_Byte(0x03, 0x03);
    PixelPlus_WriteI2C_Byte(0x19, value);
};

void
mmpPixelPlusSetSharpness(
    uint8_t value)
{
    PixelPlus_WriteI2C_Byte(0x03, 0x02);
    PixelPlus_WriteI2C_Byte(0x2F, value);
};

void
mmpPixelPlusMDEnable(
    uint8_t mode)
{
    PixelPlus_WriteI2C_Byte(0x03, 0x02);
    PixelPlus_WriteI2C_Byte(0x06, 0x02 | (mode & 0x01));
};

void
mmpPixelPlusMDDisable(
    void)
{
    uint8_t data;
    PixelPlus_WriteI2C_Byte(0x03, 0x02);
    data = PixelPlus_ReadI2C_Byte(0x06);
    data = data & 0xFC;
    PixelPlus_WriteI2C_Byte(0x06, data);
};

void
mmpPixelPlusSetSection(
    uint8_t section,
    uint8_t bONflag)
{
    uint8_t address;
    uint8_t data;
    PixelPlus_WriteI2C_Byte(0x03, 0x02);
    address = 0xF6 + (section / 8);
    data = PixelPlus_ReadI2C_Byte(address);

    if (bONflag & 0x1)
        data = data | (1<<(section % 8));
    else
        data = data & (~(1<<(section % 8)));

    PixelPlus_WriteI2C_Byte(address, data);
};

void
mmpPixelPlusSetSensitivity(
    uint8_t value)
{
    PixelPlus_WriteI2C_Byte(0x03, 0x02);
    PixelPlus_WriteI2C_Byte(0xE7, value);
    PixelPlus_WriteI2C_Byte(0xE8, value);
    PixelPlus_WriteI2C_Byte(0xE9, value);
    PixelPlus_WriteI2C_Byte(0xEA, value);
};

void
mmpPixelPlusSetMirror(
    uint8_t bEnHorMirror,
    uint8_t bEnVerMirror)
{
    uint8_t data;
    PixelPlus_WriteI2C_Byte(0x03, 0x00);
    data = PixelPlus_ReadI2C_Byte(0x05);

    if (bEnHorMirror)
        data = data | 0x1;
    else
        data = data & 0xFE;

     if (bEnVerMirror)
        data = data | 0x2;
    else
        data = data & 0xFD;
    PixelPlus_WriteI2C_Byte(0x05, data);
};

void
mmpPixelPlusGetFlickerMode(
    uint8_t *value)
{
    // off  = 00
    // auto = 40
    // 60Hz = 08
    // 50Hz = 04
    //PixelPlus_WriteI2C_Byte(0x03, 0x00);
    //*value = PixelPlus_ReadI2C_Byte(0x4F);
    
    *value = gFlickerMode;
};

void
mmpPixelPlusSetAntiFlicker60Hz(
    )
{
    //PixelPlus_WriteI2C_Byte(0x03,0x00);
    //PixelPlus_WriteI2C_Byte(0x4F,0x08);
    //PixelPlus_WriteI2C_Byte(0x59,0x00);
    //PixelPlus_WriteI2C_Byte(0x5A,0xBB);
    //PixelPlus_WriteI2C_Byte(0x5B,0x80);
    
    if (!gbPixelPlusInit)
    {
        _PixelPlus_Internal_Rom_Setting();
        _PixelPlus_PO3100K_20bit_SMPTE_30fps();
        gbPixelPlusInit = MMP_TRUE;
    }
    
    PixelPlus_WriteI2C_Byte(0x03,0x00);
    PixelPlus_WriteI2C_Byte(0x4F,0x08);
    PixelPlus_WriteI2C_Byte(0x59,0x00);
    PixelPlus_WriteI2C_Byte(0x5A,0xBA);
    PixelPlus_WriteI2C_Byte(0x5B,0x00);
    
    gFlickerMode = AntiFlicker_60Hz_MODE;
};

void
mmpPixelPlusSetAntiFlicker50Hz(
    )
{
    //PixelPlus_WriteI2C_Byte(0x03,0x00);
    //PixelPlus_WriteI2C_Byte(0x4F,0x04);
    //PixelPlus_WriteI2C_Byte(0x5C,0x00);
    //PixelPlus_WriteI2C_Byte(0x5D,0xE1);
    //PixelPlus_WriteI2C_Byte(0x5E,0x00);
    
    if (!gbPixelPlusInit)
    {
        _PixelPlus_Internal_Rom_Setting();
        _PixelPlus_PO3100K_20bit_SMPTE_25fps();
        gbPixelPlusInit = MMP_TRUE;
    }
        
    PixelPlus_WriteI2C_Byte(0x03,0x00);
    PixelPlus_WriteI2C_Byte(0x4F,0x04);
    PixelPlus_WriteI2C_Byte(0x5C,0x00);
    PixelPlus_WriteI2C_Byte(0x5D,0xDF);
    PixelPlus_WriteI2C_Byte(0x5E,0x33);
    
    gFlickerMode = AntiFlicker_50Hz_MODE;
};

void
mmpPixelPlusSetAntiFlickerOff(
    )
{
    PixelPlus_WriteI2C_Byte(0x03,0x00);
    PixelPlus_WriteI2C_Byte(0x4F,0x00);
};

void
mmpPixelPlusSetAntiFlickerAuto(
    )
{
    PixelPlus_WriteI2C_Byte(0x03,0x00);
    PixelPlus_WriteI2C_Byte(0x4F,0x40);
    PixelPlus_WriteI2C_Byte(0x51,0x05);
    PixelPlus_WriteI2C_Byte(0x52,0x76);
    PixelPlus_WriteI2C_Byte(0x53,0x04);
    PixelPlus_WriteI2C_Byte(0x54,0x8D);
    PixelPlus_WriteI2C_Byte(0x59,0x00);
    PixelPlus_WriteI2C_Byte(0x5A,0xBB);
    PixelPlus_WriteI2C_Byte(0x5B,0x80);
    PixelPlus_WriteI2C_Byte(0x5C,0x00);
    PixelPlus_WriteI2C_Byte(0x5D,0xE1);
    PixelPlus_WriteI2C_Byte(0x5E,0x00);
    PixelPlus_WriteI2C_Byte(0x5F,0x04);
    PixelPlus_WriteI2C_Byte(0x60,0x65);
    
    gFlickerMode = AntiFlicker_60Hz_MODE;
};

