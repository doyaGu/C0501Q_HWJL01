/******************************************************************************
 *
 * Copyright(c) 2007 - 2012 Realtek Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110, USA
 *
 *
 ******************************************************************************/
#include <drv_types.h>
#include <hci_intf.h>

extern const struct host_ctrl_intf_ops hci_ops;

struct dvobj_priv *hci_dvobj_init(void *data)
{
	if(hci_ops.dvobj_init)
		return hci_ops.dvobj_init(data);
	else
		return NULL;
}

void hci_dvobj_deinit(struct dvobj_priv *dvobj)
{
	if(hci_ops.dvobj_deinit)
		hci_ops.dvobj_deinit(dvobj);
}

int hci_dvobj_request_irq(struct dvobj_priv *dvobj)
{
	if(hci_ops.dvobj_request_irq)
		return hci_ops.dvobj_request_irq(dvobj);
	else
		return _FAIL;
}

void hci_dvobj_free_irq(struct dvobj_priv *dvobj)
{
	if(hci_ops.dvobj_free_irq)
		hci_ops.dvobj_free_irq(dvobj);
}

