#ifndef __UIENC_ERR_H_835AS1Z3_TJ87_F4H1_9R5L_1P7BNAY7L3WN__
#define __UIENC_ERR_H_835AS1Z3_TJ87_F4H1_9R5L_1P7BNAY7L3WN__

#ifdef __cplusplus
extern "C" {
#endif


//=============================================================================
//				  Constant Definition
//=============================================================================
/**
 *  uiEnc error code
 */
#define  UIE_ERR_BASE            0x11000000

typedef enum _UIE_ERR_TAG
{
    UIE_ERR_OK                  = 0,
    UIE_ERR_ALLOCATE_FAIL       = (UIE_ERR_BASE | 0x00000001),
    UIE_ERR_NULL_POINTER        = (UIE_ERR_BASE | 0x00000002),
    UIE_ERR_INVALID_PARAMETER   = (UIE_ERR_BASE | 0x00000003),
    UIE_ERR_TIME_OUT            = (UIE_ERR_BASE | 0x00000004),

}UIE_ERR;

//=============================================================================
//				  Macro Definition
//=============================================================================


//=============================================================================
//				  Structure Definition
//=============================================================================


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

