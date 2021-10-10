#include <sdio_ops_linux.h>
#if 1
/**
 *	sdio_register_driver - register a function driver
 *	@drv: SDIO function driver
 */
extern int sdio_register_driver(struct sdio_driver *);
/**
 *	sdio_unregister_driver - unregister a function driver
 *	@drv: SDIO function driver
 */
extern void sdio_unregister_driver(struct sdio_driver *);

/*
 * SDIO I/O operations
 */
 /**
 *	sdio_claim_host - exclusively claim a bus for a certain SDIO function
 *	@func: SDIO function that will be accessed
 *
 *	Claim a bus for a set of operations. The SDIO function given
 *	is used to figure out which bus is relevant.
 */
extern void sdio_claim_host(struct sdio_func *func);
/**
 *	sdio_release_host - release a bus for a certain SDIO function
 *	@func: SDIO function that was accessed
 *
 *	Release a bus, allowing others to claim the bus for their
 *	operations.
 */
extern void sdio_release_host(struct sdio_func *func);
/**
 *	sdio_enable_func - enables a SDIO function for usage
 *	@func: SDIO function to enable
 *
 *	Powers up and activates a SDIO function so that register
 *	access is possible.
 */
extern int sdio_enable_func(struct sdio_func *func);
/**
 *	sdio_disable_func - disable a SDIO function
 *	@func: SDIO function to disable
 *
 *	Powers down and deactivates a SDIO function. Register access
 *	to this function will fail until the function is reenabled.
 */
extern int sdio_disable_func(struct sdio_func *func);
/**
 *	sdio_set_block_size - set the block size of an SDIO function
 *	@func: SDIO function to change
 *	@blksz: new block size or 0 to use the default.
 *
 *	The default block size is the largest supported by both the function
 *	and the host, with a maximum of 512 to ensure that arbitrarily sized
 *	data transfer use the optimal (least) number of commands.
 *
 *	A driver may call this to override the default block size set by the
 *	core. This can be used to set a block size greater than the maximum
 *	that reported by the card; it is the driver's responsibility to ensure
 *	it uses a value that the card supports.
 *
 *	Returns 0 on success, -EINVAL if the host does not support the
 *	requested block size, or -EIO (etc.) if one of the resultant FBR block
 *	size register writes failed.
 *
 */
extern int sdio_set_block_size(struct sdio_func *func, unsigned blksz);
/**
 *	sdio_claim_irq - claim the IRQ for a SDIO function
 *	@func: SDIO function
 *	@handler: IRQ handler callback
 *
 *	Claim and activate the IRQ for the given SDIO function. The provided
 *	handler will be called when that IRQ is asserted.  The host is always
 *	claimed already when the handler is called so the handler must not
 *	call sdio_claim_host() nor sdio_release_host().
 */
extern int sdio_claim_irq(struct sdio_func *func, sdio_irq_handler_t *handler);
/**
 *	sdio_release_irq - release the IRQ for a SDIO function
 *	@func: SDIO function
 *
 *	Disable and release the IRQ for the given SDIO function.
 */
extern int sdio_release_irq(struct sdio_func *func);
/**
 *	sdio_align_size - pads a transfer size to a more optimal value
 *	@func: SDIO function
 *	@sz: original transfer size
 *
 *	Pads the original data size with a number of extra bytes in
 *	order to avoid controller bugs and/or performance hits
 *	(e.g. some controllers revert to PIO for certain sizes).
 *
 *	If possible, it will also adjust the size so that it can be
 *	handled in just a single request.
 *
 *	Returns the improved size, which might be unmodified.
 */
extern unsigned int sdio_align_size(struct sdio_func *func, unsigned int sz);
/**
 *	sdio_readb - read a single byte from a SDIO function
 *	@func: SDIO function to access
 *	@addr: address to read
 *	@err_ret: optional status value from transfer
 *
 *	Reads a single byte from the address space of a given SDIO
 *	function. If there is a problem reading the address, 0xff
 *	is returned and @err_ret will contain the error code.
 */
extern u8 sdio_readb(struct sdio_func *func, unsigned int addr, int *err_ret);
/**
 *	sdio_readw - read a 16 bit integer from a SDIO function
 *	@func: SDIO function to access
 *	@addr: address to read
 *	@err_ret: optional status value from transfer
 *
 *	Reads a 16 bit integer from the address space of a given SDIO
 *	function. If there is a problem reading the address, 0xffff
 *	is returned and @err_ret will contain the error code.
 */
extern u16 sdio_readw(struct sdio_func *func, unsigned int addr, int *err_ret);
/**
 *	sdio_readl - read a 32 bit integer from a SDIO function
 *	@func: SDIO function to access
 *	@addr: address to read
 *	@err_ret: optional status value from transfer
 *
 *	Reads a 32 bit integer from the address space of a given SDIO
 *	function. If there is a problem reading the address,
 *	0xffffffff is returned and @err_ret will contain the error
 *	code.
 */
extern u32 sdio_readl(struct sdio_func *func, unsigned int addr, int *err_ret);
/**
 *	sdio_memcpy_fromio - read a chunk of memory from a SDIO function
 *	@func: SDIO function to access
 *	@dst: buffer to store the data
 *	@addr: address to begin reading from
 *	@count: number of bytes to read
 *
 *	Reads from the address space of a given SDIO function. Return
 *	value indicates if the transfer succeeded or not.
 */
extern int sdio_memcpy_fromio(struct sdio_func *func, void *dst,
	unsigned int addr, int count);
/**
 *	sdio_readsb - read from a FIFO on a SDIO function
 *	@func: SDIO function to access
 *	@dst: buffer to store the data
 *	@addr: address of (single byte) FIFO
 *	@count: number of bytes to read
 *
 *	Reads from the specified FIFO of a given SDIO function. Return
 *	value indicates if the transfer succeeded or not.
 */
extern int sdio_readsb(struct sdio_func *func, void *dst,
	unsigned int addr, int count);
/**
 *	sdio_writeb - write a single byte to a SDIO function
 *	@func: SDIO function to access
 *	@b: byte to write
 *	@addr: address to write to
 *	@err_ret: optional status value from transfer
 *
 *	Writes a single byte to the address space of a given SDIO
 *	function. @err_ret will contain the status of the actual
 *	transfer.
 */
extern void sdio_writeb(struct sdio_func *func, u8 b,
	unsigned int addr, int *err_ret);
/**
 *	sdio_writew - write a 16 bit integer to a SDIO function
 *	@func: SDIO function to access
 *	@b: integer to write
 *	@addr: address to write to
 *	@err_ret: optional status value from transfer
 *
 *	Writes a 16 bit integer to the address space of a given SDIO
 *	function. @err_ret will contain the status of the actual
 *	transfer.
 */
extern void sdio_writew(struct sdio_func *func, u16 b,
	unsigned int addr, int *err_ret);
/**
 *	sdio_writel - write a 32 bit integer to a SDIO function
 *	@func: SDIO function to access
 *	@b: integer to write
 *	@addr: address to write to
 *	@err_ret: optional status value from transfer
 *
 *	Writes a 32 bit integer to the address space of a given SDIO
 *	function. @err_ret will contain the status of the actual
 *	transfer.
 */
extern void sdio_writel(struct sdio_func *func, u32 b,
	unsigned int addr, int *err_ret);
/**
 *	sdio_writeb_readb - write and read a byte from SDIO function
 *	@func: SDIO function to access
 *	@write_byte: byte to write
 *	@addr: address to write to
 *	@err_ret: optional status value from transfer
 *
 *	Performs a RAW (Read after Write) operation as defined by SDIO spec -
 *	single byte is written to address space of a given SDIO function and
 *	response is read back from the same address, both using single request.
 *	If there is a problem with the operation, 0xff is returned and
 *	@err_ret will contain the error code.
 */
extern u8 sdio_writeb_readb(struct sdio_func *func, u8 write_byte,
	unsigned int addr, int *err_ret);
/**
 *	sdio_memcpy_toio - write a chunk of memory to a SDIO function
 *	@func: SDIO function to access
 *	@addr: address to start writing to
 *	@src: buffer that contains the data to write
 *	@count: number of bytes to write
 *
 *	Writes to the address space of a given SDIO function. Return
 *	value indicates if the transfer succeeded or not.
 */
extern int sdio_memcpy_toio(struct sdio_func *func, unsigned int addr,
	void *src, int count);
/**
 *	sdio_writesb - write to a FIFO of a SDIO function
 *	@func: SDIO function to access
 *	@addr: address of (single byte) FIFO
 *	@src: buffer that contains the data to write
 *	@count: number of bytes to write
 *
 *	Writes to the specified FIFO of a given SDIO function. Return
 *	value indicates if the transfer succeeded or not.
 */
extern int sdio_writesb(struct sdio_func *func, unsigned int addr,
	void *src, int count);
/**
 *	sdio_f0_readb - read a single byte from SDIO function 0
 *	@func: an SDIO function of the card
 *	@addr: address to read
 *	@err_ret: optional status value from transfer
 *
 *	Reads a single byte from the address space of SDIO function 0.
 *	If there is a problem reading the address, 0xff is returned
 *	and @err_ret will contain the error code.
 */
extern unsigned char sdio_f0_readb(struct sdio_func *func,
	unsigned int addr, int *err_ret);
/**
 *	sdio_f0_writeb - write a single byte to SDIO function 0
 *	@func: an SDIO function of the card
 *	@b: byte to write
 *	@addr: address to write to
 *	@err_ret: optional status value from transfer
 *
 *	Writes a single byte to the address space of SDIO function 0.
 *	@err_ret will contain the status of the actual transfer.
 *
 *	Only writes to the vendor specific CCCR registers (0xF0 -
 *	0xFF) are permiited; @err_ret will be set to -EINVAL for *
 *	writes outside this range.
 */
extern void sdio_f0_writeb(struct sdio_func *func, unsigned char b,
	unsigned int addr, int *err_ret);
/**
 *	sdio_get_host_pm_caps - get host power management capabilities
 *	@func: SDIO function attached to host
 *
 *	Returns a capability bitmask corresponding to power management
 *	features supported by the host controller that the card function
 *	might rely upon during a system suspend.  The host doesn't need
 *	to be claimed, nor the function active, for this information to be
 *	obtained.
 */
extern mmc_pm_flag_t sdio_get_host_pm_caps(struct sdio_func *func);
/**
 *	sdio_set_host_pm_flags - set wanted host power management capabilities
 *	@func: SDIO function attached to host
 *
 *	Set a capability bitmask corresponding to wanted host controller
 *	power management features for the upcoming suspend state.
 *	This must be called, if needed, each time the suspend method of
 *	the function driver is called, and must contain only bits that
 *	were returned by sdio_get_host_pm_caps().
 *	The host doesn't need to be claimed, nor the function active,
 *	for this information to be set.
 */
extern int sdio_set_host_pm_flags(struct sdio_func *func, mmc_pm_flag_t flags);
#endif

int rtw_sdio_set_drvdata(struct sdio_func *func, void *data){
	sdio_set_drvdata(func, data);
	return 0;
}

void *rtw_sdio_get_drvdata(struct sdio_func *func){
	return sdio_get_drvdata(func);
}

int rtw_sdio_set_block_size(struct sdio_func *func, unsigned int blksz){
	return sdio_set_block_size(func, blksz);
}

const SDIO_BUS_OPS rtw_sdio_bus_ops = {
// 0
	NULL, //bus_probe
	NULL, //bus_remove
	sdio_enable_func,//enable_func
	sdio_disable_func,//disable_func
	sdio_register_driver,//reg_driver
	sdio_unregister_driver,//unreg_driver
// 6
	sdio_claim_irq,//claim_irq
	sdio_release_irq,//release_irq
	sdio_claim_host,//claim_host
	sdio_release_host,//release_host
// 10
	sdio_readb,//readb
	sdio_readw,//readw
	sdio_readl,//readl
// 13
	sdio_writeb,//writeb
	sdio_writew,//writew
	sdio_writel,//writel
// 16
	sdio_memcpy_fromio,//memcpy_fromio
	sdio_memcpy_toio,//memcpy_toio
	sdio_f0_readb, //f0_readb
	sdio_f0_writeb//f0_writeb
};

