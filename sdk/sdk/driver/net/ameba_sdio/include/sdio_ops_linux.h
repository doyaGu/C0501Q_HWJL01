#ifndef __SDIO_OPS_LINUX_H__
#define __SDIO_OPS_LINUX_H__

#include <linux/mmc/sdio_func.h>  // Irene Lin
#include <sdio_ops.h>
extern void *rtw_sdio_get_drvdata(struct sdio_func *func);
extern int rtw_sdio_set_drvdata(struct sdio_func *func, void *data);
extern int rtw_sdio_set_block_size(struct sdio_func *func, unsigned int blksz);
#endif
