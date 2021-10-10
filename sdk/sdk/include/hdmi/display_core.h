#ifndef DISPLAY_CORE_H
#define DISPLAY_CORE_H

typedef enum
{
	DF_1080p60 = 0,
	DF_1080p50,//1
	DF_1080i60,//2
	DF_1080i50,//3
	DF_720p60, //4
	DF_720p50, //5
	DF_576p50, //6
	DF_576i50, //7
	DF_480p60, //8
	DF_480i60, //9
	DF_LCD_800_600, //10
} DC_ENUM_DISPLAY_FORMAT;

int dc_init_display(DC_ENUM_DISPLAY_FORMAT display_format, unsigned audio_sample_rate, unsigned char *buf, unsigned pitch);
int dc_deinit_display(void);

int dc_change_display_format(DC_ENUM_DISPLAY_FORMAT display_format, unsigned char *buf, unsigned pitch);

#endif //DISPLAY_CORE_H

