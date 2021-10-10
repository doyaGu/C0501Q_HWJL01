#ifndef __cmd_pkt_register_H_rImMOuM9_9A4s_cCKG_6wBg_5AA3RfAOgaQR__
#define __cmd_pkt_register_H_rImMOuM9_9A4s_cCKG_6wBg_5AA3RfAOgaQR__

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include <stdbool.h>
//=============================================================================
//                Constant Definition
//=============================================================================
typedef enum CMD_PKT_ACT_STATE_T
{
    CMD_PKT_ACT_STATE_UNKNOW    = 0,
    CMD_PKT_ACT_STATE_DECODE,
    CMD_PKT_ACT_STATE_ENCODE,
}CMD_PKT_ACT_STATE;

//=============================================================================
//                Macro Definition
//=============================================================================
#define DEFINE_CMD_REGISTER_TEMPLATE(descType, id_enum_type)     \
    static descType *first_##descType##_item = 0;       \
    static descType *cur_##descType##_item = 0;         \
    static void                                         \
    _register_##descType##_item(                        \
        descType     *format){                          \
        descType **p;                                   \
        p = &first_##descType##_item;                   \
        while (*p != 0) p = &(*p)->next;                \
        *p = format;                                    \
        format->next = 0;                               \
    }                                                   \
    static descType*                                    \
    descType##_next(                                    \
        descType  *f){                                  \
        if(f) return f->next;                           \
        else  return first_##descType##_item;           \
    }                                                   \
    static descType*                                    \
    descType##_find(                                    \
        id_enum_type   cmd_code){                       \
        descType *dev = 0;                              \
        while(dev = descType##_next(dev))               \
        if( dev->cmd_code == cmd_code ) return dev;     \
        return 0;                                       \
    }                                                   \
    static descType*                                    \
    descType##_get_next(void){                          \
        descType *dev = 0;                              \
        dev = descType##_next(cur_##descType##_item);   \
        cur_##descType##_item = dev;                    \
        return dev;                                     \
    }


#define CMD_REGISTER_ITEM(type, x) { \
    extern type type##_##x##_desc; \
    _register_##type##_item(&type##_##x##_desc);}

#define CMD_PKT_FIND_DESC_ITEM(descType, id)    descType##_find(id)
#define CMD_PKT_GET_NEXT_DESC(descType)         descType##_get_next()

//---------------------
// cmd instance
#define _STR(s)     #s

#define CMD_PKT_DECODE_INSTANCE(func_name, cmd_struct, priv_func)                                                \
    static uint32_t                                                                                              \
    func_name(                                                                                                   \
        void *input_info, void *output_info, void *extraData){                                                   \
        CMD_PKT_PARSER      *pParser = (CMD_PKT_PARSER*)input_info;                                              \
        do{ if( !pParser )      break;                                                                           \
            if( !pParser->pCmd_pkt_privData ){                                                                   \
                uint32_t            cmd_ctxt_length = 0;                                                         \
                uint32_t            info_size = sizeof(cmd_struct);                                              \
                cmd_ctxt_length = _GET_DWORD(pParser->pCmd_pkt_buf);                                             \
                pParser->cmd_info_size = info_size + cmd_ctxt_length + 1;                                        \
                if( !(pParser->pCmd_pkt_privData = tscm_malloc(pParser->cmd_info_size)) ){                       \
                    tscm_msg_ex(TSCM_MSG_ERR, "err, alloc cmd info (%s) fail !!", _STR(cmd_struct));             \
                    break;                                                                                       \
                }                                                                                                \
                memset(pParser->pCmd_pkt_privData, 0x0, pParser->cmd_info_size);                                 \
                pParser->pCur_cmd_ctxt = (uint8_t*)pParser->pCmd_pkt_privData + info_size;                       \
                pParser->pEnd_cmd_ctxt = (uint8_t*)pParser->pCmd_pkt_privData + pParser->cmd_info_size;          \
            }                                                                                                    \
            memcpy(pParser->pCur_cmd_ctxt, pParser->pCmd_pkt_buf, pParser->cmd_pkt_buf_size);                    \
            pParser->pCur_cmd_ctxt += pParser->cmd_pkt_buf_size;                                                 \
            if( pParser->pCur_cmd_ctxt >= pParser->pEnd_cmd_ctxt ){                                              \
                uint8_t      *pCur_cmd_ctxt = 0;                                                                 \
                uint32_t     cmd_ctxt_length = 0;                                                                \
                uint8_t      check_sum_value = (-1);                                                             \
                pCur_cmd_ctxt   = (uint8_t*)pParser->pCmd_pkt_privData + sizeof(cmd_struct);                     \
                cmd_ctxt_length = _GET_DWORD(pCur_cmd_ctxt);                                                     \
                check_sum_value = _tscm_gen_check_sum(pCur_cmd_ctxt, cmd_ctxt_length);                           \
                if( check_sum_value != *(pCur_cmd_ctxt + cmd_ctxt_length)){                                      \
                    tscm_msg_ex(TSCM_MSG_ERR, "err, chenk_sum is NOT mapping !!");                               \
                    break;                                                                                       \
                }                                                                                                \
                pCur_cmd_ctxt += 4;                                                                              \
                priv_func(pCur_cmd_ctxt, pParser->pCmd_pkt_privData);                                            \
                pParser->bCollecteDone = true;                                                                   \
            }                                                                                                    \
        }while(0);                                                                                               \
        return 0;                                                                                                \
    }

/***************************************************
 * - template of basic packet context new
 * Field           Length(Byte)  Descriptions
 * Command Length      4           The total length of this command. It doesn't include the CheckSum.
 * Command Code        2           Code: 0x0000
 * Reserved            1
 * User Name           n           [string] User name for security.
 * Password            n           [string] Password for security.
 * CheckSum            1           =(byte[0]+...+byte[N]) MOD 256
 **/
#define CMD_PKT_BASIC_CTXT_NEW_INSTANCE(func_name, cmd_struct, cmd_code, diff_member)           \
    static uint32_t                                                                             \
    func_name(                                                                                  \
        void *input_info, void *output_info, void *extraData){                                  \
        cmd_struct      *pInfo = (cmd_struct*)input_info;                                       \
        uint32_t        cmd_length = 0;                                                         \
        uint8_t         *pCmd_ctxt_Buf = 0;                                                     \
        do{ uint8_t     *pCur = 0;                                                              \
            uint8_t     check_sum = 0;                                                          \
            cmd_length = 8 + (4+pInfo->user_name.length) + (4+pInfo->password.length);          \
            pCmd_ctxt_Buf = tscm_malloc(cmd_length);                                            \
            if( !pCmd_ctxt_Buf ){                                                               \
                tscm_msg_ex(TSCM_MSG_ERR, "err, alloc fail !!");                                \
                break;                                                                          \
            }                                                                                   \
            memset(pCmd_ctxt_Buf, 0x0, cmd_length);                                             \
            pCur = pCmd_ctxt_Buf;                                                               \
            _SET_DWORD(pCur, cmd_length - 1);                                                   \
            _SET_WORD(pCur, cmd_code);                                                          \
            _SET_BYTE(pCur, pInfo->diff_member);                                                \
            _SET_DWORD(pCur, pInfo->user_name.length);                                          \
            _SET_STRING(pCur, pInfo->user_name.pStream, pInfo->user_name.length);               \
            _SET_DWORD(pCur, pInfo->password.length);                                           \
            _SET_STRING(pCur, pInfo->password.pStream, pInfo->password.length);                 \
            check_sum = _tscm_gen_check_sum(pCmd_ctxt_Buf, cmd_length - 1);                     \
            pCur = pCmd_ctxt_Buf + cmd_length - 1;                                              \
            _SET_BYTE(pCur, check_sum);                                                         \
            if( output_info )       *((uint8_t**)output_info) = pCmd_ctxt_Buf;                  \
        }while(0);                                                                              \
        return 0;                                                                               \
    }

#define CMD_PKT_BASIC_CTXT_SUB_DEC_INSTANCE(sub_func_name, cmd_struct, diff_member)     \
    static void                                                                         \
    sub_func_name(                                                                      \
        uint8_t *pCur, void *pStruct_Info){                                             \
        cmd_struct   *pInfo = (cmd_struct*)pStruct_Info;                                \
        pInfo->cmd_code = _GET_WORD(pCur);     pCur += 2;                               \
        pInfo->diff_member = _GET_BYTE(pCur);           pCur += 1;                      \
        pInfo->user_name.length  = _GET_DWORD(pCur); pCur += 4;                         \
        pInfo->user_name.pStream = pCur;             pCur += pInfo->user_name.length;   \
        pInfo->password.length  = _GET_DWORD(pCur);  pCur += 4;                         \
        pInfo->password.pStream = pCur;              pCur += pInfo->password.length;    \
        return;                                                                         \
    }
//=============================================================================
//                Structure Definition
//=============================================================================
typedef uint32_t (*CMD_PKT_CODEC)(void *input_info, void *output_info, void *extraData);
typedef uint32_t (*CMD_CTXT_CREATE)(void *input_info, void *output_info, void *extraData);
typedef uint32_t (*CMD_CTXT_DESTROY)(bool bUser_Cmd_destroy, void *input_info, void *output_info, void *extraData);


typedef struct CMD_PKT_CODEC_DESC_T
{
    struct CMD_PKT_CODEC_DESC_T *next;
    uint32_t            cmd_code;
    CMD_PKT_CODEC       cmd_pkt_encode;
    CMD_PKT_CODEC       cmd_pkt_decode;
    void                *pPriv_info;
    CMD_CTXT_CREATE     cmd_ctxt_create;
    CMD_CTXT_DESTROY    cmd_ctxt_destroy;
}CMD_PKT_CODEC_DESC;

#define DEFINE_CMD_PKT_CODEC(cmd_code, cmd_ctxt_create, cmd_pkt_encoder, cmd_pkt_decoder, cmd_ctxt_destroy, pPriv_info)\
    CMD_PKT_CODEC_DESC CMD_PKT_CODEC_DESC_##cmd_code##_desc = {\
        0,\
        cmd_code,\
        cmd_pkt_encoder,\
        cmd_pkt_decoder,\
        pPriv_info,\
        cmd_ctxt_create,\
        cmd_ctxt_destroy,\
    }



//=============================================================================
//                Global Data Definition
//=============================================================================

//=============================================================================
//                Private Function Definition
//=============================================================================
static uint32_t
_floatTo4Byte(float fValue)
{
    return *((uint32_t*)&fValue);
}
static float
_4ByteToFloat(uint32_t value)
{
    return *((float*)&value);
}
//=============================================================================
//                Public Function Definition
//=============================================================================

#ifdef __cplusplus
}
#endif

#endif
