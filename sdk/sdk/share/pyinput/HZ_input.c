// need to install VS90SP1-KB980263-x86.exe for vs2008
#pragma execution_character_set("utf-8")

#include <stdio.h>
#include <string.h>
#include "PY_code.h"
#define sizeofA 5
#define sizeofB 16
#define sizeofC 34
#define sizeofD 20
#define sizeofE 3
#define sizeofF 9
#define sizeofG 19
#define sizeofH 19
//#define sizeofI  1
#define sizeofJ 14
#define sizeofK 18
#define sizeofL 24
#define sizeofM 19
#define sizeofN 23
#define sizeofO 2
#define sizeofP 17
#define sizeofQ 14
#define sizeofR 14
#define sizeofS 34
#define sizeofT 19
//#define sizeofU 1
//#define sizeofV 1
#define sizeofW 9
#define sizeofX 14
#define sizeofY 16
#define sizeofZ 36

//"拼音输入法查询码表"

const unsigned char PY_index_a[][6] = {
    {"     "},
    {"i    "},
    {"n    "},
    {"ng   "},
    {"o    "}
};
const unsigned char PY_index_b[][6] = {
    {"a    "},
    {"ai   "},
    {"an   "},
    {"ang  "},
    {"ao   "},
    {"ei   "},
    {"en   "},
    {"eng  "},
    {"i    "},
    {"ian  "},
    {"iao  "},
    {"ie   "},
    {"in   "},
    {"ing  "},
    {"o    "},
    {"u    "}
};
const unsigned char PY_index_c[][6] = {
    {"a    "},
    {"ai   "},
    {"an   "},
    {"ang  "},
    {"ao   "},
    {"e    "},
    {"en   "},
    {"eng  "},
    {"ha   "},
    {"hai  "},
    {"han  "},
    {"hang "},
    {"hao  "},
    {"he   "},
    {"hen  "},
    {"heng "},
    {"hi   "},
    {"hong "},
    {"hou  "},
    {"hu   "},
    {"huai "},
    {"huan "},
    {"huang"},
    {"hui  "},
    {"hun  "},
    {"huo  "},
    {"i    "},
    {"ong  "},
    {"ou   "},
    {"u    "},
    {"uan  "},
    {"ui   "},
    {"un   "},
    {"uo   "}
};
const unsigned char PY_index_d[][6] = {
    {"a    "},
    {"ai   "},
    {"an   "},
    {"ang  "},
    {"ao   "},
    {"e    "},
    {"eng  "},
    {"i    "},
    {"ian  "},
    {"iao  "},
    {"ie   "},
    {"ing  "},
    {"iu   "},
    {"ong  "},
    {"ou   "},
    {"u    "},
    {"uan  "},
    {"ui   "},
    {"un   "},
    {"uo   "}
};
const unsigned char PY_index_e[][6] = {
    {"     "},
    {"n    "},
    {"r    "}
};
const unsigned char PY_index_f[][6] = {
    {"a    "},
    {"an   "},
    {"ang  "},
    {"ei   "},
    {"en   "},
    {"eng  "},
    {"o    "},
    {"ou   "},
    {"u    "}
};
const unsigned char PY_index_g[][6] = {
    {"a    "},
    {"ai   "},
    {"an   "},
    {"ang  "},
    {"ao   "},
    {"e    "},
    {"ei   "},
    {"en   "},
    {"eng  "},
    {"ong  "},
    {"ou   "},
    {"u    "},
    {"ua   "},
    {"uai  "},
    {"uan  "},
    {"uang "},
    {"ui   "},
    {"un   "},
    {"uo   "}
};
const unsigned char PY_index_h[][6] = {
    {"a    "},
    {"ai   "},
    {"an   "},
    {"ang  "},
    {"ao   "},
    {"e    "},
    {"ei   "},
    {"en   "},
    {"eng  "},
    {"ong  "},
    {"ou   "},
    {"u    "},
    {"ua   "},
    {"uai  "},
    {"uan  "},
    {"uang "},
    {"ui   "},
    {"un   "},
    {"uo   "}
};
const unsigned char PY_index_j[][6] = {
    {"i    "},
    {"ia   "},
    {"ian  "},
    {"iang "},
    {"iao  "},
    {"ie   "},
    {"in   "},
    {"ing  "},
    {"iong "},
    {"iu   "},
    {"u    "},
    {"uan  "},
    {"ue   "},
    {"un   "}
};
const unsigned char PY_index_k[][6] = {
    {"a    "},
    {"ai   "},
    {"an   "},
    {"ang  "},
    {"ao   "},
    {"e    "},
    {"en   "},
    {"eng  "},
    {"ong  "},
    {"ou   "},
    {"u    "},
    {"ua   "},
    {"uai  "},
    {"uan  "},
    {"uang "},
    {"ui   "},
    {"un   "},
    {"uo   "}
};
const unsigned char PY_index_l[][6] = {
    {"a    "},
    {"ai   "},
    {"an   "},
    {"ang  "},
    {"ao   "},
    {"e    "},
    {"ei   "},
    {"eng  "},
    {"i    "},
    {"ian  "},
    {"iang "},
    {"iao  "},
    {"ie   "},
    {"in   "},
    {"ing  "},
    {"iu   "},
    {"ong  "},
    {"ou   "},
    {"u    "},
    {"uan  "},
    {"ue   "},
    {"un   "},
    {"uo   "},
    {"v    "}
};
const unsigned char PY_index_m[][6] = {
    {"a    "},
    {"ai   "},
    {"an   "},
    {"ang  "},
    {"ao   "},
    {"e    "},
    {"ei   "},
    {"en   "},
    {"eng  "},
    {"i    "},
    {"ian  "},
    {"iao  "},
    {"ie   "},
    {"in   "},
    {"ing  "},
    {"iu   "},
    {"o    "},
    {"ou   "},
    {"u    "}
};
const unsigned char PY_index_n[][6] = {
    {"a    "},
    {"ai   "},
    {"an   "},
    {"ang  "},
    {"ao   "},
    {"e    "},
    {"ei   "},
    {"en   "},
    {"eng  "},
    {"i    "},
    {"ian  "},
    {"iang "},
    {"iao  "},
    {"ie   "},
    {"in   "},
    {"ing  "},
    {"iu   "},
    {"ong  "},
    {"u    "},
    {"uan  "},
    {"ue   "},
    {"uo   "},
    {"v    "}
};
const unsigned char PY_index_o[][6] = {
    {"     "},
    {"u    "}
};
const unsigned char PY_index_p[][6] = {
    {"a    "},
    {"ai   "},
    {"an   "},
    {"ang  "},
    {"ao   "},
    {"ei   "},
    {"en   "},
    {"eng  "},
    {"i    "},
    {"ian  "},
    {"iao  "},
    {"ie   "},
    {"in   "},
    {"ing  "},
    {"o    "},
    {"ou   "},
    {"u    "}
};
const unsigned char PY_index_q[][6] = {
    {"i    "},
    {"ia   "},
    {"ian  "},
    {"iang "},
    {"iao  "},
    {"ie   "},
    {"in   "},
    {"ing  "},
    {"iong "},
    {"iu   "},
    {"u    "},
    {"uan  "},
    {"ue   "},
    {"un   "}
};
const unsigned char PY_index_r[][6] = {
    {"an   "},
    {"ang  "},
    {"ao   "},
    {"e    "},
    {"en   "},
    {"eng  "},
    {"i    "},
    {"ong  "},
    {"ou   "},
    {"u    "},
    {"uan  "},
    {"ui   "},
    {"un   "},
    {"uo   "}
};
const unsigned char PY_index_s[][6] = {
    {"a    "},
    {"ai   "},
    {"an   "},
    {"ang  "},
    {"ao   "},
    {"e    "},
    {"en   "},
    {"eng  "},
    {"ha   "},
    {"hai  "},
    {"han  "},
    {"hang "},
    {"hao  "},
    {"he   "},
    {"hen  "},
    {"heng "},
    {"hi   "},
    {"hou  "},
    {"hu   "},
    {"hua  "},
    {"huai "},
    {"huan "},
    {"huang"},
    {"hui  "},
    {"hun  "},
    {"huo  "},
    {"i    "},
    {"ong  "},
    {"ou   "},
    {"u    "},
    {"uan  "},
    {"ui   "},
    {"un   "},
    {"uo   "}
};
const unsigned char PY_index_t[][6] = {
    {"a    "},
    {"ai   "},
    {"an   "},
    {"ang  "},
    {"ao   "},
    {"e    "},
    {"eng  "},
    {"i    "},
    {"ian  "},
    {"iao  "},
    {"ie   "},
    {"ing  "},
    {"ong  "},
    {"ou   "},
    {"u    "},
    {"uan  "},
    {"ui   "},
    {"un   "},
    {"uo   "}
};
const unsigned char PY_index_w[][6] = {
    {"a    "},
    {"ai   "},
    {"an   "},
    {"ang  "},
    {"ei   "},
    {"en   "},
    {"eng  "},
    {"o    "},
    {"u    "}
};
const unsigned char PY_index_x[][6] = {
    {"i    "},
    {"ia   "},
    {"ian  "},
    {"iang "},
    {"iao  "},
    {"ie   "},
    {"in   "},
    {"ing  "},
    {"iong "},
    {"iu   "},
    {"u    "},
    {"uan  "},
    {"ue   "},
    {"un   "}
};
const unsigned char PY_index_y[][6] = {
    {"a    "},
    {"ai   "},
    {"an   "},
    {"ang  "},
    {"ao   "},
    {"e    "},
    {"i    "},
    {"in   "},
    {"ing  "},
    {"o    "},
    {"ong  "},
    {"ou   "},
    {"u    "},
    {"uan  "},
    {"ue   "},
    {"un   "}
};
const unsigned char PY_index_z[][6] = {
    {"a    "},
    {"ai   "},
    {"an   "},
    {"ang  "},
    {"ao   "},
    {"e    "},
    {"ei   "},
    {"en   "},
    {"eng  "},
    {"ha   "},
    {"hai  "},
    {"han  "},
    {"hang "},
    {"hao  "},
    {"he   "},
    {"hen  "},
    {"heng "},
    {"hi   "},
    {"hong "},
    {"hou  "},
    {"hu   "},
    {"hua  "},
    {"huai "},
    {"huan "},
    {"huang"},
    {"hui  "},
    {"hun  "},
    {"huo  "},
    {"i    "},
    {"ong  "},
    {"ou   "},
    {"u    "},
    {"uan  "},
    {"ui   "},
    {"un   "},
    {"uo   "}
};
const unsigned char PY_index_end[][6] = {"     "};

unsigned char  py_ime(unsigned char *input_py_val, unsigned char *get_hanzi, unsigned short *hh)
{
    unsigned char  py_ime_temp[8];
    unsigned char  py_ime_temp1[8];
    unsigned char  py_ime_cmp;
    //unsigned char py_ime_cmp1;
    unsigned char  py_i;
    unsigned short g_hanzi_num;
    unsigned char  *add;
    memcpy(py_ime_temp1, input_py_val, 6); //   把输入的拼音放入缓冲区
    py_ime_cmp = input_py_val[0];          // 读取拼音的第一个字母

                                           //  输入拼音开头为 'i','u','v',的汉字不存在
    if (py_ime_cmp == 'i')
        return 0;
    if (py_ime_cmp == 'u')
        return 0;
    if (py_ime_cmp == 'v')
        return 0;
    // 首字母为 'a'

    printf("py_ime_cmp: %c\n", py_ime_cmp);

    if (py_ime_cmp == 'a')
    {
        for (py_i = 0; py_i < sizeofA; py_i++)                     // 16次
        {
            memcpy(py_ime_temp, PY_index_a[py_i], 5);              //   ai,an,ang,ao，ei,...   16个
            py_ime_cmp = memcmp(&py_ime_temp1[1], py_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (py_ime_cmp == 0)
            {
                printf("py_i==%d\n", py_i);
                switch (py_i)
                {
                case 0: add = PY_mb_a; break;
                case 1: add = PY_mb_ai; break;
                case 2: add = PY_mb_an; break;
                case 3: add = PY_mb_ang; break;
                case 4: add = PY_mb_ao; break;
                }
                g_hanzi_num = (unsigned short)(strlen(add)); //拼音a的个数
                memcpy(get_hanzi, add, g_hanzi_num);         //把找到的汉字放在缓冲区
                *hh         = g_hanzi_num / 2;
                printf("HZ NUMBER=%d\n", g_hanzi_num / 2);
                return 1;
            }
        }
        return 0;
    }

    //首字母为 'b'

    if (py_ime_cmp == 'b')
    {
        for (py_i = 0; py_i < sizeofB; py_i++)                     // 16次
        {
            memcpy(py_ime_temp, PY_index_b[py_i], 5);              //   ai,an,ang,ao，ei,...   16个
            py_ime_cmp = memcmp(&py_ime_temp1[1], py_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (py_ime_cmp == 0)
            {
                switch (py_i)
                {
                case 0: add  = PY_mb_ba; break;
                case 1: add  = PY_mb_bai; break;
                case 2: add  = PY_mb_ban; break;
                case 3: add  = PY_mb_bang; break;
                case 4: add  = PY_mb_bao; break;
                case 5: add  = PY_mb_bei; break;
                case 6: add  = PY_mb_ben; break;
                case 7: add  = PY_mb_beng; break;
                case 8: add  = PY_mb_bi; break;
                case 9: add  = PY_mb_bian; break;
                case 10: add = PY_mb_biao; break;
                case 11: add = PY_mb_bie; break;
                case 12: add = PY_mb_bin; break;
                case 13: add = PY_mb_bing; break;
                case 14: add = PY_mb_bo; break;
                case 15: add = PY_mb_bu; break;
                }
                g_hanzi_num = (unsigned short)(strlen(add)); //拼音a的个数
                memcpy(get_hanzi, add, g_hanzi_num);         //把找到的汉字放在缓冲区
                *hh         = g_hanzi_num / 2;
                return 1;
            }
        }
        return 0;
    }
    //首字母为 'c'
    if (py_ime_cmp == 'c')
    {
        for (py_i = 0; py_i < sizeofC; py_i++)                     // 16次
        {
            memcpy(py_ime_temp, PY_index_c[py_i], 5);              //   ai,an,ang,ao，ei,...   16个
            py_ime_cmp = memcmp(&py_ime_temp1[1], py_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (py_ime_cmp == 0)
            {
                switch (py_i)
                {
                case 0: add  = PY_mb_ca; break;
                case 1: add  = PY_mb_cai; break;
                case 2: add  = PY_mb_can; break;
                case 3: add  = PY_mb_cang; break;
                case 4: add  = PY_mb_cao; break;
                case 5: add  = PY_mb_ce; break;
                case 6: add  = PY_mb_cen; break;
                case 7: add  = PY_mb_ceng; break;
                case 8: add  = PY_mb_cha; break;
                case 9: add  = PY_mb_chai; break;
                case 10: add = PY_mb_chan; break;
                case 11: add = PY_mb_chang; break;
                case 12: add = PY_mb_chao; break;
                case 13: add = PY_mb_che; break;
                case 14: add = PY_mb_chen; break;
                case 15: add = PY_mb_cheng; break;
                case 16: add = PY_mb_chi; break;
                case 17: add = PY_mb_chong; break;
                case 18: add = PY_mb_chou; break;
                case 19: add = PY_mb_chu; break;
                case 20: add = PY_mb_chuai; break;
                case 21: add = PY_mb_chuan; break;
                case 22: add = PY_mb_chuang; break;
                case 23: add = PY_mb_chui; break;
                case 24: add = PY_mb_chun; break;
                case 25: add = PY_mb_chuo; break;
                case 26: add = PY_mb_ci; break;
                case 27: add = PY_mb_cong; break;
                case 28: add = PY_mb_cou; break;
                case 29: add = PY_mb_cu; break;
                case 30: add = PY_mb_cuan; break;
                case 31: add = PY_mb_cui; break;
                case 32: add = PY_mb_cun; break;
                case 33: add = PY_mb_cuo; break;
                }
                g_hanzi_num = (unsigned short)strlen(add); //   拼音  a 的个数
                memcpy(get_hanzi, add, g_hanzi_num);       //  把找到的汉字放在缓冲区
                *hh         = g_hanzi_num / 2;
                return 1;
            }
        }
        return 0;
    }

    //首字母为 'd'
    if (py_ime_cmp == 'd')
    {
        for (py_i = 0; py_i < sizeofD; py_i++)                     // 16次
        {
            memcpy(py_ime_temp, PY_index_d[py_i], 5);              //   ai,an,ang,ao，ei,...   16个
            py_ime_cmp = memcmp(&py_ime_temp1[1], py_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (py_ime_cmp == 0)
            {
                switch (py_i)
                {
                case 0: add  = PY_mb_da; break;
                case 1: add  = PY_mb_dai; break;
                case 2: add  = PY_mb_dan; break;
                case 3: add  = PY_mb_dang; break;
                case 4: add  = PY_mb_dao; break;
                case 5: add  = PY_mb_de; break;
                case 6: add  = PY_mb_deng; break;
                case 7: add  = PY_mb_di; break;
                case 8: add  = PY_mb_dian; break;
                case 9: add  = PY_mb_diao; break;
                case 10: add = PY_mb_die; break;
                case 11: add = PY_mb_ding; break;
                case 12: add = PY_mb_diu; break;
                case 13: add = PY_mb_dong; break;
                case 14: add = PY_mb_dou; break;
                case 15: add = PY_mb_du; break;
                case 16: add = PY_mb_duan; break;
                case 17: add = PY_mb_dui; break;
                case 18: add = PY_mb_dun; break;
                case 19: add = PY_mb_duo; break;
                }
                g_hanzi_num = (unsigned short)(strlen(add)); //拼音a的个数
                memcpy(get_hanzi, add, g_hanzi_num);         //把找到的汉字放在缓冲区
                *hh         = g_hanzi_num / 2;
                return 1;
            }
        }
        return 0;
    }

    //首字母为 'e'
    if (py_ime_cmp == 'e')
    {
        for (py_i = 0; py_i < sizeofE; py_i++)                     // 16次
        {
            memcpy(py_ime_temp, PY_index_e[py_i], 5);              //   ai,an,ang,ao，ei,...   16个
            py_ime_cmp = memcmp(&py_ime_temp1[1], py_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (py_ime_cmp == 0)
            {
                switch (py_i)
                {
                case 0: add = PY_mb_e; break;
                case 1: add = PY_mb_en; break;
                case 2: add = PY_mb_er; break;
                }
                g_hanzi_num = (unsigned short)(strlen(add)); //拼音a的个数
                memcpy(get_hanzi, add, g_hanzi_num);         //把找到的汉字放在缓冲区
                *hh         = g_hanzi_num / 2;
                return 1;
            }
        }
        return 0;
    }

    //首字母为 'f'
    if (py_ime_cmp == 'f')
    {
        for (py_i = 0; py_i < sizeofF; py_i++)                     // 16次
        {
            memcpy(py_ime_temp, PY_index_f[py_i], 5);              //   ai,an,ang,ao，ei,...   16个
            py_ime_cmp = memcmp(&py_ime_temp1[1], py_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (py_ime_cmp == 0)
            {
                printf("py_i==%d\n", py_i);
                switch (py_i)
                {
                case 0: add = PY_mb_fa; break;
                case 1: add = PY_mb_fan; break;
                case 2: add = PY_mb_fang; break;
                case 3: add = PY_mb_fei; break;
                case 4: add = PY_mb_fen; break;
                case 5: add = PY_mb_feng; break;
                case 6: add = PY_mb_fo; break;
                case 7: add = PY_mb_fou; break;
                case 8: add = PY_mb_fu; break;
                }
                g_hanzi_num = (unsigned short)strlen(add); //   拼音  a 的个数
                memcpy(get_hanzi, add, g_hanzi_num);       //  把找到的汉字放在缓冲区
                *hh         = g_hanzi_num / 2;
                return 1;
            }
        }
        return 0;
    }

    //首字母为 'g'
    if (py_ime_cmp == 'g')
    {
        for (py_i = 0; py_i < sizeofG; py_i++)                     // 16次
        {
            memcpy(py_ime_temp, PY_index_g[py_i], 5);              //   ai,an,ang,ao，ei,...   16个
            py_ime_cmp = memcmp(&py_ime_temp1[1], py_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (py_ime_cmp == 0)
            {
                printf("py_i==%d\n", py_i);
                switch (py_i)
                {
                case 0: add  = PY_mb_ga; break;
                case 1: add  = PY_mb_gai; break;
                case 2: add  = PY_mb_gan; break;
                case 3: add  = PY_mb_gang; break;
                case 4: add  = PY_mb_gao; break;
                case 5: add  = PY_mb_ge; break;
                case 6: add  = PY_mb_gei; break;
                case 7: add  = PY_mb_gen; break;
                case 8: add  = PY_mb_geng; break;
                case 9: add  = PY_mb_gong; break;
                case 10: add = PY_mb_gou; break;
                case 11: add = PY_mb_gu; break;
                case 12: add = PY_mb_gua; break;
                case 13: add = PY_mb_guai; break;
                case 14: add = PY_mb_guan; break;
                case 15: add = PY_mb_guang; break;
                case 16: add = PY_mb_gui; break;
                case 17: add = PY_mb_gun; break;
                case 18: add = PY_mb_guo; break;
                }
                g_hanzi_num = (unsigned short)strlen(add); //   拼音  a 的个数
                memcpy(get_hanzi, add, g_hanzi_num);       //  把找到的汉字放在缓冲区
                *hh         = g_hanzi_num / 2;
                return 1;
            }
        }
        return 0;
    }

    //首字母为 'h'
    if (py_ime_cmp == 'h')
    {
        for (py_i = 0; py_i < sizeofH; py_i++)                     // 16次
        {
            memcpy(py_ime_temp, PY_index_h[py_i], 5);              //   ai,an,ang,ao，ei,...   16个
            py_ime_cmp = memcmp(&py_ime_temp1[1], py_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (py_ime_cmp == 0)
            {
                switch (py_i)
                {
                case 0: add  = PY_mb_ha; break;
                case 1: add  = PY_mb_hai; break;
                case 2: add  = PY_mb_han; break;
                case 3: add  = PY_mb_hang; break;
                case 4: add  = PY_mb_hao; break;
                case 5: add  = PY_mb_he; break;
                case 6: add  = PY_mb_hei; break;
                case 7: add  = PY_mb_hen; break;
                case 8: add  = PY_mb_heng; break;
                case 9: add  = PY_mb_hong; break;
                case 10: add = PY_mb_hou; break;
                case 11: add = PY_mb_hu; break;
                case 12: add = PY_mb_hua; break;
                case 13: add = PY_mb_huai; break;
                case 14: add = PY_mb_huan; break;
                case 15: add = PY_mb_huang; break;
                case 16: add = PY_mb_hui; break;
                case 17: add = PY_mb_hun; break;
                case 18: add = PY_mb_huo; break;
                }
                g_hanzi_num = (unsigned short)strlen(add); //   拼音  a 的个数
                memcpy(get_hanzi, add, g_hanzi_num);       //  把找到的汉字放在缓冲区
                *hh         = g_hanzi_num / 2;
                return 1;
            }
        }
        return 0;
    }

    //首字母为 'j'
    if (py_ime_cmp == 'j')
    {
        for (py_i = 0; py_i < sizeofJ; py_i++)                     // 16次
        {
            memcpy(py_ime_temp, PY_index_j[py_i], 5);              //   ai,an,ang,ao，ei,...   16个
            py_ime_cmp = memcmp(&py_ime_temp1[1], py_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (py_ime_cmp == 0)
            {
                switch (py_i)
                {
                case 0: add  = PY_mb_ji; break;
                case 1: add  = PY_mb_jia; break;
                case 2: add  = PY_mb_jian; break;
                case 3: add  = PY_mb_jiang; break;
                case 4: add  = PY_mb_jiao; break;
                case 5: add  = PY_mb_jie; break;
                case 6: add  = PY_mb_jin; break;
                case 7: add  = PY_mb_jing; break;
                case 8: add  = PY_mb_jiong; break;
                case 9: add  = PY_mb_jiu; break;
                case 10: add = PY_mb_ju; break;
                case 11: add = PY_mb_juan; break;
                case 12: add = PY_mb_jue; break;
                case 13: add = PY_mb_jun; break;
                }
                g_hanzi_num = (unsigned short)strlen(add); //   拼音  a 的个数
                memcpy(get_hanzi, add, g_hanzi_num);       //  把找到的汉字放在缓冲区
                *hh         = g_hanzi_num / 2;
                return 1;
            }
        }
        return 0;
    }

    //首字母为 'k'
    if (py_ime_cmp == 'k')
    {
        for (py_i = 0; py_i < sizeofK; py_i++)                     // 16次
        {
            memcpy(py_ime_temp, PY_index_k[py_i], 5);              //   ai,an,ang,ao，ei,...   16个
            py_ime_cmp = memcmp(&py_ime_temp1[1], py_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (py_ime_cmp == 0)
            {
                switch (py_i)
                {
                case 0: add  = PY_mb_ka; break;
                case 1: add  = PY_mb_kai; break;
                case 2: add  = PY_mb_kan; break;
                case 3: add  = PY_mb_kang; break;
                case 4: add  = PY_mb_kao; break;
                case 5: add  = PY_mb_ke; break;
                case 6: add  = PY_mb_ken; break;
                case 7: add  = PY_mb_keng; break;
                case 8: add  = PY_mb_kong; break;
                case 9: add  = PY_mb_kou; break;
                case 10: add = PY_mb_ku; break;
                case 11: add = PY_mb_kua; break;
                case 12: add = PY_mb_kuai; break;
                case 13: add = PY_mb_kuan; break;
                case 14: add = PY_mb_kuang; break;
                case 15: add = PY_mb_kui; break;
                case 16: add = PY_mb_kun; break;
                case 17: add = PY_mb_kuo; break;
                }
                g_hanzi_num = (unsigned short)strlen(add); //   拼音  a 的个数
                memcpy(get_hanzi, add, g_hanzi_num);       //  把找到的汉字放在缓冲区
                *hh         = g_hanzi_num / 2;
                return 1;
            }
        }
        return 0;
    }

    //首字母为 'l'
    if (py_ime_cmp == 'l')
    {
        for (py_i = 0; py_i < sizeofL; py_i++)                     // 16次
        {
            memcpy(py_ime_temp, PY_index_l[py_i], 5);              //   ai,an,ang,ao，ei,...   16个
            py_ime_cmp = memcmp(&py_ime_temp1[1], py_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (py_ime_cmp == 0)
            {
                switch (py_i)
                {
                case 0: add  = PY_mb_la; break;
                case 1: add  = PY_mb_lai; break;
                case 2: add  = PY_mb_lan; break;
                case 3: add  = PY_mb_lang; break;
                case 4: add  = PY_mb_lao; break;
                case 5: add  = PY_mb_le; break;
                case 6: add  = PY_mb_lei; break;
                case 7: add  = PY_mb_leng; break;
                case 8: add  = PY_mb_li; break;
                case 9: add  = PY_mb_lian; break;
                case 10: add = PY_mb_liang; break;
                case 11: add = PY_mb_liao; break;
                case 12: add = PY_mb_lie; break;
                case 13: add = PY_mb_lin; break;
                case 14: add = PY_mb_ling; break;
                case 15: add = PY_mb_liu; break;
                case 16: add = PY_mb_long; break;
                case 17: add = PY_mb_lou; break;
                case 18: add = PY_mb_lu; break;
                case 19: add = PY_mb_luan; break;
                case 20: add = PY_mb_lue; break;
                case 21: add = PY_mb_lun; break;
                case 22: add = PY_mb_luo; break;
                case 23: add = PY_mb_lv; break;
                }
                g_hanzi_num = (unsigned short)strlen(add); //   拼音  a 的个数
                memcpy(get_hanzi, add, g_hanzi_num);       //  把找到的汉字放在缓冲区
                *hh         = g_hanzi_num / 2;
                return 1;
            }
        }
        return 0;
    }

    //首字母为 'm'
    if (py_ime_cmp == 'm')
    {
        for (py_i = 0; py_i < sizeofM; py_i++)                     // 16次
        {
            memcpy(py_ime_temp, PY_index_m[py_i], 5);              //   ai,an,ang,ao，ei,...   16个
            py_ime_cmp = memcmp(&py_ime_temp1[1], py_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (py_ime_cmp == 0)
            {
                switch (py_i)
                {
                case 0: add  = PY_mb_ma; break;
                case 1: add  = PY_mb_mai; break;
                case 2: add  = PY_mb_man; break;
                case 3: add  = PY_mb_mang; break;
                case 4: add  = PY_mb_mao; break;
                case 5: add  = PY_mb_me; break;
                case 6: add  = PY_mb_mei; break;
                case 7: add  = PY_mb_men; break;
                case 8: add  = PY_mb_meng; break;
                case 9: add  = PY_mb_mi; break;
                case 10: add = PY_mb_mian; break;
                case 11: add = PY_mb_miao; break;
                case 12: add = PY_mb_mie; break;
                case 13: add = PY_mb_min; break;
                case 14: add = PY_mb_ming; break;
                case 15: add = PY_mb_miu; break;
                case 16: add = PY_mb_mo; break;
                case 17: add = PY_mb_mou; break;
                case 18: add = PY_mb_mu; break;
                }
                g_hanzi_num = (unsigned short)strlen(add); //   拼音  a 的个数
                memcpy(get_hanzi, add, g_hanzi_num);       //  把找到的汉字放在缓冲区
                *hh         = g_hanzi_num / 2;
                return 1;
            }
        }
        return 0;
    }

    //首字母为 'n'
    if (py_ime_cmp == 'n')
    {
        for (py_i = 0; py_i < sizeofN; py_i++)                     // 16次
        {
            memcpy(py_ime_temp, PY_index_n[py_i], 5);              //   ai,an,ang,ao，ei,...   16个
            py_ime_cmp = memcmp(&py_ime_temp1[1], py_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (py_ime_cmp == 0)
            {
                switch (py_i)
                {
                case 0: add  = PY_mb_na; break;
                case 1: add  = PY_mb_nai; break;
                case 2: add  = PY_mb_nan; break;
                case 3: add  = PY_mb_nang; break;
                case 4: add  = PY_mb_nao; break;
                case 5: add  = PY_mb_ne; break;
                case 6: add  = PY_mb_nei; break;
                case 7: add  = PY_mb_nen; break;
                case 8: add  = PY_mb_neng; break;
                case 9: add  = PY_mb_ni; break;
                case 10: add = PY_mb_nian; break;
                case 11: add = PY_mb_niang; break;
                case 12: add = PY_mb_niao; break;
                case 13: add = PY_mb_nie; break;
                case 14: add = PY_mb_nin; break;
                case 15: add = PY_mb_ning; break;
                case 16: add = PY_mb_niu; break;
                case 17: add = PY_mb_nong; break;
                case 18: add = PY_mb_nu; break;
                case 19: add = PY_mb_nuan; break;
                case 20: add = PY_mb_nue; break;
                case 21: add = PY_mb_nuo; break;
                case 22: add = PY_mb_nv; break;
                }
                g_hanzi_num = (unsigned short)strlen(add); //   拼音  a 的个数
                memcpy(get_hanzi, add, g_hanzi_num);       //  把找到的汉字放在缓冲区
                *hh         = g_hanzi_num / 2;
                return 1;
            }
        }
        return 0;
    }

    //首字母为 'o'
    if (py_ime_cmp == 'o')
    {
        for (py_i = 0; py_i < sizeofO; py_i++)                     // 16次
        {
            memcpy(py_ime_temp, PY_index_o[py_i], 5);              //   ai,an,ang,ao，ei,...   16个
            py_ime_cmp = memcmp(&py_ime_temp1[1], py_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (py_ime_cmp == 0)
            {
                switch (py_i)
                {
                case 0: add = PY_mb_o; break;
                case 1: add = PY_mb_ou; break;
                }
                g_hanzi_num = (unsigned short)strlen(add); //   拼音  a 的个数
                memcpy(get_hanzi, add, g_hanzi_num);       //  把找到的汉字放在缓冲区
                *hh         = g_hanzi_num / 2;
                return 1;
            }
        }
        return 0;
    }

    //首字母为 'p'
    if (py_ime_cmp == 'p')
    {
        for (py_i = 0; py_i < sizeofP; py_i++)                     // 16次
        {
            memcpy(py_ime_temp, PY_index_p[py_i], 5);              //   ai,an,ang,ao，ei,...   16个
            py_ime_cmp = memcmp(&py_ime_temp1[1], py_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (py_ime_cmp == 0)
            {
                switch (py_i)
                {
                case 0: add  = PY_mb_pa; break;
                case 1: add  = PY_mb_pai; break;
                case 2: add  = PY_mb_pan; break;
                case 3: add  = PY_mb_pang; break;
                case 4: add  = PY_mb_pao; break;
                case 5: add  = PY_mb_pei; break;
                case 6: add  = PY_mb_pen; break;
                case 7: add  = PY_mb_peng; break;
                case 8: add  = PY_mb_pi; break;
                case 9: add  = PY_mb_pian; break;
                case 10: add = PY_mb_piao; break;
                case 11: add = PY_mb_pie; break;
                case 12: add = PY_mb_pin; break;
                case 13: add = PY_mb_ping; break;
                case 14: add = PY_mb_po; break;
                case 15: add = PY_mb_pou; break;
                case 16: add = PY_mb_pu; break;
                }
                g_hanzi_num = (unsigned short)strlen(add); //   拼音  a 的个数
                memcpy(get_hanzi, add, g_hanzi_num);       //  把找到的汉字放在缓冲区
                *hh         = g_hanzi_num / 2;
                return 1;
            }
        }
        return 0;
    }

    //首字母为 'q'
    if (py_ime_cmp == 'q')
    {
        for (py_i = 0; py_i < sizeofQ; py_i++)                     // 16次
        {
            memcpy(py_ime_temp, PY_index_q[py_i], 5);              //   ai,an,ang,ao，ei,...   16个
            py_ime_cmp = memcmp(&py_ime_temp1[1], py_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (py_ime_cmp == 0)
            {
                switch (py_i)
                {
                case 0: add  = PY_mb_qi; break;
                case 1: add  = PY_mb_qia; break;
                case 2: add  = PY_mb_qian; break;
                case 3: add  = PY_mb_qiang; break;
                case 4: add  = PY_mb_qiao; break;
                case 5: add  = PY_mb_qie; break;
                case 6: add  = PY_mb_qin; break;
                case 7: add  = PY_mb_qing; break;
                case 8: add  = PY_mb_qiong; break;
                case 9: add  = PY_mb_qiu; break;
                case 10: add = PY_mb_qu; break;
                case 11: add = PY_mb_quan; break;
                case 12: add = PY_mb_que; break;
                case 13: add = PY_mb_qun; break;
                }
                g_hanzi_num = (unsigned short)strlen(add); //   拼音  a 的个数
                memcpy(get_hanzi, add, g_hanzi_num);       //  把找到的汉字放在缓冲区
                *hh         = g_hanzi_num / 2;
                return 1;
            }
        }
        return 0;
    }

    //首字母为 'r'
    if (py_ime_cmp == 'r')
    {
        for (py_i = 0; py_i < sizeofR; py_i++)                     // 16次
        {
            memcpy(py_ime_temp, PY_index_r[py_i], 5);              //   ai,an,ang,ao，ei,...   16个
            py_ime_cmp = memcmp(&py_ime_temp1[1], py_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (py_ime_cmp == 0)
            {
                switch (py_i)
                {
                case 0: add  = PY_mb_ran; break;
                case 1: add  = PY_mb_rang; break;
                case 2: add  = PY_mb_rao; break;
                case 3: add  = PY_mb_re; break;
                case 4: add  = PY_mb_ren; break;
                case 5: add  = PY_mb_reng; break;
                case 6: add  = PY_mb_ri; break;
                case 7: add  = PY_mb_rong; break;
                case 8: add  = PY_mb_rou; break;
                case 9: add  = PY_mb_ru; break;
                case 10: add = PY_mb_ruan; break;
                case 11: add = PY_mb_rui; break;
                case 12: add = PY_mb_run; break;
                case 13: add = PY_mb_ruo; break;
                }
                g_hanzi_num = (unsigned short)strlen(add); //   拼音  a 的个数
                memcpy(get_hanzi, add, g_hanzi_num);       //  把找到的汉字放在缓冲区
                *hh         = g_hanzi_num / 2;
                return 1;
            }
        }
        return 0;
    }

    //首字母为 's'
    if (py_ime_cmp == 's')
    {
        for (py_i = 0; py_i < sizeofS; py_i++)                     // 16次
        {
            memcpy(py_ime_temp, PY_index_s[py_i], 5);              //   ai,an,ang,ao，ei,...   16个
            py_ime_cmp = memcmp(&py_ime_temp1[1], py_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (py_ime_cmp == 0)
            {
                switch (py_i)
                {
                case 0: add  = PY_mb_sa; break;
                case 1: add  = PY_mb_sai; break;
                case 2: add  = PY_mb_san; break;
                case 3: add  = PY_mb_sang; break;
                case 4: add  = PY_mb_sao; break;
                case 5: add  = PY_mb_se; break;
                case 6: add  = PY_mb_sen; break;
                case 7: add  = PY_mb_seng; break;
                case 8: add  = PY_mb_sha; break;
                case 9: add  = PY_mb_shai; break;
                case 10: add = PY_mb_shan; break;
                case 11: add = PY_mb_shang; break;
                case 12: add = PY_mb_shao; break;
                case 13: add = PY_mb_she; break;
                case 14: add = PY_mb_shen; break;
                case 15: add = PY_mb_sheng; break;
                case 16: add = PY_mb_shi; break;
                case 17: add = PY_mb_shou; break;
                case 18: add = PY_mb_shu; break;
                case 19: add = PY_mb_shua; break;
                case 20: add = PY_mb_shuai; break;
                case 21: add = PY_mb_shuan; break;
                case 22: add = PY_mb_shuang; break;
                case 23: add = PY_mb_shui; break;
                case 24: add = PY_mb_shun; break;
                case 25: add = PY_mb_shuo; break;
                case 26: add = PY_mb_si; break;
                case 27: add = PY_mb_song; break;
                case 28: add = PY_mb_sou; break;
                case 29: add = PY_mb_su; break;
                case 30: add = PY_mb_suan; break;
                case 31: add = PY_mb_sui; break;
                case 32: add = PY_mb_sun; break;
                case 33: add = PY_mb_suo; break;
                }
                g_hanzi_num = (unsigned short)strlen(add); //   拼音  a 的个数
                memcpy(get_hanzi, add, g_hanzi_num);       //  把找到的汉字放在缓冲区
                *hh         = g_hanzi_num / 2;
                return 1;
            }
        }
        return 0;
    }

    //首字母为 't'
    if (py_ime_cmp == 't')
    {
        for (py_i = 0; py_i < sizeofT; py_i++)                     // 16次
        {
            memcpy(py_ime_temp, PY_index_t[py_i], 5);              //   ai,an,ang,ao，ei,...   16个
            py_ime_cmp = memcmp(&py_ime_temp1[1], py_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (py_ime_cmp == 0)
            {
                switch (py_i)
                {
                case 0: add  = PY_mb_ta; break;
                case 1: add  = PY_mb_tai; break;
                case 2: add  = PY_mb_tan; break;
                case 3: add  = PY_mb_tang; break;
                case 4: add  = PY_mb_tao; break;
                case 5: add  = PY_mb_te; break;
                case 6: add  = PY_mb_teng; break;
                case 7: add  = PY_mb_ti; break;
                case 8: add  = PY_mb_tian; break;
                case 9: add  = PY_mb_tiao; break;
                case 10: add = PY_mb_tie; break;
                case 11: add = PY_mb_ting; break;
                case 12: add = PY_mb_tong; break;
                case 13: add = PY_mb_tou; break;
                case 14: add = PY_mb_tu; break;
                case 15: add = PY_mb_tuan; break;
                case 16: add = PY_mb_tui; break;
                case 17: add = PY_mb_tun; break;
                case 18: add = PY_mb_tuo; break;
                }
                g_hanzi_num = (unsigned short)strlen(add); //   拼音  a 的个数
                memcpy(get_hanzi, add, g_hanzi_num);       //  把找到的汉字放在缓冲区
                *hh         = g_hanzi_num / 2;
                return 1;
            }
        }
        return 0;
    }

    //首字母为 'w'
    if (py_ime_cmp == 'w')
    {
        for (py_i = 0; py_i < sizeofW; py_i++)                     // 16次
        {
            memcpy(py_ime_temp, PY_index_w[py_i], 5);              //   ai,an,ang,ao，ei,...   16个
            py_ime_cmp = memcmp(&py_ime_temp1[1], py_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (py_ime_cmp == 0)
            {
                switch (py_i)
                {
                case 0: add = PY_mb_wa; break;
                case 1: add = PY_mb_wai; break;
                case 2: add = PY_mb_wan; break;
                case 3: add = PY_mb_wang; break;
                case 4: add = PY_mb_wei; break;
                case 5: add = PY_mb_wen; break;
                case 6: add = PY_mb_weng; break;
                case 7: add = PY_mb_wo; break;
                case 8: add = PY_mb_wu; break;
                }
                g_hanzi_num = (unsigned short)strlen(add); //   拼音  a 的个数
                memcpy(get_hanzi, add, g_hanzi_num);       //  把找到的汉字放在缓冲区
                *hh         = g_hanzi_num / 2;
                return 1;
            }
        }
        return 0;
    }

    //首字母为 'x'
    if (py_ime_cmp == 'x')
    {
        for (py_i = 0; py_i < sizeofX; py_i++)                     // 16次
        {
            memcpy(py_ime_temp, PY_index_x[py_i], 5);              //   ai,an,ang,ao，ei,...   16个
            py_ime_cmp = memcmp(&py_ime_temp1[1], py_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (py_ime_cmp == 0)
            {
                switch (py_i)
                {
                case 0: add  = PY_mb_xi; break;
                case 1: add  = PY_mb_xia; break;
                case 2: add  = PY_mb_xian; break;
                case 3: add  = PY_mb_xiang; break;
                case 4: add  = PY_mb_xiao; break;
                case 5: add  = PY_mb_xie; break;
                case 6: add  = PY_mb_xin; break;
                case 7: add  = PY_mb_xing; break;
                case 8: add  = PY_mb_xiong; break;
                case 9: add  = PY_mb_xiu; break;
                case 10: add = PY_mb_xu; break;
                case 11: add = PY_mb_xuan; break;
                case 12: add = PY_mb_xue; break;
                case 13: add = PY_mb_xun; break;
                }
                g_hanzi_num = (unsigned short)strlen(add); //   拼音  a 的个数
                memcpy(get_hanzi, add, g_hanzi_num);       //  把找到的汉字放在缓冲区
                *hh         = g_hanzi_num / 2;
                return 1;
            }
        }
        return 0;
    }

    //首字母为 'y'
    if (py_ime_cmp == 'y')
    {
        for (py_i = 0; py_i < sizeofY; py_i++)                     // 16次
        {
            memcpy(py_ime_temp, PY_index_y[py_i], 5);              //   ai,an,ang,ao，ei,...   16个
            py_ime_cmp = memcmp(&py_ime_temp1[1], py_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (py_ime_cmp == 0)
            {
                switch (py_i)
                {
                case 0: add  = PY_mb_ya; break;
                case 1: add  = PY_mb_yai; break;
                case 2: add  = PY_mb_yan; break;
                case 3: add  = PY_mb_yang; break;
                case 4: add  = PY_mb_yao; break;
                case 5: add  = PY_mb_ye; break;
                case 6: add  = PY_mb_yi; break;
                case 7: add  = PY_mb_yin; break;
                case 8: add  = PY_mb_ying; break;
                case 9: add  = PY_mb_yo; break;
                case 10: add  = PY_mb_yong; break;
                case 11: add = PY_mb_you; break;
                case 12: add = PY_mb_yu; break;
                case 13: add = PY_mb_yuan; break;
                case 14: add = PY_mb_yue; break;
                case 15: add = PY_mb_yun; break;
                }
                g_hanzi_num = (unsigned short)strlen(add); //   拼音  a 的个数
                memcpy(get_hanzi, add, g_hanzi_num);       //  把找到的汉字放在缓冲区
                *hh         = g_hanzi_num / 2;
                return 1;
            }
        }
        return 0;
    }

    //首字母为 'z'
    if (py_ime_cmp == 'z')
    {
        for (py_i = 0; py_i < sizeofZ; py_i++)                     // 16次
        {
            memcpy(py_ime_temp, PY_index_z[py_i], 5);              //   ai,an,ang,ao，ei,...   16个
            py_ime_cmp = memcmp(&py_ime_temp1[1], py_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (py_ime_cmp == 0)
            {
                switch (py_i)
                {
                case 0: add  = PY_mb_za; break;
                case 1: add  = PY_mb_zai; break;
                case 2: add  = PY_mb_zan; break;
                case 3: add  = PY_mb_zang; break;
                case 4: add  = PY_mb_zao; break;
                case 5: add  = PY_mb_ze; break;
                case 6: add  = PY_mb_zei; break;
                case 7: add  = PY_mb_zen; break;
                case 8: add  = PY_mb_zeng; break;
                case 9: add  = PY_mb_zha; break;
                case 10: add = PY_mb_zhai; break;
                case 11: add = PY_mb_zhan; break;
                case 12: add = PY_mb_zhang; break;
                case 13: add = PY_mb_zhao; break;
                case 14: add = PY_mb_zhe; break;
                case 15: add = PY_mb_zhen; break;
                case 16: add = PY_mb_zheng; break;
                case 17: add = PY_mb_zhi; break;
                case 18: add = PY_mb_zhong; break;
                case 19: add = PY_mb_zhou; break;
                case 20: add = PY_mb_zhu; break;
                case 21: add = PY_mb_zhua; break;
                case 22: add = PY_mb_zhuai; break;
                case 23: add = PY_mb_zhuan; break;
                case 24: add = PY_mb_zhuang; break;
                case 25: add = PY_mb_zhui; break;
                case 26: add = PY_mb_zhun; break;
                case 27: add = PY_mb_zhuo; break;
                case 28: add = PY_mb_zi; break;
                case 29: add = PY_mb_zong; break;
                case 30: add = PY_mb_zou; break;
                case 31: add = PY_mb_zu; break;
                case 32: add = PY_mb_zuan; break;
                case 33: add = PY_mb_zui; break;
                case 34: add = PY_mb_zun; break;
                case 35: add = PY_mb_zuo; break;
                }
                g_hanzi_num = (unsigned short)strlen(add); //   拼音  a 的个数
                memcpy(get_hanzi, add, g_hanzi_num);       //  把找到的汉字放在缓冲区
                *hh         = g_hanzi_num / 2;
                return 1;
            }
        }
        return 0;
    }

    return 0;                      /*无果而终*/
}