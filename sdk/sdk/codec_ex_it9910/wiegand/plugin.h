#ifndef __PLUGIN_H__
#define __PLUGIN_H__

#if CFG_CHIP_FAMILY == 9910
#  include "codecs.h"
extern struct _codec_api *ci;
//#  undef  taskYIELD
//#  define PalSleep              ci->sleep
//#  define PalGetClock           ci->PalGetClock
//#  define PalGetDuration        ci->PalGetDuration
//#  define PalGetSysClock        ci->PalGetSysClock
//#  define PalWFileOpen          ci->PalWFileOpen  
//#  define PalFileWrite          ci->PalFileWrite
//#  define PalFileClose          ci->PalFileClose
//#  define f_enterFS             ci->f_enterFS
//#  define PalHeapAlloc          ci->PalHeapAlloc
//#  define PalHeapFree           ci->PalHeapFree
//#  define taskYIELD             ci->yield
//#  define deactiveDAC           ci->deactiveDAC
//#  define deactiveADC           ci->deactiveADC
//#  define pauseDAC              ci->pauseDAC
//#  define pauseADC              ci->pauseADC
//#  define initDAC               ci->initDAC
//#  define initADC               ci->initADC
//#  define initCODEC             ci->initCODEC
//#  define or32_invalidate_cache ci->flush_dcache
#  define dc_invalidate         ci->flush_all_dcache
//#  define printf                ci->printf
//#  define SPEEX_GetBufInfo      codec_info
#else
#  define PalSleep              
#  define PalGetClock         
#  define PalGetDuration    
#  define PalGetSysClock    
#  define PalWFileOpen       
#  define PalFileWrite          
#  define PalFileClose          
#  define f_enterFS             
#  define PalHeapAlloc        
#  define PalHeapFree         
#  define taskYIELD             
#  define deactiveDAC         
#  define deactiveADC         
#  define pauseDAC             
#  define pauseADC             
#  define initDAC            
#  define initADC            
#  define initCODEC        
#  define or32_invalidate_cache 
//#  define dc_invalidate
#  define printf               
#  define SPEEX_GetBufInfo codec_info
#endif
#endif // __PLUGIN_H__
