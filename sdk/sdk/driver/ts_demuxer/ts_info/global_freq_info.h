#ifndef __GLOBLE_FREQ_INFO_H_60W075GR_OOYJ_9OJV_W900_FNAUVGYCK76W__
#define __GLOBLE_FREQ_INFO_H_60W075GR_OOYJ_9OJV_W900_FNAUVGYCK76W__

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
//=============================================================================
//				  Constant Definition
//=============================================================================
typedef enum TS_FREQ_BAND_TYPE_TAG
{
    TS_FREQ_BAND_UNKNOW     = 0, 
    TS_FREQ_BAND_6M_VHF,
    TS_FREQ_BAND_6M_UHF,
    TS_FREQ_BAND_7M_VHF,
    TS_FREQ_BAND_7M_UHF,
    TS_FREQ_BAND_8M_VHF,
    TS_FREQ_BAND_8M_UHF,
    TS_FREQ_BAND_7M_VHF_ITALY,
    
}TS_FREQ_BAND_TYPE;

/**
 * ISO 3166 country code
 **/
typedef enum TS_COUNTRY_CODE_TAG
{
    TS_COUNTRY_UNKNOW   = 0,
    TS_COUNTRY_AU       = 036, // Australia
    TS_COUNTRY_AT       = 040, // Austria
    TS_COUNTRY_CN       = 156, // China
    TS_COUNTRY_FR       = 250, // France
    TS_COUNTRY_DE       = 276, // Germany
    TS_COUNTRY_GR       = 300, // Greece
    TS_COUNTRY_HU       = 348, // Hungary
    TS_COUNTRY_IT       = 380, // Italy
    TS_COUNTRY_NL       = 524, // Netherland
    TS_COUNTRY_PL       = 616, // Poland
    TS_COUNTRY_PT       = 620, // Portugal
    TS_COUNTRY_RU       = 643, // Russian 
    TS_COUNTRY_ES       = 724, // Spain
    TS_COUNTRY_TW       = 158, // Taiwan
    TS_COUNTRY_UK       = 826, // United kingdom
    TS_COUNTRY_DK       = 208, // Denmark 
    TS_COUNTRY_SE       = 752, // Sweden
    TS_COUNTRY_FI       = 246, // Finland

} TS_COUNTRY_CODE;


typedef enum DTV_DVB_SPEC_ID_TAG
{
    DTV_DVB_SPEC_ID_DEFAULT = 0,
    DTV_DVB_SPEC_ID_DBOOK,
    DTV_DVB_SPEC_ID_NORDIG,
    DTV_DVB_SPEC_ID_EBOOK,
    DTV_DVB_SPEC_ID_DGTVI,
    
} DTV_DVB_SPEC_ID;
//=============================================================================
//				  Macro Definition
//=============================================================================

//=============================================================================
//				  Structure Definition
//=============================================================================
typedef struct TS_FREQ_BAND_TAG
{
    TS_FREQ_BAND_TYPE   type;

    uint32_t        startFreq;
    uint32_t        endFreq;
    uint32_t        bandwidth;
    uint32_t        totalCount;
    
    const uint32_t  *pSpecificFreq;
    
} TS_FREQ_BAND;

typedef struct TS_COUNTRY_INFO_TAG
{
    char                  *name;
    TS_COUNTRY_CODE       tsCountryId;
    DTV_DVB_SPEC_ID       specId;
    const TS_FREQ_BAND*   const *freq_band_DB; // pointer array
    
}TS_COUNTRY_INFO;
//=============================================================================
//				  Global Data Definition
//=============================================================================


//=============================================================================
//				  Private Function Definition
//=============================================================================


//=============================================================================
//				  Public Function Definition
//=============================================================================
TS_FREQ_BAND*
tsFreq_Get_FreqBand(
    TS_FREQ_BAND_TYPE   type,
    TS_FREQ_BAND        **pUserFreqBand_Db);


TS_COUNTRY_INFO*
tsFreq_Get_CountryInfo(
    TS_COUNTRY_CODE  tsCountryId);
    

#ifdef __cplusplus
}
#endif

#endif

