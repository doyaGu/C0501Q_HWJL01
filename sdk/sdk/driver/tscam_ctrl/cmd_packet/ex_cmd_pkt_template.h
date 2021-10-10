
/*****
 //--------------------------
 // Template
 //--------------------------
static uint32_t
xxx_Cmd_Ctxt_New(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    //========================================================================
    // This function be used to create a cmd_ctxt node for cmd queue.
    //
    // Command format (struct xxx_INFO):
    //      Field       Length(Byte)   Descriptions
    //      Cmd Length      4           The total length of this command. It doesn¡¦t include the CheckSum.
    //      Cmd Code        2           Code: 0x0001
    //      Cmd Data        N           1~N bytes
    //      CheckSum        1           =(byte[2]+¡K+byte[8+n-1]) MOD 256
    //
    // memory = 4 + 2 + N + 1
    //
    //========================================================================

    xxx_INFO     *pInfo = (xxx_INFO*)input_info;
    uint32_t     cmd_length = 0;
    uint8_t      *pCmd_ctxt_Buf = 0;

    do{
        uint8_t     *pCur = 0;
        uint8_t     check_sum = 0;

        // internal malloc need to include check_sum size
        cmd_length = (sizeof(struct xxx_INFO) + your_payload_data);

        pCmd_ctxt_Buf = tscm_malloc(cmd_length);
        if( !pCmd_ctxt_Buf )
        {
            tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");
            break;
        }

        memset(pCmd_ctxt_Buf, 0x0, cmd_length);

        pCur = pCmd_ctxt_Buf;

        // Cmd Length
        // - real cmd_length of a cmd packet doesn't include check_sum size
        _SET_DWORD(pCur, cmd_length - 1);
        // Cmd Code
        _SET_WORD(pCur, CMD_xxxxx);

        //--------------------
        // personal Cmd Data
        _SET_BYTE(pCur, uint8_t);
        _SET_WORD(pCur, uint16_t);
        _SET_DWORD(pCur, uint32_t);
        _SET_STRING(pCur, pStream, length);
        //---------------------

        // CheckSum
        check_sum = _tscm_gen_check_sum(pCmd_ctxt_Buf, cmd_length - 1);
        _SET_BYTE(pCur, check_sum);

        // return cmd_ctxt node buffer
        if( output_info )       *((uint8_t**)output_info) = pCmd_ctxt_Buf;
    }while(0);

    return 0;
}
**/

/*****
 //--------------------------
 // Template
 //--------------------------
static uint32_t
xxx_Cmd_Pkt_Decode(
    void                *input_info,
    void                *output_info,
    void                *extraData)
{
    //========================================================================
    // This function be used to create a cmd_ctxt node for cmd queue.
    //
    // Command format (struct xxx_INFO):
    //      Field       Length(Byte)   Descriptions
    //      Cmd Length      4           The total length of this command. It doesn¡¦t include the CheckSum.
    //      Cmd Code        2           Code: 0x0001
    //      Cmd Data        N           1~N bytes
    //      CheckSum        1           =(byte[2]+¡K+byte[8+n-1]) MOD 256
    //
    // memory = 4 + 2 + N + 1
    //
    //========================================================================

    CMD_PKT_PARSER      *pParser = (CMD_PKT_PARSER*)input_info;

    do{
        if( !pParser )      break;

        if( !pParser->pCmd_pkt_privData )
        {
            uint32_t            cmd_ctxt_length = 0;
            uint32_t            info_size = sizeof(xxx_INFO);

            cmd_ctxt_length = _GET_DWORD(pParser->pCmd_pkt_buf);
            pParser->cmd_info_size = info_size + cmd_ctxt_length + 1; // include check_sum size
            if( !(pParser->pCmd_pkt_privData = tscm_malloc(pParser->cmd_info_size)) )
            {
                tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");
                break;
            }
            memset(pParser->pCmd_pkt_privData, 0x0, pParser->cmd_info_size);
            pParser->pCur_cmd_ctxt = (uint8_t*)pParser->pCmd_pkt_privData + info_size;
            pParser->pEnd_cmd_ctxt = (uint8_t*)pParser->pCmd_pkt_privData + pParser->cmd_info_size;
        }

        memcpy(pParser->pCur_cmd_ctxt, pParser->pCmd_pkt_buf, pParser->cmd_pkt_buf_size);
        pParser->pCur_cmd_ctxt += pParser->cmd_pkt_buf_size;

        if( pParser->pCur_cmd_ctxt >= pParser->pEnd_cmd_ctxt )
        {
            xxx_INFO     *pInfo = (xxx_INFO*)pParser->pCmd_pkt_privData;
            uint8_t      *pCur_cmd_ctxt = 0;
            uint32_t     cmd_ctxt_length = 0;
            uint8_t      check_sum_value = (-1);

            pCur_cmd_ctxt   = (uint8_t*)pParser->pCmd_pkt_privData + sizeof(xxx_INFO);
            cmd_ctxt_length = _GET_DWORD(pCur_cmd_ctxt);
            check_sum_value = _tscm_gen_check_sum(pCur_cmd_ctxt, cmd_ctxt_length);
            if( check_sum_value != *(pCur_cmd_ctxt + cmd_ctxt_length))
            {
                tscm_msg_ex(TSCM_MSG_ERR, "err, chenk_sum is NOT mapping !!");
                break;
            }

            pCur_cmd_ctxt += 4;
            pInfo->cmd_code = _GET_WORD(pCur_cmd_ctxt);     pCur_cmd_ctxt += 2;

            //-----------------------------------
            // personal Cmd Data
            pInfo->byte_value  = _GET_BYTE(pCur_cmd_ctxt);     pCur_cmd_ctxt += 1;
            pInfo->word_value  = _GET_WORD(pCur_cmd_ctxt);     pCur_cmd_ctxt += 2;
            pInfo->dword_value = _GET_DWORD(pCur_cmd_ctxt);    pCur_cmd_ctxt += 4;

            // CMD_STRING_T
            pInfo->cmd_string.length  = _GET_DWORD(pCur_cmd_ctxt); pCur_cmd_ctxt += 4;
            pInfo->cmd_string.pStream = pCur_cmd_ctxt;             pCur_cmd_ctxt += pInfo->cmd_string.length;
            //---------------------

            pParser->bCollecteDone = true;
        }
    }while(0);

    return 0;
}
**/

