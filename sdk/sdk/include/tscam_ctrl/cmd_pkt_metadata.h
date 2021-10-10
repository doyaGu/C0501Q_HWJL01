#ifndef __cmd_packet_metadata_H_AJYyENwL_wNQ9_lrY5_A4lr_TpLZ0kcPC3xS__
#define __cmd_packet_metadata_H_AJYyENwL_wNQ9_lrY5_A4lr_TpLZ0kcPC3xS__

#ifdef __cplusplus
extern "C" {
#endif

#include "cmd_data_type.h"
#include "cmd_pkt_video_analytics.h"
//=============================================================================
//                Constant Definition
//=============================================================================

/**
 * metadata stream service
 **/
#define META_STRM_SERVICE_TAG_MASK          0xFF00
#define META_STRM_SERVICE_CMD_MASK          0x00FF
#define META_STRM_SERVICE_TAG               0xF000
// cmd
#define CMD_MetadataStreamOutput                                     0xF000


#define METADATA_TYPE_CNT                   7
typedef enum METADATA_TYPE_T
{
    METADATA_DEVICE_INFO                     = 0x01,
    METADATA_STREAM_INFO                     = 0x02,
    METADATA_LINE_DETECTOR                   = RULE_ENGINE_LINE_DETECTOR,
    METADATA_FIELD_DETECTOR                  = RULE_ENGINE_FIELD_DETECTOR,
    METADATA_DECLARATIVE_MOTION_DETECTOR     = RULE_ENGINE_DECLARATIVE_MOTION_DETECTOR,
    METADATA_COUNTING_RULE                   = RULE_ENGINE_COUNTING_RULE,
    METADATA_CELL_MOTION_DETECTOR            = RULE_ENGINE_CELL_MOTION_DETECTOR,

}METADATA_TYPE;

//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
/**
 * Field   Length(Byte)    Descriptions
 * RuleToken   n           [string] Unique rule token for this rule.
 * ObjectID    4           The object ID (or count).
 * UTC Hour    1           Range is 0 to 23.
 * UTC Minute  1           Range is 0 to 59.
 * UTC Second  1           Range is 0 to 61 (typically 59).
 * UTC Year    2
 * UTC Month   1           Range is 1 to 12.
 * UTC Day     1           Range is 1 to 31.
 **/
typedef struct LINE_DETECTOR_METADATA_T
{
    CMD_STRING_T    rule_token;
    uint32_t        object_id;
    uint8_t         utc_hour;
    uint8_t         utc_minute;
    uint8_t         utc_second;
    uint16_t        utc_year;
    uint8_t         utc_month;
    uint8_t         utc_day;

}LINE_DETECTOR_METADATA;

/**
 * Field   Length(Byte)    Descriptions
 * RuleToken   n           [string] Unique rule token for this rule.
 * ObjectID    4           The object ID (or count).
 * IsInside    1           0x0: Outside
 *                         0x1: Inside
 * UTC Hour    1           Range is 0 to 23.
 * UTC Minute  1           Range is 0 to 59.
 * UTC Second  1           Range is 0 to 61 (typically 59).
 * UTC Year    2
 * UTC Month   1           Range is 1 to 12.
 * UTC Day     1           Range is 1 to 31.
 **/
typedef struct FIELD_DETECTOR_METADATA_T
{
    CMD_STRING_T    rule_token;
    uint32_t        object_id;
    uint8_t         is_inside;
    uint8_t         utc_hour;
    uint8_t         utc_minute;
    uint8_t         utc_second;
    uint16_t        utc_year;
    uint8_t         utc_month;
    uint8_t         utc_day;

}FIELD_DETECTOR_METADATA;

/**
 * Field   Length(Byte)    Descriptions
 * RuleToken   n           [string] Unique rule token for this rule.
 * ObjectID    4           The object ID (or count).
 * UTC Hour    1           Range is 0 to 23.
 * UTC Minute  1           Range is 0 to 59.
 * UTC Second  1           Range is 0 to 61 (typically 59).
 * UTC Year    2
 * UTC Month   1           Range is 1 to 12.
 * UTC Day     1           Range is 1 to 31.
 **/
typedef struct DECLARATIVE_MOTION_DETECTOR_METADATA_T
{
    CMD_STRING_T    rule_token;
    uint32_t        object_id;
    uint8_t         utc_hour;
    uint8_t         utc_minute;
    uint8_t         utc_second;
    uint16_t        utc_year;
    uint8_t         utc_month;
    uint8_t         utc_day;

}DECLARATIVE_MOTION_DETECTOR_METADATA;

/**
 * Field   Length(Byte)    Descriptions
 * RuleToken   n           [string] Unique rule token for this rule.
 * ObjectID    4           The object ID (or count).
 * Count       4           Value of counter.
 * UTC Hour    1           Range is 0 to 23.
 * UTC Minute  1           Range is 0 to 59.
 * UTC Second  1           Range is 0 to 61 (typically 59).
 * UTC Year    2
 * UTC Month   1           Range is 1 to 12.
 * UTC Day     1           Range is 1 to 31.
 **/
typedef struct COUNTING_RULE_METADATA_T
{
    CMD_STRING_T    rule_token;
    uint32_t        object_id;
    uint32_t        count;
    uint8_t         utc_hour;
    uint8_t         utc_minute;
    uint8_t         utc_second;
    uint16_t        utc_year;
    uint8_t         utc_month;
    uint8_t         utc_day;

}COUNTING_RULE_METADATA;

/**
 * Field   Length(Byte)    Descriptions
 * RuleToken   n           [string] Unique rule token for this rule.
 * IsMotion    1           0x0: No motion
 *                         0x1: Motion detected (By default when receive the event)
 * UTC Hour    1           Range is 0 to 23.
 * UTC Minute  1           Range is 0 to 59.
 * UTC Second  1           Range is 0 to 61 (typically 59).
 * UTC Year    2
 * UTC Month   1           Range is 1 to 12.
 * UTC Day     1           Range is 1 to 31.
 **/
typedef struct CELL_MOTION_DETECTOR_METADATA_T
{
    CMD_STRING_T    rule_token;
    uint8_t         is_motion;
    uint8_t         utc_hour;
    uint8_t         utc_minute;
    uint8_t         utc_second;
    uint16_t        utc_year;
    uint8_t         utc_month;
    uint8_t         utc_day;

}CELL_MOTION_DETECTOR_METADATA;

/**
 * Field            Length(Byte)    Descriptions
 * Device Vendor ID     2
 * Device Model ID      2
 * HW Version Code      2
 * SW Version Code      4
 * Text Information     n           [string]
 **/
typedef struct DEV_INFO_PARAM_T
{
    uint16_t        dev_vendor_id;
    uint16_t        dev_model_id;
    uint16_t        hw_version_code;
    uint32_t        sw_version_code;
    CMD_STRING_T    text_info;

}DEV_INFO_PARAM;

/**
 * Field                      Length(Byte)    Descriptions
 * Stream Information List Size    1            List size of all stream informations. The following 10 entries would be grouped after List Size. If List Size > 1, the data would be {{group0}, {group1}}. The groupx = { All Stream Information parameters }.
 *   Video PID                       2            Video stream PID.
 *   Video Encoding Type             1            0:JPEG, 1: MPEG4, 2: H264, 3: MPEG2
 *   Video Resolution Width          2            Number of the columns of the Video image.
 *   Video Resolution Height         2            Number of the lines of the Video image.
 *   Video FrameRate                 1            Maximum output framerate in fps.
 *   Video Bitrate                   2            The maximum output bitrate in kbps.
 *   Audio PID                       2            Audio stream PID.
 *   Audio Encoding Type             1            0: G711, 1: G726, 2: AAC, 3: G729, 4: MPEG1, 5: MPEG2, 6: AC3, 7: PCM, 8: ADPCM
 *   Audio Bitrate                   2            The output bitrate in kbps.
 *   Audio SampleRate                2            The output sample rate in 100Hz.
 *   PCR PID                         2            The PCR PID of a stream.
 **/
typedef struct STREAM_INFO_DATA_T
{
    uint16_t        video_pid;
    uint8_t         video_encoding_type;
    uint16_t        video_resolution_width;
    uint16_t        video_resolution_height;
    uint8_t         video_framerate;
    uint16_t        video_bitrate;
    uint16_t        audio_pid;
    uint8_t         audio_encoding_type;
    uint16_t        audio_bitrate;
    uint16_t        audio_samplerate;
    uint16_t        pcr_pid;

}STREAM_INFO_DATA;

typedef struct STREAM_INFO_PARAMS_T
{
    uint8_t             stream_information_list_size;
    STREAM_INFO_DATA    *pStream_info_data;

}STREAM_INFO_PARAMS;


/**
 * MetadataStream Output context
 **/
typedef struct _byte_align4 MetadataStreamOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    uint16_t        version;
    METADATA_TYPE   type;
    uint8_t         *pParameters;

}MetadataStreamOutput_INFO;
//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================

//=============================================================================
//                Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif
