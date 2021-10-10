#define SD_BASE     ITH_SD_BASE

#define SD_SECTOR_SIZE      512
#define SD_FIFO_WIDTH       4
#define SD_FIFO_DEPTH       128
#define SD_FIFO_SIZE        (SD_FIFO_WIDTH*SD_FIFO_DEPTH)

/*
* SD Controller Register Definition
*/
#define SD_REG_STS0                             (SD_BASE + 0x0000)
#define SD_MSK_IF_RESET                         0x00000001
#define SD_MSK_CRC_BYPASS                       0x00000002
#define SD_MSK_CRC_NON_FIX                      0x00000004
#define SD_MSK_ASYN_CLK_SEL                     0x00000008
#define SD_MSK_CLK_SRC                          0x00000010
#define SD_MSK_HIGH_SPEED                       0x00000020
#define SD_MSK_BUS_WIDTH                        0x000000C0

#define SD_BUS_WIDTH_1BIT                       0x00000000
#define SD_BUS_WIDTH_4BIT                       0x00000040
#define SD_BUS_WIDTH_8BIT                       0x00000080


#define SD_REG_STS1                             (SD_BASE + 0x0004)
#define SD_MSK_RESP_TIMEOUT_BYPASS              0x00001000
#define SD_MSK_RESP_CRC_BYPASS                  0x00000800
#define SD_MSK_CRC_READ_BYPASS                  0x00000400
#define SD_MSK_CRC_WRITE_BYPASS                 0x00000200
#define SD_MSK_D0_STATUS_HIGH                   0x00000020
#define SD_MSK_RESP_TIMEOUT                     0x00000010
#define SD_MSK_RESP_CRC                         0x00000008
#define SD_MSK_CRC_READ                         0x00000004
#define SD_MSK_CRC_WRITE                        0x00000002
#define SD_MSK_CMD_END                          0x00000001
#define SD_ERROR								0x1E


#define SD_REG_CTL                             (SD_BASE + 0x0008)
#define SD_MSK_CMD_TRIGGER                      0x00000001
#define SD_MSK_AUTO_SWAP                        0x00000002
#define SD_MSK_RESP_TYPE                        0x0000000C
#define SD_MSK_CMD_TYPE                         0x00000030
#define SD_MSK_CLK_CTRL                         0x00000040   /* only write will gating, read will not gating */
#define SD_MSK_CLK_ALWAYS_ON                    0x00000200   /* 9850: if 0 => check bit6, if 1 => clock always on and bit6 not work */
#define SD_MSK_SWITCH_PIN1_4					0x00000800

enum
{
    SD_RESP_TYPE_NON = 0x00,
    SD_RESP_TYPE_48 = 0x04,
    SD_RESP_TYPE_48_BUSY = 0x08,
    SD_RESP_TYPE_136 = 0x0C
};

enum
{
    SD_CMD_NO_DATA = 0x00,
    SD_CMD_RESERVED = 0x10,
    SD_CMD_DATA_OUT = 0x20,
    SD_CMD_DATA_IN = 0x30
};

#define SD_REG_SECTOR_COUNT_L                 (SD_BASE + 0x000C)
#define SD_REG_SECTOR_COUNT_H                 (SD_BASE + 0x0014)
#define SD_REG_CLK_DIV                        (SD_BASE + 0x0010)
#define SD_REG_COMMAND                        (SD_BASE + 0x0018)

#define SD_REG_ARG0                           (SD_BASE + 0x001C)
#define SD_REG_ARG1                           (SD_BASE + 0x0020)
#define SD_REG_ARG2                           (SD_BASE + 0x0024)
#define SD_REG_ARG3                           (SD_BASE + 0x0028)

#define SD_REG_INTR                           (SD_BASE + 0x002C)
#define SD_SHT_INTR_MSK                       8
enum {
    SD_INTR_CMD_END = 0x01,
    SD_INTR_WRITE_CRC = 0x02,
    SD_INTR_READ_CRC = 0x04,
    SD_INTR_RESP_CRC = 0x08,
    SD_INTR_RESP_TIMEOUT = 0x10,
    SD_INTR_SDIO = 0x20
};
#define SD_INTR_ALL                           0x1F
#define SD_INTR_ERR                           0x1E

#define SD_INTR_SDIO_MSK        0x2000    /* 9850 A0 */
#define SD_INTR_SDIO_CPU_MSK    0x4000    /* 9850 A1, keep register status but no interrupt */
#define SD_SDIO_BIT4_MUX        0x8000    /* 9850 A1 */



#define SD_REG_RESP_7_0                       (SD_BASE + 0x0030)
#define SD_REG_RESP_15_8                      (SD_BASE + 0x0034)
#define SD_REG_RESP_23_16                     (SD_BASE + 0x0038)
#define SD_REG_RESP_31_24                     (SD_BASE + 0x003C)
#define SD_REG_RESP_39_32                     (SD_BASE + 0x0040)
#define SD_REG_RESP_47_40                     (SD_BASE + 0x0044)
#define SD_REG_RESP_55_48                     (SD_BASE + 0x0048)
#define SD_REG_RESP_63_56                     (SD_BASE + 0x004C)
#define SD_REG_RESP_71_64                     (SD_BASE + 0x0050)
#define SD_REG_RESP_79_72                     (SD_BASE + 0x0054)
#define SD_REG_RESP_87_80                     (SD_BASE + 0x0058)
#define SD_REG_RESP_95_88                     (SD_BASE + 0x005C)
#define SD_REG_RESP_103_96                    (SD_BASE + 0x0060)
#define SD_REG_RESP_111_104                   (SD_BASE + 0x0064)
#define SD_REG_RESP_119_112                   (SD_BASE + 0x0068)
#define SD_REG_RESP_127_120                   (SD_BASE + 0x006C)
#define SD_REG_RESP_135_128                   (SD_BASE + 0x0070)

#define SD_REG_LENGTH                         (SD_BASE + 0x0074)
#define SD_REG_RESP_TIMEOUT                   (SD_BASE + 0x0078)


/*
 * Smedia Wrap Register
 */
#define SDW_REG_DATA_LEN                         (SD_BASE + 0x007C)
#define SDW_REG_DATA_PORT                        (SD_BASE + 0x0080)

#define SDW_REG_WRAP_STATUS                      (SD_BASE + 0x0084)
#define SDW_MSK_INTR_MASK                        0x80000000
#define SDW_INTR_WRAP_END                        0x01000000
#define SDW_MSK_FIFO_CNT                         0x00000FF0
#define SDW_MSK_FIFO_EMPTY                       0x00000002
#define SDW_MSK_FIFO_FULL                        0x00000001

#define SDW_REG_WRAP_CTRL                        (SD_BASE + 0x0088)
#define SDW_MSK_DATA_IN                          0x00000040
#define SDW_MSK_ENDIAN_EN                        0x00000020
#define SDW_MSK_HW_HANDSHAKING                   0x00000010
#define SDW_MSK_SW_RESET                         0x00000004
#define SDW_MSK_CLEAR_FIFO                       0x00000002
#define SDW_MSK_WRAP_FIRE                        0x00000001

enum
{
    SDW_DATA_IN,
    SDW_DATA_OUT
};
enum
{
    SDW_NON_DMA,
    SDW_DMA
};
