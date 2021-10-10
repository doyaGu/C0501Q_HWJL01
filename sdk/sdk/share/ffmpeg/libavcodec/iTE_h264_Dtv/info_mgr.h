/*
 * Copyright (c) 2007 ITE Technology Corp. All Rights Reserved.
 */
/** @file info_mgr.h
 * Used to provide data structure for table manager
 * @author Vincent Lee
 * @version 0.1
 */

#ifndef INFO_MGR_H
#define INFO_MGR_H

#include "core_config.h"
#include "ite/mmp_types.h"
#include "psi_time.h"
//#include "psi_descriptor_0x58.h"
//#include "pal\keypad.h"

#ifdef __cplusplus
extern "C" {
#endif

//=============================================================================
//                              Constant Definition
//=============================================================================

#define NOR_CONFIG_HEADER_INFO_SIZE         (12)
#define NOR_CUSTOM_HEADER_INFO_SIZE         (12)
#define NOR_SERVICE_HEADER_INFO             (12)

#ifdef ENABLE_SERVICE_CONFIG_COMPRESS
    #define NOR_SERVICE_SIZE                (128*1024)
#else
    #define NOR_SERVICE_SIZE                (256*1024)
#endif

// max TV/Radio services to be installed
#define INFO_MGR_MAX_TV_SERVICE             (1000)
#define INFO_MGR_MAX_RD_SERVICE             (1000)

#define MAX_AUDIO_PID_NUMBER                (16)
#define MAX_SERVICE_NAME_NUMBER             (32)
#define MAX_TELETEXT_PID_NUMBER             (1)
#define MAX_TELETEXT_SUBTITLE_PID_NUMBER    (8)
#ifdef ENABLE_DSM_CC
#define MAX_DSM_CC_PID_NUMBER               (4)
#define MAX_SUBTITLE_PID_NUMBER             (8)
#else
#define MAX_SUBTITLE_PID_NUMBER             (MAX_AUDIO_PID_NUMBER)
#endif

#define INVALID_CHANNEL_INDEX               (0xFFFFFFFF)
#define INVALID_SERVICE_INDEX               (0xFFFF)
#define INVALID_VERSION_NUMBER              (0xFFFFFFFF)
#define INVALID_TELETEXT_INDEX              (0xFF)
#define INVALID_SUBTITLE_INDEX              (0xFF)
#define INVALID_SERVICE_NUMBER              (0xFFFF)
#define INVALID_ONID_NUMBER                 (0xFFFFFFFF)

#define NIT_TBL_INFO_VERSION_MASK           (0x001F)
#define NIT_TBL_INFO_HIT_BIT_MASK           (0x8000)
#define INVALID_NIT_NETWORK_ENTRY_ID        (0xFFFFFFFF)

#define NIT_FREQ_NETWORK_ENTRY_MASK         (0xF0000000)
#define NIT_FREQ_NETWORK_ENTRY_OFFSET       (28)
#define NIT_TRANSPORT_STREAM_IDX_MASK       (0x0F000000)
#define NIT_TRANSPORT_STREAM_IDX_OFFSET     (24)
#define NIT_FREQ_FREQUENCY_MASK             (0x00FFFFFF)

#define NIT_TRANSPORT_STREAM_ID_MASK        (0x0000FFFF)
#define NIT_TRANSPORT_STREAM_ID_OFFSET      (0)
#define NIT_BANDWIDTH_MASK                  (0xFFFF0000)
#define NIT_BANDWIDTH_OFFSET                (16)

#define SUBTITLE_PAGE_ID_MASK               (0xFFFF0000)
#define SUBTITLE_PID_MASK                   (0x0000FFFF)

#define INFO_ATTRIBUTE_HIDE                 (1 << 0)
#define INFO_ATTRIBUTE_DELETE               (1 << 1)
#define INFO_ATTRIBUTE_AVC                  (1 << 2)
#define INFO_ATTRIBUTE_HD                   (1 << 3)
#define INFO_ATTRIBUTE_CA_DESCR             (1 << 4)
#define INFO_ATTRIBUTE_FREE_CA_MODE         (1 << 5)
#define INFO_ATTRIBUTE_CA_IDENTIFIER_DESCR  (1 << 6)
#define INFO_ATTRIBUTE_ABSENT_AV            (1 << 7)
#define INFO_ATTRIBUTE_FAVOR                (1 << 8)
#define INFO_ATTRIBUTE_LOCK                 (1 << 9)
#define INFO_ATTRIBUTE_NOT_RUNNING          (1 << 10)
#define INFO_ATTRIBUTE_FORBID               (1 << 11)
#define INFO_ATTRIBUTE_INVISIBLE            (1 << 12)

// max INFO_CHANNEL saved in INFO_CONFIG
#define INFO_MGR_MAX_CHANNEL                (64)

#if defined SCHEDULE_EVENT_TABLE_NUMBER
#define INFO_MGR_MAX_SCHEDULE_TBL           (SCHEDULE_EVENT_TABLE_NUMBER)
#else
#define INFO_MGR_MAX_SCHEDULE_TBL           (1)
#endif

#define INFO_MGR_SCHEDULE_BASE_TBL_ID       (0x50)

#define INFO_PMT_PID_TBL_PID_MASK           (0x3FFF)

// same as DEMOD_TOTAL_FILTER_COUNT defined in demod_control.h
#define INFO_HW_PID_FILTER_COUNT            (32)

#define INFO_MAX_TRANSPORT_STREAM_ID_ENTRY  (4)
#define INFO_MAX_NIT_NETWORK_ENTRY          (8)
#define INFO_MAX_FREQUENCY_ENTRY            (32)

/** LOAD FILE TO CASTOR ERROR */
#define MMP_RESULT_LOAD_FILE_ERROR_FILE_SIZE    (5)
#define MMP_RESULT_LOAD_FILE_ERROR_CHECK_SUM    (6)
#define MMP_RESULT_LOAD_FILE_ERROR_VERSION      (7)

// Play back file bookmark related definition
#define INFO_VIDEO_FILE_MAXIMUM_PATH_SIZE       (512)
#define INFO_VIDEO_MAXIMUM_ENTRY_COUNT          (3)

// Record maximum buffer
#define INFO_RECORD_BUFFER_COUNT                (8)
#define INFO_RECORD_BUFFER_SIZE                 (128 * 1024)

#define INFO_CHEAT_CODE_KEY_NUM                 (8)

// stream type value
// H.222 table 2-29 - stream type assignments
//#define MPEG1_VIDEO_STREAM_TYPE                 (0x1)
//#define MPEG2_VIDEO_STREAM_TYPE                 (0x2)
//#define MPEG1_AUDIO_STREAM_TYPE                 (0x3)
//#define MPEG2_AUDIO_STREAM_TYPE                 (0x4)
//#define MPEG2_PRIVATE_DATA_STREAM_TYPE          (0x6)
//#define AVC_VIDEO_STREAM_TYPE                   (0x1B)

// descriptor tag value
// H.222 table 2-39 - program and program element descriptors
#define REGISTRATION_DESCRIPTOR                 (0x5)
#define CA_DESCRIPTOR                           (0x9)
#define ISO_639_LANGUAGE_DESCRIPTOR             (0xA)

// ETSI EN 300 468 table 12: possible locations of descriptors
#define SERVICE_LIST_DESCRIPTOR                 (0x41)
#define VBI_DATA_DESCRIPTOR                     (0x45)
#define VBI_TELETEXT_DESCRIPTOR                 (0x46)
#define SERVICE_DESCRIPTOR                      (0x48)
#define LINKAGE_DESCRIPTOR                      (0x4A)
#define SHORT_EVENT_DESCRIPTOR                  (0x4D)
#define EXTENDED_EVENT_DESCRIPTOR               (0x4E)
#define COMPONENT_DESCRIPTOR                    (0x50)
#define CA_IDENTIFIER_DESCRIPTOR                (0x53)
#define CONTENT_DESCRIPTOR                      (0x54)
#define PARENTAL_RATING_DESCRIPTOR              (0x55)
#define TELETEXT_DESCRIPTOR                     (0x56)
#define SUBTITLING_DESCRIPTOR                   (0x59)
#define TERRESTRIAL_DELIVERY_SYSTEM_DESCRIPTOR  (0x5A)
#define FREQUENCY_LIST_DESCRIPTOR               (0x62)
#define DATA_BROADCAST_ID_DESCRIPTOR            (0x66)
#define AC3_DESCRIPTOR                          (0x6A)
#define CELL_FREQUENCY_LINK_DESCRIPTOR          (0x6D)
#define ENHANCED_AC3_DESCRIPTOR                 (0x7A)

// Digital Terrestrial Television
// Requirements for Interoperability Issue 5.0
#define LOGICAL_CHANNEL_DESCRIPTOR              (0x83)
#define LOGICAL_CHANNEL_DESCRIPTOR_V2           (0x87)
#define PRIVATE_DATA_SPECIFIER_DESCRIPTER       (0x5F)

// Terrestrial delivery system descriptor bandwidth related setting.
#define TERRESTRIAL_BANDWIDTH_MASK              (0xE0)
#define TERRESTRIAL_BANDWIDTH_OFFSET            (5)

// stream content and component type value
// ETSI EN 300 468 table 26: stream content and component type
#define AC3_STREAM_CONTENT                      (0x04)
#define AC3_FORMAT_IDENTIFIER                   (0x41432D33)

// OTA descriptor tag
#define DVB_SSU_LINKAGE_TYPE                    (0x09)

// OTA broadcast_id
#define DVB_SSU_BROADCAST_ID                    (0x0A)
#define DSM_CC_STREAM_TYPE                      (0x0B)

//Service type coding - Service descriptor
//ETSI EN300 468 V.1.7.1 p63
#define DIGITAL_TV_SERVICE                      (0x01)
#define MPEG2_HD_TV_SERVICE                     (0x11)
#define ADVANCED_CODEC_SD_TV_SERVICE            (0x16)
#define ADVANCED_CODEC_HD_TV_SERVICE            (0x19)

#define DIGITAL_RADIO_SERVICE                   (0x02)
#define ADVANCED_CODEC_RADIO_SERVICE            (0x0A)

// VBI data service id - VBI data descriptor
// ETSI EN300 468 V.1.7.1 p73
#define EBU_TELETEXT                            (0x01)
#define INVERTED_TELETEXT                       (0x02)
#define VPS                                     (0x04)
#define WSS                                     (0x05)
#define CLOSED_CAPTIONING                       (0x06)
#define MONOCHROME_4_2_2_SAMPLES                (0x07)

#define INFO_MGR_MAX_FACI_LO_CAP                (70)

typedef enum INFO_TV_OUT_MODE_TAG
{
    INFO_TV_OUT_PAL = 0,
    INFO_TV_OUT_NTSC,
    INFO_TV_OUT_LCD
} INFO_TV_OUT_MODE;

typedef enum INFO_EIT_TYPE_TAG
{
    INFO_PRESENT_FOLLOW = 0,
    INFO_SCHEDULE
} INFO_EIT_TYPE;

typedef enum INFO_SERVICE_NODE_TYPE_TAG
{
    INFO_NO_LCN_NODE,
    INFO_LCN_NODE
} INFO_SERVICE_NODE_TYPE;

typedef enum INFO_LANG_ENTRY_TYPE_TAG
{
    INFO_AUDIO_ENTRY = 0,
    INFO_SUBTITLE_ENTRY,
    INFO_TELETEXT_ENTRY
} INFO_LANG_ENTRY_TYPE;

typedef enum INFO_SERVICE_TYPE_TAG
{
    TV_SERVICE = 0,
    RADIO_SERVICE,
    UNKNOWN_SERVICE // such as MHEG5, MHP...
} INFO_SERVICE_TYPE;

typedef enum INFO_QUERY_TYPE_TAG
{
    ALL_LIST = 0,
    NO_DELETE,
    DELETE_ONLY,
    NO_HIDE,
    HIDE_ONLY,
    NO_HIDE_NO_DELETE,
    INVISIBLE_ONLY,
    CA_ONLY,
    NO_CA_NO_DELETE,
    FAVOR_ONLY,
    FREQUENCY_ONLY,
    MFN_RELATED_SERVICES
} INFO_QUERY_TYPE;

typedef enum INFO_AUDIO_CODEC_TYPE_TAG
{
    AUDIO_MP3_CODEC = 0,
    AUDIO_AC3_CODEC,
    AUDIO_AAC_CODEC
} INFO_AUDIO_CODEC_TYPE;

typedef enum INFO_SUBTITLING_TYPE_TAG
{
    NORMAL_SUBTITLE = 0,
    HARD_OF_HEARING_SUBTITLE
} INFO_SUBTITLING_TYPE;

// INFO_CHANNEL: pPmtPidTbl
// MSB ---------------- LSB 16 bits
//     XX OOOOOOOOOOOOO
//     ~~ ~~~~~~~~~~~~~
//   TYPE       PMT_PID
typedef enum INFO_PMT_PROGRAM_TYPE_TAG
{
    PMT_PROGRAM_TV      = 0x0,    // 0000 0000 0000 0000
    PMT_PROGRAM_RADIO   = 0x4000, // 0100 0000 0000 0000
    PMT_PROGRAM_OTHER   = 0x8000, // 1000 0000 0000 0000
    PMT_PROGRAM_MASK    = 0xC000  // 1100 0000 0000 0000
} INFO_PMT_PROGRAM_TYPE;

// same mapping of demod pid filter mapping enumerator.
typedef enum INFO_PID_FILTER_INDEX_TAG
{
#ifdef ENABLE_DSM_CC
    INFO_PID_FILTER_DSM_CC      = (INFO_HW_PID_FILTER_COUNT - 5),
#endif
    INFO_PID_FILTER_TELETEXT    = (INFO_HW_PID_FILTER_COUNT - 4),
    INFO_PID_FILTER_SUBTITLE    = (INFO_HW_PID_FILTER_COUNT - 3),
    INFO_PID_FILTER_AUDIO       = (INFO_HW_PID_FILTER_COUNT - 2),
    INFO_PID_FILTER_VIDEO       = (INFO_HW_PID_FILTER_COUNT - 1)
} INFO_PID_FILTER_INDEX;

typedef enum INFO_VIDEO_RATIO_TAG
{
#ifdef ENABLE_AFD
    INFO_VIDEO_DEFAULT = 0,
    INFO_VIDEO_16_9,
    INFO_VIDEO_14_9,
    INFO_VIDEO_PS,
    INFO_VIDEO_PB,
    INFO_VIDEO_FULLSCREEN,
    INFO_VIDEO_ZOOMIN,
#else
    INFO_VIDEO_DEFAULT = 0,
    INFO_VIDEO_FULLSCREEN,
    INFO_VIDEO_16_9,
    INFO_VIDEO_4_3,
    INFO_VIDEO_LB,
    INFO_VIDEO_PS,
    INFO_VIDEO_ZOOMIN,
#endif
} INFO_VIDEO_RATIO;

typedef enum INFO_ZOOMIN_TIMES_TAG
{
    INFO_ZOOMIN_1X  = 1,
    INFO_ZOOMIN_2X  = 2,
    INFO_ZOOMIN_4X  = 4,
    INFO_ZOOMIN_6X  = 6,
    INFO_ZOOMIN_8X  = 8,
    INFO_ZOOMIN_10X = 10,
    INFO_ZOOMIN_12X = 12,
    INFO_ZOOMIN_14X = 14,
    INFO_ZOOMIN_16X = 16
} INFO_ZOOMIN_TIMES;

typedef enum INFO_ZOOMIN_ACT_TAG
{
    INFO_ZOOMIN_ACT_NONE = 0,
    INFO_ZOOMIN_ACT_UP,
    INFO_ZOOMIN_ACT_DOWN,
    INFO_ZOOMIN_ACT_LEFT,
    INFO_ZOOMIN_ACT_RIGHT
} INFO_ZOOMIN_ACT;

typedef enum INFO_VIDEO_OUT_MODE_TAG
{
    INFO_VIDEO_OUT_MODE_LCD = 0,
    INFO_VIDEO_OUT_MODE_NTSC,
    INFO_VIDEO_OUT_MODE_PAL,
    INFO_VIDEO_OUT_MODE_NTSC_SUSPEND,
    INFO_VIDEO_OUT_MODE_PAL_SUSPEND
} INFO_VIDEO_OUT_MODE;

typedef enum INFO_VIDEO_REGION_RATIO_TAG
{
    INFO_VIDEO_REGION_RATIO_16_9 = 0,
    INFO_VIDEO_REGION_RATIO_4_3,
    INFO_VIDEO_REGION_RATIO_24_1, // 2.4:1
    INFO_VIDEO_REGION_RATIO_DEFAULT //for initialize only.
} INFO_VIDEO_REGION_RATIO;

typedef enum INFO_BAND_FREQUENCY_TYPE_TAG
{
    INFO_FREQ_6M_VHF = 0,
    INFO_FREQ_6M_UHF,
    INFO_FREQ_7M_VHF,
    INFO_FREQ_7M_UHF,
    INFO_FREQ_8M_VHF,
    INFO_FREQ_8M_UHF,
    INFO_FREQ_7M_VHF_COMPLETE,
    INFO_FREQ_7M_VHF_ITALY
} INFO_BAND_FREQUENCY_TYPE;

typedef enum INFO_MFN_MODE_TAG
{
    INFO_MFN_NONE = 0,
    INFO_MFN_SMART_MODE,
    INFO_MFN_EXHAUSTED_MODE,
    INFO_MFN_COMBINATION_MODE
} INFO_MFN_MODE;

typedef enum INFO_MFN_STATE_TAG
{
    INFO_MFN_SMART_HOP_STATE = 0,
    INFO_MFN_EXHAUSTED_HOP_STATE
} INFO_MFN_STATE;

typedef enum INFO_SERVICE_COUNT_BASE_TAG
{
    INFO_SERVICE_COUNT_BY_DEFAULT = 0,
    INFO_SERVICE_COUNT_BY_SPEC
} INFO_SERVICE_COUNT_BASE;

typedef enum INFO_DATA_TYPE_TAG
{
    INFO_DATA_AUTO_SCAN_OPERATE = 0,
    INFO_DATA_AUTO_SCAN_STANDBY,
    INFO_DATA_LASTSCANTIME_STANDBY,
    INFO_DATA_SPLASH,
    //INFO_DATA_BACKGROUND,
    INFO_DATA_WEAK_SIGNAL_THRESHOLD,
    INFO_DATA_VIDEO_WIDTH,
    INFO_DATA_VIDEO_HEIGHT,
    INFO_DATA_VIDEO_ASPECT_RATIO,
    //INFO_DATA_VIDEO_CURRENT_DISPLAY_BUF,
    INFO_DATA_VIDEO_DECODE_AVAILABLE,
    INFO_DATA_AUDIO_DECODE_AVAILABLE,
    INFO_DATA_SERVICE_COUNT_BASE,
    INFO_DATA_TV_ACTIVE_SERVICE_INDEX,
    INFO_DATA_RADIO_ACTIVE_SERVICE_INDEX,
    INFO_DATA_VOLUME_MAP_TABLE,
    INFO_DATA_PREFER_EVENT_LANG_B,
    INFO_DATA_PREFER_EVENT_LANG_T,
    INFO_DATA_PREFER_AUDIO_LANG_B,
    INFO_DATA_PREFER_AUDIO_LANG_T,
    INFO_DATA_PREFER_SUBTITLE_LANG_B,
    INFO_DATA_PREFER_SUBTITLE_LANG_T,
    INFO_DATA_SCHEDULE,
    INFO_DATA_SCHEDULE_STATUS,
    INFO_DATA_TV_OUT_MODE,
    INFO_DATA_PASSWORD,
    INFO_DATA_UNIVERSAL_PASSWORD,
    INFO_DATA_MENU_LOCK,
    INFO_DATA_CHANNEL_LOCK,
    INFO_DATA_MATURITY_LOCK,
    INFO_DATA_COUNTRY_CODE,
    INFO_DATA_COUNTRY_NUMBER,
    INFO_DATA_OSD_COUNTRY_CODE,
    INFO_DATA_OSD_COUNTRY_NUMBER,
    INFO_DATA_SERVICE_IDENTIFY_BASIS,
    INFO_DATA_SERVICE_BLOCK_BASIS,
    INFO_DATA_SERVICE_BAN_BASIS,
    INFO_DATA_SUMMER_TIME,
    INFO_DATA_SUMMER_TIME_STATUS,
    INFO_DATA_CHEAT_CODE,
    INFO_DATA_CHEAT_MODE,
    INFO_DATA_LANGUAGE_INDEX,
    INFO_DATA_WND_SUSPENDED,
    INFO_DATA_DISPLAY_RATIO,
    INFO_DATA_SCART_OUT,
    INFO_DATA_ANT_POWER,
    INFO_DATA_STOP_EIT_UPDATE,
    INFO_DATA_FROCE_RESET,
    INFO_DATA_LAST_WINDOW,
    INFO_DATA_NEW_SERVICE,
    INFO_DATA_COLOR_CTRL_HUE,
    INFO_DATA_COLOR_CTRL_CONTRAST,
    INFO_DATA_COLOR_CTRL_SATURATION,
    INFO_DATA_COLOR_CTRL_BRIGHTNESS,
    INFO_DATA_IS_HIERARCHICAL_TX,
    INFO_DATA_AC3_MODE,
    INFO_DATA_FACI_LO_CAP,
    INFO_DATA_TSI,
    INFO_DATA_WAKEUP_INDEX,
    INFO_DATA_MUSIC_REPEAT_MODE,
    INFO_DATA_MOVIE_REPEAT_MODE,
    INFO_DATA_PVR_REPEAT_MODE
} INFO_DATA_TYPE;

typedef enum INFO_SERVICE_UPDATE_EVENT_TYPE_TAG
{
    INFO_SELECTED_SERVICE_PID_CHANGE = 0,
    INFO_SERVICE_COUNT_CHANGE,
    INFO_SELECTED_SERVICE_REMOVED
}  INFO_SERVICE_UPDATE_EVENT_TYPE;

//typedef enum INFO_RECORD_FORMAT_TAG
//{
//    INFO_RECORD_PS = 0,
//    INFO_RECORD_TS
//} INFO_RECORD_FORMAT;

typedef enum INFO_CHAR_CODE_TAG
{
    INFO_ISO_IEC_6937                       = 0,
    INFO_ISO_IEC_8859_1                     = 1,
    INFO_ISO_IEC_8859_2                     = 2,
    INFO_ISO_IEC_8859_3                     = 3,
    INFO_ISO_IEC_8859_4                     = 4,
    INFO_ISO_IEC_8859_5                     = 5,
    INFO_ISO_IEC_8859_6                     = 6,
    INFO_ISO_IEC_8859_7                     = 7,
    INFO_ISO_IEC_8859_8                     = 8,
    INFO_ISO_IEC_8859_9                     = 9,
    INFO_ISO_IEC_8859_10                    = 10,
    INFO_ISO_IEC_8859_11                    = 11,
    INFO_ISO_IEC_8859_13                    = 13,
    INFO_ISO_IEC_8859_14                    = 14,
    INFO_ISO_IEC_8859_15                    = 15,
    INFO_ISO_IEC_10646_1                    = 16,
    INFO_KSC5601_1987                       = 17,
    INFO_GB_2312_1980                       = 18,
    INFO_BIG5_SUBSET_OF_ISO_IEC_10646_1     = 19,
    INFO_UTF_8_ENCODING_OF_ISO_IEC_10646_1  = 20
} INFO_CHAR_CODE;

//typedef enum INFO_RECORD_CONTEXT_ID
//{
//    INFO_RECORD_DEMOD0,
//    INFO_RECORD_DEMOD1,
//    INFO_RECORD_TOTAL_DEMOD_COUNT,
//    INFO_RECORD_TIMESHIFT
//} INFO_RECORD_CONTEXT_ID;

// modify for new scan
typedef enum INFO_SDT_STATE_TAG
{
    SCAN_WAIT_SDT_STATE = 0,
    RECEIEVE_SDT_STATE,
    NO_SDT_STATE
} INFO_SDT_STATE;

typedef enum INFO_SERVICE_IDENTIFY_TAG
{
    INFO_SERVICE_IDENTIFY_ONID_SID = 0,
    INFO_SERVICE_IDENTIFY_ONID_SID_TSID
} INFO_SERVICE_IDENTIFY;

typedef enum INFO_SCHEDULE_MODE_TAG
{
    INFO_ONCE_MODE = 0,
    INFO_DAILY_MODE,
    INFO_WEEKLY_MODE
} INFO_SCHEDULE_MODE;

typedef enum INFO_SCHEDULE_FUNCTION_TAG
{
    INFO_SCHEDULE_RECORD = 0,
    INFO_SCHEDULE_WAKEUP
} INFO_SCHEDULE_FUNCTION;

typedef enum INFO_DVBT_SPEC_ID_TAG
{
    INFO_DVB_SPEC_ID_DEFAULT = 0,
    INFO_DVB_SPEC_ID_DBOOK,
    INFO_DVB_SPEC_ID_NORDIG,
    INFO_DVB_SPEC_ID_EBOOK,
    INFO_DVB_SPEC_ID_DGTVI
} INFO_DVBT_SPEC_ID;

typedef enum INFO_AUTO_SCAN_REPETITION_TAG
{
    INFO_AUTO_SCAN_REPETITION_DAILY = 0,
    INFO_AUTO_SCAN_REPETITION_WEEKLY,
} INFO_AUTO_SCAN_REPETITION;

typedef enum INFO_OTA_STATE_TAG
{
    INFO_OTA_STOP = 0,
    INFO_OTA_START_SEARCH,
    INFO_OTA_SERVICE_FOUND,   // receive NIT with OTA information.
    INFO_OTA_INVALID_CHANNEL, // receive NIT without OTA information.
    INFO_OTA_PID_FOUND,
    INFO_OTA_DSI_FOUND,
    INFO_OTA_DII_FOUND,
    INFO_OTA_FIRMWARE_FOUND,
    INFO_OTA_NO_UPGRADE,
    INFO_OTA_CANCEL,
} INFO_OTA_STATE;

typedef enum INFO_SCART_OUT_TAG
{
    INFO_SCART_RGB_OUT,
    INFO_SCART_CVBS_OUT
} INFO_SCART_OUT;

typedef enum INFO_AP_MODE_TAG
{
    INFO_FILE_MODE = 0,
    INFO_TS_FILE_MODE,
    INFO_FREE_TO_AIR_MODE,
} INFO_AP_MODE;

typedef enum INFO_AUDIO_TYPE_TAG
{
    INFO_AUDIO_TYPE_UNDEFINED = 0,
    INFO_AUDIO_TYPE_CLEAN_EFFECTS,
    INFO_AUDIO_TYPE_HEARING_IMPAIRED,
    INFO_AUDIO_TYPE_VISUAL_IMPAIRED
} INFO_AUDIO_TYPE;

typedef enum INFO_REPEAT_MODE_TAG
{
    INFO_REPEAT_MODE_ALL = 0,
    INFO_REPEAT_MODE_ONE,
    INFO_REPEAT_MODE_OFF
} INFO_REPEAT_MODE;

//=============================================================================
//                              Macro Definition
//=============================================================================

typedef void (*SERVICE_UPDATE_NOTIFY_API) (INFO_SERVICE_UPDATE_EVENT_TYPE notifyEvent, MMP_UINT32 extrData);

typedef MMP_UINT32 (*RECORD_PLAY_GET_TIME_API) (MMP_UINT32 timeOffset);

//=============================================================================
//                              Structure Definition
//=============================================================================

typedef struct INFO_VIDEO_REGION_TAG
{
    MMP_UINT32  x;
    MMP_UINT32  y;
    MMP_UINT32  width;
    MMP_UINT32  height;
    MMP_BOOL    bUpdate;
    INFO_VIDEO_REGION_RATIO ratio;
} INFO_VIDEO_REGION;

typedef struct INFO_CLIP_REGION_TAG
{
    MMP_UINT32  x;
    MMP_UINT32  y;
    MMP_UINT32  width;
    MMP_UINT32  height;
} INFO_CLIP_REGION;

typedef struct INFO_ZOOMIN_INFO_TAG
{
    INFO_ZOOMIN_TIMES zoominTimes;
    INFO_ZOOMIN_ACT   zooninAct;
    MMP_BOOL          bUpdate;
} INFO_ZOOMIN_INFO;

// network around location could be changed when moving
// use hit bit to maintain this network list
// delete the entries that have not hit
typedef struct INFO_MGR_NIT_NETOWRK_ENTRY_TAG
{
    // MSB 15 14 13 12 11 10 9 8 7 6 5 4 3 2 1 0 LSB
    //     ~~                          ~~~~~~~~~
    //     hit bit                     version_number
    MMP_UINT16 nitTblInfo;
    MMP_UINT16 network_id;
} INFO_MGR_NIT_NETWORK_ENTRY;

typedef struct INFO_MGR_NIT_INFO_TAG
{
    INFO_MGR_NIT_NETWORK_ENTRY  ptNetworkList[INFO_MAX_NIT_NETWORK_ENTRY];
    MMP_UINT32                  networkCount;
    MMP_UINT32                  networkIndex;

    // TRANSPORT STREAM ID LIST FORMAT
    // MSB 31 --------------------- 16, 15 ------------------ 0 LSB
    //        ~~~~~~~~~~~~~~~~~~~~~~       ~~~~~~~~~~~~~~~~~~
    //                     bandwidth       transport_stream_id
    MMP_UINT32                  ptTransportStreamIdList[INFO_MAX_TRANSPORT_STREAM_ID_ENTRY];
    MMP_UINT32                  transportStreamCount;

    // FREQUENCY LIST ENTRY FORMAT
    //              transport_stream_id entry index(4 bits)
    //              ~~~~24
    // MSB 31 ---------------------- 24, 23 ----------- 0 LSB
    //        ~~~~28
    //        network entry id index        frequency
    MMP_UINT32                  ptFreqList[INFO_MAX_FREQUENCY_ENTRY];

    MMP_UINT32                  frequencyCount;
    MMP_UINT32                  frequencyIndex;
} INFO_MGR_NIT_INFO;

typedef struct INFO_CHANNEL_TAG
{
    MMP_UINT32  bandwidth;
    MMP_UINT32  frequency;
    MMP_UINT32  original_network_id;
    MMP_UINT32  transport_stream_id;

    MMP_UINT32  patVersion;
    MMP_UINT32  sdtVersion;
    MMP_UINT32  tvPmtCount;
    MMP_UINT32  rdPmtCount;
    MMP_UINT32  tvSdtCount;
    MMP_UINT32  rdSdtCount;
    MMP_UINT32  tvServiceCount; // modify for new scan
    MMP_UINT32  rdServiceCount; // modify for new scan
    INFO_SDT_STATE sdtState;    // modify for new scan

    INFO_MGR_NIT_INFO   tNitInfo;
    MMP_BOOL            bNitReady;

    MMP_UINT16  pPmtPidTbl[64];
    MMP_UINT32  totalServiceCount;
} INFO_CHANNEL;

typedef struct INFO_639_LANG_TAG
{
    MMP_CHAR    charCode[4];
} INFO_639_LANG;

typedef struct INFO_3166_COUNTRY_CODE_TAG
{
    MMP_CHAR    charCode[4];
} INFO_3166_COUNTRY_CODE;

typedef struct INFO_SDT_ENTRY_TAG
{
    MMP_UINT32          service_id;
    INFO_SERVICE_TYPE   serviceType;
    MMP_UINT8*          pServiceName;
    MMP_UINT32          nameSize;
} INFO_SDT_ENTRY;

typedef struct INFO_SDT_NODE_TAG
{
    INFO_SDT_ENTRY              tSdtEntry;
    MMP_UINT32                  original_network_id;
    MMP_UINT32                  transport_stream_id;
    MMP_UINT32                  frequency;
    struct INFO_SDT_NODE_TAG*   ptNextNode;
} INFO_SDT_NODE;

typedef struct INFO_SDT_LIST_TAG
{
    INFO_SDT_NODE*  ptFirstNode;
    INFO_SDT_NODE*  ptLastNode;
} INFO_SDT_LIST;

typedef struct INFO_EIT_ENTRY_TAG
{
    PSI_MJDBCD_TIME start_time;
    MMP_UINT32      duration;
    MMP_UINT8*      pEventName;
    MMP_UINT8*      pText;
    MMP_UINT8       nameSize;
    MMP_UINT8       textSize;
    MMP_UINT16      rating;
#ifdef ENABLE_EXTENDED_EVENT
    MMP_UINT8*      pExtendText;
    MMP_UINT32      extendTextSize;
#endif
} INFO_EIT_ENTRY;

#if 0
typedef struct INFO_EIT_PRESENT_FOLLOW_TAG
{
    INFO_EIT_ENTRY* ptPresentEvent;
    INFO_EIT_ENTRY* ptFollowEvent;
} INFO_EIT_PRESENT_FOLLOW;

typedef struct INFO_EIT_ENTRY_NODE_TAG
{
    //MMP_UINT32                      service_id;
    INFO_EIT_ENTRY                  tEitEntry;
    struct INFO_EIT_ENTRY_NODE_TAG* ptNextNode;
} INFO_EIT_ENTRY_NODE;

typedef struct INFO_EIT_SCHEDULE_TAG
{
    MMP_UINT32              service_id;
    MMP_UINT32              version_number[INFO_MGR_MAX_SCHEDULE_TBL];
    MMP_UINT32              size[INFO_MGR_MAX_SCHEDULE_TBL];
    MMP_UINT32              section_number[INFO_MGR_MAX_SCHEDULE_TBL][8];
    INFO_EIT_ENTRY_NODE*    pFirstTblNode[INFO_MGR_MAX_SCHEDULE_TBL];
    INFO_EIT_ENTRY_NODE*    pLastTblNode[INFO_MGR_MAX_SCHEDULE_TBL];
    INFO_EIT_ENTRY_NODE*    ptFirstNode;
} INFO_EIT_SCHEDULE;

typedef struct INFO_EIT_PRESENT_FOLLOW_NODE_TAG
{
    INFO_EIT_PRESENT_FOLLOW                     tEitData;
    MMP_UINT32                                  channelIndex;
    struct INFO_EIT_PRESENT_FOLLOW_NODE_TAG*    ptNextNode;
} INFO_EIT_PRESENT_FOLLOW_NODE;

typedef struct INFO_EIT_SCHEDULE_NODE_TAG
{
    INFO_EIT_SCHEDULE                   tEitData;
    MMP_UINT32                          channelIndex;
    MMP_UINT32                          totalSize;
    struct INFO_EIT_SCHEDULE_NODE_TAG*  ptNextNode;
} INFO_EIT_SCHEDULE_NODE;

typedef struct INFO_EIT_PRESENT_FOLLOW_LIST_TAG
{
    INFO_EIT_PRESENT_FOLLOW_NODE*  ptFirstNode;
    INFO_EIT_PRESENT_FOLLOW_NODE*  ptLastNode;
} INFO_EIT_PRESENT_FOLLOW_LIST;

typedef struct INFO_EIT_SCHEDULE_LIST_TAG
{
    INFO_EIT_SCHEDULE_NODE*     ptFirstNode;
    INFO_EIT_SCHEDULE_NODE*     ptLastNode;
} INFO_EIT_SCHEDULE_LIST;
#endif

typedef struct INFO_SERVICE_MAP_ENTRY_TAG
{
    MMP_UINT16  serviceNumber;
    MMP_UINT16  serviceIndex;
    MMP_UINT32  attribute;
} INFO_SERVICE_MAP_ENTRY;

typedef struct INFO_SERVICE_MAP_CLUSTER_TAG
{
    INFO_SERVICE_MAP_ENTRY*     ptServiceMap;
    MMP_UINT32                  totalCount;
} INFO_SERVICE_MAP_CLUSTER;

typedef struct INFO_SERVICE_MAP_NODE_TAG
{
    INFO_SERVICE_MAP_ENTRY              tServiceMap;
    struct INFO_SERVICE_MAP_NODE_TAG*   ptPrevNode;
    struct INFO_SERVICE_MAP_NODE_TAG*   ptNextNode;
} INFO_SERVICE_MAP_NODE;

typedef struct INFO_SERVICE_MAP_LIST_TAG
{
    INFO_SERVICE_MAP_NODE*  ptFirstNode;
    INFO_SERVICE_MAP_NODE*  ptLastNode;
    MMP_UINT32              totalCount;
} INFO_SERVICE_MAP_LIST;

typedef struct INFO_LANG_ENTRY_TAG
{
    INFO_639_LANG   ptLangCode[MAX_AUDIO_PID_NUMBER];
    MMP_UINT32      ttxSubtitleCount;
    MMP_UINT32      totalCount;
} INFO_LANG_ENTRY;

typedef struct INFO_MFN_INFO_TAG
{
    INFO_MFN_MODE   mfnMode;
    MMP_UINT32      smartMfnThreshold;
    MMP_UINT32      exhaustedMfnThreshold;
} INFO_MFN_INFO;

// Note: once subtitleIndex is equal to INVALID_SUBTITLE_INDEX,
//       then the subtitle service is turnned off.
typedef struct INFO_SERVICE_TAG
{
    MMP_UINT16              audioPID[MAX_AUDIO_PID_NUMBER];
    MMP_UINT16              audioCODEC[MAX_AUDIO_PID_NUMBER];
    INFO_639_LANG           ptAudioLangCode[MAX_AUDIO_PID_NUMBER];
    INFO_AUDIO_TYPE         audioType[MAX_AUDIO_PID_NUMBER];
    MMP_UINT16              audioIndex; // Used to indicate audio PID
    MMP_UINT16              audioCount;

    MMP_UINT32              subtitlePID[MAX_SUBTITLE_PID_NUMBER];
    INFO_639_LANG           ptSubtitleLangCode[MAX_SUBTITLE_PID_NUMBER];
    MMP_UINT16              subtitlingType[MAX_SUBTITLE_PID_NUMBER];
    MMP_UINT16              subtitleIndex; // Used to indicate subtitle PID
    MMP_UINT16              subtitleCount;

    MMP_UINT16              teletextPID[MAX_TELETEXT_PID_NUMBER];
    MMP_UINT16              teletextInitPageNumber[MAX_TELETEXT_PID_NUMBER];
    INFO_639_LANG           ptTeletextLangCode[MAX_TELETEXT_PID_NUMBER];
    MMP_UINT16              lastSelectedTeletextIndex;
    MMP_UINT16              teletextIndex; // Used to indicate teletext PID
    MMP_UINT16              teletextCount;

    MMP_UINT16              teletextSubtitleIndex;
    MMP_UINT16              teletextSubtitlePID[MAX_TELETEXT_SUBTITLE_PID_NUMBER];
    MMP_UINT16              teletextSubtitlePageNumber[MAX_TELETEXT_SUBTITLE_PID_NUMBER];
    INFO_639_LANG           ptTeletextSubtitleLangCode[MAX_TELETEXT_SUBTITLE_PID_NUMBER];

    MMP_UINT16              teletextSubtitleCount;

    MMP_UINT16              pcrPID;
    MMP_UINT16              channelIndex;

    MMP_UINT16              service_id; // program_number
    MMP_UINT16              videoPID;
    MMP_UINT16              videoType;
} INFO_SERVICE;

typedef struct INFO_MGR_SERVICE_MAPPING_ENTRY_TAG
{
    MMP_UINT32 service_id;
    MMP_UINT32 channelIndex;
    MMP_UINT32 pmtServiceIndex;
} INFO_MGR_SERVICE_MAPPING_ENTRY;

//typedef struct INFO_MGR_VIDEO_FILE_ENTRY_TAG
//{
//    MMP_UINT8   pFilePath[INFO_VIDEO_FILE_MAXIMUM_PATH_SIZE];
//    MMP_UINT32  pathLength;
//    MMP_UINT32  bookmark;
//    MMP_UINT32  callCount; // used to erase the rare used one entry if no avaiable entry.
//} INFO_MGR_VIDEO_FILE_ENTRY;

typedef struct INFO_MGR_RECORD_FILE_INFO_TAG
{
    MMP_UINT32                              index;
    MMP_UINT32                              totalMs; // total ms of the file.
    MMP_UINT32                              fileSize;
    struct INFO_MGR_RECORD_FILE_INFO_TAG    *ptNextInfo;
} INFO_MGR_RECORD_FILE_INFO;

typedef struct INFO_MGR_YMDHMS_TIME_TAG
{
    MMP_UINT16  year;
    MMP_UINT8   month;
    MMP_UINT8   day;
    MMP_UINT8   dayOfWeek;

    MMP_INT8    hour;
    MMP_INT8    minute;
    MMP_INT8    second;
} INFO_MGR_YMDHMS_TIME;

typedef struct INFO_MGR_RECORD_CONTEXT_TAG
{
    MMP_BOOL                    bUsed;
    INFO_SERVICE_TYPE           serviceType;
    MMP_UINT32                  recordServiceIndex;

    MMP_UINT32                  recordFileIndex;

    MMP_UINT32                  maxFileSize;
    INFO_MGR_RECORD_FILE_INFO*  ptFileInfo;

    MMP_UINT32                  playIndex;
    MMP_UINT32                  readPos;

    INFO_MGR_YMDHMS_TIME        tStartDateTime; // YMDHS_Time
    MMP_UINT32                  bookmark;
    MMP_UINT32                  videoPID;
    MMP_UINT32                  videoType;
    MMP_UINT32                  audioPID;
    MMP_UINT32                  audioType;
#ifdef ENABLE_PVR_SUBTITLE
    MMP_UINT32                  subtitlePID;
#endif
    MMP_UINT32                  seekStepSize;
    MMP_UINT32                  fileSystemFormat;
    MMP_CHAR                    pFilePath[256];
    MMP_WCHAR*                  pServiceName;
    MMP_WCHAR*                  pEventName;
    MMP_UINT32                  extNameLength;
    MMP_UINT32                  timeOffset;
    RECORD_PLAY_GET_TIME_API    pfGetTimeFunc;
} INFO_MGR_RECORD_CONTEXT;

typedef struct INFO_MGR_TIMESHIFT_RECORD_CONTEXT_TAG
{
    INFO_MGR_RECORD_CONTEXT tRecordContext;
    //MMP_UINT32              dropBufferTime;
    MMP_UINT32              splitFileSize;
    MMP_UINT32              firstFileIndex;
    MMP_UINT32              maxFileCount;
} INFO_MGR_TIMESHIFT_RECORD_CONTEXT;

typedef struct INFO_MGR_TIMESHIFT_CONFIG_TAG
{
    MMP_WCHAR   driveName[3]; // A:, B:, ...
    MMP_WCHAR   pFileDirPath[32]; // RECORD/TIMESHIFT ..
    MMP_WCHAR   pFileNamePrefix[32]; // DTV
    MMP_WCHAR   pExtName[4]; // ts
    MMP_UINT32  extNameLength;
} INFO_MGR_TIMESHIFT_CONFIG;

#if 0
typedef struct INFO_MGR_RECORD_INFO_FILE_TAG
{
    MMP_WCHAR                   pServiceName[256];
    MMP_WCHAR                   pEventName[256];
    INFO_MGR_YMDHMS_TIME        tStartDateTime; // YMDHS_Time
    MMP_UINT32                  bookmark;
    MMP_UINT32                  videoPID;
    MMP_UINT32                  videoType;
    MMP_UINT32                  audioPID;
    MMP_UINT32                  auidoType;
    INFO_MGR_RECORD_FILE_INFO*  ptFileInfo;
    MMP_UINT32                  checkSum;
} INFO_MGR_RECORD_INFO_FILE;
#endif

typedef struct INFO_SCHEDULE_DATA_ENTRY_TAG
{
    INFO_SERVICE_TYPE       serviceType;
    MMP_UINT32              serviceIndex;
    PSI_MJDBCD_TIME         startTime;
    PSI_MJDBCD_TIME         endTime;
    PSI_MJDBCD_TIME         duration;
    INFO_SCHEDULE_MODE      tMode;
    INFO_SCHEDULE_FUNCTION  tFunction;
    MMP_BOOL                bValid;
} INFO_SCHEDULE_DATA_ENTRY;

typedef struct INFO_SCHEDULE_DATA_TAG
{
    INFO_SCHEDULE_DATA_ENTRY tEntry[16];
    MMP_INT count;
} INFO_SCHEDULE_DATA;

#if 0
typedef struct INFO_AUTO_SCAN_OPERATE_TAG
{
    MMP_BOOL                    bSwitch;
    PSI_MJDBCD_TIME             tAutoScanTime;
    INFO_AUTO_SCAN_REPETITION   tRepetition;
} INFO_AUTO_SCAN_OPERATE;

typedef struct INFO_AUTO_SCAN_STANDBY_TAG
{
    MMP_BOOL    bSwitch;
} INFO_AUTO_SCAN_STANDBY;

typedef struct INFO_LAST_SCAN_TIME_TAG
{
    MMP_UINT32  lastScanDay;
    MMP_UINT32  lastScanHour;
    MMP_UINT32  lastScanMin;
}INFO_LAST_SCAN_TIME;

typedef struct INFO_OTA_SETTING_TAG
{
    MMP_UINT32  OUI;
    MMP_BOOL    bForceUpgrade;
    MMP_BOOL    bAutoSearch;
    MMP_UINT32  bandwidthType;
    MMP_UINT32  channelIndex;
} INFO_OTA_SETTING;

typedef struct INFO_FACI_LO_CAP_TAG
{
    MMP_UINT32 frequency;
    MMP_UINT8 capValue;
} INFO_FACI_LO_CAP;
#endif

typedef struct INFO_CONFIG_TAG
{
    //////////////////////////////////////////////////////////////////////////
    // The following information will be stored in the "config" file.
    //////////////////////////////////////////////////////////////////////////
    // don't remove this field
    MMP_UINT32                      configStart;

    // service
    MMP_UINT32                      activeServiceType;

    // service - tv
    MMP_UINT32                      tvActiveServiceIndex;
    MMP_UINT32                      tvPrevActiveServiceIndex;

    // service - radio
    MMP_UINT32                      rdActiveServiceIndex;
    MMP_UINT32                      rdPrevActiveServiceIndex;

    // other
    MMP_BOOL                        bLcnSwitch;
    INFO_DVBT_SPEC_ID               specId;
    INFO_MFN_INFO                   mfnInfo;
    MMP_UINT32                      weakSignalTimeout;
    MMP_UINT32                      weakSignalThreshold;
    MMP_INT32                       volume;
    MMP_UINT32                      languageIndex; // OSD language index
    MMP_BOOL                        bSubtitleSwitch;
    INFO_639_LANG                   preferEventLangB;
    INFO_639_LANG                   preferEventLangT;
    INFO_639_LANG                   preferAudioLangB;
    INFO_639_LANG                   preferAudioLangT;
    INFO_639_LANG                   preferSubtitleLangB;
    INFO_639_LANG                   preferSubtitleLangT;
    MMP_INT32                       timeZone;
    INFO_VIDEO_RATIO                videoRatio;
    INFO_VIDEO_REGION_RATIO         panelRatio;
    INFO_VIDEO_REGION_RATIO         displayRatio;
    INFO_VIDEO_OUT_MODE             videoOutMode;
    MMP_BOOL                        bAfdSwitch;
    MMP_BOOL                        bSummerTimeSwitch;
    MMP_BOOL                        bSummerTimeStatus;
    INFO_CHAR_CODE                  defaultCharCode;

    INFO_SERVICE_IDENTIFY           serviceIdentifyBasis;
    MMP_UINT32                      serviceBlockBasis; // CA basis to block service
    MMP_UINT32                      serviceBanBasis;
    MMP_UINT32                      menuLock;
    MMP_UINT32                      channelLock;
    MMP_UINT32                      maturityLevel;
    MMP_UINT32                      password;
    MMP_UINT32                      universalPassword;

    INFO_SCHEDULE_DATA              tScheduleData;
    MMP_BOOL                        bScheduleStatus;
    MMP_UINT32                      currWakeupIndex;
    INFO_TV_OUT_MODE                tvOutMode;
    MMP_BOOL                        bAutoPowerDown; // for sleep timer
    //PAL_KEYPAD                      cheatCode[INFO_CHEAT_CODE_KEY_NUM];
    MMP_BOOL                        bCheatMode;
    MMP_BOOL                        bAntPowerSwitch;

    //INFO_AUTO_SCAN_STANDBY          tAutoScanStandby;
    //INFO_AUTO_SCAN_OPERATE          tAutoScanOperate;
    //INFO_LAST_SCAN_TIME             lastScanTime;

#if defined (HAVE_FAT) && defined (HAVE_PVR)
    // video playback saved file
    //INFO_MGR_VIDEO_FILE_ENTRY       ptVideoFileArray[INFO_VIDEO_MAXIMUM_ENTRY_COUNT];
    MMP_WCHAR                       *videoCaptureFilePath;
//    INFO_RECORD_FORMAT              recordFormat;
    INFO_MGR_TIMESHIFT_CONFIG       tTimeshiftConfig;
    MMP_BOOL                        bEnableTimeshift;
    MMP_UINT32                      timeshiftBufferSize; // KB
    MMP_UINT32                      timeshiftSplitFileSize; // KB
    MMP_UINT32                      recordDuration; // ms
    MMP_UINT32                      recordTarget;
#endif

    // audioIndex, subtitleIndex, teletextIndex, teletextSubtitleIndex
    MMP_UINT8                       pTvAudioIndex[INFO_MGR_MAX_TV_SERVICE];
    MMP_UINT8                       pTvSubtitleIndex[INFO_MGR_MAX_TV_SERVICE];
    MMP_UINT8                       pTvTeletextIndex[INFO_MGR_MAX_TV_SERVICE];
    MMP_UINT8                       pTvTeletextSubtitleIndex[INFO_MGR_MAX_TV_SERVICE];
    MMP_UINT8                       pRdAudioIndex[INFO_MGR_MAX_RD_SERVICE];
    MMP_BOOL                        bSuspended;
    INFO_SCART_OUT                  scartOut;
    MMP_UINT32                      forceReset;
    MMP_UINT32                      lastWindow;
    MMP_UINT32                      newService;

    // color control
    MMP_UINT32                      hue;
    MMP_UINT32                      saturation;
    MMP_UINT32                      contrast;
    MMP_UINT32                      brightness;
    MMP_UINT32                      tsi;
    MMP_BOOL                        bHierarchicalTx;

    // AC3 mode
    MMP_UINT32                      ac3Mode;
    //////////////////////////////////////////////////////////////////////////
    // The following information will be stored in the "service" file.
    //////////////////////////////////////////////////////////////////////////

    // don't remove this field
    MMP_UINT32                      serviceStart;

#ifdef ENABLE_MFN
    INFO_BAND_FREQUENCY_TYPE        vhfType;
    INFO_BAND_FREQUENCY_TYPE        uhfType;
#endif

    // country
    INFO_3166_COUNTRY_CODE          country_code;
    MMP_UINT32                      country_number;
    INFO_3166_COUNTRY_CODE          osd_country_code;
    MMP_UINT32                      osd_country_number;

    // channel
    INFO_CHANNEL                    ptChannel[INFO_MGR_MAX_CHANNEL];
    MMP_UINT32                      totalChannelCount;

    // service
    INFO_SERVICE_COUNT_BASE         serviceCountBase;
    MMP_UINT16                      tvTotalPmtServiceCount;
    MMP_UINT16                      rdTotalPmtServiceCount;
    MMP_UINT16                      tvTotalSdtServiceCount;
    MMP_UINT16                      rdTotalSdtServiceCount;

    // mapping table
    INFO_MGR_SERVICE_MAPPING_ENTRY  ptTvMappingTbl[INFO_MGR_MAX_TV_SERVICE];
    MMP_UINT32                      tvMappingEntryCount;
    INFO_MGR_SERVICE_MAPPING_ENTRY  ptRdMappingTbl[INFO_MGR_MAX_RD_SERVICE];
    MMP_UINT32                      rdMappingEntryCount;

#if defined (SUPPORT_OTA) && defined (ENABLE_DSM_CC)
    INFO_OTA_SETTING                tOtaSetting;
#endif

    // FACI LO-CAP detection value, default must be 0xff
    //INFO_FACI_LO_CAP                ptFaciLoCap[INFO_MGR_MAX_FACI_LO_CAP];
    //MMP_UINT8                       indexFaciLoCap;


    // Vincent note: MUST add new config items here.

    // don't remove this field
    MMP_UINT32                      serviceEnd;
} INFO_CONFIG;

typedef struct INFO_RECORD_EXTERNAL_BUFFER_INFO_TAG
{
    MMP_UINT8*  pRecordBuffer[INFO_RECORD_BUFFER_COUNT];
    MMP_UINT32  bufferCount;
    MMP_UINT32  bufferSize;
} INFO_RECORD_EXTERNAL_BUFFER_INFO;

#if 0
typedef struct INFO_LCN_CONFLICT_DATA_TAG
{
    MMP_UINT32         conflictLCN;
    MMP_UINT32         serviceIndex_possessor;
    MMP_UINT32         serviceIndex_candidate;
    INFO_SERVICE_TYPE  serviceType_possessor;
    INFO_SERVICE_TYPE  serviceType_candidate;
} INFO_LCN_CONFLICT_DATA;

typedef struct INFO_LCN_LIST_TAG
{
    MMP_UINT32         LCN;
    MMP_UINT32         serviceIndex;
    INFO_SERVICE_TYPE  serviceType;
} INFO_LCN_LIST;

//typedef struct INFO_IMAGE_DISPLAY_INFO_TAG
//{
//    MMP_UINT32 displayX;
//    MMP_UINT32 displayY;
//    MMP_UINT32 displayWidth;
//    MMP_UINT32 displayHeight;
//
//    MMP_UINT32 zoomX;
//    MMP_UINT32 zoomY;
//    MMP_UINT32 zoomWidth;
//    MMP_UINT32 zoomHeight;
//
//    MMP_BOOL   bUpdate;
//} INFO_IMAGE_DISPLAY_INFO;

typedef struct INFO_USRDEF_SERVICE_TAG
{
    //////////
    // channel (INFO_CHANNEL)
    //////////
    MMP_UINT32              bandwidth;
    MMP_UINT32              frequency;
    MMP_UINT32              original_network_id;
    MMP_UINT32              transport_stream_id;

    //////////
    // service
    //////////

    //---INFO_MGR_SERVICE_MAPPING_ENTRY
    MMP_UINT32              service_id;
    INFO_SERVICE_TYPE       serviceType;    // DTV_TV_SERVICE = 0, DTV_RADIO_SERVICE

    //---INFO_SERVICE_MAP_LIST
    MMP_UINT32              serviceNumber;  // (LCN)
    MMP_UINT32              attribute;

    //---INFO_SDT_LIST (INFO_SDT_ENTRY)
    MMP_UINT8*              pServiceName;   // dvb text
    MMP_UINT32              nameSize;

    //---INFO_SERVICE
    MMP_UINT32              pcrPID;

    MMP_UINT16              videoPID;
    MMP_UINT16              videoType;

    MMP_UINT16              audioPID;
    MMP_UINT16              audioCODEC;     // AUDIO_MP3_CODEC = 0, AUDIO_AC3_CODEC, AUDIO_AAC_CODEC
    INFO_AUDIO_TYPE         audioType;

    MMP_UINT16              subtitlePID;
    MMP_UINT16              subtitlingType; // NORMAL_SUBTITLE = 0, HARD_OF_HEARING_SUBTITLE

    MMP_UINT16              teletextPID;
    MMP_UINT16              teletextInitPageNumber;
} INFO_USRDEF_SERVICE;

// The callback function will be called when requested firmware header check.
typedef MMP_BOOL (*INFO_OTA_VERIFY_CALLBACK) (MMP_UINT8* ptBuffer);

//=============================================================================
//                              Extern Reference
//=============================================================================

//=============================================================================
//                              Function Declaration
//=============================================================================

//=============================================================================
/**
 * Init the whole info manager.
 * @serviceCountBase    service counts by PMT or SDT.
 * @return  whether the operation is success or failed.
 */
//=============================================================================
MMP_BOOL
infoMgr_Init(
    INFO_SERVICE_COUNT_BASE serviceCountBase);

//=============================================================================
/**
 * Terminate the whole table manager.
 *
 * @return  Whether the operation is success or failed.
 */
//=============================================================================
MMP_BOOL
infoMgr_Terminate(
    void);

//=============================================================================
/**
 * check if info manager is running.
 *
 * @return  Whether the operation is success or failed.
 */
//=============================================================================
MMP_BOOL
infoMgr_IsRunning(
    void);

//=============================================================================
/**
 * Register the dynamic service update callback function.
 * @pfCallback  The callback fucntion for dynamic services data update.
 *                             update.
 * @return  none.
 */
//=============================================================================
void
infoMgr_RegisterServiceUpdateCallback(
    SERVICE_UPDATE_NOTIFY_API pfCallback);

//=============================================================================
/**
 * Used to lock The whole information system for utilize shared resource.
 * This API should be only call for access EIT event only.
 *
 * @return none
 */
//=============================================================================
void
infoMgr_LockSystem(
    void);

//=============================================================================
/**
 * Used to unlock The whole information system for utilize shared resource.
 * This API should be only call for access EIT event only.
 *
 * @return none
 */
//=============================================================================
void
infoMgr_UnlockSystem(
    void);
#endif

#if 1
#define infoMgr_UpdatePidFilter(a,b,c)
#else
//=============================================================================
/**
 * Update demod PID filter.
 *
 * @param demodId   The specific demod.
 * @param pid       filter PID value.
 * @param type      The specific PID entry index.
 * @return none
 */
//=============================================================================
void
infoMgr_UpdatePidFilter(
    MMP_UINT32              demodId,
    MMP_UINT32              pid,
    INFO_PID_FILTER_INDEX   index);

//=============================================================================
/**
 * Turn on/off the hardware filter of demod.
 *
 * @param demodId   The specific demod.
 * @param bFilterOn Whether the hardware filter is on or not.
 * @return none
 */
//=============================================================================
void
infoMgr_ResetPidFilter(
    MMP_UINT32  demodId,
    MMP_BOOL    bFilterOn);
#endif

//=============================================================================
/**
 * Update the PAT related information of a service. If the service is inexist,
 * then insert the service to the service array. Otherwise, update the existed
 * service.
 *
 * @param ptPat     The pointer to a pmt(PSI_PAT_INFO) structure.
 * @return none
 */
//=============================================================================
void
infoMgr_UpdatePat(
    void* ptPat);

//=============================================================================
/**
 * Update the PMT related information of a service. If the service is inexist,
 * then insert the service to the service array. Otherwise, update the existed
 * service.
 *
 * @param ptPmt     The pointer to a pmt(PSI_PMT_INFO) structure.
 * @return none
 */
//=============================================================================
void
infoMgr_UpdatePmt(
    void* ptPmt);

#if 0
//=============================================================================
/**
 * Update the frequency channel NIT information by temp NIT table found through
 * channel scan period.
 * call this function in channel scan after waiting for nit
 * (no matter nit ready or timeout)
 *
 * @param none
 * @return none
 */
//=============================================================================
void
infoMgr_UpdateNitByTmpNitTbl(
    void);
#endif

//=============================================================================
/**
 * Update the NIT related information
 *
 * @param ptNit     The pointer to a nit(PSI_NIT_INFO) structure.
 * @return none
 */
//=============================================================================
void
infoMgr_UpdateNit(
    void* ptNit);

//=============================================================================
/**
 * Update the SDT related information of a service. If the SDT data is inexistent,
 * then insert the data to the SDT list. Otherwise, update the existed SDT
 * data.
 *
 * @param ptSdt     The pointer to a sdt(PSI_SDT_INFO) structure.
 * @return          Whether the SDT is valid or not.
 */
//=============================================================================
MMP_BOOL
infoMgr_UpdateSdt(
    void* ptSdt);

#if 0
MMP_BOOL
infoMgr_FilterEitSection(
    void*   ptSection);

//=============================================================================
/**
 * Update the Eit related information of a service. If the EIT data is inexistent,
 * then insert the data to the EIT list. Otherwise, update the existed EIT
 * data.
 *
 * @param eitType   The type of EIT table.
 * @param ptEit     The pointer to a sdt(PSI_EIT_INFO) structure.
 * @return none
 */
//=============================================================================
void
infoMgr_UpdateEit(
    INFO_EIT_TYPE   eitType,
    void*           ptEit);

//=============================================================================
/**
 * Update the TDT related information. What this function do is only update the
 * current UTC time.
 *
 * @param tUtcTime  The current UTC time.
 * @return none
 */
//=============================================================================
void
infoMgr_UpdateTdt(
    PSI_MJDBCD_TIME tUtcTime);

//=============================================================================
/**
 * Update the TOT related information. It not only update the current UTC time,
 * but also ...
 *
 * @param ptTot     The pointer to a tot(PSI_TOT_INFO) structure.
 * @return none
 */
//=============================================================================
void
infoMgr_UpdateTot(
    void* ptTot);

//=============================================================================
/**
 * Get the NIT related information of a specific channel Index.
 *
 * @param channelIndex      The desired channel index
 * @param ptOutNit          The NIT information of the specific frequency channel.
 * @return  Whether the operation is success or not.
 */
//=============================================================================
MMP_BOOL
infoMgr_GetNit(
    MMP_UINT32          channelIndex,
    INFO_MGR_NIT_INFO*  ptOutNit);


//=============================================================================
/**
 * Get the Sdt related information of a specific resource Id. The return data
 * structure has allocated resource. Therefore, user has to free those resource,
 * after free.
 *
 * @param serviceType       TV or Radio service
 * @param serviceIndex      The unique serviceIndex to represent a specific service.
 * @param ptSdt             The output Data with serviceName and size filled, The
 *                          field of serviceName should be free once the data is no more
 *                          use.
 * @return  Whether the operation is success or not.
 */
//=============================================================================
MMP_BOOL
infoMgr_GetSdt(
    INFO_SERVICE_TYPE   serviceType,
    MMP_UINT32          serviceIndex,
    INFO_SDT_ENTRY*     ptSdtEntry);

//=============================================================================
/**
 * Get the present/follwoing event of EIT table of a specific resource Id.
 *
 * @param serviceType       TV or Radio service
 * @param serviceIndex      The unique serviceIndex to represent a specific service.
 * @param ptPfData          The pointer to the shared present/following entry pointer.
 * @return  Whether the operation is success or not.
 */
//=============================================================================
MMP_BOOL
infoMgr_GetPresentFollowEvent(
    INFO_SERVICE_TYPE       serviceType,
    MMP_UINT32              serviceIndex,
    PSI_MJDBCD_TIME*            ptCurrentUtcTime,
    INFO_EIT_PRESENT_FOLLOW*    ptPfData);

MMP_BOOL
infoMgr_GetOneDayEventIndexRange(
    INFO_SERVICE_TYPE       serviceType,        // [in]
    MMP_UINT32              serviceIndex,       // [in]
    PSI_MJDBCD_TIME*        ptCurrentLocalTime, // [in]
    MMP_UINT                dayOffset,          // [in]
    MMP_UINT*               pOutStartIdx,       // [out]
    MMP_UINT*               pOutCount);         // [out]

MMP_BOOL
infoMgr_GetSingleEventByIndex(
    INFO_SERVICE_TYPE       serviceType,
    MMP_UINT32              serviceIndex,
    MMP_UINT                index,
    INFO_EIT_ENTRY**        ptEvent);

MMP_BOOL
infoMgr_GetSingleEventByStartTime(
    INFO_SERVICE_TYPE       serviceType,
    MMP_UINT32              serviceIndex,
    PSI_MJDBCD_TIME*        ptStartUtcTime,
    INFO_EIT_ENTRY**        ptEvent);

//=============================================================================
/**
 * Get the schedule event list of EIT table of a specific resource Id.
 *
 * @param serviceType       TV or Radio service
 * @param serviceIndex      The unique serviceIndex to represent a specific service.
 * @param ptEventNode       The pointer to the schedule event entry pointer.
 * @return  Whether the operation is success or not.
 */
//=============================================================================
MMP_BOOL
infoMgr_GetScheduleEvent(
    INFO_SERVICE_TYPE       serviceType,
    MMP_UINT32              serviceIndex,
    INFO_EIT_ENTRY_NODE**   ptEventNode);

//=============================================================================
/**
 * Update the Locked Channel information.
 *
 * @param ptChannel     The desired updated channel information.
 * @param pOutputIndex  The output of the index represented the channel.
 * @return none
 */
//=============================================================================
void
infoMgr_UpdateChannel(
    INFO_CHANNEL* ptChannel,
    MMP_UINT32*   pOutputIndex);

//=============================================================================
/**
 * Update the Channel information by index.
 *
 * @param channelIndex  The desired update channel Index.
 * @param ptChannel     The desired updated channel information.
 * @return none
 */
//=============================================================================
void
infoMgr_UpdateChannelByIndex(
    MMP_UINT32    channelIndex,
    INFO_CHANNEL* ptChannel);

//=============================================================================
/**
 * Get the channel information by frequency input.
 *
 * @param frequency     The looking up frequency.
 * @param ptOutChannel  The output channel data pointer.
 * @return  Whether the operation is success or failed.
 */
//=============================================================================
MMP_BOOL
infoMgr_GetChannelByFrequency(
    MMP_UINT32    frequency,
    INFO_CHANNEL* ptOutputChannel);

//=============================================================================
/**
 * Get the channel information by index input.
 *
 * @param frequency     The looking up index.
 * @param ptOutChannel  The output channel data pointer.
 * @return  Whether the operation is success or failed.
 */
//=============================================================================
MMP_BOOL
infoMgr_GetChannel(
    MMP_UINT32    channelIndex,
    INFO_CHANNEL* ptOutputChannel);

//=============================================================================
/**
 * Get the channel index by frequency input.
 *
 * @param frequency         The looking up frequency.
 * @param ptChannelIndex    The output channel index pointer.
 * @return  Whether the operation is success or failed.
 */
//=============================================================================
MMP_BOOL
infoMgr_GetChannelIndex(
    MMP_UINT32    frequency,
    MMP_UINT32*   ptChannelIndex);

//=============================================================================
/**
 * Set the frequency to be the active channel.
 *
 * @param channelIndex     The desired active frequency channel
 * @return  Whether the operation is success or not.
 */
//=============================================================================
MMP_BOOL
infoMgr_SetActiveChannel(
    MMP_UINT32  channelIndex);

//=============================================================================
/**
 * Get a active channel.
 *
 * @param pChannelIndex     The output of active channel index.
 * @return  Whether the operation is success or not.
 */
//=============================================================================
MMP_BOOL
infoMgr_GetActiveChannelIndex(
    MMP_UINT32* pChannelIndex);

//=============================================================================
/**
 * Get total service of a specific channel
 *
 * @param serviceType       TV or Radio service
 * @param channelIndex      The specific index of a frequency channel.
 * @return  Total services of the channel of a serviceType.
 */
//=============================================================================
MMP_UINT32
infoMgr_GetServiceCountByChannel(
    INFO_SERVICE_TYPE   serviceType,
    MMP_UINT32          channelIndex);

//=============================================================================
/**
 * Update a flag for a specific service
 *
 * @param serviceType       TV or Radio service
 * @param serviceIndex      The unique serviceIndex to represent a specific service.
 * @param attribute         The attribute type that is willing to update.
 * @param bToggle           Toggle on/off
 * @return  Whether the operation is success or not.
 */
//=============================================================================
MMP_BOOL
infoMgr_SetServiceAttribute(
    INFO_SERVICE_TYPE   serviceType,
    MMP_UINT32          serviceIndex,
    MMP_UINT32          attribute,
    MMP_BOOL            bToggle);

//=============================================================================
/**
 * Update a flag for LCN switch setting
 *
 * @param bOn   flag on or off
 * @return none
 */
//=============================================================================
void
infoMgr_SetLcn(
    MMP_BOOL bOn);

//=============================================================================
/**
 * Get the current LCN switch setting
 *
 * @return  Whether the LCN switch setting is on or off.
 */
//=============================================================================
MMP_BOOL
infoMgr_GetLcn(
    void);

//=============================================================================
/**
 * Setup the MFN status
 *
 * @param mode                      The preference MFN mode.
 * @param smartMfnThreshold         The threshold of smark mode, the param is
 *                                  only usable when DTV_MFN_COMBINATION_MODE.
 * @param exhaustedMfnThreshold     The threshold of exhausted mode, the param is
 *                                  only usable when DTV_MFN_COMBINATION_MODE.
 * @return      none
 */
//=============================================================================
void
infoMgr_SetMfn(
    INFO_MFN_MODE mode,
    MMP_UINT32   smartMfnThreshold,
    MMP_UINT32   exhaustedMfnThreshold);

//=============================================================================
/**
 * Get the current MFN mode setting.
 *
 * @return  Which mode is set.
 */
//=============================================================================
INFO_MFN_MODE
infoMgr_GetMfn(
    void);

//=============================================================================
/**
 * Set weak signal timeout to do MFN handover.
 *
 * @param timeout   timeout value.
 * @return none.
 */
//=============================================================================
#ifdef ENABLE_MFN
void
infoMgr_SetWeakSignalTimeout(
    MMP_UINT32 timeout);

//=============================================================================
/**
 * Get weak signal timeout.
 *
 * @return timeout value.
 */
//=============================================================================
MMP_UINT32
infoMgr_GetWeakSignalTimeout(
    void);
#endif

//=============================================================================
/**
 * Get the current NIT update setting
 *
 * @return  Whether the NIT update setting is on or off.
 */
//=============================================================================
MMP_BOOL
infoMgr_GetNitUpdate(
    void);

//=============================================================================
/**
 * Get Hop frequency count of active channel.
 *
 * @return  Hop frequency count of active channel.
 */
//=============================================================================
//MMP_UINT32
//infoMgr_GetHopFrequencyCount(
//    void);

//=============================================================================
/**
 * Clear whole hit bit of network information of NIT table.
 *
 * @return  none.
 */
//=============================================================================
void
infoMgr_ClearNitHitInfo(
    void);

//=============================================================================
/**
 * Change the fequency info of active channel through the NIT frequency table.
 *
 * @param pFrequency            The output of the next frequency.
 * @param pBandwidth            The output of the frequency bandwidth.
 * @param pTransportStreamId    The output of the transport_stream_id.
 * @param pOriginalNetworkId    The output of the original_network_id;
 * @param pHitIndex             The output of the hit related entry index.
 * @return                      Whether the setting is succes or not.
 */
//=============================================================================
MMP_BOOL
infoMgr_GetNextMfnHopFrequencyInfo(
    MMP_UINT32* pFrequency,
    MMP_UINT32* pBandwidth,
    MMP_UINT32* pTransportStreamId,
    MMP_UINT32* pOriginalNetworkId,
    MMP_UINT32* pHitIndex);

//=============================================================================
/**
 * Update the db with the active MFN hop frequency info.
 *
 * @param frequencyIndex     The desired active frequency index.
 * @return                   none.
 */
//=============================================================================
MMP_BOOL
infoMgr_SetActiveMfnHopFrequencyInfo(
    MMP_UINT32 frequencyIndex);

//=============================================================================
/**
 * Update a specific service information.
 *
 * @param serviceType       TV or Radio service
 * @param serviceIndex      The unique serviceIndex to represent a specific service.
 * @param ptNewService      The new content of service.
 * @return  Whether the operation is success or not.
 */
//=============================================================================
MMP_BOOL
infoMgr_UpdateService(
    INFO_SERVICE_TYPE   serviceType,
    MMP_UINT32          serviceIndex,
    INFO_SERVICE*       ptNewService);

//=============================================================================
/**
 * Get current active service type.
 *
 * @return  current active service type.
 */
//=============================================================================
INFO_SERVICE_TYPE
infoMgr_GetActiveServiceType(
    void);

//=============================================================================
/**
 * Set current active service type.
 *
 * @return  Whether the operation is success or not.
 */
//=============================================================================
MMP_BOOL
infoMgr_SetActiveServiceType(
    INFO_SERVICE_TYPE   serviceType);

//=============================================================================
/**
 * Get a specific service information.
 *
 * @param serviceType       TV or Radio service
 * @param serviceIndex      The unique serviceIndex to represent a specific service.
 * @param ptOutput          The output of the looking service.
 * @return  Whether the operation is success or not.
 */
//=============================================================================
MMP_BOOL
infoMgr_GetService(
    INFO_SERVICE_TYPE   serviceType,
    MMP_UINT32          serviceIndex,
    INFO_SERVICE*       ptOutput);

//=============================================================================
/**
 * Get channel index and service_id of a specific service.
 *
 * @param serviceType       TV or Radio service
 * @param serviceIndex      The unique serviceIndex to represent a specific service.
 * @param pChannelIndex     The channel index of the looking service.
 * @param pServiceId        The service id of the looking service.
 * @return  Whether the operation is success or not.
 */
//=============================================================================
MMP_BOOL
infoMgr_GetMappingInfoByServiceIndex(
    INFO_SERVICE_TYPE   serviceType,
    MMP_UINT32          serviceIndex,
    MMP_UINT32*         pChannelIndex,
    MMP_UINT32*         pServiceId);

//=============================================================================
/**
 * Get the ISO_639_LANGUAGE code content of audio, subtitle or teletext.
 *
 * @param serviceType       TV or Radio service
 * @param serviceIndex      The unique serviceIndex to represent a specific service.
 * @param ptLangEntry       The language code out put entry.
 * @param entryType         The language code entry type.
 * @return  Whether the operation is success or not.
 */
//=============================================================================
MMP_BOOL
infoMgr_GetLangCode(
    INFO_SERVICE_TYPE       serviceType,
    MMP_UINT32              serviceIndex,
    INFO_LANG_ENTRY_TYPE    entryType,
    INFO_LANG_ENTRY*        ptLangEntry);

//=============================================================================
/**
 * Get the current active service.
 *
 * @param serviceType   TV or Radio service
 * @param ptOutput      The output of the looking service.
 * @return  Whether the operation is success or not.
 */
//=============================================================================
MMP_BOOL
infoMgr_GetActiveService(
    INFO_SERVICE_TYPE serviceType,
    INFO_SERVICE*     ptOutput);

//=============================================================================
/**
 * Get the current active service index.
 *
 * @param serviceType   TV or Radio service
 * @return  Valid service index or not.
 */
//=============================================================================
MMP_UINT32
infoMgr_GetActiveServiceIndex(
    INFO_SERVICE_TYPE serviceType);

//=============================================================================
/**
 * Get the service index of first service of the program list.
 *
 * @param serviceType   TV or Radio service
 * @return  Valid service index or not.
 */
//=============================================================================
MMP_UINT32
infoMgr_GetFirstServiceIndex(
    INFO_SERVICE_TYPE serviceType);

//=============================================================================
/**
 * The API will retrieve the next service index of current service.
 *
 * @param serviceType   TV or Radio service.
 * @param serviceIndex  The unique serviceIndex to represent a specific service.
 * @return  Valid service index or not.
 */
//=============================================================================
MMP_UINT32
infoMgr_GetNextServiceIndex(
    INFO_SERVICE_TYPE serviceType,
    MMP_UINT32        serviceIndex);

//=============================================================================
/**
 * The API will retrieve the previous service index of request service index.
 *
 * @param serviceType       TV or Radio service.
 * @param serviceIndex      The unique service index to represent a specific
 *                          service.
 * @return  Valid service index or not.
 */
//=============================================================================
MMP_UINT32
infoMgr_GetPrevServiceIndex(
    INFO_SERVICE_TYPE serviceType,
    MMP_UINT32        serviceIndex);

//=============================================================================
/**
 * Get a specific service number (logical channel number).
 *
 * @param serviceType       TV or Radio service
 * @param serviceIndex      The unique service index to represent a specific
 *                          service.
 * @return  The service number (logical channel number), 0 is invalid channel
 *          number.
 */
//=============================================================================
MMP_UINT32
infoMgr_GetServiceNumber(
    INFO_SERVICE_TYPE   serviceType,
    MMP_UINT32          serviceIndex);

//=============================================================================
/**
 * Get a specific service attribute.
 *
 * @param serviceType       TV or Radio service
 * @param serviceIndex      The unique service index to represent a specific
 *                          service.
 * @return  The service attribute.
 */
//=============================================================================
MMP_UINT32
infoMgr_GetServiceAttribute(
    INFO_SERVICE_TYPE   serviceType,
    MMP_UINT32          serviceIndex);

//=============================================================================
/**
 * Get the map data array by query parameters.
 *
 * @param serviceType   TV or Radio service.
 * @param queryType     The query type of list search.
 * @param parameter     The parameter field is only useful for two types -
                        FAVOR_ONLY or FREQUENCY_ONLY. In case FAVOR_ONLY,
                        the field is represent the favor id, frequency, otherwise.
 * @param ptOutput      The output map array of request type.
 * @return  Whether the operation is success or not.
 */
//=============================================================================
MMP_BOOL
infoMgr_GetServiceMapCluster(
    INFO_SERVICE_TYPE   serviceType,
    INFO_QUERY_TYPE     queryType,
    MMP_UINT32          parameter,
    INFO_SERVICE_MAP_CLUSTER* ptOutput);

//=============================================================================
/**
 * Select a specific service to be the active service.
 *
 * @param serviceType       TV or Radio service
 * @param serviceIndex      index of the specific service.
 * @return  Whether the operation is success or not.
 */
//=============================================================================
MMP_BOOL
infoMgr_SetActiveService(
    INFO_SERVICE_TYPE   serviceType,
    MMP_UINT32          serviceIndex);

//=============================================================================
/**
 * The API will retrieve the service index of previous active service.
 *
 * @param serviceType       TV or Radio service
 * @param pServiceIndex     The index of the previous active service.
 * @return  Whether the operation is success or not.
 */
//=============================================================================
MMP_BOOL
infoMgr_GetPrevActiveServiceIndex(
    INFO_SERVICE_TYPE serviceType,
    MMP_UINT32*       pServiceIndex);

//=============================================================================
/**
 * The API will retrieve the current active sound track of the service.
 *
 * @param serviceIndex      Index of the specific service.
 * @param ptAudioIndex      The output of current sound track index.
 * @return  The current audio sound track index.
 */
//=============================================================================
MMP_BOOL
infoMgr_GetActiveSoundTrack(
    MMP_UINT32  serviceIndex,
    MMP_UINT32* ptAudioIndex);

//=============================================================================
/**
 * The API will change the active sound track of the service.
 *
 * @param serviceIndex      Index of the specific service.
 * @param audioIndex        The new sound track.
 * @return  Whether the operation is success or not.
 */
//=============================================================================
MMP_BOOL
infoMgr_SetActiveSoundTrack(
    MMP_UINT32  serviceIndex,
    MMP_UINT32  audioIndex);

//=============================================================================
/**
* The API will get sound track type of the service.
*
* @param serviceType       TV or Radio service
* @param serviceIndex      Index of the specific service.
* @param pCodecType        The output of audio sound track type,AC3 or MP3.
* @param pAudioType        Audio type which specifies the type of stream
* @return  Whether the operation is success or not.
*/
//=============================================================================
MMP_BOOL
infoMgr_GetSoundTrackType(
    INFO_SERVICE_TYPE      serviceType,
    MMP_UINT32             serviceIndex,
    INFO_AUDIO_CODEC_TYPE* pCodecType,
    INFO_AUDIO_TYPE*       pAudioType);

//=============================================================================
/**
 * The API will retrieve the current active subtitle of the service.
 *
 * @param serviceIndex      Index of the specific service.
 * @param ptSubtitleIndex   The output of current subtitle index.
 * @return                  Whether the active subtitle channel exists or the
 *                          service is turned off.
 */
//=============================================================================
MMP_BOOL
infoMgr_GetActiveSubtitle(
    MMP_UINT32  serviceIndex,
    MMP_UINT32* ptSubtitleIndex,
    INFO_SUBTITLING_TYPE* ptSubtitlingType);

//=============================================================================
/**
 * The API will change the active subtitle of the service.
 *
 * @param serviceIndex      Index of the specific service.
 * @param subtitleIndex     The new subtitle index, if the index is the INVALID
 *                          index, then the subtitle service is turned off.
 * @return                  Whether the operation is success or not.
 */
//=============================================================================
MMP_BOOL
infoMgr_SetActiveSubtitle(
    MMP_UINT32  serviceIndex,
    MMP_UINT32  subtitleIndex);

//=============================================================================
/**
* Used to get whether use system default subtitle index or not.
*
* @param serviceIndex          Index of the specific service.
* @return                      whether use system default subtitle index or not
*/
//=============================================================================
//MMP_BOOL
//infoMgr_GetSysSubtitle(
//    MMP_UINT32  serviceIndex);

//=============================================================================
/**
* Used to set whether use system default subtitle index or not.
*
* @param serviceIndex   Index of the specific service.
* @param bOn            Whether use system default subtitle index or not.
*/
//=============================================================================
//void
//infoMgr_SetSysSubtitle(
//    MMP_UINT32  serviceIndex,
//    MMP_UINT32  bOn);

//=============================================================================
/**
 * The API will retrieve the current active teletext of the service.
 *
 * @param serviceIndex      Index of the specific service.
 * @param ptTeletextIndex   The output of current teletext index.
 * @return                  Whether the active teletext channel exists or the
 *                          service is turned off.
 */
//=============================================================================
MMP_BOOL
infoMgr_GetActiveTeletext(
    MMP_UINT32  serviceIndex,
    MMP_UINT32* ptTeletextIndex);

//=============================================================================
/**
 * The API will retrieve the last selected teletext of the service.
 *
 * @param serviceIndex      Index of the specific service.
 * @param ptTeletextIndex   The output of the last selected teletext index.
 * @return                  Whether the last selected teletext is valid or not.
 */
//=============================================================================
MMP_BOOL
infoMgr_GetLastSelectedTeletext(
    MMP_UINT32  serviceIndex,
    MMP_UINT32* ptTeletextIndex);

//=============================================================================
/**
 * The API will retrieve the initial page number of specific teletext stream
 * of the service.
 * @param serviceIndex      Unique index map to a service.
 * @param teletextIndex     the specific teletext index.
 * @return the initial page number if the return value is >= 0x100
 */
//=============================================================================
MMP_UINT32
infoMgr_GetTeletextInitPageNumber(
    MMP_UINT32  serviceIndex,
    MMP_UINT32  teletextIndex);

//=============================================================================
/**
 * The API will change the active teletext of the service.
 *
 * @param serviceIndex      Index of the specific service.
 * @param teletextIndex     The new teletext index, if the index is the INVALID
 *                          index, then the teletext service is turned off.
 * @return                  Whether the operation is success or not.
 */
//=============================================================================
MMP_BOOL
infoMgr_SetActiveTeletext(
    MMP_UINT32  serviceIndex,
    MMP_UINT32  teletextIndex);

//=============================================================================
/**
 * The API will retrieve the page number of specific teletext subtitle stream
 * of the service.
 * @param serviceIndex      Unique index map to a service.
 * @param teletextIndex     the specific teletext index.
 * @return the page number if the return value is >= 0x100
 */
//=============================================================================
MMP_UINT32
infoMgr_GetTeletextSubtitlePageNumber(
    MMP_UINT32  serviceIndex,
    MMP_UINT32  teletextIndex);

//=============================================================================
/**
 * The API will retrieve the current active teletext subtitle of the service.
 *
 * @param serviceIndex              Index of the specific service.
 * @param ptTeletextSubtitleIndex   The output of current teletext subtitle
 *                                  index.
 * @return Whether the active teletext channel exists or the service is turned
 *         off.
 */
//=============================================================================
MMP_BOOL
infoMgr_GetActiveTeletextSubtitle(
    MMP_UINT32  serviceIndex,
    MMP_UINT32* ptTeletextSubtitleIndex);

//=============================================================================
/**
 * The API will change the active teletext subtitle of the service.
 *
 * @param serviceIndex              Index of the specific service.
 * @param ptTeletextSubtitleIndex   The new teletext subtitle index, if the
 *                                  index is the INVALID index, then the
 *                                  teletext subtitle service is turned off.
 * @return Whether the operation is success or not.
 */
//=============================================================================
MMP_BOOL
infoMgr_SetActiveTeletextSubtitle(
    MMP_UINT32  serviceIndex,
    MMP_UINT32  teletextSubtitleIndex);

//=============================================================================
/**
 * Get the current system volume.
 *
 * @return the current system volume.
 * @remark
 *  This function is only called by smtkDtvXXXVolume related functions. Anyone
 *  who wants to get the system volume please call smtkDtvGetVolume().
 */
//=============================================================================
MMP_INT32
infoMgr_GetVolume(
    void);

//=============================================================================
/**
 * Set the current system volume.
 *
 * @return the current system volume.
 * @remark
 *  This function is only called by smtkDtvXXXVolume related functions. Anyone
 *  who wants to change the system volume please call smtkDtvXXXVolume() related
 *  functions.
 */
//=============================================================================
void
infoMgr_SetVolume(
    MMP_INT32 volume);
#endif

//=============================================================================
/**
 * Get the current system UTC time.
 *
 * @return the current system UTC time.
 */
//=============================================================================
PSI_MJDBCD_TIME
infoMgr_GetCurrentUtcTime(
    void);

#if 0
PSI_MJDBCD_TIME
infoMgr_TimeSub(
    PSI_MJDBCD_TIME tPsiMJDBCDTime,
    MMP_UINT32 bcdHMS);

PSI_MJDBCD_TIME
infoMgr_TimeAdd(
    PSI_MJDBCD_TIME tPsiMJDBCDTime,
    MMP_INT32 bcdHMS);

MMP_INT
infoMgr_TimeCompare(
    PSI_MJDBCD_TIME tPsiMJDBCDTimeA,
    PSI_MJDBCD_TIME tPsiMJDBCDTimeB);

//=============================================================================
/**
 * Get total service count by service type.
 *
 * @param serviceType   TV or Radio service
 * @return total service count for specified service type.
 */
//=============================================================================
MMP_UINT32
infoMgr_GetServiceCount(
    INFO_SERVICE_TYPE serviceType,
    MMP_BOOL bAllocate);

//=============================================================================
/**
 * Get the time zone.
 *
 * @return The time zone.
 */
//=============================================================================
MMP_INT32
infoMgr_GetTimeZone(
    void);

//=============================================================================
/**
 * Set the time zone.
 *
 * @param timeZone  The time zone.
 * @return none.
 */
//=============================================================================
void
infoMgr_SetTimeZone(
    MMP_INT32 timeZone);

//=============================================================================
/**
* Get the current Subtitle switch setting.
*
* @return  Whether the Subtitle switch setting is on or off.
*/
//=============================================================================
MMP_BOOL
infoMgr_GetSubtitle(
    void);

//=============================================================================
/**
* Update a flag for Subtitle switch setting.
*
* @param bOn   flag on or off.
* @return none
*/
//=============================================================================
void
infoMgr_SetSubtitle(
    MMP_BOOL bOn);

#ifdef ENABLE_MFN
void
infoMgr_SetFrequencyBandType(
    INFO_BAND_FREQUENCY_TYPE vhfType,
    INFO_BAND_FREQUENCY_TYPE uhfType);
#endif

INFO_CONFIG*
infoMgr_GetCoreConfig(
    void);

INFO_SERVICE**
infoMgr_GetCoreTvService(
    void);

INFO_SERVICE**
infoMgr_GetCoreRdService(
    void);

MMP_UINT32
infoMgr_GetCoreConfigVersion(
    void);

MMP_UINT32
infoMgr_GetEitUpdateTimes(
    void);

MMP_UINT32
infoMgr_GetTdtUpdateTimes(
    void);

MMP_UINT32
infoMgr_GetTotUpdateTimes(
    void);

MMP_BOOL
infoMgr_SaveService(
    void);

MMP_UINT16
infoMgr_LoadConfig(
   MMP_UINT32 fileSize,
   MMP_UINT8* pBufferStart);

MMP_RESULT
infoMgr_LoadService(
    MMP_UINT32 fileSize,
    MMP_UINT8* pBufferStart);

MMP_BOOL
infoMgr_ResetConfig(
    void);

MMP_BOOL
infoMgr_IsResetConfig(
    void);

void
infoMgr_FreeScheduleEvent(
    void);

MMP_BOOL
infoMgr_IsLoadConfigFail(
    void);

void
infoMgr_CopyEvent(
    INFO_EIT_ENTRY* ptDstEvent,
    INFO_EIT_ENTRY* ptSrcEvent);

void
infoMgr_FreeEvent(
    INFO_EIT_ENTRY* ptEvent);
#endif

//=============================================================================
/**
 * Get the current video ratio.
 * @param none.
 * @return the current video ratio.
 */
//=============================================================================
INFO_VIDEO_RATIO
infoMgr_GetVideoRatio(
    void);

//=============================================================================
/**
 * Set the current video ratio.
 * @param videoRatio    The current system video ratio.
 * @return none.
 */
//=============================================================================
void
infoMgr_SetVideoRatio(
    INFO_VIDEO_RATIO videoRatio);

//=============================================================================
/**
* Get the panel ratio.
* @param none.
* @return the panel ratio.
*/
//=============================================================================
INFO_VIDEO_REGION_RATIO
infoMgr_GetPanelRatio(
    void);

//=============================================================================
/**
* Set the panel ratio.
* @param panelRatio    The panel ratio set from HOST side.
* @return none.
*/
//=============================================================================
void
infoMgr_SetPanelRatio(
    INFO_VIDEO_REGION_RATIO panelRatio);

//=============================================================================
/**
* Get the current AFD switch setting.
*
* @return  Whether the AFD switch setting is on or off.
*/
//=============================================================================
MMP_BOOL
infoMgr_GetAfd(
    void);

//=============================================================================
/**
* Update a flag for AFD switch setting
*
* @param bOn   flag on or off
* @return none
*/
//=============================================================================
void
infoMgr_SetAfd(
    MMP_BOOL bOn);

//=============================================================================
/**
 * Get the video region.
 *
 * @param ptVideoRegion     The pointer of video region x, y, width, height.
 * @return none.
 */
//=============================================================================
void
infoMgr_GetVideoRegion(
    INFO_VIDEO_REGION* ptVideoRegion);

//=============================================================================
/**
 * Set the video region.
 *
 * @param tVideoRegion      The video region x, y, width, height.
 * @return none.
 */
//=============================================================================
void
infoMgr_SetVideoRegion(
    INFO_VIDEO_REGION tVideoRegion);

//=============================================================================
/**
* Get the video clip region.
*
* @param ptClipRegion     The pointer of video clip region x, y, width, height.
* @return none.
*/
//=============================================================================
void
infoMgr_GetClipRegion(
    INFO_CLIP_REGION* ptClipRegion);

//=============================================================================
/**
* Set the video clip region.
*
* @param tClipRegion      The video clip region x, y, width, height.
* @return none.
*/
//=============================================================================
void
infoMgr_SetClipRegion(
    INFO_CLIP_REGION tClipRegion);

//=============================================================================
/**
* Get the ZoomIn video info.
*
* @param The pointer of ZoomIn video info: times, act.
* @return none.
*/
//=============================================================================
void
infoMgr_GetZoomInInfo(
    INFO_ZOOMIN_INFO* ptZoomIn);

//=============================================================================
/**
* Set the ZoomIn video info.
*
* @param tVideoRegion      The ZoomIn video info: times, act.
* @return none.
*/
//=============================================================================
void
infoMgr_SetZoomInInfo(
    INFO_ZOOMIN_INFO tZoomIn);

void
infoMgr_CalcCheckSum(
    MMP_UINT8* pResult,
    MMP_UINT8* pData,
    MMP_UINT32 dwCount);

#if 0
//=============================================================================
/**
 * Set data by data type.
 * @param type      data type.
 * @param value     data value.
 * @return none.
 */
//=============================================================================
PAL_KEYPAD*
infoMgr_GetCheatCode(
   void);

void
infoMgr_SetCheatCode(
   PAL_KEYPAD* cheatCode);

void
infoMgr_SetData(
    INFO_DATA_TYPE type,
    MMP_UINT32 value);

//=============================================================================
/**
 * Get data by data type.
 * @param type      data type.
 * @return data value.
 */
//=============================================================================
MMP_UINT32
infoMgr_GetData(
    INFO_DATA_TYPE type);

//=============================================================================
/**
 * Set whether the state is scan or not scan state.
 * @param bInscan  in scan or not in scan state.
 * @return none.
 */
//=============================================================================
void
infoMgr_SetScanState(
    MMP_BOOL bInScan);

//=============================================================================
/**
 * Set service to ascend, order forward.
 * @param serviceType   service type , tv or radio.
 * @param serviceIndex  service index.
 * @return true if service order change, otherwise false.
 */
//=============================================================================
MMP_BOOL
infoMgr_SetServiceAscend(
    INFO_SERVICE_TYPE serviceType,
    MMP_UINT32 serviceIndex);

//=============================================================================
/**
 * Set service to descend, order backward.
 * @param serviceType   service type, tv or radio.
 * @param serviceIndex  service index.
 * @return true if service order change, otherwise false.
 */
//=============================================================================
MMP_BOOL
infoMgr_SetServiceDescend(
    INFO_SERVICE_TYPE serviceType,
    MMP_UINT32 serviceIndex);

//=============================================================================
/**
 * Set service to insert below target service.
 * @param serviceType           service type, tv or radio.
 * @param serviceIndex          service index.
 * @param targetServiceIndex    target service index.
 * @return true if service order change, otherwise false.
 */
//=============================================================================
MMP_BOOL
infoMgr_SetServiceInsert(
    INFO_SERVICE_TYPE serviceType,
    MMP_UINT32 serviceIndex,
    MMP_UINT32 targetServiceIndex);

//=============================================================================
/**
 * Sort service by service number.
 * @param serviceType   The service type, TV or Radio.
 * @return none
 */
//=============================================================================
void
infoMgr_SortService(
    INFO_SERVICE_TYPE serviceType);

//=============================================================================
/**
 * Set the default character coding. The default character coding is used to
 * interpret the DVB text string if the first byte of the text field doesn't
 * specify the character coding.
 * @param defaultCharCode   The default character coding.
 * @return none
 */
//=============================================================================
void
infoMgr_SetDefaultCharCode(
    INFO_CHAR_CODE defaultCharCode);

//=============================================================================
/**
 * Get the default character coding. The default character coding is used to
 * interpret the DVB text string if the first byte of the text field doesn't
 * specify the character coding.
 * @return The default character coding
 */
//=============================================================================
INFO_CHAR_CODE
infoMgr_GetDefaultCharCode(
    void);

//=============================================================================
/**
 * Set Channel Hardware PID filter and acquire channel.
 * @param demodId       The specific demod if there is more than one.
 * @param channelIndex  To setup channel hardware filter by specific channel info.
 * @return Whether the operation is success or fail.
 */
//=============================================================================
MMP_BOOL
infoMgr_SwitchChannel(
    MMP_UINT32 demodId,
    MMP_UINT32 channelIndex);

//=============================================================================
/**
 * Get whether any unused demod.
 * @param pDemodId  The output of free demod Id.
 * @return Whether the operation is success or fail.
 */
//=============================================================================
MMP_BOOL
infoMgr_GetFreeDemodId(
    MMP_UINT32* pDemodId);

//=============================================================================
/**
 * Release the demod control right back to system.
 * @param demodId  The specific demod id for relase process.
 * @return Whether the demod id is valid or not.
 */
//=============================================================================
MMP_BOOL
infoMgr_ReleaseDemodId(
    MMP_UINT32 demodId);

//=============================================================================
/**
 * Set the main demod Id for on air play purpose.
 * @param demodId  The specific demod id to be the stream src of on air play.
 * @return Whether the demod id is valid or not.
 */
//=============================================================================
MMP_BOOL
infoMgr_SetMainDemodId(
    MMP_UINT32 demodId);
#endif

//=============================================================================
/**
 * Get the main demod Id of the system
 * @param pDemodId  The output of main demod Id.
 * @return Whether the main demod Id is exist or not.
 */
//=============================================================================
MMP_BOOL
infoMgr_GetMainDemodId(
    MMP_UINT32* pDemodId);

#if 0
//=============================================================================
/**
 * Release the main demod Id for on air play purpose.
 * @return none
 */
//=============================================================================
void
infoMgr_ReleaseMainDemodId(
    void);

//=============================================================================
/**
 * Enable or disable data input of specific demod
 * @param demodId  The specific demod id.
 * @return Whether the demod Id is valid or not.
 */
//=============================================================================
//MMP_BOOL
//infoMgr_SetTsiDataInput(
//    MMP_UINT32 demodId,
//    MMP_BOOL   bEnable);

MMP_UINT32
infoMgr_GetBanServiceCount(
    INFO_SERVICE_TYPE serviceType);
#endif

void
infoMgr_AssignServiceNumber(
    MMP_BOOL bSort);

#if 0
INFO_LCN_CONFLICT_DATA*
infoMgr_GetConflictData(
    void);

void
infoMgr_AssignConflictServiceNumber(
    MMP_BOOL   bSelectCandidate);

MMP_UINT32
infoMgr_GetAvailableServiceIndex(
    INFO_SERVICE_TYPE   serviceType,
    INFO_QUERY_TYPE     queryType,
    MMP_UINT32          serviceListIndex);

//=============================================================================
/**
 * Get service index of preferred service by service number.
 * @param serviceType           TV or Radio.
 * @param serviceNumber         Preferred service number.
 * @return service index that match preferred service number.
 */
//=============================================================================
MMP_UINT32
infoMgr_GetPreferServiceIndex(
    INFO_SERVICE_TYPE serviceType,
    MMP_UINT32        serviceNumber);

//=============================================================================
/**
 * Free the resource allocated by sdt table.
 *
 * @param eitType       The type of EIT table is willing to be free.
 * @return  none.
 */
//=============================================================================
void
infoMgr_FreeEitTable(
    INFO_EIT_TYPE eitType);

//=============================================================================
/**
* Get the local time offset information in TOT.
*
* @param ptLocalTimeOffset    The local time offset information.
* @return success or not..
*/
//=============================================================================
MMP_BOOL
infoMgr_GetLocalTimeOffsetInfo(
    PSI_LOCAL_TIME_OFFSET_DESCRIPTOR* ptLocalTimeOffset);

//=============================================================================
/**
* Set a flag for auto power down.
*
* @param bOn   flag on or off
* @return      none
*/
//=============================================================================
void
infoMgr_SetAutoPowerDown(
    MMP_BOOL bOn);

//=============================================================================
/**
* Get the auto power down setting.
*
* @return  auto power down is on or off.
*/
//=============================================================================
MMP_BOOL
infoMgr_GetAutoPowerDown(
    void);
#endif

//=============================================================================
/**
* Use to Free allocated resource
*
* @return none
*/
//=============================================================================
void
infoMgr_FreeResource(
    void);

#if 0
#if defined (HAVE_FAT) && defined (HAVE_PVR)
////=============================================================================
///**
// * Get the bookmark posistion of the video file.
// * @param pFilePath     The file path of desired video file.
// * @param pathLength    The file path byte size.
// * @param pOutBookmark  The bookmark of the video file if the file is existed.
// * @return Whether the bookmark of the video file is stored or not.
// */
////=============================================================================
//MMP_BOOL
//infoMgr_GetFileBookmark(
//    MMP_UINT8*  pFilePath,
//    MMP_UINT32  pathLength,
//    MMP_UINT32* pOutBookmark);
//
////=============================================================================
///**
// * Set the bookmark posistion of the video file.
// * @param pFilePath     The file path of desired video file.
// * @param pathLength    The file path byte size.
// * @param pOutBookmark  The bookmark of the video file if the file is existed.
// * @return none.
// */
////=============================================================================
//void
//infoMgr_SetFileBookmark(
//    MMP_UINT8*  pFilePath,
//    MMP_UINT32  pathLength,
//    MMP_UINT32  bookmark);
//=============================================================================
/**
 * Set video captured output file path.
 * @param path  The output file path.
 * @return none
 */
//=============================================================================
void
infoMgr_SetVideoCaptureFilePath(
    MMP_WCHAR   *path);

//=============================================================================
/**
 * Get video captured output file path.
 * @param path  The output file path.
 * @return none
 */
//=============================================================================
MMP_WCHAR*
infoMgr_GetVideoCaptureFilePath(
    void);

//=============================================================================
/**
 * Set record file format.
 * @param recordFormat  The record file format.
 * @return none
 */
//=============================================================================
//void
//infoMgr_SetRecordFormat(
//        INFO_RECORD_FORMAT recordFormat);

//=============================================================================
/**
 * Get record file format.
 * @return the saved record file format.
 */
//=============================================================================
//INFO_RECORD_FORMAT
//infoMgr_GetRecordFormat(
//        void);
#endif
#endif

//=============================================================================
/**
 * Set the record playback context.
 * @param ptContext ptr to the record playback context.
 * @return Whether the operation is success or not.
 */
//=============================================================================
MMP_BOOL
infoMgr_SetRecordPlaybackContext(
    INFO_MGR_RECORD_CONTEXT* ptContext);

#if 0
//=============================================================================
/**
 * Release record context
 * @param ptRecordContext The existed record context.
 * @return none
 */
//=============================================================================
void
infoMgr_ReleaseRecordContext(
    INFO_MGR_RECORD_CONTEXT* ptRecordContext);

//=============================================================================
/**
 * Get the record playback context.
 * @param pptContext ptr to the record playback context.
 * @return Whether the operation is success or not.
 */
//=============================================================================
MMP_BOOL
infoMgr_GetRecordPlaybackContext(
    INFO_MGR_RECORD_CONTEXT** pptContext);

//=============================================================================
/**
 * Get current record file play time.
 * @return current play time (ms).
 */
//=============================================================================
MMP_UINT32
infoMgr_GetCurrentPlayTime(
    void);

//=============================================================================
/**
 * Get total record file play time.
 * @return total avaliable play time (ms).
 */
//=============================================================================
MMP_UINT32
infoMgr_GetTotalPlayTime(
    void);
#endif

//=============================================================================
/**
 * Get seek time related file index and seek position.
 * @param seekMs search seek time.
 * @param bufferAlignment the buffer alignment limit.
 * @param pFileIndex output of seek file index.
 * @param pSeekPosition output of seek file position.
 * @return none.
 */
//=============================================================================
void
infoMgr_GetRecordPlaybackSeekPosition(
    MMP_UINT32  seekMs,
    MMP_UINT32  bufferAlignment,
    MMP_UINT32* pFileIndex,
    MMP_UINT32* pSeekPosition);

#if 0
#if 0
//=============================================================================
/**
 * Get total dropped buffer time (for timeshift case only and will be reset
 * after read)
 * @return total avaliable play time (ms).
 */
//=============================================================================
//MMP_UINT32
//infoMgr_GetTimeShiftDropBufferTime(
//    void);

//=============================================================================
/**
 * Set the timeshift record config.
 * @param ptTimeShiftConfig ptr to the config of time shift record.
 * @return none.
 */
//=============================================================================
void
infoMgr_SetTimeShiftConfig(
    INFO_MGR_TIMESHIFT_CONFIG* ptTimeShiftConfig);

//=============================================================================
/**
 * Get the timeshift record config.
 * @param ptTimeShiftConfig ptr to the return config of time shift record.
 * @return none.
 */
//=============================================================================
void
infoMgr_GetTimeShiftConfig(
    INFO_MGR_TIMESHIFT_CONFIG* ptTimeShiftConfig);

//=============================================================================
/**
 * Get record context by specific demod id
 * @param demodId          The specific demod id a specific record context.
 * @param pptRecordContext The output pointer to free record context.
 * @return Whether any free entry existed or not.
 */
//=============================================================================
MMP_BOOL
infoMgr_GetRecordContext(
    MMP_UINT32                  demodId,
    INFO_MGR_RECORD_CONTEXT**   pptRecordContext);

//=============================================================================
/**
 * Reset record context by demod id.
 * @param demodId          The specific demod id  to a specific record context.
 * @return Whether the entry existed or not.
 */
//=============================================================================
MMP_BOOL
infoMgr_ResetRecordContextByDemodId(
    MMP_UINT32 demodId);

//=============================================================================
/**
 * Is any recording processing of a specific demod Id?
 * @param demodId          The specific demod id a specific record context.
 * @return Whether in recording or not.
 */
//=============================================================================
MMP_BOOL
infoMgr_IsInRecordByDemodId(
    MMP_UINT32 demodId);

//=============================================================================
/**
 * Get timeshift context
 * @param pptTimeShiftContext The output pointer to timeshift context.
 * @return Whether the operation is success or not.
 */
//=============================================================================
MMP_BOOL
infoMgr_GetTimeShiftContext(
    INFO_MGR_TIMESHIFT_RECORD_CONTEXT**   pptTimeShiftContext);

//=============================================================================
/**
 * Reset timeshift context.
 * @return Whether the operation is success or not.
 */
//=============================================================================
MMP_BOOL
infoMgr_ResetTimeShiftContext(
    void);

//=============================================================================
/**
 * Is In timeshift record/play.
 * @return Whether the operation is success or not.
 */
//=============================================================================
MMP_BOOL
infoMgr_IsInTimeShift(
    void);

//=============================================================================
/**
* Convert record context and save into info file
*
* @param ptRecordContext    The input record context.
* @param pWriteBuffer       The output data buffer
* @param dataSize           the output size of converted info file.
* @return  none.
*/
//=============================================================================
void
infoMgr_ConvertRecordContextToInfoData(
    INFO_MGR_RECORD_CONTEXT*    ptRecordContext,
    MMP_UINT8*                  pWriteBuffer,
    MMP_UINT32*                 pDataSize);

//=============================================================================
/**
* Set Time shift file buffer size. // KB
*
* @param bufferSize         the time shift file buffer size of KB. 0 is disable
*                           time shift feature.
* @param splitFileSize      the split file size
* @return  none.
*/
//=============================================================================
void
infoMgr_SetTimeshiftBufferSize(
    MMP_UINT32 bufferSize,
    MMP_UINT32 splitFileSize);

//=============================================================================
/**
* Get Time shift file buffer size. // KB
*
* @return  the time shift buffer size.
*/
//=============================================================================
MMP_UINT32
infoMgr_GetTimeshiftBufferSize(
    void);

//=============================================================================
/**
* Get Time shift split file size. // KB
*
* @return  the time shift split file size.
*/
//=============================================================================
MMP_UINT32
infoMgr_GetTimeshiftSplitFileSize(
    void);

//=============================================================================
/**
* Enable/Disable time shift feature.
* @param bEnable if True, then enable time shift feature. Otherwise, disable the
*                feature.
*
* @return
*/
//=============================================================================
void
infoMgr_SetTimeshiftFeature(
    MMP_BOOL bEnable);

//=============================================================================
/**
* Enable/Disable time shift feature.
*
* @return  Whether the time shift feature is enable or disable.
*/
//=============================================================================
MMP_BOOL
infoMgr_GetTimeshiftFeature(
    void);

//=============================================================================
/**
* Set default record time duration (ms).
* @param duration The default record time duration.
* @return  none.
*/
//=============================================================================
void
infoMgr_SetRecordTimeDuration(
    MMP_UINT32 duration);

//=============================================================================
/**
* Get default record time duration. (ms)
*
* @return  the default record time duration.
*/
//=============================================================================
MMP_UINT32
infoMgr_GetRecordTimeDuration(
    void);

//=============================================================================
/**
* Set prefer record target device.
* @param device type of record target device.
* @return  none.
*/
//=============================================================================
void
infoMgr_SetRecordTargetConfig(
    MMP_UINT32 device);

//=============================================================================
/**
* Get prefer record target device.
*
* @return  the default record target device.
*/
//=============================================================================
MMP_UINT32
infoMgr_GetRecordTargetConfig(
    void);

//=============================================================================
/**
* Set active record target device.
* @param device type of record target device.
* @return  none.
*/
//=============================================================================
void
infoMgr_SetActiveRecordTarget(
    MMP_UINT32 device);

//=============================================================================
/**
* Get active record target device.
*
* @return  the active record target device.
*/
//=============================================================================
MMP_UINT32
infoMgr_GetActiveRecordTarget(
    void);

//=============================================================================
/**
 * Set the maximum record size of record file.
 * @param maximumSize   The maximum size of record file.
 * @return Whether the operation is success or not.
 */
//=============================================================================
MMP_BOOL
infoMgr_SetMaximumRecordSize(
    MMP_UINT32  maximumSize);

//=============================================================================
/**
 * Get the maximum record size of record file.
 * @param none
 * @return maxRecordFile size.
 */
//=============================================================================
MMP_UINT32
infoMgr_GetMaximumRecordSize(
    void);

//=============================================================================
/**
 * Set the external record buffer info
 * @param ptPvrBufferInfo The external record buffer instead malloc.
 * @return void.
 */
//=============================================================================
void
infoMgr_SetExternalRecordBufferInfo(
    INFO_RECORD_EXTERNAL_BUFFER_INFO* ptRecordBufferInfo);

//=============================================================================
/**
 * Get the external record buffer info
 * @param ptPvrBufferInfo The external record buffer instead malloc.
 * @return void.
 */
//=============================================================================
void
infoMgr_GetExternalRecordBufferInfo(
    INFO_RECORD_EXTERNAL_BUFFER_INFO* ptRecordBufferInfo);
#endif

void
infoMgr_SetDVBSpec(
    INFO_DVBT_SPEC_ID specId);

#if defined (SUPPORT_OTA) && defined (ENABLE_DSM_CC)
//=============================================================================
/**
 * Retrieve the OTA state.
 * @return the OTA state.
 */
//=============================================================================
INFO_OTA_STATE
infoMgr_GetOtaState(
    void);

//=============================================================================
/**
 * Retrieve the OTA PID.
 * @return the OTA pid.
 */
//=============================================================================
MMP_UINT32
infoMgr_GetOtaPid(
    void);

//=============================================================================
/**
 * Reset the OTA info.
 * @return the OTA info.
 */
//=============================================================================
INFO_OTA_STATE
infoMgr_ResetOta(
    void);

//=============================================================================
/**
 * Start the OTA data collection.
 * @param pfVerify firmware verify callback function.
 * @return none.
 */
//=============================================================================
void
infoMgr_StartOta(
    INFO_OTA_VERIFY_CALLBACK pfVerify);

//=============================================================================
/**
 * Get the OTA firmware download percentage.
 * @return download percentage.
 */
//=============================================================================
MMP_UINT32
infoMgr_GetOtaDownloadPercentage(
    void);

//=============================================================================
/**
 * Retrieve download firmware buffer.
 * @param ppFirmwareBuffer      the output of firmware buffer.
 * @param pFirmwareBufferSize   buffer size of the firmware.
 * @return download firmware buffer.
 */
//=============================================================================
MMP_BOOL
infoMgr_GetOtaFirmwareBuffer(
    MMP_UINT8** ppFirmwareBuffer,
    MMP_UINT32* pFirmwareBufferSize);

//=============================================================================
/**
 * Retrieve the OTA upgrade setting.
 * @param ptOtaSetting The output of current OTA setting.
 * @return none.
 */
//=============================================================================
void
infoMgr_GetOtaSetting(
    INFO_OTA_SETTING* ptOtaSetting);

//=============================================================================
/**
 * Set the OTA upgrade setting.
 * @param ptOtaSetting The new setting OTA system.
 * @return none.
 */
//=============================================================================
void
infoMgr_SetOtaSetting(
    INFO_OTA_SETTING* ptOtaSetting);
#endif

#ifdef ENABLE_DSM_CC
//=============================================================================
/**
 * update DSM_CC information.
 * @return none.
 */
//=============================================================================
MMP_BOOL
infoMgr_UpdateDsmCcInfo(
    void* ptDsmCc);
#endif

#ifdef AFA_ORION_TUNER
//=============================================================================
/**
 * Operate FACI lo_cap
 */
//=============================================================================
void
infoMgr_GetFaciLoCapList(
    INFO_FACI_LO_CAP* ptList);

void
infoMgr_ResetFaciLoCapList(
    void);

void
infoMgr_SaveFaciLoCap(
    MMP_UINT32 frequency);

void
infoMgr_GetFaciLoCapValue(
    MMP_UINT32 frequency,
    MMP_UINT8* lo_cap);
#endif

MMP_BOOL
infoMgr_AddUsrdefService(
    INFO_USRDEF_SERVICE* ptUsrService,
    MMP_UINT32           count);

void
infoMgr_SetApMode(
    INFO_AP_MODE apMode);

INFO_AP_MODE
infoMgr_GetApMode(
    void);

MMP_RESULT
infoMgr_DumpUsrdefService(
   INFO_USRDEF_SERVICE** pptUsrService,
   MMP_UINT32*           pCount);

void
infoMgr_SetCollectScheduleEventState(
    MMP_BOOL enable);
#endif

#ifdef __cplusplus
}
#endif

#endif
