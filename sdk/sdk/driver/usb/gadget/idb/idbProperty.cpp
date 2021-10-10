/*
 *
 *  Copyright (C) 2012 ITE TECH. INC.   All Rights Reserved.
 *
 */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "ite/itp.h"
#include "ite/itc.h"
#include "ite/ite_idb.h"
#define printf(...)             do {} while (0)
#include "idbProperty.h"
#define                         IT_CONFIG_USE_MALLOC    1

#define IT_USBD_MK4CC(ch0,ch1,ch2,ch3)              \
    (((unsigned int)(unsigned char)(ch0) << 0)  |   \
     ((unsigned int)(unsigned char)(ch1) << 8)  |   \
     ((unsigned int)(unsigned char)(ch2) << 16) |   \
     ((unsigned int)(unsigned char)(ch3) << 24))

static struct idb_config* the_idb;

int it_auto_log::m_depth(0);

unsigned int                    it_usbd_property::m_video_source(it_usbd_property::VIDEO_SOURCE_HDMI);
unsigned int                    it_usbd_property::m_audio_source(it_usbd_property::AUDIO_SOURCE_HDMI);
unsigned int                    it_usbd_property::m_cmd_sequence(0);
unsigned int                    it_usbd_property::m_firmware_size_now(0);
unsigned int                    it_usbd_property::m_firmware_size_total(0);
unsigned char*                  it_usbd_property::m_firmware_data(NULL);
unsigned char*                  it_usbd_property::m_firmware_reboot(0);
unsigned int                    it_usbd_property::m_firmware_status(100);
int                             it_usbd_property::m_video_procamp_brightness(0/*750*/);
int                             it_usbd_property::m_video_procamp_contrast(100);
int                             it_usbd_property::m_video_procamp_hue(0);
int                             it_usbd_property::m_video_procamp_saturation(100);
int                                 it_usbd_property::m_video_procamp_gain(1);
int                             it_usbd_property::m_video_compression_keyframe_rate(10);
int                             it_usbd_property::m_video_compression_quality(5000);
unsigned long                   it_usbd_property::m_my_time_base(0);
char                            it_usbd_property::m_ts_service_name[256] = "PC GRABBER";
char                            it_usbd_property::m_ts_provider_name[256] = "ITE Tech. Inc.";
unsigned int                    it_usbd_property::m_ts_program_number(0x100);
unsigned int                    it_usbd_property::m_ts_pmt_pid(0x1000);
unsigned int                    it_usbd_property::m_ts_video_pid(0x7d1);
unsigned int                    it_usbd_property::m_ts_audio_pid(0x7d2);
unsigned int                    it_usbd_property::m_ts_stuffing_ratio(60);
unsigned int                    it_usbd_property::m_video_codec(0);
unsigned int                    it_usbd_property::m_video_bitrate(18600);
unsigned int                    it_usbd_property::m_video_dimension_width(1920);
unsigned int                    it_usbd_property::m_video_dimension_height(1080);
unsigned int                    it_usbd_property::m_video_max_framerate(0);
unsigned int                    it_usbd_property::m_video_framerate(60);
unsigned int                    it_usbd_property::m_video_gop_size(30);
unsigned int                    it_usbd_property::m_video_aspect_ratio(0);
//unsigned int                    it_usbd_property::m_audio_codec(AAC_AUDIO_ENCODER);
unsigned int                    it_usbd_property::m_audio_bitrate(192000);
unsigned int                    it_usbd_property::m_audio_sampling_rate(96000);
unsigned int                    it_usbd_property::m_mic_volume(0);
//unsigned int                    it_usbd_property::m_linein_boost(LINE_BOOST_0_DB);
unsigned int                    it_usbd_property::m_pc_mode(0);
unsigned char                   it_usbd_property::m_i2c_addr(0);
unsigned char                   it_usbd_property::m_i2c_num_bytes_to_write(0);
unsigned char                   it_usbd_property::m_i2c_num_bytes_to_read(0);
unsigned char                   it_usbd_property::m_i2c_data[255] = {0};
unsigned int                    it_usbd_property::m_video_source_index((unsigned int) -1);
unsigned int                    it_usbd_property::m_video_mode_index((unsigned int) -1);
unsigned int                    it_usbd_property::m_video_max_frame_rate2_mode_index((unsigned int) -1);
unsigned int                    it_usbd_property::m_video_max_frame_rate2_width((unsigned int) -1);
unsigned int                    it_usbd_property::m_video_max_frame_rate2_height((unsigned int) -1);

it_usbd_property::property_range    it_usbd_property::m_property_ranges[] =
{
    {ID_VIDEO_PROCAMP_BRIGHTNESS,           750,    750, -10000,  10000, "procamp.brightness"},
    {ID_VIDEO_PROCAMP_CONTRAST,             100,    100,      0,  10000, "procamp.contrast"},
    {ID_VIDEO_PROCAMP_HUE,                    0,      0,   -180,    180, "procamp.hue"},
    {ID_VIDEO_PROCAMP_SATURATION,           100,    100,      0,  10000, "procamp.saturation"},
    {ID_VIDEO_PROCAMP_GAIN,                   1,      1,      1,    500, "procamp.gain"},
    {ID_VIDEO_COMPRESSION_GETINFO,          750,    750, -10000,  10000, "procamp.getinfo"},
    {ID_VIDEO_COMPRESSION_KEYFRAME_RATE,    750,    750, -10000,  10000, "compression.keyframe_rate"},
    {ID_VIDEO_COMPRESSION_QUALITY,          750,    750, -10000,  10000, "compression.quality"},
    {0,                                       0,      0,      0,      0, ".end"},
};

extern "C" void idbAsycnDataSwitch(int asyncdataSwitch);
int
it_usbd_property::do_property(
        unsigned char                   (&data_in)[512],
        unsigned char                   (&data_out)[512])
{
    int                                 status;
    unsigned int                        cmd_sequence;

    typedef union
    {
        it_usbd_property::property                          property;
        it_usbd_property::property_state                    state;
        it_usbd_property::property_source                   source;
        it_usbd_property::property_firmware_size            firmware_size;
        it_usbd_property::property_firmware_data            firmware_data;
        it_usbd_property::property_video_procamp            video_procamp;
        it_usbd_property::property_video_compression_getinfo   video_compression_getinfo;
        it_usbd_property::property_video_compression        video_compression;
        it_usbd_property::property_debug_query_time         debug_query_time;
        it_usbd_property::property_asyncdata_switch         asyncdata_switch;
        unsigned char                                       data[512];
    } _data_in;

    typedef union
    {
        it_usbd_property::property                          property;
        it_usbd_property::property_state                    state;
        it_usbd_property::property_source                   source;
        it_usbd_property::property_firmware_size            firmware_size;
        it_usbd_property::property_firmware_data            firmware_data;
        it_usbd_property::property_firmware_status          firmware_status;
        it_usbd_property::property_video_procamp            video_procamp;
        it_usbd_property::property_video_compression_getinfo   video_compression_getinfo;
        it_usbd_property::property_video_compression        video_compression;
        it_usbd_property::property_debug_query_time         debug_query_time;
        unsigned char                                       data[512];
    } _data_out;

    _data_in&                            property_in((_data_in&)  *(_data_in*) data_in);
    _data_out&                           property_out((_data_out&) *(_data_out*) data_out);

    cmd_sequence = le32_to_cpu(property_in.property.status);
    if (m_cmd_sequence != cmd_sequence)
    {
        //printf("[X] cmd_seq %08X==>%08X \n", m_cmd_sequence, cmd_sequence);
    }
    m_cmd_sequence = 0x99100000UL | (++cmd_sequence & 0x0000ffffUL);

    switch (le32_to_cpu(property_in.property.id))
    {
        default:
l_unhandled:
            printf("[X] Unknown property!\n");
            printf("      id:    0X%08X\n", le32_to_cpu(property_in.property.id));
            printf("      flags: 0X%08X\n", le32_to_cpu(property_in.property.flags));
            printf("      size:  %d\n",     le32_to_cpu(property_in.property.size));
            property_out.property.size     = cpu_to_le32(sizeof(it_usbd_property::property));
            property_out.property.id       = property_in.property.id;
            property_out.property.flags    = property_in.property.flags;
            property_out.property.status   = cpu_to_le32((unsigned int) -1);
            status = -1;
            break;

#if defined(CONFIG_USBD_HAVE_PCGRABBER)
        case it_usbd_property::ID_PC_GRABBER:
            status = do_pc_grabber(property_in.property, property_out.property);
            break;
#endif

        case it_usbd_property::ID_REBOOT:
            do_reboot(property_in.property, property_out.property);
            property_out.property.size=0;
            break;

        case it_usbd_property::ID_PROFILE:
            status = do_profile(property_in.property, property_out.property);
            break;

#if defined(CONFIG_USBD_HAVE_PCGRABBER)
        case it_usbd_property::ID_STATE:
            status = it_usbd_property::do_state(property_in.state, property_out.state);
            break;

        case it_usbd_property::ID_SOURCE:
            status = it_usbd_property::do_source(property_in.source, property_out.source);
            break;

        case it_usbd_property::ID_BITRATE:
            /*
             *  TODO: handle bitrate
             */
            memcpy(property_out.data, property_in.data, le32_to_cpu(property_in.property.size));
            status = -1;
            break;
#endif

        case it_usbd_property::ID_FIRMWARE_SIZE:
            status = it_usbd_property::do_firmware_size(property_in.firmware_size, property_out.property);
            break;

        case it_usbd_property::ID_FIRMWARE_DATA:
            status = it_usbd_property::do_firmware_data(property_in.firmware_data, property_out.property);
            break;

        case it_usbd_property::ID_FIRMWARE_UPDATE:
            break;

        case it_usbd_property::ID_FIRMWARE_STATUS:
            status = it_usbd_property::do_firmware_status(property_in.property, property_out.firmware_status);
            break;

#if defined(CONFIG_USBD_HAVE_PCGRABBER)
        case it_usbd_property::ID_VIDEO_PROCAMP_BRIGHTNESS:
            do_video_procamp_brightness_property(
                property_in.video_procamp,
                property_out.video_procamp);
            break;
        case it_usbd_property::ID_VIDEO_PROCAMP_CONTRAST:
            do_video_procamp_contrast_property(
                property_in.video_procamp,
                property_out.video_procamp);
            break;
        case it_usbd_property::ID_VIDEO_PROCAMP_HUE:
            do_video_procamp_hue_property(
                property_in.video_procamp,
                property_out.video_procamp);
            break;
        case it_usbd_property::ID_VIDEO_PROCAMP_SATURATION:
            do_video_procamp_saturation_property(
                property_in.video_procamp,
                property_out.video_procamp);
            break;
        case it_usbd_property::ID_VIDEO_PROCAMP_GAIN:
            do_video_procamp_gain_property(
                property_in.video_procamp,
                property_out.video_procamp);
            break;

#if (0)
        case it_usbd_property::ID_VIDEO_PROCAMP_BRIGHTNESS:
        case it_usbd_property::ID_VIDEO_PROCAMP_CONTRAST:
        case it_usbd_property::ID_VIDEO_PROCAMP_HUE:
        case it_usbd_property::ID_VIDEO_PROCAMP_SATURATION:
        case it_usbd_property::ID_VIDEO_PROCAMP_GAIN:
            for (int i = 0; i < sizeof(m_property_ranges) / sizeof(m_property_ranges[0]); i++)
            {
                if (m_property_ranges[i].id == le32_to_cpu(property_in.property.id))
                {
                    status = it_usbd_property::do_video_procamp_property(
                        property_in.video_procamp,
                        property_out.video_procamp,
                        m_property_ranges[i]);
                    break;
                }
            }
            break;
#endif
        case it_usbd_property::ID_VIDEO_COMPRESSION_GETINFO:
            status = it_usbd_property::do_video_compression_getinfo_property(
                property_in.video_compression_getinfo,
                property_out.video_compression_getinfo);
            break;

        case it_usbd_property::ID_VIDEO_COMPRESSION_KEYFRAME_RATE:
            status = it_usbd_property::do_video_compression_keyframe_rate_property(
                property_in.video_compression,
                property_out.video_compression);
            break;

        case it_usbd_property::ID_VIDEO_COMPRESSION_QUALITY:
            status = it_usbd_property::do_video_compression_quality_property(
                property_in.video_compression,
                property_out.video_compression);
            break;
#endif
        case it_usbd_property::ID_DEBUG_QUERY_TIME:
            status = it_usbd_property::do_debug_query_time_property(
                property_in.debug_query_time,
                property_out.debug_query_time);
            break;

#if(0)
        case it_usbd_property::ID_HW_GRABBER:
            status = it_usbd_property::do_hw_grabber_property(
                property_in.property,
                property_out.property);
            break;
#endif
        case it_usbd_property::ID_ASYNCDATA_SWITCH:
        {
            idbAsycnDataSwitch(property_in.asyncdata_switch.asyncdataSwitch);
            property_out.property.size=0;
            break;
        }
    }

    return status;
}

#if defined(CONFIG_USBD_HAVE_PCGRABBER)
int
it_usbd_property::do_pc_grabber(
    property&                           _property_in,
    property&                           _property_out)
{
    property_pc_grabber_vt&             property_in((property_pc_grabber_vt&)_property_in);
    property_pc_grabber_vt&             property_out((property_pc_grabber_vt&)_property_out);
    int                                 status = 0;
    unsigned int                        flags;
    union {
        unsigned int                        subid;
        unsigned int                        acodec;
        unsigned int                        vcodec;
        unsigned int                        led_status;
        unsigned int                        interrupt_sate;
        unsigned int                        root;
        unsigned int                        hdcp;
        unsigned int                        digital_volume;
        unsigned int                        enable_record;
    };
    unsigned int                        size;
    union {
        unsigned int                        year;
        unsigned int                        hour;
    };
    union {
        unsigned int                        month;
        unsigned int                        minute;
    };
    union {
        unsigned int                        day;
        unsigned int                        second;
    };
    unsigned char                       hour8;
    unsigned char                       minute8;
    unsigned char                       second8;
    //it_auto_log                         auto_log(__FUNCTION__, (int*)&property_in.subid, &status);


    /*
     *  validation check
     */
    if (le32_to_cpu(property_in.size) < sizeof(property_pc_grabber))
    {
        status = -1;
        goto l_exit;
    }

    subid = le32_to_cpu(property_in.subid);

    /*
     *  check flags
     */
    flags = le32_to_cpu(property_in.flags);
    if (flags & it_usbd_property::FLAG_SET)
    {
        size = le32_to_cpu(property_in.size);
        switch (subid)
        {
            default:
l_default_set:
                printf("\[X] %s(SET) Unknown subid:0x%08X\n", __FUNCTION__, subid);
                status = -1;
                goto l_exit;

            case it_usbd_property::SUBID_PC_GRABBER_TS_SERVICE_NAME:
                strncpy(m_ts_service_name, property_in.str, sizeof(m_ts_service_name));
                m_ts_service_name[sizeof(m_ts_service_name) - 1] = 0;
                break;

            case it_usbd_property::SUBID_PC_GRABBER_TS_PROVIDER_NAME:
                strncpy(m_ts_provider_name, property_in.str, sizeof(m_ts_provider_name));
                m_ts_provider_name[sizeof(m_ts_provider_name) - 1] = 0;
                break;

            case it_usbd_property::SUBID_PC_GRABBER_TS_PROGRAM_NUMBER:
                m_ts_program_number = le32_to_cpu(property_in.u32[0]);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_TS_PMT_PID:
                m_ts_pmt_pid = le32_to_cpu(property_in.u32[0]);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_TS_AUDIO_PID:
                m_ts_audio_pid = le32_to_cpu(property_in.u32[0]);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_TS_VIDEO_PID:
                m_ts_video_pid = le32_to_cpu(property_in.u32[0]);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_TS_STUFFING_RATIO:
                m_ts_stuffing_ratio = le32_to_cpu(property_in.u32[0]);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_VIDEO_CODEC:
                vcodec = le32_to_cpu(property_in.u32[0]);
                switch (vcodec)
                {
                    default:
                        goto l_default_set;
                        break;

                    case IT_USBD_MK4CC('H', '2', '6', '4'):
                    case IT_USBD_MK4CC('A', 'V', 'C', 0):
                        m_video_codec = 0;
                        break;

                }
                break;

            case it_usbd_property::SUBID_PC_GRABBER_VIDEO_BITRATE:
                m_video_bitrate = le32_to_cpu(property_in.u32[0]);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_VIDEO_DIMENSION:
                m_video_dimension_width = le32_to_cpu(property_in.u32[0]);
                m_video_dimension_height = le32_to_cpu(property_in.u32[1]);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_VIDEO_FRAMERATE:
                m_video_framerate = le32_to_cpu(property_in.u32[0]);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_VIDEO_GOP_SIZE:
                m_video_gop_size = le32_to_cpu(property_in.u32[0]);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_VIDEO_ASPECT_RATIO:
                m_video_aspect_ratio = le32_to_cpu(property_in.u32[0]);
                coreSetAspectRatio((INPUT_ASPECT_RATIO) m_video_aspect_ratio);
                break;

				case it_usbd_property::SUBID_PC_GRABBER_AUDIO_CODEC:
                acodec = le32_to_cpu(property_in.u32[0]);
                switch (acodec)
                {
                    default:
                        goto l_default_set;
                        break;

                    case IT_USBD_MK4CC('M', 'P', 'E', 'G'):
                    case IT_USBD_MK4CC('M', 'P', '3', 0):
                    case IT_USBD_MK4CC('M', 'P', '2', 0):
                        m_audio_codec = MPEG_AUDIO_ENCODER;
printf("[!] m_audio_codec = MPEG_AUDIO_ENCODER\n");
                        break;

                    case IT_USBD_MK4CC('L', 'A', 'T', 'M'):
                    case IT_USBD_MK4CC('A', 'D', 'T', 'S'):
                    case IT_USBD_MK4CC('A', 'A', 'C', 0):
                        m_audio_codec = AAC_AUDIO_ENCODER;
printf("[!] m_audio_codec = AAC_AUDIO_ENCODER\n");
                        break;
                }
                break;

            case it_usbd_property::SUBID_PC_GRABBER_AUDIO_BITRATE:
                m_audio_bitrate = le32_to_cpu(property_in.u32[0]);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_AUDIO_SAMPLE_RATE:
                m_audio_sampling_rate = le32_to_cpu(property_in.u32[0]);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_MIC_VOLUME:
                m_mic_volume = le32_to_cpu(property_in.u32[0]);
                coreSetMicInVolStep(m_mic_volume);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_LINEIN_BOOST:
                m_linein_boost = le32_to_cpu(property_in.u32[0]);
                coreSetLineInBoost((LINE_BOOST) m_linein_boost);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_MODE:
printf("SUBID_PC_GRABBER_MODE(SET:%d:%d->%d):\n", size - sizeof(property_pc_grabber), m_pc_mode, le32_to_cpu(property_in.u32[0]));
                it_usbd_pcgrabber_base->set_pc_mode(le32_to_cpu(property_in.u32[0]));
                break;

            case it_usbd_property::SUBID_PC_GRABBER_BLINGRING_LED:
                led_status = le32_to_cpu(property_in.u32[0]);
                coreSetBlingRingLed((LED_STATUS) led_status);
printf("SUBID_PC_GRABBER_BLINGRING_LED(SET:%d:%d):\n", size - sizeof(property_pc_grabber), led_status);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_RED_LED:
                led_status = le32_to_cpu(property_in.u32[0]);
                coreSetRedLed((LED_STATUS) led_status);
printf("SUBID_PC_GRABBER_RED_LED(SET:%d:%d):\n", size - sizeof(property_pc_grabber), led_status);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_GREEN_LED:
                led_status = le32_to_cpu(property_in.u32[0]);
                coreSetGreenLed((LED_STATUS) led_status);
printf("SUBID_PC_GRABBER_GREEN_LED(SET:%d:%d):\n", size - sizeof(property_pc_grabber), led_status);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_BLUE_LED:
                led_status = le32_to_cpu(property_in.u32[0]);
                coreSetBlueLed((LED_STATUS) led_status);
printf("SUBID_PC_GRABBER_BLUE_LED(SET:%d:%d):\n", size - sizeof(property_pc_grabber), led_status);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_I2C:
printf("SUBID_PC_GRABBER_I2C(SET:%d):\n", size - sizeof(property_pc_grabber));
#if (1)
                status = coreCapsenseWriteI2C(property_in.u8, size - sizeof(property_pc_grabber));
                if (status) {
                    goto l_default_set;
                }
#endif
for (int i = 0; i < size - sizeof(property_pc_grabber); i++)
{
    if (!(i & 0x0f)) {
        printf("\n%02X: ", i);
    }
    else if (!(i & 0x07)) {
        printf(" - ");
    }
    printf("%02X ", property_in.u8[i]);
}
                break;

            case it_usbd_property::SUBID_PC_GRABBER_FW:
printf("SUBID_PC_GRABBER_FW(SET:%02X:%d):\n", property_in.fw.addr, size - offsetof(property_pc_grabber_vt, fw.data));
                if (size > offsetof(property_pc_grabber_vt, fw.data))
                {
                    status = coreCapsenseWriteFW(property_in.fw.addr, property_in.fw.data, size - offsetof(property_pc_grabber_vt, fw.data));
                    if (status) {
                        goto l_default_set;
                    }
for (int i = 0; i < size - offsetof(property_pc_grabber_vt, fw.data); i++)
{
    if (!(i & 0x0f)) {
        printf("\n%02X: ", i);
    }
    else if (!(i & 0x07)) {
        printf(" - ");
    }
    printf("%02X ", property_in.fw.data[i]);
}
printf("\n");
                }
                break;

            case it_usbd_property::SUBID_PC_GRABBER_DATE:
                year = le32_to_cpu(property_in.date.year);
                month = le32_to_cpu(property_in.date.month);
                day = le32_to_cpu(property_in.date.day);

                if (size - offsetof(property_pc_grabber_vt, date.year) >= sizeof(property_in.date))
                {
                    status = coreRtcSetDate(year, month, day);
                }
                else {
                    status = -1;
                }
printf("[%c] SUBID_PC_GRABBER_DATE(SET:%d:%d-%d-%d)=%d:\n",
    status ? 'X' : 'O',
    size - offsetof(property_pc_grabber_vt, date.year),
    year,
    month,
    day,
    status);
                if (status) {
                    goto l_default_set;
                }
                break;


            case it_usbd_property::SUBID_PC_GRABBER_TIME:
                hour = le32_to_cpu(property_in.time.hour);
                minute = le32_to_cpu(property_in.time.minute);
                second = le32_to_cpu(property_in.time.second);

                if (size - offsetof(property_pc_grabber_vt, time.hour) >= sizeof(property_in.time))
                {
                    status = coreRtcSetTime(hour, minute, second);
                }
                else {
                    status = -1;
                }
printf("[%c] SUBID_PC_GRABBER_TIME(SET:%d:%d-%d-%d)=%d:\n",
    status ? 'X' : 'O',
    size - offsetof(property_pc_grabber_vt, time.hour),
    hour,
    minute,
    second,
    status);
                if (status) {
                    goto l_default_set;
                }
                break;

            case it_usbd_property::SUBID_PC_GRABBER_ROOT:
                root = le32_to_cpu(property_in.u32[0]);
                coreSetRoot((MMP_BOOL) root);
printf("SUBID_PC_GRABBER_ROOT(SET:%d:%d):\n", size - sizeof(property_pc_grabber), root);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_DISABLE_HDCP:
                hdcp = le32_to_cpu(property_in.u32[0]);
                coreDisableHDCP((MMP_BOOL) hdcp);
printf("SUBID_PC_GRABBER_DISABLE_HDCP(SET:%d:%d):\n", size - sizeof(property_pc_grabber), hdcp);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_VIDEO_ENCODER_PARAMS:
            {
                VIDEO_ENCODER_PARAMETER     video_encoder_params;
                memset(&video_encoder_params, 0, sizeof(video_encoder_params));
                VIDEO_ENCODER_UPDATE_FLAGS  flags;

printf(
    "[!] SUBID_PC_GRABBER_VIDEO_ENCODER_PARAMS(SET:%d:%d)\n",
    size,
    size - sizeof(it_usbd_property::property_pc_grabber));

                if (size >= sizeof(it_usbd_property::property_pc_grabber) + sizeof(property_in.video_encoder_params)) {
                    m_video_source_index = le32_to_cpu(property_in.video_encoder_params.source_index);
                    m_video_mode_index = le32_to_cpu(property_in.video_encoder_params.mode_index);
                    flags = (VIDEO_ENCODER_UPDATE_FLAGS) le32_to_cpu(property_in.video_encoder_params.flags);
                    video_encoder_params.EnWidth = (MMP_UINT16) le32_to_cpu(property_in.video_encoder_params.width);
                    video_encoder_params.EnHeight = (MMP_UINT16) le32_to_cpu(property_in.video_encoder_params.height);
                    video_encoder_params.EnBitrate = (MMP_UINT16) le32_to_cpu(property_in.video_encoder_params.kbit_rate);
                    video_encoder_params.EnDeinterlaceOn = (MMP_BOOL) le32_to_cpu(property_in.video_encoder_params.deinterlace_on);
                    video_encoder_params.EnFrameDouble = (MMP_BOOL) le32_to_cpu(property_in.video_encoder_params.frame_double);
                    video_encoder_params.EnFrameRate = (MMP_UINT16) le32_to_cpu(property_in.video_encoder_params.frame_rate);
                    video_encoder_params.EnGOPSize = (MMP_UINT16) le32_to_cpu(property_in.video_encoder_params.gop_size);
                    video_encoder_params.EnAspectRatio = (ASPECT_RATIO) le32_to_cpu(property_in.video_encoder_params.aspect_ratio);
                    video_encoder_params.EnSkipMode = (VIDEO_ENCODER_SKIP_MODE) le32_to_cpu(property_in.video_encoder_params.skip_mode);
                    video_encoder_params.InstanceNum = (MMP_UINT16) le32_to_cpu(property_in.video_encoder_params.instance_num);
                    if (flags) {
                        unsigned int source_index = (m_video_source_index == -1) ? coreGetCaptureSource() : m_video_source_index;
                        unsigned int mode_index = (m_video_mode_index == -1) ? coreGetInputSrcInfo() : m_video_mode_index;
                        it_auto_log auto_log("coreSetVideoEnPara");
                        coreSetVideoEnPara((CAPTURE_VIDEO_SOURCE) source_index, flags, (VIDEO_ENCODER_INPUT_INFO) mode_index, &video_encoder_params);
                    }
                    status = 0;
                }
                else {
                    status = -1;
                }

                if (status) {
                    goto l_default_set;
                }
                break;
            }

            case it_usbd_property::SUBID_PC_GRABBER_VIDEO_MAX_FRAMERATE2:
            {
                if (size < sizeof(property_pc_grabber) + 3 * sizeof(property_in.u32[0]))
                {
                    goto l_default_get;
                }
                m_video_max_frame_rate2_mode_index = le32_to_cpu(property_in.u32[0]);
                m_video_max_frame_rate2_width = le32_to_cpu(property_in.u32[1]);
                m_video_max_frame_rate2_height = le32_to_cpu(property_in.u32[2]);
                status = 0;
                break;
            }

            case it_usbd_property::SUBID_PC_GRABBER_AUDIO_ENCODER_PARAMS:
            {
                CORE_AUDIO_ENCODE_PARA audio_encoder_params;
                memset(&audio_encoder_params, 0, sizeof(audio_encoder_params));

                printf(
                    "[%c] SUBID_PC_GRABBER_AUDIO_ENCODER_PARAMS(SET:%d:%d)\n"
                    "    codec_type: %d\n"
                    "    bit_rate:   %d\n"
                    "    sampl_rate: %d\n",
                    status ? 'X' : 'O',
                    size,
                    size - sizeof(it_usbd_property::property_pc_grabber),
                    le32_to_cpu(property_in.audio_encoder_params.codec_type),
                    le32_to_cpu(property_in.audio_encoder_params.bit_rate),
                    le32_to_cpu(property_in.audio_encoder_params.sample_rate));

                if (size >= sizeof(it_usbd_property::property_pc_grabber) + sizeof(property_in.audio_encoder_params)) {
                    audio_encoder_params.audioEncoderType = (AUDIO_ENCODER_TYPE) le32_to_cpu(property_in.audio_encoder_params.codec_type);
                    audio_encoder_params.bitRate = (MMP_UINT32) le32_to_cpu(property_in.audio_encoder_params.bit_rate);
                    {
                        it_auto_log auto_log("coreSetAudioEncodeParameter");
                        coreSetAudioEncodeParameter(&audio_encoder_params);
                        m_audio_codec = le32_to_cpu(property_in.audio_encoder_params.codec_type);
                        m_audio_bitrate = le32_to_cpu(property_in.audio_encoder_params.bit_rate);
                        m_audio_sampling_rate = le32_to_cpu(property_in.audio_encoder_params.sample_rate);
                    }
                    status = 0;
                }
                else {
                    status = -1;
                }


                if (status) {
                    goto l_default_set;
                }
                break;
            }

            case it_usbd_property::SUBID_PC_GRABBER_DIGITAL_VOLUME:
            {
                digital_volume = le32_to_cpu(property_in.u32[0]);
                coreSetDigitalVolume((DIGITAL_AUDIO_VOLUME) digital_volume);
printf("SUBID_PC_GRABBER_DIGITAL_VOLUME(SET:%d:%d):\n", size - sizeof(property_pc_grabber), digital_volume);
                break;
            }

            case it_usbd_property::SUBID_PC_GRABBER_ENABLE_RECORD:
            {
                enable_record = le32_to_cpu(property_in.u32[0]);
                enable_record = !!enable_record;
                coreSetPCModeEnableRecord((MMP_BOOL) enable_record);
printf("SUBID_PC_GRABBER_ENABLE_RECORD(SET:%d:%d):\n", size - sizeof(property_pc_grabber), enable_record);
            }
            break;

#if (0)
            case it_usbd_property::SUBID_PC_GRABBER_GENERIC_I2C:
                /*
                 *  just defer the request and return OK
                 */
printf(
    "[!] SUBID_PC_GRABBER_GENERIC_I2C(SET addr %d:%d, nw: %d:%d, nr: %d:%d)!\n",
    m_i2c_addr, property_in.i2c.addr,
    m_i2c_num_bytes_to_write, property_in.i2c.num_bytes_to_write,
    m_i2c_num_bytes_to_read, property_in.i2c.num_bytes_to_read);
                m_i2c_addr = property_in.i2c.addr;
                m_i2c_num_bytes_to_write = property_in.i2c.num_bytes_to_write;
                m_i2c_num_bytes_to_read = property_in.i2c.num_bytes_to_read;
                memcpy(m_i2c_data, property_in.i2c.data, m_i2c_num_bytes_to_write);
                status = 0;
                break;
#endif
        }
    }
    else if (flags & it_usbd_property::FLAG_GET)
    {
        size = le32_to_cpu(property_in.size);
        if ((size < sizeof(property_pc_grabber)) || (size > 512))
        {
            printf("\[X] %s(GET) size(%d) invalid!\n", __FUNCTION__, size);
            status = -1;
            goto l_exit;
        }

        switch (subid)
        {
            default:
l_default_get:
                printf("\[X] %s(GET) Unknown subid:0x%08X\n", __FUNCTION__, subid);
        status = -1;
        goto l_exit;

            case it_usbd_property::SUBID_PC_GRABBER_VERSION_INFO:
                size -= sizeof(property_pc_grabber);
                if (size < 1)
                {
                    goto l_default_get;
                }
                snprintf(property_out.str, size, "%d.%d.%d.%d.%d.(%s %s)", CUSTOMER_CODE, PROJECT_CODE, SDK_MAJOR_VERSION, SDK_MINOR_VERSION, BUILD_NUMBER, __TIME__, __DATE__);
                property_out.str[size - 1] = 0;
                size = sizeof(property_pc_grabber) + strlen(property_out.str) + 1;
                break;

            case it_usbd_property::SUBID_PC_GRABBER_SOURCE_INFO:
                {
                size -= sizeof(property_pc_grabber);
                printf("[!] size=%d\n", size);
                if (size < sizeof(property_out.source_info))
                {
                    goto l_default_get;
                }
                printf("[!] size=%d\n", size);
                INPUT_VIDEO_SRC_INFO info;
                int ret = coreGetActiveVideoInfo(&info);
                printf("[!] ret=%d\n", ret);
                if (ret) {
                    //goto l_default_get;
                }

                property_out.source_info.input = info.vidDevice;
                property_out.source_info.width = info.vidWidth;
                property_out.source_info.height = info.vidHeight;
                property_out.source_info.frame_rate = info.vidFrameRate;
                property_out.source_info.interlaced = !!info.bInterlaceSrc;
                property_out.source_info.aspect_ratio = coreGetInputAspectRatio();
                property_out.source_info.hdcp = !!coreGetIsContentProtection();
                property_out.source_info.hdmi_audio_format = coreGetHDMIAudioMode();
printf(
    "[!] input: %d\n"
    "[!] width: %d\n"
    "[!] height: %d\n"
    "[!] frame_rate: %d\n"
    "[!] interlaced: %d\n"
    "[!] aspect_ratio: %d\n"
    "[!] hdcp: %d\n"
    "[!] hdmi_audio_format: %d\n",
    property_out.source_info.input,
    property_out.source_info.width,
    property_out.source_info.height,
    property_out.source_info.frame_rate,
    property_out.source_info.interlaced,
    property_out.source_info.aspect_ratio,
    property_out.source_info.hdcp,
    property_out.source_info.hdmi_audio_format);
    
    
                property_out.source_info.input = cpu_to_le32(property_out.source_info.input);
                property_out.source_info.width = cpu_to_le32(property_out.source_info.width);
                property_out.source_info.height = cpu_to_le32(property_out.source_info.height);
                property_out.source_info.frame_rate = cpu_to_le32(property_out.source_info.frame_rate);
                property_out.source_info.interlaced = cpu_to_le32(property_out.source_info.interlaced);
                property_out.source_info.aspect_ratio = cpu_to_le32(property_out.source_info.aspect_ratio);
                property_out.source_info.hdcp = cpu_to_le32(property_out.source_info.hdcp);
                property_out.source_info.hdmi_audio_format = cpu_to_le32(property_out.source_info.hdmi_audio_format);
                
                size = sizeof(property_pc_grabber) + sizeof(property_out.source_info);
                printf("[!] size=%d\n", size);
                }
                break;

            case it_usbd_property::SUBID_PC_GRABBER_TS_SERVICE_NAME:
                size -= sizeof(property_pc_grabber);
                if (size < 1)
                {
                    goto l_default_get;
                }
                strncpy(property_out.str, m_ts_service_name, size);
                property_out.str[size - 1] = 0;
                size = sizeof(property_pc_grabber) + strlen(property_out.str) + 1;
                break;

            case it_usbd_property::SUBID_PC_GRABBER_TS_PROVIDER_NAME:
                size -= sizeof(property_pc_grabber);
                if (size < 1)
                {
                    goto l_default_get;
                }
                strncpy(property_out.str, m_ts_provider_name, size);
                property_out.str[size - 1] = 0;
                size = sizeof(property_pc_grabber) + strlen(property_out.str) + 1;
                break;

            case it_usbd_property::SUBID_PC_GRABBER_TS_PROGRAM_NUMBER:
                if (size < sizeof(property_pc_grabber) + sizeof(property_out.u32[0]))
                {
                    goto l_default_get;
                }
                property_out.u32[0] = cpu_to_le32(m_ts_program_number);
                size = sizeof(property_pc_grabber) + sizeof(property_out.u32[0]);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_TS_PMT_PID:
                if (size < sizeof(property_pc_grabber) + sizeof(property_out.u32[0]))
                {
                    goto l_default_get;
                }
                property_out.u32[0] = cpu_to_le32(m_ts_pmt_pid);
                size = sizeof(property_pc_grabber) + sizeof(property_out.u32[0]);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_TS_AUDIO_PID:
                if (size < sizeof(property_pc_grabber) + sizeof(property_out.u32[0]))
                {
                    goto l_default_get;
                }
                property_out.u32[0] = cpu_to_le32(m_ts_audio_pid);
                size = sizeof(property_pc_grabber) + sizeof(property_out.u32[0]);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_TS_VIDEO_PID:
                if (size < sizeof(property_pc_grabber) + sizeof(property_out.u32[0]))
                {
                    goto l_default_get;
                }
                property_out.u32[0] = cpu_to_le32(m_ts_video_pid);
                size = sizeof(property_pc_grabber) + sizeof(property_out.u32[0]);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_TS_STUFFING_RATIO:
                if (size < sizeof(property_pc_grabber) + sizeof(property_out.u32[0]))
                {
                    goto l_default_get;
                }
                property_out.u32[0] = cpu_to_le32(m_ts_stuffing_ratio);
                size = sizeof(property_pc_grabber) + sizeof(property_out.u32[0]);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_VIDEO_CODEC:
                if (size < sizeof(property_pc_grabber) + sizeof(property_out.u32[0]))
                {
                    goto l_default_get;
                }
                vcodec = IT_USBD_MK4CC('H', '2', '6', '4');
                property_out.u32[0] = cpu_to_le32(vcodec);
                size = sizeof(property_pc_grabber) + sizeof(property_out.u32[0]);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_VIDEO_BITRATE:
                if (size < sizeof(property_pc_grabber) + sizeof(property_out.u32[0]))
                {
                    goto l_default_get;
                }
                property_out.u32[0] = cpu_to_le32(m_video_bitrate);
                size = sizeof(property_pc_grabber) + sizeof(property_out.u32[0]);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_VIDEO_DIMENSION:
                if (size < sizeof(property_pc_grabber) + sizeof(property_out.u32[0]) * 2)
                {
                    goto l_default_get;
                }
                property_out.u32[0] = cpu_to_le32(m_video_dimension_width);
                property_out.u32[1] = cpu_to_le32(m_video_dimension_height);
                size = sizeof(property_pc_grabber) + sizeof(property_out.u32[0]) * 2;
                break;

            case it_usbd_property::SUBID_PC_GRABBER_VIDEO_FRAMERATE:
                if (size < sizeof(property_pc_grabber) + sizeof(property_out.u32[0]))
                {
                    goto l_default_get;
                }
                if (!m_video_framerate)
                {
                    m_video_framerate = m_video_max_framerate = coreGetMaxFrameRate(coreGetInputSrcInfo(), m_video_dimension_width, m_video_dimension_height);
                }
                property_out.u32[0] = cpu_to_le32(m_video_framerate);
                size = sizeof(property_pc_grabber) + sizeof(property_out.u32[0]);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_VIDEO_MAX_FRAMERATE:
                if (size < sizeof(property_pc_grabber) + sizeof(property_out.u32[0]))
                {
                    goto l_default_get;
                }
                m_video_max_framerate = coreGetMaxFrameRate(coreGetInputSrcInfo(), m_video_dimension_width, m_video_dimension_height);
                property_out.u32[0] = cpu_to_le32(m_video_max_framerate);
                size = sizeof(property_pc_grabber) + sizeof(property_out.u32[0]);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_VIDEO_MAX_FRAMERATE2:
                if (size < sizeof(property_pc_grabber) + sizeof(property_out.u32[0]))
                {
                    goto l_default_get;
                }
                m_video_max_framerate = coreGetMaxFrameRate(
                                            (VIDEO_ENCODER_INPUT_INFO) m_video_max_frame_rate2_mode_index,
                                            m_video_max_frame_rate2_width,
                                            m_video_max_frame_rate2_height);
                property_out.u32[0] = cpu_to_le32(m_video_max_framerate);
                size = sizeof(property_pc_grabber) + sizeof(property_out.u32[0]);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_VIDEO_GOP_SIZE:
                if (size < sizeof(property_pc_grabber) + sizeof(property_out.u32[0]))
                {
                    goto l_default_get;
                }
                property_out.u32[0] = cpu_to_le32(m_video_gop_size);
                size = sizeof(property_pc_grabber) + sizeof(property_out.u32[0]);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_VIDEO_ASPECT_RATIO:
                if (size < sizeof(property_pc_grabber) + sizeof(property_out.u32[0]))
                {
                    goto l_default_get;
                }
                m_video_aspect_ratio = coreGetAspectRatio();
                property_out.u32[0] = cpu_to_le32(m_video_aspect_ratio);
                size = sizeof(property_pc_grabber) + sizeof(property_out.u32[0]);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_AUDIO_CODEC:
                if (size < sizeof(property_pc_grabber) + sizeof(property_out.u32[0]))
                {
                    goto l_default_get;
                }
                if (m_audio_codec == AAC_AUDIO_ENCODER)
                {
                    acodec = IT_USBD_MK4CC('A', 'A', 'C', 0);
                }
                else
                {
                    acodec = IT_USBD_MK4CC('M', 'P', '2', 0);
                }
                
                property_out.u32[0] = cpu_to_le32(acodec);
                size = sizeof(property_pc_grabber) + sizeof(property_out.u32[0]);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_AUDIO_BITRATE:
                if (size < sizeof(property_pc_grabber) + sizeof(property_out.u32[0]))
                {
                    goto l_default_get;
                }
                property_out.u32[0] = cpu_to_le32(m_audio_bitrate);
                size = sizeof(property_pc_grabber) + sizeof(property_out.u32[0]);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_AUDIO_SAMPLE_RATE:
                if (size < sizeof(property_pc_grabber) + sizeof(property_out.u32[0]))
                {
                    goto l_default_get;
                }
                property_out.u32[0] = cpu_to_le32(m_audio_sampling_rate);
                size = sizeof(property_pc_grabber) + sizeof(property_out.u32[0]);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_MIC_VOLUME:
                if (size < sizeof(property_pc_grabber) + sizeof(property_out.u32[0]))
                {
                    goto l_default_get;
                }
                m_mic_volume = coreGetMicInVolStep();
                property_out.u32[0] = cpu_to_le32(m_mic_volume);
                size = sizeof(property_pc_grabber) + sizeof(property_out.u32[0]);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_LINEIN_BOOST:
                if (size < sizeof(property_pc_grabber) + sizeof(property_out.u32[0]))
                {
                    goto l_default_get;
                }
                m_linein_boost = coreGetLineInBoost();
                property_out.u32[0] = cpu_to_le32(m_linein_boost);
                size = sizeof(property_pc_grabber) + sizeof(property_out.u32[0]);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_MODE:
                if (size < sizeof(property_pc_grabber) + sizeof(property_out.u32[0]))
                {
                    goto l_default_get;
                }
                property_out.u32[0] = cpu_to_le32(m_pc_mode);
                size = sizeof(property_pc_grabber) + sizeof(property_out.u32[0]);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_BLINGRING_LED:
                if (size < sizeof(property_pc_grabber) + sizeof(property_out.u32[0]))
                {
                    goto l_default_get;
                }
                led_status = coreGetBlingRingLed();
                property_out.u32[0] = cpu_to_le32(led_status);
                size = sizeof(property_pc_grabber) + sizeof(property_out.u32[0]);
printf("SUBID_PC_GRABBER_BLINGRING_LED(GET:%d:%d):\n", size - sizeof(property_pc_grabber), led_status);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_RED_LED:
                if (size < sizeof(property_pc_grabber) + sizeof(property_out.u32[0]))
                {
                    goto l_default_get;
                }
                led_status = coreGetRedLed();
                property_out.u32[0] = cpu_to_le32(led_status);
                size = sizeof(property_pc_grabber) + sizeof(property_out.u32[0]);
printf("SUBID_PC_GRABBER_RED_LED(GET:%d:%d):\n", size - sizeof(property_pc_grabber), led_status);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_GREEN_LED:
                if (size < sizeof(property_pc_grabber) + sizeof(property_out.u32[0]))
                {
                    goto l_default_get;
                }
                led_status = coreGetGreenLed();
                property_out.u32[0] = cpu_to_le32(led_status);
                size = sizeof(property_pc_grabber) + sizeof(property_out.u32[0]);
printf("SUBID_PC_GRABBER_GREEN_LED(GET:%d:%d):\n", size - sizeof(property_pc_grabber), led_status);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_BLUE_LED:
                if (size < sizeof(property_pc_grabber) + sizeof(property_out.u32[0]))
                {
                    goto l_default_get;
                }
                led_status = coreGetBlueLed();
                property_out.u32[0] = cpu_to_le32(led_status);
                size = sizeof(property_pc_grabber) + sizeof(property_out.u32[0]);
printf("SUBID_PC_GRABBER_BLUE_LED(GET:%d:%d):\n", size - sizeof(property_pc_grabber), led_status);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_I2C:
                printf("SUBID_PC_GRABBER_I2C(GET:%d):\n", size - sizeof(property_pc_grabber));
#if (1)
                status = coreCapsenseReadI2C(property_out.u8, size - sizeof(property_pc_grabber));
                if (status)
                {
                    goto l_default_get;
                }

                for (int i = 0; i < size - sizeof(property_pc_grabber); i++)
                {
                    if (!(i & 0x0f)) {
                        printf("\n%02X: ", i);
                    }
                    else if (!(i & 0x07)) {
                        printf(" - ");
                    }
                    printf("%02X ", property_out.u8[i]);
                }
#else
                for (int i = 0; i < size - sizeof(property_pc_grabber); i++)
                {
                    property_out.u8[i] = 'A' + (i % 26);
                }
#endif
                break;

            case it_usbd_property::SUBID_PC_GRABBER_FW:
printf("SUBID_PC_GRABBER_FW(GET:%d):\n", size - offsetof(property_pc_grabber_vt, fw.data));
                if (size > offsetof(property_pc_grabber_vt, fw.data))
                {
                    status = coreCapsenseReadFW(property_out.fw.data, size - offsetof(property_pc_grabber_vt, fw.data));
                    if (status)
                    {
                        goto l_default_get;
                    }
for (int i = 0; i < size - offsetof(property_pc_grabber_vt, fw.data); i++)
{
    if (!(i & 0x0f)) {
        printf("\n%02X: ", i);
    }
    else if (!(i & 0x07)) {
        printf(" - ");
    }
    printf("%02X ", property_out.fw.data[i]);
}
printf("\n");
                }
                break;

            case it_usbd_property::SUBID_PC_GRABBER_INTERRUPT_STATE:
                if (size < sizeof(property_pc_grabber) + sizeof(property_out.u32[0]))
                {
                    goto l_default_get;
                }
                interrupt_sate = coreCapsenseInterruptState();
                property_out.u32[0] = cpu_to_le32(interrupt_sate);
                size = sizeof(property_pc_grabber) + sizeof(property_out.u32[0]);
printf("SUBID_PC_GRABBER_INTERRUPT_STATE(GET:%d)=%d\n", size - sizeof(property_pc_grabber), interrupt_sate);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_DATE:
                status = coreRtcGetDate(&year, &month, &day);

printf("[%c] SUBID_PC_GRABBER_DATE(GET:%d:%d-%d-%d)=%d:\n",
    status ? 'X' : 'O',
    size - offsetof(property_pc_grabber_vt, date.year),
    year,
    month,
    day,
    status);

                if (status) {
                    goto l_default_set;
                }
                property_out.date.year = cpu_to_le32(year);
                property_out.date.month = cpu_to_le32(month);
                property_out.date.day = cpu_to_le32(day);
                size = sizeof(property_pc_grabber) + sizeof(property_out.date);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_TIME:
                status = coreRtcGetTime(&hour8, &minute8, &second8);
                hour = hour8;
                minute = minute8;
                second = second8;

printf("[%c] SUBID_PC_GRABBER_TIME(GET:%d:%d-%d-%d)=%d:\n",
    status ? 'X' : 'O',
    size - offsetof(property_pc_grabber_vt, time.hour),
    hour,
    minute,
    second,
    status);

                if (status) {
                    goto l_default_set;
                }
                property_out.time.hour = cpu_to_le32(hour);
                property_out.time.minute = cpu_to_le32(minute);
                property_out.time.second = cpu_to_le32(second);
                size = sizeof(property_pc_grabber) + sizeof(property_out.time);
                break;

            case it_usbd_property::SUBID_PC_GRABBER_VIDEO_ENCODER_PARAMS:
            {
                VIDEO_ENCODER_PARAMETER video_encoder_params;
                unsigned int source_index = (m_video_source_index == -1) ? coreGetCaptureSource() : m_video_source_index;
                unsigned int mode_index = (m_video_mode_index == -1) ? coreGetInputSrcInfo() : m_video_mode_index;

                memset(&video_encoder_params, 0, sizeof(video_encoder_params));

                if (size >= sizeof(it_usbd_property::property_pc_grabber) + sizeof(property_out.video_encoder_params)) {
                    it_auto_log auto_log("coreGetVideoEnPara");

                    coreGetVideoEnPara(
                        (CAPTURE_VIDEO_SOURCE) source_index,
                        (VIDEO_ENCODER_INPUT_INFO) mode_index,
                        &video_encoder_params);
                }
                else {
                    status = -1;
                }

                if (status) {
                    goto l_default_set;
                }

                property_out.video_encoder_params.source_index = cpu_to_le32((unsigned int) source_index);
                property_out.video_encoder_params.mode_index = cpu_to_le32((unsigned int) mode_index);
                property_out.video_encoder_params.flags = cpu_to_le32((unsigned int) 0);
                property_out.video_encoder_params.width = cpu_to_le32((unsigned int) video_encoder_params.EnWidth);
                property_out.video_encoder_params.height = cpu_to_le32((unsigned int) video_encoder_params.EnHeight);
                property_out.video_encoder_params.kbit_rate = cpu_to_le32((unsigned int) video_encoder_params.EnBitrate);
                property_out.video_encoder_params.deinterlace_on = cpu_to_le32((unsigned int) video_encoder_params.EnDeinterlaceOn);
                property_out.video_encoder_params.frame_double = cpu_to_le32((unsigned int) video_encoder_params.EnFrameDouble);
                property_out.video_encoder_params.frame_rate = cpu_to_le32((unsigned int) video_encoder_params.EnFrameRate);
                property_out.video_encoder_params.gop_size = cpu_to_le32((unsigned int) video_encoder_params.EnGOPSize);
                property_out.video_encoder_params.aspect_ratio = cpu_to_le32((unsigned int) video_encoder_params.EnAspectRatio);
                property_out.video_encoder_params.skip_mode = cpu_to_le32((unsigned int) video_encoder_params.EnSkipMode);
                property_out.video_encoder_params.instance_num = cpu_to_le32((unsigned int) video_encoder_params.InstanceNum);
                size = sizeof(it_usbd_property::property_pc_grabber) + sizeof(property_out.video_encoder_params);

                printf(
                    "[%c] SUBID_PC_GRABBER_VIDEO_ENCODER_PARAMS(GET:%d:%d)\n"
                    "    source_index:   %d\n"
                    "    mode_index:     %d\n"
                    "    flags:          %d\n"
                    "    width:          %d\n"
                    "    height:         %d\n"
                    "    bit_rate:       %d\n"
                    "    deinterlace_on: %d\n"
                    "    frame_double:   %d\n"
                    "    frame_rate:     %d\n"
                    "    gop_size:       %d\n"
                    "    aspect_ratio:   %d\n"
                    "    skip_mode:      %d\n"
                    "    instance_num:   %d\n",
                    status ? 'X' : 'O',
                    size,
                    size - sizeof(it_usbd_property::property_pc_grabber),
                    le32_to_cpu(property_out.video_encoder_params.source_index),
                    le32_to_cpu(property_out.video_encoder_params.mode_index),
                    le32_to_cpu(property_out.video_encoder_params.flags),
                    le32_to_cpu(property_out.video_encoder_params.width),
                    le32_to_cpu(property_out.video_encoder_params.height),
                    le32_to_cpu(property_out.video_encoder_params.kbit_rate),
                    le32_to_cpu(property_out.video_encoder_params.deinterlace_on),
                    le32_to_cpu(property_out.video_encoder_params.frame_double),
                    le32_to_cpu(property_out.video_encoder_params.frame_rate),
                    le32_to_cpu(property_out.video_encoder_params.gop_size),
                    le32_to_cpu(property_out.video_encoder_params.aspect_ratio),
                    le32_to_cpu(property_out.video_encoder_params.skip_mode),
                    le32_to_cpu(property_out.video_encoder_params.instance_num));
                break;
            }

            case it_usbd_property::SUBID_PC_GRABBER_AUDIO_ENCODER_PARAMS:
            {
                CORE_AUDIO_ENCODE_PARA audio_encoder_params;
                memset(&audio_encoder_params, 0, sizeof(audio_encoder_params));

                if (size >= sizeof(it_usbd_property::property_pc_grabber) + sizeof(property_out.audio_encoder_params)) {
                    it_auto_log auto_log("coreGetAudioEncodeParameter");
                    coreGetAudioEncodeParameter(&audio_encoder_params);
                    m_audio_codec = audio_encoder_params.audioEncoderType;
                    m_audio_bitrate = audio_encoder_params.bitRate;
                    m_audio_sampling_rate = 0;
                }
                else {
                    status = -1;
                }

                if (status) {
                    goto l_default_set;
                }

                property_out.audio_encoder_params.codec_type = cpu_to_le32(m_audio_codec);
                property_out.audio_encoder_params.bit_rate = cpu_to_le32(m_audio_bitrate);
                property_out.audio_encoder_params.sample_rate = (MMP_UINT32) cpu_to_le32(m_audio_sampling_rate);
                size = sizeof(it_usbd_property::property_pc_grabber) + sizeof(property_out.audio_encoder_params);
                printf(
                    "[%c] SUBID_PC_GRABBER_AUDIO_ENCODER_PARAMS(GET:%d:%d)\n"
                    "    codec_type: %d\n"
                    "    bit_rate:   %d\n"
                    "    sampl_rate: %d\n",
                    status ? 'X' : 'O',
                    size,
                    size - sizeof(it_usbd_property::property_pc_grabber),
                    le32_to_cpu(property_out.audio_encoder_params.codec_type),
                    le32_to_cpu(property_out.audio_encoder_params.bit_rate),
                    le32_to_cpu(property_out.audio_encoder_params.sample_rate));
                break;
            }

            case it_usbd_property::SUBID_PC_GRABBER_DIGITAL_VOLUME:
            {
                if (size < sizeof(property_pc_grabber) + sizeof(property_out.u32[0]))
                {
                    goto l_default_get;
                }
                digital_volume = coreGetDigitalVolume();
                property_out.u32[0] = cpu_to_le32(digital_volume);
                size = sizeof(property_pc_grabber) + sizeof(property_out.u32[0]);
printf("SUBID_PC_GRABBER_DIGITAL_VOLUME(GET:%d:%d):\n", size - sizeof(property_pc_grabber), digital_volume);
            }
            break;

            case it_usbd_property::SUBID_PC_GRABBER_ENABLE_RECORD:
            {
                MMP_BOOL value;

                if (size < sizeof(property_pc_grabber) + sizeof(property_out.u32[0]))
                {
                    goto l_default_get;
                }
                coreGetPCModeEnableRecord(&value);
                enable_record = !!value;
                property_out.u32[0] = cpu_to_le32(enable_record);
                size = sizeof(property_pc_grabber) + sizeof(property_out.u32[0]);
printf("SUBID_PC_GRABBER_ENABLE_RECORD(GET:%d:%d):\n", size - sizeof(property_pc_grabber), enable_record);
            }
            break;

            case it_usbd_property::SUBID_PC_GRABBER_DISABLE_HDCP:
            {
                MMP_BOOL value;

                if (size < sizeof(property_pc_grabber) + sizeof(property_out.u32[0]))
                {
                    goto l_default_get;
                }
                value = coreIsDisableHDCP();
                hdcp = !!value;
                property_out.u32[0] = cpu_to_le32(hdcp);
                size = sizeof(property_pc_grabber) + sizeof(property_out.u32[0]);

printf("SUBID_PC_GRABBER_DISABLE_HDCP(GET:%d:%d):\n", size - sizeof(property_pc_grabber), hdcp);
            }
            break;

#if (0)
            case it_usbd_property::SUBID_PC_GRABBER_GENERIC_I2C:
                property_out.i2c.num_bytes_to_write = m_i2c_addr;
                property_out.i2c.num_bytes_to_write = m_i2c_num_bytes_to_write;
                if (m_i2c_num_bytes_to_write)
                {
                    /*
                     *  TODO: call i2c write routine
                     */
                    memcpy(property_out.i2c.data, m_i2c_data, m_i2c_num_bytes_to_write);
                    property_out.i2c.num_bytes_written = m_i2c_num_bytes_to_write;
                }
                else
                {
                    property_out.i2c.num_bytes_written = 0;
                }

                property_out.i2c.num_bytes_to_read = m_i2c_num_bytes_to_read;
                if (m_i2c_num_bytes_to_read)
                {
                    /*
                     *  TODO: call i2c write routine
                     */
                    static unsigned char data[255] = {0};
                    property_out.i2c.num_bytes_read = m_i2c_num_bytes_to_read;
                    for (unsigned char i = 0; i < m_i2c_num_bytes_to_read; i++)
                    {
                        data[i] = 0x80 + i;
                    }
                    memcpy(property_out.i2c.data + m_i2c_num_bytes_to_write, data, m_i2c_num_bytes_to_read);
                }
                else
                {
                    property_out.i2c.num_bytes_read = 0;
                }
                size = offsetof(property_pc_grabber_vt, i2c.data) + m_i2c_num_bytes_to_write + property_out.i2c.num_bytes_read;
                if (size > 255) {
printf("[W] SUBID_PC_GRABBER_GENERIC_I2C() size %d ==> %d\n", size, 255);
                    size = 255;
                }
                m_i2c_addr = 0;
                m_i2c_num_bytes_to_write = 0;
                m_i2c_num_bytes_to_read = 0;
                break;
#endif
        }
    }
    else
    {
        printf("%s() invalid flags (0x%08X)!\n", __FUNCTION__, flags);
        status = -1;
        goto l_exit;
    }

    /*
     *  everything is ok!
     */
    status = 0;

#if (0)
    printf("\n\n"
        "=======================================\n"
        "version            :   \"%d.%d.%d.%d.%d\"\n"
        "service_name       :   \"%s\"\n"
        "provider_name      :   \"%s\"\n"
        "program_number     :   %-4d (0x%03X)\n"
        "pmt_pid            :   %-4d (0x%03X)\n"
        "video_pid          :   %-4d (0x%03X)\n"
        "audio_pid          :   %-4d (0x%03X)\n"
        "stuffing_ratio     :   %d%%\n"
        "video codec        :   %d (%s)\n"
        "video bitrate      :   %dKbit\n"
        "video dimension    :   %d x %d\n"
        "audio codec        :   %d (%s)\n"
        "audio sampling rate:   %dHz\n"
        "=======================================\n\n\n",
        CUSTOMER_CODE, PROJECT_CODE, SDK_MAJOR_VERSION, SDK_MINOR_VERSION, BUILD_NUMBER,
        m_ts_service_name,
        m_ts_provider_name,
        m_ts_program_number, m_ts_program_number,
        m_ts_pmt_pid, m_ts_pmt_pid,
        m_ts_video_pid, m_ts_video_pid,
        m_ts_audio_pid, m_ts_audio_pid,
        m_ts_stuffing_ratio,
        m_video_codec, m_video_codec ? "???" : "H264",
        m_video_bitrate,
        m_video_dimension_width, m_video_dimension_height,
        m_audio_codec, (m_audio_codec == AAC_AUDIO_ENCODER) ? "AAC" : ((m_audio_codec == MPEG_AUDIO_ENCODER) ? "MPEG" : "???"),
        m_audio_sampling_rate);
#endif

l_exit:
    property_out.size    = cpu_to_le32(size);
    property_out.id      = cpu_to_le32(it_usbd_property::ID_PC_GRABBER);
    property_out.flags   = status ? 0 : property_in.flags;
    property_out.status  = cpu_to_le32((unsigned int) status);
    property_out.subid   = cpu_to_le32(subid);

    return status;
}
#endif

void
it_usbd_property::do_reboot(
    property&                           property_in,
    property&                           property_out)
{
    it_auto_log                         auto_log(__FUNCTION__);

	ithPrintf("reboot...\n");
	exit(0);
}

int
it_usbd_property::do_profile(
    property&                           _property_in,
    property&                           _property_out)
{
    property_profile&                   property_in((property_profile&)_property_in);
    property_profile&                   property_out((property_profile&)_property_out);
    int                                 status;
    unsigned int                        flags;
    unsigned int                        state;
    unsigned int                        size;
    unsigned int                        profile = 0x1234aa00;
    it_auto_log                         auto_log(__FUNCTION__);


    /*
     *  validation check
     */
    if (le32_to_cpu(property_in.property.size) < sizeof(it_usbd_property::property))
    {
        status = -1;
        goto l_exit;
    }

    /*
     *  check flags
     */
    flags = le32_to_cpu(property_in.property.flags);
    if (!(flags & it_usbd_property::FLAG_GET))
    {
        status = -1;
        goto l_exit;
    }
    else
    {
		/*
        if (is_pc_grabber_enabled())
        {
            profile |= PROFILE_PC_GRABBER;
        }

        if (is_hw_grabber_enabled())
        {
            profile |= PROFILE_HW_GRABBER;
        }
		*/
    }

    /*
     *  everything is ok!
     */
    status = 0;

l_exit:
    property_out.property.size    = cpu_to_le32(sizeof(it_usbd_property::property_profile));
    property_out.property.id      = cpu_to_le32(it_usbd_property::ID_PROFILE);
    property_out.property.flags   = status ? 0 : property_in.property.flags;
    property_out.property.status  = cpu_to_le32((unsigned int) status);
    property_out.profile          = cpu_to_le32(profile);

    return status;
}

#if defined(CONFIG_USBD_HAVE_PCGRABBER)
int
it_usbd_property::do_state(
    property_state&                     property_in,
    property_state&                     property_out)
{
    int                                 status;
    it_auto_log                         auto_log(__FUNCTION__, (int*)&property_in.state, &status);
    unsigned int                        flags;
    unsigned int                        state;

    /*
     *  validation check
     */
    if (property_in.property.size != cpu_to_le32(sizeof(it_usbd_property::property_state)))
    {
        status = -1;
        goto l_exit;
    }

    /*
     *  check flags
     */
    flags = le32_to_cpu(property_in.property.flags);
    if (flags & it_usbd_property::FLAG_SET)
    {
        switch ((state = le32_to_cpu(property_in.state)))
        {
            default:
                status = -1;
                goto l_exit;

            case it_usbd_property::STATE_STOP:
                m_my_time_base = 0;
            case it_usbd_property::STATE_PAUSE:
            case it_usbd_property::STATE_RUN:
                it_usbd_state = state;
                break;
        }

        if (it_usbd_state == it_usbd_property::STATE_RUN)
        {
            it_usbd_reset_fifo(FIFO0);
            it_usbd_reset_fifo(FIFO1);
            it_usbd_reset_fifo(FIFO2);
            it_usbd_reset_fifo(FIFO3);
            it_usbd_pcgrabber_base->start();
        }
        else
        {
            if (it_usbd_state == it_usbd_property::STATE_STOP)
                it_usbd_pcgrabber_base->stop();
        }
    }
    else if (!(flags & it_usbd_property::FLAG_GET))
    {
        status = -1;
        goto l_exit;
    }

    /*
     *  everything is ok!
     */
    status = 0;

l_exit:
    property_out.property.size    = cpu_to_le32(sizeof(it_usbd_property::property_state));
    property_out.property.id      = cpu_to_le32(it_usbd_property::ID_STATE);
    property_out.property.flags   = status ? 0 : property_in.property.flags;
    property_out.property.status  = cpu_to_le32((unsigned int) status);
    property_out.state            = cpu_to_le32(it_usbd_state);

    return status;
}

int
it_usbd_property::do_source(
    property_source&                    property_in,
    property_source&                    property_out)
{
    int                                 status;
    it_auto_log                         auto_log(__FUNCTION__, NULL, &status);
    unsigned int                        flags;
    unsigned int                        video_source;
    unsigned int                        audio_source;

    /*
     *  check size
     */
    if (property_in.property.size != cpu_to_le32(sizeof(it_usbd_property::property_source)))
    {
        status = -1;
        goto l_exit;
    }

    /*
     *  check flags
     */
    flags = le32_to_cpu(property_in.property.flags);
    if (flags & it_usbd_property::FLAG_SET)
    {
        video_source = le32_to_cpu(property_in.video_source);
        audio_source = le32_to_cpu(property_in.audio_source);

printf("audio_source=%d, video_source=%d\n", audio_source, video_source);

        if ((it_usbd_property::m_video_source != video_source) ||
            (it_usbd_property::m_audio_source != audio_source))
        {
            it_usbd_pcgrabber_base->set_source(audio_source, video_source);
            it_usbd_property::m_audio_source = audio_source;
            it_usbd_property::m_video_source = video_source;
        }
    }
    else if (!(flags & it_usbd_property::FLAG_GET))
    {
        status = -1;
        goto l_exit;
    }

    /*
     *  everything is ok!
     */
    status = 0;


l_exit:
    property_out.property.size    = cpu_to_le32(sizeof(it_usbd_property::property_source));
    property_out.property.id      = cpu_to_le32(it_usbd_property::ID_SOURCE);
    property_out.property.flags   = status ? 0 : property_in.property.flags;
    property_out.property.status  = cpu_to_le32((unsigned int) status);
    property_out.video_source     = cpu_to_le32(it_usbd_property::m_video_source);
    property_out.audio_source     = cpu_to_le32(it_usbd_property::m_audio_source);

    return status;
}
#endif

int
it_usbd_property::do_firmware_size(
    property_firmware_size&             property_in,
    property&                           property_out)
{
    int                                 status = 0;
    it_auto_log                         auto_log(__FUNCTION__, NULL, &status);
    unsigned int                        flags;

    /*
     *  check size
     */
    if (property_in.property.size != cpu_to_le32(sizeof(property_firmware_size)))
    {
        status = -1;
        goto l_exit;
    }

    /*
     *  check flags
     */
    flags = le32_to_cpu(property_in.property.flags);
    if (flags & it_usbd_property::FLAG_SET)
    {
        if (m_firmware_data)
        {
#if defined(IT_CONFIG_USE_MALLOC)
            free(m_firmware_data);
#else
            delete [] m_firmware_data;
#endif
            m_firmware_data = NULL;
        }
        m_firmware_size_total = le32_to_cpu(property_in.size);
        m_firmware_size_now   = 0;
        m_firmware_status     = 100;

printf("[!] firmware_size=%d\r\n", m_firmware_size_total);
        if (m_firmware_size_total)
        {
#if defined(IT_CONFIG_USE_MALLOC)
            m_firmware_data =  (unsigned char*) malloc(m_firmware_size_total);
#else
            try
            {
                m_firmware_data =  new (unsigned char[m_firmware_size_total]);
            }
            catch (...)
            {
                m_firmware_data = NULL;
            }
#endif
            if (!m_firmware_data)
            {
printf("[X] unable to allocate firmware_data!\r\n");
                m_firmware_size_total = 0;
                status = -1;
            }
        }
    }
    else
    {
        status = -1;
    }


l_exit:
    property_out.size   = cpu_to_le32(sizeof(it_usbd_property::property));
    property_out.id     = cpu_to_le32(it_usbd_property::ID_FIRMWARE_SIZE);
    property_out.flags  = cpu_to_le32(it_usbd_property::FLAG_SET);
    property_out.status = cpu_to_le32((unsigned int) status);

    if (status)
    {
        printf("[X] %s(%d)\r\n", __FUNCTION__, le32_to_cpu(property_in.size));
    }

    return status;
}

int
it_usbd_property::do_firmware_data(
    property_firmware_data&             property_in,
    property&                           property_out)
{
    int                                 status;
    it_auto_log                         auto_log(__FUNCTION__, NULL, &status);
    unsigned int                        flags;
    unsigned int                        size;

    /*
     *  validation
     */
    size = le32_to_cpu(property_in.property.size) - sizeof(it_usbd_property::property);
    if ((!size)                                                                                         ||
        /*(size > sizeof(it_usbd_property::property_firmware_data) - sizeof(it_usbd_property::property))  ||*/
        (!m_firmware_data)                                                                              ||
        (size > m_firmware_size_total - m_firmware_size_now))
    {
        status = -1;
        goto l_exit;
    }

    /*
     *  check flags
     */
    flags = le32_to_cpu(property_in.property.flags);
    if (flags != it_usbd_property::FLAG_SET)
    {
        status = -1;
        goto l_exit;
    }

    /*
     *  append firmware data
     */
    memcpy(m_firmware_data + m_firmware_size_now, property_in.data, size);
    m_firmware_size_now += size;

//printf("[!] firmware_data: %d/%d\r\n", m_firmware_size_now, m_firmware_size_total);
printf("[!] %d/%d\r\n", m_firmware_size_now, m_firmware_size_total);

    /*
     *  update firmware if completely xferred!
     */
    if (m_firmware_size_now == m_firmware_size_total)
    {
		printf("Start threadFirmware!!!\n");
        //it_usbd_pcgrabber_base->upgrade(m_firmware_data, m_firmware_size_total);
		m_firmware_status=100;
		void* threadFirmware(void *arg);
		static void* args[] = {m_firmware_data, &m_firmware_size_total, &m_firmware_status};
		pthread_t task;
		pthread_attr_t attr;
		pthread_attr_init(&attr);
		pthread_attr_setstacksize(&attr, 8192);
		pthread_create(&task, &attr, threadFirmware, args);
    }

    /*
     *  everything is ok!
     */
    status = 0;


l_exit:
    property_out.size   = cpu_to_le32(sizeof(it_usbd_property::property));
    property_out.id     = cpu_to_le32(it_usbd_property::ID_FIRMWARE_DATA);
    property_out.flags  = cpu_to_le32(it_usbd_property::FLAG_SET);
    property_out.status = cpu_to_le32((unsigned int) status);
	// test only
	property_firmware_data* property_firmware_data_out=(property_firmware_data*)&property_out;
	property_firmware_data_out->property.size=property_in.property.size;
	memcpy(property_firmware_data_out->data, property_in.data, size);
	////property_firmware_data_out->data[1]=0xaa;
	//


    if (status)
    {
        printf("[X] %s(%d)\r\n", __FUNCTION__, le32_to_cpu(size));
    }

    return status;
}

int
it_usbd_property::do_firmware_status(
    property&                           property_in,
    property_firmware_status&           property_out)
{
    int                                 status;
    it_auto_log                         auto_log(__FUNCTION__, NULL, &status);
    unsigned int                        flags;
    unsigned int                        size;

    /*
     *  validation
     */
    if ((property_in.size != cpu_to_le32(sizeof(it_usbd_property::property)))   ||
        (!m_firmware_data)                                                      ||
        (!m_firmware_size_total)                                                ||
        (m_firmware_size_now != m_firmware_size_total))
    {
        status = -1;
        goto l_exit;
    }

    /*
     *  check flags
     */
    flags = le32_to_cpu(property_in.flags);
    if (flags != it_usbd_property::FLAG_GET)
    {
        status = -1;
        goto l_exit;
    }

    /*
     *  TODO: get status from the firmware upgrader
     */
    //m_firmware_status = 0;//it_usbd_pcgrabber_base->is_upgrading();
    property_out.status = cpu_to_le32(m_firmware_status);

    /*
     *  everything is ok!
     */
    status = 0;

l_exit:
    property_out.property.size   = cpu_to_le32(sizeof(it_usbd_property::property_firmware_status));
    property_out.property.id     = cpu_to_le32(it_usbd_property::ID_FIRMWARE_STATUS);
    property_out.property.flags  = cpu_to_le32(it_usbd_property::FLAG_GET);
    property_out.property.status = cpu_to_le32((unsigned int) status);


    return status;
}

#if defined(CONFIG_USBD_HAVE_PCGRABBER)
int
it_usbd_property::do_video_compression_getinfo_property(
    property_video_compression_getinfo& property_in,
    property_video_compression_getinfo& property_out)
{
    it_auto_log                         auto_log(__FUNCTION__, NULL);

    property_out.property.size   = cpu_to_le32(sizeof(it_usbd_property::property_video_compression_getinfo));
    property_out.property.id     = cpu_to_le32(it_usbd_property::ID_VIDEO_COMPRESSION_GETINFO);
    property_out.property.flags  = cpu_to_le32(it_usbd_property::FLAG_GET);
    property_out.property.status = cpu_to_le32((unsigned int) 0);
    property_out.stream          = cpu_to_le32((unsigned int) 0);
    property_out.keyframe_rate   = cpu_to_le32((unsigned int) 10);
    property_out.quality         = cpu_to_le32((unsigned int) 5000);

    return 0;
}

int
it_usbd_property::do_video_compression_keyframe_rate_property(
    property_video_compression&         property_in,
    property_video_compression&         property_out)
{
    int                                 status;
    unsigned int                        flags;
    it_auto_log                         auto_log(__FUNCTION__, NULL, &status);

    flags = le32_to_cpu(property_in.property.flags);

    property_out.property.size   = cpu_to_le32(sizeof(it_usbd_property::property_video_compression_getinfo));
    property_out.property.id     = cpu_to_le32(it_usbd_property::ID_VIDEO_COMPRESSION_KEYFRAME_RATE);
    property_out.stream          = cpu_to_le32((unsigned int) 0);

    if (flags & it_usbd_property::FLAG_SET)
    {
        it_usbd_property::m_video_compression_keyframe_rate = cpu_to_le32((unsigned int) property_in.value);
        property_out.property.flags = cpu_to_le32(it_usbd_property::FLAG_SET);
    }
    else if (flags & it_usbd_property::FLAG_GET)
    {
        property_out.property.flags = cpu_to_le32(it_usbd_property::FLAG_GET);
    }
    else
    {
        status = -1;
        goto l_exit;
    }

    property_out.value = cpu_to_le32((unsigned int) it_usbd_property::m_video_compression_keyframe_rate);
    status = 0;

l_exit:
printf("compression.keyframerate=%d, status=%08X\n", it_usbd_property::m_video_compression_keyframe_rate, status);
    property_out.property.status = cpu_to_le32((unsigned int) status);
    return status;
}

int
it_usbd_property::do_video_compression_quality_property(
    property_video_compression&         property_in,
    property_video_compression&         property_out)
{
    int                                 status;
    unsigned int                        flags;
    it_auto_log                         auto_log(__FUNCTION__, NULL, &status);

    flags = le32_to_cpu(property_in.property.flags);

    property_out.property.size   = cpu_to_le32(sizeof(it_usbd_property::property_video_compression_getinfo));
    property_out.property.id     = cpu_to_le32(it_usbd_property::ID_VIDEO_COMPRESSION_QUALITY);
    property_out.stream          = cpu_to_le32((unsigned int) 0);

    if (flags & it_usbd_property::FLAG_SET)
    {
        it_usbd_property::m_video_compression_quality = cpu_to_le32((unsigned int) property_in.value);
        property_out.property.flags = cpu_to_le32(it_usbd_property::FLAG_SET);
    }
    else if (flags & it_usbd_property::FLAG_GET)
    {
        property_out.property.flags = cpu_to_le32(it_usbd_property::FLAG_GET);
    }
    else
    {
        status = -1;
        goto l_exit;
    }

    property_out.value = cpu_to_le32((unsigned int) it_usbd_property::m_video_compression_quality);
    status = 0;


l_exit:
printf("compression.quality=%d, status=%08X\n", it_usbd_property::m_video_compression_quality, status);
    property_out.property.status = cpu_to_le32((unsigned int) status);
    return status;
}

int
it_usbd_property::do_video_procamp_property(
    property_video_procamp&             property_in,
    property_video_procamp&             property_out,
    property_range&                     range)
{
    int                                 status;
    unsigned int                        flags;
    it_auto_log                         auto_log(__FUNCTION__, NULL, &status);

    flags = le32_to_cpu(property_in.property.flags);

    property_out.property.size   = cpu_to_le32(sizeof(it_usbd_property::property_video_procamp));
    property_out.property.id     = property_in.property.id;

    if (flags & it_usbd_property::FLAG_SET)
    {
        range.current_value = cpu_to_le32((unsigned int) property_in.value);
        property_out.property.flags = cpu_to_le32(it_usbd_property::FLAG_SET);
    }
    else if (flags & it_usbd_property::FLAG_GET)
    {
        property_out.property.flags = cpu_to_le32(it_usbd_property::FLAG_GET);
    }
    else
    {
        status = -1;
        goto l_exit;
    }

    property_out.value = cpu_to_le32((unsigned int) range.current_value);
    status = 0;


l_exit:
printf("%s=%d, status=%08X\n", range.name, range.current_value, status);
    property_out.property.status = cpu_to_le32((unsigned int) status);
    return status;
}
#endif
int
it_usbd_property::do_debug_query_time_property(
    property_debug_query_time&             property_in,
    property_debug_query_time&             property_out)
{
    int                                 status;
    //it_auto_log                         auto_log(__FUNCTION__, NULL, &status);
    unsigned int                        flags;
    unsigned long                       my_time;//PalGetClock();
    unsigned long                       peer_time = le32_to_cpu(property_in.time);
	struct timeval tv;
	gettimeofday(&tv, 0);
	my_time = (unsigned long) (tv.tv_sec * 1000 + tv.tv_usec / 1000);
    if (!m_my_time_base)
    {
        m_my_time_base = my_time;
        printf("[!] %s() m_my_time_base = %d\n", __FUNCTION__, m_my_time_base);
    }

    my_time -= m_my_time_base;
    printf("[!] %s(%u, %u)\n", __FUNCTION__, my_time, peer_time);


    /*
     *  check size
     */
    if (property_in.property.size != cpu_to_le32(sizeof(it_usbd_property::property_debug_query_time)))
    {
        status = -1;
        goto l_exit;
    }

    /*
     *  check flags
     */
    flags = le32_to_cpu(property_in.property.flags);

    /*
     *  everything is ok!
     */
    status = 0;


l_exit:
    property_out.property.size    = cpu_to_le32(sizeof(it_usbd_property::property_debug_query_time));
    property_out.property.id      = cpu_to_le32(it_usbd_property::ID_DEBUG_QUERY_TIME);
    property_out.property.flags   = status ? 0 : property_in.property.flags;
    property_out.property.status  = cpu_to_le32((unsigned int) status);
    property_out.time             = cpu_to_le32((unsigned long long) my_time);

    return status;
}

#if defined(CONFIG_USBD_HAVE_PCGRABBER)
#if (1)
int
it_usbd_property::do_video_procamp_brightness_property(
    property_video_procamp&             property_in,
    property_video_procamp&             property_out)
{
    int                                 status;
    unsigned int                        flags;
    VIDEO_COLOR_CTRL                    video_color_ctrl;
    it_auto_log                         auto_log(__FUNCTION__, NULL, &status);

    flags = le32_to_cpu(property_in.property.flags);

    property_out.property.size   = cpu_to_le32(sizeof(it_usbd_property::property_video_procamp));
    property_out.property.id     = cpu_to_le32(it_usbd_property::ID_VIDEO_PROCAMP_BRIGHTNESS);

    if (flags & it_usbd_property::FLAG_SET)
    {
        it_usbd_property::m_video_procamp_brightness = cpu_to_le32((unsigned int) property_in.value);
        coreGetVideoColor(&video_color_ctrl);
        video_color_ctrl.brightness = it_usbd_property::m_video_procamp_brightness;
        coreSetVideoColor(&video_color_ctrl);
        property_out.property.flags = cpu_to_le32(it_usbd_property::FLAG_SET);
    }
    else if (flags & it_usbd_property::FLAG_GET)
    {
        coreGetVideoColor(&video_color_ctrl);
        it_usbd_property::m_video_procamp_brightness = video_color_ctrl.brightness;
        property_out.property.flags = cpu_to_le32(it_usbd_property::FLAG_GET);
    }
    else
    {
        status = -1;
        goto l_exit;
    }

printf(
    "[!] %s(%s:%d)\n",
    __FUNCTION__,
    (flags & it_usbd_property::FLAG_SET) ? "SET" : "GET",
    it_usbd_property::m_video_procamp_brightness);

    property_out.value = cpu_to_le32((unsigned int) it_usbd_property::m_video_procamp_brightness);
    status = 0;


l_exit:
    property_out.property.status = cpu_to_le32((unsigned int) status);
    return status;
}

int
it_usbd_property::do_video_procamp_contrast_property(
    property_video_procamp&             property_in,
    property_video_procamp&             property_out)
{
    int                                 status;
    unsigned int                        flags;
    VIDEO_COLOR_CTRL                    video_color_ctrl;
    it_auto_log                         auto_log(__FUNCTION__, NULL, &status);

    flags = le32_to_cpu(property_in.property.flags);

    property_out.property.size   = cpu_to_le32(sizeof(it_usbd_property::property_video_procamp));
    property_out.property.id     = cpu_to_le32(it_usbd_property::ID_VIDEO_PROCAMP_CONTRAST);

    if (flags & it_usbd_property::FLAG_SET)
    {
        it_usbd_property::m_video_procamp_contrast = cpu_to_le32((unsigned int) property_in.value);
        coreGetVideoColor(&video_color_ctrl);
        video_color_ctrl.contrast = ((MMP_FLOAT) it_usbd_property::m_video_procamp_contrast) / 100;
        coreSetVideoColor(&video_color_ctrl);
        property_out.property.flags = cpu_to_le32(it_usbd_property::FLAG_SET);
    }
    else if (flags & it_usbd_property::FLAG_GET)
    {
        coreGetVideoColor(&video_color_ctrl);
        it_usbd_property::m_video_procamp_contrast = (int) (video_color_ctrl.contrast * 100);
        property_out.property.flags = cpu_to_le32(it_usbd_property::FLAG_GET);
    }
    else
    {
        status = -1;
        goto l_exit;
    }

printf(
    "[!] %s(%s:%d)\n",
    __FUNCTION__,
    (flags & it_usbd_property::FLAG_SET) ? "SET" : "GET",
    it_usbd_property::m_video_procamp_contrast);

    property_out.value = cpu_to_le32((unsigned int) it_usbd_property::m_video_procamp_contrast);
    status = 0;


l_exit:
    property_out.property.status = cpu_to_le32((unsigned int) status);
    return status;
}

int
it_usbd_property::do_video_procamp_hue_property(
    property_video_procamp&             property_in,
    property_video_procamp&             property_out)
{
    int                                 status;
    unsigned int                        flags;
    VIDEO_COLOR_CTRL                    video_color_ctrl;
    it_auto_log                         auto_log(__FUNCTION__, NULL, &status);

    flags = le32_to_cpu(property_in.property.flags);

    property_out.property.size   = cpu_to_le32(sizeof(it_usbd_property::property_video_procamp));
    property_out.property.id     = cpu_to_le32(it_usbd_property::ID_VIDEO_PROCAMP_HUE);

    if (flags & it_usbd_property::FLAG_SET)
    {
        it_usbd_property::m_video_procamp_hue = cpu_to_le32((unsigned int) property_in.value);
        coreGetVideoColor(&video_color_ctrl);
        video_color_ctrl.hue = it_usbd_property::m_video_procamp_hue;
        coreSetVideoColor(&video_color_ctrl);
        property_out.property.flags = cpu_to_le32(it_usbd_property::FLAG_SET);
    }
    else if (flags & it_usbd_property::FLAG_GET)
    {
        coreGetVideoColor(&video_color_ctrl);
        it_usbd_property::m_video_procamp_hue = video_color_ctrl.hue;
        property_out.property.flags = cpu_to_le32(it_usbd_property::FLAG_GET);
    }
    else
    {
        status = -1;
        goto l_exit;
    }

printf(
    "[!] %s(%s:%d)\n",
    __FUNCTION__,
    (flags & it_usbd_property::FLAG_SET) ? "SET" : "GET",
    it_usbd_property::m_video_procamp_hue);

    property_out.value = cpu_to_le32((unsigned int) it_usbd_property::m_video_procamp_hue);
    status = 0;


l_exit:
    property_out.property.status = cpu_to_le32((unsigned int) status);
    return status;
}

int
it_usbd_property::do_video_procamp_saturation_property(
    property_video_procamp&             property_in,
    property_video_procamp&             property_out)
{
    int                                 status;
    unsigned int                        flags;
    VIDEO_COLOR_CTRL                    video_color_ctrl;
    it_auto_log                         auto_log(__FUNCTION__, NULL, &status);

    flags = le32_to_cpu(property_in.property.flags);

    property_out.property.size   = cpu_to_le32(sizeof(it_usbd_property::property_video_procamp));
    property_out.property.id     = cpu_to_le32(it_usbd_property::ID_VIDEO_PROCAMP_SATURATION);

    if (flags & it_usbd_property::FLAG_SET)
    {
        it_usbd_property::m_video_procamp_saturation = cpu_to_le32((unsigned int) property_in.value);
        coreGetVideoColor(&video_color_ctrl);
        video_color_ctrl.saturation = ((MMP_FLOAT) it_usbd_property::m_video_procamp_saturation) / 100;
        coreSetVideoColor(&video_color_ctrl);
        property_out.property.flags = cpu_to_le32(it_usbd_property::FLAG_SET);
    }
    else if (flags & it_usbd_property::FLAG_GET)
    {
        coreGetVideoColor(&video_color_ctrl);
        it_usbd_property::m_video_procamp_saturation = (int) (video_color_ctrl.saturation * 100);
        property_out.property.flags = cpu_to_le32(it_usbd_property::FLAG_GET);
    }
    else
    {
        status = -1;
        goto l_exit;
    }

printf(
    "[!] %s(%s:%d)\n",
    __FUNCTION__,
    (flags & it_usbd_property::FLAG_SET) ? "SET" : "GET",
    it_usbd_property::m_video_procamp_saturation);

    property_out.value = cpu_to_le32((unsigned int) it_usbd_property::m_video_procamp_saturation);
    status = 0;


l_exit:
    property_out.property.status = cpu_to_le32((unsigned int) status);
    return status;
}

int
it_usbd_property::do_video_procamp_gain_property(
    property_video_procamp&             property_in,
    property_video_procamp&             property_out)
{
    int                                 status;
    unsigned int                        flags;
    it_auto_log                         auto_log(__FUNCTION__, NULL, &status);

    flags = le32_to_cpu(property_in.property.flags);

    property_out.property.size   = cpu_to_le32(sizeof(it_usbd_property::property_video_procamp));
    property_out.property.id     = cpu_to_le32(it_usbd_property::ID_VIDEO_PROCAMP_GAIN);

    if (flags & it_usbd_property::FLAG_SET)
    {
        it_usbd_property::m_video_procamp_gain = cpu_to_le32((unsigned int) property_in.value);
        property_out.property.flags = cpu_to_le32(it_usbd_property::FLAG_SET);
    }
    else if (flags & it_usbd_property::FLAG_GET)
    {
        property_out.property.flags = cpu_to_le32(it_usbd_property::FLAG_GET);
    }
    else
    {
        status = -1;
        goto l_exit;
    }

printf(
    "[!] %s(%s:%d)\n",
    __FUNCTION__,
    (flags & it_usbd_property::FLAG_SET) ? "SET" : "GET",
    it_usbd_property::m_video_procamp_gain);

    property_out.value = cpu_to_le32((unsigned int) it_usbd_property::m_video_procamp_gain);
    status = 0;


l_exit:
    property_out.property.status = cpu_to_le32((unsigned int) status);
    return status;
}
#endif
#endif

#undef printf


int idbDoProperty(void* inProp, void* outProp) {
    typedef union
    {
        it_usbd_property::property property;
        unsigned char data[512];
    } _prop;


    _prop& property_in((_prop&)  *(_prop*) inProp);
    _prop& property_out((_prop&) *(_prop*) outProp);
	
    if (le32_to_cpu(property_in.property.id) < it_usbd_property::ID_CUSTOMER_POOL) {
        it_usbd_property::do_property(property_in.data, property_out.data);
        return le32_to_cpu(property_out.property.size);
    }
    else {
        return 0;
    }
}

int idbGetPropertyId(void* prop) {
    return le32_to_cpu(((it_usbd_property::property*) prop)->id);
}

int idbIsCustomerId(void* prop) {
    return (idbGetPropertyId(prop) >= it_usbd_property::ID_CUSTOMER_POOL);
}

int idbHowManyMore(void* prop, int bytesReceived) {
    int howManyMore = le32_to_cpu(((it_usbd_property::property*) prop)->size)-bytesReceived;
    return (howManyMore>0)?howManyMore:0;}

int iteIdbInitialize(const struct idb_config *cfg)
{
    int rc = 0;
    if(the_idb)
    {
        ithPrintf(" idb: double initial?? \n");
        goto end;
    }
 
    the_idb = (struct idb_config*) malloc(sizeof(struct idb_config));
    if(!the_idb)
    {
        ithPrintf("-- alloc the_idb fail --\n");
        rc = -1;
        goto end;
    }
 
    the_idb->ops = cfg->ops;
 
    end:
    return rc;
}

void* threadFirmware(void *arg) {
    char* firmwareData;
    int firmwareSize;
	unsigned int* firmwareStatus;
printf("+++%s\n", __FUNCTION__);
    firmwareData=(char*)(((void**) arg)[0]); 
    firmwareSize=*(int*)(((void**) arg)[1]);
	firmwareStatus=(unsigned int*)(((void**) arg)[2]);
	*firmwareStatus=100;

	if (the_idb->ops->upgradefw(firmwareData, firmwareSize) == 0)
	*firmwareStatus=0;
	
printf("---%s\n", __FUNCTION__);
}