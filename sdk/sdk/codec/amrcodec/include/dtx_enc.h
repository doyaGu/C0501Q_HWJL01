/*
********************************************************************************
*
*      GSM AMR-NB speech codec   R98   Version 7.6.0   December 12, 2001
*                                R99   Version 3.3.0
*                                REL-4 Version 4.1.0
*
********************************************************************************
*
*      File             : dtx_enc.h
*      Purpose          : DTX mode computation of SID parameters
*
********************************************************************************
*/
#ifndef dtx_enc_h
#define dtx_enc_h "$Id $"

/*
********************************************************************************
*                         INCLUDE FILES
********************************************************************************
*/
#include "typedef.h"
#include "cnst.h"
#include "q_plsf.h"
#include "gc_pred.h"
#include "mode.h"

/*
********************************************************************************
*                         LOCAL VARIABLES AND TABLES
********************************************************************************
*/
#define DTX_HIST_SIZE 8
#define DTX_ELAPSED_FRAMES_THRESH (24 + 7 -1)
#define DTX_HANG_CONST 7             /* yields eight frames of SP HANGOVER  */

/*
********************************************************************************
*                         DEFINITION OF DATA TYPES
********************************************************************************
*/
typedef struct {
   Word16 lsp_hist[M * DTX_HIST_SIZE];
   Word16 log_en_hist[DTX_HIST_SIZE];
   Word16 hist_ptr;
   Word16 log_en_index;
   Word16 init_lsf_vq_index;
   Word16 lsp_index[3];

   /* DTX handler stuff */
   Word16 dtxHangoverCount;
   Word16 decAnaElapsedCount;

} dtx_encState;
/*
********************************************************************************
*                         DECLARATION OF PROTOTYPES
********************************************************************************
*/
/*
**************************************************************************
*  Function    : dtx_enc_init
*  Purpose     : Allocates memory and initializes state variables
*  Description : Stores pointer to filter status struct in *st. This
*                pointer has to be passed to dtx_enc in each call.
*  Returns     : 0 on success
*
**************************************************************************
*/
int dtx_enc_init (dtx_encState **st);

/*
**************************************************************************
*
*  Function    : dtx_enc_reset
*  Purpose     : Resets state memory
*  Returns     : 0 on success
*
**************************************************************************
*/
int dtx_enc_reset (dtx_encState *st);

/*
**************************************************************************
*
*  Function    : dtx_enc_exit
*  Purpose     : The memory used for state memory is freed
*  Description : Stores NULL in *st
*
**************************************************************************
*/
//void dtx_enc_exit (dtx_encState **st);

/*
**************************************************************************
*
*  Function    : dtx_enc
*  Purpose     :
*  Description :
*
**************************************************************************
*/
int dtx_enc(dtx_encState *st,        /* i/o : State struct                    */
            Word16 computeSidFlag,   /* i   : compute SID                     */
        Q_plsfState *qSt,        /* i/o : Qunatizer state struct          */
            gc_predState* predState, /* i/o : State struct                    */
        Word16 **anap            /* o   : analysis parameters             */
        );

/*
**************************************************************************
*
*  Function    : dtx_buffer
*  Purpose     : handles the DTX buffer
*
**************************************************************************
*/
int dtx_buffer(dtx_encState *st,   /* i/o : State struct                    */
           Word16 lsp_new[],   /* i   : LSP vector                      */
           Word16 speech[]     /* i   : speech samples                  */
           );

/*
**************************************************************************
*
*  Function    : tx_dtx_handler
*  Purpose     : adds extra speech hangover to analyze speech on the decoding side.
*  Description : returns 1 when a new SID analysis may be made
*                otherwise it adds the appropriate hangover after a sequence
*                with out updates of SID parameters .
*
**************************************************************************
*/
Word16 tx_dtx_handler(dtx_encState *st,       /* i/o : State struct          */
                      Word16 vadFlag,         /* i   : vad control variable  */
                      enum Mode *usedMode     /* o   : mode changed or not   */
                      );

#endif
