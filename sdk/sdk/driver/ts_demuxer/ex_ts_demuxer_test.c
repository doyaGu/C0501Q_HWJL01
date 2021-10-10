#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "pthread.h"
#include "ts_demuxer/ite_ts_demuxer.h"

//=============================================================================
//                  Constant Definition
//=============================================================================

//=============================================================================
//                  Macro Definition
//=============================================================================
#undef _err
#if (_MSC_VER)
    #define _err(string, args, ...)           do{ printf(string, __VA_ARGS__); \
                                                  printf("  %s [#%d]\n", __FILE__, __LINE__); \
                                                  while(1); \
                                                }while(0)
    #ifndef trace
    #define trace(string, args, ...)          do{ printf(string, __VA_ARGS__); \
                                                    printf("  %s [#%d]\n", __FILE__, __LINE__); \
                                                }while(0)
    #endif

#else
    #define _err(string, args...)           do{ printf(string, ## args); \
                                                  printf("  %s [#%d]\n", __FILE__, __LINE__); \
                                                  while(1); \
                                                }while(0)
    #ifndef trace
    #define trace(string, args...)          do{ printf(string, ## args); \
                                                printf("  %s [#%d]\n", __FILE__, __LINE__); \
                                              }while(0)
    #endif
#endif
//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================
static int32_t
_output_open(
    TSD_INFO_REPO   *pTsdInfoRepo,
    void            *extraData)
{
    static FILE     *fout = 0;
    uint32_t        totalSrvc = 0;
    uint32_t        totalChnls = 0;

    if( !(fout = fopen("./ts_info.cfg", "w")) )
        _err("open fail !!");

    totalChnls = pTsdInfoRepo->totalChnls;
    totalSrvc  = pTsdInfoRepo->totalSrvc;

    fwrite(&totalChnls, 1, sizeof(uint32_t), fout);
    fwrite(&totalSrvc, 1, sizeof(uint32_t), fout);

    pTsdInfoRepo->privInfo = (void*)fout;
    return 0;
}

static int32_t
_input_open(
    TSD_INFO_REPO   *pTsdInfoRepo,
    void            *extraData)
{
    static FILE     *fout = 0;
    uint32_t        totalSrvc = 0;
    uint32_t        totalChnls = 0;

    if( !(fout = fopen("./ts_info.cfg", "r")) )
        _err("open fail !!");

    fread(&totalChnls, 1, sizeof(uint32_t), fout);
    fread(&totalSrvc, 1, sizeof(uint32_t), fout);

    pTsdInfoRepo->totalChnls = totalChnls;
    pTsdInfoRepo->totalSrvc = totalSrvc;

    pTsdInfoRepo->privInfo = (void*)fout;
    return 0;
}

static int32_t
_ouput_close(
    TSD_INFO_REPO   *pTsdInfoRepo,
    void            *extraData)
{
    FILE     *fout = 0;

    fout = pTsdInfoRepo->privInfo;
    if( fout )  fclose(fout);
    return 0;
}

static uint32_t
_export_info(
    TSD_INFO_REPO   *pTsdInfoRepo,
    uint8_t         *buf,
    uint32_t        byteSize)
{
    FILE     *fout = 0;
    uint32_t rst = 0;

    fout = pTsdInfoRepo->privInfo;
    if( fout )  rst = fwrite(buf, 1, byteSize, fout);

    return rst;
}

static uint32_t
_import_info(
    TSD_INFO_REPO   *pTsdInfoRepo,
    uint8_t         *buf,
    uint32_t        byteSize)
{
    FILE     *fout = 0;
    uint32_t rst = 0;
    fout = pTsdInfoRepo->privInfo;
    if( fout )  rst = fread(buf, 1, byteSize, fout);

    return rst;
}

FILE        *f_ts_scan = 0;
uint32_t    ts_scan_file_size = 0;

static void
testFunc(void)
{
    uint8_t     *tsi_buf = 0;
    uint32_t    tsi_buf_size = (200 << 10);

    if( f_ts_scan = fopen("./Greece_30_12_09_1st_stream.ts", "rb") )
    {
        TSD_SCAN_PARAM      scanParm = {0};
        TSD_HANDLE          *pHTsd = 0;
        TSD_SAMPLE_INFO     sampleInfo[4] = {0};
        TSD_INFO_REPO       tsdInfoRepo = {0};
        TSD_PRE_SET_INFO    presetInfo = {0};
        uint8_t             *pPesVBuf = {0};
        uint8_t             *pPesABuf = {0};
        uint32_t            cnt = 0;

        uint32_t    pesSampleBufSize = (512 << 10);
        uint32_t    pesSampleBufCnt  = 4;

        fseek(f_ts_scan, 0, SEEK_END);
        ts_scan_file_size = ftell(f_ts_scan);
        fseek(f_ts_scan, 0, SEEK_SET);

        {
            /////////////////////////////////////
            // ->   pesSampleBufSize     <-
            // +-------------------+------+
            // |                          | 1
            // +-------------------+------+
            // +-------------------+------+
            // |                          | 2
            // +-------------------+------+
            //              :
            // +-------------------+------+
            // |                          | pesSampleBufCnt
            // +-------------------+------+
            /////////////////////////////////////

            if( pPesVBuf = malloc(pesSampleBufSize * pesSampleBufCnt) )
                memset(pPesVBuf, 0x0, pesSampleBufSize * pesSampleBufCnt);
            else
                _err(" malloc fail !!");

            if( pPesABuf = malloc(pesSampleBufSize * pesSampleBufCnt) )
                memset(pPesABuf, 0x0, pesSampleBufSize * pesSampleBufCnt);
            else
                _err(" malloc fail !!");
        }

        presetInfo.tsRecevier.tsrx_type = TSD_TSRX_TYPE_TSI;
        presetInfo.tsdOutMode   = TSD_OUT_PES_DATA;
        presetInfo.tsdDemodType = 0;
        presetInfo.demod_id     = 0;
        presetInfo.tsi_id       = 0;

        presetInfo.bOnPesOutput = true;
        presetInfo.pesOutBuf_a.pBufAddr = pPesABuf;
        presetInfo.pesOutBuf_a.pitch    = pesSampleBufSize;
        presetInfo.pesOutBuf_a.heigth   = pesSampleBufCnt;
        presetInfo.pesOutBuf_v.pBufAddr = pPesVBuf;
        presetInfo.pesOutBuf_v.pitch    = pesSampleBufSize;
        presetInfo.pesOutBuf_v.heigth   = pesSampleBufCnt;

        tsd_CreateHandle(&pHTsd, &presetInfo, 0);

        scanParm.scanFrequency = 557;
        scanParm.bandwidth     = 6000;
        scanParm.countryId     = TSD_COUNTRY_TAIWAN;
#if 1
        tsd_Scan_Channel(pHTsd, TSD_FREQ_SCAN_MANUAL, &scanParm, 1);
#else
        tsd_Scan_Channel(pHTsd, TSD_FREQ_SCAN_AUTO, &scanParm, 0);
#endif
        //----------------------------------
        // export
        tsdInfoRepo.tsd_repo_open  = _output_open;
        tsdInfoRepo.tsd_repo_close = _ouput_close;
        tsdInfoRepo.tsd_repo_export = _export_info;
        tsd_Control(pHTsd, TSD_CTRL_EXPORT_INFO, 0, (void*)&tsdInfoRepo);

        // import
        tsdInfoRepo.tsd_repo_open  = _input_open;
        tsdInfoRepo.tsd_repo_close = _ouput_close;
        tsdInfoRepo.tsd_repo_import = _import_info;
        tsd_Control(pHTsd, TSD_CTRL_IMPORT_INFO, 0, (void*)&tsdInfoRepo);
        //------------------------------------

        tsd_Control(pHTsd, TSD_CTRL_SORT_SRVC_INFO, (uint32_t*)TSD_INFO_SORT_FREQ, 0);

        //---------------------------------------
        // dvb text conver
        if( 0 )
        {
            TSD_SRVC_USER_INFO  *pSrvcUserInfo = 0;
            wchar_t             service_name[256] = {0};
            TSD_TXT_CONV        tsdTxtConv = {0};

            tsd_Get_ServiceInfo(pHTsd, 1, &pSrvcUserInfo, 0);

            tsdTxtConv.dvbTxt         = pSrvcUserInfo->serviceName;
            tsdTxtConv.dvbTxtLength   = pSrvcUserInfo->nameSize;
            tsdTxtConv.utf16Txt       = service_name;
            tsdTxtConv.utf16TxtLength = 256;
            tsd_Conv_Text(pHTsd, &tsdTxtConv, 0);
        }
        //---------------------------------------

        tsd_Change_Service(pHTsd, 1, 0); // select action service

        cnt = 0;
        while( cnt < 20 ) //1)
        {
            int     j;
            tsd_Get_Sample(pHTsd, (TSD_SAMPLE_VIDEO | TSD_SAMPLE_AUDIO), &sampleInfo, 0);
            for(j = 0; j < 4; j++)
            {
                TSD_SAMPLE_INFO     *pSampleInfo = &sampleInfo[j];
                switch( pSampleInfo->type )
                {
                    case TSD_SAMPLE_VIDEO:
                        if( pSampleInfo->sampleAddr )
                            printf("V: sampleAddr=0x%08x, sampleSize=%d\n", pSampleInfo->sampleAddr, pSampleInfo->sampleSize);
                        break;
                    case TSD_SAMPLE_AUDIO:
                        if( pSampleInfo->sampleAddr )
                            printf("A: sampleAddr=0x%08x, sampleSize=%d\n", pSampleInfo->sampleAddr, pSampleInfo->sampleSize);
                        break;
                }
            }
            cnt++;
        }

        tsd_DestroyHandle(&pHTsd);

        if( pPesVBuf )   free(pPesVBuf);
        if( pPesABuf )   free(pPesABuf);
    }
    else
    {
        _err("open ts fail !! ");
    }
    return;
}

//=============================================================================
//                  Public Function Definition
//=============================================================================
int
main(int argc, char    **argv)
{

    //pthread_t t1; //, t2;

#ifdef PTW32_STATIC_LIB
pthread_win32_process_attach_np();
#endif

    testFunc();

#ifdef PTW32_STATIC_LIB
pthread_win32_process_detach_np();
#endif

  return 0;

}
