INCLUDE_DIRECTORIES(
    ${SMEDIA_INC_DIRS}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/aec
    )

SET(AMR_AEC_SRCS
    aec/aec.c
)

SET(AMR_COMMON_SRCS
    common/basicop2.c
    common/copy.c
    common/count.c
    common/gc_pred.c
    common/gmed_n.c
    common/int_lpc.c
    common/inv_sqrt.c
    common/log2.c
    common/lsp_az.c
    common/lsp_lsf.c
    common/n_proc.c
    common/oper_32b.c
    common/pow2.c
    common/pred_lt.c
    common/reorder.c
    common/residu.c
    common/set_zero.c
    common/sqrt_l.c
    common/syn_filt.c
    common/weight_a.c
)

SET(AMR_ENC_SRCS
    encode/autocorr.c
    encode/az_lsp.c
    encode/c2_9pf.c
    encode/c2_11pf.c
    encode/c3_14pf.c
    encode/c4_17pf.c
    encode/c8_31pf.c
    encode/c1035pf.c
    encode/calc_cor.c
    encode/calc_en.c
    encode/cbsearch.c
    encode/cl_ltp.c
    encode/cod_amr.c
    encode/convolve.c
    encode/cor_h.c
    encode/dtx_enc.c
    encode/enc_lag3.c
    encode/enc_lag6.c
    encode/e_homing.c
    encode/gain_q.c
    encode/g_adapt.c
    encode/g_code.c
    encode/g_pitch.c
    encode/hp_max.c
    encode/inter_36.c
    encode/lag_wind.c
    encode/levinson.c
    encode/lflg_upd.c
    encode/lpc.c
    encode/lsfwt.c
    encode/lsp.c
    encode/mac_32.c
    encode/ol_ltp.c
    encode/pitch_fr.c
    encode/pitch_ol.c
    encode/pre_big.c
    encode/pre_proc.c
    encode/prm2bits.c
    encode/p_ol_wgh.c
    encode/qgain475.c
    encode/qgain795.c
    encode/qua_gain.c
    encode/q_gain_c.c
    encode/q_gain_p.c
    encode/q_plsf.c
    encode/q_plsf_3.c
    encode/q_plsf_5.c
    encode/r_fft.c
    encode/s10_8pf.c
    encode/set_sign.c
    encode/sid_sync.c
    encode/spreproc.c
    encode/spstproc.c
    encode/sp_enc.c
    encode/strfunc.c
    encode/ton_stab.c
    encode/vad1.c
    encode/vad2.c
    encode/vadname.c
)

SET(AMR_DEC_SRCS
    decode/a_refl.c
    decode/agc.c
    decode/b_cn_cod.c
    decode/bgnscd.c
    decode/bits2prm.c
    decode/c_g_aver.c
    decode/d1035pf.c
    decode/d2_11pf.c
    decode/d2_9pf.c
    decode/d3_14pf.c
    decode/d4_17pf.c
    decode/d8_31pf.c
    decode/d_gain_c.c
    decode/d_gain_p.c
    decode/d_homing.c
    decode/d_plsf.c
    decode/d_plsf_3.c
    decode/d_plsf_5.c
    decode/dec_amr.c
    decode/dec_gain.c
    decode/dec_lag3.c
    decode/dec_lag6.c
    decode/dtx_dec.c
    decode/ec_gains.c
    decode/ex_ctrl.c
    decode/int_lsf.c
    decode/lsp_avg.c
    decode/ph_disp.c
    decode/post_pro.c
    decode/preemph.c
    decode/pstfilt.c
    decode/sp_dec.c
)

SET(AMR_MAIN_SRCS
    codec.c
)

SET(AMR_SRCS
    ${AMR_COMMON_SRCS}
    ${AMR_DEC_SRCS}
    #${AMR_ENC_SRCS}
    #${AMR_AEC_SRCS}
    ${AMR_MAIN_SRCS}
)

IF(ENABLE_CODECS_PLUGIN)
    ADD_EXECUTABLE(amrcodec.plugin
        ${AMR_SRCS}
    )
 if (DEFINED CFG_ROM_COMPRESS)
    set(args ${args} -z -b 512K)
    if (CFG_CHIP_FAMILY EQUAL 9070)
        ADD_CUSTOM_COMMAND(
            TARGET amrcodec.plugin
            POST_BUILD
            COMMAND ${CMAKE_OBJCOPY}
                    #-O binary ${CMAKE_CURRENT_BINARY_DIR}/amrcodec.plugin ${LIBRARY_OUTPUT_PATH}/amr.codecs
                    -O binary -j .risc1_header -j .text -j .data ${CMAKE_CURRENT_BINARY_DIR}/amrcodec.plugin ${LIBRARY_OUTPUT_PATH}/amr.rawcodecs
            COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/amr.rawcodecs
            COMMAND mkrom
            ARGS ${args} ${PROJECT_SOURCE_DIR}/sdk/target/ram/${CFG_RAM_INIT_SCRIPT} ${LIBRARY_OUTPUT_PATH}/amr.rawcodecs ${LIBRARY_OUTPUT_PATH}/amr.codecs
        )
     else ()
        ADD_CUSTOM_COMMAND(
            TARGET amrcodec.plugin
            POST_BUILD
            COMMAND ${CMAKE_OBJCOPY}
                    #-O binary ${CMAKE_CURRENT_BINARY_DIR}/amrcodec.plugin ${LIBRARY_OUTPUT_PATH}/amr.codecs
                    -O binary ${CMAKE_CURRENT_BINARY_DIR}/amrcodec.plugin ${LIBRARY_OUTPUT_PATH}/amr.rawcodecs
            COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/amr.rawcodecs
            COMMAND mkrom
            ARGS ${args} ${PROJECT_SOURCE_DIR}/sdk/target/ram/${CFG_RAM_INIT_SCRIPT} ${LIBRARY_OUTPUT_PATH}/amr.rawcodecs ${LIBRARY_OUTPUT_PATH}/amr.codecs
        )
     endif ()
  else (DEFINED CFG_ROM_COMPRESS)
    if (CFG_CHIP_FAMILY EQUAL 9070)
        ADD_CUSTOM_COMMAND(
            TARGET amrcodec.plugin
            POST_BUILD
            COMMAND ${CMAKE_OBJCOPY}
                    -O binary ${CMAKE_CURRENT_BINARY_DIR}/amrcodec.plugin ${LIBRARY_OUTPUT_PATH}/amr.codecs
            COMMAND ${DATACONV} -x ${LIBRARY_OUTPUT_PATH}/amr.codecs
        )
    endif ()
  endif (DEFINED CFG_ROM_COMPRESS)    
ELSE(ENABLE_CODECS_PLUGIN)
    ADD_LIBRARY(amrcodec STATIC
        ${AMR_SRCS}
    )
ENDIF(ENABLE_CODECS_PLUGIN)

