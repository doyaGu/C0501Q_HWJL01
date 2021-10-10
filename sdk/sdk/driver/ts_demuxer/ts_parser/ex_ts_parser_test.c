
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "ts_demuxer_defs.h"
#include "ts_parser.h" // it should not be inlcude by AP layer
//=============================================================================
//				  Constant Definition
//=============================================================================

//=============================================================================
//				  Macro Definition
//=============================================================================

//=============================================================================
//				  Structure Definition
//=============================================================================

//=============================================================================
//				  Global Data Definition
//=============================================================================

//=============================================================================
//				  Private Function Definition
//=============================================================================
static int
_test_parse_ts(
    void)
{
    FILE            *f_ts[4] = {0};
    uint32_t        ts_file_size = 0;
    uint8_t         *tsi_buf = 0;
    uint32_t        tsi_buf_size = (200 << 10);
    TSP_HANDLE      *pHTsp = 0;
    TS_SRVC_HANDLE  *pHSrvc = 0;
    uint32_t        read_size = 0;
    TS_SERVICE_INFO *pSrvc_info;
    TS_CHNL_HANDLE  *pHChnl = 0;
    uint32_t        curChnlIdx = 0;
    uint32_t        freq[4] = {533, 655, 778, 127};
    uint32_t        bw[4] = {6000, 5000, 7000, 9000};
    int             i;

    if( !(f_ts[0] = fopen("./Greece_30_12_09_1st_stream.ts", "rb")) )
        _err("open  fail !");
    if( !(f_ts[1] = fopen("./TS_690000KHz_8M_20110420_1716.ts", "rb")) )
        _err("open  fail !");
    if( !(f_ts[2] = fopen("./TS_730000KHz_8M_20110414_2331.ts", "rb")) )
        _err("open  fail !");
    if( !(f_ts[3] = fopen("./TS_746000KHz_8M_20110414_2336.ts", "rb")) )
        _err("open  fail !");

    if( tsi_buf = tsd_malloc(tsi_buf_size) )
        memset(tsi_buf, 0, sizeof(tsi_buf_size));
    else
        _err("malloc fail !!");

    tsChnl_CreateHandle(&pHChnl, 0);
    tsSrvc_CreateHandle(&pHSrvc, 0);

    for(i = 0; i < 4; i++)
    {
        TS_CHNL_INFO    curChnlInfo = {0};

        fseek(f_ts[i], 0, SEEK_END);
        ts_file_size = ftell(f_ts[i]);
        fseek(f_ts[i], 0, SEEK_SET);
       
        // if demod lock, add channel info to database
        curChnlInfo.frequency = freq[i];
        curChnlInfo.bandwidth = bw[i];
        tsChnl_AddChannel(pHChnl, &curChnlInfo, &curChnlIdx, 0);

        tsp_CreateHandle(&pHTsp, 0);

        tsp_Control(pHTsp, TSP_CTRL_ATTACH_SRVC_HANDLE, 0, (void*)pHSrvc);

        tsSrvc_Set_InScanStatus(true);
        pHTsp->pTsChnlInfo = &curChnlInfo;

        // start psi parsing
        while( ts_file_size )
        {
            read_size = fread(tsi_buf, 1, tsi_buf_size, f_ts[i]);
            ts_file_size -= read_size;

            pHTsp->bRingBuf   = 0;
            pHTsp->dataBuf  = tsi_buf;
            pHTsp->dataSize = read_size;

            tsp_ParseStream(pHTsp, 0);

        }

        tsp_DestroyHandle(&pHTsp);
        fclose(f_ts[i]);
    }

    tsSrvc_GetServiceInfo(pHSrvc, 1, &pSrvc_info, 0);
    tsSrvc_DestroyHandle(&pHSrvc);
    tsChnl_DestroyHandle(&pHChnl);
    _err("end \n");
}
