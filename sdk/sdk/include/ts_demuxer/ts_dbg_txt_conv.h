#ifndef __TS_TXT_CONV_H_67A1BAAR_86UI_9DUB_Z98T_WPUIKEOTYO7R__
#define __TS_TXT_CONV_H_67A1BAAR_86UI_9DUB_Z98T_WPUIKEOTYO7R__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdint.h>
//=============================================================================
//				  Constant Definition
//=============================================================================
//functions return codes
#define RET_OK 				0 				//success
#define RET_INVALID_UTF16	1				//invalid utf16 atom
#define RET_INVALID_UTF8	2				//invalid utf8  atom
#define RET_BUFFER_OVERFLOW 3				//allocated buffer is too small
#define RET_STRING_TOO_BIG  4				//provided string is too big

//some macros
#define be_to_le(x) ( ( (x & 0x00FF) << 8 ) | ((x & 0xFF00 ) >> 8 ) ) //for 2 bytes words only
#define LEAD_SURROGATE_MIN 0xD800
#define LEAD_SURROGATE_MAX 0xDBFF
#define TRAIL_SURROGATE_MIN 0xDC00
#define TRAIL_SURROGATE_MAX 0xDFFF

#define LEAD_OFFSET (LEAD_SURROGATE_MIN - ( 0x10000 >> 10 ))
#define SURROGAT_OFFSET ( 0x10000 - (0xD800 << 10) - 0xDC00 )

#define lead_surrogate( x ) ( LEAD_OFFSET + ( x >> 10 ) )
#define trail_surrogate( x ) ( TRAIL_SURROGATE_MIN + ( x & 0x3FF) )

#define UTF16_EOL	'\00'
#define UTF8_EOL 	'\0'

#ifndef UINT_MAX
    #define UINT_MAX    0xFFFFFFFF
#endif
//=============================================================================
//				  Macro Definition
//=============================================================================

//=============================================================================
//				  Structure Definition
//=============================================================================

//=============================================================================
//				  Global Data Definition
//=============================================================================

//=============================================================================
//				  Private Function Definition
//=============================================================================

static int 
is_valid_utf16_atom(
    const uint16_t *chr)
{
	//http://unicode.org/faq/utf_bom.html#utf16-7 
	if ( ( *chr >= LEAD_SURROGATE_MIN  && *chr <= LEAD_SURROGATE_MAX ) && 
         ( *(chr+1) < TRAIL_SURROGATE_MIN  || *(chr+1) > TRAIL_SURROGATE_MAX ) )
		return 1 == 2;
	
	if ( ( *chr >= TRAIL_SURROGATE_MIN && *chr <= TRAIL_SURROGATE_MAX ) && 
         ( *(chr-1) < LEAD_SURROGATE_MIN  || *(chr-1) > LEAD_SURROGATE_MAX ) )
		return 1 == 2;
	
	return 1 == 1;
}


static uint32_t 
utf16le_code_point(
    const uint16_t *chr)
{
	if ( *chr >= LEAD_SURROGATE_MIN ) //we have 2 word value
	{
		return ( chr[0] << 10 ) + chr[1] + SURROGAT_OFFSET;
	}
	else
		return (uint32_t)(*chr);
}

static uint32_t 
utf16be_code_point(
    const uint16_t *chr)
{
	if ( be_to_le(*chr) >= LEAD_SURROGATE_MIN ) //we have 2 word value
	{
		return ( be_to_le(chr[0]) << 10 ) + be_to_le(chr[1]) + SURROGAT_OFFSET;
	}
	else
		return (uint32_t)(be_to_le(*chr));	
}

// internal function. supposes, that codepoint is valid utf16 value. do not call directly from your code
// 
static unsigned int 
convert_utf16atom( 
    uint32_t            codepoint, 
    uint8_t             *dest, 
    const unsigned int  dest_size, 
    unsigned int        *cur_pos)
{
	
	if ( codepoint < 0x80 && *cur_pos < dest_size ) 
		dest[(*cur_pos)++] = codepoint;
	else
	if ( codepoint < 0x800 && *cur_pos + 1 < dest_size  )
	{
		dest[(*cur_pos)++] = (uint8_t)(( codepoint >> 6 ) | 0xc0);
        dest[(*cur_pos)++] = (uint8_t)(( codepoint & 0x3f ) | 0x80);
	}
	else 
	if ( codepoint < 0x10000 && *cur_pos + 2 < dest_size ) 
	{              
    	dest[(*cur_pos)++] = (uint8_t)((  codepoint >> 12 ) | 0xe0);
        dest[(*cur_pos)++] = (uint8_t)((( codepoint >> 6 ) & 0x3f) | 0x80);
        dest[(*cur_pos)++] = (uint8_t)((  codepoint & 0x3f ) | 0x80);
     }
     else
     if ( *cur_pos + 3 < dest_size )
     {      
     	dest[(*cur_pos)++] = (uint8_t)((  codepoint >> 18) | 0xf0);
        dest[(*cur_pos)++] = (uint8_t)((( codepoint >> 12) & 0x3f) | 0x80);
        dest[(*cur_pos)++] = (uint8_t)((( codepoint >> 6) & 0x3f) | 0x80);
        dest[(*cur_pos)++] = (uint8_t)((  codepoint & 0x3f) | 0x80);
     }
     else 
     	return RET_BUFFER_OVERFLOW;
	return RET_OK;
} 

static unsigned int 
utf8_to_utf16( 
    const uint8_t *source, 
    uint16_t      *dest, 
    unsigned int  dest_size, 
    unsigned int  *bytes_used, 
    const char    to_be)
{
        unsigned int offset = 0;
		unsigned int cur_pos = 0;
		uint32_t cp;
		int i = 0;
		while ( *(source + offset) != UTF8_EOL )
        {
			if ( offset == UINT_MAX )
				return RET_STRING_TOO_BIG;
			
			if ( (*(source + offset) & 0xF0) == 0xF0 ) // 4 bytes sequence
			{
				//calc code point from 4 bytes
				if ( ( (*(source + offset + 1) & 0x80 ) != 0x80 ) && 
                     ( (*(source + offset + 2) & 0x80 ) != 0x80 ) && 
                     ( (*(source + offset + 3) & 0x80 ) != 0x80 ) ) //second or third byte has no marker 
					return RET_INVALID_UTF8;	
				cp = ( ( ( ( ( *(source + offset) & 0x7 ) << 2 ) | ( ( *(source + offset + 1) & 0x30 ) >> 4 ) ) ) << 16 ) | 
						 ( (  ( ( *(source + offset + 1) & 0xF ) << 4 ) | ( ( *(source + offset + 2) & 0x3C ) >> 2 )  ) << 8 )  | 
						 ( ( ( *(source + offset + 2) & 0x3 ) << 6 ) | ( *(source + offset + 3) & 0x3F ) );
				offset+=4;
			} 
			else
			if ( (*(source + offset) & 0xE0) == 0xE0 ) // 3 bytes sequence
			{
				//calc code point from 3 bytes
				if ( ( (*(source + offset + 1) & 0x80 ) != 0x80 ) && ( (*(source + offset + 2) & 0x80 ) != 0x80 ) )  //second or third byte has no marker 
					return RET_INVALID_UTF8;	
				cp = ( ( ( (*( source + offset ) & 0xF) << 4) | ( ( *(source + offset + 1) & 0x3C ) >> 2 ) ) << 8 ) | ( ( *(source + offset + 1) & 0x3 ) << 6 ) | ( *(source + offset + 2) & 0x3F )  ;
				offset+=3;
			}
			else // 2 bytes sequence
			if ( ( *(source + offset ) & 0xC0 ) == 0xC0 )
			{
				//calc code point from 2 bytes	
				if ( (*(source + offset + 1) & 0x80 ) != 0x80 ) //second byte has no marker 
					return RET_INVALID_UTF8;
				cp = (((*(source + offset) & 0x1F ) >> 2 ) << 8 ) | ( ( (*(source + offset )  & 0x3) << 6 ) | ( *(source + offset + 1) & 0x3F ) );
				offset+=2;
				if ( cp <= 127 ) //overlong sequence
					return RET_INVALID_UTF8;
			}
			else // ASCII char?
			if ( *( source + offset ) <= 127 )
			{
				cp = *( source + offset++ );
			} 
			else //second byte?
				return RET_INVALID_UTF8;
			
			//encode codepoint 
			if ( cp > 0xFFFF ) //two words sequence
			{
					if ( to_be == 0 ) 
					{
						dest[cur_pos+1] = LEAD_SURROGATE_MIN + ( ( cp - 0x10000 ) >> 10 );  //first word, little-endian notation!
						dest[cur_pos]   = TRAIL_SURROGATE_MIN + ( ( cp - 0x10000 ) & 0x3FF ); //second word
					}
					else
					{
						dest[cur_pos] =  be_to_le ( LEAD_SURROGATE_MIN + ( ( cp - 0x10000 ) >> 10 ) );  //first word, big-endian notation!
						dest[cur_pos + 1]   = be_to_le( TRAIL_SURROGATE_MIN + ( ( cp - 0x10000 ) & 0x3FF ) ); //second word
					}
					cur_pos+=2;
			}
			else 
			{
					if ( to_be == 0 ) //little endian
						dest[cur_pos++] = cp;
					else
						dest[cur_pos++] = be_to_le( cp );
			}
			if ( cur_pos >= dest_size )
				return RET_BUFFER_OVERFLOW;

        }
		*bytes_used = ( cur_pos + 1 ) * 2;
		dest[cur_pos] = UTF16_EOL;
        return RET_OK;
}


static unsigned int 
utf16_to_utf8(
    const uint16_t      *source, 
    uint8_t             *dest, 
    const unsigned int  dest_size, 
    unsigned int        *bytes_used, 
    const char          to_be)
{
	unsigned int cur_pos = 0;
	unsigned int offset = 0;
	unsigned int rez = 0;
	uint32_t cp;
	int i = 0;
	
	while ( 42 )
	{
		if ( !is_valid_utf16_atom( source+offset ) )
			return RET_INVALID_UTF16; 
		if ( source[offset] == UTF16_EOL ) 
			break;
		
		if ( offset == UINT_MAX )
			return RET_STRING_TOO_BIG; // omg, what was it?
			
		if ( to_be == 0 )
			cp = utf16le_code_point( &source[offset++] );
		else 
			cp = utf16be_code_point( &source[offset++] );
		if ( cp > 0xFFFF )
			offset++;
		
		rez = convert_utf16atom( cp, dest, dest_size, &cur_pos );
		if ( rez != RET_OK )
			return rez; 
	   
	}
	dest[cur_pos++] = UTF8_EOL;
	*bytes_used = cur_pos; 
	return RET_OK;	
}

//=============================================================================
//				  Public Function Definition
//=============================================================================
//user functions - call it from your code
#define utf16be_to_utf8(x, y, z, w)     utf16_to_utf8(x, y, z, w, 42)
#define utf16le_to_utf8(x, y, z, w)     utf16_to_utf8(x, y, z, w, 0)
#define utf8_to_utf16le(x, y, z, w)     utf8_to_utf16(x, y, z, w, 0)
#define utf8_to_utf16be(x, y, z, w)     utf8_to_utf16(x, y, z, w, 21) 


#ifdef __cplusplus
}
#endif

#endif
