#ifndef	SDC_DBG_H
#define	SDC_DBG_H

#ifdef __cplusplus
extern "C" {
#endif


//=============================================================================
//                              LOG definition
//=============================================================================
// Log definitions
typedef enum
{
    SDC_ZONE_ERROR      = (0x1 << 0),
    SDC_ZONE_WARNING    = (0x1 << 1),
    SDC_ZONE_INFO       = (0x1 << 2),
    SDC_ZONE_DEBUG      = (0x1 << 3),
    SDC_ZONE_ENTER      = (0x1 << 4),
    SDC_ZONE_LEAVE      = (0x1 << 5),
    SDC_ZONE_ALL        = 0xFFFFFFFF
} SDCLogZones;

#if 1
#define SDC_LOG_ZONES    (SDC_ZONE_ERROR | SDC_ZONE_WARNING)
//#define SDC_LOG_ZONES    (SDC_ZONE_ERROR | SDC_ZONE_WARNING)

#if defined(WIN32)
#define SD_PRINTF   printf
#else
#define SD_PRINTF   ithPrintf
#endif

#define LOG_ERROR   ((void) ((SDC_ZONE_ERROR & SDC_LOG_ZONES) ? (SD_PRINTF("[SDC][ERR]"
#define LOG_WARNING ((void) ((SDC_ZONE_WARNING & SDC_LOG_ZONES) ? (SD_PRINTF("[SDC][WARN]"
#define LOG_INFO    ((void) ((SDC_ZONE_INFO & SDC_LOG_ZONES) ? (SD_PRINTF("[SDC][INFO]"
#define LOG_DEBUG   ((void) ((SDC_ZONE_DEBUG & SDC_LOG_ZONES) ? (SD_PRINTF("[SDC][DBG]"
#define LOG_ENTER   ((void) ((SDC_ZONE_ENTER & SDC_LOG_ZONES) ? (SD_PRINTF("[SDC][ENTER]"
#define LOG_LEAVE   ((void) ((SDC_ZONE_LEAVE & SDC_LOG_ZONES) ? (SD_PRINTF("[SDC][LEAVE]"
#define LOG_CMD     ((void) ((false) ? (SD_PRINTF("[SDC][CMD]"
#define LOG_DATA    ((void) ((true) ? (SD_PRINTF(
#define LOG_END     )), 1 : 0));
#else

#define LOG_ZONES
#define LOG_ERROR
#define LOG_WARNING
#define LOG_INFO
#define LOG_DEBUG
#define LOG_ENTER
#define LOG_LEAVE
#define LOG_DATA
#define LOG_END         ;
#endif

#define check_result(rc)		do{ if(rc) LOG_ERROR "[%s] res = 0x%08X \n", __FUNCTION__, rc LOG_END } while(0)


#ifdef __cplusplus
}
#endif

#endif
