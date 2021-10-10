/*
 * Copyright (c) 2011 ITE Tech. Inc. All Rights Reserved.
 */
/** @file
 * Castor3 keypad module.
 *
 * @author Joseph Chang
 * @version 1.0
 */
#include "../itp_cfg.h"
#include "iic/mmp_iic.h"
#include <errno.h>
#include <pthread.h>
#include "openrtos/FreeRTOS.h"


/**************************************************************************
** MACRO defination                                                      **
***************************************************************************/
//#define ENABLE_KEYPAD_DBG_MODE
//#define ENABLE_SAVE_RESTORE_ALL_INTR


#ifndef	CFG_TOUCH_KEY_NUM
#define CFG_TOUCH_KEY_NUM	4
#endif

#define IT7230_IIC_ADDR		(0x2E>>1)


#define TK_GPIO_PIN	    CFG_GPIO_KEYPAD

#if (TK_GPIO_PIN<32)
#define TK_GPIO_MASK    (1<<TK_GPIO_PIN)
#else
#define TK_GPIO_MASK    (1<<(TK_GPIO_PIN-32))
#endif

/**************************************************************************
**            "ITE_types.h"                                              **
***************************************************************************/
#define TRUE	1
#define FALSE	0
//#define NULL	((void *)0)

typedef unsigned char			UCHAR;
typedef unsigned short			USHORT;
typedef unsigned long			ULONG;
typedef UCHAR					BYTE;
typedef UCHAR *					PBYTE;
typedef USHORT                  WORD;
typedef USHORT *                PWORD;
typedef unsigned char *			PUCHAR;
typedef unsigned short *		PUSHORT;
typedef unsigned long *			PULONG;
typedef unsigned char			BOOL;
typedef void					VOID;
/**************************************************************************
**         "ITE_types.h"                                                 **
***************************************************************************/





/**************************************************************************
** "IT7230_TouchKey.h" ^                                                 **
***************************************************************************/
typedef struct InitCapSReg
{
    BYTE  page;    
    BYTE  reg;     
    WORD   value;   
} sInitCapSReg; 

//--- Calibration Engine (Page number = 0) ---//
#define CAPS_CE_Base                0x00
#define CAPS_PSR                    (CAPS_CE_Base+0x00)
#define CAPS_SIR                    (CAPS_CE_Base+0x01)
#define CAPS_SXPOSR                 (CAPS_CE_Base+0x02)
#define CAPS_SXCHSR                 (CAPS_CE_Base+0x03)
#define CAPS_SXCLSR                 (CAPS_CE_Base+0x04)
#define CAPS_SXDSR                  (CAPS_CE_Base+0x05)
#define CAPS_SXCSR                  (CAPS_CE_Base+0x06)
#define CAPS_SLR                    (CAPS_CE_Base+0x07)

#define CAPS_S0CCDCVR               (CAPS_CE_Base+0x20)
#define CAPS_S1CCDCVR               (CAPS_CE_Base+0x21)
#define CAPS_S2CCDCVR               (CAPS_CE_Base+0x22)
#define CAPS_S3CCDCVR               (CAPS_CE_Base+0x23)
#define CAPS_S4CCDCVR               (CAPS_CE_Base+0x24)
#define CAPS_S5CCDCVR               (CAPS_CE_Base+0x25)
#define CAPS_S6CCDCVR               (CAPS_CE_Base+0x26)
#define CAPS_S7CCDCVR               (CAPS_CE_Base+0x27)
#define CAPS_S8CCDCVR               (CAPS_CE_Base+0x28)
#define CAPS_S9CCDCVR               (CAPS_CE_Base+0x29)
#define CAPS_S10CCDCVR              (CAPS_CE_Base+0x2A)
#define CAPS_S11CCDCVR              (CAPS_CE_Base+0x2B)
#define CAPS_S12CCDCVR              (CAPS_CE_Base+0x2C)

// Stage 0 register

#define CAPS_S0FR0                  (CAPS_CE_Base+0x30)
#define CAPS_S0FR1                  (CAPS_CE_Base+0x31)
#define CAPS_S0FR2                  (CAPS_CE_Base+0x32)
#define CAPS_S0DLR                  (CAPS_CE_Base+0x33)
#define CAPS_S0CVR                  (CAPS_CE_Base+0x34)
#define CAPS_S0OHCR                 (CAPS_CE_Base+0x35)
#define CAPS_S0OLCR                 (CAPS_CE_Base+0x36)
#define CAPS_S0OHR                  (CAPS_CE_Base+0x37)
#define CAPS_S0OLR                  (CAPS_CE_Base+0x38)
#define CAPS_S0OHUR                 (CAPS_CE_Base+0x39)
#define CAPS_S0OLUR                 (CAPS_CE_Base+0x3A)
#define CAPS_S0HTR                  (CAPS_CE_Base+0x3B)
#define CAPS_S0LTR                  (CAPS_CE_Base+0x3C)
#define CAPS_S0SR                   (CAPS_CE_Base+0x3D)
#define CAPS_S0CDC_CMP              (CAPS_CE_Base+0x3E)
#define CAPS_S0CSR                  (CAPS_CE_Base+0x3F)

// Stage 1 register

#define CAPS_S1FR0                  (CAPS_CE_Base+0x40)
#define CAPS_S1FR1                  (CAPS_CE_Base+0x41)
#define CAPS_S1FR2                  (CAPS_CE_Base+0x42)
#define CAPS_S1DLR                  (CAPS_CE_Base+0x43)
#define CAPS_S1CVR                  (CAPS_CE_Base+0x44)
#define CAPS_S1OHCR                 (CAPS_CE_Base+0x45)
#define CAPS_S1OLCR                 (CAPS_CE_Base+0x46)
#define CAPS_S1OHR                  (CAPS_CE_Base+0x47)
#define CAPS_S1OLR                  (CAPS_CE_Base+0x48)
#define CAPS_S1OHUR                 (CAPS_CE_Base+0x49)
#define CAPS_S1OLUR                 (CAPS_CE_Base+0x4A)
#define CAPS_S1HTR                  (CAPS_CE_Base+0x4B)
#define CAPS_S1LTR                  (CAPS_CE_Base+0x4C)
#define CAPS_S1SR                   (CAPS_CE_Base+0x4D)
#define CAPS_S1CDC_CMP              (CAPS_CE_Base+0x4E)
#define CAPS_S1CSR                  (CAPS_CE_Base+0x4F)

// Stage 2 register

#define CAPS_S2FR0                  (CAPS_CE_Base+0x50)
#define CAPS_S2FR1                  (CAPS_CE_Base+0x51)
#define CAPS_S2FR2                  (CAPS_CE_Base+0x52)
#define CAPS_S2DLR                  (CAPS_CE_Base+0x53)
#define CAPS_S2CVR                  (CAPS_CE_Base+0x54)
#define CAPS_S2OHCR                 (CAPS_CE_Base+0x55)
#define CAPS_S2OLCR                 (CAPS_CE_Base+0x56)
#define CAPS_S2OHR                  (CAPS_CE_Base+0x57)
#define CAPS_S2OLR                  (CAPS_CE_Base+0x58)
#define CAPS_S2OHUR                 (CAPS_CE_Base+0x59)
#define CAPS_S2OLUR                 (CAPS_CE_Base+0x5A)
#define CAPS_S2HTR                  (CAPS_CE_Base+0x5B)
#define CAPS_S2LTR                  (CAPS_CE_Base+0x5C)
#define CAPS_S2SR                   (CAPS_CE_Base+0x5D)
#define CAPS_S2CDC_CMP              (CAPS_CE_Base+0x5E)
#define CAPS_S2CSR                  (CAPS_CE_Base+0x5F)

// Stage 3 register

#define CAPS_S3FR0                  (CAPS_CE_Base+0x60)
#define CAPS_S3FR1                  (CAPS_CE_Base+0x61)
#define CAPS_S3FR2                  (CAPS_CE_Base+0x62)
#define CAPS_S3DLR                  (CAPS_CE_Base+0x63)
#define CAPS_S3CVR                  (CAPS_CE_Base+0x64)
#define CAPS_S3OHCR                 (CAPS_CE_Base+0x65)
#define CAPS_S3OLCR                 (CAPS_CE_Base+0x66)
#define CAPS_S3OHR                  (CAPS_CE_Base+0x67)
#define CAPS_S3OLR                  (CAPS_CE_Base+0x68)
#define CAPS_S3OHUR                 (CAPS_CE_Base+0x69)
#define CAPS_S3OLUR                 (CAPS_CE_Base+0x6A)
#define CAPS_S3HTR                  (CAPS_CE_Base+0x6B)
#define CAPS_S3LTR                  (CAPS_CE_Base+0x6C)
#define CAPS_S3SR                   (CAPS_CE_Base+0x6D)
#define CAPS_S3CDC_CMP              (CAPS_CE_Base+0x6E)
#define CAPS_S3CSR                  (CAPS_CE_Base+0x6F)

// Stage 4 register

#define CAPS_S4FR0                  (CAPS_CE_Base+0x70)
#define CAPS_S4FR1                  (CAPS_CE_Base+0x71)
#define CAPS_S4FR2                  (CAPS_CE_Base+0x72)
#define CAPS_S4DLR                  (CAPS_CE_Base+0x73)
#define CAPS_S4CVR                  (CAPS_CE_Base+0x74)
#define CAPS_S4OHCR                 (CAPS_CE_Base+0x75)
#define CAPS_S4OLCR                 (CAPS_CE_Base+0x76)
#define CAPS_S4OHR                  (CAPS_CE_Base+0x77)
#define CAPS_S4OLR                  (CAPS_CE_Base+0x78)
#define CAPS_S4OHUR                 (CAPS_CE_Base+0x79)
#define CAPS_S4OLUR                 (CAPS_CE_Base+0x7A)
#define CAPS_S4HTR                  (CAPS_CE_Base+0x7B)
#define CAPS_S4LTR                  (CAPS_CE_Base+0x7C)
#define CAPS_S4SR                   (CAPS_CE_Base+0x7D)
#define CAPS_S4CDC_CMP              (CAPS_CE_Base+0x7E)
#define CAPS_S4CSR                  (CAPS_CE_Base+0x7F)

// Stage 5 register

#define CAPS_S5FR0                  (CAPS_CE_Base+0x80)
#define CAPS_S5FR1                  (CAPS_CE_Base+0x81)
#define CAPS_S5FR2                  (CAPS_CE_Base+0x82)
#define CAPS_S5DLR                  (CAPS_CE_Base+0x83)
#define CAPS_S5CVR                  (CAPS_CE_Base+0x84)
#define CAPS_S5OHCR                 (CAPS_CE_Base+0x85)
#define CAPS_S5OLCR                 (CAPS_CE_Base+0x86)
#define CAPS_S5OHR                  (CAPS_CE_Base+0x87)
#define CAPS_S5OLR                  (CAPS_CE_Base+0x88)
#define CAPS_S5OHUR                 (CAPS_CE_Base+0x89)
#define CAPS_S5OLUR                 (CAPS_CE_Base+0x8A)
#define CAPS_S5HTR                  (CAPS_CE_Base+0x8B)
#define CAPS_S5LTR                  (CAPS_CE_Base+0x8C)
#define CAPS_S5SR                   (CAPS_CE_Base+0x8D)
#define CAPS_S5CDC_CMP              (CAPS_CE_Base+0x8E)
#define CAPS_S5CSR                  (CAPS_CE_Base+0x8F)

// Stage 6 register

#define CAPS_S6FR0                  (CAPS_CE_Base+0x90)
#define CAPS_S6FR1                  (CAPS_CE_Base+0x91)
#define CAPS_S6FR2                  (CAPS_CE_Base+0x92)
#define CAPS_S6DLR                  (CAPS_CE_Base+0x93)
#define CAPS_S6CVR                  (CAPS_CE_Base+0x94)
#define CAPS_S6OHCR                 (CAPS_CE_Base+0x95)
#define CAPS_S6OLCR                 (CAPS_CE_Base+0x96)
#define CAPS_S6OHR                  (CAPS_CE_Base+0x97)
#define CAPS_S6OLR                  (CAPS_CE_Base+0x98)
#define CAPS_S6OHUR                 (CAPS_CE_Base+0x99)
#define CAPS_S6OLUR                 (CAPS_CE_Base+0x9A)
#define CAPS_S6HTR                  (CAPS_CE_Base+0x9B)
#define CAPS_S6LTR                  (CAPS_CE_Base+0x9C)
#define CAPS_S6SR                   (CAPS_CE_Base+0x9D)
#define CAPS_S6CDC_CMP              (CAPS_CE_Base+0x9E)
#define CAPS_S6CSR                  (CAPS_CE_Base+0x9F)

// Stage 7 register

#define CAPS_S7FR0                  (CAPS_CE_Base+0xA0)
#define CAPS_S7FR1                  (CAPS_CE_Base+0xA1)
#define CAPS_S7FR2                  (CAPS_CE_Base+0xA2)
#define CAPS_S7DLR                  (CAPS_CE_Base+0xA3)
#define CAPS_S7CVR                  (CAPS_CE_Base+0xA4)
#define CAPS_S7OHCR                 (CAPS_CE_Base+0xA5)
#define CAPS_S7OLCR                 (CAPS_CE_Base+0xA6)
#define CAPS_S7OHR                  (CAPS_CE_Base+0xA7)
#define CAPS_S7OLR                  (CAPS_CE_Base+0xA8)
#define CAPS_S7OHUR                 (CAPS_CE_Base+0xA9)
#define CAPS_S7OLUR                 (CAPS_CE_Base+0xAA)
#define CAPS_S7HTR                  (CAPS_CE_Base+0xAB)
#define CAPS_S7LTR                  (CAPS_CE_Base+0xAC)
#define CAPS_S7SR                   (CAPS_CE_Base+0xAD)
#define CAPS_S7CDC_CMP              (CAPS_CE_Base+0xAE)
#define CAPS_S7CSR                  (CAPS_CE_Base+0xAF)

// Stage 8 register

#define CAPS_S8FR0                  (CAPS_CE_Base+0xB0)
#define CAPS_S8FR1                  (CAPS_CE_Base+0xB1)
#define CAPS_S8FR2                  (CAPS_CE_Base+0xB2)
#define CAPS_S8DLR                  (CAPS_CE_Base+0xB3)
#define CAPS_S8CVR                  (CAPS_CE_Base+0xB4)
#define CAPS_S8OHCR                 (CAPS_CE_Base+0xB5)
#define CAPS_S8OLCR                 (CAPS_CE_Base+0xB6)
#define CAPS_S8OHR                  (CAPS_CE_Base+0xB7)
#define CAPS_S8OLR                  (CAPS_CE_Base+0xB8)
#define CAPS_S8OHUR                 (CAPS_CE_Base+0xB9)
#define CAPS_S8OLUR                 (CAPS_CE_Base+0xBA)
#define CAPS_S8HTR                  (CAPS_CE_Base+0xBB)
#define CAPS_S8LTR                  (CAPS_CE_Base+0xBC)
#define CAPS_S8SR                   (CAPS_CE_Base+0xBD)
#define CAPS_S8CDC_CMP              (CAPS_CE_Base+0xBE)
#define CAPS_S8CSR                  (CAPS_CE_Base+0xBF)

// Stage 9 register

#define CAPS_S9FR0                  (CAPS_CE_Base+0xC0)
#define CAPS_S9FR1                  (CAPS_CE_Base+0xC1)
#define CAPS_S9FR2                  (CAPS_CE_Base+0xC2)
#define CAPS_S9DLR                  (CAPS_CE_Base+0xC3)
#define CAPS_S9CVR                  (CAPS_CE_Base+0xC4)
#define CAPS_S9OHCR                 (CAPS_CE_Base+0xC5)
#define CAPS_S9OLCR                 (CAPS_CE_Base+0xC6)
#define CAPS_S9OHR                  (CAPS_CE_Base+0xC7)
#define CAPS_S9OLR                  (CAPS_CE_Base+0xC8)
#define CAPS_S9OHUR                 (CAPS_CE_Base+0xC9)
#define CAPS_S9OLUR                 (CAPS_CE_Base+0xCA)
#define CAPS_S9HTR                  (CAPS_CE_Base+0xCB)
#define CAPS_S9LTR                  (CAPS_CE_Base+0xCC)
#define CAPS_S9SR                   (CAPS_CE_Base+0xCD)
#define CAPS_S9CDC_CMP              (CAPS_CE_Base+0xCE)
#define CAPS_S9CSR                  (CAPS_CE_Base+0xCF)

// Stage 10 register

#define CAPS_S10FR0                 (CAPS_CE_Base+0xD0)
#define CAPS_S10FR1                 (CAPS_CE_Base+0xD1)
#define CAPS_S10FR2                 (CAPS_CE_Base+0xD2)
#define CAPS_S10DLR                 (CAPS_CE_Base+0xD3)
#define CAPS_S10CVR                 (CAPS_CE_Base+0xD4)
#define CAPS_S10OHCR                (CAPS_CE_Base+0xD5)
#define CAPS_S10OLCR                (CAPS_CE_Base+0xD6)
#define CAPS_S10OHR                 (CAPS_CE_Base+0xD7)
#define CAPS_S10OLR                 (CAPS_CE_Base+0xD8)
#define CAPS_S10OHUR                (CAPS_CE_Base+0xD9)
#define CAPS_S10OLUR                (CAPS_CE_Base+0xDA)
#define CAPS_S10HTR                 (CAPS_CE_Base+0xDB)
#define CAPS_S10LTR                 (CAPS_CE_Base+0xDC)
#define CAPS_S10SR                  (CAPS_CE_Base+0xDD)
#define CAPS_S10CDC_CMP             (CAPS_CE_Base+0xDE)
#define CAPS_S10CSR                 (CAPS_CE_Base+0xDF)

// Stage 11 register

#define CAPS_S11FR0                 (CAPS_CE_Base+0xE0)
#define CAPS_S11FR1                 (CAPS_CE_Base+0xE1)
#define CAPS_S11FR2                 (CAPS_CE_Base+0xE2)
#define CAPS_S11DLR                 (CAPS_CE_Base+0xE3)
#define CAPS_S11CVR                 (CAPS_CE_Base+0xE4)
#define CAPS_S11OHCR                (CAPS_CE_Base+0xE5)
#define CAPS_S11OLCR                (CAPS_CE_Base+0xE6)
#define CAPS_S11OHR                 (CAPS_CE_Base+0xE7)
#define CAPS_S11OLR                 (CAPS_CE_Base+0xE8)
#define CAPS_S11OHUR                (CAPS_CE_Base+0xE9)
#define CAPS_S11OLUR                (CAPS_CE_Base+0xEA)
#define CAPS_S11HTR                 (CAPS_CE_Base+0xEB)
#define CAPS_S11LTR                 (CAPS_CE_Base+0xEC)
#define CAPS_S11SR                  (CAPS_CE_Base+0xED)
#define CAPS_S11CDC_CMP             (CAPS_CE_Base+0xEE)
#define CAPS_S11CSR                 (CAPS_CE_Base+0xEF)

// Stage 12 register

#define CAPS_S12FR0                 (CAPS_CE_Base+0xF0)
#define CAPS_S12FR1                 (CAPS_CE_Base+0xF1)
#define CAPS_S12FR2                 (CAPS_CE_Base+0xF2)
#define CAPS_S12DLR                 (CAPS_CE_Base+0xF3)
#define CAPS_S12CVR                 (CAPS_CE_Base+0xF4)
#define CAPS_S12OHCR                (CAPS_CE_Base+0xF5)
#define CAPS_S12OLCR                (CAPS_CE_Base+0xF6)
#define CAPS_S12OHR                 (CAPS_CE_Base+0xF7)
#define CAPS_S12OLR                 (CAPS_CE_Base+0xF8)
#define CAPS_S12OHUR                (CAPS_CE_Base+0xF9)
#define CAPS_S12OLUR                (CAPS_CE_Base+0xFA)
#define CAPS_S12HTR                 (CAPS_CE_Base+0xFB)
#define CAPS_S12LTR                 (CAPS_CE_Base+0xFC)
#define CAPS_S12SR                  (CAPS_CE_Base+0xFD)
#define CAPS_S12CDC_CMP             (CAPS_CE_Base+0xFE)
#define CAPS_S12CSR                 (CAPS_CE_Base+0xFF)

//--- Calibration Engine (Page number = 1) ---//

#define CAPS_PCR                    (CAPS_CE_Base+0x01)
#define CAPS_PMR                    (CAPS_CE_Base+0x02)
#define CAPS_CFER                   (CAPS_CE_Base+0x03)
#define CAPS_RTR                    (CAPS_CE_Base+0x04)
#define CAPS_CTR                    (CAPS_CE_Base+0x05)
#define CAPS_CRMR                   (CAPS_CE_Base+0x06)
#define CAPS_PDR                    (CAPS_CE_Base+0x07)
#define CAPS_DR                     (CAPS_CE_Base+0x08)

// Hardware slider 0 register

#define CAPS_S0UBR                  (CAPS_CE_Base+0x09)
#define CAPS_S0LBR                  (CAPS_CE_Base+0x0A)
#define CAPS_S0OSR                  (CAPS_CE_Base+0x0B)

// Hardware slider 1 register

#define CAPS_S1UBR                  (CAPS_CE_Base+0x0C)
#define CAPS_S1LBR                  (CAPS_CE_Base+0x0D)
#define CAPS_S1OSR                  (CAPS_CE_Base+0x0E)

// Connection register

#define CAPS_S0CR                   (CAPS_CE_Base+0x10)
#define CAPS_S1CR                   (CAPS_CE_Base+0x11)
#define CAPS_S2CR                   (CAPS_CE_Base+0x12)
#define CAPS_S3CR                   (CAPS_CE_Base+0x13)
#define CAPS_S4CR                   (CAPS_CE_Base+0x14)
#define CAPS_S5CR                   (CAPS_CE_Base+0x15)
#define CAPS_S6CR                   (CAPS_CE_Base+0x16)
#define CAPS_S7CR                   (CAPS_CE_Base+0x17)
#define CAPS_S8CR                   (CAPS_CE_Base+0x18)
#define CAPS_S9CR                   (CAPS_CE_Base+0x19)
#define CAPS_S10CR                  (CAPS_CE_Base+0x1A)
#define CAPS_S11CR                  (CAPS_CE_Base+0x1B)
#define CAPS_S12CR                  (CAPS_CE_Base+0x1C)


#define CAPS_GODCR0                 (CAPS_CE_Base+0x40)
#define CAPS_GODCR1                 (CAPS_CE_Base+0x41)
#define CAPS_I2CODCR                (CAPS_CE_Base+0x42)
#define CAPS_IODCR                  (CAPS_CE_Base+0x43)
#define CAPS_GPIOOSRR               (CAPS_CE_Base+0x44)
#define CAPS_GPIOICR                (CAPS_CE_Base+0x45)
#define CAPS_GPIOCR0                (CAPS_CE_Base+0x46)
#define CAPS_GPIOCR1                (CAPS_CE_Base+0x47)
#define CAPS_GPIOCR2                (CAPS_CE_Base+0x48)       

//--- Interrupt Controller (Page number = 0) ---//

#define CAPS_INTC_Base              0x08
#define CAPS_ISR                    (CAPS_INTC_Base+0x00)
#define CAPS_SXCHAIER               (CAPS_INTC_Base+0x01)
#define CAPS_SXCHRIER               (CAPS_INTC_Base+0x02)
#define CAPS_SXCLAIER               (CAPS_INTC_Base+0x03)
#define CAPS_SXCLRIER               (CAPS_INTC_Base+0x04)
#define CAPS_SXCIER                 (CAPS_INTC_Base+0x05)

//--- General Purpose I/O Port (Page number = 1) ---// 

#define CAPS_GPIO_Base              0x50                
#define CAPS_GPIOMSR                (CAPS_GPIO_Base+0x00)
#define CAPS_GPIODR                 (CAPS_GPIO_Base+0x01)
#define CAPS_GPIOIR                 (CAPS_GPIO_Base+0x02)
#define CAPS_GPIOOR                 (CAPS_GPIO_Base+0x03)
#define CAPS_GPIOMR                 (CAPS_GPIO_Base+0x04)
#define CAPS_GPIOLR                 (CAPS_GPIO_Base+0x05)
#define CAPS_GPIOER                 (CAPS_GPIO_Base+0x06)
#define CAPS_GPIOISR                (CAPS_GPIO_Base+0x07)
#define CAPS_GPIOIMR                (CAPS_GPIO_Base+0x08)
#define CAPS_GPIOPCR                (CAPS_GPIO_Base+0x09)
#define CAPS_GPIONPCR               (CAPS_GPIO_Base+0x0A)                   
#define CAPS_LEDCMR0                (CAPS_GPIO_Base+0x0B)
#define CAPS_LEDCMR1                (CAPS_GPIO_Base+0x0C)
#define CAPS_LEDCMR2                (CAPS_GPIO_Base+0x0D)
#define CAPS_LEDCMR3                (CAPS_GPIO_Base+0x0E)
#define CAPS_LEDRPR                 (CAPS_GPIO_Base+0x0F)
#define CAPS_LEDBR                  (CAPS_GPIO_Base+0x10)
#define CAPS_LEDCGCR                (CAPS_GPIO_Base+0x11)
#define CAPS_LEDPR0                 (CAPS_GPIO_Base+0x12)
#define CAPS_LEDPR1                 (CAPS_GPIO_Base+0x13)
#define CAPS_LEDPR2                 (CAPS_GPIO_Base+0x14)
#define CAPS_LEDPR3                 (CAPS_GPIO_Base+0x15)

//--- Capacitance-to-digital Converter (Page number = 1) ---//

#define CAPS_CDC_Base          		0x20
#define CAPS_C0COR                  (CAPS_CDC_Base+0x00)
#define CAPS_C1COR                  (CAPS_CDC_Base+0x01)
#define CAPS_C2COR                  (CAPS_CDC_Base+0x02)
#define CAPS_C3COR                  (CAPS_CDC_Base+0x03)
#define CAPS_C4COR                  (CAPS_CDC_Base+0x04)
#define CAPS_C5COR                  (CAPS_CDC_Base+0x05)
#define CAPS_C6COR                  (CAPS_CDC_Base+0x06)
#define CAPS_C7COR                  (CAPS_CDC_Base+0x07)
#define CAPS_C8COR                  (CAPS_CDC_Base+0x08)
#define CAPS_C9COR                  (CAPS_CDC_Base+0x09)
#define CAPS_C10COR                 (CAPS_CDC_Base+0x0A)
#define CAPS_C11COR                 (CAPS_CDC_Base+0x0B)
#define CAPS_C12COR                 (CAPS_CDC_Base+0x0C)
#define CAPS_ICR0                   (CAPS_CDC_Base+0x0D)
#define CAPS_ICR1                   (CAPS_CDC_Base+0x0E)
#define CAPS_COER0                  (CAPS_CDC_Base+0x0F)
#define CAPS_COER1                  (CAPS_CDC_Base+0x10)
#define CAPS_PDCR                   (CAPS_CDC_Base+0x11)
#define CAPS_CGCR                   (CAPS_CDC_Base+0x12)

//--- On-chip OTP Memory (Page number = 2) ---//

#define CAPS_OTPOMR                 0x02
#define Page_0_Stop_Addr            0x80
#define Page_1_Stop_Addr            0x81
/////////////////////////////////////////////////////////////////////////////////
//--------------------------Define stage type ---------------------------------// 
/////////////////////////////////////////////////////////////////////////////////
// Stage type
#define NONE        0x00
#define BUTTON      0x10
#define HW_SLD_0    0x20
#define HW_SLD_1    0x30
#define SW_SLD_0    0x40
#define SW_SLD_1    0x50
/////////////////////////////////////////////////////////////////////////////////
//--------------------------Define common flags--------------------------------// 
/////////////////////////////////////////////////////////////////////////////////
#define PAGE_0          0x00
#define PAGE_1          0x01
#define PAGE_2          0x02
/////////////////////////////////////////////////////////////////////////////////
//------------------------- Definitions of Bits--------------------------------//
/////////////////////////////////////////////////////////////////////////////////
#define _BIT0                           0x0001
#define _BIT1                           0x0002
#define _BIT2                           0x0004
#define _BIT3                           0x0008
#define _BIT4                           0x0010
#define _BIT5                           0x0020
#define _BIT6                           0x0040
#define _BIT7                           0x0080
#define _BIT8                           0x0100
#define _BIT9                           0x0200
#define _BIT10                          0x0400
#define _BIT11                          0x0800
#define _BIT12                          0x1000
#define _BIT13                          0x2000
#define _BIT14                          0x4000
#define _BIT15                          0x8000

/////////////////////////////////////////////////////////////////////////////////
//--------------------------Define Option function-----------------------------// 
/////////////////////////////////////////////////////////////////////////////////
#define LED_FUNCTION
#define MOBILE_INTERFERENCE
#define ENABLE_LOW_CONTACT

#define NON_INIT         0
#define PRE_RESTORT      1
#define FINISH_RESTORT   2



const sInitCapSReg asInitCapSReg[] = 
//const sInitCapSReg code asInitCapSReg[] = 
{
{ PAGE_1,  CAPS_PCR     ,0x0001},  
{ PAGE_1,  CAPS_PSR     ,0x0001},  
{ PAGE_1,  CAPS_PMR      ,0x0000},
{ PAGE_1,  CAPS_RTR      ,0x003F},
{ PAGE_1,  CAPS_CTR      ,0x000F},
{ PAGE_1,  CAPS_CRMR     ,0x0020},
{ PAGE_1,  CAPS_PDR      ,0x1FFF},
{ PAGE_1,  CAPS_DR       ,0x0050},
{ PAGE_1,  CAPS_S0CR    ,0x8030},
{ PAGE_1,  CAPS_S1CR    ,0x8070},
{ PAGE_1,  CAPS_S2CR    ,0x8091},
{ PAGE_1,  CAPS_S3CR    ,0x80B1},
{ PAGE_1,  CAPS_C0COR   ,0x68C0},
{ PAGE_1,  CAPS_C1COR   ,0x69C0},
{ PAGE_1,  CAPS_C3COR   ,0x68C7},
{ PAGE_1,  CAPS_C7COR   ,0x68C2},
{ PAGE_1,  CAPS_C9COR   ,0x68BF},
{ PAGE_1,  CAPS_C11COR   ,0x68BE},
{ PAGE_1,  CAPS_ICR0     ,0xFFFA},
{ PAGE_1,  CAPS_ICR1     ,0x0FFF},
{ PAGE_1,  CAPS_COER0    ,0xFFFF},
{ PAGE_1,  CAPS_COER1    ,0x03FF},
{ PAGE_1,  CAPS_CGCR     ,0x0001},
{ PAGE_1,  CAPS_LEDBR   ,0x0000},
{ PAGE_1,  CAPS_GPIODR   ,0x0000},
{ PAGE_1,  CAPS_GPIOOR   ,0x0000},
{ PAGE_1,  CAPS_GPIOMR   ,0x6000},
{ PAGE_1,  CAPS_GPIOLR   ,0x0003},
{ PAGE_1,  CAPS_GPIOER   ,0x0000},
{ PAGE_1,  CAPS_LEDCMR0  ,0xDDFF},
{ PAGE_1,  CAPS_LEDCMR1  ,0xDDDD},
{ PAGE_1,  CAPS_LEDCMR2  ,0xDDDD},
{ PAGE_1,  CAPS_LEDCMR3  ,0x0DDD},
{ PAGE_1,  CAPS_LEDRPR   ,0x3040},
{ PAGE_1,  CAPS_LEDBR    ,0x031F},
{ PAGE_1,  CAPS_LEDCGCR  ,0x0000},
{ PAGE_1,  CAPS_LEDPR0   ,0x3322},
{ PAGE_1,  CAPS_LEDPR1   ,0x3333},
{ PAGE_1,  CAPS_LEDPR2   ,0x3333},
{ PAGE_1,  CAPS_LEDPR3   ,0x0003},
{ PAGE_1,  CAPS_GPIOMSR  ,0x6003},
{ PAGE_0,  CAPS_S0DLR   ,0x8000},
{ PAGE_0,  CAPS_S0OHCR  ,0x0A00},
{ PAGE_0,  CAPS_S0OLCR  ,0x7000},
{ PAGE_0,  CAPS_S0SR    ,0xCF8F},
{ PAGE_0,  CAPS_S1DLR   ,0x8000},
{ PAGE_0,  CAPS_S1OHCR  ,0x0A00},
{ PAGE_0,  CAPS_S1OLCR  ,0x7000},
{ PAGE_0,  CAPS_S1SR    ,0xCF8F},
{ PAGE_0,  CAPS_S2DLR   ,0x8000},
{ PAGE_0,  CAPS_S2OHCR  ,0x0A00},
{ PAGE_0,  CAPS_S2OLCR  ,0x7000},
{ PAGE_0,  CAPS_S2SR    ,0xCF8F},
{ PAGE_0,  CAPS_S3DLR   ,0x8000},
{ PAGE_0,  CAPS_S3OHCR  ,0x0A00},
{ PAGE_0,  CAPS_S3OLCR  ,0x7000},
{ PAGE_0,  CAPS_S3SR    ,0xCF8F},
{ PAGE_0,  CAPS_SXCHAIER ,0x000F},
{ PAGE_0,  CAPS_SXCHRIER ,0x000F},
{ PAGE_0,  CAPS_SXCLAIER ,0x000F},
{ PAGE_0,  CAPS_SXCLRIER ,0x000F},
{ PAGE_1,  CAPS_GPIONPCR     ,0x1FFF},
{ PAGE_1,  CAPS_CFER     ,0xC000},
{ PAGE_1,  CAPS_PCR      ,0x3006}
};
/**************************************************************************
** "IT7230_SettingTable.c"                                               **
***************************************************************************/



#ifdef LED_FUNCTION
void IT7230_LEDStatus(WORD wLedStatus);
#endif

//void IT7230_CheckPage(BYTE page);
WORD IT7230_CapS_Read_Reg(BYTE page, BYTE addr_byte);   
void IT7230_CapS_Write_Reg(BYTE page, BYTE addr_byte, WORD data_word);
void IT7230_Init_CapS_Table(void);
WORD IT7230_GetKeyStatus(void);
void IT7230_Restort_Reg(void);
WORD IT7230_Check_ESD(void);
BYTE IT7230_Init_Status(void);
WORD IT7230_GetLowContactStatus(void);
WORD IT7230_LowContactCalibration(void);

/**************************************************************************
** "IT7230_TouchKey.h" ^                                                 **
***************************************************************************/















/**************************************************************************
** global variable                                                      **
***************************************************************************/
static const unsigned char kpTchKeyTable[] =    { 0, 1, 2, 3 };

static	pthread_mutex_t     keypad_mutex = PTHREAD_MUTEX_INITIALIZER;

static	uint8_t		gRegPage=0xFE;
static	uint16_t	gLastMultiKey=0;
static	uint16_t	gLastKey=0;
static	uint16_t	wLastTKeyStatus=0;
static	uint8_t		g_kpI2cPort = IIC_PORT_0;
/**************************************************************************
** "IT7230_TouchKey.c"                                                   **
***************************************************************************/
//static BYTE current_page = 0;
static BYTE Retry = 10;
static BYTE ITE7230InitState = 0;
static WORD wCAPS_PDR_BACKUP;
static WORD wCAPS_RTR_BACKUP;
static WORD wCAPS_CTR_BACKUP;
static WORD wCAPS_S2OHCR_CHECK;
static WORD wCAPS_PCR_CHECK;
static WORD wSTAGE_NUM;

#ifdef ENABLE_LOW_CONTACT
static BOOL bTKLowContactFlag = FALSE;
#endif

#ifdef LED_FUNCTION
static WORD wCAPS_GPIOOR_CHECK;
#endif
/**************************************************************************
** "IT7230_TouchKey.c"                                                   **
***************************************************************************/


/**************************************************************************
** private function                                                      **
***************************************************************************/
static uint8_t _checkSum(uint16_t sum)
{
	uint8_t	i;
	uint8_t	cnt=0;
	
	for(i=0; i<CFG_TOUCH_KEY_NUM; i++)
	{
		if( (sum>>i)&0x01 )	cnt++;
	}
	return cnt;
}

static void _resetDevice(void)
{
	//pull down iic_SCL for 4ms

	#ifdef	ENABLE_KEYPAD_DBG_MODE
	printf("[Keypad error] IT7235BR hangup, and reset keypad device.\n");
	#endif
	
	//set GPIO3 as mode0	
	ithGpioSetMode(2, ITH_GPIO_MODE0);
	ithGpioSetMode(3, ITH_GPIO_MODE0);
	
	//set output mode
	ithGpioSetOut(2);
	ithGpioSetOut(3);
	
	//set GPIO output 0
	ithGpioClear(2);
	ithGpioClear(3);
	
	//for 4ms
	usleep(8000);
	ithGpioSet(2);	
	ithGpioSet(3);
	usleep(100);
	
	//set GPIO3 as mode1(IIC mode)
	ithGpioSetMode(2, ITH_GPIO_MODE3);
	ithGpioSetMode(3, ITH_GPIO_MODE3);
	usleep(100);
	
	#ifdef	ENABLE_KEYPAD_DBG_MODE
	printf(" end of keypad reset flow\n");
	#endif
}

void _initTkGpioPin(void)
{
	ithGpioSetMode(TK_GPIO_PIN, ITH_GPIO_MODE1);
	ithGpioSetIn(TK_GPIO_PIN);
	ithGpioCtrlEnable(TK_GPIO_PIN, ITH_GPIO_PULL_ENABLE);
	ithGpioCtrlEnable(TK_GPIO_PIN, ITH_GPIO_PULL_UP);
	ithGpioEnable(TK_GPIO_PIN);	     
}

/**************************************************************************
** "IT7230_TouchKey.c"                                                   **
***************************************************************************/
/////////////////////////////////////////////////////////////////////////////
//----Return It7230 ITE7230InitState                                    ---//
/////////////////////////////////////////////////////////////////////////////
BYTE IT7230_Init_Status(void)
{
    return ITE7230InitState;
}
/////////////////////////////////////////////////////////////////////////////
static void IT7230_CheckPage(BYTE page)
{
	BYTE i;    
	BYTE wPage[2]; 
	
	#ifdef	ENABLE_KEYPAD_DBG_MODE
	printf("_CheckPage:page=%x,gPage=%x\n",page,gRegPage);
	#endif
	
    if ( page != gRegPage)
    {
        for (i = 0; i < Retry; i++)
        {
			#ifdef ENABLE_SAVE_RESTORE_ALL_INTR
			if( (IT7230_Init_Status() == FINISH_RESTORT) || (IT7230_Init_Status() == PRE_RESTORT))
			{
				//printf("	##1 disable all intr(%x) ##\n",page);
				portSAVEDISABLE_INTERRUPTS();
			}
			#endif
			
			wPage[0] = page;
			wPage[1] = 0;
			
		    if( mmpIicSendData(g_kpI2cPort, IIC_MASTER_MODE, IT7230_IIC_ADDR, CAPS_PSR, wPage, 2)==0 )
		    {
		    	#ifdef ENABLE_SAVE_RESTORE_ALL_INTR
		    	if( (IT7230_Init_Status() == FINISH_RESTORT) || (IT7230_Init_Status() == PRE_RESTORT))
		    	{
		    		portRESTORE_INTERRUPTS();
		    		//printf("	## restore all intr(%x) ##\n",page);
		    	}
		    	#endif
		    	
		        gRegPage = page;	
		        usleep(4000);	        
                break;
            }
            
            #ifdef ENABLE_SAVE_RESTORE_ALL_INTR
            if( (IT7230_Init_Status() == FINISH_RESTORT) || (IT7230_Init_Status() == PRE_RESTORT))
            {
            	portRESTORE_INTERRUPTS();
            	//printf("	## restore all intr(%x) ##\n",page);
            }
            #endif
            
            usleep(1000);
        }
	}
	//usleep(2000);
}
/////////////////////////////////////////////////////////////////////////////
//----Read IT7230 registers , IT7230_CheckPage() & retry 3 Timer-----------//
/////////////////////////////////////////////////////////////////////////////
WORD IT7230_CapS_Read_Reg(BYTE page, BYTE addr_byte)
{
	WORD data_word;
    BYTE i;
    BYTE buf[2];
    BYTE wBuf[1];
    
    #ifdef	ENABLE_KEYPAD_DBG_MODE
    printf("iic_rd:p=%d, a=%x\n",page,addr_byte);
    #endif
    
	IT7230_CheckPage(page);

    for (i = 0; i < Retry; i++)
    {
		#ifdef ENABLE_SAVE_RESTORE_ALL_INTR
		if( (IT7230_Init_Status() == FINISH_RESTORT) || (IT7230_Init_Status() == PRE_RESTORT))
		{
			//printf("	##3 disable all intr(%x,%x) ##\n",page,addr_byte);
			portSAVEDISABLE_INTERRUPTS();
		}
		#endif
    
    	wBuf[0] = addr_byte;			    
	    if( mmpIicReceiveDataEx(g_kpI2cPort, IIC_MASTER_MODE, IT7230_IIC_ADDR, wBuf, 1, buf, 2)==0 )
	    {
	    	#ifdef ENABLE_SAVE_RESTORE_ALL_INTR
	    	if( (IT7230_Init_Status() == FINISH_RESTORT) || (IT7230_Init_Status() == PRE_RESTORT))
	    	{
				portRESTORE_INTERRUPTS();
				//printf("	## restore all intr(%x,%x) ##\n",page,addr_byte);
			}
			#endif
			
	    	data_word = (((WORD)buf[1]<<8)&0xFF00) + ((WORD)buf[0]&0xFF);
            return data_word;
            break;
        }
        
        #ifdef ENABLE_SAVE_RESTORE_ALL_INTR
        if( (IT7230_Init_Status() == FINISH_RESTORT) || (IT7230_Init_Status() == PRE_RESTORT))
        {
        	portRESTORE_INTERRUPTS();
        	//printf("	## restore all intr(%x,%x) ##\n",page,addr_byte);
        }
        #endif
        
        usleep(5000);
    }
}
/////////////////////////////////////////////////////////////////////////////
//----Write IT7230 registers ,IT7230_CheckPage() & retry 3 Timer-----------//
/////////////////////////////////////////////////////////////////////////////
void IT7230_CapS_Write_Reg(BYTE page, BYTE addr_byte, WORD data_word)
{
  	BYTE i = 0x00;
	BYTE wPage[2];	
	
	IT7230_CheckPage(page);

    for (i = 0; i < Retry; i++)
    {
		#ifdef ENABLE_SAVE_RESTORE_ALL_INTR
		if( (IT7230_Init_Status() == FINISH_RESTORT) || (IT7230_Init_Status() == PRE_RESTORT))
		{
			//printf("	##2 disable all intr(%x,%x,%x) ##\n",page,addr_byte,data_word);
			portSAVEDISABLE_INTERRUPTS();
		}
		#endif
		
    	wPage[0] = (BYTE)(data_word&0xFF);	
    	wPage[1] = (BYTE)((data_word>>8)&0xFF);	
    		
        if( mmpIicSendData(g_kpI2cPort, IIC_MASTER_MODE, IT7230_IIC_ADDR, addr_byte, wPage, 2)==0 )
    	{
	    	#ifdef ENABLE_SAVE_RESTORE_ALL_INTR
	    	if( (IT7230_Init_Status() == FINISH_RESTORT) || (IT7230_Init_Status() == PRE_RESTORT))
	    	{
				portRESTORE_INTERRUPTS();
				//printf("	## restore all intr(%x,%x,%x) ##\n",page,addr_byte,data_word);
			}
			#endif
            break;
        }
	    #ifdef ENABLE_SAVE_RESTORE_ALL_INTR
	    if( (IT7230_Init_Status() == FINISH_RESTORT) || (IT7230_Init_Status() == PRE_RESTORT))
	    {
			portRESTORE_INTERRUPTS();
			//printf("	## restore all intr(%x,%x,%x) ##\n",page,addr_byte,data_word);
		}
		#endif
        usleep(5000);
    }

	//When the "reset" bit of PCR register is 1, current_page is set to 0.
	if ( (CAPS_PCR == addr_byte) && (1 == page) && (data_word & 0x0001))
		gRegPage = 0;

}
/////////////////////////////////////////////////////////////////////////////
//----Write IT7230 Initial Table 
/////////////////////////////////////////////////////////////////////////////
void IT7230_Init_CapS_Table(void)
{
    BYTE i = 0x00;
    WORD tableCnt = (sizeof(asInitCapSReg)/sizeof(sInitCapSReg));
    
    //printf("\n");

    while (i < tableCnt)
    { 
    	//printf("iic_wt[%x]:%02x,%02x,%04x\n", i, asInitCapSReg[i].page, asInitCapSReg[i].reg, asInitCapSReg[i].value);
        IT7230_CapS_Write_Reg(asInitCapSReg[i].page, asInitCapSReg[i].reg, asInitCapSReg[i].value);

        //*********************************************************************//
        //---Backup these Registers for "IT7230_Check_ESD()",It is necessary---//
        switch (asInitCapSReg[i].reg){
            
		        case CAPS_RTR:
                    wCAPS_RTR_BACKUP = asInitCapSReg[i].value ;
                    break;
                    
                case CAPS_CTR:
                    wCAPS_CTR_BACKUP = asInitCapSReg[i].value ;
                    break;
                    
                case CAPS_PDR:
                    wCAPS_PDR_BACKUP = asInitCapSReg[i].value ;
                    break;
                    
                case CAPS_PCR:
                    wCAPS_PCR_CHECK = asInitCapSReg[i].value ;
                    break;
                default:
                    break;
        }

    
        //********************************************************************//
        
        i++;
        if (1 == i)
        {
            usleep(1000);//Delay 1ms.
        }
    }
}
/////////////////////////////////////////////////////////////////////////////
//----Restort Reg after the "IT7230_Init()" function ,It is necessary---//
/////////////////////////////////////////////////////////////////////////////
void IT7230_Restort_Reg(void)
{
    BYTE i;
	//Restore RTR/CTR/CFER
    if(IT7230_Init_Status() == PRE_RESTORT)
    {
        IT7230_CapS_Write_Reg(PAGE_1, CAPS_PDR, wCAPS_PDR_BACKUP);
        IT7230_CapS_Write_Reg(PAGE_1, CAPS_RTR, wCAPS_RTR_BACKUP);
        IT7230_CapS_Write_Reg(PAGE_1, CAPS_CTR, wCAPS_CTR_BACKUP);
    	IT7230_CapS_Write_Reg(PAGE_1, CAPS_CFER, 0xC000);
        IT7230_CapS_Read_Reg(PAGE_0, CAPS_SIR);

        
        //---To detect AC ON contact,  ---//
        #ifdef ENABLE_LOW_CONTACT
        if(!bTKLowContactFlag)
        {
            for( i=0 ; i<wSTAGE_NUM ; i++)
            {
                if( i!=2 )
                IT7230_CapS_Write_Reg(PAGE_0, 0x3A+(i*0x10), 0x0460);//Low Clamp
            }

            IT7230_CapS_Write_Reg(PAGE_0, CAPS_SXCLAIER, 0x007B);//Low Contact Eanble
        }
        #endif
        //----------------------------------------------------------//
        wCAPS_S2OHCR_CHECK=(IT7230_CapS_Read_Reg(PAGE_0, CAPS_S2HTR));
        
        #ifdef	ENABLE_KEYPAD_DBG_MODE
        printf("wCAPS_S2OHCR_CHECK:%04X\n",wCAPS_S2OHCR_CHECK);
        #endif
        
        ITE7230InitState  = FINISH_RESTORT;
    }
}

/* If the return value is non-zero, this value represent the time interval
 * prior to calling IT7230_Restort_Reg. The system program must call
 * the function IT7230_Restort_Reg after passing through the time by the
 * return value(ms) of this function.
 * If the return value is zero, that means this function has an error and
 * the system program doesn't have to call the function IT7230_Restort_Reg.
 *
 * An Example for calling this function.
 * 
 * .....
 * tmrRestoreTouchKeyStatus = IT7230_Init();
 * if(tmrRestoreTouchKeyStatus) {
 * 	Set flagDoRestoreTouchKeyStatus.
 * 	Set a timer with tmrRestoreTouchKeyStatus.
 * } else {
 * 	Clear flagDoRestoreTouchKeyStatus.
 * }
 * // to do system functions
 * ....
 * if( flagDoRestoreTouchKeyStatus && (!tmrRestoreTouchKeyStatus))
 * 	IT7230_Restort_Reg();
 * ....
*/ 
WORD IT7230_Init(void)
{
	WORD wDecimationRate = 0;
	
    ITE7230InitState = NON_INIT;
    
	// Initialize the global variables.
	wSTAGE_NUM          = 0;
	wCAPS_S2OHCR_CHECK  = 0;
	wCAPS_PCR_CHECK     = 0;
	wCAPS_PDR_BACKUP    = 0;
	wCAPS_RTR_BACKUP    = 0;
	wCAPS_CTR_BACKUP    = 0;

    
    #ifdef LED_FUNCTION
	wCAPS_GPIOOR_CHECK  = 0;
    #endif

	//IT7230_I2C_Reset();
	_resetDevice();

    //*********************************************************************//
    //Check I2C address is or bus handle, if error occurs, it is not necessary to init setting table..
	//if(!IT7230_I2C_Device_Addr_Check())//non_Ack
	//{
	//	return 0;
	//}
	//*********************************************************************//
	
    IT7230_Init_CapS_Table();
    //while(1);

    // Check Init_CapS_Table are written into success.   
	if( (IT7230_CapS_Read_Reg(PAGE_1, CAPS_PDR) != wCAPS_PDR_BACKUP )||
		(IT7230_CapS_Read_Reg(PAGE_1, CAPS_CTR) != wCAPS_CTR_BACKUP )||
		(IT7230_CapS_Read_Reg(PAGE_1, CAPS_RTR) != wCAPS_RTR_BACKUP )||
		(IT7230_CapS_Read_Reg(PAGE_1, CAPS_PCR) != wCAPS_PCR_CHECK	))
    {

        #ifdef	ENABLE_KEYPAD_DBG_MODE
        printf("Init error.1\n");
        #endif

        return 0;
    }


	wDecimationRate = (( wCAPS_PCR_CHECK >> 8 )& 0x0F);
	wSTAGE_NUM      = (( wCAPS_PCR_CHECK >> 12 )+1);

    //*********************************************************************//
    //---force Calibration
    IT7230_CapS_Write_Reg(PAGE_1, CAPS_RTR, 0x0000);
    IT7230_CapS_Write_Reg(PAGE_1, CAPS_CTR, 0x0000);
    IT7230_CapS_Write_Reg(PAGE_1, CAPS_PDR, 0x1FFF);
	IT7230_CapS_Write_Reg(PAGE_1, CAPS_CFER,0x4000);

    ITE7230InitState  = PRE_RESTORT;
    //*********************************************************************//
    //---Waiting for Calibration Tune CDC,It is necessary.
    
	if( wDecimationRate == 0x00 )
	{
		return (550);
	}
	else if( wDecimationRate == 0x04 )
	{
		return (350);
	}
	else
	{
		return (250);
	}

    #ifdef	ENABLE_KEYPAD_DBG_MODE
    printf("Init error.2\n");
    #endif
    
	return 0;
    
}

/////////////////////////////////////////////////////////////////////////////////
//----Please check the function about the 1~2 seconds----//
/////////////////////////////////////////////////////////////////////////////

/* If the return value is non-zero, this value represent the time interval
 * prior to calling IT7230_Restort_Reg(). The system program must call the
 * function IT7230_Restort_Reg() after passing through the time by the
 * return value(ms) of this function.
 * If the return value is zero, that means the function IT7230_Init hasn't
 * been called or it returns zero, so the system program doesn't have to call
 * the function IT7230_Restort_Reg.
 * The system program designer can see the previous example to learn how to
 * use it.
 *
*/ 
WORD IT7230_Check_ESD(void)
{

    #ifdef MOBILE_INTERFERENCE
	BYTE    i;
    BOOL    uErrorFlag = FALSE;
	WORD    Current_temp;
    static  BYTE uErrorCount = 0;
    #endif
    
        if( (IT7230_CapS_Read_Reg(PAGE_1, CAPS_PCR)& 0x0006)!= 0x0006 ) 
    	{
    	    #ifdef	ENABLE_KEYPAD_DBG_MODE
            printf("CAPS_PCR:%04X\n",IT7230_CapS_Read_Reg(PAGE_1, CAPS_PCR));
            #endif
    		return IT7230_Init();
    	}   

        if( IT7230_Init_Status() == FINISH_RESTORT )
        {
            if ( wCAPS_S2OHCR_CHECK != IT7230_CapS_Read_Reg(PAGE_0, CAPS_S2HTR)) 
        	{
                #ifdef	ENABLE_KEYPAD_DBG_MODE
                printf("wCAPS_S2OHCR_CHECK:%04X/CAPS_S2HTR:%04X\n",wCAPS_S2OHCR_CHECK,IT7230_CapS_Read_Reg(PAGE_0, CAPS_S2HTR));
                #endif
        		return IT7230_Init();

        	}
            
        }

        if( (IT7230_Init_Status() == FINISH_RESTORT) || (IT7230_Init_Status() == PRE_RESTORT))
        {
            #ifdef LED_FUNCTION
        	if ( wCAPS_GPIOOR_CHECK != IT7230_CapS_Read_Reg(PAGE_1, CAPS_GPIOOR)) 
        	{
        		IT7230_CapS_Write_Reg(PAGE_1, CAPS_GPIOOR, wCAPS_GPIOOR_CHECK);
        	}
            #endif
        }
        
	   #ifdef MOBILE_INTERFERENCE
        if( IT7230_Init_Status() == FINISH_RESTORT )
        {
            for( i = 0 ; i < wSTAGE_NUM ; i++ )
    		{
    			Current_temp = IT7230_CapS_Read_Reg(PAGE_0, CAPS_S0CDC_CMP+0x10*i);
                if( (Current_temp == 0x0000 )||( Current_temp== 0xFFFF ))
    			{
    				 uErrorCount++;
                     uErrorFlag = TRUE;
                     break ;
    			}
    		}

            if(uErrorFlag)
            {
                if( uErrorCount >= 2 )
                {
                    uErrorCount = 0;

                    return IT7230_Init();
                }
            }
            else
            {
               uErrorCount = 0; 
            }
        }
		
        #endif

    

    return 0;

}
/////////////////////////////////////////////////////////////////////////////
//----Setting the "CAPS_GPIOOR" Reg to Change LED status ----                                                             //
/////////////////////////////////////////////////////////////////////////////
#ifdef LED_FUNCTION
void IT7230_LEDStatus(WORD wLedStatus)
{
   if( ITE7230InitState == FINISH_RESTORT || ITE7230InitState == PRE_RESTORT )
   {
       if(IT7230_CapS_Read_Reg( PAGE_1,  CAPS_GPIOOR) != wLedStatus)
       {
    	  IT7230_CapS_Write_Reg( PAGE_1,  CAPS_GPIOOR, wLedStatus);
       } 
       
       wCAPS_GPIOOR_CHECK = wLedStatus;
   }
}
#endif
/////////////////////////////////////////////////////////////////////////////
//----Get the Touch Key Status,offer polling and interrupt mode---// 
/////////////////////////////////////////////////////////////////////////////
WORD IT7230_GetKeyStatus(void)
{	
    WORD wTKeyStatus =0 ;
    unsigned int INTPin=0;

    if( IT7230_Init_Status() == FINISH_RESTORT )
    {
        INTPin = ithGpioGet(TK_GPIO_PIN);
        //printf("intr=%x, mask=%x\n",INTPin,TK_GPIO_MASK);
        if( !(INTPin & TK_GPIO_MASK) )
        {
        	//check if PCR is the same as #line:574 PCR value
        	if( (IT7230_CapS_Read_Reg(PAGE_1, CAPS_PCR)&0x3006)!= 0x3006 ) 
        	{
        		uint16_t SleepTime;
        		
        		printf("[IT7230 WARNING]: PCR!=0x3006,(%x)\n",IT7230_CapS_Read_Reg(PAGE_1, CAPS_PCR) );
        		SleepTime = IT7230_Init();
        		printf("[IT7230 WARNING]: reinit time = %d\n",SleepTime);
    			if( SleepTime != 0 )
				{
					printf("sleep for restoring register\n");
					usleep(SleepTime*1000);
    			    IT7230_Restort_Reg();
    			    SleepTime = 0 ;
    			    #ifdef	ENABLE_KEYPAD_DBG_MODE
				    printf("ReInitial_IT7230_success\n");
    			    #endif
				}
				printf("[IT7230 WARNING]: reinit success\n");
        	}
        	//printf("int,read = %x,%x\n",PAGE_0,CAPS_SXCHSR);
            wTKeyStatus = IT7230_CapS_Read_Reg(PAGE_0, CAPS_SXCHSR);
            
            if(wLastTKeyStatus!=wTKeyStatus)  	wLastTKeyStatus = wTKeyStatus;
        }
        else
        {
        	if(wLastTKeyStatus)
        	{
        		wTKeyStatus = wLastTKeyStatus;
        	}
        }
    }
    #ifdef	ENABLE_KEYPAD_DBG_MODE
    printf("IT7230_Key = %x\n",wTKeyStatus);
    #endif
    
    return wTKeyStatus ;
    
}

#ifdef ENABLE_LOW_CONTACT
/////////////////////////////////////////////////////////////////////////////
WORD IT7230_GetLowContactStatus(void)
{
    WORD wLowContactStatus = 0;

    if( !bTKLowContactFlag )
    {
        if( IT7230_Init_Status() == FINISH_RESTORT )
        {
            wLowContactStatus = IT7230_CapS_Read_Reg(PAGE_0, CAPS_SXCLSR);
        }
    }
    return wLowContactStatus ;
}
/////////////////////////////////////////////////////////////////////////////
WORD IT7230_LowContactCalibration(void) 
{

    WORD wDecimationRate;
    BYTE i;
    for( i=0; i< wSTAGE_NUM ; i++)
    {
        IT7230_CapS_Write_Reg(PAGE_0, 0x3A+(i*0x10), 0x7000);
    }
        
    IT7230_CapS_Write_Reg(PAGE_0, CAPS_SXCLAIER, 0x0000);//DISENABLE Low Contact
                   
    wDecimationRate = (( wCAPS_PCR_CHECK >> 8 )& 0x0F);

    //*********************************************************************//
    //---force Calibration
    IT7230_CapS_Write_Reg(PAGE_1, CAPS_RTR, 0x0000);
    IT7230_CapS_Write_Reg(PAGE_1, CAPS_CTR, 0x0000);
    IT7230_CapS_Write_Reg(PAGE_1, CAPS_PDR, 0x1FFF);
    IT7230_CapS_Write_Reg(PAGE_1, CAPS_CFER,0x4000);

    //*********************************************************************//
    bTKLowContactFlag = TRUE ; 
    ITE7230InitState = PRE_RESTORT;
	
    //---Waiting for Calibration Tune CDC,It is necessary.
	if( wDecimationRate == 0x00 )
	{
		return (550);
	}
	else if( wDecimationRate == 0x04 )
	{
		return (350);
	}
	else
	{
		return (250);
	}

	return 0;

}
#endif
/**************************************************************************
** "IT7230_TouchKey.c"                                                   **
***************************************************************************/















/**************************************************************************
** public function(keypad API)                                           **
***************************************************************************/

int itpKeypadProbe(void)
{
    unsigned int i;
    uint16_t tk_value;
    uint8_t ChkSum;
    
    #ifdef	ENABLE_KEYPAD_DBG_MODE
    printf("itpKeyPadProbe_IT7230\n");
    #endif
    
    pthread_mutex_lock(&keypad_mutex);
    
    tk_value = IT7230_GetKeyStatus();
    
    pthread_mutex_unlock(&keypad_mutex);
    
    ChkSum = _checkSum(tk_value);
       
    if( gLastKey != tk_value )
    {
		if( tk_value )
		{
			#ifdef LED_FUNCTION
			IT7230_LEDStatus(0x0003);
			#endif
		}
		else
		{
			#ifdef LED_FUNCTION
			IT7230_LEDStatus(0x0000);
			#endif
		}
       
		gLastKey = tk_value ;

		#ifdef	ENABLE_KEYPAD_DBG_MODE
		printf("tk_value:%04X\n",tk_value);
		#endif
    }
       
    if(ChkSum==0)	return -1;
    
    if(ChkSum>1)
    {
    	if(tk_value!=gLastMultiKey)
    	{
    		printf("[KEYPAD]warning:: multi-key, skip it(tk_value=%x, chk=%d)\n",tk_value, ChkSum);
    		gLastMultiKey = tk_value;
    	}
    	return -1;
    }
    
	#ifdef	ENABLE_KEYPAD_DBG_MODE
    printf("[KEYPAD]:got key=%04x\n",tk_value);
    #endif
    
    for (i = 0; i < CFG_TOUCH_KEY_NUM; i++)
    {    	
        if( (tk_value>>i) & 0x1 )
        {        	
            return (int)kpTchKeyTable[(uint8_t)i];
        }
    }
}

void itpKeypadInit(void)
{	
	uint16_t SleepTime;	
	
	pthread_mutex_lock(&keypad_mutex);
	
	_initTkGpioPin();
	
	SleepTime = IT7230_Init();
    if( SleepTime != 0 )
	{
		usleep(SleepTime*1000);
        IT7230_Restort_Reg();
        SleepTime = 0 ;
        #ifdef	ENABLE_KEYPAD_DBG_MODE
	    printf("Initial_IT7230_success\n");
        #endif
	}
	pthread_mutex_unlock(&keypad_mutex);
}

int itpKeypadGetMaxLevel(void)
{
    return CFG_TOUCH_KEY_NUM;
}
