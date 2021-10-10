#ifndef __NF_REG_H__
#define __NF_REG_H__

#define NFIDLE             0xC000
#define NFCADDRESS         0x1000

// define MMIO register for NAND flash //
#define NF_BASE     ITH_NAND_BASE    /* APB Map: 0xD0700000,  HOST Map: 0x7000  */


#define NF_REG_WRITE_DATA_BASE_ADDR         (NF_BASE+0x00)
#define NF_REG_READ_DATA_BASE_ADDR          (NF_BASE+0x04)
#define NF_REG_WRITE_SPARE_BASE_ADDR        (NF_BASE+0x08)
#define NF_REG_READ_SPARE_BASE_ADDR         (NF_BASE+0x0C)
#define NF_REG_LBA_DATA_LENGTH              (NF_BASE+0x1C)//?

#define NF_REG_MEM_DST_CORR_ECC_ADDR        (NF_BASE+0x10)

#define NF_REG_STORAGE_ADDR1                (NF_BASE+0x14)
#define NF_REG_STORAGE_ADDR2                (NF_BASE+0x18)

#define NF_REG_USER_DEF_CTRL                (NF_BASE+0x20)

#define NF_REG_CMD1                         (NF_BASE+0x28)
#define NF_REG_CMD2                         (NF_BASE+0x2C)
#define NF_REG_CMD3                         (NF_BASE+0x30)
#define NF_REG_CMD4                         (NF_BASE+0x34)

#define NF_REG_CMD_FIRE                     (NF_BASE+0x38)

#define NF_REG_GENERAL                      (NF_BASE+0x1C)
#define NF_REG_IDCYCLE                      (NF_BASE+0x40)
#define NF_REG_STATUS                       (NF_BASE+0x3C)
#define NF_REG_CLOCK_DIVIDE                 (NF_BASE+0x24)

#define NF_REG_ECC_SECTOR1_STATUS           (NF_BASE+0x2C)
#define NF_REG_ECC_SECTOR1_ERR_ADDR1        (NF_BASE+0x2E)
#define NF_REG_ECC_SECTOR1_ERR_ADDR2        (NF_BASE+0x30)
#define NF_REG_ECC_SECTOR1_ERR_ADDR3        (NF_BASE+0x32)
#define NF_REG_ECC_SECTOR1_ERR_ADDR4        (NF_BASE+0x34)

#define NF_REG_NFMC_ID1                     (NF_BASE+0x44)
#define NF_REG_NFMC_ID2                     (NF_BASE+0x48)

#define NF_REG_BCH_ECC_MAP                  (NF_BASE+0x4C)
#define NF_REG_ECC_ERROR_MAP                (NF_BASE+0x50)
#define NF_REG_WR_SCRAMB_INIT               (NF_BASE+0x54)
#define NF_REG_RD_SCRAMB_INIT               (NF_BASE+0x58)

#define NF_REG_ECC_12BIT_LAST_SEG_INIT_VAL  (NF_BASE+0x5C)
#define NF_REG_ECC_24BIT_LAST_SEG_INIT_VAL  (NF_BASE+0x60)
#define NF_REG_ECC_40BIT_LAST_SEG_INIT_VAL  (NF_BASE+0x64)
#define NF_REG_ECC_12BIT_SEG_INIT_VAL       (NF_BASE+0x68)
#define NF_REG_ECC_24BIT_SEG_INIT_VAL       (NF_BASE+0x6C)
#define NF_REG_ECC_40BIT_SEG_INIT_VAL       (NF_BASE+0x70)

#define NF_REG_INTR                         (NF_BASE+0x74)
#define NF_REG_RD_SCRAMB_CHECK              (NF_BASE+0x78)


// define command of nand flash //
#define READ_CMD_1ST        0x00
#define READ_CMD_2ND        0x30
#define READ_CMD_SPARE      0x50

#define PROGRAM_CMD_1ST     0x80
#define PROGRAM_CMD_2ND     0x10

#define ERASE_CMD_1ST       0x60
#define ERASE_CMD_2ND       0xD0

#define READSTATUS_CMD_2ND  0X70

#define READID90_CMD_1ST    0X90
#define READID91_CMD_1ST    0X91

#define RESET_CMD_2ST       0xFF

#define LL_RP_1STHALF       0x01
#define LL_RP_2NDHALF       0x00
#define LL_RP_DATA          0xfe
#define LL_RP_SPARE         0xff

#define READ_ID_1_CYCLE         0x0001
#define READ_ID_2_CYCLE         0x0002
#define READ_ID_3_CYCLE         0x0003
#define READ_ID_4_CYCLE         0x0004
#define READ_ID_5_CYCLE         0x0005
#define READ_ID_6_CYCLE         0x0006
#define READ_ID_7_CYCLE         0x0007
#define READ_ID_8_CYCLE         0x0008

// ECC Register
#define Ecc1stStatus        0x702C
#define Ecc2ndStatus        0x703E
#define Ecc3rdStatus        0x7050
#define Ecc4thStatus        0x7062

#define Ecc1st512Addr1      0x702E
#define Ecc1st512Addr2      0x7030
#define Ecc1st512Addr3      0x7032
#define Ecc1st512Addr4      0x7034

#define Ecc1st512Data1      0x7036
#define Ecc1st512Data2      0x7038
#define Ecc1st512Data3      0x703A
#define Ecc1st512Data4      0x703C

#define Ecc2nd512Addr1      0x7040
#define Ecc2nd512Addr2      0x7042
#define Ecc2nd512Addr3      0x7044
#define Ecc2nd512Addr4      0x7046

#define Ecc2nd512Data1      0x7048
#define Ecc2nd512Data2      0x704A
#define Ecc2nd512Data3      0x704C
#define Ecc2nd512Data4      0x704E

#define Ecc3rd512Addr1      0x7052
#define Ecc3rd512Addr2      0x7054
#define Ecc3rd512Addr3      0x7056
#define Ecc3rd512Addr4      0x7058

#define Ecc3rd512Data1      0x705A
#define Ecc3rd512Data2      0x705C
#define Ecc3rd512Data3      0x705E
#define Ecc3rd512Data4      0x7060

#define Ecc4th512Addr1      0x7064
#define Ecc4th512Addr2      0x7066
#define Ecc4th512Addr3      0x7068
#define Ecc4th512Addr4		0x706A

#define Ecc4th512Data1		0x706C
#define Ecc4th512Data2		0x706E
#define Ecc4th512Data3		0x7070
#define Ecc4th512Data4		0x7072

#define ECC_ERR_AND_NUMOFBITS 0x800C
#define ECCSTATUSERRBIT       0x8000
#define ECCSTATUSERRNUMBIT    0x0C
#define ECCSTATUSPARERRBIT    0x02
#define ECCSTATUSERROVERBIT   0x01

#define NF_CMD_FIRE             0x00000001
#define NF_CMD_SW_RESET         0x00000100

typedef enum NF_CMD_TYPE_TAG
{
    NF_ERASE,
    NF_WRITE,
    NF_READ,
    NF_READ_ID,
    NF_CMD_UNKNOW
}NF_CMD_TYPE;

typedef enum HWECC_RESP_TYPE_TAG
{
    NF_NOECCEN,
    NF_NOERROR,
    NF_ECCCORPASS,
    NF_PARITYADDRERR,
    NF_OVER4ERR,
    NF_NEEDRETRY
}HWECC_RESP_TYPE;

//static const MMP_UINT32 WRITE_SYMBOL  = (((MMP_UINT32)'F') << 24) | (((MMP_UINT32)'T') << 16) | (((MMP_UINT32)'L') << 8) | 'W';


#endif
