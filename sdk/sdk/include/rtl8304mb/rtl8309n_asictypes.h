﻿#ifndef _RTL8309N_ASICTYPES_H
#define _RTL8309N_ASICTYPES_H

//#include <rtl_types.h>

#define RTL8304MB

#ifdef RTL8309M
#define RTL8309M_PORT0  RTL8309N_PORT0
#define RTL8309M_PORT1  RTL8309N_PORT1 
#define RTL8309M_PORT2  RTL8309N_PORT2
#define RTL8309M_PORT3  RTL8309N_PORT3
#define RTL8309M_PORT4  RTL8309N_PORT4
#define RTL8309M_PORT5  RTL8309N_PORT5
#define RTL8309M_PORT6  RTL8309N_PORT6
#define RTL8309M_PORT7  RTL8309N_PORT7 
#define RTL8309M_PORT8  RTL8309N_PORT8
#endif

#ifdef RTL8304MB
#define RTL8304MB_PORT0  RTL8309N_PORT0
#define RTL8304MB_PORT1  RTL8309N_PORT3 
#define RTL8304MB_PORT2  RTL8309N_PORT4
#define RTL8304MB_PORT3  RTL8309N_PORT8
#endif

#define SETMSKBIT(x,y) (x |= (uint32)0x1 << y) 

#define GETMSKBIT(x,y) ((x &((uint32)0x1 << y)) ? TRUE : FALSE)

#endif
