#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "pthread.h"
#include "ts_extract.h"

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
#define GEN_FILE_SIZE_MBYTES            3 // n MBytes
#define AGGRE_TAG_LENGTH                1

static TSEXT_PASSPORT_INFO  passport_info[4] =
{
    {AGGRE_TAG_LENGTH, 0x0},
    {AGGRE_TAG_LENGTH, 0x1},
    {AGGRE_TAG_LENGTH, 0x2},
    {AGGRE_TAG_LENGTH, 0x3}
};

//=============================================================================
//                  Private Function Definition
//=============================================================================
static void
_gen_aggre_pattern(void)
{
    unsigned int        tsi_buf_size = (500 << 10);
    unsigned char       *pTsiBuf = 0, *pCur = 0;
    unsigned int        index = 0;
    unsigned int        sect_size = 0, sect_cnt = 0, pkt_cnt = 0;
    FILE                *fp[4] = {0}, *fout = 0, *fp_tmp = 0;

    srand(clock());

    if( !(fp[0] = fopen("./mux1-cp.ts", "rb")) )    _err("open file fail !!");
    if( !(fp[1] = fopen("./mux1-cp.ts", "rb")) )    _err("open file fail !!");
    if( !(fp[2] = fopen("./mux1-cp.ts", "rb")) )    _err("open file fail !!");
    if( !(fp[3] = fopen("./mux1-cp.ts", "rb")) )    _err("open file fail !!");

    if( !(fout = fopen("./aggre_ts_tag1-2-3-4_length_1.ts", "wb")) )    _err("open file fail !!");

    if( !(pTsiBuf = malloc(tsi_buf_size)) )    _err("malloc fail !!");

    sect_cnt = ((GEN_FILE_SIZE_MBYTES << 20) + tsi_buf_size - 1) / tsi_buf_size;
    pkt_cnt = tsi_buf_size / (188 + passport_info[0].tag_len);
    sect_size = pkt_cnt * (188 + passport_info[0].tag_len);

    while( sect_cnt )
    {
        pkt_cnt = tsi_buf_size / (188 + passport_info[0].tag_len);

        pCur = pTsiBuf;
        while( pkt_cnt )
        {
            unsigned int    read_size = 0;
            index = ((rand() >> 3) & 0x3);
            fp_tmp = fp[index];
            read_size = fread(pCur, 1, 188, fp_tmp);
            if( read_size != 188 )
            {
                fseek(fp_tmp, 0, SEEK_SET);
                read_size = fread(pCur, 1, 188, fp_tmp);
            }

            pCur += 188;
            if( passport_info[index].tag_len == 1 )
            {
                *pCur = (unsigned char)passport_info[index].tag_value;
                pCur++;
            }
            else
            {
                memset(pCur, 0xaa, passport_info[index].tag_len);
                *pCur = (unsigned char)((passport_info[index].tag_value & 0xFF000000) >> 24);
                pCur++;
                *pCur = (unsigned char)((passport_info[index].tag_value & 0x00FF0000) >> 16);
                pCur++;
                *pCur = (unsigned char)((passport_info[index].tag_value & 0x0000FF00) >> 8);
                pCur++;
                *pCur = (unsigned char)((passport_info[index].tag_value & 0x000000FF));
                pCur++;
                pCur += (passport_info[index].tag_len - 4);
            }

            pkt_cnt--;
        }

        fwrite(pTsiBuf, 1, sect_size, fout);
        sect_cnt--;
    }

    if( pTsiBuf )          free(pTsiBuf);

    fclose(fout);
    fclose(fp[0]);
    fclose(fp[1]);
    fclose(fp[2]);
    fclose(fp[3]);
    return;
}

static uint32_t
_scan_report(
    TSE_USER_MBOX_ARG   *pUser_mbox_arg,
    void                *extraData)
{
    bool        *pBScan_ready = 0;
    uint32_t    *pPort_index = 0;

    do{
        pBScan_ready = (bool*)pUser_mbox_arg->arg.scan.pTunnelInfo[0];
        pPort_index  = (uint32_t*)pUser_mbox_arg->arg.scan.pTunnelInfo[1];

        *pPort_index = pUser_mbox_arg->arg.scan.port_index;
        *pBScan_ready = true;

        printf(" ------ get scan ready (port = %d)\n", (*pPort_index));
    }while(0);

    return 0;
}

static void
testFunc(void)
{
    TSEXT_HANDLE        *pHTsExt = 0;
    TSEXT_INIT_PARAM    initParam = {0};
    TSEXT_BUF_INFO      buf_info = {0};
    uint8_t             *pPacketBuf = 0, *pStreamBuf = 0;
    uint32_t            port_index = (-1);
    bool                bScan_ready = false;
    FILE                *fp = 0;
    uint32_t            data_size = (500 << 10);
    int                 cnt = 0;

    // if( !(fp = fopen("./mux1-cp.ts", "rb")) )  _err("open file fail !!");
    if( !(fp = fopen("./aggre_ts_tag1-2-3-4_length_1.ts", "rb")) ) _err("open file fail !!");

    if( !(pStreamBuf = malloc(data_size)) )     _err("malloc fail !!");

    initParam.act_pkt_size = 188 + passport_info[0].tag_len;
    initParam.bBy_Pass_Tss = false;
    tsExt_CreateHandle(&pHTsExt, &initParam, 0);

    pHTsExt->tse_scan_state_recv.func                                      = _scan_report;
    pHTsExt->tse_scan_state_recv.tse_user_mbox_arg.type                    = TSE_USER_MBOX_TYPE_SCAN_STATUS;
    pHTsExt->tse_scan_state_recv.tse_user_mbox_arg.arg.scan.pTunnelInfo[0] = (void*)&bScan_ready;
    pHTsExt->tse_scan_state_recv.tse_user_mbox_arg.arg.scan.pTunnelInfo[1] = (void*)&port_index;

    tsExt_Set_Pkt_Proc_Mode(pHTsExt, 0, TSEXT_PKT_PROC_ANALYZE, 0);

    tsExt_Add_Passport_Info(pHTsExt, &passport_info[0], 0);
    tsExt_Add_Passport_Info(pHTsExt, &passport_info[1], 0);
    tsExt_Add_Passport_Info(pHTsExt, &passport_info[2], 0);
    tsExt_Add_Passport_Info(pHTsExt, &passport_info[3], 0);

    while( !feof(fp) )
    {
        static int  cnt = 0;
        uint32_t    stream_data_size = ((rand() >> 3) % data_size);

        memset(pStreamBuf, 0xaa, sizeof(data_size));
        fread(pStreamBuf, 1, stream_data_size, fp);
        //printf("  input size: %8d, %d-th\n", stream_data_size, cnt);

        buf_info.pBufAddr  = pStreamBuf;
        buf_info.bufLength = stream_data_size;
        if( bScan_ready == true )
        {
            tsExt_Set_Pkt_Proc_Mode(pHTsExt, port_index, TSEXT_PKT_PROC_SPLIT, 0);
            bScan_ready = false;
        }

        tsExt_Extract(pHTsExt, &buf_info, 0);

        cnt++;
    }

    {
        TSEXT_PKT_ANAL_INFO     *pCur_Pkt_analyzer_info = 0;
        TSEXT_SAMPLE_INFO       sample_info = {0};
        uint32_t                srvc_idx = 0, port_idx = 0;
        char                    out_name[128] = {0};
        FILE                    *fout = 0;

        // dump service stream
        for(port_idx = 0; port_idx < 4; port_idx++)
        {
            sample_info.port_idx = port_idx;

            pCur_Pkt_analyzer_info = pHTsExt->pPkt_analyzer_info[sample_info.port_idx];
            if( !pCur_Pkt_analyzer_info )   continue;

            for(srvc_idx = 0; srvc_idx < pCur_Pkt_analyzer_info->total_service; srvc_idx++)
            {
                sample_info.service_idx  = srvc_idx;
                sample_info.customer_idx = 0;
                tsExt_Get_Sample(pHTsExt, &sample_info, 0);

                sprintf(out_name, "port-%d_srvc-%d.ts",
                    sample_info.port_idx, srvc_idx);

                if( sample_info.bufLength && sample_info.pBufAddr )
                {
                    if( !(fout = fopen(out_name, "wb")) )   _err("open file fail !!");

                    fwrite(sample_info.pBufAddr, 1, sample_info.bufLength, fout);

                    fclose(fout);
                }
            }
        }
    }

    tsExt_Set_Pkt_Proc_Mode(pHTsExt, port_index, TSEXT_PKT_PROC_IDLE, 0);

    tsExt_DestroyHandle(&pHTsExt, 0);

    if( pStreamBuf )    free(pStreamBuf);
    fclose(fp);

#if (_MSC_VER)
    _CrtDumpMemoryLeaks();
#endif
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
// pthread_win32_process_attach_np();
#endif

    srand(clock());

    _gen_aggre_pattern();

    testFunc();
    while(1);

#ifdef PTW32_STATIC_LIB
// pthread_win32_process_detach_np();
#endif

  return 0;

}