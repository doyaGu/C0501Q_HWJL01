#ifndef __UIENC_TYPES_H_4WB1R60V_I89O_SM33_24JI_MCSBKPGHDMC5__
#define __UIENC_TYPES_H_4WB1R60V_I89O_SM33_24JI_MCSBKPGHDMC5__

#ifdef __cplusplus
extern "C" {
#endif


#include "uiEnc_err.h"
#include "uiEnc_defs.h"
//=============================================================================
//				  Constant Definition
//=============================================================================
typedef enum UIE_ENG_STATUS_TAG
{
    UIE_ENG_IDLE   = 0,
    UIE_ENG_BUSY,
    
}UIE_ENG_STATUS;

//=============================================================================
//				  Macro Definition
//=============================================================================
/**
 *  Debug message
 */
extern UIE_UINT32  uieMsgOnFlag;

typedef enum _UIE_MSG_TYPE
{
    UIE_MSG_TYPE_ERR          = (0x1 << 0),   
} UIE_MSG_TYPE;


#ifdef _MSC_VER // WIN32
    #ifndef trac
    #define trac(string, ...)               do{ printf(string, __VA_ARGS__); \
                                                printf("  %s [#%d]\n", __FILE__, __LINE__); \
                                            }while(0)
    #endif

    #define uie_msg(type, string, ...)      ((void)((type & uieMsgOnFlag) ? printf(string, __VA_ARGS__) : UIE_NULL))
    #define uie_msg_ex(type, string, ...)   do{ if(type & uieMsgOnFlag){ \
                                                   printf(string, __VA_ARGS__); \
                                                   printf("  %s [#%d]\n", __FILE__, __LINE__);} \
                                            }while(0)  

#else
    #ifndef trac
    #define trac(string, args...)               do{ printf(string, ## args); \
                                                    printf("  %s [#%d]\n", __FILE__, __LINE__); \
                                                }while(0)
    #endif

    #define uie_msg(type, string, args...)      ((void)((type & uieMsgOnFlag) ? printf(string, ## args) : UIE_NULL))
    #define uie_msg_ex(type, string, args...)   do{ if(type & uieMsgOnFlag){ \
                                                       printf(string, ## args); \
                                                       printf("  %s [#%d]\n", __FILE__, __LINE__);} \
                                                }while(0)    
#endif

#define uie_enable_msg(type)                (uieMsgOnFlag |= type)
#define uie_disable_msg(type)               (uieMsgOnFlag &= ~(type)) 

//=============================================================================
//				  Structure Definition
//=============================================================================
typedef struct _UIE_SRC_INFO_TAG
{
    UIE_UINT16      srcAddr_L;
    UIE_UINT16      srcAddr_H;
    
    UIE_UINT16      src_w;
    UIE_UINT16      src_h;
    UIE_UINT16      src_pitch;
    
}UIE_SRC_INFO;

typedef struct _UIE_DST_INFO_TAG
{
    UIE_UINT16      dstAddr_L;
    UIE_UINT16      dstAddr_H;
    
    UIE_UINT16      lineBytes;
    UIE_UINT16      dst_pitch;
    
}UIE_DST_INFO;

/*typedef struct _UIE_CNTXT_TAG
{
    UIE_SRC_INFO        srcInfo;
    
    UIE_DST_INFO        dstInfo;
    
    UIE_INT             runSize;
    UIE_INT             unitSize;
    UIE_INT             srcBpp;
    
    UIE_ENG_STATUS      engStatus;
    
}UIE_CNTXT;
//*/
//=============================================================================
//				  Global Data Definition
//=============================================================================


//=============================================================================
//				  Private Function Definition
//=============================================================================


//=============================================================================
//				  Public Function Definition
//=============================================================================


#ifdef __cplusplus
}
#endif

#endif

