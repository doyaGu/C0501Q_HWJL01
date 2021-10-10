/*
 *  linux/drivers/mmc/core/sdio_bus.c
 *
 *  Copyright 2007 Pierre Ossman
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * SDIO function driver model
 */

#include <linux/os.h>
#include <linux/err.h>

#include <linux/mmc/card.h>
#include <linux/mmc/host.h>
#include <linux/mmc/sdio_func.h>
#include <linux/mmc/sdio.h>
#include <linux/mmc/errorno.h>

#include "sdio_cis.h"
#include "sdio_bus.h"
#include "debug.h"


/*
 * We have a per-function "registered driver" list.
 */
LIST_HEAD(sdio_driver_list);

static const struct sdio_device_id *sdio_match_one(struct sdio_func *func,
	const struct sdio_device_id *id)
{
	if (id->class != (__u8)SDIO_ANY_ID && id->class != func->class)
		return NULL;
	if (id->vendor != (__u16)SDIO_ANY_ID && id->vendor != func->vendor)
		return NULL;
	if (id->device != (__u16)SDIO_ANY_ID && id->device != func->device)
		return NULL;
	return id;
}

static const struct sdio_device_id *sdio_match_device(struct sdio_func *func,
	struct sdio_driver *sdrv)
{
	const struct sdio_device_id *ids;

	ids = sdrv->id_table;

	if (ids) {
		while (ids->class || ids->vendor || ids->device) {
			if (sdio_match_one(func, ids))
				return ids;
			ids++;
		}
	}

	return NULL;
}

static int sdio_bus_probe(struct sdio_func *func)
{
    struct mmc_card *card = func->card;
    struct list_head *tmp;
	struct sdio_driver *drv;
	const struct sdio_device_id *id = NULL;
	int ret;

    for (tmp=sdio_driver_list.next; tmp!=&sdio_driver_list;) {
		drv = list_entry(tmp, struct sdio_driver, driver_list);
		tmp = tmp->next;

		id = sdio_match_device(func, drv);
		if (id)
			break;
   	}

	if (!id)
		return -ENODEV;


	/* Set the default block size so the driver is sure it's something
	 * sensible. */
	sdio_claim_host(func);
	ret = sdio_set_block_size(func, 0);
	sdio_release_host(func);
	if (ret)
		goto disable_runtimepm;

	ret = drv->probe(func, id);
	if (ret)
		goto disable_runtimepm;

    func->drv = drv;
	card->sdio_info[card->sdio_drv_num].func_type = func->type;
	card->sdio_info[card->sdio_drv_num].ctxt = func;
	card->sdio_drv_num++;

	return 0;

disable_runtimepm:
	#if 0
	if (func->card->host->caps & MMC_CAP_POWER_OFF_CARD)
		pm_runtime_put_noidle(dev);
	#endif
	return ret;
}

static int sdio_bus_remove(struct sdio_func *func)
{
	struct sdio_driver *drv = func->drv;
	int ret = 0;

	drv->remove(func);

	if (func->irq_handler) {
		pr_warn("WARNING: driver %s did not remove its interrupt handler!\n",
			drv->name);
		sdio_claim_host(func);
		sdio_release_irq(func);
		sdio_release_host(func);
	}

	return ret;
}

/**
 *	sdio_register_driver - register a function driver
 *	@drv: SDIO function driver
 */
int sdio_register_driver(struct sdio_driver *drv)
{
    INIT_LIST_HEAD(&drv->driver_list);

    /* Add it to the list of known drivers */
    list_add_tail(&drv->driver_list, &sdio_driver_list);

    return 0;
}

/**
 *	sdio_unregister_driver - unregister a function driver
 *	@drv: SDIO function driver
 */
void sdio_unregister_driver(struct sdio_driver *drv)
{
}

static void sdio_release_func(struct device *dev)
{
	struct sdio_func *func = dev_to_sdio_func(dev);

	sdio_free_func_cis(func);

	kfree(func->info);

	kfree(func);
}

/*
 * Allocate and initialise a new SDIO function structure.
 */
struct sdio_func *sdio_alloc_func(struct mmc_card *card)
{
	struct sdio_func *func;

	func = kzalloc(sizeof(struct sdio_func), GFP_KERNEL);
	if (!func)
		return ERR_PTR(-ENOMEM);

	func->card = card;

	return func;
}

/*
 * Register a new SDIO function with the driver model.
 */
int sdio_add_func(struct sdio_func *func)
{
	int ret;

	ret = sdio_bus_probe(func);
	if (ret == 0) {
		sdio_func_set_present(func);
	}

	return ret;
}

/*
 * Unregister a SDIO function with the driver model, and
 * (eventually) free it.
 * This function can be called through error paths where sdio_add_func() was
 * never executed (because a failure occurred at an earlier point).
 */
void sdio_remove_func(struct sdio_func *func)
{
	if (!sdio_func_present(func))
		return;

    sdio_bus_remove(func);
	sdio_release_func(&func->dev);
	device_del(&func->dev);
	put_device(&func->dev);
}

