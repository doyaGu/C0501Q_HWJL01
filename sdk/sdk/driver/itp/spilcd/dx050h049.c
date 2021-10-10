#include <stdbool.h>
#include <stdint.h>
#include "ite/itp.h"

/* 
    Panel Resolution: 480x854
    LCD Single Chip Driver: ILI9806E
*/

#define CLK_PORT CFG_LCD_SPI_CLK_GPIO
#define CS_PORT CFG_LCD_SPI_CS_GPIO
#define DATA_PORT CFG_LCD_SPI_DATA_GPIO
#define LCM_RST CFG_LCD_LCM_RESET
#define LCD_POWER_PORT CFG_LCD_LCM_POWER

void delay(int count)
{
    int i;
    ithEnterCritical();
    for (i = 0; i < count; i++)
        i++;
    ithExitCritical();
}

static void dx050h049_spi_init_gpio()
{
    ithGpioSet(CLK_PORT);
    ithGpioSetMode(CLK_PORT, ITH_GPIO_MODE0);
    ithGpioSetOut(CLK_PORT);

    ithGpioSet(CS_PORT);
    ithGpioSetMode(CS_PORT, ITH_GPIO_MODE0);
    ithGpioSetOut(CS_PORT);

    ithGpioSet(DATA_PORT);
    ithGpioSetMode(DATA_PORT, ITH_GPIO_MODE0);
    ithGpioSetOut(DATA_PORT);

    ithGpioSet(LCD_POWER_PORT);
    ithGpioSetMode(LCD_POWER_PORT, ITH_GPIO_MODE0);
    ithGpioSetOut(LCD_POWER_PORT);

    usleep(5 * 1000);

    ithGpioSet(LCM_RST);
    ithGpioSetMode(LCM_RST, ITH_GPIO_MODE0);
    ithGpioSetOut(LCM_RST);

    ithGpioClear(CLK_PORT);
    ithGpioSet(CS_PORT);
}

static void dx050h049_spi_send(uint8_t data, bool dc)
{
    uint16_t buf = 0;
    int i;

    // If the D/C bit is “low”, the transmission byte is interpreted as a command byte.
    // If the D/C bit is “high”, the transmission byte is stored in the command register as a parameter data.
    buf = (dc) ? (0x0100 | data) : data;
    for (i = 0; i < 9; i++)
    {
        ithGpioClear(CLK_PORT);
        delay(20);

        if (buf & 0x0100)
        {
            ithGpioSet(DATA_PORT);
        }
        else
        {
            ithGpioClear(DATA_PORT);
        }
        delay(20);

        ithGpioSet(CLK_PORT);
        delay(20);

        buf <<= 1;
    }
    ithGpioClear(CLK_PORT);
}

static uint8_t dx050h049_spi_read(uint8_t cmd)
{
    uint8_t data = 0;
    int i;

    ithGpioClear(CLK_PORT);
    delay(10);

    dx050h049_spi_send(cmd, false);

    ithGpioSet(DATA_PORT);
    ithGpioSetIn(DATA_PORT);
    delay(5);

    for (i = 0; i < 8; i++)
    {
        ithGpioClear(CLK_PORT);
        delay(30);
        ithGpioSet(CLK_PORT);
        delay(20);

        if (ithGpioGet(DATA_PORT))
        {
            data |= 0x01;
        }
        data <<= 1;
    }
    ithGpioClear(CLK_PORT);
    ithGpioSetOut(DATA_PORT);
    ithGpioSet(CLK_PORT);
    delay(2);

    return data;
}

static void dx050h049_spi_send_cmd(uint8_t cmd)
{
    ithGpioClear(CS_PORT);
    delay(10);

    dx050h049_spi_send(cmd, false);

    ithGpioSet(CS_PORT);
    delay(2);
}

static void dx050h049_spi_send_data(uint8_t data)
{
    ithGpioClear(CS_PORT);
    delay(10);

    dx050h049_spi_send(data, true);

    ithGpioSet(CS_PORT);
    delay(2);
}

static void dx050h049_spi_init_lcd()
{
    ithGpioSet(LCM_RST);
    usleep(10 * 1000);
    ithGpioClear(LCM_RST);
    usleep(10 * 1000);
    ithGpioSet(LCM_RST);
    usleep(120 * 1000);

    // Change to Page 1 Command Set
    dx050h049_spi_send_cmd(0xFF);  // ENEXTC
    dx050h049_spi_send_data(0xFF); // Fixed
    dx050h049_spi_send_data(0x98); // Fixed
    dx050h049_spi_send_data(0x06); // Fixed
    dx050h049_spi_send_data(0x04); // Fixed
    dx050h049_spi_send_data(0x01); // Page 1

    // Output SDO
    dx050h049_spi_send_cmd(0x08);  // IFMODE1
    dx050h049_spi_send_data(0x10); // SDO_STATUS = 1: always output, but without output tri-state.

    // Enable DE polarity
    dx050h049_spi_send_cmd(0x21);  // DISCTRL2
    dx050h049_spi_send_data(0x01); // EPL: DE polarity (“0”= Low enable, “1”= High enable)

    // Set panel resolution
    dx050h049_spi_send_cmd(0x30);  // RESCTRL
    dx050h049_spi_send_data(0x01); // RES[2:0] = 001, 480X854

    // Set display inversion mode
    dx050h049_spi_send_cmd(0x31);  // INVTR
    dx050h049_spi_send_data(0x00); // NLA[3:0] = 000, Column inversion

    // Set EXTP & EXTN and Output DDVDH / DDVDL voltage
    dx050h049_spi_send_cmd(0x40);  // PWCTRL1
    dx050h049_spi_send_data(0x15); // EXT_CPCK_SEL[1:0] = 01, Output x 2 waveform; BT[3:0] = 0101, VCI X 2.5 / VCI X -2.5

    // Set DDVDH / DDVDL clamp level
    dx050h049_spi_send_cmd(0x41);  // PWCTRL2
    dx050h049_spi_send_data(0x33); // DDVDH_CLP[2:0] = 011, DDVDH clamp level = 5.2V; DDVDL_CLP[2:0] = 011, DDVDL clamp level = -5.2V

    // Set VGH / VGL Output
    dx050h049_spi_send_cmd(0x42);  // PWCTRL3
    dx050h049_spi_send_data(0x03); // VGH_CP[1:0] = 00, 2DDVDH-DDVDL; VGL_CP[1:0] = DDVDL-DDVDH

    // Disable VGH clamp level
    dx050h049_spi_send_cmd(0x43);  // PWCTRL4
    dx050h049_spi_send_data(0x09); // VGH_CLPEN = 0, disable VGH clamp level; VGH_CLP[3:0] = 1001, VGH clamp level = 15.0V

    // Disable VGL clamp level
    dx050h049_spi_send_cmd(0x44);  // PWCTRL5
    dx050h049_spi_send_data(0x09); // VGL_CLPEN = 0, disable VGL clamp level; VGL_CLP[3:0] = 1001, VGL clamp level = -12.0V

    // Set VGH_REG / VGL_REG operating voltage
    dx050h049_spi_send_cmd(0x45);  // PWCTRL6
    dx050h049_spi_send_data(0x16); // VGH_REG[3:0] = 0001, VGH_REG operation voltage = 9.0V; VGL_REG[3:0] = 0110, VGL_REG operation voltage = -10.0V

    // Set VREG1OUT voltage for positive Gamma
    dx050h049_spi_send_cmd(0x50);  // PWCTRL9
    dx050h049_spi_send_data(0x68); // VREG1[7:0] = 01101000, VREG1OUT = 4.3000V

    // Set VREG2OUT voltage for negative Gamma
    dx050h049_spi_send_cmd(0x51);  // PWCTRL10
    dx050h049_spi_send_data(0x68); // VREG2[7:0] = 01101000, VREG1OUT = -4.3000V

    // Set factor to generate VCOM voltage
    // GS = 0, VCOM voltage = -1.0500V; GS = 1, VCOM voltage = -1.1125V
    dx050h049_spi_send_cmd(0x52);  // VMCTRL1
    dx050h049_spi_send_data(0x00); // VCM1[8] = 0
    dx050h049_spi_send_cmd(0x53);  // VMCTRL2
    dx050h049_spi_send_data(0x45); // VCM1[7:0] = 01000101, VCOM = -1.0500V
    dx050h049_spi_send_cmd(0x54);  // VMCTRL3
    dx050h049_spi_send_data(0x00); // VCM2[8] = 0
    dx050h049_spi_send_cmd(0x55);  // VMCTRL4
    dx050h049_spi_send_data(0x4A); // VCM2[7:0] = 01001010, VCOM = -1.1125V

    // Adjust Vcore voltage and Set Low voltage detection for voltage level setting
    dx050h049_spi_send_cmd(0x57);  // LVD
    dx050h049_spi_send_data(0x50); // VCORE_VD[2:0] = 000, 1.50V; VDET[2:0] = 101, 2.3V

    // Adjust Source SDT timing
    dx050h049_spi_send_cmd(0x60);  // Source Timing Adjust 1
    dx050h049_spi_send_data(0x07); // SDTI[5:0] = 000111, 7 time scales

    // Adjust Source CR timing
    dx050h049_spi_send_cmd(0x61);  // Source Timing Adjust 2
    dx050h049_spi_send_data(0x00); // CRTI[5:0] = 000000, 0 time scales

    // Adjust Source EQ timing
    dx050h049_spi_send_cmd(0x62);  // Source Timing Adjust 3
    dx050h049_spi_send_data(0x08); // EQTI[5:0] = 001000, 8 time scales

    // Adjust Source PC timing
    dx050h049_spi_send_cmd(0x63);  // Source Timing Adjust 4
    dx050h049_spi_send_data(0x00); // PCTI[5:0] = 000000, 0 time scales

    // Positive Gamma Control
    dx050h049_spi_send_cmd(0xA0);
    dx050h049_spi_send_data(0x00); // REGAM0_P[5:0] = 000000
    dx050h049_spi_send_cmd(0xA1);
    dx050h049_spi_send_data(0x09); // REGAM4_P[5:0] = 001001
    dx050h049_spi_send_cmd(0xA2);
    dx050h049_spi_send_data(0x0F); // REGAM8_P[5:0] = 001111
    dx050h049_spi_send_cmd(0xA3);
    dx050h049_spi_send_data(0x0B); // REGAM16_P[4:0] = 01011
    dx050h049_spi_send_cmd(0xA4);
    dx050h049_spi_send_data(0x06); // REGAM24_P[4:0] = 00110
    dx050h049_spi_send_cmd(0xA5);
    dx050h049_spi_send_data(0x09); // REGAM52_P[4:0] = 01001
    dx050h049_spi_send_cmd(0xA6);
    dx050h049_spi_send_data(0x07); // REGAM80_P[3:0] = 0111
    dx050h049_spi_send_cmd(0xA7);
    dx050h049_spi_send_data(0x05); // REGAM108_P[3:0] = 0101
    dx050h049_spi_send_cmd(0xA8);
    dx050h049_spi_send_data(0x08); // REGAM147_P[3:0] = 1000
    dx050h049_spi_send_cmd(0xA9);
    dx050h049_spi_send_data(0x0C); // REGAM175_P[3:0] = 1100
    dx050h049_spi_send_cmd(0xAA);
    dx050h049_spi_send_data(0x12); // REGAM203_P[4:0] = 10010
    dx050h049_spi_send_cmd(0xAB);
    dx050h049_spi_send_data(0x08); // REGAM231_P[4:0] = 01000
    dx050h049_spi_send_cmd(0xAC);
    dx050h049_spi_send_data(0x0D); // REGAM239_P[4:0] = 01101
    dx050h049_spi_send_cmd(0xAD);
    dx050h049_spi_send_data(0x17); // REGAM247_P[5:0] = 010111
    dx050h049_spi_send_cmd(0xAE);
    dx050h049_spi_send_data(0x0E); // REGAM251_P[5:0] = 001110
    dx050h049_spi_send_cmd(0xAF);
    dx050h049_spi_send_data(0x00); // REGAM255_P[5:0] = 000000

    // Negative Gamma Correction
    dx050h049_spi_send_cmd(0xC0);
    dx050h049_spi_send_data(0x00); // REGAM0_N[5:0] = 000000
    dx050h049_spi_send_cmd(0xC1);
    dx050h049_spi_send_data(0x08); // REGAM4_N[5:0] = 001000
    dx050h049_spi_send_cmd(0xC2);
    dx050h049_spi_send_data(0x0E); // REGAM8_N[5:0] = 001110
    dx050h049_spi_send_cmd(0xC3);
    dx050h049_spi_send_data(0x0B); // REGAM16_N[4:0] = 01011
    dx050h049_spi_send_cmd(0xC4);
    dx050h049_spi_send_data(0x05); // REGAM24_N[4:0] = 00101
    dx050h049_spi_send_cmd(0xC5);
    dx050h049_spi_send_data(0x09); // REGAM52_N[4:0] = 01001
    dx050h049_spi_send_cmd(0xC6);
    dx050h049_spi_send_data(0x07); // REGAM80_N[3:0] = 0111
    dx050h049_spi_send_cmd(0xC7);
    dx050h049_spi_send_data(0x04); // REGAM108_N[3:0] = 0100
    dx050h049_spi_send_cmd(0xC8);
    dx050h049_spi_send_data(0x08); // REGAM147_N[3:0] = 1000
    dx050h049_spi_send_cmd(0xC9);
    dx050h049_spi_send_data(0x0C); // REGAM175_N[3:0] = 1100
    dx050h049_spi_send_cmd(0xCA);
    dx050h049_spi_send_data(0x11); // REGAM203_N[4:0] = 10001
    dx050h049_spi_send_cmd(0xCB);
    dx050h049_spi_send_data(0x07); // REGAM231_N[4:0] = 00111
    dx050h049_spi_send_cmd(0xCC);
    dx050h049_spi_send_data(0x0D); // REGAM239_N[4:0] = 01101
    dx050h049_spi_send_cmd(0xCD);
    dx050h049_spi_send_data(0x17); // REGAM247_N[5:0] = 010111
    dx050h049_spi_send_cmd(0xCE);
    dx050h049_spi_send_data(0x0E); // REGAM251_N[5:0] = 001110
    dx050h049_spi_send_cmd(0xCF);
    dx050h049_spi_send_data(0x00); // REGAM255_N[5:0] = 000000

    // Change to Page 6 Command Set
    dx050h049_spi_send_cmd(0xFF);  // ENEXTC
    dx050h049_spi_send_data(0xFF); // Fixed
    dx050h049_spi_send_data(0x98); // Fixed
    dx050h049_spi_send_data(0x06); // Fixed
    dx050h049_spi_send_data(0x04); // Fixed
    dx050h049_spi_send_data(0x06); // Page 6

    // GIP Setting
    dx050h049_spi_send_cmd(0x00);
    dx050h049_spi_send_data(0x21); // STV_A_Rise[10:8] = 001; GIP_0_SET0 = 1
    dx050h049_spi_send_cmd(0x01);
    dx050h049_spi_send_data(0x09); // STV_A_Rise[7:0] = 00001001
    dx050h049_spi_send_cmd(0x02);
    dx050h049_spi_send_data(0x00); // GIP_0_SET1 = 0
    dx050h049_spi_send_cmd(0x03);
    dx050h049_spi_send_data(0x00); // GIP_0_SET2 = 0
    dx050h049_spi_send_cmd(0x04);
    dx050h049_spi_send_data(0x01); // GIP_0_SET3 = 0
    dx050h049_spi_send_cmd(0x05);
    dx050h049_spi_send_data(0x01); // GIP_0_SET4 = 0
    dx050h049_spi_send_cmd(0x06);
    dx050h049_spi_send_data(0x80); // CLK_A_Rise[10:8] = 100; GIP_0_SET5 = 0
    dx050h049_spi_send_cmd(0x07);
    dx050h049_spi_send_data(0x05); // CLK_A_Rise[7:0] = 00000101
    dx050h049_spi_send_cmd(0x08);
    dx050h049_spi_send_data(0x02); // GIP_0_SET6 = 2
    dx050h049_spi_send_cmd(0x09);
    dx050h049_spi_send_data(0x80); // GIP_0_SET7 = 0x80
    dx050h049_spi_send_cmd(0x0A);
    dx050h049_spi_send_data(0x00); // GIP_0_SET8 = 0
    dx050h049_spi_send_cmd(0x0B);
    dx050h049_spi_send_data(0x00); // GIP_0_SET9 = 0
    dx050h049_spi_send_cmd(0x0C);
    dx050h049_spi_send_data(0x0A); // GIP_0_SET10 = 0x0A
    dx050h049_spi_send_cmd(0x0D);
    dx050h049_spi_send_data(0x0A); // GIP_0_SET11 = 0x0A
    dx050h049_spi_send_cmd(0x0E);
    dx050h049_spi_send_data(0x00); // GIP_0_SET12 = 0
    dx050h049_spi_send_cmd(0x0F);
    dx050h049_spi_send_data(0x00); // GIP_0_SET13 = 0
    dx050h049_spi_send_cmd(0x10);
    dx050h049_spi_send_data(0xE0); // GIP_0_SET14 = 0xE0
    dx050h049_spi_send_cmd(0x11);
    dx050h049_spi_send_data(0xE4); // GIP_0_SET15 = 0xE4
    dx050h049_spi_send_cmd(0x12);
    dx050h049_spi_send_data(0x04); // GIP_0_SET16 = 4
    dx050h049_spi_send_cmd(0x13);
    dx050h049_spi_send_data(0x00); // GIP_0_SET17 = 0
    dx050h049_spi_send_cmd(0x14);
    dx050h049_spi_send_data(0x00); // GIP_0_SET18 = 0
    dx050h049_spi_send_cmd(0x15);
    dx050h049_spi_send_data(0xC0); // GIP_0_SET19 = 0xC0
    dx050h049_spi_send_cmd(0x16);
    dx050h049_spi_send_data(0x08); // GIP_0_SET20 = 8
    dx050h049_spi_send_cmd(0x17);
    dx050h049_spi_send_data(0x00); // GIP_0_SET21 = 0
    dx050h049_spi_send_cmd(0x18);
    dx050h049_spi_send_data(0x00); // GIP_0_SET22 = 0
    dx050h049_spi_send_cmd(0x19);
    dx050h049_spi_send_data(0x00); // GIP_0_SET23 = 0
    dx050h049_spi_send_cmd(0x1A);
    dx050h049_spi_send_data(0x00); // GIP_0_SET24 = 0
    dx050h049_spi_send_cmd(0x1B);
    dx050h049_spi_send_data(0x00); // GIP_0_SET25 = 0
    dx050h049_spi_send_cmd(0x1C);
    dx050h049_spi_send_data(0x00); // GIP_0_SET26 = 0
    dx050h049_spi_send_cmd(0x1D);
    dx050h049_spi_send_data(0x00); // GIP_0_SET27 = 0

    dx050h049_spi_send_cmd(0x20);
    dx050h049_spi_send_data(0x01); // GIP_1_SET0 = 1
    dx050h049_spi_send_cmd(0x21);
    dx050h049_spi_send_data(0x23); // GIP_1_SET1 = 0x23
    dx050h049_spi_send_cmd(0x22);
    dx050h049_spi_send_data(0x45); // GIP_1_SET2 = 0x45
    dx050h049_spi_send_cmd(0x23);
    dx050h049_spi_send_data(0x67); // GIP_1_SET3 = 0x67
    dx050h049_spi_send_cmd(0x24);
    dx050h049_spi_send_data(0x01); // GIP_1_SET4 = 1
    dx050h049_spi_send_cmd(0x25);
    dx050h049_spi_send_data(0x23); // GIP_1_SET5 = 0x23
    dx050h049_spi_send_cmd(0x26);
    dx050h049_spi_send_data(0x45); // GIP_1_SET6 = 0x45
    dx050h049_spi_send_cmd(0x27);
    dx050h049_spi_send_data(0x67); // GIP_1_SET7 = 0x67

    dx050h049_spi_send_cmd(0x30);
    dx050h049_spi_send_data(0x01); // GIP_2_SET0 = 1
    dx050h049_spi_send_cmd(0x31);
    dx050h049_spi_send_data(0x11); // GIP_2_SET1 = 0x11
    dx050h049_spi_send_cmd(0x32);
    dx050h049_spi_send_data(0x00); // GIP_2_SET2 = 0
    dx050h049_spi_send_cmd(0x33);
    dx050h049_spi_send_data(0xEE); // GIP_2_SET3 = 0xEE
    dx050h049_spi_send_cmd(0x34);
    dx050h049_spi_send_data(0xFF); // GIP_2_SET4 = 0xFF
    dx050h049_spi_send_cmd(0x35);
    dx050h049_spi_send_data(0xBB); // GIP_2_SET5 = 0xBB
    dx050h049_spi_send_cmd(0x36);
    dx050h049_spi_send_data(0xCA); // GIP_2_SET6 = 0xCA
    dx050h049_spi_send_cmd(0x37);
    dx050h049_spi_send_data(0xDD); // GIP_2_SET7 = 0xDD
    dx050h049_spi_send_cmd(0x38);
    dx050h049_spi_send_data(0xAC); // GIP_2_SET8 = 0xAC
    dx050h049_spi_send_cmd(0x39);
    dx050h049_spi_send_data(0x76); // GIP_2_SET9 = 0x76
    dx050h049_spi_send_cmd(0x3A);
    dx050h049_spi_send_data(0x67); // GIP_2_SET10 = 0x67
    dx050h049_spi_send_cmd(0x3B);
    dx050h049_spi_send_data(0x22); // GIP_2_SET11 = 0x22
    dx050h049_spi_send_cmd(0x3C);
    dx050h049_spi_send_data(0x22); // GIP_2_SET12 = 0x22
    dx050h049_spi_send_cmd(0x3D);
    dx050h049_spi_send_data(0x22); // GIP_2_SET13 = 0x22
    dx050h049_spi_send_cmd(0x3E);
    dx050h049_spi_send_data(0x22); // GIP_2_SET14 = 0x22
    dx050h049_spi_send_cmd(0x3F);
    dx050h049_spi_send_data(0x22); // GIP_2_SET15 = 0x22
    dx050h049_spi_send_cmd(0x40);
    dx050h049_spi_send_data(0x22); // GIP_2_SET16 = 0x22

    // ???
    dx050h049_spi_send_cmd(0x52);
    dx050h049_spi_send_data(0x10);

    // GOUT_VGLO Control
    dx050h049_spi_send_cmd(0x53);  // GVLOCTRL
    dx050h049_spi_send_data(0x10); // GOUT_VGLO[3:0] = 0000, VGL

    // GOUT_VGHO Control
    dx050h049_spi_send_cmd(0x54);  // GVHOCTRL
    dx050h049_spi_send_data(0x13); // GOUT_VGHO[2:0] = 001, VGH

    // Change to Page 7 Command Set
    dx050h049_spi_send_cmd(0xFF);  // ENEXTC
    dx050h049_spi_send_data(0xFF); // Fixed
    dx050h049_spi_send_data(0x98); // Fixed
    dx050h049_spi_send_data(0x06); // Fixed
    dx050h049_spi_send_data(0x04); // Fixed
    dx050h049_spi_send_data(0x07); // Page 7

    // Disable VGL_REG
    dx050h049_spi_send_cmd(0x17);  // VGLREGEN
    dx050h049_spi_send_data(0x22); // VGLREG_EN = 0

    // ???
    dx050h049_spi_send_cmd(0x02);
    dx050h049_spi_send_data(0x77);

    // ???
    dx050h049_spi_send_cmd(0xE1);
    dx050h049_spi_send_data(0x79);

    // Change to Page 0 Command Set
    dx050h049_spi_send_cmd(0xFF);  // ENEXTC
    dx050h049_spi_send_data(0xFF); // Fixed
    dx050h049_spi_send_data(0x98); // Fixed
    dx050h049_spi_send_data(0x06); // Fixed
    dx050h049_spi_send_data(0x04); // Fixed
    dx050h049_spi_send_data(0x00); // Page 0

    // Set panel operation mode
    dx050h049_spi_send_cmd(0x36);  // Display Access Control
    dx050h049_spi_send_data(0x03); // BGR = 0, RGB color filter panel; SS = 1, Flip Horizontal; GS = 1, Flip Vertical

    // Turns off sleep mode
    dx050h049_spi_send_cmd(0x11);  // SLPOUT
    dx050h049_spi_send_data(0x00); // No parameter
    usleep(120 * 1000);

    // Recover from Display Off mode
    dx050h049_spi_send_cmd(0x29);  // DISON
    dx050h049_spi_send_data(0x00); // No parameter
    usleep(10 * 1000);
}

// void lcm_init_DX050H049()
// {

// }

void itpSpiLcdInit()
{
    printf("Init LCM: lcm_init_DX050H049.\n");
    printf("CLK_PORT = %d\n", CLK_PORT);
    printf("CS_PORT = %d\n", CS_PORT);
    printf("DATA_PORT = %d\n", DATA_PORT);

    printf("LCM_RST = %d\n", LCM_RST);
    printf("LCD_POWER_PORT = %d\n", LCD_POWER_PORT);

    dx050h049_spi_init_gpio();
    dx050h049_spi_init_lcd();
}