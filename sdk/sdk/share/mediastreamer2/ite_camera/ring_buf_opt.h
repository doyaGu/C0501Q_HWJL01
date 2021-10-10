#ifndef __rb_opt_template_H_22e8RcxR_Jasx_5FnD_5wSB_03NtoMPaJ7aa__
#define __rb_opt_template_H_22e8RcxR_Jasx_5FnD_5wSB_03NtoMPaJ7aa__

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include <string.h>
//=============================================================================
//                Constant Definition
//=============================================================================
#define RB_OPT_SUPPORT_NUM      3
//=============================================================================
//                Macro Definition
//=============================================================================

//=============================================================================
//                Structure Definition
//=============================================================================
/**
 *  ring buffer operator, never stop when write data
 **/
typedef struct RB_OPT_T
{
    uint32_t    r_ptr[RB_OPT_SUPPORT_NUM];
    uint32_t    w_ptr;
    uint32_t    start_ptr;
    uint32_t    end_ptr;
}RB_OPT;
//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================

static int
rb_opt_init(
    RB_OPT      *pRbOpt,
    uint32_t    start_ptr,
    uint32_t    buf_size)
{
    if( !pRbOpt )   return -1;
    pRbOpt->end_ptr = (sizeof(pRbOpt->r_ptr) / sizeof(pRbOpt->r_ptr[0]));
    while(pRbOpt->end_ptr--)
        pRbOpt->r_ptr[pRbOpt->end_ptr] = start_ptr;
    pRbOpt->w_ptr     = start_ptr;
    pRbOpt->start_ptr = start_ptr;
    pRbOpt->end_ptr   = start_ptr + buf_size;
    return 0;
}

static int
rb_opt_update_w(
    RB_OPT      *pRbOpt,
    uint32_t    pData,
    uint32_t    data_size)
{
    uint32_t    w_ptr = pRbOpt->w_ptr;
    if( (w_ptr + data_size) > pRbOpt->end_ptr )
    {
        int     remain = pRbOpt->end_ptr - w_ptr;

        if( remain > 0 )    memcpy((void*)w_ptr, (void*)pData, remain);
        // else                printf("err ! w_ptr > end_prt\n");
        pData += remain;
        memcpy((void*)pRbOpt->start_ptr, (void*)pData, data_size - remain);
        w_ptr = pRbOpt->start_ptr + data_size - remain;
    }
    else
    {
        memcpy((void*)w_ptr, (void*)pData, data_size);
        w_ptr += data_size;
    }
    pRbOpt->w_ptr = w_ptr;
    return 0;
}

static int
rb_opt_update_r(
    RB_OPT      *pRbOpt,
    uint32_t    index,
    uint32_t    *pData,
    uint32_t    *pData_size)
{
    uint32_t    request_size = 0;
    uint32_t    w_ptr = pRbOpt->w_ptr;

    if( (*pData_size) )
    {
        // fit request size
        request_size = (*pData_size);
        if( w_ptr < pRbOpt->r_ptr[index] )
        {
            uint32_t    valid_size = pRbOpt->end_ptr - pRbOpt->r_ptr[index];

            if( valid_size < request_size )
            {
                *pData_size = valid_size;
                *pData      = (*pData_size) ? pRbOpt->r_ptr[index] : 0;

                pRbOpt->r_ptr[index] = pRbOpt->start_ptr;
            }
            else
            {
                *pData_size = request_size;
                *pData      = (*pData_size) ? pRbOpt->r_ptr[index] : 0;
                pRbOpt->r_ptr[index] += (*pData_size);
            }
        }
        else
        {
            uint32_t    valid_size = w_ptr - pRbOpt->r_ptr[index];

            if( valid_size < request_size )
                *pData_size = valid_size;
            else
                *pData_size = request_size;

            *pData = (*pData_size) ? pRbOpt->r_ptr[index] : 0;

            pRbOpt->r_ptr[index] += (*pData_size);
        }
    }
    else
    {
        // align writing pointer
        if( w_ptr < pRbOpt->r_ptr[index] )
        {
            *pData_size = pRbOpt->end_ptr - pRbOpt->r_ptr[index];
            *pData      = pRbOpt->r_ptr[index];

            pRbOpt->r_ptr[index] = pRbOpt->start_ptr;
        }
        else
        {
            *pData_size = w_ptr - pRbOpt->r_ptr[index];
            *pData      = (*pData_size) ? pRbOpt->r_ptr[index] : 0;
            pRbOpt->r_ptr[index] += (*pData_size);
        }
    }
    return 0;
}
//=============================================================================
//                Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif
