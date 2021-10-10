#include "usb/config.h"

sem_t* usb_create_sem(int cnt)
{
    sem_t* x = malloc(sizeof(sem_t)); 
    sem_init(x, 0, cnt); 
    return x;
}


