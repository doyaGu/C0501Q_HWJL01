#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

#include "ts_packet_analysis.h"

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

#if (_MSC_VER)
    #include "ts_txt_conv.h"
    #include "locale.h"
    #define _show_srvc_txt(srvc_txt, srvc_txt_length) \
                do{ wchar_t tmp_txt[512] = {0}; \
                    tsTxt_ConvToUtf16(tmp_txt, srvc_txt, srvc_txt_length, 0); \
                    setlocale(LC_ALL, "cht");   \
                    wprintf(L"- %s\n", tmp_txt); \
                }while(0)
#elif 0
    #include "ts_txt_conv.h"
    #include "ts_demuxer/ts_debug.h"
    #define _show_srvc_txt(srvc_txt, srvc_txt_length) \
                do{ char utf8_txt[256] = {0};   \
                    uint16_t utf16_txt[256] = {0}; \
                    unsigned int bytes_used = 0;    \
                    tsTxt_ConvToUtf16(utf16_txt, srvc_txt, srvc_txt_length, 0); \
                    utf16le_to_utf8(utf16_txt, utf8_txt, 256, &bytes_used); \
                    printf("%s", utf8_txt);    \
                }while(0)
#else
    #define _show_srvc_txt(srvc_txt, srvc_txt_length)
#endif

#define CA_DESCRIPTOR                       (0x9)

typedef enum STREAM_TYPE_TAG {
    ISO_IEC_RESERVED                = 0x00,
    ISO_IEC_11172_2_VIDEO           = 0x01,
    ISO_IEC_13818_2_VIDEO           = 0x02,
    ISO_IEC_11172_3_AUDIO           = 0x03,
    ISO_IEC_13818_3_AUDIO           = 0x04,
    ISO_IEC_13818_1_PRIVATE_SECTION = 0x05,
    ISO_IEC_13818_1_PES             = 0x06,
    ISO_IEC_13522_MHEG              = 0x07,
    ANNEX_A_DSM_CC                  = 0x08,
    ITU_T_REC_H_222_1               = 0x09,
    ISO_IEC_13818_6_TYPE_A          = 0x0A,
    ISO_IEC_13818_6_TYPE_B          = 0x0B,
    ISO_IEC_13818_6_TYPE_C          = 0x0C,
    ISO_IEC_13818_6_TYPE_D          = 0x0D,
    ISO_IEC_13818_1_AUXILIARY       = 0x0E,
    ISO_IEC_13818_7_AUDIO           = 0x0F,
    ISO_IEC_14496_2_VISUAL          = 0x10,
    ISO_IEC_14496_3_AUDIO           = 0x11,
    ISO_IEC_14496_10_VIDEO          = 0x1B,
    ISO_IEC_13818_1_RESERVED        = 0x1C,
    USER_PRIVATE                    = 0x80
} STREAM_TYPE;
//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================
static uint32_t
Pat_User_Recv(
    TSPA_USER_MBOX_ARG  *pUser_mbox_arg,
    void                *extraData)
{
    uint32_t        result = 0;

    do{
        TS_PACKET_ANAL      *pHTspa = 0;
        PSI_PAT_INFO        *pPat_Info = 0;
        uint32_t            totalSrvcCnt = 0;
        static uint32_t     pat_version_number = -1;

        if( !pUser_mbox_arg )       _err("Null arg !!");

        pHTspa    = (TS_PACKET_ANAL*)pUser_mbox_arg->arg.pat.pTunnelInfo;
        pPat_Info = (PSI_PAT_INFO*)pUser_mbox_arg->arg.pat.pPat_info;

        if( !pHTspa || !pPat_Info ||
            pPat_Info->version_number == pat_version_number )
            break;

        pHTspa->bReceivePat = true;

        if( pPat_Info->version_number == pat_version_number )
            break;

        {
            //infoMgr_UpdatePat((void*) ptPatInfo);
            PSI_PAT_PROGRAM  *pCurProgram = 0;

            for( pCurProgram = pPat_Info->pFirstProgram;
                 pCurProgram != 0;
                 pCurProgram = pCurProgram->pNextProgram )
            {
                if( pCurProgram->program_number )   totalSrvcCnt++;
            }
        }


        printf("    PAT get total service %d \n", totalSrvcCnt);

    }while(0);

    return result;
}

typedef struct USER_PID_INFO_T
{
    uint32_t            pmt_pid;
    uint32_t            version_number;
    uint32_t            programNumber;
    uint32_t            videoPID;
    uint32_t            videoType;
    uint16_t            audioCount;
    uint32_t            audioPID[8];

}USER_PID_INFO;

static uint32_t
Pmt_User_Recv(
    TSPA_USER_MBOX_ARG  *pUser_mbox_arg,
    void                *extraData)
{
    uint32_t                result = 0;
    static USER_PID_INFO    userPidInfo[100] = {0};
    static uint32_t         cur_index = 0;
    do{

        TS_PACKET_ANAL      *pHTspa = 0;
        PSI_PMT_INFO        *pPmt_Info = 0;
        PSI_DESCR           *ptDescriptor = 0;
        uint32_t            j, audioIdx = 0;
        static uint32_t     pmt_version_number = -1;

        if( !pUser_mbox_arg )       _err("Null arg !!");

        pHTspa    = (TS_PACKET_ANAL*)pUser_mbox_arg->arg.pmt.pTunnelInfo;
        pPmt_Info = (PSI_PMT_INFO*)pUser_mbox_arg->arg.pmt.pPmt_info;

        if( !pHTspa || !pPmt_Info )     break;

        // analysis PMT
        if( pPmt_Info->pid != PAT_PID && pPmt_Info->pid != NIT_PID
         && pPmt_Info->pid != SDT_PID && pPmt_Info->pid != EIT_PID
         && pPmt_Info->pid != TDT_TOT_PID )
        {
            PSI_PMT_ES_INFO         *pCurEsInfo = 0;
            PSI_DESCR               *pDescriptor = 0;
            bool                    bLoop_break = false;

            for(j = 0; j < cur_index; j++)
            {
                if( userPidInfo[j].programNumber == pPmt_Info->program_number )
                    bLoop_break = true;
            }

            if( bLoop_break == true &&
                pPmt_Info->version_number == pmt_version_number )
                break;

            // skip CA system
            for( ptDescriptor = pPmt_Info->ptFirstDescriptor;
                 ptDescriptor;
                 ptDescriptor = ptDescriptor->ptNextDescriptor )
            {
                if (CA_DESCRIPTOR == ptDescriptor->descriptor_tag)
                    return result;
            }

            memset(&userPidInfo[cur_index], 0x0, sizeof(USER_PID_INFO));

            for( pCurEsInfo = pPmt_Info->ptFirstEsInfo;
                 pCurEsInfo != 0;
                 pCurEsInfo = pCurEsInfo->ptNexEsInfo )
            {
                uint32_t    esPid = pCurEsInfo->elementary_PID;
                uint32_t    audioAc3ChkNum = 0;

                switch( pCurEsInfo->stream_type )
                {
                    case ISO_IEC_11172_2_VIDEO:   // mpeg1 video
                    case ISO_IEC_13818_2_VIDEO:   // mpeg2 video
                    case ISO_IEC_14496_10_VIDEO:  // avc (h.264) video
                        userPidInfo[cur_index].videoPID  = esPid;
                        userPidInfo[cur_index].videoType = pCurEsInfo->stream_type;
                        break;

                    case ISO_IEC_11172_3_AUDIO:  // mpeg1 audio
                    case ISO_IEC_13818_3_AUDIO:  // mpeg2 audio
                        if( audioIdx+1 < 8 )
                        {
                            userPidInfo[cur_index].audioPID[audioIdx] = esPid;
                            audioIdx++;
                        }
                        break;

                    case ISO_IEC_13818_7_AUDIO:
                    case ISO_IEC_14496_3_AUDIO:
                        // aac latm,adts audio
                        if( audioIdx+1 < 8 )
                        {
                            userPidInfo[cur_index].audioPID[audioIdx] = (uint16_t)pCurEsInfo->elementary_PID;
                            audioIdx++;
                        }
                        break;

                    default:
                        pDescriptor = pCurEsInfo->ptFirstDescriptor;
                        while( pDescriptor )
                        {
                            switch( pDescriptor->descriptor_tag )
                            {
                                case 0x6A:  // AC3_descriptor
                                case 0x7A:  // enhanced_AC3_descriptor
                                    audioAc3ChkNum++;
                                    break;
                                case 0x50:  // component_descriptor
                                    if (0x04 == (*pDescriptor->pPayload & 0x0F)) // reserved for AC3
                                        audioAc3ChkNum++;
                                    break;
                                case 0x5: // registration_descriptor
                                    if( pDescriptor->descriptor_length >= 4 )
                                    {
                                        uint32_t        format_identifier = 0;
                                        format_identifier = *((uint32_t*) pDescriptor->pPayload);
                                        if( format_identifier == 0x41432D33 )
                                            audioAc3ChkNum++;
                                    }
                                    break;
                                case 0x59:
                                    if( 0x06 == pCurEsInfo->stream_type &&
                                        pDescriptor->pPayload )
                                    {
                                    }
                                    break;
                            }

                            pDescriptor = pDescriptor->ptNextDescriptor;
                        }

                        if( audioAc3ChkNum )
                        {
                            if( audioIdx+1 < 8 )
                            {
                                userPidInfo[cur_index].audioPID[audioIdx] = esPid;
                                audioIdx++;
                            }
                            audioAc3ChkNum = 0;
                        }
                        break;
                }
            }

            userPidInfo[cur_index].audioCount    = audioIdx;
            userPidInfo[cur_index].pmt_pid       = pPmt_Info->pid;
            userPidInfo[cur_index].programNumber = pPmt_Info->program_number;

        }

        pmt_version_number = pPmt_Info->version_number;

        printf("  pmtPid= 0x%x, program_num=0x%x\n",
                    userPidInfo[cur_index].pmt_pid,
                    userPidInfo[cur_index].programNumber);
        printf("    vid= 0x%x\n", userPidInfo[cur_index].videoPID);
        for(j = 0; j < userPidInfo[cur_index].audioCount; j++)
            printf("    aid= 0x%x\n", userPidInfo[cur_index].audioPID[j]);

        cur_index++;

    }while(0);

    return result;
}

static uint32_t
Sdt_User_Recv(
    TSPA_USER_MBOX_ARG  *pUser_mbox_arg,
    void                *extraData)
{
    uint32_t        result = 0;

    do{
        TS_PACKET_ANAL      *pHTspa = 0;
        PSI_SDT_INFO        *pSdt_Info = 0;
        PSI_SDT_SERVICE     *pCurService = 0;
        PSI_DESCR           *pDescriptor = 0;
        uint8_t             *pCurAddr = 0;
        static uint32_t     cur_version_number = -1;

        if( !pUser_mbox_arg )       _err("Null arg !!");

        pHTspa    = (TS_PACKET_ANAL*)pUser_mbox_arg->arg.sdt.pTunnelInfo;
        pSdt_Info = (PSI_SDT_INFO*)pUser_mbox_arg->arg.sdt.pSdt_info;

        if( !pHTspa || !pSdt_Info ||
            pSdt_Info->version_number == cur_version_number )
            break;

        for( pCurService = pSdt_Info->ptFirstService;
             pCurService;
             pCurService = pCurService->ptNextService )
        {
            pDescriptor = pCurService->ptFirstDescriptor;
            while( pDescriptor )
            {
                // 0x48 = service description tag
                if( 0x48 == pDescriptor->descriptor_tag )
                {
                    // service type coding
                    // (ETSI EN 300 468 V1.12.1 section 6.2.33)
                    #define DIGITAL_TELEVISION_SERVICE                  0X01
                    #define DIGITAL_RADIO_SOUND_SERVICE                 0X02
                    #define ADVANCED_SD_DIGITAL_TELEVISION_SERVICE      0X16
                    #define ADVANCED_HD_DIGITAL_TELEVISION_SERVICE      0X19

                    uint32_t    nameSize = 0;
                    uint8_t     service_type = 0xFF;

                    pCurAddr = pDescriptor->pPayload;

                    service_type = pCurAddr[0];

                    pCurAddr += (2 + pCurAddr[1]);
                    nameSize = pCurAddr[0];
                    pCurAddr += 1;

                    if( (service_type == DIGITAL_TELEVISION_SERVICE ||
                         service_type == DIGITAL_RADIO_SOUND_SERVICE ||
                         service_type == ADVANCED_SD_DIGITAL_TELEVISION_SERVICE ||
                         service_type == ADVANCED_HD_DIGITAL_TELEVISION_SERVICE) &&
                         nameSize > 0 &&
                         nameSize < 256 )
                    {
                        _show_srvc_txt(pCurAddr, nameSize);
                        printf("   sdt_program_num= 0x%x\n", pCurService->service_id);
                        cur_version_number = pSdt_Info->version_number;
                    }
                }
                pDescriptor = pDescriptor->ptNextDescriptor;
            }
        }

        if( pHTspa->bReceiveSdt == false )
            pHTspa->bReceiveSdt = true;

    }while(0);

    return result;
}

static void
testFunc(void)
{
    TS_PACKET_ANAL      *pHTspa = 0;
    TSPA_SETUP_PARAM    setupParam = {0};
    uint8_t             *pPacketBuf = 0, *pStreamBuf = 0;

    FILE                *fp = 0;
    uint32_t            packet_size = 188*100;
    int                 cnt = 0;

    if( !(fp = fopen("./mux1-cp.ts", "rb")) )
        _err("open file fail !!");

    if( !(pStreamBuf = malloc(packet_size)) )
        _err("malloc fail !!");

    setupParam.bInScan  = true;
    setupParam.bWaitNit = false;
    tspa_CreateHandle(&pHTspa, &setupParam, 0);

    pHTspa->pat_user_mbox.func                                   = Pat_User_Recv;
    pHTspa->pat_user_mbox.tsps_user_mbox_arg.type                = TSPS_USER_MBOX_TYPE_PAT;
    pHTspa->pat_user_mbox.tsps_user_mbox_arg.arg.pat.pTunnelInfo = (void*)pHTspa;

    pHTspa->pmt_user_mbox.func                                   = Pmt_User_Recv;
    pHTspa->pmt_user_mbox.tsps_user_mbox_arg.type                = TSPS_USER_MBOX_TYPE_PMT;
    pHTspa->pmt_user_mbox.tsps_user_mbox_arg.arg.pmt.pTunnelInfo = (void*)pHTspa;

    pHTspa->sdt_user_mbox.func                                   = Sdt_User_Recv;
    pHTspa->sdt_user_mbox.tsps_user_mbox_arg.type                = TSPS_USER_MBOX_TYPE_SDT;
    pHTspa->sdt_user_mbox.tsps_user_mbox_arg.arg.sdt.pTunnelInfo = (void*)pHTspa;

    while( !feof(fp) )
    {
        memset(pStreamBuf, 0xaa, sizeof(packet_size));
        fread(pStreamBuf, 1, packet_size, fp);

        cnt = 0;
        while( cnt < 100 )
        {
            pPacketBuf = pStreamBuf + (188*cnt);
            tspa_Analysis(pHTspa, pPacketBuf, 0);
            cnt++;
        }
    }

    tspa_DestroyHandle(&pHTspa, 0);

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

    testFunc();
    while(1);

    return 0;
}