#ifndef __TS_DEMUXER_H_5PUMMT31_RDBV_AJHQ_WA2L_8MMHX99O54KH__
#define __TS_DEMUXER_H_5PUMMT31_RDBV_AJHQ_WA2L_8MMHX99O54KH__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <stdbool.h>
#include "ts_demuxer_err.h"
//=============================================================================
//				  Constant Definition
//=============================================================================
#define TSD_MAX_SRVC_NAME_SIZE          256

/**
 * ts demuxer handle ts "file" status
 **/
typedef enum TSD_CTRL_STATUS_TAG
{
    TSD_CTRL_NORMAL_MODE = 0,    // normal streaming parsing
    TSD_CTRL_WAIT_VIDEO_AUDIO,   // scan parsing 
    TSD_CTRL_WAIT_FIRST_TIMESTAMP,
    TSD_CTRL_WAIT_LAST_TIMESTAMP,
    
}TSD_CTRL_STATUS;

/**
 * ts demuxer control cmd
 **/
typedef enum _TSD_CTRL_CMD_TAG
{
    /* 00 */ TSD_CTRL_UNKNOW     = 0,
    /* 01 */ TSD_CTRL_UPDATE_SRVC_INFO,
    /* 02 */ TSD_CTRL_SORT_SRVC_INFO,
    /* 03 */ TSD_CTRL_IMPORT_INFO,
    /* 04 */ TSD_CTRL_EXPORT_INFO,
    /* 05 */ TSD_CTRL_ENABLE_TSI,
    /* 06 */ TSD_CTRL_DISABLE_TSI,
    /* 07 */ TSD_CTRL_GET_COUNTRY_FREQ_CNT,
    /* 08 */ TSD_CTRL_GET_SIGNAL_STATUS,
    /* 09 */ TSD_CTRL_GET_CURR_UTC_TIME,
    /* 10 */ TSD_CTRL_RESET_ACT_INFO,
    /* 11 */ TSD_CTRL_SET_DEMOD_STATUS, // for the dual H/W demod 
    /* 12 */ TSD_CTRL_SKIP_EPG_PARSING,
    /* 13 */ TSD_CTRL_SET_DEMOD_SUSPEND_MODE,
    /* 14 */ TSD_CTRL_RESET_TS_DB,  // clear ts service/channel database database
    /* 15 */ TSD_CTRL_DEMOD_SUSPEND_FIRE,
    /* 16 */ TSD_CTRL_DEMOD_RESET,
    
}TSD_CTRL_CMD; 

/**
 * demod H/W engine status
 **/
typedef enum TSD_DEMOD_STATUS_TAG
{
    TSD_DEMOD_STATUS_UNKNOW    = 0, 
    TSD_DEMOD_STATUS_IDLE      = 1,
    TSD_DEMOD_STATUS_RUNNING   = 2,

}TSD_DEMOD_STATUS;


/**
 * frequency scan mode
 **/
typedef enum TSD_FREQ_SCAN_MODE_TAG
{
    TSD_FREQ_SCAN_AUTO    = 0, 
    TSD_FREQ_SCAN_MANUAL,

}TSD_FREQ_SCAN_MODE;

/**
 *  country id for frequency definitions
 **/
typedef enum _TSD_COUNTRY_ID_TAG
{
    TSD_COUNTRY_UNKNOW          = 0,
    TSD_COUNTRY_AUSTRALIA,
    TSD_COUNTRY_AUSTRIA,
    TSD_COUNTRY_CHINA,
    TSD_COUNTRY_FRANCE,
    TSD_COUNTRY_GERMANY,
    TSD_COUNTRY_GREECE,
    TSD_COUNTRY_HUNGARY,
    TSD_COUNTRY_ITALY,
    TSD_COUNTRY_NETHERLANDS,
    TSD_COUNTRY_POLAND,
    TSD_COUNTRY_PORTUGAL,
    TSD_COUNTRY_RUSSIAN,
    TSD_COUNTRY_SPAIN,
    TSD_COUNTRY_TAIWAN,
    TSD_COUNTRY_UK,
    TSD_COUNTRY_DENMARK,
    TSD_COUNTRY_SWEDEN,
    TSD_COUNTRY_FINLAND,

    TSD_COUNTRY_CNT,

}TSD_COUNTRY_ID;

/**
 * ts demuxer output sample mode 
 **/
typedef enum _TSD_OUT_MODE_TAG
{
    TSD_OUT_UNKNOW      = 0,
    TSD_OUT_TS_BY_PASS, 
    TSD_OUT_PES_DATA,

}TSD_OUT_MODE;

/**
 * ts demuxer used demod type
 **/
typedef enum _TSD_DEMOD_TYPE_TAG
{
    TSD_DEMOD_UNKNOW    = 0,
    TSD_DEMOD_OMEGA,
    TSD_DEMOD_IT9135,
    TSD_DEMOD_IT9137,
    TSD_DEMOD_IT9137_USB,

}TSD_DEMOD_TYPE;

/**
 * sample type
 **/
typedef enum _TSD_SAMPLE_TYPE_TAG
{
    TSD_SAMPLE_UNKNOW       = 0,
    TSD_SAMPLE_TS           = 0x20000000,     // filtered ts stream by pass
    TSD_SAMPLE_VIDEO        = 0x00000001,     // PES video sample
    TSD_SAMPLE_AUDIO        = 0x00000002,     // PES audio sample
    TSD_SAMPLE_SUBTITLE     = 0x00000004,     // PES subtitle sample
    TSD_SAMPLE_TELETEXT     = 0x00000008,     // PES teletext sample
    TSD_SAMPLE_PES_ALL      = 0x0000000F,
    
}TSD_SAMPLE_TYPE;

/**
 * demod scan status
 **/
typedef enum TSD_SCAN_STATE_TAG
{
    TSD_SCAN_ZERO = 0,
    TSD_SCAN_IN_SCANNING,
    TSD_SCAN_CHANNEL_PARSE,
    
}TSD_SCAN_STATE;

/**
 * channel/service info sort type
 **/
typedef enum _TSD_REPO_INFO_TYPE_TAG
{
    TSD_REPO_UNKNOW_INFO    = 0,
    TSD_REPO_CHANNEL_INFO,
    TSD_REPO_SERVICE_INFO,
    TSD_REPO_EPG_SCHEDULE_INFO,
    TSD_REPO_EPG_PRESENT_INFO,
    TSD_REPO_EPG_FOLLOWING_INFO,    
    TSD_REPO_EPG_EVENT_RATING,
    TSD_REPO_EPG_EVENT_NAME,        // dvb-t text data
    TSD_REPO_EPG_EVENT_START_TIME,  // mjd_bcd time
    TSD_REPO_EPG_EVENT_END_TIME,    // mjd_bcd time
    TSD_REPO_EPG_EVENT_DESCRIPTION, // dvb-t text data
    TSD_REPO_EPG_EVENT_NAME_UTF16,
    TSD_REPO_EPG_EVENT_START_TIME_UTF16,
    TSD_REPO_EPG_EVENT_END_TIME_UTF16,
    TSD_REPO_EPG_EVENT_DESCRIPTION_UTF16,
    
}TSD_REPO_INFO_TYPE;

/**
 * channel/service info sort type
 **/
typedef enum _TSD_INFO_SORT_TYPE_TAG
{
    TSD_INFO_SORT_NONE  = 0,
    TSD_INFO_SORT_FREQ,
    TSD_INFO_SORT_SRVC_NAME,
    
}TSD_INFO_SORT_TYPE;

//=============================================================================
//				  Macro Definition
//=============================================================================
#define DATETIME_TO_UID(pUid32, year, month, day, hour, minute, sec)                              \
    if(pUid32)  (*pUid32) = (((((year-1980)<<9)&0xfe00)|((month<<5)&0x01e0)|(day&0x001f))<<16) |  \
                            (((hour<<11)&0xf800)|((minute<<5)&0x07e0)|(sec&0x001f)) 
                    
#define UID_TO_DATETIME(uid32, pYear, pMonth, pDay, pHour, pMinute, pSec)   \
    do{ if(pSec)    *pSec=((uid32&0xFFFF)&0x001f);                          \
        if(pMinute) *pMinute=((uid32&0xFFFF)&0x07e0)>>5;                    \
        if(pHour)   *pHour=((uid32&0xFFFF)&0xf800)>>11;                     \
        if(pDay)    *pDay=(((uid32&0xFFFF0000)>>16)&0x001f);                \
        if(pMonth)  *pMonth=(((uid32&0xFFFF0000)>>16)&0x01e0)>>5;           \
        if(pYear)   *pYear=(1980+((((uid32&0xFFFF0000)>>16)&0xfe00)>>9));   \
    }while(0)

//=============================================================================
//				  Structure Definition
//=============================================================================
/**
 * the result of frequency scan for AP application.
 **/
typedef struct _TSD_SCAN_RST_INFO_TAG
{
    TSD_SCAN_STATE  scanState;
    bool            bChannelLock;

    uint16_t        srvc_name[TSD_MAX_SRVC_NAME_SIZE];
    uint32_t        srvc_name_length;

    // Ap layer request to interrup frequency scan process, if need
    bool            bStopScanProc;

}TSD_SCAN_RST_INFO;

typedef void (*TSD_FREQ_SCAN_CALLBACK) (TSD_SCAN_RST_INFO *scanInfo, void *extraData);

/**
 * frequency scan parameters for AP application.
 **/
typedef struct _TSD_SCAN_PARAM_TAG
{
    TSD_COUNTRY_ID        countryId;
    uint32_t              bandwidth;
    uint32_t              scanFrequency;
    uint32_t              channelWaitMs;
    TSD_FREQ_SCAN_CALLBACK  pfCallBack;  // for get scan result in AP layer
    
}TSD_SCAN_PARAM;

/**
 * channel info for AP application.
 **/
typedef struct _TSD_CHNL_USER_INFO_TAG
{
    uint32_t        bandwidth;
    uint32_t        frequency; 
    uint32_t        totalSrvcCnt;

}TSD_CHNL_USER_INFO;

/**
 * service info for AP application.
 **/
typedef struct _TSD_SRVC_USER_INFO_TAG
{
    bool        bTV;

    uint8_t     serviceName[TSD_MAX_SRVC_NAME_SIZE];
    uint32_t    nameSize;
    uint16_t    audioCount;
    uint16_t    subtitleCount;
    uint16_t    actAudioIdx;
    uint16_t    actSubtitleIdx;    
    
}TSD_SRVC_USER_INFO;

/**
 * Current UTC time for parental control
 **/
typedef struct _TSD_UTC_TIME_TAG
{
    uint16_t    year;
    uint8_t     month;
    uint8_t     day;

    uint8_t     hour;
    uint8_t     minute;
    uint8_t     second;
    
}TSD_UTC_TIME;

/**
 * EPG of one EIT info for AP application.
 **/
typedef struct _TSD_EPG_USER_INFO_TAG
{
    uint16_t            rating;  // parental rating
    //uint64_t            start_time;
    //uint32_t            duration;
    
    // event description
    uint8_t             *pEventName;
    uint8_t             nameSize;
    uint8_t             *pText;
    uint8_t             textSize;
    
    // extended event data
    uint8_t             *pExtendText;
    uint32_t            extendTextSize;
    
}TSD_EPG_USER_INFO;

/**
 * PES sample info
 **/
typedef struct _TSD_SAMPLE_INFO_TAG
{
    TSD_SAMPLE_TYPE         type;
    
    // if(bShareBuf == true) => no malloc, just return tsi buf pointer 
    // else malloc buf and free with AP layer
    bool                    bShareBuf;  
    uint8_t                 *sampleAddr;
    uint32_t                sampleSize;

    // for usb demod...
    bool                    bUsbDefaultAccess;
    void                    *pfCallback;
    void                    *pCtrlParam;

    // disable mutex, it is dangerous.
    #define TSD_DISABLE_MUTEX       0xbeafdddd
    uint32_t                disableMutex;
    
}TSD_SAMPLE_INFO;

/**
 * ts demuxer export/import config from AP layer
 **/
typedef struct _TSD_INFO_REPO_TAG
{
    void        *privInfo;
    bool        bSkipImport;
    bool        bSkipExport;

    uint32_t    totalSrvc;     // when import, must be set by AP layer
    uint32_t    totalChnls;    // when import, must be set by AP layer
    
    TSD_REPO_INFO_TYPE  repoType;

    // if error => return negative number  
    int32_t     (*tsd_repo_open)(struct _TSD_INFO_REPO_TAG *pTsdInfoRepo, void *exraData);
    int32_t     (*tsd_repo_close)(struct _TSD_INFO_REPO_TAG *pTsdInfoRepo, void *exraData);

    // return real byte size
    uint32_t    (*tsd_repo_import)(struct _TSD_INFO_REPO_TAG *pTsdInfoRepo, uint8_t *buf, uint32_t byteSize);
    uint32_t    (*tsd_repo_export)(struct _TSD_INFO_REPO_TAG *pTsdInfoRepo, uint8_t *buf, uint32_t byteSize);

}TSD_INFO_REPO;

/**
 * ts demuxer DVB text conversion parameters
 **/
typedef struct _TSD_TXT_CONV_TAG
{
    // input
    uint8_t     *dvbTxt;
    uint32_t    dvbTxtLength;
    
    // output
    uint16_t    *utf16Txt;
    uint32_t    utf16TxtLength;
    
}TSD_TXT_CONV; 

/**
 * ts receiver info (onBoard or usb)
 **/
typedef struct _TSD_TS_RX_INFO_TAG
{
#define TSD_TSRX_TYPE_TSI       0x11
#define TSD_TSRX_TYPE_USB       0x22
#define TSD_TSRX_TYPE_CUSTMOR   0x33

    uint32_t        tsrx_type;
    
    union{
        // tsi
        struct {
            uint32_t    tsi_index;
            void        *data;
        }tsi_info;

        // usb demod
        struct {
            uint32_t    demod_index;
            void        *pDevCtrlInfo;
            bool        bWithoutCopy;  // no copy
        }usb_demod_info;        
    }privData;
    
    uint32_t (*ts_rx_init)(struct _TSD_TS_RX_INFO_TAG *pRxInfo, void *extraData);
    uint32_t (*ts_rx_turn_on)(struct _TSD_TS_RX_INFO_TAG *pRxInfo, void *extraData);
    uint32_t (*ts_rx_turn_off)(struct _TSD_TS_RX_INFO_TAG *pRxInfo, void *extraData);
    uint32_t (*ts_rx_get_data)(struct _TSD_TS_RX_INFO_TAG *pRxInfo, uint8_t **sampleAddr, uint32_t *sampleLength, void *extraData);
    uint32_t (*ts_rx_deinit)(struct _TSD_TS_RX_INFO_TAG *pRxInfo, void *extraData);
    
}TSD_TS_RX_INFO;

typedef struct _TSD_BUF_INFO_TAG
{
    uint32_t    pid;
    uint8_t     *pBufAddr;
    //uint32_t    bufLength;
    uint32_t    pitch;
    uint32_t    heigth;

}TSD_BUF_INFO;

//#define TSD_MAX_ES_CNT_PRE_PES      1 

/**
 * ts demuxer pre-setting info for create handle
 **/
typedef struct _TSD_PRE_SET_INFO_TAG
{
    TSD_OUT_MODE    tsdOutMode;
    
    TSD_DEMOD_TYPE  tsdDemodType; // demode type, ex. omega
    uint32_t        demod_id;     // demod index 0 or 1
    
    uint32_t        tsi_id;       // tsi index 0 or 1

    TSD_TS_RX_INFO  tsRecevier;

    // pes output buffer info
    bool            bOnPesOutput;
    TSD_BUF_INFO    pesOutBuf_a; // pes audio packet    
    TSD_BUF_INFO    pesOutBuf_v; // pes video packet
    TSD_BUF_INFO    pesOutBuf_s; // pes subtitle packet
    TSD_BUF_INFO    pesOutBuf_t; // pes teletext packet

}TSD_PRE_SET_INFO;


#define SERVICE_MAX_AUDIO_COUNT             8
#define SERVICE_MAX_SUBTITLE_COUNT          8

typedef struct _TSD_SERVICE_PID_INFO_TAG
{
    uint32_t            videoPID;
    uint32_t            videoType;
    uint32_t            audioPID[SERVICE_MAX_AUDIO_COUNT];
    uint32_t            audioType[SERVICE_MAX_AUDIO_COUNT];
    uint32_t            subtitlePID[SERVICE_MAX_SUBTITLE_COUNT];
} TSD_SERVICE_PID_INFO;

/**
 * ts demuxer handle
 **/
typedef struct _TSD_HANDLE_TAG
{
    TSD_SCAN_PARAM      tsdScanParam;
    
    uint32_t            actChnlIdx;
    uint32_t            actSrvcIdx;
    uint32_t            actBandwidth;
    uint32_t            actFreq;

    uint32_t            act_demod_id;
    uint32_t            act_tsi_id;
    
    uint32_t            totalSrvc;
    uint32_t            totalChnl;
    
    TSD_INFO_SORT_TYPE  sortType;

    // for scan progress bar
    int32_t             scan_total_times;
    int32_t             scan_cnt;
    
}TSD_HANDLE;

//=============================================================================
//				  Global Data Definition
//=============================================================================


//=============================================================================
//				  Private Function Definition
//=============================================================================


//=============================================================================
//				  Public Function Definition
//=============================================================================
TSD_ERR
tsd_CreateHandle(
    TSD_HANDLE          **pHTsd,
    TSD_PRE_SET_INFO    *pSetInfo,
    void                *extraData);


TSD_ERR
tsd_DestroyHandle(
    TSD_HANDLE  **pHTsd);


TSD_ERR
tsd_Scan_Channel(
    TSD_HANDLE          *pHTsd,
    TSD_FREQ_SCAN_MODE  scanMode,
    TSD_SCAN_PARAM      *scanParm,
    void                *extraData);


TSD_ERR
tsd_Get_Sample(
    TSD_HANDLE       *pHTsd,
    TSD_SAMPLE_TYPE  requestType,    
    TSD_SAMPLE_INFO  *pSampleInfo,
    void             *extraData);


TSD_ERR
tsd_Merge_Service_Info(
    TSD_HANDLE          *pHTsd_master,
    TSD_HANDLE          *pHTsd_slave,
    void                *extraData);
    

TSD_ERR
tsd_Get_ChannelInfo(
    TSD_HANDLE          *pHTsd,
    uint32_t            index,
    TSD_CHNL_USER_INFO  **pChnlUserInfo,
    void                *extraData);
    

TSD_ERR
tsd_Get_ServiceInfo(
    TSD_HANDLE          *pHTsd,
    uint32_t            index,
    TSD_SRVC_USER_INFO  **pSrvcUserInfo,
    void                *extraData);    


TSD_ERR
tsd_Get_CountryName(
    TSD_HANDLE          *pHTsd,
    TSD_COUNTRY_ID      countryId,
    char                **name,
    void                *extraData);


TSD_ERR
tsd_Get_Service_Schedule(
    TSD_HANDLE          *pHTsd,
    uint32_t            serviceIdx,
    TSD_INFO_REPO       *pTsdInfoRepo,
    void                *extraData);


TSD_ERR
tsd_Get_Service_PF(
    TSD_HANDLE          *pHTsd,
    uint32_t            serviceIdx,
    TSD_INFO_REPO       *pTsdInfoRepo,
    void                *extraData);

TSD_ERR
tsd_Get_PidInfo(
    TSD_HANDLE              *pHTsd,
    uint32_t                srvcIdx,
    TSD_SERVICE_PID_INFO    *pServicePidInfo);

TSD_ERR
tsd_Change_Service(
    TSD_HANDLE      *pHTsd,
    uint32_t        index,
    void            *extraData);


TSD_ERR
tsd_Conv_Text(
    TSD_HANDLE          *pHTsd,
    TSD_TXT_CONV        *txtConv,
    void                *extraData);


TSD_ERR
tsd_Change_Subtitle(
    TSD_HANDLE      *pHTsd,
    uint32_t        index,
    void            *extraData);
    

TSD_ERR
tsd_Change_SoundTrack(
    TSD_HANDLE      *pHTsd,
    uint32_t        index,
    void            *extraData);

    
TSD_ERR
tsd_Control(
    TSD_HANDLE      *pHTsd,
    TSD_CTRL_CMD    cmd,
    uint32_t        *value,
    void            *extraData);

    
#ifdef __cplusplus
}
#endif

#endif
