
#include "pes_stream_cfg.h"
#include "pes_stream_audio.h"

#if (CONFIG_PES_STREAM_OPR_AUDIO_DESC)
//=============================================================================
//				  Constant Definition
//=============================================================================

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
PES_STREAM_OPR PES_STREAM_OPR_audio_desc =
{
    "pes audio",        // char        *name;
    0,                  // struct PES_STREAM_OPR_T     *next;
    PES_STREAM_AUDIO,   // PES_STREAM_ID               id;
    0,                  // void        *privInfo;
    0,                  // uint32_t    (*init)(PES_STREAM_MBOX *pPesStreamMbox, void *extraData);
    0,                  // uint32_t    (*deinit)(PES_STREAM_MBOX *pPesStreamMbox, void *extraData);
    0,                  // uint32_t    (*proc)(PES_STREAM_MBOX *pPesStreamMbox, void *extraData);
    0,                  // uint32_t    (*get_info)(PES_STREAM_MBOX *pPesStreamMbox, void *extraData);
};
#else /* #if (CONFIG_PES_STREAM_OPR_AUDIO_DESC) */
PES_STREAM_OPR PES_STREAM_OPR_audio_desc =
{
    "pes audio",        // char        *name;
    0,                  // struct PES_STREAM_OPR_T     *next;
    PES_STREAM_AUDIO,   // PES_STREAM_ID               id;
    0,                  // void        *privInfo;
};
#endif
