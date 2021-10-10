#ifndef ITE_RTL8304MB_H
#define ITE_RTL8304MB_H

#include "ite/itp.h"

#ifdef __cplusplus
extern "C" {
#endif


#define rtl8304mb_read_mode			phy_read_mode
#define rtl8304mb_linkIntrHandler	phy_linkIntrHandler
#define rtl8304mb_get_link_status	phy_get_link_status


void PhyInit(void);
int rtl8304mb_link_port3(void);
int rtl8304mb_read_mode(int* speed, int* duplex);
void rtl8304mb_linkIntrHandler(unsigned int pin, void *arg);

int rtl8304mb_mib_dump(int port);
uint32_t rtl8304mb_get_link_status(void);


#ifdef __cplusplus
}
#endif

#endif // ITE_RTL8304MB_H
