

#include "global_freq_info.h"
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
static const uint32_t pFreq7MVHF_Italy[7] = {177500, 186000, 194500, 203500, 212500, 219500, 226500};

/**
 * detail definition for each frequency band
 **/
static const TS_FREQ_BAND vhf_6M_FreqBand = {TS_FREQ_BAND_6M_VHF, 177000, 213000, 6000,  7, 0}; // FREQ_6M_VHF
static const TS_FREQ_BAND uhf_6M_FreqBand = {TS_FREQ_BAND_6M_UHF, 473000, 887000, 6000, 70, 0}; // FREQ_6M_UHF
static const TS_FREQ_BAND vhf_7M_FreqBand = {TS_FREQ_BAND_7M_VHF, 177500, 226500, 7000,  8, 0}; // FREQ_7M_VHF
static const TS_FREQ_BAND uhf_7M_FreqBand = {TS_FREQ_BAND_7M_UHF, 529500, 816500, 7000, 42, 0}; // FREQ_7M_UHF
static const TS_FREQ_BAND vhf_8M_FreqBand = {TS_FREQ_BAND_8M_VHF, 178750, 218750, 8000,  6, 0}; // FREQ_8M_VHF
static const TS_FREQ_BAND uhf_8M_FreqBand = {TS_FREQ_BAND_8M_UHF, 474000, 858000, 8000, 49, 0}; // FREQ_8M_UHF

// Italy specific frequency
static const TS_FREQ_BAND vhf_7M_FreqBand_Italy = {TS_FREQ_BAND_7M_VHF_ITALY, 177500, 226500, 7000, 7, pFreq7MVHF_Italy}; 

static const TS_FREQ_BAND *g_ts_FreqBand[] =
{
    &vhf_6M_FreqBand,
    &uhf_6M_FreqBand, 
    &vhf_7M_FreqBand,
    &uhf_7M_FreqBand,
    &vhf_8M_FreqBand,
    &uhf_8M_FreqBand,
    0,
};

static const TS_FREQ_BAND   *g_ts_6M_bandwdith[]  = {&vhf_6M_FreqBand, &uhf_6M_FreqBand, 0};
static const TS_FREQ_BAND   *g_ts_7M_bandwdith[]  = {&vhf_7M_FreqBand, &uhf_7M_FreqBand, 0};
static const TS_FREQ_BAND   *g_ts_8M_bandwdith[]  = {&vhf_8M_FreqBand, &uhf_8M_FreqBand, 0};
static const TS_FREQ_BAND   *g_ts_78M_bandwdith[] = {&vhf_7M_FreqBand, &uhf_8M_FreqBand, 0};
static const TS_FREQ_BAND   *g_ts_italy_78M_bandwdith[] = {&vhf_7M_FreqBand_Italy, &uhf_8M_FreqBand, 0};


static const TS_COUNTRY_INFO g_ts_country_info[] =
{
    {"Taiwan",    TS_COUNTRY_TW,  DTV_DVB_SPEC_ID_DEFAULT,    g_ts_6M_bandwdith},
    {"UK",        TS_COUNTRY_UK,  DTV_DVB_SPEC_ID_DBOOK  ,    g_ts_8M_bandwdith},
    {"Australia", TS_COUNTRY_AU,  DTV_DVB_SPEC_ID_DEFAULT,    g_ts_7M_bandwdith},
    {"China",     TS_COUNTRY_CN,  DTV_DVB_SPEC_ID_DEFAULT,    g_ts_8M_bandwdith},
    {"France",    TS_COUNTRY_FR,  DTV_DVB_SPEC_ID_EBOOK  ,    g_ts_8M_bandwdith},
    {"Hungary",   TS_COUNTRY_HU,  DTV_DVB_SPEC_ID_EBOOK  ,    g_ts_8M_bandwdith},
    {"Poland",    TS_COUNTRY_PL,  DTV_DVB_SPEC_ID_EBOOK  ,    g_ts_8M_bandwdith},
    {"Portugal",  TS_COUNTRY_PT,  DTV_DVB_SPEC_ID_EBOOK  ,    g_ts_8M_bandwdith},
    {"Russian",   TS_COUNTRY_RU,  DTV_DVB_SPEC_ID_DEFAULT,    g_ts_8M_bandwdith},
    {"Spain",     TS_COUNTRY_ES,  DTV_DVB_SPEC_ID_EBOOK  ,    g_ts_8M_bandwdith},
    {"Germany",   TS_COUNTRY_DE,  DTV_DVB_SPEC_ID_EBOOK  ,    g_ts_78M_bandwdith},
    {"Greece",    TS_COUNTRY_GR,  DTV_DVB_SPEC_ID_EBOOK  ,    g_ts_78M_bandwdith},
    {"Italy",     TS_COUNTRY_IT,  DTV_DVB_SPEC_ID_DGTVI  ,    g_ts_italy_78M_bandwdith},
    
    {"", TS_COUNTRY_UNKNOW, 0, 0},
}; 

//=============================================================================
//				  Private Function Definition
//=============================================================================


//=============================================================================
//				  Public Function Definition
//=============================================================================
TS_FREQ_BAND*
tsFreq_Get_FreqBand(
    TS_FREQ_BAND_TYPE   type,
    TS_FREQ_BAND        **pUserFreqBand_Db)
{
    uint32_t        i = 0;
    TS_FREQ_BAND    *tmpFreqBand = 0;
    TS_FREQ_BAND    **pFreqBand_Db = 0;
    
    pFreqBand_Db = ((uint32_t)pUserFreqBand_Db) ? pUserFreqBand_Db : (TS_FREQ_BAND**)g_ts_FreqBand;
    
    while( pFreqBand_Db[i] )
    {
        if( pFreqBand_Db[i]->type == type )
        {
            tmpFreqBand = pFreqBand_Db[i];
            break;
        }
        
        i++;
    }
    
    return tmpFreqBand;
}


TS_COUNTRY_INFO*
tsFreq_Get_CountryInfo(
    TS_COUNTRY_CODE  tsCountryId)
{
    uint32_t            i = 0;
    TS_COUNTRY_INFO     *tmpCoutryInfo = 0;
    
    while( g_ts_country_info[i].tsCountryId != TS_COUNTRY_UNKNOW )
    {
        if( g_ts_country_info[i].tsCountryId == tsCountryId )
        {
            tmpCoutryInfo = (TS_COUNTRY_INFO*)&g_ts_country_info[i];
            break;
        }
        
        i++;
    }
    
    return tmpCoutryInfo;
}



