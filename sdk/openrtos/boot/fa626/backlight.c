#include "ite/ith.h"

#if (CFG_CHIP_FAMILY == 9070 || CFG_CHIP_FAMILY == 9910)
#define PWM_TIMER (CFG_GPIO_BACKLIGHT_PWM - 4)
#define GPIO_MODE ITH_GPIO_MODE3
#else
#define PWM_TIMER ITH_PWM5
#define GPIO_MODE ITH_GPIO_MODE2
#endif

static void backlight_init(void);
static uint32_t __div32(uint32_t a, uint32_t b);
static uint64_t __div64_32(uint64_t n, uint32_t base);
static unsigned int GetPllFreq(int n);
static void GpioSetMode(unsigned int pin, ITHGpioMode mode);
static void TimerReset(ITHTimer timer);
static unsigned int GetPllFreqOut1(int n);
static unsigned int GetPllFreqOut2(int n);
static unsigned int GetBusClock(void);

// _start is default function name of entry point.
void _start(void)
{
    asm volatile("mcr p15, 0, %0, c7, c14, 0" : : "r"(0));  // clean and invalidate D-Cache all
    asm volatile("mcr p15, 0, %0, c7, c5, 0" : : "r"(0));   // invalidate I-Cache all
    backlight_init();
}

static uint32_t __div32(uint32_t a, uint32_t b)
{
    unsigned int q = 0, bit = 1;

    if (b == 0)
        return 0;

    while ((signed int) b >= 0) {
        b <<= 1;
        bit <<= 1;
    }

    while (bit) {
        if (b <= a) {
            a -= b;
            q += bit;
        }
        b >>= 1;
        bit >>= 1;
    }

    return q;
}

static uint64_t __div64_32(uint64_t n, uint32_t base)
{
    uint64_t rem = n;
    uint64_t b = base;
    uint64_t res, d = 1;
    uint32_t high = rem >> 32;

    /* Reduce the thing a bit first */
    res = 0;
    if (high >= base) {
        high = __div32(high, base);
        res = (uint64_t) high << 32;
        rem -= (uint64_t) (high*base) << 32;
    }

    while ((int64_t)b > 0 && b < rem) {
        b = b+b;
        d = d+d;
    }

    do {
        if (rem >= b) {
            rem -= b;
            res += d;
        }
        b >>= 1;
        d >>= 1;
    } while (d);

    return res;
}

static unsigned int GetPllFreq(int n)
{
    unsigned int pll   = 0;
    uint32_t pllBase   = n * 0x10;
    uint32_t reg1      = ithReadRegH(pllBase + ITH_PLL1_SET1_REG);
    uint32_t sdmSel    = (reg1 & (1<<15)) ? 1 : 0;
    uint32_t sdmBypass = (reg1 & (1<<14)) ? 1 : 0;
    uint32_t sdmFix    = ((reg1 & (3<<12)) >> 12) + 3;
    uint32_t preDiv    = (reg1 & 0x1f);

    if (sdmSel)
    { // SDM divider
        uint32_t reg4   = ithReadRegH(pllBase + ITH_PLL1_SET4_REG);
        uint32_t sdm    = (reg4 & 0x7ff);
        uint32_t sdm_dv = (reg4 & (3<<12)) >> 12;

        if (sdm & (1<<10))
            sdm = sdm | 0xfffff800;

        switch(sdm_dv)
        {
        case 0:
            sdm += 16 * 1024;
            break;

        case 1:
            sdm += 17 * 1024;
            break;

        case 2:
            sdm += 18 * 1024;
            break;

        case 3:
            sdm += (uint32_t)(16.5f * 2048);
            break;
        }

        if (sdm_dv != 3)
            pll = (unsigned int)__div64_32(__div64_32((uint64_t)CFG_OSC_CLK * sdmFix * sdm, preDiv), 1024);
        else
            pll = (unsigned int)__div64_32(__div64_32((uint64_t)CFG_OSC_CLK * sdmFix * sdm, preDiv), 2048);

    }
    else
    {
        // fix divider
        uint32_t reg3 = ithReadRegH(pllBase + ITH_PLL1_SET3_REG);
        uint32_t num = reg3 & 0x3ff;
        pll = (unsigned int)__div64_32((uint64_t)CFG_OSC_CLK * num, preDiv);
    }
    return pll;
}

static unsigned int GetPllFreqOut1(int n)
{
    uint32_t pllBase = n * 0x10;
    unsigned int srcclk = GetPllFreq(n);
    unsigned int clk = __div32(srcclk, (ithReadRegH(pllBase + ITH_PLL1_SET2_REG) & 0x7f));
    return clk;
}

static unsigned int GetPllFreqOut2(int n)
{
    uint32_t pllBase = n * 0x10;
    unsigned int srcclk = GetPllFreq(n);
    unsigned int clk = __div32(srcclk, ((ithReadRegH(pllBase + ITH_PLL1_SET2_REG) >> 8) & 0x7f));
    return clk;
}

static unsigned int GetBusClock(void)
{
    unsigned int src = (ithReadRegH(ITH_APB_CLK1_REG) & ITH_WCLK_SRC_SEL_MASK) >> ITH_WCLK_SRC_SEL_BIT;
    unsigned int div = (ithReadRegH(ITH_APB_CLK1_REG) & ITH_WCLK_RATIO_MASK) >> ITH_WCLK_RATIO_BIT;
    unsigned int clk;

    switch (src)
    {
    case 0x1: // From PLL1 output2
        clk = GetPllFreqOut2(0);
        break;

    case 0x2: // From PLL2 output1
        clk = GetPllFreqOut1(1);
        break;

    case 0x3: // From PLL2 output2
        clk = GetPllFreqOut2(1);
        break;

    case 0x4: // From PLL3 output1
        clk = GetPllFreqOut1(2);
        break;

    case 0x5: // From PLL3 output2
        clk = GetPllFreqOut2(2);
        break;

    case 0x6: // From CKSYS (12MHz/30MHz)
        clk = CFG_OSC_CLK;
        break;

    case 0x7: // From Ring OSC (200KHz)
        clk = 200000;
        break;

    case 0x00: // From PLL1 output1 (default)
    default:
        clk = GetPllFreqOut1(0);
        break;
    }
    return __div32(clk, (div + 1));
}

static void backlight_init(void)
{
    uint32_t busClock= GetBusClock();
    uint32_t blCount = __div32(busClock, CFG_BACKLIGHT_FREQ);
    uint32_t blMatch = __div32(blCount * CFG_BACKLIGHT_DEFAULT_DUTY_CYCLE, 100);

    GpioSetMode(CFG_GPIO_BACKLIGHT_PWM, GPIO_MODE);
    TimerReset(PWM_TIMER);
    ithTimerSetCounter(PWM_TIMER, 0);
    ithTimerSetPwmMatch(PWM_TIMER, blMatch, blCount);

    ithSetRegBitA(ITH_TIMER_BASE + ITH_TIMER1_CR_REG + PWM_TIMER * 0x4, ITH_TIMER_UPDOWN_BIT);
    ithSetRegBitA(ITH_TIMER_BASE + ITH_TIMER1_CR_REG + PWM_TIMER * 0x4, ITH_TIMER_MODE_BIT);
    ithSetRegBitA(ITH_TIMER_BASE + ITH_TIMER1_CR_REG + PWM_TIMER * 0x4, ITH_TIMER_PWMEN_BIT);
    ithSetRegBitA(ITH_TIMER_BASE + ITH_TIMER1_CR_REG + PWM_TIMER * 0x4, ITH_TIMER_EN_BIT);
}

static void GpioSetMode(unsigned int pin, ITHGpioMode mode)
{
    uint32_t value, mask;

    if (pin < 16)
    {
        value = mode << (pin * 2);
        mask = 0x3 << (pin * 2);
        ITH_WRITE_REG_MASK_A(ITH_GPIO_BASE + ITH_GPIO1_MODE_REG, value, mask);
    }
    else if (pin < 32)
    {
        value = mode << ((pin - 16) * 2);
        mask = 0x3 << ((pin - 16) * 2);
        ITH_WRITE_REG_MASK_A(ITH_GPIO_BASE + ITH_GPIO2_MODE_REG, value, mask);
    }
    else if (pin < 48)
    {
        value = mode << ((pin - 32) * 2);
        mask = 0x3 << ((pin - 32) * 2);
        ITH_WRITE_REG_MASK_A(ITH_GPIO_BASE + ITH_GPIO3_MODE_REG, value, mask);
    }
#if (CFG_CHIP_FAMILY == 9070 || CFG_CHIP_FAMILY == 9910)
    else
    {
        value = mode << ((pin - 48) * 2);
        mask = 0x3 << ((pin - 48) * 2);
        ITH_WRITE_REG_MASK_A(ITH_GPIO_BASE + ITH_GPIO4_MODE_REG, value, mask);
    }
#else //(CFG_CHIP_FAMILY == 9850)
    else if (pin < 64)
    {
        value = mode << ((pin - 48) * 2);
        mask = 0x3 << ((pin - 48) * 2);
        ITH_WRITE_REG_MASK_A(ITH_GPIO_BASE + ITH_GPIO4_MODE_REG, value, mask);
    }
    else if (pin < 80)
    {
        value = mode << ((pin - 64) * 2);
        mask = 0x3 << ((pin - 64) * 2);
        ITH_WRITE_REG_MASK_A(ITH_GPIO_BASE + ITH_GPIO5_MODE_REG, value, mask);
    } 
    else if (pin < 96)
    {
        value = mode << ((pin - 80) * 2);
        mask = 0x3 << ((pin - 80) * 2);
        ITH_WRITE_REG_MASK_A(ITH_GPIO_BASE + ITH_GPIO6_MODE_REG, value, mask);
    } 
    else
    {
        value = mode << ((pin - 96) * 2);
        mask = 0x3 << ((pin - 96) * 2);
        ITH_WRITE_REG_MASK_A(ITH_GPIO_BASE + ITH_GPIO7_MODE_REG, value, mask);
    }
#endif
}

static void TimerReset(ITHTimer timer)
{
    ITH_WRITE_REG_MASK_A(ITH_TIMER_BASE + ITH_TIMER_INTRSTATE_REG, 0x7 << (timer * 4), 0x7 << (timer * 4));
    ITH_WRITE_REG_MASK_A(ITH_TIMER_BASE + ITH_TIMER_INTRMASK_REG, 0, 0x7 << (timer * 4));

    ithWriteRegA(ITH_TIMER_BASE + timer * 0x10 + ITH_TIMER1_CNT_REG, 0);
    ithWriteRegA(ITH_TIMER_BASE + timer * 0x10 + ITH_TIMER1_LOAD_REG, 0);
    ithWriteRegA(ITH_TIMER_BASE + timer * 0x10 + ITH_TIMER1_MATCH1_REG, 0);
    ithWriteRegA(ITH_TIMER_BASE + timer * 0x10 + ITH_TIMER1_MATCH2_REG, 0);
    ithWriteRegA(ITH_TIMER_BASE + ITH_TIMER1_CR_REG + timer * 0x4, 0);
}
