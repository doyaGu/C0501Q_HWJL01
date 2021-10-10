#ifndef __NF_REG_H__
#define __NF_REG_H__

#define NFIDLE             0xC000
#define NFCADDRESS         0x1000

// define MMIO register for NAND flash //

#define NF_REG_WRITE_DATA_BASE_ADDR1        0x1300
#define NF_REG_WRITE_DATA_BASE_ADDR2        0x1302
#define NF_REG_READ_DATA_BASE_ADDR1         0x1304
#define NF_REG_READ_DATA_BASE_ADDR2         0x1306
#define NF_REG_WRITE_SPARE_BASE_ADDR1       0x1308
#define NF_REG_WRITE_SPARE_BASE_ADDR2       0x130A
#define NF_REG_READ_SPARE_BASE_ADDR1        0x130C
#define NF_REG_READ_SPARE_BASE_ADDR2        0x130E

#define NF_REG_LBA_DATA_LENGTH              0x131C

#define NF_REG_MEM_DST_CORR_ECC_ADDR1       0x1310
#define NF_REG_MEM_DST_CORR_ECC_ADDR2       0x1312

#define NF_REG_STORAGE_ADDR1                0x1314
#define NF_REG_STORAGE_ADDR2                0x1316
#define NF_REG_STORAGE_ADDR3                0x1318


#define NF_REG_CMD1                         0x1324
#define NF_REG_CMD2                         0x1326
#define NF_REG_CMD3                         0x1328

#define NF_REG_GENERAL1                     0x131A
#define NF_REG_IDCYCLE                      0x131E
#define NF_REG_STATUS                       0x132A
#define NF_REG_CLOCK_DIVIDE1                0x1320
#define NF_REG_CLOCK_DIVIDE2                0x1322

#define NF_REG_ECC_SECTOR1_STATUS           0x132C
#define NF_REG_ECC_SECTOR1_ERR_ADDR1        0x132E
#define NF_REG_ECC_SECTOR1_ERR_ADDR2        0x1330
#define NF_REG_ECC_SECTOR1_ERR_ADDR3        0x1332
#define NF_REG_ECC_SECTOR1_ERR_ADDR4        0x1334
#define NF_REG_ECC_SECTOR1_CORRECT_VAL1     0x1336
#define NF_REG_ECC_SECTOR1_CORRECT_VAL2     0x1338
#define NF_REG_ECC_SECTOR1_CORRECT_VAL3     0x133A
#define NF_REG_ECC_SECTOR1_CORRECT_VAL4     0x133C

#define NF_REG_ECC_SECTOR2_STATUS           0x133E
#define NF_REG_ECC_SECTOR2_ERR_ADDR1        0x1340
#define NF_REG_ECC_SECTOR2_ERR_ADDR2        0x1342
#define NF_REG_ECC_SECTOR2_ERR_ADDR3        0x1344
#define NF_REG_ECC_SECTOR2_ERR_ADDR4        0x1346
#define NF_REG_ECC_SECTOR2_CORRECT_VAL1     0x1348
#define NF_REG_ECC_SECTOR2_CORRECT_VAL2     0x134A
#define NF_REG_ECC_SECTOR2_CORRECT_VAL3     0x134C
#define NF_REG_ECC_SECTOR2_CORRECT_VAL4     0x134E

#define NF_REG_ECC_SECTOR3_STATUS           0x1350
#define NF_REG_ECC_SECTOR3_ERR_ADDR1        0x1352
#define NF_REG_ECC_SECTOR3_ERR_ADDR2        0x1354
#define NF_REG_ECC_SECTOR3_ERR_ADDR3        0x1356
#define NF_REG_ECC_SECTOR3_ERR_ADDR4        0x1358
#define NF_REG_ECC_SECTOR3_CORRECT_VAL1     0x135A
#define NF_REG_ECC_SECTOR3_CORRECT_VAL2     0x135C
#define NF_REG_ECC_SECTOR3_CORRECT_VAL3     0x135E
#define NF_REG_ECC_SECTOR3_CORRECT_VAL4     0x1360

#define NF_REG_ECC_SECTOR4_STATUS           0x1362
#define NF_REG_ECC_SECTOR4_ERR_ADDR1        0x1364
#define NF_REG_ECC_SECTOR4_ERR_ADDR2        0x1366
#define NF_REG_ECC_SECTOR4_ERR_ADDR3        0x1368
#define NF_REG_ECC_SECTOR4_ERR_ADDR4        0x136A
#define NF_REG_ECC_SECTOR4_CORRECT_VAL1     0x136C
#define NF_REG_ECC_SECTOR4_CORRECT_VAL2     0x136E
#define NF_REG_ECC_SECTOR4_CORRECT_VAL3     0x1370
#define NF_REG_ECC_SECTOR4_CORRECT_VAL4     0x1372

#define NF_REG_NFMC_ID1                     0x1374
#define NF_REG_NFMC_ID2                     0x1376
#define NF_REG_NFMC_ID3                     0x1378

#define NF_REG_BCH_ECC_ERR_MAP              0x137A
#define NF_REG_BCH_ECC_CORR_ERR_MAP         0x137C

#define NF_REG_LBA_ADDR12              		0x137E
#define NF_REG_LBA_ADDR34         			0x1380
#define NF_REG_LBA_ADDR56              		0x1382

#define NF_REG_BCH_ECC_16BIT_LASTSEGMENT    0x1384
#define NF_REG_BCH_ECC_14BIT_LASTSEGMENT    0x1386
#define NF_REG_BCH_ECC_8BIT_LASTSEGMENT     0x1388
#define NF_REG_BCH_ECC_16BIT_SEGMENT        0x138A
#define NF_REG_BCH_ECC_14BIT_SEGMENT        0x138C
#define NF_REG_BCH_ECC_8BIT_SEGMENT         0x138E

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

#define READ_ID_1_CYCLE         0x1000
#define READ_ID_2_CYCLE         0x2000
#define READ_ID_3_CYCLE         0x3000
#define READ_ID_4_CYCLE         0x4000
#define READ_ID_5_CYCLE         0x5000
#define READ_ID_6_CYCLE         0x6000
#define READ_ID_7_CYCLE         0x7000
#define READ_ID_8_CYCLE         0x8000

// ECC Register
#define Ecc1stStatus        0x132C
#define Ecc2ndStatus        0x133E
#define Ecc3rdStatus        0x1350
#define Ecc4thStatus        0x1362

#define Ecc1st512Addr1      0x132E
#define Ecc1st512Addr2      0x1330
#define Ecc1st512Addr3      0x1332
#define Ecc1st512Addr4      0x1334

#define Ecc1st512Data1      0x1336
#define Ecc1st512Data2      0x1338
#define Ecc1st512Data3      0x133A
#define Ecc1st512Data4      0x133C

#define Ecc2nd512Addr1      0x1340
#define Ecc2nd512Addr2      0x1342
#define Ecc2nd512Addr3      0x1344
#define Ecc2nd512Addr4      0x1346

#define Ecc2nd512Data1      0x1348
#define Ecc2nd512Data2      0x134A
#define Ecc2nd512Data3      0x134C
#define Ecc2nd512Data4      0x134E

#define Ecc3rd512Addr1      0x1352
#define Ecc3rd512Addr2      0x1354
#define Ecc3rd512Addr3      0x1356
#define Ecc3rd512Addr4      0x1358

#define Ecc3rd512Data1      0x135A
#define Ecc3rd512Data2      0x135C
#define Ecc3rd512Data3      0x135E
#define Ecc3rd512Data4      0x1360

#define Ecc4th512Addr1      0x1364
#define Ecc4th512Addr2      0x1366
#define Ecc4th512Addr3      0x1368
#define Ecc4th512Addr4		0x136A

#define Ecc4th512Data1		0x136C
#define Ecc4th512Data2		0x136E
#define Ecc4th512Data3		0x1370
#define Ecc4th512Data4		0x1372

#define ECC_ERR_AND_NUMOFBITS 0x800C
#define ECCSTATUSERRBIT       0x8000
#define ECCSTATUSERRNUMBIT    0x0C
#define ECCSTATUSPARERRBIT    0x02
#define ECCSTATUSERROVERBIT   0x01

typedef enum NF_CMD_TYPE_TAG
{
    NF_ERASE,
    NF_WRITE,
    NF_READ,
    NF_READ_ID,
    NF_CMD_UNKNOW
}NF_CMD_TYPE;

#ifdef NF_PATCH_READ_10K_LIFE_CYCLE_ISSUE
typedef enum HWECC_RESP_TYPE_TAG
{
    NF_NOECCEN,
    NF_NOERROR,
    NF_ECCCORPASS,
    NF_ECCCORPASS_1BIT,
    NF_ECCCORPASS_2BIT,
    NF_ECCCORPASS_3BIT,
	NF_ECCCORPASS_4BIT,
    NF_PARITYADDRERR,
    NF_OVER4ERR,
    NF_NEEDRETRY
}HWECC_RESP_TYPE;
#else
typedef enum HWECC_RESP_TYPE_TAG
{
    NF_NOECCEN,
    NF_NOERROR,
    NF_ECCCORPASS,
    NF_PARITYADDRERR,
    NF_OVER4ERR,
    NF_NEEDRETRY
}HWECC_RESP_TYPE;
#endif

static const unsigned long WRITE_SYMBOL = (((unsigned long)'F') << 24) | (((unsigned long)'T') << 16) | (((unsigned long)'L') << 8) | 'W';
//static const unsigned long	WRITE_SYMBOL = 0x43475649;
#endif
