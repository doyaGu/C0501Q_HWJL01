/****************************************************************************
 *
 *            Copyright (c) 2003-2008 by HCC Embedded
 *
 * This software is copyrighted by and is the sole property of
 * HCC.  All rights, title, ownership, or other interests
 * in the software remain the property of HCC.  This
 * software may only be used in accordance with the corresponding
 * license agreement.  Any unauthorized use, duplication, transmission,
 * distribution, or disclosure of this software is expressly forbidden.
 *
 * This Copyright notice may not be removed or modified without prior
 * written consent of HCC.
 *
 * HCC reserves the right to modify this software without notice.
 *
 * HCC Embedded
 * Budapest 1133
 * Vaci ut 110
 * Hungary
 *
 * Tel:  +36 (1) 450 1302
 * Fax:  +36 (1) 450 1303
 * http: www.hcc-embedded.com
 * email: info@hcc-embedded.com
 *
 ***************************************************************************/

#include "ramdrv_f.h"

#define RAMDRV_CNT 1

static char ramdrv0[] =
{
#include "ramdisk.inc"
};

#define RAMDRV0_SIZE (sizeof(ramdrv0))          /* defintion for size of ramdrive0 */

#define MEM_LONG_ACCESS 1 /* set this value to 1 if 32bit access available */

typedef struct {
  char *ramdrv;
  unsigned long maxsector;
  int use;
  F_DRIVER *driver;
} t_RamDrv;

static F_DRIVER t_drivers[RAMDRV_CNT];

static t_RamDrv RamDrv[RAMDRV_CNT] = {
  { ramdrv0, (RAMDRV0_SIZE/F_DEF_SECTOR_SIZE), 0, &t_drivers[0] },
};


/****************************************************************************
 *
 * ram_readmultiplesector
 *
 * read multiple sectors from ramdrive
 *
 * INPUTS
 *
 * driver - driver structure
 * data - data pointer where to store data
 * sector - where to read data from
 * cnt - number of sectors to read
 *
 * RETURNS
 *
 * error code or zero if successful
 *
 ***************************************************************************/

static int ram_readmultiplesector(F_DRIVER *driver,void *data, unsigned long sector, int cnt) {
long len;
char *d=(char*)data;
char *s;
t_RamDrv *p=(t_RamDrv *)(driver->user_ptr);

   if (sector>=p->maxsector) return RAM_ERR_SECTOR;

   s=p->ramdrv;
   s+=sector*F_DEF_SECTOR_SIZE;
   len=F_DEF_SECTOR_SIZE*(long)cnt;

#if MEM_LONG_ACCESS
   if ((!(len&3)) && (!(((long)d)&3)) && (!(((long)s)&3))) {
      long *dd=(long*)d;
      long *ss=(long*)s;
      len>>=2;
      while (len--) {
         *dd++=*ss++;
      }

      return RAM_NO_ERROR;
   }
#endif

   while (len--) {
      *d++=*s++;
   }

   return RAM_NO_ERROR;
}

/****************************************************************************
 * Read one sector
 ***************************************************************************/

static int ram_readsector(F_DRIVER *driver,void *data, unsigned long sector) {
  return ram_readmultiplesector(driver,data,sector,1);
}

/****************************************************************************
 *
 * ram_writemultiplesector
 *
 * write multiple sectors into ramdrive
 *
 * INPUTS
 *
 * driver - driver structure
 * data - data pointer
 * sector - where to write data
 * cnt - number of sectors to write
 *
 * RETURNS
 *
 * error code or zero if successful
 *
 ***************************************************************************/

static int ram_writemultiplesector(F_DRIVER *driver,void *data, unsigned long sector, int cnt) {
long len;
char *s=(char*)data;
char *d;
t_RamDrv *p=(t_RamDrv *)(driver->user_ptr);

   if (sector>=p->maxsector) return RAM_ERR_SECTOR;

   d=p->ramdrv;
   d+=sector*F_DEF_SECTOR_SIZE;
   len=F_DEF_SECTOR_SIZE*(long)cnt;

#if MEM_LONG_ACCESS
   if ((!(len&3)) && (!(((long)d)&3)) && (!(((long)s)&3))) {
      long *dd=(long*)d;
      long *ss=(long*)s;
      len>>=2;
      while (len--) {
         *dd++=*ss++;
      }
      return RAM_NO_ERROR;
   }
#endif

   while (len--) {
      *d++=*s++;
   }

   return RAM_NO_ERROR;
}

/****************************************************************************
 * Write one sector
 ***************************************************************************/

static int ram_writesector(F_DRIVER *driver,void *data, unsigned long sector) {
  return ram_writemultiplesector(driver,data,sector,1);
}

/****************************************************************************
 *
 * ram_getphy
 *
 * determinate ramdrive physicals
 *
 * INPUTS
 *
 * driver - driver structure
 * phy - this structure has to be filled with physical information
 *
 * RETURNS
 *
 * error code or zero if successful
 *
 ***************************************************************************/

static int ram_getphy(F_DRIVER *driver,F_PHY *phy)
{
	t_RamDrv *p=(t_RamDrv *)(driver->user_ptr);
	phy->number_of_sectors=p->maxsector;

	return RAM_NO_ERROR;
}

/****************************************************************************
 *
 * ram_release
 *
 * Releases a drive
 *
 * INPUTS
 *
 * driver_param - driver parameter
 *
 ***************************************************************************/

static void ram_release (F_DRIVER *driver)
{
	t_RamDrv *p=(t_RamDrv *)(driver->user_ptr);
	if (p>=RamDrv && p<RamDrv+RAMDRV_CNT) p->use=0;
}

/****************************************************************************
 *
 * ram_initfunc
 *
 * this init function has to be passed for highlevel to initiate the
 * driver functions
 *
 * INPUTS
 *
 * driver_param - driver parameter
 *
 * RETURNS
 *
 * driver structure pointer
 *
 ***************************************************************************/

F_DRIVER *ram_initfunc(unsigned long driver_param)
{
   t_RamDrv *p;
   unsigned int i;

   if (driver_param==F_AUTO_ASSIGN)
   {
     p=RamDrv;
     for (i=0;i<RAMDRV_CNT && p->use;i++,p++);
   }
   else
   {
     p=RamDrv+driver_param;
   }

   if (p<RamDrv || p>=RamDrv+RAMDRV_CNT) return 0;
   if (p->use) return 0;

   (void)memset (p->driver,0,sizeof(F_DRIVER));

   p->driver->readsector=ram_readsector;
   p->driver->writesector=ram_writesector;
   p->driver->readmultiplesector=ram_readmultiplesector;
   p->driver->writemultiplesector=ram_writemultiplesector;
   p->driver->getphy=ram_getphy;
   p->driver->release=ram_release;
   p->driver->user_ptr=p;
   p->use=1;

   return p->driver;
}

/******************************************************************************
 *
 *  End of ramdrv.c
 *
 *****************************************************************************/

