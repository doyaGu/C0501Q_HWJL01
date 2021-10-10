#ifndef __cmd_data_type_H_GhsOop45_8z2K_48Ct_PI3c_e9tQE4QMYM0b__
#define __cmd_data_type_H_GhsOop45_8z2K_48Ct_PI3c_e9tQE4QMYM0b__

#ifdef __cplusplus
extern "C" {
#endif


#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
//=============================================================================
//                Constant Definition
//=============================================================================

//=============================================================================
//                Macro Definition
//=============================================================================
#ifndef _byte_align4
    #if (_MSC_VER)
        #define _byte_align4                        __declspec(align(4))
    #else
        #define _byte_align4                        __attribute__ ((aligned(4)))
    #endif
#endif

//=============================================================================
//                Structure Definition
//=============================================================================
/**
 * command data type: string
 **/
typedef struct _CMD_STRING_T
{
    uint32_t        length;
    uint8_t         *pStream;
}CMD_STRING_T;


/**
 * command data type: string list
 **/
typedef struct _CMD_STRING_LIST_T
{
    uint32_t            list_size;
    CMD_STRING_T        *pCmd_string;
}CMD_STRING_LIST_T;

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
