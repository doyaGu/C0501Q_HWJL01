static const unsigned short I2S_DIGVOL_TABLE[] = {
	0x000, /* idx[ 0]:-INFdB */
	0x001, /* idx[ 1]:-48 dB */
	0x002, /* idx[ 2]:-42 dB */
	0x003, /* idx[ 3]:-39 dB */
	0x004, /* idx[ 4]:-36 dB */
	0x005, /* idx[ 5]:-34 dB */
	0x006, /* idx[ 6]:-33 dB */
	0x007, /* idx[ 7]:-31 dB */
	0x008, /* idx[ 8]:-30 dB */
	0x009, /* idx[ 9]:-29 dB */
	0x00A, /* idx[10]:-28 dB */
	0x00B, /* idx[11]:-27 dB */
	0x00D, /* idx[12]:-26 dB */
	0x00E, /* idx[13]:-25 dB */
	0x010, /* idx[14]:-24 dB */
	0x012, /* idx[15]:-23 dB */
	0x014, /* idx[16]:-22 dB */
	0x017, /* idx[17]:-21 dB */
	0x01A, /* idx[18]:-20 dB */
	0x01D, /* idx[19]:-19 dB */
	0x020, /* idx[20]:-18 dB */
	0x024, /* idx[21]:-17 dB */
	0x029, /* idx[22]:-16 dB */
	0x02E, /* idx[23]:-15 dB */
	0x033, /* idx[24]:-14 dB */
	0x039, /* idx[25]:-13 dB */
	0x040, /* idx[26]:-12 dB */
	0x048, /* idx[27]:-11 dB */
	0x051, /* idx[28]:-10 dB */
	0x05B, /* idx[29]:- 9 dB */
	0x066, /* idx[30]:- 8 dB */
	0x072, /* idx[31]:- 7 dB */
	0x080, /* idx[32]:- 6 dB */
	0x090, /* idx[33]:- 5 dB */
	0x0A1, /* idx[34]:- 4 dB */
	0x0B5, /* idx[35]:- 3 dB */
	0x0CA, /* idx[36]:- 2 dB */
	0x0E4, /* idx[37]:- 1 dB */
	0x100, /* idx[38]:  0 dB */
};

#define I2S_DIGVOL_TABLE_Size (sizeof(I2S_DIGVOL_TABLE) / sizeof(unsigned short))

#define I2S_DIGITAL_VOL_MAX_IDX  (I2S_DIGVOL_TABLE_Size - 1) /* 0x100: 0 dB */
#define I2S_DIGITAL_VOL_ZERO_IDX (I2S_DIGVOL_TABLE_Size - 1) /* 0x100: 0 dB */
#define I2S_DIGITAL_VOL_MIN_IDX  0                           /* 0x000: -INFdB */

