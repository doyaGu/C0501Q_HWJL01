
#define ITH_DMA_INT_REG                 0x00
#define ITH_DMA_INT_TC_REG              0x04
#define ITH_DMA_INT_TC_CLR_REG          0x08

#define ITH_DMA_INT_ERRABT_REG          0x0C
#define ITH_DMA_INT_ERRABT_CLR_REG      0x10

#define ITH_DMA_TC_REG                  0x14
#define ITH_DMA_ERRABT_REG              0x18
#define ITH_DMA_CH_EN_REG               0x1C
#define ITH_DMA_SYNC_REG                0x20
#define ITH_DMA_LDM_REG                 0x24
#define ITH_DMA_WDT_REG                 0x28
#define ITH_DMA_GE_REG                  0x2C
#define ITH_DMA_PSE_REG                 0x30
#define ITH_DMA_REVISION_REG            0x34
#define ITH_DMA_FEATURE_REG             0x38

#define ITH_DMA_WO_VALUE_REG            0x50

#define ITH_DMA_CTRL_CH(x)              (0x100+(x)*0x20)
#define ITH_DMA_CFG_CH(x)               (0x104+(x)*0x20)
#define ITH_DMA_SRC_CH(x)               (0x108+(x)*0x20)
#define ITH_DMA_DST_CH(x)               (0x10C+(x)*0x20)
#define ITH_DMA_LLP_CH(x)               (0x110+(x)*0x20)
#define ITH_DMA_SIZE_CH(x)              (0x114+(x)*0x20)
#define ITH_DMA_STRIDE_CH(x)            (0x118+(x)*0x20)

/*
 * Error/abort interrupt status/clear register
 * Error/abort status register
 */
#define ITH_DMA_EA_ERR_CH(x)            (1<<(x))
#define ITH_DMA_EA_WDT_CH(x)            (1<<((x)+8))
#define ITH_DMA_EA_ABT_CH(x)            (1<<((x)+16))

/*
* Control register
*/
#define ITH_DMA_CTRL_WE(x)              ((1 << (x)) & 0xff)
#define ITH_DMA_CTRL_WSYNC              (1 << 8)
#define ITH_DMA_CTRL_SE(x)              (((x) & 0x7) << 9)
#define ITH_DMA_CTRL_SE_ENABLE          (1 << 12)
#define ITH_DMA_CTRL_WE_ENABLE          (1 << 13)
#define ITH_DMA_CTRL_2D                 (1 << 14)
#define ITH_DMA_CTRL_EXP                (1 << 15)
#define ITH_DMA_CTRL_ENABLE             (1 << 16)
#define ITH_DMA_CTRL_WDT_ENABLE         (1 << 17)
#define ITH_DMA_CTRL_DST_INC            (0x0 << 18)
#define ITH_DMA_CTRL_DST_FIXED          (0x2 << 18)
#define ITH_DMA_CTRL_SRC_INC            (0x0 << 20)
#define ITH_DMA_CTRL_SRC_FIXED          (0x2 << 20)
#define ITH_DMA_CTRL_DST_BURST_CTRL(x)  (((x) & 0x3) << 18)
#define ITH_DMA_CTRL_SRC_BURST_CTRL(x)  (((x) & 0x3) << 20)
#define ITH_DMA_CTRL_DST_WIDTH(x)       (((x) & 0x7) << 22)
#define ITH_DMA_CTRL_SRC_WIDTH(x)       (((x) & 0x7) << 25)
#define ITH_DMA_CTRL_MASK_TC            (1 << 28)
#define ITH_DMA_CTRL_SrcTcnt(x)         (((x) & 0x7) << 29)

/*
* Configuration register
*/
#define ITH_DMA_CFG_MASK_TCI            (1 << 0)	/* mask tc interrupt */
#define ITH_DMA_CFG_MASK_EI             (1 << 1)	/* mask error interrupt */
#define ITH_DMA_CFG_MASK_AI             (1 << 2)	/* mask abort interrupt */
#define ITH_DMA_CFG_SRC_HANDSHAKE(x)    (((x) & 0xf) << 3)
#define ITH_DMA_CFG_SRC_HANDSHAKE_EN(x) ((x) << 7)
#define ITH_DMA_CFG_DST_HANDSHAKE(x)    (((x+8) & 0xf) << 9)
#define ITH_DMA_CFG_DST_HANDSHAKE_EN(x) ((x) << 13)
#define ITH_DMA_CFG_LLP_CNT(cfg)        (((cfg) >> 16) & 0xf)
#define ITH_DMA_CFG_GW(x)               (((x) & 0xff) << 20)
#define ITH_DMA_CFG_HIGH_PRIO           (1 << 28)
#define ITH_DMA_CFG_WO_MODE             (1 << 30)
#define ITH_DMA_CFG_UNALIGN_MODE        (1 << 31)

#define ITH_DMA_INT_MASK                0x7   // D[2]:abort, D[1]:error, D[0]:tc

/*
* Transfer size register
*/
#define ITH_DMA_CYC_MASK                0x3fffff
#define ITH_DMA_CYC_TOTAL(x)            ((x) & ITH_DMA_CYC_MASK)
#define ITH_DMA_CYC_2D(x, y)            (((x) & 0xffff) | (((y) & 0xffff) << 16))

/*
* Stride register
*/
#define ITH_DMA_STRIDE_SRC(x)           ((x) & 0xffff)
#define ITH_DMA_STRIDE_DST(x)           (((x) & 0xffff) << 16)

/*
* Hardware handshaking source select
*/
#define ITH_DMA_HW_SRC_SEL_S_REG(ch)      ((ch < 4) ? 0x60 : 0x64)
#define ITH_DMA_HW_SRC_SEL_D_REG(ch)      ((ch < 4) ? 0x68 : 0x6C)
#define ITH_DMA_HW_SRC_SEL_MSK(ch)        (0x1F << ((ch % 4)*8))
#define ITH_DMA_HW_SRC_SEL_VAL(ch, val)   ((val) << ((ch % 4)*8))

typedef enum
{
    ITH_DMA_BURST_1 = 0,
    ITH_DMA_BURST_2 = 1,
    ITH_DMA_BURST_4 = 2,
    ITH_DMA_BURST_8 = 3,
    ITH_DMA_BURST_16 = 4,
    ITH_DMA_BURST_32 = 5,
    ITH_DMA_BURST_64 = 6,
    ITH_DMA_BURST_128 = 7
} ITHDmaBurst;

typedef enum
{
    ITH_DMA_CH_PRIO_LOW  = 0x0,
    ITH_DMA_CH_PRIO_HIGH = ITH_DMA_CFG_HIGH_PRIO
} ITHDmaPriority;

typedef enum
{
    ITH_DMA_WIDTH_8 = 0,
    ITH_DMA_WIDTH_16 = 1,
    ITH_DMA_WIDTH_32 = 2,
    ITH_DMA_WIDTH_64 = 3
} ITHDmaWidth;

typedef enum
{
    ITH_DMA_CTRL_INC = 0,
    ITH_DMA_CTRL_FIX = 2
} ITHDmaAddrCtl;

typedef enum
{
    ITH_DMA_NORMAL_MODE = 0,
    ITH_DMA_HW_HANDSHAKE_MODE = 1
} ITHDmaMode;

typedef enum
{
    ITH_DMA_MEM          = -1,
    ITH_DMA_IR_CAP_TX    = 0,
    ITH_DMA_IR_CAP_RX    = 1,
    ITH_DMA_IIC0_TX      = 2,
    ITH_DMA_IIC0_RX      = 3,
    ITH_DMA_IIC1_TX      = 4,
    ITH_DMA_IIC1_RX      = 5,
    ITH_DMA_AXISPI_TX_RX = 6,
    ITH_DMA_SPI0_TX      = 7,
    ITH_DMA_SPI0_RX      = 8,
    ITH_DMA_SPI1_TX      = 9,
    ITH_DMA_SPI1_RX      = 10,
    ITH_DMA_UART0_TX     = 11,
    ITH_DMA_UART0_RX     = 12,
    ITH_DMA_UART1_TX	     = 13,
    ITH_DMA_UART1_RX	     = 14,
    ITH_DMA_UART2_TX	     = 15,
    ITH_DMA_UART2_RX	     = 16,
    ITH_DMA_UART3_TX	     = 17,
    ITH_DMA_UART3_RX	     = 18,
    ITH_DMA_UART4_TX	     = 19,
    ITH_DMA_UART4_RX	     = 20,
    ITH_DMA_UART5_TX	     = 21,
    ITH_DMA_UART5_RX	     = 22
} ITHDmaRequest;



