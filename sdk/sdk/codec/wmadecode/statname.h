
/**
 * @file statname.h
 *  name mangling macros for static linking.
 */

#ifndef __STATNAME_H__
#  define __STATNAME_H__

/* define STAT_PREFIX to a unique name for static linking
 * all the C functions and global variables will be mangled by the preprocessor
 *   e.g. void DCT4(...) becomes void xmwa_DCT4(...)
 */
#  define STAT_PREFIX          xwma

#  define STATCC1(x,y,z)       STATCC2(x,y,z)
#  define STATCC2(x,y,z)       x##y##z

#  if 0
#    define STATNAME(func,id)  STATCC1(__link_, id, __)
#  elif defined(STAT_PREFIX)
#    define STATNAME(func,id)  STATCC1(STAT_PREFIX, _, func)
#  elif defined(__FREERTOS__) && !defined(ENABLE_CODECS_PLUGIN)
#    define STATNAME(func,id)  STATCC1(__link_, id, __)
#  else
#    define STATNAME(func,id)  func
#  endif // defined(STAT_PREFIX)

/* variables */
#  define wmadec                                  STATNAME(wmadec                                , 001)
#  define vlcbuf4                                 STATNAME(vlcbuf4                               , 002)
#  define vlcbuf3                                 STATNAME(vlcbuf3                               , 003)
#  define vlcbuf2                                 STATNAME(vlcbuf2                               , 004)
#  define vlcbuf1                                 STATNAME(vlcbuf1                               , 005)
#  define wma_critical_freqs                      STATNAME(wma_critical_freqs                    , 006)
#  define scale_huffcodes                         STATNAME(scale_huffcodes                       , 007)
#  define scale_huffbits                          STATNAME(scale_huffbits                        , 008)
#  define scale                                   STATNAME(scale                                 , 009)
#  define pow_a_table                             STATNAME(pow_a_table                           , 010)
#  define pow_10_to_yover16                       STATNAME(pow_10_to_yover16                     , 011)
#  define pow1_table                              STATNAME(pow1_table                            , 012)
#  define pow0_table                              STATNAME(pow0_table                            , 013)
#  define lsp_pow_e_table                         STATNAME(lsp_pow_e_table                       , 014)
#  define lsp_codebook                            STATNAME(lsp_codebook                          , 015)
#  define levels5                                 STATNAME(levels5                               , 016)
#  define levels4                                 STATNAME(levels4                               , 017)
#  define levels3                                 STATNAME(levels3                               , 018)
#  define levels2                                 STATNAME(levels2                               , 019)
#  define levels1                                 STATNAME(levels1                               , 020)
#  define levels0                                 STATNAME(levels0                               , 021)
#  define hgain_huffcodes                         STATNAME(hgain_huffcodes                       , 022)
#  define hgain_huffbits                          STATNAME(hgain_huffbits                        , 023)
#  define exponent_band_44100                     STATNAME(exponent_band_44100                   , 024)
#  define exponent_band_32000                     STATNAME(exponent_band_32000                   , 025)
#  define exponent_band_22050                     STATNAME(exponent_band_22050                   , 026)
#  define coef_vlcs                               STATNAME(coef_vlcs                             , 027)
#  define coef5_huffcodes                         STATNAME(coef5_huffcodes                       , 028)
#  define coef5_huffbits                          STATNAME(coef5_huffbits                        , 029)
#  define coef4_huffcodes                         STATNAME(coef4_huffcodes                       , 030)
#  define coef4_huffbits                          STATNAME(coef4_huffbits                        , 031)
#  define coef3_huffcodes                         STATNAME(coef3_huffcodes                       , 032)
#  define coef3_huffbits                          STATNAME(coef3_huffbits                        , 033)
#  define coef2_huffcodes                         STATNAME(coef2_huffcodes                       , 034)
#  define coef2_huffbits                          STATNAME(coef2_huffbits                        , 035)
#  define coef1_huffcodes                         STATNAME(coef1_huffcodes                       , 036)
#  define coef1_huffbits                          STATNAME(coef1_huffbits                        , 037)
#  define coef0_huffcodes                         STATNAME(coef0_huffcodes                       , 038)
#  define coef0_huffbits                          STATNAME(coef0_huffbits                        , 039)
#  define atan_table                              STATNAME(atan_table                            , 040)
#  define asf_guid_stream_type_audio              STATNAME(asf_guid_stream_type_audio            , 041)
#  define asf_guid_stream_properties              STATNAME(asf_guid_stream_properties            , 042)
#  define asf_guid_file_properties                STATNAME(asf_guid_file_properties              , 043)
#  define asf_guid_extended_content_encryption    STATNAME(asf_guid_extended_content_encryption  , 044)
#  define asf_guid_extended_content_description   STATNAME(asf_guid_extended_content_description , 045)
#  define asf_guid_data                           STATNAME(asf_guid_data                         , 046)
#  define asf_guid_content_encryption             STATNAME(asf_guid_content_encryption           , 047)
#  define asf_guid_content_description            STATNAME(asf_guid_content_description          , 048)
#  define ff_log2_tab                             STATNAME(ff_log2_tab                           , 049)
#  define streamBuf                               STATNAME(streamBuf                             , 050)
#  define pcmWriteBuf                             STATNAME(pcmWriteBuf                           , 051)

/* functions */
#  define build_table                             STATNAME(build_table                           , 052)
#  define asf_utf16LEdecode                       STATNAME(asf_utf16LEdecode                     , 053)
#  define asf_read_object_header                  STATNAME(asf_read_object_header                , 054)
#  define asf_intdecode                           STATNAME(asf_intdecode                         , 055)
#  define wma_decode_superframe_init              STATNAME(wma_decode_superframe_init            , 056)
#  define wma_decode_superframe_frame             STATNAME(wma_decode_superframe_frame           , 057)
#  define wma_decode_init                         STATNAME(wma_decode_init                       , 058)
#  define updateFreqInfo                          STATNAME(updateFreqInfo                        , 059)
#  define read_filebuf                            STATNAME(read_filebuf                          , 060)
#  define mdct_init_global                        STATNAME(mdct_init_global                      , 061)
#  define init_vlc                                STATNAME(init_vlc                              , 062)
#  define get_asf_metadata                        STATNAME(get_asf_metadata                      , 063)
#  define fft_init_global                         STATNAME(fft_init_global                       , 064)
#  define fft_calc_unscaled                       STATNAME(fft_calc_unscaled                     , 065)
#  define ff_mdct_init                            STATNAME(ff_mdct_init                          , 066)
#  define ff_imdct_calc                           STATNAME(ff_imdct_calc                         , 067)
#  define advance_buffer                          STATNAME(advance_buffer                        , 068)
#  define IntTo64                                 STATNAME(IntTo64                               , 069)
#  define IntFrom64                               STATNAME(IntFrom64                             , 070)
#  define fsincos                                 STATNAME(fsincos                               , 071)
#  define fixsqrt32                               STATNAME(fixsqrt32                             , 072)
#  define Fixed32To64                             STATNAME(Fixed32To64                           , 073)
#  define Fixed32From64                           STATNAME(Fixed32From64                         , 074)
#  define fixdiv64                                STATNAME(fixdiv64                              , 075)
#  define fixdiv32                                STATNAME(fixdiv32                              , 076)
#  define wma_window                              STATNAME(wma_window                            , 077)
#  define wma_decode_block                        STATNAME(wma_decode_block                      , 078)

#endif // __STATNAME_H__

