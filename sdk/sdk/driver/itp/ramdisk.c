
//=============================================================================
//                              Include Files
//=============================================================================
#include <string.h>
#include <stdlib.h>

//=============================================================================
//                              Constant Definition
//=============================================================================
#define FAT12       0x01
#define FAT16       0x02
#define FAT32       0x04

#define SECTORSIZE 0x200

//=============================================================================
//                              Macro Definition
//=============================================================================
//=============================================================================
//                              Private Structure Definition
//=============================================================================
typedef struct
{
    unsigned char   BS_jmpBoot[3];
    unsigned char   BS_OEMName[8];
    unsigned char   BPB_BytsPerSec[2];
    unsigned char   BPB_SecPerClus;
    unsigned char   BPB_ResvdSecCnt[2];
    unsigned char   BPB_NumFATs;
    unsigned char   BPB_RootEntCnt[2];
    unsigned char   BPB_TotSec16[2];
    unsigned char   BPB_Media;
    unsigned char   BPB_FATSz16[2];
    unsigned char   BPB_SecPerTrk[2];
    unsigned char   BPB_NumHeads[2];
    unsigned char   BPB_HiddSec[4];
    unsigned char   BPB_TotSec32[4];
  
    unsigned char   BS_DrvNum;
    unsigned char   BS_Reserved1;
    unsigned char   BS_BootSig;
    unsigned char   BS_SN[4];
    unsigned char   BS_VolName[11];
    unsigned char   BS_FilSysType[8];
    unsigned char   BS_ExeCode[448];
    unsigned char   signature1;
    unsigned char   signature2;
	unsigned char   dummy[48];
} FAT16_BPB_t;
 
typedef struct
{
    unsigned char   NAME[8];
    unsigned char   EXT[3];
    unsigned char   DIR_Attr;
    unsigned char   DIR_NTRes;
    unsigned char   DIR_CrtTimeTenth;
    unsigned char   DIR_CrtTime[2];
    unsigned char   DIR_CrtDate[2];
    unsigned char   DIR_LstAccDate[2];
    unsigned char   DIR_FstClusHI[2];
    unsigned char   DIR_WrtTime[2];
    unsigned char   DIR_WrtDate[2];
    unsigned char   DIR_FstClusLO[2];
    unsigned char   DIR_FileSize[4];
} DIR_t;

//=============================================================================
//                              Global Data Definition
//=============================================================================
//Offset for each field related to boot sector, unit: sector
static unsigned int  FAT1_Addr; 
static unsigned int  FAT2_Addr;
static unsigned int  ROOT_Addr;
static unsigned int  DATA_Addr;

// FAT mode
static unsigned char   FAT_Mode;

static unsigned short  BytsPerSec;
static unsigned short  ResvdSecCnt;
static unsigned short  RootEntCnt;
static unsigned short  TotSec16;
static unsigned short  FATSz16;
static unsigned short  SecPerTrk;
static unsigned short  NumHeads;
static unsigned int    HiddSec;
static unsigned int    TotSec32;
static unsigned int    SN;
static unsigned short  CrtTime;
static unsigned short  CrtDate;
static unsigned short  LstAccDate;
static unsigned short  FstClusHI;
static unsigned short  WrtTime;
static unsigned short  WrtDate;
static unsigned short  FstClusLO;
static unsigned int    FileSize;

// bit number for shift operate
static unsigned int  bit_bytesofsec;
static unsigned int  bit_bytesofclus;
static unsigned int  bit_secofclus;

static FAT16_BPB_t BS;
static DIR_t   ROOT[512];
static unsigned short  *FATTable;
static unsigned int  ClusterCount = 0; //actual count, 
static unsigned char *RAMDISK = NULL;
static unsigned int RAMDISK_size;
static int inited = 0;


//=============================================================================
//                              Macro Definition
//=============================================================================
static __inline unsigned int log2bin_f( unsigned int value)
{
    unsigned int n = 0;
    while (value)
    {
        value >>= 1;
        n++;
    }
    return n-1;
}

//=============================================================================
//                              Private Function Declaration
//=============================================================================
static int FS_Format(void);
static int FS_CheckBS(unsigned short Addr);
static int FS_SyncFat(void);
static int FS_SyncRoot(void);
static int FS_BuildBS(void);

//=============================================================================
//                              Public Function Definition
//=============================================================================

int 
RAMDISK_Initialize(void)    
{
    if (inited)
        return 1;
    
    RAMDISK_size = 0x400000;
    if(!RAMDISK)
        RAMDISK = (unsigned char*)malloc(RAMDISK_size);
    FS_Format();
	inited = 1;

    return 1;
}

int 
RAMDISK_Terminate(void)    
{
    if(RAMDISK)
    {
        free(RAMDISK);
        RAMDISK = NULL;
    }
    if(FATTable)
    {
        free(FATTable);
        FATTable = NULL;
    }
    inited = 0;
	return 1;
}


int
RAMDISK_GetCapacity(unsigned int* sectorNum, unsigned int* blockLength)
{
	*blockLength = SECTORSIZE;
	*sectorNum = RAMDISK_size/SECTORSIZE;
	return 1;
}

int
RAMDISK_WriteSector(unsigned int blockId, unsigned int sizeInSector, unsigned short* srcBuffer)
{	
	unsigned int targetpos;
	targetpos = blockId*SECTORSIZE;
	memcpy(RAMDISK+targetpos,srcBuffer,sizeInSector*SECTORSIZE);
	return 1;
}

int
RAMDISK_ReadSector(unsigned int blockId, unsigned int sizeInSector, unsigned short* destBuffer)
{
	unsigned int targetpos;
	targetpos = blockId*SECTORSIZE;
	memcpy(destBuffer,RAMDISK+targetpos,sizeInSector*SECTORSIZE);
	return 1;
}

//=============================================================================
//                              Private Function Definition
//=============================================================================

static int 
FS_Format(void)
{
    unsigned short i=0;
    unsigned int len = 0;
    
    if (FS_BuildBS())
    {
        RAMDISK_WriteSector(0,1,(unsigned short*)&BS);
    }

    FS_CheckBS(0);
    len = FATSz16<<log2bin_f(BytsPerSec);
    FATTable = (unsigned short*)malloc(len);
    memset(FATTable, 0, len);
    FATTable[0]=0xFFF8;
    FATTable[1]=0xFFFF;
    memset((void*)&ROOT, 0, sizeof(ROOT));
    FS_SyncFat();
    FS_SyncRoot();
    return 1;
}


static int 
FS_CheckBS(unsigned short Addr)
{
    unsigned int  FATSz;
    unsigned int  TotSec;
    unsigned int  RootDirSectors;
    
    if(RAMDISK_ReadSector(Addr,1,(unsigned short *)&BS)!= 1)
    {
        return 0;               
    }
    
    if( BS.signature1 != 0x55 || BS.signature2 != 0xAA )
    {
        return 0;
    }

    // set bit number 
    bit_bytesofsec = log2bin_f(BytsPerSec);
    bit_bytesofclus = log2bin_f(BS.BPB_SecPerClus<<bit_bytesofsec);
    bit_secofclus = log2bin_f(BS.BPB_SecPerClus);
    
    if( FATSz16 != 0)
    {
        FATSz = FATSz16;
    }
    else
    {
        return 0; // powei, do not support FAT32
    }
    if( TotSec16 != 0)
    {
        TotSec = TotSec16;
    }
    else
    {
        TotSec = TotSec32;
    }
    RootDirSectors = ((RootEntCnt<<5) + (BytsPerSec-1)) >> bit_bytesofsec;    
    FAT1_Addr = Addr+ResvdSecCnt;
    FAT2_Addr = FAT1_Addr+FATSz;
    ROOT_Addr = FAT2_Addr+FATSz;
    DATA_Addr = ROOT_Addr+RootDirSectors;
    ClusterCount = (TotSec - DATA_Addr)>>bit_secofclus;
    if( ClusterCount < 4085 )
    {
        FAT_Mode = FAT12;
    }
    else if( ClusterCount < 65525 )
    {
        FAT_Mode = FAT16;
    }
    else
    {
        FAT_Mode = FAT32;
        return 0;
    }
    
    return 1;
}//end of MMP_BOOL FS_CheckBS()


static int 
FS_SyncFat(void)
{
    unsigned int  FATSzBytes = FATSz16<<bit_bytesofsec;
    
    if( FAT_Mode == FAT12 )
    {
        unsigned short *tmpbuf = (unsigned short*)malloc(FATSzBytes);
        unsigned short  value;
        unsigned int  i;

        memset(tmpbuf, 0, FATSzBytes);

        for( i=0;i<ClusterCount+2;i++ )
        {
            value = FATTable[i];
            if( (i & 1) && (i & 2) )  //3
            {
                tmpbuf[(i>>1)+(i>>2)+1] |= (unsigned short)((value<<4)&0xFFF0);
            }
            else if( !(i & 1) && (i & 2) ) //2  
            {
                tmpbuf[(i>>1)+(i>>2)] |= (unsigned short)((value<<8)&0xFF00);
                tmpbuf[(i>>1)+(i>>2)+1] |= (unsigned short)((value>>8)&0x000F);
            }
            else if( (i & 1) && !(i & 2) ) //1
            {
                tmpbuf[(i>>1)+(i>>2)] |= (unsigned short)((value<<12)&0xF000);
                tmpbuf[(i>>1)+(i>>2)+1] |= (unsigned short)((value>>4)&0x00FF);
            }
            else  //0
            {
                tmpbuf[(i>>1)+(i>>2)] |= (value&0x0FFF);            
            }           
        }

        if(RAMDISK_WriteSector(FAT1_Addr,FATSz16,(unsigned short*)tmpbuf)!=1) 
        {
            return 0;
        }

        if(RAMDISK_WriteSector(FAT2_Addr,FATSz16,(unsigned short*)tmpbuf)!=1) 
        {
            return 0;
        }       
        free(tmpbuf);   
    }
    else // FAT16
    {
        if(RAMDISK_WriteSector(FAT1_Addr,FATSz16,(unsigned short*)FATTable)!=1) 
        {
            return 0;
        }

        if(RAMDISK_WriteSector(FAT2_Addr,FATSz16,(unsigned short*)FATTable)!=1) 
        {
            return 0;
        }
    }

    return 1;
}//FS_SyncFat


static int 
FS_SyncRoot(void)
{
    unsigned int  i = 0;
    unsigned int  RootDirSectors;

    RootDirSectors = (((RootEntCnt<<5) + (BytsPerSec-1)) >> bit_bytesofsec);  

    if(RAMDISK_WriteSector(ROOT_Addr,RootDirSectors,(unsigned short*)&ROOT)!=1)
    {
        return 0;
    }
    return 1;
}//synroot


static int 
FS_BuildBS(void)
{
    unsigned int last = 0 ;
    unsigned int blocklen = 0;
    unsigned int capacity = 0;
    unsigned int TMP1 = 0;
    unsigned int TMP2 = 0;
    unsigned short RootDirSecs = 0;
    

        if (RAMDISK_GetCapacity(&last,&blocklen) == 1)
        {
            capacity = (last+1)*blocklen;
        }
        else
        {
            return 0;
        }
        memset((void*)&BS, 0, sizeof(FAT16_BPB_t));
        BS.BS_jmpBoot[0] = 0xEB;
        BS.BS_jmpBoot[1] = 0x3C;
        BS.BS_jmpBoot[2] = 0x90;
        BS.BS_OEMName[0] = 0x4D;
        BS.BS_OEMName[1] = 0x53;
        BS.BS_OEMName[2] = 0x44;
        BS.BS_OEMName[3] = 0x4F;
        BS.BS_OEMName[4] = 0x53;
        BS.BS_OEMName[5] = 0x35;
        BS.BS_OEMName[6] = 0x2E;
        BS.BS_OEMName[7] = 0x30;

		BytsPerSec = SECTORSIZE;
        BS.BPB_BytsPerSec[0] = BytsPerSec&0x00FF;
        BS.BPB_BytsPerSec[1] = (BytsPerSec&0xFF00)>>8;
		    
        if ( capacity <= 32680*512 ) BS.BPB_SecPerClus = 0x02;
        else if ( capacity> 32680*512 && capacity<= 262144*512 ) BS.BPB_SecPerClus = 0x04;
        else if ( capacity> 262144*512 && capacity<= 524288*512 ) BS.BPB_SecPerClus = 0x08;
        else if ( capacity> 524288*512 && capacity<= 1048576*512 ) BS.BPB_SecPerClus = 0x10;

        ResvdSecCnt = 0x0001;
        BS.BPB_ResvdSecCnt[0] = 0x01; //0x0001;
        BS.BPB_ResvdSecCnt[1] = 0x0;

        BS.BPB_NumFATs = 0x02;

        RootEntCnt = 0x0200;
        BS.BPB_RootEntCnt[0] = 0x00; //0x0200; // for max compatibility
        BS.BPB_RootEntCnt[1] = 0x02; 
    
        if (capacity > 32*1048576) 
            TotSec16 = 0x00;
        else 
        {
            TotSec16 = (unsigned short)(capacity>>log2bin_f(BytsPerSec));
        }
        BS.BPB_TotSec16[0] = TotSec16&0x00FF;
        BS.BPB_TotSec16[1] = (TotSec16&0xFF00)>>8;

        BS.BPB_Media = 0xF8;

        RootDirSecs = ((RootEntCnt<<5)+(BytsPerSec-1))>>log2bin_f(BytsPerSec); //verified
        TMP1 = (last+1) - (ResvdSecCnt+RootDirSecs); 
        TMP2 = (BS.BPB_SecPerClus<<8) + BS.BPB_NumFATs;        
        FATSz16 = (unsigned short)((((TMP1 + (TMP2-1))/TMP2)<<16)>>16);
        BS.BPB_FATSz16[0] = FATSz16&0x00FF;
        BS.BPB_FATSz16[1] = (FATSz16&0xFF00)>>8;

        BS.BPB_SecPerTrk[0] = 0x00 ;
        BS.BPB_SecPerTrk[1] = 0x00 ;

        BS.BPB_NumHeads[0] = 0xFF;
        BS.BPB_NumHeads[1] = 0x0;

        BS.BPB_HiddSec[0] = 0x00;
        BS.BPB_HiddSec[1] = 0x00;
        BS.BPB_HiddSec[2] = 0x00;
        BS.BPB_HiddSec[3] = 0x00;

        TotSec32 = capacity<<log2bin_f(BytsPerSec);
        BS.BPB_TotSec32[0] = TotSec32&0xFF;
        BS.BPB_TotSec32[1] = (TotSec32&0xFF00)>>8;
        BS.BPB_TotSec32[3] = (TotSec32&0xFF0000)>>16;
        BS.BPB_TotSec32[4] = (TotSec32&0xFF000000)>>24;

        BS.BS_DrvNum = 0x00;
        BS.BS_BootSig = 0x29;
        BS.BS_SN[0] = 0x00;
        BS.BS_SN[1] = 0x00;
        BS.BS_SN[2] = 0x00;
        BS.BS_SN[3] = 0x00;
        //BS.BS_VolName, "NUCLEUSDisk",11);
        BS.BS_VolName[0] = 'M';
        BS.BS_VolName[1] = 'M';
        BS.BS_VolName[2] = 'P';
        BS.BS_VolName[3] = '3';
        BS.BS_VolName[4] = '6';
        BS.BS_VolName[5] = '0';
        BS.BS_VolName[6] = '-';
        BS.BS_VolName[7] = 'M';
        BS.BS_VolName[8] = 'M';
        BS.BS_VolName[9] = 'C';
        BS.BS_VolName[10] = ' ';
//      BS.BS_FilSysType=, "FAT16   ",8);
        BS.BS_FilSysType[0] = 'F';
        BS.BS_FilSysType[1] = 'A';
        BS.BS_FilSysType[2] = 'T';
        BS.BS_FilSysType[3] = '1';
        BS.BS_FilSysType[4] = '6';
        BS.BS_FilSysType[5] = ' ';
        BS.BS_FilSysType[6] = ' ';
        BS.BS_FilSysType[7] = ' ';
        memset((void*)&BS.BS_ExeCode, 0, 448);
        BS.signature1 = 0x55;
        BS.signature2 = 0xAA;

        return 1;
}


