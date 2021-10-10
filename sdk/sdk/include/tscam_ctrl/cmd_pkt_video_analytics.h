#ifndef __cmd_packet_video_analytics_H_nOQmXz1U_bfBj_8bZF_XAcC_pBQcmdPdHaHZ__
#define __cmd_packet_video_analytics_H_nOQmXz1U_bfBj_8bZF_XAcC_pBQcmdPdHaHZ__

#ifdef __cplusplus
extern "C" {
#endif

#include "cmd_data_type.h"
//=============================================================================
//                Constant Definition
//=============================================================================
/**
 * video analytics service
 **/
#define VIDEO_ANA_SERVICE_TAG_MASK          0xFF00
#define VIDEO_ANA_SERVICE_CMD_MASK          0x00FF
#define VIDEO_ANA_INPUT_SERVICE_TAG         0x0600
#define VIDEO_ANA_OUTPUT_SERVICE_TAG        0x8600
// cmd
#define CMD_GetSupportedRulesInput                                   0x0600
#define CMD_GetSupportedRulesOutput                                  0x8600
#define CMD_GetRulesInput                                            0x0601
#define CMD_GetRulesOutput                                           0x8601
#define CMD_CreateRuleInput                                          0x0680
#define CMD_CreateRuleOutput                                         0x8680
#define CMD_ModifyRuleInput                                          0x0681
#define CMD_ModifyRuleOutput                                         0x8681
#define CMD_DeleteRuleInput                                          0x0682
#define CMD_DeleteRuleOutput                                         0x8682

typedef enum RULE_ENGINE_ITEM_T
{
    RULE_ENGINE_LINE_DETECTOR                   = 0x10,
    RULE_ENGINE_FIELD_DETECTOR                  = 0x11,
    RULE_ENGINE_DECLARATIVE_MOTION_DETECTOR     = 0x12,
    RULE_ENGINE_COUNTING_RULE                   = 0x13,
    RULE_ENGINE_CELL_MOTION_DETECTOR            = 0x14,

}RULE_ENGINE_ITEM;
//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
/**
 * the point of Polygon in Rule Engine in video analytics service
 **/
typedef struct POLYGON_POINT_T
{
    uint16_t    point_x;
    uint16_t    point_y;
}POLYGON_POINT;


/**
 * Field       Length(Byte)    Descriptions
 * Direction           2       0x00: Do not care
 *                             0x01: Right/Up to Left/Down
 *                             0x02: Left/Down to Right/Up
 * Polyline List Size  1       Number of points of the polyline.
 *                             The following 2 entries would be grouped after List Size. List Size should greater than 2, the data would be {{x,y}, {x,y}}.
 * Polyline x          2
 * Polyline y          2
 * Metadata Stream     1       0: Off, 1: On
 **/
typedef struct _byte_align4 RULE_ENG_LINE_DETECTOR_T
{
    uint16_t        direction;
    uint8_t         polyline_list_size;
    POLYGON_POINT   *pPolygon_point;
    uint8_t         metadata_stream;

}RULE_ENG_LINE_DETECTOR;


/**
 * Field       Length(Byte)    Descriptions
 * Polyline List Size  1       Number of points of the polyline.
 *                             The following 2 entries would be grouped after List Size. List Size should greater than 2, the data would be {{x,y}, {x,y}}.
 * Polyline x          2
 * Polyline y          2
 * Metadata Stream     1       0: Off, 1: On
 **/
typedef struct _byte_align4 RULE_ENG_FIELD_DETECTOR_T
{
    uint8_t         polyline_list_size;
    POLYGON_POINT   *pPolygon_point;
    uint8_t         metadata_stream;

}RULE_ENG_FIELD_DETECTOR;


/**
 * Field       Length(Byte)    Descriptions
 * MotionExpression    1       Not define now. Default is 0x0.
 * Polyline List Size  1       Number of points of the polyline.
 *                             The following 2 entries would be grouped after List Size. List Size should greater than 2, the data would be {{x,y}, {x,y}}.
 * Polyline x          2
 * Polyline y          2
 * Metadata Stream     1       0: Off, 1: On
 **/
typedef struct _byte_align4 RULE_ENG_DECLARATIVE_MOTION_DETECTOR_T
{
    uint16_t        motion_expression;
    uint8_t         polyline_list_size;
    POLYGON_POINT   *pPolygon_point;
    uint8_t         metadata_stream;

}RULE_ENG_DECLARATIVE_MOTION_DETECTOR;


/**
 * Field       Length(Byte)    Descriptions
 * ReportTimeInterval  4       Time interval to report count information.
 * ResetTimeInterval   4       Periodic count reset time
 * Direction           2       0x00: Do not care
 *                             0x01: Right/Up to Left/Down
 *                             0x02: Left/Down to Right/Up
 * Polyline List Size  1       Number of points of the polyline.
 *                             The following 2 entries would be grouped after List Size. List Size should greater than 2, the data would be {{x,y}, {x,y}}.
 * Polyline x          2
 * Polyline y          2
 * Metadata Stream     1       0: Off, 1: On
 **/
typedef struct _byte_align4 RULE_ENG_COUNTING_RULE_T
{
    uint32_t        report_time_interval;
    uint32_t        reset_time_interval;
    uint16_t        direction;
    uint8_t         polyline_list_size;
    POLYGON_POINT   *pPolygon_point;
    uint8_t         metadata_stream;

}RULE_ENG_COUNTING_RULE;


/**
 * Field           Length(Byte)    Descriptions
 * MinCount                2   Minimum count of adjacent activated cells to trigger motion. This parameter allows suppressing false alarms.
 * AlarmOnDelay            4   Minimum time in milliseconds which is needed to change the alarm state from off to on. This delay is intended to prevent very brief alarm events from triggering.
 * AlarmOffDelay           4   Minimum time in milliseconds which is needed to change the alarm state from on to off. This delay is intended to prevent too many alarm changes.
 * ActiveCellsSize         2   Number of byte of ActiveCells.
 * ActiveCells             n   [string] A "1" denotes an active cell and a "0" an inactive cell. The first cell is in the upper left corner. Then the cell order goes first from left to right and then from up to down (see Figure). If the number of cells is not a multiple of 8 the last byte is filled with zeros. The information is run length encoded according to Packbit coding in ISO 12369 (TIFF, Revision 6.0). For example, The activated cells are filled gray and the deactivated cells are white. The active cells in hex coding are: "ff ff ff f0 f0 f0". This is encoded with the Packbit algorithm: "fe ff fe f0". Finally, this packed data is base64Binary encoded: "/v/+8A==".
 * Sensitivity             1   The lower the value of the sensitivity is set, the higher the variations of
 *                             the luminance values need to be in order to be identified as motion.
 *                             Hence, a low sensitivity can be used to suppress estimated motion due
 *                             to noise.
 *                             Range is 0 to 100
 * Layout Bounds x         2   Rectangle specifying the cell grid area.
 * Layout Bounds y         2
 * Layout Bounds width     2
 * Layout Bounds height    2
 * Layout Columns          1   Number of columns of the cell grid (x dimension)
 * Layout Rows             1   Number of rows of the cell grid (y dimension).
 * Metadata Stream         1   0: Off, 1: On
 **/
typedef struct _byte_align4 RULE_ENG_CELL_MOTION_DETECTOR_T
{
    uint16_t        min_count;
    uint32_t        alarm_on_delay;
    uint32_t        alarm_off_delay;
    uint16_t        active_cells_size;
    CMD_STRING_T    active_cells;
    uint8_t         sensitivity;
    uint16_t        layout_bounds_x;
    uint16_t        layout_bounds_y;
    uint16_t        layout_bounds_width;
    uint16_t        layout_bounds_height;
    uint8_t         layout_columns;
    uint8_t         layout_rows;
    uint8_t         metadata_stream;

}RULE_ENG_CELL_MOTION_DETECTOR;

/**
 * GetSupportedRules Input context
 **/
typedef struct _byte_align4 GetSupportedRulesInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}GetSupportedRulesInput_INFO;

/**
 * GetSupportedRules Output context
 **/
typedef struct _byte_align4 GetSupportedRulesOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    uint8_t         supported_rules_list_size;
    uint8_t         *pRule_type;

}GetSupportedRulesOutput_INFO;

/**
 * GetRules Input context
 **/
typedef struct _byte_align4 GetRulesInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}GetRulesInput_INFO;

/**
 * GetRules Output context
 **/
typedef struct _byte_align4 RULE_ENGINE_PARA_T
{
    CMD_STRING_T        name;
    CMD_STRING_T        rule_token;
    RULE_ENGINE_ITEM    rule_type;
    uint8_t             *pRule_engine_items;

}RULE_ENGINE_PARA;

typedef struct _byte_align4 GetRulesOutput_INFO_T
{
    uint16_t            cmd_code;
    uint8_t             return_code;
    CMD_STRING_T        user_name;
    CMD_STRING_T        password;
    uint8_t             rule_engine_para_list_size;
    RULE_ENGINE_PARA    *pRule_engine_para;

}GetRulesOutput_INFO;

/**
 * CreateRule Input context
 **/
typedef struct _byte_align4 CreateRuleInput_INFO_T
{
    uint16_t            cmd_code;
    uint8_t             reserved;
    CMD_STRING_T        user_name;
    CMD_STRING_T        password;
    CMD_STRING_T        name;
    CMD_STRING_T        rule_token;
    RULE_ENGINE_ITEM    rule_type;
    uint8_t             *pRule_engine_items;

}CreateRuleInput_INFO;

/**
 * CreateRule Output context
 **/
typedef struct _byte_align4 CreateRuleOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}CreateRuleOutput_INFO;

/**
 * ModifyRule Input context
 **/
typedef struct _byte_align4 ModifyRuleInput_INFO_T
{
    uint16_t            cmd_code;
    uint8_t             reserved;
    CMD_STRING_T        user_name;
    CMD_STRING_T        password;
    CMD_STRING_T        name;
    CMD_STRING_T        rule_token;
    RULE_ENGINE_ITEM    rule_type;
    uint8_t             *pRule_engine_items;

}ModifyRuleInput_INFO;

/**
 * ModifyRule Output context
 **/
typedef struct _byte_align4 ModifyRuleOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}ModifyRuleOutput_INFO;

/**
 * DeleteRule Input context
 **/
typedef struct _byte_align4 DeleteRuleInput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         reserved;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;
    CMD_STRING_T    rule_token;

}DeleteRuleInput_INFO;

/**
 * DeleteRule Output context
 **/
typedef struct _byte_align4 DeleteRuleOutput_INFO_T
{
    uint16_t        cmd_code;
    uint8_t         return_code;
    CMD_STRING_T    user_name;
    CMD_STRING_T    password;

}DeleteRuleOutput_INFO;
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
