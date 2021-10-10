#ifndef _DEFINES_H
#define _DEFINES_H

#define NO_HW_ENGINE
#if !defined(WIN32) && !defined(__CYGWIN__) && !defined(__OR32__)
#  define __OR32__
#endif

#if defined(WIN32) || defined(__CYGWIN__) || defined(NO_HW_ENGINE)
//AMR Encoder Define TAG
/* autocorr.c */
//#define AUTOCORR_ENG1
  #define AUTOCORR_PUREC1

//#define AUTOCORR_ENG2
//#define AUTOCORR_OPRISC2
  #define AUTOCORR_PUREC2

//#define AUTOCORR_ENG3
//#define AUTOCORR_OPRISC3
  #define AUTOCORR_PUREC3

/* az_lsp.c */
//#define AZLSP_OPRISC
  #define AZLSP_PUREC

/* c2_9pf.c */
//#define C29PF_SET
//#define C29PF_ENG

//#define CODE2I409BIT_ENG
  #define CODE2I409BIT_PUREC

/* calc_cor.c */
//#define CALCCOR_ENG
  #define CALCCOR_PUREC

/* calc_en.c */
//#define CALCFILTENERGIES_ENG1
//#define CALCFILTENERGIES_OPRISC1
  #define CALCFILTENERGIES_PUREC1

//#define CALCFILTENERGIES_ENG2
//#define CALCFILTENERGIES_OPRISC2
  #define CALCFILTENERGIES_PUREC2

//#define CALCFILTENERGIES_ENG3
//#define CALCFILTENERGIES_OPRISC3
  #define CALCFILTENERGIES_PUREC3

//#define CALCFILTENERGIES_ENG795475    // for 795 mode
  #define CALCFILTENERGIES_PUREC795475

//#define CALCUNFILT_ENG1
  #define CALCUNFILT_PUREC1   // for 795 mode

//#define CALCUNFILT_ENG2
  #define CALCUNFILT_PUREC2   // for 795 mode

//#define CALCUNFILT_ENG3
  #define CALCUNFILT_PUREC3   // for 795 mode

//#define CALCTARGETENERGY_ENG    // for 475 mode
  #define CALCTARGETENERGY_PUREC

/* cl_ltp.c */
//#define CLLTP_ENG
  #define CLLTP_PUREC      // for new optimum

/* convolve.c */
//#define CONVOLVE_ENG
//#define CONVOLVE_OPRISC
  #define CONVOLVE_PUREC

/* cor_h.c */
//#define CORHX2_ENG
//#define CORHX2_OPRISC
  #define CORHX2_PUREC

//#define CORH_ENG1
//#define CORH_OPRISC1
  #define CORH_PUREC1

//#define CORH_ENG2
  #define CORH_PUREC2

//#define CORH_OPRISC3 //1021
  #define CORH_PUREC3

/* dtx_enc.c */
//#define DTXBUFFER_ENG
  #define DTXBUFFER_PUREC

/* g_code.c */
//#define GCODE_ENG1
//#define GCODE_OPRISC1
  #define GCODE_PUREC1

//#define GCODE_ENG2
//#define GCODE_OPRISC2
  #define GCODE_PUREC2

/* gc_pred */
//#define GCPRED_ENG
  #define GCPRED_PUREC

/* g_pitch */
//#define GPITCH_ENG1
  #define GPITCH_PUREC1

//#define GPITCH_ENG2
  #define GPITCH_PUREC2

//#define GPITCH_ENG3
  #define GPITCH_PUREC3

//#define GPITCH_ENG4
  #define GPITCH_PUREC4

//#define GPITCH_ENG5
  #define GPITCH_PUREC5

  #define GPITCH_INLINE

/* pitch_fr */
//#define NORMCORR_ENG1
//#define NORMCORR_OPRISC1
  #define NORMCORR_PUREC1

//#define NORMCORR_ENG2
//#define NORMCORR_OPRISC2
  #define NORMCORR_PUREC2

//#define NORMCORR_ENG3
//#define NORMCORR_OPRISC3
  #define NORMCORR_PUREC3

//#define NORMCORR_ENG4
  #define NORMCORR_PUREC4

/* pitch_ol.c */
//#define LAGMAX_ENG
//#define LAGMAX_OPRISC
  #define LAGMAX_PUREC

//#define PITCHOL_ENG
//#define PITCHOL_OPRISC
  #define PITCHOL_PUREC

//#define PITCHOL_ENGCPY1
  #define PITCHOL_PUREC1

/* p_ol_wgh.c */ //for mode 102
//#define POLWGHLAGMAX_ENG
//#define POLWGHLAGMAX_OPRISC
  #define POLWGHLAGMAX_PUREC

//#define PITCHOLWGH_ENG
  #define PITCHOLWGH_PUREC

/* residu.c */
//#define RESIDU_ENG
  #define RESIDU_PUREC

/* set_sign.c */     // for mode 102, 122
//#define SETSIGN12K2_ENG1
  #define SETSIGN12K2_PUREC1

//#define SETSIGN12K2_ENG2
  #define SETSIGN12K2_PUREC2

/* syn_filt.c */
//#define SYNFILT_OPRISC // new optimum

/* vad1.c */
//#define VAD1_ENG
  #define VAD1_PUREC

/* memory copy */
//#define CODAMR_CPY
//#define QPLSF3_CPY
//#define LSP_CPY
//#define PITCHOL_CPY
//#define SPREPRO_CPY
//#define SYNFILT_CPY

/* memory set */
//#define CODAMR_SET
#else // defined(WIN32) || defined(__CYGWIN__) || defined(NO_HW_ENGINE)

//AMR Encoder Define TAG
/* autocorr.c */
  #define AUTOCORR_ENG1
//#define AUTOCORR_PUREC1

//#define AUTOCORR_ENG2
//#define AUTOCORR_OPRISC2
  #define AUTOCORR_PUREC2

//#define AUTOCORR_ENG3
  #define AUTOCORR_OPRISC3
//#define AUTOCORR_PUREC3

/* az_lsp.c */
  #define AZLSP_OPRISC
//#define AZLSP_PUREC

/* c2_9pf.c */
//#define C29PF_SET
//#define C29PF_ENG

//#define CODE2I409BIT_ENG
  #define CODE2I409BIT_PUREC

/* calc_cor.c */
  #define CALCCOR_ENG
//#define CALCCOR_PUREC

/* calc_en.c */
  #define CALCFILTENERGIES_ENG1
//#define CALCFILTENERGIES_OPRISC1
//#define CALCFILTENERGIES_PUREC1

  #define CALCFILTENERGIES_ENG2
//#define CALCFILTENERGIES_OPRISC2
//#define CALCFILTENERGIES_PUREC2

  #define CALCFILTENERGIES_ENG3
//#define CALCFILTENERGIES_OPRISC3
//#define CALCFILTENERGIES_PUREC3

  #define CALCFILTENERGIES_ENG795475    // for 795 mode
//#define CALCFILTENERGIES_PUREC795475

  #define CALCUNFILT_ENG1
//#define CALCUNFILT_PUREC1   // for 795 mode

  #define CALCUNFILT_ENG2
//#define CALCUNFILT_PUREC2   // for 795 mode

  #define CALCUNFILT_ENG3
//#define CALCUNFILT_PUREC3   // for 795 mode

  #define CALCTARGETENERGY_ENG    // for 475 mode
//#define CALCTARGETENERGY_PUREC

/* cl_ltp.c */
  #define CLLTP_ENG
//#define CLLTP_PUREC      // for new optimum

/* convolve.c */
  #define CONVOLVE_ENG
//#define CONVOLVE_OPRISC
//#define CONVOLVE_PUREC

/* cor_h.c */
  #define CORHX2_ENG
//#define CORHX2_OPRISC
//#define CORHX2_PUREC

  #define CORH_ENG1
//#define CORH_OPRISC1
//#define CORH_PUREC1

  #define CORH_ENG2
//#define CORH_PUREC2

//#define CORH_OPRISC3 //1021
  #define CORH_PUREC3

/* dtx_enc.c */
  #define DTXBUFFER_ENG
//#define DTXBUFFER_PUREC

/* g_code.c */
  #define GCODE_ENG1
//#define GCODE_OPRISC1
//#define GCODE_PUREC1

  #define GCODE_ENG2
//#define GCODE_OPRISC2
//#define GCODE_PUREC2

/* gc_pred */
  #define GCPRED_ENG
//#define GCPRED_PUREC

/* g_pitch */
  #define GPITCH_ENG1
//#define GPITCH_PUREC1

  #define GPITCH_ENG2
//#define GPITCH_PUREC2

  #define GPITCH_ENG3
//#define GPITCH_PUREC3

  #define GPITCH_ENG4
//#define GPITCH_PUREC4

  #define GPITCH_ENG5
//#define GPITCH_PUREC5

  #define GPITCH_INLINE

/* pitch_fr */
  #define NORMCORR_ENG1
//#define NORMCORR_OPRISC1
//#define NORMCORR_PUREC1

  #define NORMCORR_ENG2
//#define NORMCORR_OPRISC2
//#define NORMCORR_PUREC2

  #define NORMCORR_ENG3
//#define NORMCORR_OPRISC3
//#define NORMCORR_PUREC3

  #define NORMCORR_ENG4
//#define NORMCORR_PUREC4

/* pitch_ol.c */
  #define LAGMAX_ENG
//#define LAGMAX_OPRISC
//#define LAGMAX_PUREC

  #define PITCHOL_ENG
//#define PITCHOL_OPRISC
//#define PITCHOL_PUREC

//#define PITCHOL_ENGCPY1
  #define PITCHOL_PUREC1

/* p_ol_wgh.c */ //for mode 102
  #define POLWGHLAGMAX_ENG
//#define POLWGHLAGMAX_OPRISC
//#define POLWGHLAGMAX_PUREC

  #define PITCHOLWGH_ENG
//#define PITCHOLWGH_PUREC

/* residu.c */
  #define RESIDU_ENG
//#define RESIDU_PUREC

/* set_sign.c */     // for mode 102, 122
  #define SETSIGN12K2_ENG1
//#define SETSIGN12K2_PUREC1

  #define SETSIGN12K2_ENG2
//#define SETSIGN12K2_PUREC2

/* syn_filt.c */
  #define SYNFILT_OPRISC // new optimum

/* vad1.c */
  #define VAD1_ENG
//#define VAD1_PUREC

/* memory copy */
  #define CODAMR_CPY
//#define QPLSF3_CPY
  #define LSP_CPY
  #define PITCHOL_CPY
  #define SPREPRO_CPY
  #define SYNFILT_CPY

/* memory set */
  #define CODAMR_SET
#endif  // defined(WIN32) || defined(__CYGWIN__)

#if defined(__OR32__)
 // #include "proj_defs.h"
/*
 * Function Division
 */
  #define ENI2S
  #define DRIVERIN
  #define DRIVEROUT
//#define FILEIN
//#define FILEOUT
//#define TEST5FRAME
//#define CHECKSUM
//#define ORTIMER
  #define AMR_RESET_DECODED_BYTE
  
/* Optimize
 */
  #define OPRISCASM
  #define OPRISCENG
//#define OPRISCENGMEM

//#define NO_ROUNDING
#endif // defined(__OR32__)

/*
 * AMR Codec OPTION
 */
#define MMS_IO
//#define MAGICWORD

/*
 * AEC OPTION
 */
// Setting of original AEC
#define NLMS_LEN  (80*1)          /* NLMS filter length in taps */

// Self Add Settings
//#define NOISEDET
//#define OUTGAINCONTROL
//#define SPEEDFRAG
  #define DEPEAKMIPS

/* Self added Constant */
// Speed Fragmant Settings
#define FRAG_LEN 20
#define FRAG_TIMES 2

#if !defined(MM365)
// The maclc still have problem on MM370. Comment by Kuoping, 2006.11.29.
//#define HAVE_MACLC
#endif

#if defined(__OR32__)
//#  include "or32.h"
#endif

#endif // _DEFINES_H

