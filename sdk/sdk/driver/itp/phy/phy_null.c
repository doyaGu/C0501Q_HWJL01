#include "ite/ite_mac.h"


void
PhyInit(int ethMode)
{
    return;
}

int phy_read_mode(int* speed, int* duplex)
{
    (*duplex) = DUPLEX_FULL;
    (*speed) = SPEED_100;

    return 0; // 0 means link up
}
ㄋ
uint32_t phy_get_link_status(void)
{
    return 1;
}


/**
* Check interrupt status for link change. 
* Call from mac driver's internal ISR for phy's interrupt.
*/
int(*itpPhyLinkChange)(void) = NULL;
/**
* Replace mac driver's ISR for phy's interrupt. 
*/
ITHGpioIntrHandler itpPhylinkIsr = NULL;
/**
* Returns 0 if the device reports link status up/ok 
*/
int(*itpPhyReadMode)(int* speed, int* duplex) = phy_read_mode;
/**
* Get link status.
*/
uint32_t(*itpPhyLinkStatus)(void) = phy_get_link_status;


