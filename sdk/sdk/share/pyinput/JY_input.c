// need to install VS90SP1-KB980263-x86.exe for vs2008
#pragma execution_character_set("utf-8")

#include <stdio.h>
#include <string.h>
#include "JY_code.h"
#define sizeofB 56  //ㄅ
#define sizeofP 59  //ㄆ
#define sizeofM 57  //ㄇ
#define sizeofF 31	//ㄈ
#define sizeofD 59  //ㄉ
#define sizeofT 66  //ㄊ
#define sizeofN 53  //ㄋ
#define sizeofL 80  //ㄌ
#define sizeofG 55  //ㄍ
#define sizeofK 49  //ㄎ
#define sizeofH 63  //ㄏ
#define sizeofJ 44	//ㄐ
#define sizeofQ 49	//ㄑ
#define sizeofX 51	//ㄒ
#define sizeofZH 59  //ㄓ
#define sizeofCH 61  //ㄔ
#define sizeofSH 58  //ㄕ
#define sizeofR 31  //ㄖ
#define sizeofZ 48  //ㄗ
#define sizeofC 44  //ㄘ
#define sizeofS 39  //ㄙ
#define sizeofA 5  //ㄚ
#define sizeofO 3  //ㄛ
#define sizeofE 4  //ㄜ
#define sizeofEI 1  //ㄝ,ㄟ
#define sizeofAI 4  //ㄞ
#define sizeofAO 4  //ㄠ
#define sizeofOU 3  //ㄡ
#define sizeofAN 3  //ㄢ
#define sizeofEN 2  //ㄣ
#define sizeofANG 3  //ㄤ
#define sizeofER 4  //ㄦ
#define sizeofY 41  //ㄧ
#define sizeofW 32  //ㄨ
#define sizeofYU 18  //ㄩ


//注音符號與標準鍵盤對照表
// ㄅ:1  ㄆ:Q  ㄇ:A  ㄈ:Z  ㄉ:2  ㄊ:W  ㄋ:S  ㄌ:X
// ㄍ:E  ㄎ:D  ㄏ:C  ㄐ:R  ㄑ:F  ㄒ:V  ㄓ:5  ㄔ:T
// ㄕ:G  ㄖ:B  ㄗ:Y  ㄘ:H  ㄙ:N  ㄧ:U  ㄨ:J  ㄩ:M
// ㄚ 8  ㄛ:I  ㄜ:K  ㄝ:o  ㄞ:9  ㄟ:O  ㄠ:L  ㄡ:.
// ㄢ 0  ㄣ:P  ㄤ:;  ㄥ:/  ㄦ:-
// 二聲: 6  三聲: 3  四聲: 4  入聲: 7

// ㄅ
const unsigned char JY_index_b[][6] = {
    {"8    "},
    {"86   "},
    {"83   "},
    {"84   "},
    {"87   "},
    {"i    "},
    {"i6   "},
    {"i3   "},
    {"i4   "},
    {"i7   "},
    {"9    "},
    {"96   "},
    {"93   "},
    {"94   "},
    {"o    "},
    {"o3   "},
    {"o4   "},
    {"l    "},
    {"l6   "},
    {"l3   "},
    {"l4   "},
    {"0    "},
    {"03   "},
    {"04   "},
    {"p    "},
    {"p3   "},
    {"p4   "},
    {";    "},
    {";3   "},
    {";4   "},
    {"/    "},
    {"/6   "},
    {"/3   "},
    {"/4   "},        
    {"u    "},
    {"u6   "},
    {"u3   "},
    {"u4   "},        
    {"uo   "},
    {"uo6  "},
    {"uo3  "},
    {"uo4  "},
    {"ul   "},
    {"ul3  "},
    {"ul4  "},
    {"u0   "},
    {"u03  "},
    {"u04  "},            
    {"up   "},
    {"up4  "},            
    {"u/   "},
    {"u/3  "},
    {"u/4  "},
    {"j    "},
    {"j3   "},
    {"j4   "}                    
};

// ㄆ
const unsigned char JY_index_p[][6] = {
    {"8    "},
    {"86   "},
    {"84   "},
    {"i    "},
    {"i6   "},
    {"i3   "},
    {"i4   "},
    {"9    "},
    {"96   "},
    {"93   "},
    {"94   "},
    {"o    "},
    {"o6   "},
    {"o4   "},
    {"l    "},
    {"l6   "},
    {"l3   "},
    {"l4   "},
    {".    "},
    {".6   "},
    {".3   "},   
    {"0    "},
    {"06   "},
    {"04   "},
    {"p    "},
    {"p6   "},
    {"p4   "},
    {";    "},
    {";6   "},
    {";3   "},
    {";4   "},
    {"/    "},
    {"/6   "},
    {"/3   "},
    {"/4   "},        
    {"u    "},
    {"u6   "},
    {"u3   "},
    {"u4   "},
    {"uo   "},
    {"uo3  "},
    {"ul   "},
    {"ul6  "},
    {"ul3  "},
    {"ul4  "},
    {"u0   "},
    {"u06  "},
    {"u03  "},
    {"u04  "},            
    {"up   "},
    {"up6  "},            
    {"up3  "},            
    {"up4  "},            
    {"u/   "},
    {"u/6  "},
    {"j    "},
    {"j6   "},
    {"j3   "},
    {"j4   "}      
};

// ㄇ
const unsigned char JY_index_m[][6] = {
    {"8    "},
    {"86   "},
    {"83   "},
    {"84   "},
    {"87   "},
    {"i    "},
    {"i6   "},
    {"i3   "},
    {"i4   "},
    {"i7   "},
    {"k7   "},    
    {"96   "},
    {"93   "},
    {"94   "},
    {"o6   "},
    {"o3   "},
    {"o4   "},
    {"l    "},
    {"l6   "},
    {"l3   "},
    {"l4   "},
    {".6   "},
    {".3   "},   
    {"0    "},
    {"06   "},
    {"03   "},
    {"04   "},
    {"p    "},
    {"p6   "},
    {"p4   "},
    {"p7   "},
    {";6   "},
    {";3   "},
    {"/    "},
    {"/6   "},
    {"/3   "},
    {"/4   "},        
    {"u    "},
    {"u6   "},
    {"u3   "},
    {"u4   "},
    {"uo   "},
    {"uo4  "},
    {"ul   "},
    {"ul6  "},
    {"ul3  "},
    {"ul4  "},
    {"u.4  "},    
    {"u06  "},
    {"u03  "},
    {"u04  "},            
    {"up6  "},            
    {"up3  "},                     
    {"u/6  "},
    {"u/4  "},
    {"j3   "},
    {"j4   "}
};

// ㄈ
const unsigned char JY_index_f[][6] = {
    {"8    "},
    {"86   "},
    {"83   "},
    {"84   "},
    {"i6   "}, 
    {"o    "},
    {"o6   "},
    {"o3   "},
    {"o4   "},
    {".6   "},
    {".3   "},   
    {"0    "},
    {"06   "},
    {"03   "},
    {"04   "},
    {"p    "},
    {"p6   "},
    {"p3   "},
    {"p4   "},
    {";    "},
    {";6   "},
    {";3   "},
    {";4   "},
    {"/    "},
    {"/6   "},
    {"/3   "},
    {"/4   "},        
    {"j    "},
    {"j6   "},
    {"j3   "},
    {"j4   "}
};

// ㄉ
const unsigned char JY_index_d[][6] = {
    {"8    "},
    {"86   "},
    {"83   "},
    {"84   "},
    {"k6   "},
    {"k7   "},
    {"9    "},
    {"93   "},
    {"94   "},
    {"o3   "},
    {"l    "},
    {"l3   "},
    {"l4   "},
    {".    "},
    {".3   "},   
    {".4   "},   
    {"0    "},
    {"03   "},
    {"04   "},
    {";    "},
    {";3   "},
    {";4   "},
    {"/    "},
    {"/3   "},
    {"/4   "},        
    {"u    "},
    {"u6   "},
    {"u3   "},
    {"u4   "},
    {"uo   "},
    {"uo6  "},
    {"ul3  "},
    {"ul4  "},
    {"u.   "},    
    {"u0   "},
    {"u03  "},
    {"u04  "},                                
    {"u/   "},
    {"u/3  "},
    {"u/4  "},
    {"j    "},
    {"j6   "},
    {"j3   "},
    {"j4   "},
    {"ji   "},
    {"ji6  "},
    {"ji3  "},
    {"ji4  "},
    {"jo   "},
    {"jo4  "},
    {"j0   "},
    {"j03  "},
    {"j04  "},
    {"jp   "},
    {"jp3  "},
    {"jp4  "},
    {"j/   "},
    {"j/3  "},
    {"j/4  "}   
};

// ㄊ
const unsigned char JY_index_t[][6] = {
    {"8    "},
    {"83   "},
    {"84   "},
    {"k6   "},
    {"k4   "},
    {"9    "},
    {"96   "},
    {"94   "},
    {"l    "},
    {"l6   "},
    {"l3   "},
    {"l4   "},
    {".    "},
    {".6   "},   
    {".3   "},   
    {".4   "},   
    {"0    "},
    {"06   "},
    {"03   "},
    {"04   "},
    {";    "},
    {";6   "},
    {";3   "},
    {";4   "},
    {"/6   "},      
    {"u    "},
    {"u6   "},
    {"u3   "},
    {"u4   "},
    {"uo   "},
    {"uo3  "},
    {"uo4  "},
    {"ul   "},
    {"ul6  "},
    {"ul3  "},
    {"ul4  "}, 
    {"u0   "},
    {"u06  "},
    {"u03  "},
    {"u04  "},                                
    {"u/   "},
    {"u/6  "},
    {"u/3  "},
    {"u/4  "},
    {"j    "},
    {"j6   "},
    {"j3   "},
    {"j4   "},
    {"j06  "},
    {"j03  "},
    {"j04  "},
    {"ji   "},
    {"ji6  "},
    {"ji3  "},
    {"ji4  "},
    {"jo   "},
    {"jo6  "},
    {"jo3  "},
    {"jo4  "},
    {"jp   "},
    {"jp2  "},
    {"jp4  "},
    {"j/   "},
    {"j/6  "},
    {"j/3  "},
    {"j/4  "}   
};

// ㄋ
const unsigned char JY_index_n[][6] = {
    {"86   "},
    {"83   "},
    {"84   "},
	{"87   "},
    {"k4   "},
    {"k7   "},
    {"93   "},
    {"94   "},
    {"o3   "},
    {"o4   "},    
    {"l6   "},
    {"l3   "},
    {"l4   "}, 
    {".4   "},   
    {"0    "},
    {"06   "},
    {"03   "},
    {"04   "},
    {"p4   "},
    {";6   "},
    {";3   "},
    {"/6   "},      
    {"u6   "},
    {"u3   "},
    {"u4   "},
    {"uo   "},
    {"uo4  "},
    {"ul3  "},
    {"ul4  "}, 
    {"u.   "},
    {"u.6  "},
    {"u.3  "},
    {"u.4  "},                                    
    {"u0   "},
    {"u06  "},
    {"u03  "},
    {"u04  "}, 
    {"up6  "},  
    {"u;6  "},  
    {"u;4  "},                                       
    {"u/6  "},
    {"u/3  "},
    {"u/4  "},
    {"j6   "},
    {"j3   "},
    {"j4   "},
    {"ji6  "},
    {"ji4  "},
    {"j03  "},
    {"j/6  "},
    {"j/4  "},
    {"m3   "},
    {"mo4  "}     
};

//ㄌ
const unsigned char JY_index_l[][6] = {
    {"8    "},
    {"86   "},
    {"83   "},
    {"84   "},
    {"87   "},
    {"k4   "},
    {"k7   "},
    {"96   "},
    {"94   "},
    {"o    "},
    {"o6   "},    
    {"o3   "},
    {"o4   "},    
    {"l    "},
    {"l6   "},
    {"l3   "},
    {"l4   "}, 
    {"06   "},
    {"03   "},
    {"04   "},
    {";6   "},
    {";3   "},
    {";4   "},
    {"/6   "},      
    {"/3   "},      
    {"/4   "},      
    {"u    "},
    {"u6   "},
    {"u3   "},
    {"u4   "},
    {"u7   "},
    {"uo   "},
    {"uo3  "},
    {"uo4  "},
    {"ul   "},
    {"ul6  "}, 
    {"ul3  "},
    {"ul4  "}, 
    {"u.   "},
    {"u.6  "},
    {"u.3  "},
    {"u.4  "},                                    
    {"u06  "},
    {"u03  "},
    {"u04  "}, 
    {"up6  "},  
    {"up3  "},  
    {"up4  "},  
    {"u;6  "},  
    {"u;3  "},                                       
    {"u;4  "},                                       
    {"u/6  "},
    {"u/3  "},
    {"u/4  "},
    {".    "},
    {".6   "},
    {".3   "},
    {".4   "},    
    {"j    "},
    {"j6   "},
    {"j3   "},
    {"j4   "},
    {"ji   "},
    {"ji6  "},
    {"ji3  "},
    {"ji4  "},
    {"j06  "},
    {"j03  "},
    {"j04  "},
    {"jp   "},
    {"jp6  "},
    {"jp4  "},
    {"j/   "},
    {"j/6  "},
    {"j/3  "},
    {"j/4  "},
    {"m6   "},
    {"m3   "},
    {"m4   "},
    {"mo4  "}   
};

//ㄍ
const unsigned char JY_index_g[][6] = {
    {"86   "},
    {"83   "},
    {"84   "},
    {"k    "},
    {"k6   "},
    {"k3   "},
    {"k4   "},
    {"9    "},
    {"96   "},
    {"94   "},  
    {"o3   "},  
    {"l    "},
    {"l3   "},
    {"l4   "}, 
    {".    "},
    {".3   "},
    {".4   "},
    {"0    "},
    {"03   "},
    {"04   "},
    {"p    "},
    {"p4   "},
    {";    "},
    {";3   "},
    {";4   "},
    {"/    "},      
    {"/3   "},      
    {"/4   "},        
    {"j    "},
    {"j3   "},
    {"j4   "},
    {"j8   "},
    {"j83  "},
    {"j84  "},    
    {"ji   "},
    {"ji6  "},
    {"ji3  "},
    {"ji4  "},
    {"j9   "},
    {"j93  "},
    {"j94  "}, 
    {"jo   "},
    {"jo3  "},
    {"jo4  "},        
    {"j0   "},
    {"j03  "},
    {"j04  "},
    {"jp3  "},
    {"jp4  "},
    {"j;   "},
    {"j;3  "},
    {"j;4  "},
    {"j/   "},
    {"j/3  "},
    {"j/4  "}
};

//ㄎ
const unsigned char JY_index_k[][6] = {
    {"8    "},
    {"83   "},
    {"k    "},
    {"k6   "},
    {"k3   "},
    {"k4   "},
    {"9    "},
    {"93   "},
    {"94   "},   
    {"l    "},
    {"l3   "},
    {"l4   "}, 
    {".    "},
    {".3   "},
    {".4   "},
    {"0    "},
    {"03   "},
    {"04   "},
    {"p3   "},
    {"p4   "},
    {";    "},
    {";6   "},
    {";4   "},
    {"/    "},           
    {"j    "},
    {"j3   "},
    {"j4   "},
    {"j8   "},
    {"j83  "},
    {"j84  "},    
    {"ji4  "},
    {"j9   "},
    {"j93  "},
    {"j94  "}, 
    {"jo   "},
    {"jo6  "},
    {"jo3  "},
    {"jo4  "},        
    {"j0   "},
    {"j03  "},
    {"jp   "},
    {"jp3  "},
    {"jp4  "},
    {"j;   "},
    {"j;6  "},
    {"j;4  "},
    {"j/   "},
    {"j/3  "},
    {"j/4  "}
};

//ㄏ
const unsigned char JY_index_h[][6] = {
    {"8    "},
    {"86   "},
    {"84   "},
    {"k    "},
    {"k6   "},
    {"k4   "},
    {"9    "},
    {"96   "},
    {"93   "},
    {"94   "},   
    {"o    "},   
    {"l    "},
    {"l6   "},
    {"l3   "},
    {"l4   "}, 
    {".    "},
    {".6   "},
    {".3   "},
    {".4   "},
    {"0    "},
    {"06   "},
    {"03   "},
    {"04   "},
    {"p6   "},
    {"p3   "},
    {"p4   "},
    {";    "},
    {";6   "},
    {"/    "},           
    {"/6   "},           
    {"/4   "},           
    {"j    "},
    {"j6   "},
    {"j3   "},
    {"j4   "},
    {"j8   "},
    {"j86  "},
    {"j84  "},    
    {"ji   "},
    {"ji6  "},
    {"ji3  "},
    {"ji4  "},
    {"j96  "},
    {"j94  "}, 
    {"jo   "},
    {"jo6  "},
    {"jo3  "},
    {"jo4  "},        
    {"j0   "},
    {"j06  "},
    {"j03  "},
    {"j04  "},
    {"jp   "},
    {"jp6  "},
    {"jp4  "},
    {"j;   "},
    {"j;6  "},
    {"j;3  "},
    {"j;4  "},
    {"j/   "},
    {"j/6  "},
    {"j/3  "},
    {"j/4  "}
};

//ㄐ
const unsigned char JY_index_j[][6] = {
    {"u    "},
    {"u6   "},
    {"u3   "},
    {"u4   "},
    {"u8   "},
    {"u86  "},
    {"u83  "},
    {"u84  "},            
    {"uo   "},
    {"uo6  "},
    {"uo3  "},
    {"uo4  "},
    {"ul   "},
    {"ul6  "},
    {"ul3  "},
    {"ul4  "},
    {"u.   "},
    {"u.3  "},
    {"u.4  "},   
    {"u0   "},
    {"u03  "},
    {"u04  "},            
    {"up   "},
    {"up3  "},            
    {"up4  "},            
    {"u/   "},
    {"u/3  "},
    {"u/4  "},
    {"u;   "},
    {"u;3  "},
    {"u;4  "},
    {"m    "},
    {"m6   "},
    {"m3   "},
    {"m4   "},
    {"mo   "},
    {"mo6  "},
    {"m0   "},
    {"m03  "},
    {"m04  "},
    {"mp   "},
    {"mp4  "},
    {"m/   "},
    {"m/3  "}    
};

//ㄑ
const unsigned char JY_index_q[][6] = {
    {"u    "},
    {"u6   "},
    {"u3   "},
    {"u4   "},
    {"u8   "},
    {"u83  "},
    {"u84  "},            
    {"uo   "},
    {"uo6  "},
    {"uo3  "},
    {"uo4  "},
    {"ul   "},
    {"ul6  "},
    {"ul3  "},
    {"ul4  "},
    {"u.   "},
    {"u.6  "},
    {"u.3  "},   
    {"u0   "},
    {"u06  "},
    {"u03  "},
    {"u04  "},            
    {"up   "},
    {"up6  "},            
    {"up3  "},            
    {"up4  "},            
    {"u;   "},
    {"u;6  "},
    {"u;3  "},
    {"u;4  "},
    {"u/   "},
    {"u/6  "},
    {"u/3  "},
    {"u/4  "},
    {"m    "},
    {"m6   "},
    {"m3   "},
    {"m4   "},
    {"mo   "},
    {"mo6  "},
    {"mo4  "},
    {"m0   "},
    {"m06  "},
    {"m03  "},
    {"m04  "},
    {"mp   "},
    {"mp6  "},
    {"m/   "},
    {"m/6  "}    
};

//ㄒ
const unsigned char JY_index_x[][6] = {
    {"u    "},
    {"u6   "},
    {"u3   "},
    {"u4   "},
    {"u8   "},
    {"u86  "},
    {"u84  "},            
    {"uo   "},
    {"uo6  "},
    {"uo3  "},
    {"uo4  "},
    {"ul   "},
    {"ul3  "},
    {"ul4  "},
    {"u.   "},
    {"u.3  "},
    {"u.4  "},   
    {"u0   "},
    {"u06  "},
    {"u03  "},
    {"u04  "},            
    {"up   "},
    {"up6  "},            
    {"up3  "},            
    {"up4  "},            
    {"u;   "},
    {"u;6  "},
    {"u;3  "},
    {"u;4  "},
    {"u/   "},
    {"u/6  "},
    {"u/3  "},
    {"u/4  "},
    {"m    "},
    {"m6   "},
    {"m3   "},
    {"m4   "},
    {"mo   "},
    {"mo6  "},
    {"mo3  "},
    {"mo4  "},
    {"m0   "},
    {"m06  "},
    {"m03  "},
    {"m04  "},
    {"mp   "},
    {"mp6  "},
    {"mp4  "},
    {"m/   "},
    {"m/6  "},    
    {"m/4  "}    
};

//ㄓ
const unsigned char JY_index_zh[][6] = {
    {"     "},
    {"6    "},
    {"3    "},
    {"4    "},
    {"8    "},
    {"86   "},
    {"83   "},
    {"84   "},
    {"k    "},
    {"k6   "},
    {"k3   "},
    {"k4   "},   
    {"9    "},
    {"96   "},
    {"93   "},
    {"94   "},
    {"l    "},
    {"l6   "},
    {"l3   "},
    {"l4   "},
    {".    "},
    {".6   "},   
    {".3   "},
    {".4   "},   
    {"0    "},
    {"03   "},
    {"04   "},
    {"p    "},
    {"p3   "},
    {"p4   "},
    {";    "},
    {";3   "},
    {";4   "},
    {"/    "},
    {"/3   "},
    {"/4   "},        
    {"j    "},
    {"j6   "},
    {"j3   "},
    {"j4   "},
    {"j8   "},
    {"j83  "},
    {"ji   "},
    {"ji6  "},
    {"j9   "},
    {"j93  "},
    {"j94  "},
    {"j;   "},
    {"j;4  "},
    {"jo   "},
    {"jo4  "},
    {"j0   "},
    {"j03  "},
    {"j04  "},
    {"jp   "},
    {"jp3  "},
    {"j/   "},
    {"j/3  "},
    {"j/4  "}
};

//ㄔ
const unsigned char JY_index_ch[][6] = {
    {"     "},
    {"6    "},
    {"3    "},
    {"4    "},
    {"8    "},
    {"86   "},
    {"83   "},
    {"84   "},
    {"k    "},
    {"k3   "},
    {"k4   "},   
    {"9    "},
    {"96   "},
    {"94   "},
    {"l    "},
    {"l6   "},
    {"l3   "},
    {".    "},
    {".6   "},   
    {".3   "},
    {".4   "},   
    {"0    "},
    {"06   "},
    {"03   "},
    {"04   "},
    {"p    "},
    {"p6   "},
    {"p4   "},
    {";    "},
    {";6   "},
    {";3   "},
    {";4   "},
    {"/    "},
    {"/6   "},
    {"/3   "},
    {"/4   "},        
    {"j    "},
    {"j6   "},
    {"j3   "},
    {"j4   "},
    {"ji   "},
    {"ji4  "},
    {"j93  "},
    {"j94  "},
    {"jo   "},
    {"jo6  "},
    {"j0   "},
    {"j06  "},
    {"j03  "},
    {"j04  "},
    {"j;   "},
    {"j;6  "},
    {"j;3  "},
    {"j;4  "},
    {"jp   "},
    {"jp6  "},
    {"jp3  "},
    {"j/   "},
    {"j/6  "},
    {"j/3  "},
    {"j/4  "}
};

//ㄕ
const unsigned char JY_index_sh[][6] = {
    {"     "},
    {"6    "},
    {"3    "},
    {"4    "},
    {"8    "},
    {"86   "},
    {"83   "},
    {"84   "},
    {"k    "},
    {"k6   "},
    {"k3   "},
    {"k4   "},   
    {"9    "},
    {"93   "},
    {"94   "},
    {"o6   "},   
    {"l    "},
    {"l6   "},
    {"l3   "},
    {"l4   "},
    {".    "},
    {".3   "},
    {".4   "},   
    {"0    "},
    {"03   "},
    {"04   "},
    {"p    "},
    {"p6   "},
    {"p3   "},
    {"p4   "},
    {";    "},
    {";3   "},
    {";4   "},
    {"/    "},
    {"/6   "},
    {"/3   "},
    {"/4   "},        
    {"j    "},
    {"j6   "},
    {"j3   "},
    {"j4   "},
    {"j8   "},
    {"j83  "},
    {"j84  "},
    {"ji   "},
    {"ji4  "},
    {"j9   "},
    {"j93  "},
    {"j94  "},
    {"jo2  "},
    {"jo3  "},
    {"jo4  "},
    {"j0   "},
    {"j04  "},
    {"jp3  "},
    {"jp4  "},
    {"j;   "},
    {"j;3  "}
};

//ㄖ
const unsigned char JY_index_r[][6] = {
    {"4    "},
    {"k3   "},
    {"k4   "},   
    {"l6   "},
    {"l3   "},
    {"l4   "},
    {".6   "},   
    {".4   "},   
    {"06   "},
    {"03   "},
    {"p6   "},
    {"p3   "},
    {"p4   "},
    {";6   "},
    {";3   "},
    {";4   "},
    {"/    "},
    {"/6   "},  
    {"j6   "},
    {"j3   "},
    {"j4   "},
    {"ji4  "},
    {"jo6  "},
    {"jo3  "},
    {"jo4  "},
    {"j06  "},
    {"j03  "},
    {"jp6  "},
    {"jp4  "},
    {"j/6  "},
    {"j/3  "}
};

//ㄗ
const unsigned char JY_index_z[][6] = {
    {"     "},
    {"6    "},
    {"3    "},
    {"4    "},
    {"8    "},
    {"86   "},
    {"k6   "},
    {"k3   "},
    {"k4   "},   
    {"9    "},
    {"93   "},
    {"94   "},
    {"l    "},
    {"l6   "},
    {"l3   "},
    {"l4   "},
    {"o6   "},       
    {".    "},
    {".3   "},
    {".4   "},   
    {"0    "},
    {"06   "},
    {"03   "},
    {"04   "},
    {"p3   "},
    {"p4   "},
    {";    "},
    {";3   "},
    {";4   "},
    {"/    "},
    {"/4   "},        
    {"j    "},
    {"j6   "},
    {"j3   "},
    {"ji   "},
    {"ji6  "},
    {"ji3  "},
    {"ji4  "},
    {"jo3  "},
    {"jo4  "},
    {"jp   "},
    {"jp3  "},
    {"j0   "},
    {"j03  "},
    {"j04  "},
    {"j/   "},
    {"j/3  "},
    {"j/4  "}
};

//ㄘ
const unsigned char JY_index_c[][6] = {
    {"     "},
    {"6    "},
    {"3    "},
    {"4    "},
    {"8    "},
    {"k4   "},   
    {"9    "},
    {"96   "},
    {"93   "},
    {"94   "},
    {"l    "},
    {"l6   "},
    {"l3   "},
    {".4   "},   
    {"0    "},
    {"06   "},
    {"03   "},
    {"04   "},
    {"p    "},
    {"p6   "},
    {";    "},
    {";6   "},
    {"/    "},
    {"/6   "},
    {"/4   "},        
    {"j    "},
    {"j6   "},
    {"j4   "},
    {"ji   "},
    {"ji6  "},
    {"ji3  "},
    {"ji4  "},
    {"jo   "},
    {"jo3  "},
    {"jo4  "},
    {"j0   "},
    {"j06  "},
    {"j04  "},
    {"jp   "},
    {"jp6  "},
    {"jp3  "},
    {"jp4  "},
    {"j/   "},
    {"j/6  "}
};

//ㄙ
const unsigned char JY_index_s[][6] = {
    {"     "},
    {"3    "},
    {"4    "},
    {"8    "},
    {"83   "},
    {"84   "},
    {"k4   "},   
    {"9    "},
    {"94   "},
    {"0    "},
    {"03   "},
    {"04   "}, 
    {"l    "},
    {"l3   "},
    {"l4   "},
    {".    "},
    {".3   "},
    {".4   "},   
    {"p    "},
    {";    "},
    {";3   "},
    {";4   "},
    {"/    "},      
    {"j    "},
    {"j6   "},
    {"j4   "},
    {"ji   "},
    {"ji3  "},
    {"jo   "},
    {"jo6  "},
    {"jo3  "},
    {"jo4  "},
    {"j0   "},
    {"j04  "},
    {"jp   "},
    {"jp3  "},
    {"j/   "},
    {"j/3  "},
    {"j/4  "}
};

const unsigned char JY_index_a[][6] = {
    {"     "},
    {"6    "},
    {"3    "},
    {"4    "},
    {"7    "}
};

const unsigned char JY_index_o[][6] = {
    {"     "},
    {"6    "},
    {"3    "}
};

const unsigned char JY_index_e[][6] = {
    {"     "},
    {"6    "},
    {"3    "},
    {"4    "}
};

const unsigned char JY_index_ei[][6] = {
    {"4    "}
};

const unsigned char JY_index_ai[][6] = {
    {"     "},
    {"6    "},
    {"3    "},
    {"4    "}
};

const unsigned char JY_index_ao[][6] = {
    {"     "},
    {"6    "},
    {"3    "},
    {"4    "}
};

const unsigned char JY_index_ou[][6] = {
    {"     "},
    {"3    "},
    {"4    "}
};

const unsigned char JY_index_an[][6] = {
    {"     "},
    {"3    "},
    {"4    "}
};

const unsigned char JY_index_en[][6] = {
    {"     "},
    {"4    "}
};

const unsigned char JY_index_ang[][6] = {
    {"     "},
    {"6    "},
    {"4    "}
};

const unsigned char JY_index_er[][6] = {
    {"6    "},
    {"3    "},
    {"4    "},
    {"7    "}
};

// ㄧ
const unsigned char JY_index_y[][6] = {
    {"     "},
    {"6    "},
    {"3    "},
    {"4    "},
    {"8    "},
    {"86   "},
    {"83   "},
    {"84   "},
    {"87   "},
    {"i    "},
    {"i7   "},
    {"o    "},
    {"o6   "},
    {"o3   "},
    {"o4   "},
    {"96   "},
    {"l    "},
    {"l6   "},
    {"l3   "},
    {"l4   "},
    {"l7   "},
    {".    "},
    {".6   "},
    {".3   "},
    {".4   "},
    {"0    "},
    {"06   "},
    {"03   "},
    {"04   "},
    {"p    "},
    {"p6   "},
    {"p3   "},
    {"p4   "},
    {";    "},
    {";6   "},
    {";3   "},
    {";4   "},
    {"/    "},
    {"/6   "},
    {"/3   "},
    {"/4   "}
};

// ㄨ
const unsigned char JY_index_w[][6] = {
    {"     "},
    {"6    "},
    {"3    "},
    {"4    "},
    {"8    "},
    {"86   "},
    {"83   "},
    {"84   "},
    {"i    "},
    {"i3   "},
    {"i4   "},
    {"9    "},
    {"94   "},
    {"o    "},
    {"o6   "},
    {"o3   "},
    {"o4   "},
    {"0    "},
    {"06   "},
    {"03   "},
    {"04   "},
    {"p    "},
    {"p6   "},
    {"p3   "},
    {"p4   "},
    {";    "},
    {";6   "},
    {";3   "},
    {";4   "},
    {"/    "},
    {"/3   "},
    {"/4   "}
};

// ㄩ
const unsigned char JY_index_yu[][6] = {
    {"     "},
    {"6    "},
    {"3    "},
    {"4    "},
    {"o    "},
    {"o4   "},
    {"0    "},
    {"06   "},
    {"03   "},
    {"04   "},
    {"p    "},
    {"p6   "},
    {"p3   "},
    {"p4   "},
    {"/    "},
    {"/6   "},
    {"/3   "},
    {"/4   "}
};

const unsigned char JY_index_end[][6] = {"     "};

unsigned char  jy_ime(unsigned char *input_jy_val, unsigned char *get_hanzi, unsigned short *hh)
{
    unsigned char  jy_ime_temp[8];
    unsigned char  jy_ime_temp1[8];
    unsigned char  jy_ime_cmp;
    //unsigned char jy_ime_cmp1;
    unsigned char  jy_i;
    unsigned short g_hanzi_num;
    unsigned char  *add;
    memcpy(jy_ime_temp1, input_jy_val, 6); //   把输入的拼音放入缓冲区
    jy_ime_cmp = input_jy_val[0];          // 读取拼音的第一个字母

                                           //  输入拼音开头为 'i','u','v',的汉字不存在

    printf("jy_ime_cmp: %c\n", jy_ime_cmp);

    //首字母为 'ㄅ' (1)

    if (jy_ime_cmp == '1')
    {
        for (jy_i = 0; jy_i < sizeofB; jy_i++)                     // 16次
        {
            memcpy(jy_ime_temp, JY_index_b[jy_i], 5);              //   ai,an,ang,ao，ei,...   16个
            jy_ime_cmp = memcmp(&jy_ime_temp1[1], jy_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (jy_ime_cmp == 0)
            {
                switch (jy_i)
                {
                case 0: add  = JY_mb_ba; break; 
                case 1: add  = JY_mb_ba2; break; 
                case 2: add  = JY_mb_ba3; break; 
                case 3: add  = JY_mb_ba4; break; 
                case 4: add  = JY_mb_ba5; break; 
                case 5: add  = JY_mb_bo; break; 
                case 6: add  = JY_mb_bo2; break; 
                case 7: add  = JY_mb_bo3; break; 
                case 8: add  = JY_mb_bo4; break; 
                case 9: add  = JY_mb_bo5; break;  
                case 10: add  = JY_mb_bai; break;
                case 11: add  = JY_mb_bai2; break;
                case 12: add  = JY_mb_bai3; break;
                case 13: add  = JY_mb_bai4; break;
                case 14: add  = JY_mb_bei; break;
                case 15: add  = JY_mb_bei3; break;
                case 16: add  = JY_mb_bei4; break; 
                case 17: add  = JY_mb_bao; break; 
                case 18: add  = JY_mb_bao2; break;
                case 19: add  = JY_mb_bao3; break; 
                case 20: add  = JY_mb_bao4; break; 
                case 21: add  = JY_mb_ban; break; 
                case 22: add  = JY_mb_ban3; break; 
                case 23: add  = JY_mb_ban4; break; 
                case 24: add  = JY_mb_ben; break; 
                case 25: add  = JY_mb_ben3; break; 
                case 26: add  = JY_mb_ben4; break; 
                case 27: add  = JY_mb_bang; break; 
                case 28: add  = JY_mb_bang3; break; 
                case 29: add  = JY_mb_bang4; break; 
                case 30: add  = JY_mb_beng; break; 
                case 31: add  = JY_mb_beng2; break; 
                case 32: add  = JY_mb_beng3; break; 
                case 33: add  = JY_mb_beng4; break; 
                case 34: add  = JY_mb_bi; break; 
                case 35: add  = JY_mb_bi2; break; 
                case 36: add  = JY_mb_bi3; break; 
                case 37: add  = JY_mb_bi4; break; 
                case 38: add  = JY_mb_bie; break; 
                case 39: add  = JY_mb_bie2; break; 
                case 40: add  = JY_mb_bie3; break; 
                case 41: add  = JY_mb_bie4; break; 
                case 42: add  = JY_mb_biao; break; 
                case 43: add  = JY_mb_biao3; break; 
                case 44: add  = JY_mb_biao4; break; 
                case 45: add  = JY_mb_bian; break; 
                case 46: add  = JY_mb_bian3; break; 
                case 47: add  = JY_mb_bian4; break; 
                case 48: add  = JY_mb_bin; break; 
                case 49: add  = JY_mb_bin4; break; 
                case 50: add  = JY_mb_bing; break; 
                case 51: add  = JY_mb_bing3; break; 
                case 52: add  = JY_mb_bing4; break; 
                case 53: add  = JY_mb_bu; break; 
                case 54: add  = JY_mb_bu3; break; 
                case 55: add  = JY_mb_bu4; break;                
                }
                g_hanzi_num = (unsigned short)(strlen(add)); //注音ㄅ的个数
                memcpy(get_hanzi, add, g_hanzi_num);         //把找到的汉字放在缓冲区
                *hh         = g_hanzi_num / 2;
                return 1;
            }
        }
        return 0;
    }
    
    //首字母为 'ㄆ' (q)
    if (jy_ime_cmp == 'q')
    {
        for (jy_i = 0; jy_i < sizeofP; jy_i++)                     // 16次
        {
            memcpy(jy_ime_temp, JY_index_p[jy_i], 5);              //   ai,an,ang,ao，ei,...   16个
            jy_ime_cmp = memcmp(&jy_ime_temp1[1], jy_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (jy_ime_cmp == 0)
            {
                switch (jy_i)
                {
                case 0: add  = JY_mb_pa; break; 
                case 1: add  = JY_mb_pa2; break; 
                case 2: add  = JY_mb_pa4; break; 
                case 3: add  = JY_mb_po; break; 
                case 4: add  = JY_mb_po2; break; 
                case 5: add  = JY_mb_po3; break; 
                case 6: add  = JY_mb_po4; break; 
                case 7: add  = JY_mb_pai; break; 
                case 80: add  = JY_mb_pai2; break; 
                case 9: add  = JY_mb_pai3; break; 
                case 10: add  = JY_mb_pai4; break; 
                case 11: add  = JY_mb_pei; break;
                case 12: add  = JY_mb_pei2; break;
                case 13: add  = JY_mb_pei4; break;
                case 14: add  = JY_mb_pao; break; 
                case 15: add  = JY_mb_pao2; break; 
                case 16: add  = JY_mb_pao3; break; 
                case 17: add  = JY_mb_pao4; break; 
                case 18: add  = JY_mb_pou; break; 
                case 19: add  = JY_mb_pou2; break; 
                case 20: add  = JY_mb_pou3; break; 
                case 21: add  = JY_mb_pan; break; 
                case 22: add  = JY_mb_pan2; break; 
                case 23: add  = JY_mb_pan4; break; 
                case 24: add  = JY_mb_pen; break; 
                case 25: add  = JY_mb_pen2; break; 
                case 26: add  = JY_mb_pen4; break; 
                case 27: add  = JY_mb_pang; break; 
                case 28: add  = JY_mb_pang2; break; 
                case 29: add  = JY_mb_pang3; break; 
                case 30: add  = JY_mb_pang4; break; 
                case 31: add  = JY_mb_peng; break; 
                case 32: add  = JY_mb_peng2; break; 
                case 33: add  = JY_mb_peng3; break; 
                case 34: add  = JY_mb_peng4; break; 
                case 35: add  = JY_mb_pi; break;
                case 36: add  = JY_mb_pi2; break;
                case 37: add  = JY_mb_pi3; break;
                case 38: add  = JY_mb_pi4; break;
                case 39: add  = JY_mb_pie; break; 
                case 40: add  = JY_mb_pie3; break; 
                case 41: add  = JY_mb_piao; break; 
                case 42: add  = JY_mb_piao2; break; 
                case 43: add  = JY_mb_piao3; break; 
                case 44: add  = JY_mb_piao4; break; 
                case 45: add  = JY_mb_pian; break; 
                case 46: add  = JY_mb_pian2; break; 
                case 47: add  = JY_mb_pian3; break; 
                case 48: add  = JY_mb_pian4; break; 
                case 49: add  = JY_mb_pin; break; 
                case 50: add  = JY_mb_pin2; break; 
                case 51: add  = JY_mb_pin3; break; 
                case 52: add  = JY_mb_pin4; break; 
                case 53: add  = JY_mb_ping; break; 
                case 54: add  = JY_mb_ping2; break; 
                case 55: add  = JY_mb_pu; break; 
                case 56: add  = JY_mb_pu2; break; 
                case 57: add  = JY_mb_pu3; break; 
                case 58: add  = JY_mb_pu4; break;                 
                }
                g_hanzi_num = (unsigned short)strlen(add); //   注音ㄆ的个数
                memcpy(get_hanzi, add, g_hanzi_num);       //  把找到的汉字放在缓冲区
                *hh         = g_hanzi_num / 2;
                return 1;
            }
        }
        return 0;
    }

    //首字母为 'ㄇ'
    if (jy_ime_cmp == 'a')
    {
        for (jy_i = 0; jy_i < sizeofM; jy_i++)                     // 16次
        {
            memcpy(jy_ime_temp, JY_index_m[jy_i], 5);              //   ai,an,ang,ao，ei,...   16个
            jy_ime_cmp = memcmp(&jy_ime_temp1[1], jy_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (jy_ime_cmp == 0)
            {
                switch (jy_i)
                {
                case 0: add  = JY_mb_ma; break;
                case 1: add  = JY_mb_ma2; break;
                case 2: add  = JY_mb_ma3; break;
                case 3: add  = JY_mb_ma4; break;
                case 4: add  = JY_mb_ma5; break;
                case 5: add  = JY_mb_mo; break;
                case 6: add  = JY_mb_mo2; break;
                case 7: add  = JY_mb_mo3; break;
                case 8: add  = JY_mb_mo4; break;
                case 9: add  = JY_mb_mo5; break;
                case 10: add  = JY_mb_me5; break;
                case 11: add  = JY_mb_mai2; break;
                case 12: add  = JY_mb_mai3; break;
                case 13: add  = JY_mb_mai4; break;
                case 14: add  = JY_mb_mei2; break;
                case 15: add  = JY_mb_mei3; break;
                case 16: add  = JY_mb_mei4; break;
                case 17: add  = JY_mb_mao; break;
                case 18: add  = JY_mb_mao2; break;
                case 19: add  = JY_mb_mao3; break;
                case 20: add  = JY_mb_mao4; break;
                case 21: add  = JY_mb_mou2; break;
                case 22: add  = JY_mb_mou3; break;
                case 23: add  = JY_mb_man; break;
                case 24: add  = JY_mb_man2; break;
                case 25: add  = JY_mb_man3; break;
                case 26: add  = JY_mb_man4; break;
                case 27: add  = JY_mb_men; break;
                case 28: add  = JY_mb_men2; break;
                case 29: add  = JY_mb_men4; break;
                case 30: add  = JY_mb_men5; break;
                case 31: add  = JY_mb_mang2; break;
                case 32: add  = JY_mb_mang3; break;
                case 33: add  = JY_mb_meng; break;
                case 34: add  = JY_mb_meng2; break;
                case 35: add  = JY_mb_meng3; break;
                case 36: add  = JY_mb_meng4; break;
                case 37: add  = JY_mb_mi; break;
                case 38: add  = JY_mb_mi2; break;
                case 39: add  = JY_mb_mi3; break;
                case 40: add  = JY_mb_mi4; break;
                case 41: add  = JY_mb_mie; break;
                case 42: add  = JY_mb_mie4; break;
                case 43: add  = JY_mb_miao; break;
                case 44: add  = JY_mb_miao2; break;
                case 45: add  = JY_mb_miao3; break;
                case 46: add  = JY_mb_miao4; break;
                case 47: add  = JY_mb_miu4; break;
                case 48: add  = JY_mb_mian2; break;
                case 49: add  = JY_mb_mian3; break;
                case 50: add  = JY_mb_mian4; break;
                case 51: add  = JY_mb_min2; break;
                case 52: add  = JY_mb_min3; break;
                case 53: add  = JY_mb_ming2; break;
                case 54: add  = JY_mb_ming4; break;
                case 55: add  = JY_mb_mu3; break;
                case 56: add  = JY_mb_mu4; break;
                }
                g_hanzi_num = (unsigned short)strlen(add); //   拼音  a 的个数
                memcpy(get_hanzi, add, g_hanzi_num);       //  把找到的汉字放在缓冲区
                *hh         = g_hanzi_num / 2;
                return 1;
            }
        }
        return 0;
    }

    //首字母为 'ㄈ'
    if (jy_ime_cmp == 'z')
    {
        for (jy_i = 0; jy_i < sizeofF; jy_i++)                     // 16次
        {
            memcpy(jy_ime_temp, JY_index_f[jy_i], 5);              //   ai,an,ang,ao，ei,...   16个
            jy_ime_cmp = memcmp(&jy_ime_temp1[1], jy_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (jy_ime_cmp == 0)
            {
                printf("jy_i==%d\n", jy_i);
                switch (jy_i)
                {
                case 0: add = JY_mb_fa; break; 
                case 1: add = JY_mb_fa2; break; 
                case 2: add = JY_mb_fa3; break; 
                case 3: add = JY_mb_fa4; break; 
                case 4: add = JY_mb_fo2; break; 
                case 5: add = JY_mb_fei; break; 
                case 6: add = JY_mb_fei2; break; 
                case 7: add = JY_mb_fei3; break; 
                case 8: add = JY_mb_fei4; break; 
                case 9: add = JY_mb_fou2; break; 
                case 10: add = JY_mb_fou3; break; 
                case 11: add = JY_mb_fan; break;
                case 12: add = JY_mb_fan2; break;
                case 13: add = JY_mb_fan3; break;
                case 14: add = JY_mb_fan4; break;
                case 15: add = JY_mb_fen; break;
                case 16: add = JY_mb_fen2; break;
                case 17: add = JY_mb_fen3; break;
                case 18: add = JY_mb_fen4; break;
                case 19: add = JY_mb_fang; break;
                case 20: add = JY_mb_fang2; break;
                case 21: add = JY_mb_fang3; break;
                case 22: add = JY_mb_fang4; break;
                case 23: add = JY_mb_feng; break; 
                case 24: add = JY_mb_feng2; break; 
                case 25: add = JY_mb_feng3; break; 
                case 26: add = JY_mb_feng4; break; 
                case 27: add = JY_mb_fu; break; 
                case 28: add = JY_mb_fu2; break; 
                case 29: add = JY_mb_fu3; break; 
                case 30: add = JY_mb_fu4; break; 
                }
                g_hanzi_num = (unsigned short)strlen(add); //   拼音  a 的个数
                memcpy(get_hanzi, add, g_hanzi_num);       //  把找到的汉字放在缓冲区
                *hh         = g_hanzi_num / 2;
                return 1;
            }
        }
        return 0;
    }

    //首字母为 'ㄉ' (2)
    if (jy_ime_cmp == '2')
    {
        for (jy_i = 0; jy_i < sizeofD; jy_i++)                     // 16次
        {
            memcpy(jy_ime_temp, JY_index_d[jy_i], 5);              //   ai,an,ang,ao，ei,...   16个
            jy_ime_cmp = memcmp(&jy_ime_temp1[1], jy_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (jy_ime_cmp == 0)
            {
                switch (jy_i)
                {
                case 0: add  = JY_mb_da; break;
                case 1: add  = JY_mb_da2; break;
                case 2: add  = JY_mb_da3; break;
                case 3: add  = JY_mb_da4; break;
                case 4: add  = JY_mb_de2; break; 
                case 5: add  = JY_mb_de5; break; 
                case 6: add  = JY_mb_dai; break;
                case 7: add  = JY_mb_dai3; break;
                case 8: add  = JY_mb_dai4; break;
                case 9: add  = JY_mb_dei3; break; 
                case 10: add  = JY_mb_dao; break; 
                case 11: add  = JY_mb_dao3; break; 
                case 12: add  = JY_mb_dao4; break; 
                case 13: add  = JY_mb_dou; break; 
                case 14: add  = JY_mb_dou3; break; 
                case 15: add  = JY_mb_dou4; break; 
                case 16: add  = JY_mb_dan; break; 
                case 17: add  = JY_mb_dan3; break; 
                case 18: add  = JY_mb_dan4; break; 
                case 19: add  = JY_mb_dang; break; 
                case 20: add  = JY_mb_dang3; break; 
                case 21: add  = JY_mb_dang4; break; 
                case 22: add  = JY_mb_deng; break;
                case 23: add  = JY_mb_deng3; break;
                case 24: add  = JY_mb_deng4; break;
                case 25: add  = JY_mb_di; break; 
                case 26: add  = JY_mb_di2; break; 
                case 27: add  = JY_mb_di3; break; 
                case 28: add  = JY_mb_di4; break; 
                case 29: add  = JY_mb_die; break; 
                case 30: add  = JY_mb_die2; break; 
                case 31: add  = JY_mb_diao3; break; 
                case 32: add  = JY_mb_diao4; break; 
                case 33: add  = JY_mb_diu; break; 
                case 34: add  = JY_mb_dian; break; 
                case 35: add  = JY_mb_dian3; break; 
                case 36: add  = JY_mb_dian4; break; 
                case 37: add  = JY_mb_ding; break; 
                case 38: add  = JY_mb_ding3; break; 
                case 39: add  = JY_mb_ding4; break; 
                case 40: add  = JY_mb_du; break; 
                case 41: add  = JY_mb_du2; break; 
                case 42: add  = JY_mb_du3; break; 
                case 43: add  = JY_mb_du4; break; 
                case 44: add  = JY_mb_duo; break;
                case 45: add  = JY_mb_duo2; break;
                case 46: add  = JY_mb_duo3; break;
                case 47: add  = JY_mb_duo4; break;
                case 48: add  = JY_mb_dui; break; 
                case 49: add  = JY_mb_dui4; break; 
                case 50: add  = JY_mb_duan; break; 
                case 51: add  = JY_mb_duan3; break; 
                case 52: add  = JY_mb_duan4; break; 
                case 53: add  = JY_mb_dun; break; 
                case 54: add  = JY_mb_dun3; break; 
                case 55: add  = JY_mb_dun4; break; 
                case 56: add  = JY_mb_dong; break; 
                case 57: add  = JY_mb_dong3; break; 
                case 58: add  = JY_mb_dong4; break; 
                }
                g_hanzi_num = (unsigned short)(strlen(add)); //拼音a的个数
                memcpy(get_hanzi, add, g_hanzi_num);         //把找到的汉字放在缓冲区
                *hh         = g_hanzi_num / 2;
                return 1;
            }
        }
        return 0;
    }

    //首字母为 'ㄊ' (w)
    if (jy_ime_cmp == 'w')
    {
        for (jy_i = 0; jy_i < sizeofT; jy_i++)                     // 16次
        {
            memcpy(jy_ime_temp, JY_index_t[jy_i], 5);              //   ai,an,ang,ao，ei,...   16个
            jy_ime_cmp = memcmp(&jy_ime_temp1[1], jy_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (jy_ime_cmp == 0)
            {
                switch (jy_i)
                {
                case 0: add  = JY_mb_ta; break; 
                case 1: add  = JY_mb_ta3; break; 
                case 2: add  = JY_mb_ta4; break; 
                case 3: add  = JY_mb_te2; break; 
                case 4: add  = JY_mb_te4; break;
                case 5: add  = JY_mb_tai; break; 
                case 6: add  = JY_mb_tai2; break; 
                case 7: add  = JY_mb_tai4; break; 
                case 8: add  = JY_mb_tao; break; 
                case 9: add  = JY_mb_tao2; break; 
                case 10: add  = JY_mb_tao3; break; 
                case 11: add  = JY_mb_tao4; break; 
                case 12: add  = JY_mb_tou; break; 
                case 13: add  = JY_mb_tou2; break; 
                case 14: add  = JY_mb_tou3; break; 
                case 15: add  = JY_mb_tou4; break; 
                case 16: add  = JY_mb_tan; break; 
                case 17: add  = JY_mb_tan2; break; 
                case 18: add  = JY_mb_tan3; break; 
                case 19: add  = JY_mb_tan4; break; 
                case 20: add  = JY_mb_tang; break; 
                case 21: add  = JY_mb_tang2; break; 
                case 22: add  = JY_mb_tang3; break; 
                case 23: add  = JY_mb_tang4; break; 
                case 24: add  = JY_mb_teng2; break; 
                case 25: add  = JY_mb_ti; break; 
                case 26: add  = JY_mb_ti2; break; 
                case 27: add  = JY_mb_ti3; break; 
                case 28: add  = JY_mb_ti4; break; 
                case 29: add  = JY_mb_tie; break; 
                case 30: add  = JY_mb_tie3; break; 
                case 31: add  = JY_mb_tie4; break; 
                case 32: add  = JY_mb_tiao; break; 
                case 33: add  = JY_mb_tiao2; break; 
                case 34: add  = JY_mb_tiao3; break; 
                case 35: add  = JY_mb_tiao4; break; 
                case 36: add  = JY_mb_tian; break;
                case 37: add  = JY_mb_tian2; break;
                case 38: add  = JY_mb_tian3; break;
                case 39: add  = JY_mb_tian4; break;
                case 40: add  = JY_mb_ting; break; 
                case 41: add  = JY_mb_ting2; break; 
                case 42: add  = JY_mb_ting3; break; 
                case 43: add  = JY_mb_ting4; break; 
                case 44: add  = JY_mb_tu; break;
                case 45: add  = JY_mb_tu2; break;
                case 46: add  = JY_mb_tu3; break;
                case 47: add  = JY_mb_tu4; break;
                case 48: add  = JY_mb_tuan2; break; 
                case 49: add  = JY_mb_tuan3; break; 
                case 50: add  = JY_mb_tuan4; break; 
                case 51: add  = JY_mb_tuo; break; 
                case 52: add  = JY_mb_tuo2; break; 
                case 53: add  = JY_mb_tuo3; break; 
                case 54: add  = JY_mb_tuo4; break; 
                case 55: add  = JY_mb_tui; break; 
                case 56: add  = JY_mb_tui2; break; 
                case 57: add  = JY_mb_tui3; break; 
                case 58: add  = JY_mb_tui4; break; 
                case 59: add  = JY_mb_tun; break; 
                case 60: add  = JY_mb_tun2; break; 
                case 61: add  = JY_mb_tun4; break; 
                case 62: add  = JY_mb_tong; break; 
                case 63: add  = JY_mb_tong2; break; 
                case 64: add  = JY_mb_tong3; break; 
                case 65: add  = JY_mb_tong4; break;                 
                }
                g_hanzi_num = (unsigned short)strlen(add); //   拼音  a 的个数
                memcpy(get_hanzi, add, g_hanzi_num);       //  把找到的汉字放在缓冲区
                *hh         = g_hanzi_num / 2;
                return 1;
            }
        }
        return 0;
    }

    //首字母为 'ㄋ' (s)
    if (jy_ime_cmp == 's')
    {
        for (jy_i = 0; jy_i < sizeofN; jy_i++)                     // 16次
        {
            memcpy(jy_ime_temp, JY_index_n[jy_i], 5);              //   ai,an,ang,ao，ei,...   16个
            jy_ime_cmp = memcmp(&jy_ime_temp1[1], jy_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (jy_ime_cmp == 0)
            {
                switch (jy_i)
                {               
                case 0: add  = JY_mb_na2; break; 
                case 1: add  = JY_mb_na3; break; 
                case 2: add  = JY_mb_na4; break; 
                case 3: add  = JY_mb_na5; break; 
                case 4: add  = JY_mb_ne4; break; 
                case 5: add  = JY_mb_ne5; break; 
                case 6: add  = JY_mb_nai3; break; 
                case 7: add  = JY_mb_nai4; break; 
                case 8: add  = JY_mb_nei3; break; 
                case 9: add  = JY_mb_nei4; break; 
                case 10: add  = JY_mb_nao2; break; 
                case 11: add  = JY_mb_nao3; break; 
                case 12: add  = JY_mb_nao4; break; 
                case 13: add  = JY_mb_nou4; break; 
                case 14: add  = JY_mb_nan; break; 
                case 15: add  = JY_mb_nan2; break; 
                case 16: add  = JY_mb_nan3; break; 
                case 17: add  = JY_mb_nan4; break; 
                case 18: add  = JY_mb_nen4; break;
                case 19: add  = JY_mb_nang2; break; 
                case 20: add  = JY_mb_nang3; break; 
                case 21: add  = JY_mb_neng2; break; 
                case 22: add  = JY_mb_ni2; break;
                case 23: add  = JY_mb_ni3; break;
                case 24: add  = JY_mb_ni4; break;
                case 25: add  = JY_mb_nie; break; 
                case 26: add  = JY_mb_nie4; break; 
                case 27: add  = JY_mb_niao3; break;
                case 28: add  = JY_mb_niao4; break;
                case 29: add  = JY_mb_niu; break;
                case 30: add  = JY_mb_niu2; break;
                case 31: add  = JY_mb_niu3; break;
                case 32: add  = JY_mb_niu4; break;
                case 33: add  = JY_mb_nian; break; 
                case 34: add  = JY_mb_nian2; break; 
                case 35: add  = JY_mb_nian3; break; 
                case 36: add  = JY_mb_nian4; break; 
                case 37: add  = JY_mb_nin2; break; 
                case 38: add  = JY_mb_niang2; break; 
                case 39: add  = JY_mb_niang4; break; 
                case 40: add  = JY_mb_ning2; break; 
                case 41: add  = JY_mb_ning3; break; 
                case 42: add  = JY_mb_ning4; break; 
                case 43: add  = JY_mb_nu2; break; 
                case 44: add  = JY_mb_nu3; break; 
                case 45: add  = JY_mb_nu4; break; 
                case 46: add  = JY_mb_nuo2; break; 
                case 47: add  = JY_mb_nuo4; break; 
                case 48: add  = JY_mb_nuan3; break; 
                case 49: add  = JY_mb_nong2; break; 
                case 50: add  = JY_mb_nong4; break; 
                case 51: add  = JY_mb_nv3; break;
                case 52: add  = JY_mb_nue4; break;
                }
                g_hanzi_num = (unsigned short)strlen(add); //   拼音  a 的个数
                memcpy(get_hanzi, add, g_hanzi_num);       //  把找到的汉字放在缓冲区
                *hh         = g_hanzi_num / 2;
                return 1;
            }
        }
        return 0;
    }

    //首字母为 'ㄌ' (x)
    if (jy_ime_cmp == 'x')
    {
        for (jy_i = 0; jy_i < sizeofL; jy_i++)                     // 16次
        {
            memcpy(jy_ime_temp, JY_index_l[jy_i], 5);              //   ai,an,ang,ao，ei,...   16个
            jy_ime_cmp = memcmp(&jy_ime_temp1[1], jy_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (jy_ime_cmp == 0)
            {
                switch (jy_i)
                {
                case 0: add  = JY_mb_la; break; 
                case 1: add  = JY_mb_la2; break; 
                case 2: add  = JY_mb_la3; break; 
                case 3: add  = JY_mb_la4; break; 
                case 4: add  = JY_mb_la5; break; 
                case 5: add  = JY_mb_le4; break; 
                case 6: add  = JY_mb_le5; break; 
                case 7: add  = JY_mb_lai2; break; 
                case 8: add  = JY_mb_lai4; break; 
                case 9: add  = JY_mb_lei; break; 
                case 10: add  = JY_mb_lei2; break; 
                case 11: add  = JY_mb_lei3; break; 
                case 12: add  = JY_mb_lei4; break; 
                case 13: add  = JY_mb_lao; break; 
                case 14: add  = JY_mb_lao2; break; 
                case 15: add  = JY_mb_lao3; break; 
                case 16: add  = JY_mb_lao4; break; 
                case 17: add  = JY_mb_lan2; break; 
                case 18: add  = JY_mb_lan3; break; 
                case 19: add  = JY_mb_lan4; break; 
                case 20: add  = JY_mb_lang2; break; 
                case 21: add  = JY_mb_lang3; break; 
                case 22: add  = JY_mb_lang4; break; 
                case 23: add  = JY_mb_leng2; break; 
                case 24: add  = JY_mb_leng3; break; 
                case 25: add  = JY_mb_leng4; break;
                case 26: add  = JY_mb_li; break;
                case 27: add  = JY_mb_li2; break;
                case 28: add  = JY_mb_li3; break;
                case 29: add  = JY_mb_li4; break;
                case 30: add  = JY_mb_li5; break;
                case 31: add  = JY_mb_lie; break; 
                case 32: add  = JY_mb_lie3; break; 
                case 33: add  = JY_mb_lie4; break; 
                case 34: add  = JY_mb_liao; break;
                case 35: add  = JY_mb_liao2; break;
                case 36: add  = JY_mb_liao3; break;
                case 37: add  = JY_mb_liao4; break;
                case 38: add  = JY_mb_liu; break; 
                case 39: add  = JY_mb_liu2; break; 
                case 40: add  = JY_mb_liu3; break; 
                case 41: add  = JY_mb_liu4; break; 
                case 42: add  = JY_mb_lian2; break; 
                case 43: add  = JY_mb_lian3; break; 
                case 44: add  = JY_mb_lian4; break; 
                case 45: add  = JY_mb_lin2; break;
                case 46: add  = JY_mb_lin3; break;
                case 47: add  = JY_mb_lin4; break;
                case 48: add  = JY_mb_liang2; break; 
                case 49: add  = JY_mb_liang3; break; 
                case 50: add  = JY_mb_liang4; break; 
                case 51: add  = JY_mb_ling2; break; 
                case 52: add  = JY_mb_ling3; break; 
                case 53: add  = JY_mb_ling4; break; 
                case 54: add  = JY_mb_lou; break; 
                case 55: add  = JY_mb_lou2; break; 
                case 56: add  = JY_mb_lou3; break; 
                case 57: add  = JY_mb_lou4; break; 
                case 58: add  = JY_mb_lu; break; 
                case 59: add  = JY_mb_lu2; break; 
                case 60: add  = JY_mb_lu3; break; 
                case 61: add  = JY_mb_lu4; break; 
                case 62: add  = JY_mb_luo; break; 
                case 63: add  = JY_mb_luo2; break; 
                case 64: add  = JY_mb_luo3; break; 
                case 65: add  = JY_mb_luo4; break; 
                case 66: add  = JY_mb_luan2; break;
                case 67: add  = JY_mb_luan3; break;
                case 68: add  = JY_mb_luan4; break;
                case 69: add  = JY_mb_lun; break; 
                case 70: add  = JY_mb_lun2; break; 
                case 71: add  = JY_mb_lun4; break; 
                case 72: add  = JY_mb_long; break;
                case 73: add  = JY_mb_long2; break;
                case 74: add  = JY_mb_long3; break;
                case 75: add  = JY_mb_long4; break;
                case 76: add  = JY_mb_lv2; break; 
                case 77: add  = JY_mb_lv3; break; 
                case 78: add  = JY_mb_lv4; break; 
                case 79: add  = JY_mb_lue4; break;                 
                }
                g_hanzi_num = (unsigned short)strlen(add); //   拼音  a 的个数
                memcpy(get_hanzi, add, g_hanzi_num);       //  把找到的汉字放在缓冲区
                *hh         = g_hanzi_num / 2;
                return 1;
            }
        }
        return 0;
    }

    //首字母为 'ㄍ' (e)
    if (jy_ime_cmp == 'e')
    {
        for (jy_i = 0; jy_i < sizeofG; jy_i++)                     // 16次
        {
            memcpy(jy_ime_temp, JY_index_g[jy_i], 5);              //   ai,an,ang,ao，ei,...   16个
            jy_ime_cmp = memcmp(&jy_ime_temp1[1], jy_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (jy_ime_cmp == 0)
            {
                printf("jy_i==%d\n", jy_i);
                switch (jy_i)
                {
                case 0: add  = JY_mb_ga2; break; 
                case 1: add  = JY_mb_ga3; break; 
                case 2: add  = JY_mb_ga4; break; 
                case 3: add  = JY_mb_ge; break;
                case 4: add  = JY_mb_ge2; break;
                case 5: add  = JY_mb_ge3; break;
                case 6: add  = JY_mb_ge4; break;
                case 7: add  = JY_mb_gai; break; 
                case 8: add  = JY_mb_gai3; break; 
                case 9: add  = JY_mb_gai4; break; 
                case 10: add  = JY_mb_gei3; break; 
                case 11: add  = JY_mb_gao; break; 
                case 12: add  = JY_mb_gao3; break;
                case 13: add  = JY_mb_gao4; break; 
                case 14: add  = JY_mb_gou; break; 
                case 15: add  = JY_mb_gou3; break; 
                case 16: add  = JY_mb_gou4; break; 
                case 17: add  = JY_mb_gan; break;
                case 18: add  = JY_mb_gan3; break;
                case 19: add  = JY_mb_gan4; break;
                case 20: add  = JY_mb_gen; break; 
                case 21: add  = JY_mb_gen4; break; 
                case 22: add  = JY_mb_gang; break; 
                case 23: add  = JY_mb_gang3; break; 
                case 24: add  = JY_mb_gang4; break; 
                case 25: add  = JY_mb_geng; break; 
                case 26: add  = JY_mb_geng3; break; 
                case 27: add  = JY_mb_geng4; break; 
                case 28: add  = JY_mb_gu; break; 
                case 29: add  = JY_mb_gu3; break; 
                case 30: add  = JY_mb_gu4; break; 
                case 31: add  = JY_mb_gua; break;
                case 32: add  = JY_mb_gua3; break;
                case 33: add  = JY_mb_gua4; break;
                case 34: add  = JY_mb_guo; break; 
                case 35: add  = JY_mb_guo2; break; 
                case 36: add  = JY_mb_guo3; break; 
                case 37: add  = JY_mb_guo4; break; 
                case 38: add  = JY_mb_guai; break; 
                case 39: add  = JY_mb_guai3; break; 
                case 40: add  = JY_mb_guai4; break; 
                case 41: add  = JY_mb_gui; break; 
                case 42: add  = JY_mb_gui3; break; 
                case 43: add  = JY_mb_gui4; break; 
                case 44: add  = JY_mb_guan; break; 
                case 45: add  = JY_mb_guan3; break; 
                case 46: add  = JY_mb_guan4; break; 
                case 47: add  = JY_mb_gun3; break; 
                case 48: add  = JY_mb_gun4; break; 
                case 49: add  = JY_mb_guang; break; 
                case 50: add  = JY_mb_guang3; break; 
                case 51: add  = JY_mb_guang4; break; 
                case 52: add  = JY_mb_gong; break;
                case 53: add  = JY_mb_gong3; break;
                case 54: add  = JY_mb_gong4; break;                
                }
                g_hanzi_num = (unsigned short)strlen(add); //   拼音  a 的个数
                memcpy(get_hanzi, add, g_hanzi_num);       //  把找到的汉字放在缓冲区
                *hh         = g_hanzi_num / 2;
                return 1;
            }
        }
        return 0;
    }

    //首字母为 'ㄎ'
    if (jy_ime_cmp == 'd')
    {
        for (jy_i = 0; jy_i < sizeofK; jy_i++)                     // 16次
        {
            memcpy(jy_ime_temp, JY_index_k[jy_i], 5);              //   ai,an,ang,ao，ei,...   16个
            jy_ime_cmp = memcmp(&jy_ime_temp1[1], jy_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (jy_ime_cmp == 0)
            {
                switch (jy_i)
                {
                case 0: add = JY_mb_ka; break; 
                case 1: add = JY_mb_ka3; break; 
                case 2: add = JY_mb_ke; break; 
                case 3: add = JY_mb_ke2; break; 
                case 4: add = JY_mb_ke3; break; 
                case 5: add = JY_mb_ke4; break; 
                case 6: add = JY_mb_kai; break; 
                case 7: add = JY_mb_kai3; break; 
                case 8: add = JY_mb_kai4; break; 
                case 9: add = JY_mb_kao; break; 
                case 10: add = JY_mb_kao3; break; 
                case 11: add = JY_mb_kao4; break; 
                case 12: add = JY_mb_kou; break; 
                case 13: add = JY_mb_kou3; break; 
                case 14: add = JY_mb_kou4; break; 
                case 15: add = JY_mb_kan; break; 
                case 16: add = JY_mb_kan3; break; 
                case 17: add = JY_mb_kan4; break; 
                case 18: add = JY_mb_ken3; break; 
                case 19: add = JY_mb_ken4; break; 
                case 20: add = JY_mb_kang; break; 
                case 21: add = JY_mb_kang2; break; 
                case 22: add = JY_mb_kang4; break; 
                case 23: add = JY_mb_keng; break; 
                case 24: add = JY_mb_ku; break; 
                case 25: add = JY_mb_ku3; break; 
                case 26: add = JY_mb_ku4; break; 
                case 27: add = JY_mb_kua; break; 
                case 28: add = JY_mb_kua3; break; 
                case 29: add = JY_mb_kua4; break; 
                case 30: add = JY_mb_kuo4; break; 
                case 31: add = JY_mb_kuai; break; 
                case 32: add = JY_mb_kuai3; break; 
                case 33: add = JY_mb_kuai4; break; 
                case 34: add = JY_mb_kui; break; 
                case 35: add = JY_mb_kui2; break; 
                case 36: add = JY_mb_kui3; break; 
                case 37: add = JY_mb_kui4; break; 
                case 38: add = JY_mb_kuan; break; 
                case 39: add = JY_mb_kuan3; break; 
                case 40: add = JY_mb_kun; break; 
                case 41: add = JY_mb_kun3; break; 
                case 42: add = JY_mb_kun4; break; 
                case 43: add = JY_mb_kuang; break; 
                case 44: add = JY_mb_kuang2; break; 
                case 45: add = JY_mb_kuang4; break; 
                case 46: add = JY_mb_kong; break; 
                case 47: add = JY_mb_kong3; break; 
                case 48: add = JY_mb_kong4; break; 
                }
                g_hanzi_num = (unsigned short)strlen(add); //   拼音  a 的个数
                memcpy(get_hanzi, add, g_hanzi_num);       //  把找到的汉字放在缓冲区
                *hh         = g_hanzi_num / 2;
                return 1;
            }
        }
        return 0;
    }

    //首字母为 'ㄏ'
    if (jy_ime_cmp == 'c')
    {
        for (jy_i = 0; jy_i < sizeofH; jy_i++)                     // 16次
        {
            memcpy(jy_ime_temp, JY_index_h[jy_i], 5);              //   ai,an,ang,ao，ei,...   16个
            jy_ime_cmp = memcmp(&jy_ime_temp1[1], jy_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (jy_ime_cmp == 0)
            {
                switch (jy_i)
                {
                case 0: add = JY_mb_ha; break; 
                case 1: add = JY_mb_ha2; break; 
                case 2: add = JY_mb_ha4; break; 
                case 3: add = JY_mb_he; break; 
                case 4: add = JY_mb_he2; break; 
                case 5: add = JY_mb_he4; break; 
                case 6: add = JY_mb_hai; break; 
                case 7: add = JY_mb_hai2; break; 
                case 8: add = JY_mb_hai3; break; 
                case 9: add = JY_mb_hai4; break; 
                case 10: add = JY_mb_hei; break;
                case 11: add = JY_mb_hao; break; 
                case 12: add = JY_mb_hao2; break; 
                case 13: add = JY_mb_hao3; break; 
                case 14: add = JY_mb_hao4; break; 
                case 15: add = JY_mb_hou; break; 
                case 16: add = JY_mb_hou2; break; 
                case 17: add = JY_mb_hou3; break; 
                case 18: add = JY_mb_hou4; break; 
                case 19: add = JY_mb_han; break; 
                case 20: add = JY_mb_han2; break; 
                case 21: add = JY_mb_han3; break; 
                case 22: add = JY_mb_han4; break; 
                case 23: add = JY_mb_hen2; break; 
                case 24: add = JY_mb_hen3; break; 
                case 25: add = JY_mb_hen4; break; 
                case 26: add = JY_mb_hang; break; 
                case 27: add = JY_mb_hang2; break; 
                case 28: add = JY_mb_heng; break; 
                case 29: add = JY_mb_heng2; break; 
                case 30: add = JY_mb_heng4; break; 
                case 31: add = JY_mb_hu; break; 
                case 32: add = JY_mb_hu2; break; 
                case 33: add = JY_mb_hu3; break; 
                case 34: add = JY_mb_hu4; break; 
                case 35: add = JY_mb_hua; break; 
                case 36: add = JY_mb_hua2; break; 
                case 37: add = JY_mb_hua4; break; 
                case 38: add = JY_mb_huo; break; 
                case 39: add = JY_mb_huo2; break; 
                case 40: add = JY_mb_huo3; break; 
                case 41: add = JY_mb_huo4; break; 
                case 42: add = JY_mb_huai2; break; 
                case 43: add = JY_mb_huai4; break; 
                case 44: add = JY_mb_hui; break; 
                case 45: add = JY_mb_hui2; break; 
                case 46: add = JY_mb_hui3; break; 
                case 47: add = JY_mb_hui4; break; 
                case 48: add = JY_mb_huan; break; 
                case 49: add = JY_mb_huan2; break; 
                case 50: add = JY_mb_huan3; break; 
                case 51: add = JY_mb_huan4; break; 
                case 52: add = JY_mb_hun; break; 
                case 53: add = JY_mb_hun2; break; 
                case 54: add = JY_mb_hun4; break; 
                case 55: add = JY_mb_huang; break; 
                case 56: add = JY_mb_huang2; break; 
                case 57: add = JY_mb_huang3; break; 
                case 58: add = JY_mb_huang4; break; 
                case 59: add = JY_mb_hong; break; 
                case 60: add = JY_mb_hong2; break; 
                case 61: add = JY_mb_hong3; break; 
                case 62: add = JY_mb_hong4; break; 
                }
                g_hanzi_num = (unsigned short)strlen(add); //   拼音  a 的个数
                memcpy(get_hanzi, add, g_hanzi_num);       //  把找到的汉字放在缓冲区
                *hh         = g_hanzi_num / 2;
                return 1;
            }
        }
        return 0;
    }

    //首字母为 'ㄐ'
    if (jy_ime_cmp == 'r')
    {
        for (jy_i = 0; jy_i < sizeofJ; jy_i++)                     // 16次
        {
            memcpy(jy_ime_temp, JY_index_j[jy_i], 5);              //   ai,an,ang,ao，ei,...   16个
            jy_ime_cmp = memcmp(&jy_ime_temp1[1], jy_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (jy_ime_cmp == 0)
            {
                switch (jy_i)
                {
                case 0: add = JY_mb_ji; break; 
                case 1: add = JY_mb_ji2; break; 
                case 2: add = JY_mb_ji3; break; 
                case 3: add = JY_mb_ji4; break; 
                case 4: add = JY_mb_jia; break; 
                case 5: add = JY_mb_jia2; break; 
                case 6: add = JY_mb_jia3; break; 
                case 7: add = JY_mb_jia4; break; 
                case 8: add = JY_mb_jie; break;
                case 9: add = JY_mb_jie2; break;
                case 10: add = JY_mb_jie3; break;
                case 11: add = JY_mb_jie4; break;
                case 12: add = JY_mb_jiao; break; 
                case 13: add = JY_mb_jiao2; break; 
                case 14: add = JY_mb_jiao3; break; 
                case 15: add = JY_mb_jiao4; break; 
                case 16: add = JY_mb_jiu; break; 
                case 17: add = JY_mb_jiu3; break; 
                case 18: add = JY_mb_jiu4; break; 
                case 19: add = JY_mb_jian; break; 
                case 20: add = JY_mb_jian3; break; 
                case 21: add = JY_mb_jian4; break; 
                case 22: add = JY_mb_jin; break; 
                case 23: add = JY_mb_jin3; break; 
                case 24: add = JY_mb_jin4; break; 
                case 25: add = JY_mb_jing; break; 
                case 26: add = JY_mb_jing3; break; 
                case 27: add = JY_mb_jing4; break; 
                case 28: add = JY_mb_jiang; break;
                case 29: add = JY_mb_jiang3; break;
                case 30: add = JY_mb_jiang4; break;
                case 31: add = JY_mb_ju; break; 
                case 32: add = JY_mb_ju2; break; 
                case 33: add = JY_mb_ju3; break; 
                case 34: add = JY_mb_ju4; break; 
                case 35: add = JY_mb_jue; break; 
                case 36: add = JY_mb_jue2; break; 
                case 37: add = JY_mb_juan; break; 
                case 38: add = JY_mb_juan3; break; 
                case 39: add = JY_mb_juan4; break; 
                case 40: add = JY_mb_jun; break; 
                case 41: add = JY_mb_jun4; break; 
                case 42: add = JY_mb_jiong; break; 
                case 43: add = JY_mb_jiong3; break; 
                }
                g_hanzi_num = (unsigned short)strlen(add); //   拼音  a 的个数
                memcpy(get_hanzi, add, g_hanzi_num);       //  把找到的汉字放在缓冲区
                *hh         = g_hanzi_num / 2;
                return 1;
            }
        }
        return 0;
    }

    //首字母为 'ㄑ'
    if (jy_ime_cmp == 'f')
    {
        for (jy_i = 0; jy_i < sizeofQ; jy_i++)                     // 16次
        {
            memcpy(jy_ime_temp, JY_index_q[jy_i], 5);              //   ai,an,ang,ao，ei,...   16个
            jy_ime_cmp = memcmp(&jy_ime_temp1[1], jy_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (jy_ime_cmp == 0)
            {
                switch (jy_i)
                {
                case 0: add = JY_mb_qi; break; 
                case 1: add = JY_mb_qi2; break; 
                case 2: add = JY_mb_qi3; break; 
                case 3: add = JY_mb_qi4; break; 
                case 4: add = JY_mb_qia; break; 
                case 5: add = JY_mb_qia3; break; 
                case 6: add = JY_mb_qia4; break; 
                case 7: add = JY_mb_qie; break; 
                case 8: add = JY_mb_qie2; break; 
                case 9: add = JY_mb_qie3; break; 
                case 10: add = JY_mb_qie4; break; 
                case 11: add = JY_mb_qiao; break; 
                case 12: add = JY_mb_qiao2; break; 
                case 13: add = JY_mb_qiao3; break; 
                case 14: add = JY_mb_qiao4; break; 
                case 15: add = JY_mb_qiu; break; 
                case 16: add = JY_mb_qiu2; break; 
                case 17: add = JY_mb_qiu3; break; 
                case 18: add = JY_mb_qian; break; 
                case 19: add = JY_mb_qian2; break; 
                case 20: add = JY_mb_qian3; break; 
                case 21: add = JY_mb_qian4; break; 
                case 22: add = JY_mb_qin; break; 
                case 23: add = JY_mb_qin2; break; 
                case 24: add = JY_mb_qin3; break; 
                case 25: add = JY_mb_qin4; break; 
                case 26: add = JY_mb_qiang; break; 
                case 27: add = JY_mb_qiang2; break; 
                case 28: add = JY_mb_qiang3; break; 
                case 29: add = JY_mb_qiang4; break; 
                case 30: add = JY_mb_qing; break; 
                case 31: add = JY_mb_qing2; break; 
                case 32: add = JY_mb_qing3; break; 
                case 33: add = JY_mb_qing4; break; 
                case 34: add = JY_mb_qu; break; 
                case 35: add = JY_mb_qu2; break; 
                case 36: add = JY_mb_qu3; break; 
                case 37: add = JY_mb_qu4; break; 
                case 38: add = JY_mb_que; break; 
                case 39: add = JY_mb_que2; break; 
                case 40: add = JY_mb_que4; break; 
                case 41: add = JY_mb_quan; break; 
                case 42: add = JY_mb_quan2; break; 
                case 43: add = JY_mb_quan3; break; 
                case 44: add = JY_mb_quan4; break; 
                case 45: add = JY_mb_qun; break; 
                case 46: add = JY_mb_qun2; break; 
                case 47: add = JY_mb_qiong; break; 
                case 48: add = JY_mb_qiong2; break; 
                }
                g_hanzi_num = (unsigned short)strlen(add); //   拼音  a 的个数
                memcpy(get_hanzi, add, g_hanzi_num);       //  把找到的汉字放在缓冲区
                *hh         = g_hanzi_num / 2;
                return 1;
            }
        }
        return 0;
    }
    
    //首字母为 'ㄒ'
    if (jy_ime_cmp == 'v')
    {
        for (jy_i = 0; jy_i < sizeofX; jy_i++)                     // 16次
        {
            memcpy(jy_ime_temp, JY_index_x[jy_i], 5);              //   ai,an,ang,ao，ei,...   16个
            jy_ime_cmp = memcmp(&jy_ime_temp1[1], jy_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (jy_ime_cmp == 0)
            {
                switch (jy_i)
                {
                case 0: add = JY_mb_xi; break; 
                case 1: add = JY_mb_xi2; break; 
                case 2: add = JY_mb_xi3; break; 
                case 3: add = JY_mb_xi4; break; 
                case 4: add = JY_mb_xia; break; 
                case 5: add = JY_mb_xia2; break; 
                case 6: add = JY_mb_xia4; break; 
                case 7: add = JY_mb_xie; break; 
                case 8: add = JY_mb_xie2; break; 
                case 9: add = JY_mb_xie3; break; 
                case 10: add = JY_mb_xie4; break; 
                case 11: add = JY_mb_xiao; break; 
                case 12: add = JY_mb_xiao3; break; 
                case 13: add = JY_mb_xiao4; break; 
                case 14: add = JY_mb_xiu; break; 
                case 15: add = JY_mb_xiu3; break; 
                case 16: add = JY_mb_xiu4; break; 
                case 17: add = JY_mb_xian; break; 
                case 18: add = JY_mb_xian2; break; 
                case 19: add = JY_mb_xian3; break; 
                case 20: add = JY_mb_xian4; break; 
                case 21: add = JY_mb_xin; break;
                case 22: add = JY_mb_xin2; break;
                case 23: add = JY_mb_xin3; break;
                case 24: add = JY_mb_xin4; break;
                case 25: add = JY_mb_xiang; break; 
                case 26: add = JY_mb_xiang2; break; 
                case 27: add = JY_mb_xiang3; break; 
                case 28: add = JY_mb_xiang4; break; 
                case 29: add = JY_mb_xing; break; 
                case 30: add = JY_mb_xing2; break; 
                case 31: add = JY_mb_xing3; break; 
                case 32: add = JY_mb_xing4; break; 
                case 33: add = JY_mb_xu; break; 
                case 34: add = JY_mb_xu2; break; 
                case 35: add = JY_mb_xu3; break; 
                case 36: add = JY_mb_xu4; break; 
                case 37: add = JY_mb_xue; break; 
                case 38: add = JY_mb_xue2; break; 
                case 39: add = JY_mb_xue3; break; 
                case 40: add = JY_mb_xue4; break; 
                case 41: add = JY_mb_xuan; break; 
                case 42: add = JY_mb_xuan2; break; 
                case 43: add = JY_mb_xuan3; break; 
                case 44: add = JY_mb_xuan4; break; 
                case 45: add = JY_mb_xun; break; 
                case 46: add = JY_mb_xun2; break; 
                case 47: add = JY_mb_xun4; break; 
                case 48: add = JY_mb_xiong; break; 
                case 49: add = JY_mb_xiong2; break; 
                case 50: add = JY_mb_xiong4; break; 
                }
                g_hanzi_num = (unsigned short)strlen(add); //   拼音  a 的个数
                memcpy(get_hanzi, add, g_hanzi_num);       //  把找到的汉字放在缓冲区
                *hh         = g_hanzi_num / 2;
                return 1;
            }
        }
        return 0;
    }

    //首字母为 'ㄓ'
    if (jy_ime_cmp == '5')
    {
        for (jy_i = 0; jy_i < sizeofZH; jy_i++)                     // 16次
        {
            memcpy(jy_ime_temp, JY_index_zh[jy_i], 5);              //   ai,an,ang,ao，ei,...   16个
            jy_ime_cmp = memcmp(&jy_ime_temp1[1], jy_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (jy_ime_cmp == 0)
            {
                switch (jy_i)
                {
                case 0: add = JY_mb_zhi; break;
                case 1: add = JY_mb_zhi2; break;
                case 2: add = JY_mb_zhi3; break;
                case 3: add = JY_mb_zhi4; break;
                case 4: add = JY_mb_zha; break; 
                case 5: add = JY_mb_zha2; break; 
                case 6: add = JY_mb_zha3; break; 
                case 7: add = JY_mb_zha4; break; 
                case 8: add = JY_mb_zhe; break; 
                case 9: add = JY_mb_zhe2; break; 
                case 10: add = JY_mb_zhe3; break; 
                case 11: add = JY_mb_zhe4; break; 
                case 12: add = JY_mb_zhai; break; 
                case 13: add = JY_mb_zhai2; break; 
                case 14: add = JY_mb_zhai3; break; 
                case 15: add = JY_mb_zhai4; break; 
                case 16: add = JY_mb_zhao; break; 
                case 17: add = JY_mb_zhao2; break; 
                case 18: add = JY_mb_zhao3; break; 
                case 19: add = JY_mb_zhao4; break; 
                case 20: add = JY_mb_zhou; break; 
                case 21: add = JY_mb_zhou2; break; 
                case 22: add = JY_mb_zhou3; break; 
                case 23: add = JY_mb_zhou4; break; 
                case 24: add = JY_mb_zhan; break;
                case 25: add = JY_mb_zhan3; break;
                case 26: add = JY_mb_zhan4; break;
                case 27: add = JY_mb_zhen; break;
                case 28: add = JY_mb_zhen3; break;
                case 29: add = JY_mb_zhen4; break;
                case 30: add = JY_mb_zhang; break;
                case 31: add = JY_mb_zhang3; break;
                case 32: add = JY_mb_zhang4; break;
                case 33: add = JY_mb_zheng; break; 
                case 34: add = JY_mb_zheng3; break; 
                case 35: add = JY_mb_zheng4; break; 
                case 36: add = JY_mb_zhu; break; 
                case 37: add = JY_mb_zhu2; break; 
                case 38: add = JY_mb_zhu3; break; 
                case 39: add = JY_mb_zhu4; break; 
                case 40: add = JY_mb_zhua; break; 
                case 41: add = JY_mb_zhua3; break; 
                case 42: add = JY_mb_zhuo; break; 
                case 43: add = JY_mb_zhuo2; break; 
                case 44: add = JY_mb_zhuai; break; 
                case 45: add = JY_mb_zhuai3; break; 
                case 46: add = JY_mb_zhuai4; break; 
                case 47: add = JY_mb_zhuang; break; 
                case 48: add = JY_mb_zhuang4; break; 
                case 49: add = JY_mb_zhui; break; 
                case 50: add = JY_mb_zhui4; break; 
                case 51: add = JY_mb_zhuan; break; 
                case 52: add = JY_mb_zhuan3; break; 
                case 53: add = JY_mb_zhuan4; break; 
                case 54: add = JY_mb_zhun; break; 
                case 55: add = JY_mb_zhun3; break; 
                case 56: add = JY_mb_zhong; break; 
                case 57: add = JY_mb_zhong3; break; 
                case 58: add = JY_mb_zhong4; break; 

                }
                g_hanzi_num = (unsigned short)strlen(add); //   拼音  a 的个数
                memcpy(get_hanzi, add, g_hanzi_num);       //  把找到的汉字放在缓冲区
                *hh         = g_hanzi_num / 2;
                return 1;
            }
        }
        return 0;
    }

    //首字母为 'ㄔ'
    if (jy_ime_cmp == 't')
    {
        for (jy_i = 0; jy_i < sizeofCH; jy_i++)                     // 16次
        {
            memcpy(jy_ime_temp, JY_index_ch[jy_i], 5);              //   ai,an,ang,ao，ei,...   16个
            jy_ime_cmp = memcmp(&jy_ime_temp1[1], jy_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (jy_ime_cmp == 0)
            {
                switch (jy_i)
                {
                case 0: add = JY_mb_chi; break; 
                case 1: add = JY_mb_chi2; break; 
                case 2: add = JY_mb_chi3; break; 
                case 3: add = JY_mb_chi4; break; 
                case 4: add = JY_mb_cha; break; 
                case 5: add = JY_mb_cha2; break; 
                case 6: add = JY_mb_cha3; break; 
                case 7: add = JY_mb_cha4; break; 
                case 8: add = JY_mb_che; break; 
                case 9: add = JY_mb_che3; break; 
                case 10: add = JY_mb_che4; break; 
                case 11: add = JY_mb_chai; break; 
                case 12: add = JY_mb_chai2; break; 
                case 13: add = JY_mb_chai4; break; 
                case 14: add = JY_mb_chao; break; 
                case 15: add = JY_mb_chao2; break; 
                case 16: add = JY_mb_chao3; break; 
                case 17: add = JY_mb_chou; break;
                case 18: add = JY_mb_chou2; break;
                case 19: add = JY_mb_chou3; break;
                case 20: add = JY_mb_chou4; break;
                case 21: add = JY_mb_chan; break; 
                case 22: add = JY_mb_chan2; break; 
                case 23: add = JY_mb_chan3; break; 
                case 24: add = JY_mb_chan4; break; 
                case 25: add = JY_mb_chen; break; 
                case 26: add = JY_mb_chen2; break; 
                case 27: add = JY_mb_chen4; break; 
                case 28: add = JY_mb_chang; break; 
                case 29: add = JY_mb_chang2; break; 
                case 30: add = JY_mb_chang3; break; 
                case 31: add = JY_mb_chang4; break; 
                case 32: add = JY_mb_cheng; break; 
                case 33: add = JY_mb_cheng2; break; 
                case 34: add = JY_mb_cheng3; break; 
                case 35: add = JY_mb_cheng4; break; 
                case 36: add = JY_mb_chu; break; 
                case 37: add = JY_mb_chu2; break; 
                case 38: add = JY_mb_chu3; break; 
                case 39: add = JY_mb_chu4; break; 
                case 40: add = JY_mb_chuo; break; 
                case 41: add = JY_mb_chuo4; break; 
                case 42: add = JY_mb_chuai3; break; 
                case 43: add = JY_mb_chuai4; break; 
                case 44: add = JY_mb_chui; break; 
                case 45: add = JY_mb_chui2; break; 
                case 46: add = JY_mb_chuan; break; 
                case 47: add = JY_mb_chuan2; break; 
                case 48: add = JY_mb_chuan3; break; 
                case 49: add = JY_mb_chuan4; break; 
                case 50: add = JY_mb_chuang; break; 
                case 51: add = JY_mb_chuang2; break; 
                case 52: add = JY_mb_chuang3; break; 
                case 53: add = JY_mb_chuang4; break; 
                case 54: add = JY_mb_chun; break; 
                case 55: add = JY_mb_chun2; break; 
                case 56: add = JY_mb_chun3; break; 
                case 57: add = JY_mb_chong; break; 
                case 58: add = JY_mb_chong2; break; 
                case 59: add = JY_mb_chong3; break; 
                case 60: add = JY_mb_chong4; break; 

                }
                g_hanzi_num = (unsigned short)strlen(add); //   拼音  a 的个数
                memcpy(get_hanzi, add, g_hanzi_num);       //  把找到的汉字放在缓冲区
                *hh         = g_hanzi_num / 2;
                return 1;
            }
        }
        return 0;
    }
    
    //首字母为 'ㄕ'
    if (jy_ime_cmp == 'g')
    {
        for (jy_i = 0; jy_i < sizeofSH; jy_i++)                     // 16次
        {
            memcpy(jy_ime_temp, JY_index_sh[jy_i], 5);              //   ai,an,ang,ao，ei,...   16个
            jy_ime_cmp = memcmp(&jy_ime_temp1[1], jy_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (jy_ime_cmp == 0)
            {
                switch (jy_i)
                {
                case 0: add = JY_mb_shi; break; 
                case 1: add = JY_mb_shi2; break; 
                case 2: add = JY_mb_shi3; break; 
                case 3: add = JY_mb_shi4; break; 
                case 4: add = JY_mb_sha; break; 
                case 5: add = JY_mb_sha2; break; 
                case 6: add = JY_mb_sha3; break; 
                case 7: add = JY_mb_sha4; break; 
                case 8: add = JY_mb_she; break; 
                case 9: add = JY_mb_she2; break; 
                case 10: add = JY_mb_she3; break; 
                case 11: add = JY_mb_she4; break; 
                case 12: add = JY_mb_shai; break;
                case 13: add = JY_mb_shai3; break;
                case 14: add = JY_mb_shai4; break;
                case 15: add = JY_mb_shei2; break;
                case 16: add = JY_mb_shao; break; 
                case 17: add = JY_mb_shao2; break; 
                case 18: add = JY_mb_shao3; break; 
                case 19: add = JY_mb_shao4; break; 
                case 20: add = JY_mb_shou; break; 
                case 21: add = JY_mb_shou3; break; 
                case 22: add = JY_mb_shou4; break; 
                case 23: add = JY_mb_shan; break; 
                case 24: add = JY_mb_shan3; break; 
                case 25: add = JY_mb_shan4; break; 
                case 26: add = JY_mb_shen; break;
                case 27: add = JY_mb_shen2; break;
                case 28: add = JY_mb_shen3; break;
                case 29: add = JY_mb_shen4; break;
                case 30: add = JY_mb_shang; break; 
                case 31: add = JY_mb_shang3; break; 
                case 32: add = JY_mb_shang4; break; 
                case 33: add = JY_mb_sheng; break;
                case 34: add = JY_mb_sheng2; break;
                case 35: add = JY_mb_sheng3; break;
                case 36: add = JY_mb_sheng4; break;
                case 37: add = JY_mb_shu; break;
                case 38: add = JY_mb_shu2; break;
                case 39: add = JY_mb_shu3; break;
                case 40: add = JY_mb_shu4; break;
                case 41: add = JY_mb_shua; break; 
                case 42: add = JY_mb_shua3; break; 
                case 43: add = JY_mb_shua4; break; 
                case 44: add = JY_mb_shuo; break; 
                case 45: add = JY_mb_shuo4; break; 
                case 46: add = JY_mb_shuai; break;
                case 47: add = JY_mb_shuai3; break;
                case 48: add = JY_mb_shuai4; break;
                case 49: add = JY_mb_shui2; break; 
                case 50: add = JY_mb_shui3; break; 
                case 51: add = JY_mb_shui4; break; 
                case 52: add = JY_mb_shuan; break; 
                case 53: add = JY_mb_shuan4; break; 
                case 54: add = JY_mb_shun3; break; 
                case 55: add = JY_mb_shun4; break; 
                case 56: add = JY_mb_shuang; break; 
                case 57: add = JY_mb_shuang3; break; 
                }
                g_hanzi_num = (unsigned short)strlen(add); //   拼音  a 的个数
                memcpy(get_hanzi, add, g_hanzi_num);       //  把找到的汉字放在缓冲区
                *hh         = g_hanzi_num / 2;
                return 1;
            }
        }
        return 0;
    }
    
    //首字母为 'ㄖ'
    if (jy_ime_cmp == 'b')
    {
        for (jy_i = 0; jy_i < sizeofR; jy_i++)                     // 16次
        {
            memcpy(jy_ime_temp, JY_index_r[jy_i], 5);              //   ai,an,ang,ao，ei,...   16个
            jy_ime_cmp = memcmp(&jy_ime_temp1[1], jy_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (jy_ime_cmp == 0)
            {
                switch (jy_i)
                {
                case 0: add = JY_mb_ri4; break; 
                case 1: add = JY_mb_re3; break; 
                case 2: add = JY_mb_re4; break; 
                case 3: add = JY_mb_rao2; break; 
                case 4: add = JY_mb_rao3; break; 
                case 5: add = JY_mb_rao4; break; 
                case 6: add = JY_mb_rou2; break;
                case 7: add = JY_mb_rou4; break;
                case 8: add = JY_mb_ran2; break; 
                case 9: add = JY_mb_ran3; break; 
                case 10: add = JY_mb_ren2; break;
                case 11: add = JY_mb_ren3; break;
                case 12: add = JY_mb_ren4; break;
                case 13: add = JY_mb_rang2; break; 
                case 14: add = JY_mb_rang3; break; 
                case 15: add = JY_mb_rang4; break; 
                case 16: add = JY_mb_reng; break; 
                case 17: add = JY_mb_reng2; break; 
                case 18: add = JY_mb_ru2; break; 
                case 19: add = JY_mb_ru3; break; 
                case 20: add = JY_mb_ru4; break; 
                case 21: add = JY_mb_ruo4; break;  
                case 22: add = JY_mb_rui2; break; 
                case 23: add = JY_mb_rui3; break; 
                case 24: add = JY_mb_rui4; break; 
                case 25: add = JY_mb_ruan2; break;
                case 26: add = JY_mb_ruan3; break;
                case 27: add = JY_mb_run2; break; 
                case 28: add = JY_mb_run4; break; 
                case 29: add = JY_mb_rong2; break;
                case 30: add = JY_mb_rong3; break;
                }
                g_hanzi_num = (unsigned short)strlen(add); //   拼音  a 的个数
                memcpy(get_hanzi, add, g_hanzi_num);       //  把找到的汉字放在缓冲区
                *hh         = g_hanzi_num / 2;
                return 1;
            }
        }
        return 0;
    }

    // 首字母为 'ㄗ'

    if (jy_ime_cmp == 'y')
    {
        for (jy_i = 0; jy_i < sizeofZ; jy_i++)                     // 16次
        {
            memcpy(jy_ime_temp, JY_index_z[jy_i], 5);              //   ai,an,ang,ao，ei,...   16个
            jy_ime_cmp = memcmp(&jy_ime_temp1[1], jy_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (jy_ime_cmp == 0)
            {
                printf("jy_i==%d\n", jy_i);
                switch (jy_i)
                {
                case 0: add = JY_mb_zi; break; 
                case 1: add = JY_mb_zi3; break; 
                case 2: add = JY_mb_zi4; break; 
                case 3: add = JY_mb_zi5; break; 
                case 4: add = JY_mb_za; break; 
                case 5: add = JY_mb_za2; break;  
                case 6: add = JY_mb_ze2; break; 
                case 7: add = JY_mb_ze3; break; 
                case 8: add = JY_mb_ze4; break; 
                case 9: add = JY_mb_zai; break; 
                case 10: add = JY_mb_zai3; break; 
                case 11: add = JY_mb_zai4; break;  
                case 12: add = JY_mb_zao; break; 
                case 13: add = JY_mb_zao2; break; 
                case 14: add = JY_mb_zao3; break; 
                case 15: add = JY_mb_zao4; break;  
                case 16: add = JY_mb_zei2; break; 
                case 17: add = JY_mb_zou; break; 
                case 18: add = JY_mb_zou3; break; 
                case 19: add = JY_mb_zou4; break; 
                case 20: add = JY_mb_zan; break; 
                case 21: add = JY_mb_zan2; break; 
                case 22: add = JY_mb_zan3; break; 
                case 23: add = JY_mb_zan4; break; 
                case 24: add = JY_mb_zen3; break; 
                case 25: add = JY_mb_zen4; break;   
                case 26: add = JY_mb_zang; break; 
                case 27: add = JY_mb_zang3; break; 
                case 28: add = JY_mb_zang4; break; 
                case 29: add = JY_mb_zeng; break; 
                case 30: add = JY_mb_zeng4; break; 
                case 31: add = JY_mb_zu; break;
                case 32: add = JY_mb_zu2; break;
                case 33: add = JY_mb_zu3; break; 
                case 34: add = JY_mb_zuo; break;  
                case 35: add = JY_mb_zuo2; break;  
                case 36: add = JY_mb_zuo3; break;  
                case 37: add = JY_mb_zuo4; break;  
                case 38: add = JY_mb_zui3; break; 
                case 39: add = JY_mb_zui4; break; 
                case 40: add = JY_mb_zun; break; 
                case 41: add = JY_mb_zun3; break; 
                case 42: add = JY_mb_zuan; break; 
                case 43: add = JY_mb_zuan3; break; 
                case 44: add = JY_mb_zuan4; break; 
                case 45: add = JY_mb_zong; break; 
                case 46: add = JY_mb_zong3; break; 
                case 47: add = JY_mb_zong4; break; 
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
    
    // 首字母为 'ㄘ'

    if (jy_ime_cmp == 'h')
    {
        for (jy_i = 0; jy_i < sizeofC; jy_i++)                     // 16次
        {
            memcpy(jy_ime_temp, JY_index_c[jy_i], 5);              //   ai,an,ang,ao，ei,...   16个
            jy_ime_cmp = memcmp(&jy_ime_temp1[1], jy_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (jy_ime_cmp == 0)
            {
                printf("jy_i==%d\n", jy_i);
                switch (jy_i)
                {
                case 0: add = JY_mb_ci; break;
                case 1: add = JY_mb_ci2; break;
                case 2: add = JY_mb_ci3; break;
                case 3: add = JY_mb_ci4; break;
                case 4: add = JY_mb_ca; break; 
                case 5: add = JY_mb_ce4; break; 
                case 6: add = JY_mb_cai; break; 
                case 7: add = JY_mb_cai2; break; 
                case 8: add = JY_mb_cai3; break; 
                case 9: add = JY_mb_cai4; break; 
                case 10: add = JY_mb_cao; break; 
                case 11: add = JY_mb_cao2; break; 
                case 12: add = JY_mb_cao3; break; 
                case 13: add = JY_mb_cou4; break; 
                case 14: add = JY_mb_can; break;
                case 15: add = JY_mb_can2; break;
                case 16: add = JY_mb_can3; break;
                case 17: add = JY_mb_can4; break;
                case 18: add = JY_mb_cen; break; 
                case 19: add = JY_mb_cen2; break; 
                case 20: add = JY_mb_cang; break; 
                case 21: add = JY_mb_cang2; break; 
                case 22: add = JY_mb_ceng; break; 
                case 23: add = JY_mb_ceng2; break; 
                case 24: add = JY_mb_ceng4; break; 
                case 25: add = JY_mb_cu; break; 
                case 26: add = JY_mb_cu2; break; 
                case 27: add = JY_mb_cu4; break; 
                case 28: add = JY_mb_cuo; break; 
                case 29: add = JY_mb_cuo2; break; 
                case 30: add = JY_mb_cuo3; break; 
                case 31: add = JY_mb_cuo4; break; 
                case 32: add = JY_mb_cui; break; 
                case 33: add = JY_mb_cui3; break; 
                case 34: add = JY_mb_cui4; break; 
                case 35: add = JY_mb_cuan; break; 
                case 36: add = JY_mb_cuan2; break; 
                case 37: add = JY_mb_cuan4; break; 
                case 38: add = JY_mb_cun; break; 
                case 39: add = JY_mb_cun2; break; 
                case 40: add = JY_mb_cun3; break; 
                case 41: add = JY_mb_cun4; break; 
                case 42: add = JY_mb_cong; break; 
                case 43: add = JY_mb_cong2; break;                 
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
    
    // 首字母为 'ㄙ'

    if (jy_ime_cmp == 'n')
    {
        for (jy_i = 0; jy_i < sizeofS; jy_i++)                     // 16次
        {
            memcpy(jy_ime_temp, JY_index_s[jy_i], 5);              //   ai,an,ang,ao，ei,...   16个
            jy_ime_cmp = memcmp(&jy_ime_temp1[1], jy_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (jy_ime_cmp == 0)
            {
                printf("jy_i==%d\n", jy_i);
                switch (jy_i)
                {
                case 0: add = JY_mb_si; break;
                case 1: add = JY_mb_si3; break;
                case 2: add = JY_mb_si4; break;
                case 3: add = JY_mb_sa; break; 
                case 4: add = JY_mb_sa3; break; 
                case 5: add = JY_mb_sa4; break; 
                case 6: add = JY_mb_se4; break; 
                case 7: add = JY_mb_sai; break; 
                case 8: add = JY_mb_sai4; break; 
                case 9: add = JY_mb_san; break; 
                case 10: add = JY_mb_san3; break; 
                case 11: add = JY_mb_san4; break; 
                case 12: add = JY_mb_sao; break; 
                case 13: add = JY_mb_sao3; break; 
                case 14: add = JY_mb_sao4; break; 
                case 15: add = JY_mb_sou; break;
                case 16: add = JY_mb_sou3; break;
                case 17: add = JY_mb_sou4; break;
                case 18: add = JY_mb_sen; break; 
                case 19: add = JY_mb_sang; break; 
                case 20: add = JY_mb_sang3; break; 
                case 21: add = JY_mb_sang4; break; 
                case 22: add = JY_mb_seng; break; 
                case 23: add = JY_mb_su; break; 
                case 24: add = JY_mb_su2; break; 
                case 25: add = JY_mb_su4; break; 
                case 26: add = JY_mb_suo; break;
                case 27: add = JY_mb_suo3; break;
                case 28: add = JY_mb_sui; break;
                case 29: add = JY_mb_sui2; break;
                case 30: add = JY_mb_sui3; break;
                case 31: add = JY_mb_sui4; break;
                case 32: add = JY_mb_suan; break; 
                case 33: add = JY_mb_suan4; break; 
                case 34: add = JY_mb_sun; break; 
                case 35: add = JY_mb_sun3; break; 
                case 36: add = JY_mb_song; break; 
                case 37: add = JY_mb_song3; break; 
                case 38: add = JY_mb_song4; break; 
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
    
    // 首字母为 'ㄚ'

    if (jy_ime_cmp == '8')
    {
        for (jy_i = 0; jy_i < sizeofA; jy_i++)                     // 16次
        {
            memcpy(jy_ime_temp, JY_index_a[jy_i], 5);              //   ai,an,ang,ao，ei,...   16个
            jy_ime_cmp = memcmp(&jy_ime_temp1[1], jy_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (jy_ime_cmp == 0)
            {
                printf("jy_i==%d\n", jy_i);
                switch (jy_i)
                {
                case 0: add = JY_mb_a; break;
                case 1: add = JY_mb_a2; break;
                case 2: add = JY_mb_a3; break;
                case 3: add = JY_mb_a4; break;
                case 4: add = JY_mb_a5; break;
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
                
    // 首字母为 'ㄛ'

    if (jy_ime_cmp == 'i')
    {
        for (jy_i = 0; jy_i < sizeofO; jy_i++)                     // 16次
        {
            memcpy(jy_ime_temp, JY_index_o[jy_i], 5);              //   ai,an,ang,ao，ei,...   16个
            jy_ime_cmp = memcmp(&jy_ime_temp1[1], jy_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (jy_ime_cmp == 0)
            {
                printf("jy_i==%d\n", jy_i);
                switch (jy_i)
                {
                case 0: add = JY_mb_o; break; 
                case 1: add = JY_mb_o2; break; 
                case 2: add = JY_mb_o3; break; 
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

    // 首字母为 'ㄜ'

    if (jy_ime_cmp == 'k')
    {
        for (jy_i = 0; jy_i < sizeofE; jy_i++)                     // 16次
        {
            memcpy(jy_ime_temp, JY_index_e[jy_i], 5);              //   ai,an,ang,ao，ei,...   16个
            jy_ime_cmp = memcmp(&jy_ime_temp1[1], jy_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (jy_ime_cmp == 0)
            {
                printf("jy_i==%d\n", jy_i);
                switch (jy_i)
                {
                case 0: add = JY_mb_e; break;
                case 1: add = JY_mb_e2; break;
                case 2: add = JY_mb_e3; break;
                case 3: add = JY_mb_e4; break;
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

    // 首字母为 'ㄝ ㄟ'

    if (jy_ime_cmp == ',' || jy_ime_cmp == 'o')
    {
        for (jy_i = 0; jy_i < sizeofEI; jy_i++)                     // 16次
        {
            memcpy(jy_ime_temp, JY_index_ei[jy_i], 5);              //   ai,an,ang,ao，ei,...   16个
            jy_ime_cmp = memcmp(&jy_ime_temp1[1], jy_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (jy_ime_cmp == 0)
            {
                printf("jy_i==%d\n", jy_i);
                switch (jy_i)
                {
                case 0: add = JY_mb_ei4; break;
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

    // 首字母为 'ㄞ'

    if (jy_ime_cmp == '9')
    {
        for (jy_i = 0; jy_i < sizeofAI; jy_i++)                     // 16次
        {
            memcpy(jy_ime_temp, JY_index_ai[jy_i], 5);              //   ai,an,ang,ao，ei,...   16个
            jy_ime_cmp = memcmp(&jy_ime_temp1[1], jy_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (jy_ime_cmp == 0)
            {
                printf("jy_i==%d\n", jy_i);
                switch (jy_i)
                {
                case 0: add = JY_mb_ai; break;
                case 1: add = JY_mb_ai2; break;
                case 2: add = JY_mb_ai3; break;
                case 3: add = JY_mb_ai4; break;
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

    // 首字母为 'ㄠ'

    if (jy_ime_cmp == 'l')
    {
        for (jy_i = 0; jy_i < sizeofAO; jy_i++)                     // 16次
        {
            memcpy(jy_ime_temp, JY_index_ao[jy_i], 5);              //   ai,an,ang,ao，ei,...   16个
            jy_ime_cmp = memcmp(&jy_ime_temp1[1], jy_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (jy_ime_cmp == 0)
            {
                printf("jy_i==%d\n", jy_i);
                switch (jy_i)
                {
                case 0: add = JY_mb_ao; break;
                case 1: add = JY_mb_ao2; break;
                case 2: add = JY_mb_ao3; break;
                case 3: add = JY_mb_ao4; break;
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

    // 首字母为 'ㄡ'

    if (jy_ime_cmp == '.')
    {
        for (jy_i = 0; jy_i < sizeofOU; jy_i++)                     // 16次
        {
            memcpy(jy_ime_temp, JY_index_ou[jy_i], 5);              //   ai,an,ang,ao，ei,...   16个
            jy_ime_cmp = memcmp(&jy_ime_temp1[1], jy_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (jy_ime_cmp == 0)
            {
                printf("jy_i==%d\n", jy_i);
                switch (jy_i)
                {
                case 0: add = JY_mb_ou; break; 
                case 1: add = JY_mb_ou3; break; 
                case 2: add = JY_mb_ou4; break; 
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

    // 首字母为 'ㄢ'

    if (jy_ime_cmp == '0')
    {
        for (jy_i = 0; jy_i < sizeofAN; jy_i++)                     // 16次
        {
            memcpy(jy_ime_temp, JY_index_an[jy_i], 5);              //   ai,an,ang,ao，ei,...   16个
            jy_ime_cmp = memcmp(&jy_ime_temp1[1], jy_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (jy_ime_cmp == 0)
            {
                printf("jy_i==%d\n", jy_i);
                switch (jy_i)
                {
                case 0: add = JY_mb_an; break;
                case 1: add = JY_mb_an3; break;
                case 2: add = JY_mb_an4; break;
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

    // 首字母为 'ㄣ'

    if (jy_ime_cmp == 'p')
    {
        for (jy_i = 0; jy_i < sizeofEN; jy_i++)                     // 16次
        {
            memcpy(jy_ime_temp, JY_index_en[jy_i], 5);              //   ai,an,ang,ao，ei,...   16个
            jy_ime_cmp = memcmp(&jy_ime_temp1[1], jy_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (jy_ime_cmp == 0)
            {
                printf("jy_i==%d\n", jy_i);
                switch (jy_i)
                {
                case 0: add = JY_mb_en; break; 
                case 1: add = JY_mb_en4; break; 
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

    // 首字母为 'ㄤ'

    if (jy_ime_cmp == ';')
    {
        for (jy_i = 0; jy_i < sizeofANG; jy_i++)                     // 16次
        {
            memcpy(jy_ime_temp, JY_index_ang[jy_i], 5);              //   ai,an,ang,ao，ei,...   16个
            jy_ime_cmp = memcmp(&jy_ime_temp1[1], jy_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (jy_ime_cmp == 0)
            {
                printf("jy_i==%d\n", jy_i);
                switch (jy_i)
                {
                case 0: add = JY_mb_ang; break;
                case 1: add = JY_mb_ang2; break;
                case 2: add = JY_mb_ang4; break;
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

    // 首字母为 'ㄦ'

    if (jy_ime_cmp == '-')
    {
        for (jy_i = 0; jy_i < sizeofER; jy_i++)                     // 16次
        {
            memcpy(jy_ime_temp, JY_index_er[jy_i], 5);              //   ai,an,ang,ao，ei,...   16个
            jy_ime_cmp = memcmp(&jy_ime_temp1[1], jy_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (jy_ime_cmp == 0)
            {
                printf("jy_i==%d\n", jy_i);
                switch (jy_i)
                {
                case 0: add = JY_mb_er2; break; 
                case 1: add = JY_mb_er3; break; 
                case 2: add = JY_mb_er4; break; 
                case 3: add = JY_mb_er5; break; 
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
                                                                            
    // 首字母为 'ㄧ'

    if (jy_ime_cmp == 'u')
    {
        for (jy_i = 0; jy_i < sizeofY; jy_i++)                     // 16次
        {
            memcpy(jy_ime_temp, JY_index_y[jy_i], 5);              //   ai,an,ang,ao，ei,...   16个
            jy_ime_cmp = memcmp(&jy_ime_temp1[1], jy_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (jy_ime_cmp == 0)
            {
                printf("jy_i==%d\n", jy_i);
                switch (jy_i)
                {
                case 0: add = JY_mb_yi; break; 
                case 1: add = JY_mb_yi2; break; 
                case 2: add = JY_mb_yi3; break; 
                case 3: add = JY_mb_yi4; break; 
                case 4: add = JY_mb_ya; break; 
                case 5: add = JY_mb_ya2; break; 
                case 6: add = JY_mb_ya3; break; 
                case 7: add = JY_mb_ya4; break; 
                case 8: add = JY_mb_ya5; break; 
                case 9: add = JY_mb_yo; break; 
                case 10: add = JY_mb_yo5; break; 
                case 11: add = JY_mb_ye; break; 
                case 12: add = JY_mb_ye2; break; 
                case 13: add = JY_mb_ye3; break; 
                case 14: add = JY_mb_ye4; break; 
                case 15: add = JY_mb_yai2; break; 
                case 16: add = JY_mb_yao; break; 
                case 17: add = JY_mb_yao2; break; 
                case 18: add = JY_mb_yao3; break; 
                case 19: add = JY_mb_yao4; break; 
                case 20: add = JY_mb_yao5; break; 
                case 21: add = JY_mb_you; break; 
                case 22: add = JY_mb_you2; break; 
                case 23: add = JY_mb_you3; break; 
                case 24: add = JY_mb_you4; break; 
                case 25: add = JY_mb_yan; break; 
                case 26: add = JY_mb_yan2; break; 
                case 27: add = JY_mb_yan3; break; 
                case 28: add = JY_mb_yan4; break; 
                case 29: add = JY_mb_yin; break;
                case 30: add = JY_mb_yin2; break;
                case 31: add = JY_mb_yin3; break;
                case 32: add = JY_mb_yin4; break;
                case 33: add = JY_mb_yang; break; 
                case 34: add = JY_mb_yang2; break; 
                case 35: add = JY_mb_yang3; break; 
                case 36: add = JY_mb_yang4; break; 
                case 37: add = JY_mb_ying; break;
                case 38: add = JY_mb_ying2; break;
                case 39: add = JY_mb_ying3; break;
                case 40: add = JY_mb_ying4; break;
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
    
    // 首字母为 'ㄨ'

    if (jy_ime_cmp == 'j')
    {
        for (jy_i = 0; jy_i < sizeofW; jy_i++)                     // 16次
        {
            memcpy(jy_ime_temp, JY_index_w[jy_i], 5);              //   ai,an,ang,ao，ei,...   16个
            jy_ime_cmp = memcmp(&jy_ime_temp1[1], jy_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (jy_ime_cmp == 0)
            {
                printf("jy_i==%d\n", jy_i);
                switch (jy_i)
                {
                case 0: add = JY_mb_wu; break;
                case 1: add = JY_mb_wu2; break;
                case 2: add = JY_mb_wu3; break;
                case 3: add = JY_mb_wu4; break;
                case 4: add = JY_mb_wa; break; 
                case 5: add = JY_mb_wa2; break; 
                case 6: add = JY_mb_wa3; break; 
                case 7: add = JY_mb_wa4; break; 
                case 8: add = JY_mb_wo; break; 
                case 9: add = JY_mb_wo3; break; 
                case 10: add = JY_mb_wo4; break; 
                case 11: add = JY_mb_wai; break; 
                case 12: add = JY_mb_wai4; break; 
                case 13: add = JY_mb_wei; break;
                case 14: add = JY_mb_wei2; break;
                case 15: add = JY_mb_wei3; break;
                case 16: add = JY_mb_wei4; break;
                case 17: add = JY_mb_wan; break; 
                case 18: add = JY_mb_wan2; break; 
                case 19: add = JY_mb_wan3; break; 
                case 20: add = JY_mb_wan4; break; 
                case 21: add = JY_mb_wen; break; 
                case 22: add = JY_mb_wen2; break; 
                case 23: add = JY_mb_wen3; break; 
                case 24: add = JY_mb_wen4; break; 
                case 25: add = JY_mb_wang; break;
                case 26: add = JY_mb_wang2; break;
                case 27: add = JY_mb_wang3; break;
                case 28: add = JY_mb_wang4; break;
                case 29: add = JY_mb_weng; break;
                case 30: add = JY_mb_weng3; break;
                case 31: add = JY_mb_weng4; break;
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
    
    // 首字母为 'ㄩ'

    if (jy_ime_cmp == 'm')
    {
        for (jy_i = 0; jy_i < sizeofYU; jy_i++)                     // 16次
        {
            memcpy(jy_ime_temp, JY_index_yu[jy_i], 5);              //   ai,an,ang,ao，ei,...   16个
            jy_ime_cmp = memcmp(&jy_ime_temp1[1], jy_ime_temp, 5); // 输入的拼音和'a'的第2个韵母比较
            if (jy_ime_cmp == 0)
            {
                printf("jy_i==%d\n", jy_i);
                switch (jy_i)
                {
                case 0: add = JY_mb_yu; break;
                case 1: add = JY_mb_yu2; break;
                case 2: add = JY_mb_yu3; break;
                case 3: add = JY_mb_yu4; break;
                case 4: add = JY_mb_yue; break; 
                case 5: add = JY_mb_yue4; break; 
                case 6: add = JY_mb_yuan; break; 
                case 7: add = JY_mb_yuan2; break; 
                case 8: add = JY_mb_yuan3; break; 
                case 9: add = JY_mb_yuan4; break; 
                case 10: add = JY_mb_yun; break;
                case 11: add = JY_mb_yun2; break;
                case 12: add = JY_mb_yun3; break;
                case 13: add = JY_mb_yun4; break;
                case 14: add = JY_mb_yong; break; 
                case 15: add = JY_mb_yong2; break; 
                case 16: add = JY_mb_yong3; break; 
                case 17: add = JY_mb_yong4; break; 
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


    return 0;                      /*无果而终*/
}