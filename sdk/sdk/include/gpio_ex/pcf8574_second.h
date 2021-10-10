#ifndef __PCF8574_SECOND_H__ 
#define __PCF8574_SECOND_H__ 
/****************************************************************************/ 
/*																			*/ 
/*				Copyright (C) 2000.  GENESIS MICROCHIP INC.					*/ 
/*		All rights reserved.  No part of this program may be reproduced.	*/ 
/*==========================================================================*/ 
/*																			*/ 
/* MODULE:      PCF8574.h                                                  	*/ 
/*																			*/ 
/* USAGE:       Header file for module PCF8574.c							*/ 
/*																			*/ 
/****************************************************************************/ 
 
/****************************************************************************/ 
/*	G L O B A L    D E F I N I T I O N										*/ 
/****************************************************************************/ 

 
#define BIT0		0x01
#define BIT1		0x02
#define BIT2		0x04
#define BIT3		0x08
#define BIT4		0x10
#define BIT5		0x20
#define BIT6		0x40
#define BIT7		0x80


 
#define	P00		BIT0 
#define	P01		BIT1 
#define	P02		BIT2 
#define	P03		BIT3 
#define	P04		BIT4 
#define	P05		BIT5 
#define	P06		BIT6 
#define	P07		BIT7 

 
/****************************************************************************/ 
/*	G L O B A L    F U N C T I O N    P R O T O T Y P E S					*/ 
/****************************************************************************/ 
void IOExpander_WritePort_2	(uint8_t W_Data); 
void IOExpander_SetInPortPin_2(uint8_t W_PinNum);
void IOExpander_SetPortPin_2	(uint8_t W_PinNum); 
void IOExpander_SetOutPortPin_2(uint8_t W_PinNum) ;
void IOExpander_ClrPortPin_2	(uint8_t W_PinNum); 
uint8_t IOExpander_ReadPortPin_2	(uint8_t W_PinNum); 
uint8_t IOExpander_ReadPort_2	(void);
uint8_t IOExpander_GetInputStatus_2(void); 
void IOExpanderDriver_initial_2(void);
#endif 

