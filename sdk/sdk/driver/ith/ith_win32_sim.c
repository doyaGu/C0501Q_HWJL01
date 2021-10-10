#include "ith_cfg.h"
#include <stdio.h>
#include <windows.h>

static uint8_t mmio[0xFFFF];
static uint8_t vram[CFG_RAM_SIZE];

int SpiOpen(DWORD dwClockRate)
{
    // default values
    mmio[ITH_PLL1_SET1_REG]     = mmio[ITH_PLL2_SET1_REG] = mmio[ITH_PLL3_SET1_REG] = 0x85;
    mmio[ITH_PLL1_SET1_REG + 1] = mmio[ITH_PLL2_SET1_REG + 1] = mmio[ITH_PLL3_SET1_REG + 1] = 0x01;

    mmio[ITH_PLL1_SET2_REG]     = 0x14;
    mmio[ITH_PLL1_SET2_REG + 1] = 0x14;

    mmio[ITH_PLL2_SET2_REG]     = mmio[ITH_PLL3_SET2_REG] = 0x18;
    mmio[ITH_PLL2_SET2_REG + 1] = mmio[ITH_PLL3_SET2_REG + 1] = 0x18;

    mmio[ITH_CMDQ_SR1_REG]      = 0x07;
    mmio[0x00D4]                = 0x2;
    mmio[0x0412]                = 0xE0;
    mmio[0x1288]                = 0x0;
    mmio[0x165E]                = 0x8;

    return 0;
}

#ifdef CFG_DEV_TEST
DEFINE_COULD_BE_MOCKED_NOT_VOID_FUNC1(
uint16_t, ithReadRegH, uint16_t, addr)
#else
uint16_t ithReadRegH(uint16_t addr)
#endif
{
    return *(uint16_t *)&mmio[addr];
}

#ifdef CFG_DEV_TEST
DEFINE_COULD_BE_MOCKED_VOID_FUNC2(
ithWriteRegH, uint16_t, addr, uint16_t, data)
#else
void ithWriteRegH(uint16_t addr, uint16_t data)
#endif
{
    uint16_t *ptr = (uint16_t *)&mmio[addr];
    *ptr = data;
}

void *ithMapVram(uint32_t addr, uint32_t size, uint32_t flags)
{
    return &vram[addr];
}

void ithUnmapVram(void *ptr, uint32_t size)
{}

void ithFlushDCacheRange(void *addr, uint32_t size)
{}

uint32_t ithSysAddr2VramAddr(void *sys_addr)
{
    ASSERT(((uint8_t *)vram) <= ((uint8_t *)sys_addr));
    ASSERT(((uint8_t *)sys_addr) <= (((uint8_t *)vram) + CFG_RAM_SIZE));

    return (uint32_t)sys_addr - (uint32_t)vram;
}
