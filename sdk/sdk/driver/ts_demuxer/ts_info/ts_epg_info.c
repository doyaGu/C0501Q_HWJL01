
#include "list_template.h"
#include "ts_epg_info.h"
#include "ts_demuxer_defs.h"
#include "psi_descriptor_kit.h"
#include "psi_descriptor_0x58.h"
//=============================================================================
//				  Constant Definition
//=============================================================================
#define ENABLE_WARNING_MSG      0
#define SHOW_EPG_TXT            0
#define ENABLE_EXTENDED_EVENT   0

#define EXTENDED_EVENT_SUM                      16  // epg extended event

#define INVALID_MJD_TIME                        0x7FFFFFFF
#define LIMIT_SCHEDULE_EVENT_SIZE               819200
#define SCHEDULE_BASE_TBL_ID                    (0x50)

// ETSI EN 300 468 table 12: possible locations of descriptors
#define SHORT_EVENT_DESCRIPTOR                  (0x4D)
#define EXTENDED_EVENT_DESCRIPTOR               (0x4E)
#define PARENTAL_RATING_DESCRIPTOR              (0x55)

typedef enum _EPG_STATUS_TAG
{
    EPG_STATUS_IDLE = 0x11,
    EPG_STATUS_BUSY = 0xBB,
    EPG_STATUS_FAIL = 0xFF, 

}EPG_STATUS;

typedef enum _EPG_FIND_CMP_TYPE_TAG
{
    EPG_FIND_CMP_SEQ_ID         = 0, // sequence node
    EPG_FIND_CMP_SERVICE_ID,
    EPG_FIND_CMP_CUSTOMIZE_0, 
    
}EPG_FIND_CMP_TYPE;

//=============================================================================
//				  Macro Definition
//=============================================================================
#if _MSC_VER && (SHOW_EPG_TXT)
    #include "ts_txt_conv.h"
    #include "locale.h"
    #define _show_epg_txt(epg_txt, epg_txt_length) \
                do{ wchar_t tmp_txt[512] = {0}; \
                    tsTxt_ConvToUtf16(tmp_txt, epg_txt, epg_txt_length, 0); \
                    setlocale(LC_ALL, "cht");   \
                    wprintf(L"- %s\n", tmp_txt); \
                }while(0)
#elif (SHOW_EPG_TXT)
    #include "ts_txt_conv.h"
    #include "ts_demuxer/ts_debug.h"
    #define _show_epg_txt(epg_txt, epg_txt_length) \
                do{ char utf8_txt[256] = {0};   \
                    uint16_t utf16_txt[256] = {0}; \
                    unsigned int bytes_used = 0;    \
                    tsTxt_ConvToUtf16(utf16_txt, epg_txt, epg_txt_length, 0); \
                    utf16le_to_utf8(utf16_txt, utf8_txt, 256, &bytes_used); \
                    printf("%s\n", utf8_txt);    \
                }while(0)
#else
    #define _show_epg_txt(epg_txt, epg_txt_length)
#endif
//=============================================================================
//				  Structure Definition
//=============================================================================
typedef struct _epg_cmp_data_tag
{
    uint32_t    frequency;
    uint32_t    service_id;
    
}epg_cmp_data;

typedef struct _TS_EPG_DB_TAG
{
    TS_EPG_HANDLE       hEpg;

    EPG_STATUS          epgStatus;

    pthread_mutex_t     eit_collect_mutex;
    
    uint32_t            totalService;
    
    TS_1_SRVC_EIT       *pFirst_srvc_eits;
    TS_1_SRVC_EIT       *pCur_srvc_eits;    

    bool                bSkip;    // lock/unlok or other applications
    bool                bSkipEitSchedule;

    uint32_t            curr_freq;

    ISO_639_LANG        preferEventLangB; // set by AP
    ISO_639_LANG        preferEventLangT; // set by AP

    // TDT
    bool                bUpdate_TDT; // force update TDT (UTC time)
    PSI_MJDBCD_TIME     curUtcTime;    

    // TOT
    PSI_LOCAL_TIME_OFFSET_DESCRIPTOR    localTimeOffset;

}TS_EPG_DB;

//=============================================================================
//				  Global Data Definition
//=============================================================================

//=============================================================================
//				  Private Function Definition
//=============================================================================
DEFINE_LIST_TEMPLATE(TS_1_SRVC_EIT);

static int
_Search_Func(
    int             cmpMode,
    TS_1_SRVC_EIT   *pCur, 
    void            *pattern)
{
    int             rst = 0;
    epg_cmp_data    *cmpData = (epg_cmp_data*)pattern;
    
    switch( cmpMode )
    {
        case EPG_FIND_CMP_SERVICE_ID:
            rst = ((pCur->frequency == cmpData->frequency) && (pCur->service_id == cmpData->service_id));
            break;
            
        case EPG_FIND_CMP_SEQ_ID:
            rst = 1;
            break;
    }
    return rst;
}

static int
_Eit_Node_Compare(
    MBT_NODE    *node_base,
    MBT_NODE    *node_other,
    void        *param)
{
    EIT_MBT_NODE   *pEit_node_a = mbt_get_struct(EIT_MBT_NODE, mbt_node, node_base);
    EIT_MBT_NODE   *pEit_node_b = mbt_get_struct(EIT_MBT_NODE, mbt_node, node_other);
    
    return (pEit_node_b->start_time.high16 != pEit_node_a->start_time.high16) ? 
            (pEit_node_b->start_time.high16 - pEit_node_a->start_time.high16) : (pEit_node_b->start_time.low24 - pEit_node_a->start_time.low24);
    
}

static void*
_Eit_Node_Create(
    MBT_NODE_OPRATOR    *oprator,
    uint32_t            struct_size)
{
#define IS_SAME(char4_a, char4_b)   (!(memcmp(char4_a, char4_b, sizeof(char4_a))))
    TS_EPG_DB       *pEpgDb = 0;
    EIT_MBT_NODE    *pEit_node_c = 0;
    PSI_EIT_EVENT   *ptSrcEvent = 0;
    PSI_DESCRIPTOR  *ptDescriptor = 0;
    ISO_639_LANG    emptyEventLang = {0xFF};
    ISO_639_LANG    eventLang = {0};
    ISO_639_LANG    preferEventLang = {0xFF};    
    uint32_t        event_name_length = 0, text_length = 0;
    uint8_t         *event_name = 0, *text = 0;
    uint8_t         *pCur = 0;
    uint32_t        rating = 0;
    uint32_t        nodeSize = 0;
    uint32_t        i;

#if (ENABLE_EXTENDED_EVENT)
    uint32_t        total_extended_text_length = 0;
    uint32_t        extended_text_length[EXTENDED_EVENT_SUM] = {0};
    uint8_t         *extended_text_addr[EXTENDED_EVENT_SUM] = {0};
    uint32_t        length_of_items = 0, index = 0;
#endif

    ptSrcEvent = (PSI_EIT_EVENT*)oprator->privData;
    pEpgDb     = (TS_EPG_DB*)oprator->privData_1;
    
    for(ptDescriptor = ptSrcEvent->ptFirstDescriptor;
        ptDescriptor;
        ptDescriptor = ptDescriptor->ptNextDescriptor)
    {
        pCur = ptDescriptor->pPayload;
        switch( ptDescriptor->descriptor_tag )
        {
            case SHORT_EVENT_DESCRIPTOR:
                // ISO_639_language_code
                for(i = 0; i < 3; i++)     eventLang.charCode[i] = pCur[i]; 
 
                if( IS_SAME(emptyEventLang.charCode, preferEventLang.charCode) ||
                    IS_SAME(eventLang.charCode, pEpgDb->preferEventLangB.charCode) ||
                    IS_SAME(eventLang.charCode, pEpgDb->preferEventLangT.charCode) )
                {
                    for (i = 0; i < 3; i++)     preferEventLang.charCode[i] = eventLang.charCode[i];

                    // event_name and event_name_length
                    event_name_length = pCur[3];
                    pCur += 4;
                    event_name = (event_name_length > 0) ? pCur : 0;
                    pCur += event_name_length;

                    // text and text_length
                    text_length = pCur[0];
                    pCur += 1;
                    text = (text_length > 0) ? pCur : 0;
                }
                break;

            case PARENTAL_RATING_DESCRIPTOR:
                rating = pCur[3];
                rating = ((rating) && (rating < 0x10)) ? rating + 3 : 0;
                break;
                
#if (ENABLE_EXTENDED_EVENT)
            case EXTENDED_EVENT_DESCRIPTOR:
                if( index < EXTENDED_EVENT_SUM )
                {
                    // ISO_639_language_code
                    for(i = 0; i < 3; i++)     eventLang.charCode[i] = pCur[1 + i]; 

                    if( IS_SAME(eventLang.charCode, preferEventLang.charCode) )
                    {
                        length_of_items = pCur[4];
                        if( length_of_items == 0 )
                        {
                            extended_text_length[index] = pCur[5];
                            if( extended_text_length[index] > 0 )
                                extended_text_addr[index] = &pCur[6];
                        }
                        else
                        {
                            extended_text_length[index] = pCur[5 + length_of_items];
                            if( extended_text_length[index] > 0 )
                                extended_text_addr[index] = &pCur[6 + length_of_items];
                        } 

                        if( extended_text_length[index] > 249 )
                            extended_text_length[index] = 249;

                        if( index && extended_text_addr[index] )
                        {
                            if( (*extended_text_addr[index]) == 0x10 )
                            {
                                extended_text_addr[index]   += 3;
                                extended_text_length[index] -= 3;
                            }
                            else
                            {
                                if( (*extended_text_addr[index]) < 0x20 )
                                {
                                    extended_text_addr[index]   += 1;
                                    extended_text_length[index] -= 1;
                                }
                            }
                        }

                        total_extended_text_length += extended_text_length[index];
                        index++;
                    }
                }
                break;
#endif
            default:
                epg_msg_ex(ENABLE_WARNING_MSG, " eit not support descriptor_tag (0x%x) !!", ptDescriptor->descriptor_tag);
                break;
        }
    }

    nodeSize = struct_size + event_name_length + text_length;
    
#if (ENABLE_EXTENDED_EVENT)
    nodeSize += total_extended_text_length;
#endif    
    
    if( nodeSize < LIMIT_SCHEDULE_EVENT_SIZE &&
        (pEit_node_c = (EIT_MBT_NODE*)tsd_malloc(nodeSize)) )
    {
        memset(pEit_node_c, 0x0, nodeSize);
        
        // initial value
        pEit_node_c->start_time        = ptSrcEvent->start_time;
        pEit_node_c->duration          = ptSrcEvent->duration;
        
        pEit_node_c->userInfo.rating   = (uint16_t)rating;
        
        // assign event name
        pEit_node_c->userInfo.nameSize   = (uint8_t)event_name_length;
        pEit_node_c->userInfo.pEventName = (event_name_length > 0) ? ((uint8_t*)pEit_node_c) + struct_size : 0;
        if( pEit_node_c->userInfo.pEventName && event_name )
            memcpy(pEit_node_c->userInfo.pEventName, event_name, event_name_length);

        /*{
            PSI_YMDHMS_TIME     YmdHmsTime = {0};
            
            YmdHmsTime = psiTime_MjdBcdToYmdHms(ptSrcEvent->start_time);
            printf("\t%d/%d/%d, %06x", YmdHmsTime.year, YmdHmsTime.month, YmdHmsTime.day, ptSrcEvent->start_time);
        }//*/
        _show_epg_txt(event_name, event_name_length);

        // assign event descripted text
        pEit_node_c->userInfo.textSize   = (uint8_t)text_length;
        pEit_node_c->userInfo.pText      = (text_length > 0) ? 
                                                ((uint8_t*)pEit_node_c) + struct_size + event_name_length : 0;
        if( pEit_node_c->userInfo.pText && text_length )
            memcpy(pEit_node_c->userInfo.pText, text, text_length);
        
        _show_epg_txt(text, text_length);

#if (ENABLE_EXTENDED_EVENT)
        pEit_node_c->userInfo.extendTextSize = total_extended_text_length;
        pEit_node_c->userInfo.pExtendText    = (event_name_length > 0) ? pEit_node_c->userInfo.pText + text_length : 0;
        if( pEit_node_c->userInfo.pExtendText )
        {
            uint8_t     *pExtendText = pEit_node_c->userInfo.pExtendText;

            for(index = 0;
                (index < EXTENDED_EVENT_SUM) && (extended_text_addr[index] != 0);
                index++)
            {
                memcpy(pExtendText, extended_text_addr[index], extended_text_length[index]);
                pExtendText += extended_text_length[index];
            }
        }        
#endif        
    }

    return (void*)pEit_node_c;
}

static void
_Eit_Node_Destroy(
    MBT_NODE_OPRATOR    *oprator,
    MBT_NODE            *pMbt_c_node)
{
    EIT_MBT_NODE   *pEit_node_c = mbt_get_struct(EIT_MBT_NODE, mbt_node, pMbt_c_node);

    // free member
    
    // destroy eit_node
    free(pEit_node_c);
}

static int
_sort_date(
    const void *parg0,
    const void *parg1)
{
    EPG_1_DAY_SCHEDULE  *pEpg_day_0 = (EPG_1_DAY_SCHEDULE*)parg0;
    EPG_1_DAY_SCHEDULE  *pEpg_day_1 = (EPG_1_DAY_SCHEDULE*)parg1;
    
    return pEpg_day_0->mjd_time - pEpg_day_1->mjd_time;
}

static TS_1_SRVC_EIT*
_Get_Cur_Srvc_Eits(
    TS_EPG_DB   *pEpgDb,
    uint32_t    frequency,
    uint32_t    service_id)
{
    epg_cmp_data    cmpData = {0};
    TS_1_SRVC_EIT   *pNew_srvc_eits = 0;
    
    // search epg db by frequency and service_id
    cmpData.frequency  = frequency;
    cmpData.service_id = service_id;

    do{
        if( pEpgDb->pFirst_srvc_eits )
            pNew_srvc_eits = LIST_FIND(TS_1_SRVC_EIT,
                                       _Search_Func, 
                                       EPG_FIND_CMP_SERVICE_ID, 
                                       pEpgDb->pCur_srvc_eits, 
                                       &cmpData);
        // if current service not exsit in database, create it.
        if( !pNew_srvc_eits )
        {
            int     i;

            if( !(pNew_srvc_eits = tsd_malloc(sizeof(TS_1_SRVC_EIT))) )
                break;

            memset(pNew_srvc_eits, 0, sizeof(TS_1_SRVC_EIT));
            pNew_srvc_eits->version_num = (uint32_t)(-1);
            
            for(i = 0; i < EPG_MAX_SCHEDULE_DAY; i++)
                pNew_srvc_eits->epg_1_day[i].mjd_time = INVALID_MJD_TIME;

            pNew_srvc_eits->frequency  = frequency;
            pNew_srvc_eits->service_id = service_id;
            
            // add to epg service list
            if( !pEpgDb->pFirst_srvc_eits )  
            {
                pEpgDb->pFirst_srvc_eits = pEpgDb->pCur_srvc_eits = pNew_srvc_eits;
                LIST_INIT(pEpgDb->pFirst_srvc_eits);
            }
            else 
            {
                pEpgDb->pCur_srvc_eits = LIST_ADD(TS_1_SRVC_EIT, pEpgDb->pCur_srvc_eits, pNew_srvc_eits);
            }

            pEpgDb->totalService++;
        }
    }while(0); 

    return pNew_srvc_eits;
}

static void
_Update_Eit(
    TS_EPG_DB       *pEpgDb,
    PSI_EIT_INFO    *ptEitInfo,
    TS_1_SRVC_EIT   *pCur_srvc_eits)
{
    do{
        bool                bDoTimeChk = false;
        MBT_NODE_OPRATOR    node_op = {0};
        PSI_EIT_EVENT*      ptInfoEvent = 0;
        PSI_MJDBCD_TIME     curTime = pEpgDb->curUtcTime;
        EPG_1_DAY_SCHEDULE  *pEpg_cur_day = 0;
        EPG_1_DAY_SCHEDULE  *pEpg_tmp_day = 0;
        bool                bSortByDay = false;
        int                 day_offset, i;
        
        if( ptEitInfo->totalEventCount == 0 || !ptEitInfo->ptFirstEvent )
            break;
        
        // assign callback function
        node_op.create  = _Eit_Node_Create;
        node_op.destroy = _Eit_Node_Destroy;
    
        // time check
        bDoTimeChk = (ptEitInfo->table_id == SCHEDULE_BASE_TBL_ID) 
                            && (ptEitInfo->section_number < 64);
                            
        for(ptInfoEvent = ptEitInfo->ptFirstEvent;
            ptInfoEvent;
            ptInfoEvent = ptInfoEvent->ptNextEvent)
        {
            if( bDoTimeChk && PSI_MJDBCD_TIME_LESS(ptInfoEvent->start_time, curTime) )
            {
                PSI_MJDBCD_TIME     tEndTime = {0};
                
                tEndTime = psiTime_Add(ptInfoEvent->start_time, ptInfoEvent->duration);
                if( PSI_MJDBCD_TIME_LESS(tEndTime, curTime) )
                    continue;
            }

            day_offset = (int)ptInfoEvent->start_time.high16 - (int)curTime.high16;
            if( day_offset < 0 || day_offset > (EPG_MAX_SCHEDULE_DAY - 1) )
                continue;

            // destory old (by date) data 
            for(i = 0; i < EPG_MAX_SCHEDULE_DAY; i++)
            {
                pEpg_cur_day = &pCur_srvc_eits->epg_1_day[i];
                
                if( pEpg_cur_day->mjd_time != INVALID_MJD_TIME && 
                    pEpg_cur_day->mjd_time < curTime.high16 )
                {
                    bSortByDay = true;
                    mbt_del_tree(EIT_MBT_NODE, mbt_node, &pEpg_cur_day->pEitMbtRoot, &node_op);
                    memset(pEpg_cur_day, 0, sizeof(EPG_1_DAY_SCHEDULE));
                    pEpg_cur_day->mjd_time = INVALID_MJD_TIME;
                }
            }
            
            if( bSortByDay )
            {
                qsort((void*)pCur_srvc_eits->epg_1_day, EPG_MAX_SCHEDULE_DAY, 
                      sizeof(EPG_1_DAY_SCHEDULE), _sort_date);
                bSortByDay  = false;
            }

            // search empty EPG_1_DAY_SCHEDULE
            pEpg_cur_day = &pCur_srvc_eits->epg_1_day[day_offset];
            pEpg_cur_day->mjd_time = curTime.high16;

            node_op.privData_1 = (void*)pEpgDb; // Tunnel info
            if( !pEpg_cur_day->pEitMbtRoot )
            {
                // the first node
                node_op.privData = (void*)ptInfoEvent; // for initial node member when creating
                mbt_create_node(EIT_MBT_NODE, mbt_node, &pEpg_cur_day->pEitMbtRoot, &node_op);
                if( pEpg_cur_day->pEitMbtRoot )      pEpg_cur_day->total_eits++;
            }
            else
            {
                EIT_MBT_NODE  *pRoot_eit_node = 0;
                EIT_MBT_NODE  *pNew_eit_node = 0;
                EIT_MBT_NODE  pattern_eit_node = {0};
                
                mbt_get_root(EIT_MBT_NODE, mbt_node, pEpg_cur_day->pEitMbtRoot, &pRoot_eit_node);
                pEpg_cur_day->pEitMbtRoot = pRoot_eit_node;
                
                // search eit_node exist or not
                pattern_eit_node.start_time = ptInfoEvent->start_time;
                mbt_find_node(EIT_MBT_NODE, mbt_node, 
                        pEpg_cur_day->pEitMbtRoot, &pattern_eit_node, &pNew_eit_node, _Eit_Node_Compare, 0);

                // del old eit_node data
                if( pNew_eit_node )
                {
                    mbt_del_node(EIT_MBT_NODE, mbt_node, 
                            &pEpg_cur_day->pEitMbtRoot, pNew_eit_node, _Eit_Node_Compare, &node_op);
                    pEpg_cur_day->total_eits--;
                    if( (int)pEpg_cur_day->total_eits < 0 )
                        epg_msg_ex(1, " something wring in counting total eits !!");
                }
                
                // create new_node
                node_op.privData = (void*)ptInfoEvent; // for initial node member when creating
                mbt_create_node(EIT_MBT_NODE, mbt_node, &pNew_eit_node, &node_op);

                // if be verified
                if( pNew_eit_node )
                {
                    if( !pEpg_cur_day->pEitMbtRoot )
                    {
                        pEpg_cur_day->pEitMbtRoot = pNew_eit_node;
                        epg_msg_ex(1, " get new eit_root !!");
                    }
                    else
                    {
                        mbt_add_node(EIT_MBT_NODE, mbt_node, 
                                &pEpg_cur_day->pEitMbtRoot, pNew_eit_node, _Eit_Node_Compare, 0);
                    }
                    
                    pEpg_cur_day->total_eits++;
                }
            }
        }
    }while(0);
    
    return;
}

bool
tsEpg_Eit_SectionFilter_CallBack(
    TS_DEMUX        *ptDemux,
    PSI_SECTION     *ptSection)
{
    bool            bFilterOn = true;
    TS_EPG_DB       *pEpgDb = DOWN_CAST(TS_EPG_DB, ptDemux->pHTsEpg, hEpg);
    TS_1_SRVC_EIT   *pCur_srvc_eits = 0; 

    _trace_enter("\n");
    
    if( ptDemux->pHTsEpg && pEpgDb && pEpgDb->bSkip == true ) 
    {
        epg_msg_ex(ENABLE_WARNING_MSG, " ts epg info skip data !!");
        _trace_leave();
        return bFilterOn;
    }

    if( ptDemux->pHTsEpg && pEpgDb )
    {
        _mutex_lock(TSD_MSG_TYPE_TRACE_EPG, pEpgDb->eit_collect_mutex);
        
        do{
            pCur_srvc_eits = _Get_Cur_Srvc_Eits(pEpgDb, pEpgDb->curr_freq, ptSection->table_id_extension);

            if( !pCur_srvc_eits )       break;
            
            if( ptSection->table_id == EIT_ACTUAL_PRESENT_FOLLOWING_EVENT_TABLE_ID )
            {
                // present/following
                if( (pCur_srvc_eits->version_num_p_f == ptSection->section_number) &&
                    (pCur_srvc_eits->rx_section_p_f & (1UL<<(ptSection->section_number&0x1))) )
                    break;
                    
                pCur_srvc_eits->rx_section_p_f = 0;
            }
            else
            {
                uint32_t        tableIdx = (-1);
                
                // schedule
                tableIdx = ptSection->table_id - SCHEDULE_BASE_TBL_ID;
                if( tableIdx >= EPG_MAX_SCHEDULE_TBL || ptSection->section_number > 255 )
                    break;
                    
                if( (pCur_srvc_eits->version_num_sch[tableIdx] == ptSection->version_number) &&
                    (pCur_srvc_eits->rx_section_sch[tableIdx][ptSection->version_number>>5] 
                        & (1UL<<(ptSection->section_number&0x1F))) )
                    break;
                
                memset(pCur_srvc_eits->rx_section_sch[tableIdx], 0, sizeof(pCur_srvc_eits->rx_section_sch[tableIdx]));
            }
        }while(0);

        _mutex_unlock(TSD_MSG_TYPE_TRACE_EPG, pEpgDb->eit_collect_mutex);
    }

    bFilterOn = false;
    _trace_leave();
    return bFilterOn;
}

void
tsEpg_Eit_P_F_CallBack(
    TS_DEMUX        *ptDemux,
    PSI_EIT_INFO    *ptEitInfo)
{
    TS_EPG_DB       *pEpgDb = DOWN_CAST(TS_EPG_DB, ptDemux->pHTsEpg, hEpg);

    _trace_enter("\n");
    
    if( ptDemux->pHTsEpg && pEpgDb && pEpgDb->bSkip == true ) 
    {
        epg_msg_ex(ENABLE_WARNING_MSG, " ts epg info skip present/following !!");
        _trace_leave();
        return ;
    }

    if( ptDemux->pHTsEpg && pEpgDb )
    {
        _mutex_lock(TSD_MSG_TYPE_TRACE_EPG, pEpgDb->eit_collect_mutex);
        
        do{
            TS_1_SRVC_EIT   *pCur_srvc_eits = 0; 
            
            pCur_srvc_eits = _Get_Cur_Srvc_Eits(pEpgDb, pEpgDb->curr_freq, ptEitInfo->service_id);
            if( !pCur_srvc_eits )       break;

            _Update_Eit(pEpgDb, ptEitInfo, pCur_srvc_eits);

            pCur_srvc_eits->version_num_p_f = (uint8_t)ptEitInfo->version_number;
            pCur_srvc_eits->rx_section_p_f |= (1UL << (ptEitInfo->section_number&0x1));
        }while(0);
        
        _mutex_unlock(TSD_MSG_TYPE_TRACE_EPG, pEpgDb->eit_collect_mutex);
    }

    _trace_leave();
    return;
}

void
tsEpg_Eit_Schedule_CallBack(
    TS_DEMUX        *ptDemux,
    PSI_EIT_INFO    *ptEitInfo)
{
    TS_EPG_DB       *pEpgDb = DOWN_CAST(TS_EPG_DB, ptDemux->pHTsEpg, hEpg);

    _trace_enter("\n");
    
    if( ptDemux->pHTsEpg && pEpgDb && 
        (pEpgDb->bSkip == true || pEpgDb->bSkipEitSchedule == true) ) 
    {
        epg_msg_ex(ENABLE_WARNING_MSG, " ts epg info skip schedule !!");
        _trace_leave();
        return ;
    }
    
    if( ptDemux->pHTsEpg && pEpgDb )
    {
        _mutex_lock(TSD_MSG_TYPE_TRACE_EPG, pEpgDb->eit_collect_mutex);

        do{
            uint32_t        tableIdx = (-1);
            TS_1_SRVC_EIT   *pCur_srvc_eits = 0;
            
            pCur_srvc_eits = _Get_Cur_Srvc_Eits(pEpgDb, pEpgDb->curr_freq, ptEitInfo->service_id);
            if( !pCur_srvc_eits )       break;

            _Update_Eit(pEpgDb, ptEitInfo, pCur_srvc_eits);
            
            tableIdx = ptEitInfo->table_id - SCHEDULE_BASE_TBL_ID;
            if( tableIdx > (EPG_MAX_SCHEDULE_TBL - 1) ||
                (ptEitInfo->section_number>>5) > 7 )
            {
                epg_msg_ex(EPG_MSG_TYPE_ERR, " Out array size !!");
                break;
            }
            
            pCur_srvc_eits->version_num_sch[tableIdx] = (uint8_t)ptEitInfo->version_number;
            pCur_srvc_eits->rx_section_sch[tableIdx][ptEitInfo->section_number>>5] 
                                |= (1UL<<(ptEitInfo->section_number&0x1F));
            
        }while(0);
        
        _mutex_unlock(TSD_MSG_TYPE_TRACE_EPG, pEpgDb->eit_collect_mutex);
    }

    _trace_leave();
    return;
}

void
tsEpg_Tdt_CallBack(
    TS_DEMUX        *ptDemux,
    PSI_MJDBCD_TIME tUtcTime)
{
    TS_EPG_DB       *pEpgDb = DOWN_CAST(TS_EPG_DB, ptDemux->pHTsEpg, hEpg);

    _trace_enter("\n");
    
    if( ptDemux->pHTsEpg && pEpgDb )
    {
        _mutex_lock(TSD_MSG_TYPE_TRACE_EPG, pEpgDb->eit_collect_mutex);
        
        if( psiTime_IsValidUtcTime(tUtcTime) &&
            psiTime_Compare(tUtcTime, pEpgDb->curUtcTime) > 0 )
        {
            pEpgDb->curUtcTime          = tUtcTime;
            pEpgDb->hEpg.curUtcTime     = tUtcTime;

            // to notice AP that TDT is updated
            pEpgDb->hEpg.bRefreshed_TDT = true;
        }

        _mutex_unlock(TSD_MSG_TYPE_TRACE_EPG, pEpgDb->eit_collect_mutex);    
    }

    _trace_leave();
    return;
}

void
tsEpg_Tot_CallBack(
    TS_DEMUX        *ptDemux,
    PSI_TOT_INFO    *ptTotInfo)
{
    TS_EPG_DB       *pEpgDb = DOWN_CAST(TS_EPG_DB, ptDemux->pHTsEpg, hEpg);

    _trace_enter("\n");
    
    if( ptDemux->pHTsEpg && pEpgDb )
    {
        _mutex_lock(TSD_MSG_TYPE_TRACE_EPG, pEpgDb->eit_collect_mutex);

        do{
            PSI_DESCRIPTOR      *ptDescriptor = 0;
            
            if( !psiTime_IsValidUtcTime(ptTotInfo->UTC_time) )
                break;
                
            pEpgDb->curUtcTime = ptTotInfo->UTC_time;
            
            for(ptDescriptor = ptTotInfo->ptFirstDescriptor;
                ptDescriptor;
                ptDescriptor = ptDescriptor->ptNextDescriptor)
            {
                if( LOCAL_TIME_OFFSET_DESCRIPTOR_TAG == ptDescriptor->descriptor_tag )
                {
                    PSI_LOCAL_TIME_OFFSET_DESCRIPTOR    *ptLocalTimeOffsetDescriptor = 0;
                    
                    ptLocalTimeOffsetDescriptor = psiDescriptor_DecodeLocalTimeOffsetDescriptor(ptDescriptor);
            
                    if( ptLocalTimeOffsetDescriptor )
                    {
                        uint32_t    i, j;
            
                        pEpgDb->localTimeOffset.totalLocalTimeOffsetCount = ptLocalTimeOffsetDescriptor->totalLocalTimeOffsetCount;
            
                        for(i = 0; i < ptLocalTimeOffsetDescriptor->totalLocalTimeOffsetCount; i++)
                        {
                            pEpgDb->localTimeOffset.tLocalTimeOffset[i]
                                = ptLocalTimeOffsetDescriptor->tLocalTimeOffset[i];
            
                            for(j = 0; j < 3; j++)
                            {
                                pEpgDb->localTimeOffset.tLocalTimeOffset[i].country_code[j]
                                    = ptLocalTimeOffsetDescriptor->tLocalTimeOffset[i].country_code[j];
                            }
                        }
                        
                        // to notice AP that TDT is updated
                        pEpgDb->hEpg.bRefreshed_TOT = true;
                        break;
                    }
                }
            }     

        }while(0);

        _mutex_unlock(TSD_MSG_TYPE_TRACE_EPG, pEpgDb->eit_collect_mutex);        
    }
    
    _trace_leave();
    return;
}
//=============================================================================
//				  Public Function Definition
//=============================================================================
TSD_ERR
tsEpg_CreateHandle(
    TS_EPG_HANDLE   **pHEpg,
    void            *extraData)
{
    TSD_ERR         result = EPG_ERR_OK;
    TS_EPG_DB       *pEpgDb = 0; 

    _trace_enter("\n");
    
    do{
        if( *pHEpg != 0 )
        {
            epg_msg_ex(EPG_MSG_TYPE_ERR, " error, Exist epg DB handle !!");
            result = EPG_ERR_INVALID_PARAMETER;
            break;
        }

        // ------------------------
        // craete epg database handle
        pEpgDb = tsd_malloc(sizeof(TS_EPG_DB));
        if( !pEpgDb )
        {
            epg_msg_ex(EPG_MSG_TYPE_ERR, " error, allocate fail !!");
            result = EPG_ERR_ALLOCATE_FAIL;
            break;
        }

        memset((void*)pEpgDb, 0x0, sizeof(TS_EPG_DB));
        pEpgDb->epgStatus = EPG_STATUS_IDLE;
        
        //--------------------------------
        // initial variable
        
        // if not error
        _mutex_init(TSD_MSG_TYPE_TRACE_EPG, pEpgDb->eit_collect_mutex);

        (*pHEpg) = &pEpgDb->hEpg;
        
    }while(0);
    
    if( result != EPG_ERR_OK )
    {
        pEpgDb->epgStatus = EPG_STATUS_FAIL;
        epg_msg_ex(EPG_MSG_TYPE_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }
    
    _trace_leave();
    return result;
}

TSD_ERR
tsEpg_DestroyHandle(
    TS_EPG_HANDLE   **pHEpg)
{
    TSD_ERR         result = EPG_ERR_OK;
    TS_EPG_DB       *pEpgDb = DOWN_CAST(TS_EPG_DB, (*pHEpg), hEpg);
    pthread_mutex_t eit_collect_mutex = 0;

    _trace_enter("0x%x\n", pHEpg);
    
    _verify_handle((*pHEpg), result);
    _mutex_lock(TSD_MSG_TYPE_TRACE_EPG, pEpgDb->eit_collect_mutex);

    /**
     * avoid broke destroy process in pre-empty multi-task environment
     * (released_handle be used on other function) 
     */
    _disable_irq(); 
    
    if( pEpgDb )
    {
        uint32_t            i, j;
        TS_1_SRVC_EIT       *pCur_srvc_eits = 0;
        TS_1_SRVC_EIT       *pTmp_srvc_eits = 0;
        MBT_NODE_OPRATOR    node_op = {0};        

        pCur_srvc_eits = pEpgDb->pFirst_srvc_eits;
        for(i = 0; i < pEpgDb->totalService; i++)
        {
            pTmp_srvc_eits = LIST_FIND(TS_1_SRVC_EIT, _Search_Func, 
                                       EPG_FIND_CMP_SEQ_ID, 
                                       pCur_srvc_eits, 0);
            if( pTmp_srvc_eits )
            {
                pCur_srvc_eits = LIST_DEL(TS_1_SRVC_EIT, pTmp_srvc_eits);
                
                // del tree
                for(j = 0; j < EPG_MAX_SCHEDULE_DAY; j++)
                {
                    if( !pTmp_srvc_eits->epg_1_day[j].pEitMbtRoot )
                        continue;
                    
                    node_op.destroy = _Eit_Node_Destroy;
                    mbt_del_tree(EIT_MBT_NODE, mbt_node, &pTmp_srvc_eits->epg_1_day[j].pEitMbtRoot, &node_op);
                }
                
                // del list node
                free(pTmp_srvc_eits); 
            }
        }

        *pHEpg = 0;  // notice AP that handle has be destroyed
        eit_collect_mutex = pEpgDb->eit_collect_mutex;

        // free handle
        free(pEpgDb);
    }
    
    _mutex_unlock(TSD_MSG_TYPE_TRACE_EPG, eit_collect_mutex);
    _mutex_deinit(TSD_MSG_TYPE_TRACE_EPG, eit_collect_mutex);
    _enable_irq(); 
    _trace_leave();
    return result;
}

TSD_ERR
tsEpg_Change_Service(
    TS_EPG_HANDLE      *pHEpg, 
    uint32_t           frequency,
    uint32_t           service_id,
    void               *extraData)
{
    TSD_ERR         result = EPG_ERR_OK;
    TS_EPG_DB       *pEpgDb = DOWN_CAST(TS_EPG_DB, pHEpg, hEpg);

    _trace_enter("0x%x, %d, %d, 0x%x\n", pHEpg, frequency, service_id, extraData);
    
    _verify_handle(pHEpg, result);
    _mutex_lock(TSD_MSG_TYPE_TRACE_EPG, pEpgDb->eit_collect_mutex);

    if( pEpgDb && pEpgDb->epgStatus != EPG_STATUS_FAIL )
    {        
        pEpgDb->bSkip     = false; // receive new service eit data
        pEpgDb->curr_freq = frequency;
    }

    if( result != EPG_ERR_OK )
    {
        pEpgDb->epgStatus = EPG_STATUS_FAIL;
        epg_msg_ex(EPG_MSG_TYPE_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    }  

    _mutex_unlock(TSD_MSG_TYPE_TRACE_EPG, pEpgDb->eit_collect_mutex);
    _trace_leave();
    return result;    
}

TSD_ERR
tsEpg_Get_Service_Schedule(
    TS_EPG_HANDLE      *pHEpg, 
    uint32_t           frequency,
    uint32_t           service_id,
    TS_1_SRVC_EIT      **pOne_srvc_eits,
    void               *extraData)
{
    TSD_ERR         result = EPG_ERR_OK;
    TS_EPG_DB       *pEpgDb = DOWN_CAST(TS_EPG_DB, pHEpg, hEpg);

    _trace_enter("0x%x, %d, %d, 0x%x, 0x%x\n", pHEpg, frequency, service_id, pOne_srvc_eits, extraData);
    
    _verify_handle(pHEpg, result);
    _mutex_lock(TSD_MSG_TYPE_TRACE_EPG, pEpgDb->eit_collect_mutex);
    
    if( pEpgDb && pEpgDb->epgStatus != EPG_STATUS_FAIL )
    {
        epg_cmp_data    cmpData = {0};
        TS_1_SRVC_EIT   *pFind_srvc_eits = 0;
                
        // search epg db by frequency and service_id
        cmpData.frequency  = frequency;
        cmpData.service_id = service_id;
        
        do{
            if( pEpgDb->pFirst_srvc_eits )
                pFind_srvc_eits = LIST_FIND(TS_1_SRVC_EIT,
                                           _Search_Func, 
                                           EPG_FIND_CMP_SERVICE_ID, 
                                           pEpgDb->pCur_srvc_eits, 
                                           &cmpData); 

            if( pOne_srvc_eits )        *pOne_srvc_eits = pFind_srvc_eits;

        }while(0);
    }

    if( result != EPG_ERR_OK )
    {
        pEpgDb->epgStatus = EPG_STATUS_FAIL;
        epg_msg_ex(EPG_MSG_TYPE_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    } 

    _mutex_unlock(TSD_MSG_TYPE_TRACE_EPG, pEpgDb->eit_collect_mutex);
    _trace_leave();
    return result; 
}

TSD_ERR
tsEpg_Control(
    TS_EPG_HANDLE      *pHEpg,
    TS_EPG_CTL_CMD     cmd,
    uint32_t           *value,
    void               *extraData)
{
    TSD_ERR         result = EPG_ERR_OK;
    TS_EPG_DB       *pEpgDb = DOWN_CAST(TS_EPG_DB, pHEpg, hEpg);

    _trace_enter("0x%x, %d, 0x%x, 0x%x\n", pHEpg, cmd, value, extraData);

    _verify_handle(pHEpg, result);
    _mutex_lock(TSD_MSG_TYPE_TRACE_EPG, pEpgDb->eit_collect_mutex);
    
    if( pEpgDb && pEpgDb->epgStatus != EPG_STATUS_FAIL )
    {
        switch( cmd )
        {
            case TS_EPG_CTL_PREFER_EVENT_LANG_B:
                pEpgDb->hEpg.preferEventLangB = *((ISO_639_LANG*)extraData);
                break;
                
            case TS_EPG_CTL_PREFER_EVENT_LANG_T:
                pEpgDb->hEpg.preferEventLangT = *((ISO_639_LANG*)extraData);
                break;
                
            case TS_EPG_CTL_SKIP_EPG:
                pEpgDb->bSkip = (value) ? true : false;
                break;
            
            case TS_EPG_CTL_SKIP_SCHEDULE:
                pEpgDb->bSkipEitSchedule = (value) ? true : false;
                break;
            
            case TS_EPG_CTL_UPDATE_TDT:
                pEpgDb->bUpdate_TDT = true;
                break;
                
            default:
                result = EPG_ERR_NO_IMPLEMENT;
                break;
        }
    }

    if( result != EPG_ERR_OK &&
        result != EPG_ERR_NO_IMPLEMENT )
    {
        pEpgDb->epgStatus = EPG_STATUS_FAIL;
        epg_msg_ex(EPG_MSG_TYPE_ERR, "%s() err 0x%x !", __FUNCTION__, result);
    } 

    _mutex_unlock(TSD_MSG_TYPE_TRACE_EPG, pEpgDb->eit_collect_mutex);
    _trace_leave();
    return result;    
}


// TSD_ERR
// tsEpg_template(
//     TS_EPG_HANDLE      *pHEpg)
// {
//     TSD_ERR         result = EPG_ERR_OK;
//     TS_EPG_DB       *pEpgDb = DOWN_CAST(TS_EPG_DB, pHEpg, hEpg);
//     
//     if( pEpgDb && pEpgDb->epgStatus != EPG_STATUS_FAIL )
//     {
//     }
// 
//     if( result != EPG_ERR_OK )
//     {
//         pEpgDb->epgStatus = EPG_STATUS_FAIL;
//         epg_msg_ex(EPG_MSG_TYPE_ERR, "%s() err 0x%x !", __FUNCTION__, result);
//     }     
//     return result;    
// }