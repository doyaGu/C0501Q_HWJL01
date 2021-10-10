#ifndef __jpg_err_H_LDHkjwKE_70Rb_3VBe_0XAS_jWt2N3Ria3ki__
#define __jpg_err_H_LDHkjwKE_70Rb_3VBe_0XAS_jWt2N3Ria3ki__

#ifdef __cplusplus
extern "C" {
#endif


//=============================================================================
//                  Constant Definition
//=============================================================================
#define JPG_ERR_BASE            0x60000000
#define JPG_ERR_OSA_BASE        0x60100000
#define JPG_ERR_PARSER_BASE     0x60200000


typedef enum _JPG_ERR_TAG
{
    // jpg common error
    JPG_ERR_OK                  = 0,
    JPG_ERR_ALLOCATE_FAIL       = (JPG_ERR_BASE | 0x00000001),
    JPG_ERR_NULL_POINTER        = (JPG_ERR_BASE | 0x00000002),
    JPG_ERR_HW_NOT_SUPPORT      = (JPG_ERR_BASE | 0x00000003),

    JPG_ERR_OPEN_FAIL           = (JPG_ERR_BASE | 0x00000004),
    JPG_ERR_ISP_HW_FAIL         = (JPG_ERR_BASE | 0x00000005),
    JPG_ERR_JPG_HW_FAIL         = (JPG_ERR_BASE | 0x00000006),
    JPG_ERR_INVALID_PARAMETER   = (JPG_ERR_BASE | 0x00000007),
    JPG_ERR_WRONG_COLOR_FORMAT  = (JPG_ERR_BASE | 0x00000008),
    JPG_ERR_DECODE_IRQ          = (JPG_ERR_BASE | 0x00000009),
    JPG_ERR_BUSY_TIMEOUT        = (JPG_ERR_BASE | 0x0000000A),
    JPG_ERR_BS_CONSUME_TIMEOUT  = (JPG_ERR_BASE | 0x0000000B),
    JPG_ERR_ENC_NOT_SUPPORT     = (JPG_ERR_BASE | 0x0000000C),
    JPG_ERR_ENC_TIMEOUT         = (JPG_ERR_BASE | 0x0000000D),
    JPG_ERR_ENC_OVER_BS_BUF     = (JPG_ERR_BASE | 0x0000000E),
    JPG_ERR_NO_IMPLEMENT        = (JPG_ERR_BASE | 0x0000000F),

    // jpg parser error
    JPG_ERR_NOT_JPEG            = (JPG_ERR_PARSER_BASE | 0x00000001),
    JPG_ERR_JPROG_STREAM        = (JPG_ERR_PARSER_BASE | 0x00000002),
    JPG_ERR_HDER_ONLY_TABLES    = (JPG_ERR_PARSER_BASE | 0x00000003),


}JPG_ERR;

//=============================================================================
//                  Macro Definition
//=============================================================================

//=============================================================================
//                  Structure Definition
//=============================================================================

//=============================================================================
//                  Global Data Definition
//=============================================================================

//=============================================================================
//                  Private Function Definition
//=============================================================================

//=============================================================================
//                  Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif
