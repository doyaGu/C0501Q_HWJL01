/*
 *
 *  Copyright (C) 2012 ITE TECH. INC.   All Rights Reserved.
 *
 */
#if !defined(__IT_USBD_PROPERTY_H__)
#define __IT_USBD_PROPERTY_H__

#ifdef __cplusplus
extern "C" {
#endif    
int idbDoProperty(void* inProp, void* outProp);
int idbGetPropertyId(void* prop);
int idbIsCustomerId(void* prop);
int idbHowManyMore(void* prop, int bytesReceived);
#ifdef __cplusplus
}
#endif    

#ifdef __cplusplus
class it_usbd_pcgrabber_base
{
private:

public:
    static it_usbd_pcgrabber_base* init_instance();
    virtual void start() = 0;
    virtual void stop() = 0;
    virtual void advance_buffer(int len) = 0;
    virtual char* get_buffer(int& len) = 0;
    virtual void set_source(int audio_source, int video_source) = 0;
    virtual int is_upgrading() = 0;
    virtual void upgrade(void* image, unsigned int length) = 0;
    virtual void set_pc_mode(int enable) = 0;
    virtual unsigned short get_usb_vid() = 0;
    virtual unsigned short get_usb_pid() = 0;
    virtual const char* get_usb_manufacturer_string() = 0;
    virtual const char* get_usb_product_string() = 0;
    virtual const char* get_usb_serial_number_string() = 0;
};

extern it_usbd_pcgrabber_base* it_usbd_pcgrabber_base;

class it_auto_log
{
private:
    static int                  m_depth;
    const char*                 m_function_name;
    int*                        m_leave_value;
    void log(const char* msg0, int* p_value)
    {
        for (int i = 0; i < m_depth; i++)
        {
            printf("    ");
        }

        if (p_value)
        {
            printf("%s%s()=%08X\r\n", msg0, m_function_name ? m_function_name : "<NULL>", *p_value);
        }
        else
        {
            printf("%s%s()\r\n", msg0, m_function_name ? m_function_name : "<NULL>");
        }
    }

public:
    it_auto_log(const char* function_name, int* p_enter_value = 0, int* p_leave_value = 0):m_leave_value(p_leave_value)
    {
        //return;
        m_function_name = function_name;
        log("+++", p_enter_value);
        m_depth++;
    }

    virtual ~it_auto_log()
    {
        //return;
        m_depth--;
        log("---", m_leave_value);
    }
};

#else
#define it_auto_log(...)
#endif


#if defined(__cplusplus)
class it_usbd_property
{
public:
    enum 
    {
        ID_REBOOT                       = 0x99100001,
        ID_STATE                        = 0x99100002,
        ID_SOURCE                       = 0x99100003,
        ID_BITRATE                      = 0x99100004,
        ID_FIRMWARE_SIZE                = 0x99100005,
        ID_FIRMWARE_DATA                = 0x99100006,
        ID_FIRMWARE_UPDATE              = 0x99100007,
        ID_FIRMWARE_STATUS              = 0x99100008,
        ID_LOCK                         = 0x99100009,
        ID_PROFILE                      = 0x9910000A,

        ID_VIDEO_PROCAMP_START          = 0x99100101,
        ID_VIDEO_PROCAMP_BRIGHTNESS     = ID_VIDEO_PROCAMP_START,
        ID_VIDEO_PROCAMP_CONTRAST,
        ID_VIDEO_PROCAMP_HUE,
        ID_VIDEO_PROCAMP_SATURATION,
        ID_VIDEO_PROCAMP_GAIN,
        ID_VIDEO_PROCAMP_END,

        ID_VIDEO_COMPRESSION_START      = 0x99100201,
        ID_VIDEO_COMPRESSION_GETINFO    = ID_VIDEO_COMPRESSION_START,
        ID_VIDEO_COMPRESSION_KEYFRAME_RATE,
        ID_VIDEO_COMPRESSION_QUALITY,
        ID_VIDEO_COMPRESSION_END,

        ID_PC_GRABBER                   = 0x9910e001,

        ID_DEBUG_QUERY_TIME             = 0x9910f001,
        ID_HW_GRABBER                   = 0x9910f002,

        ID_ASYNCDATA_SWITCH = 0x995F0000,

		ID_CUSTOMER_POOL = 0x99990000,	// the Customer ID Pool starts here!!!
		ID_CUSTOMER_DEMOLOGGER = ID_CUSTOMER_POOL,
		ID_CUSTOMER_DEMOECHO,
        ID_CUSTOMER_ASYNCDATA=0x99999999,
    };

    enum
    {
        FLAG_GET                        = (0x00000001) << 0,
        FLAG_SET                        = (0x00000001) << 1,
    };

    enum
    {
        PROFILE_PC_GRABBER              = (0x00000001) << 0,
        PROFILE_HW_GRABBER              = (0x00000001) << 1,
    };

    enum
    {
        STATE_STOP                      = 0,
        STATE_PAUSE,
        STATE_RUN
    };    

    enum {
        VIDEO_SOURCE_HDMI               = 0,
        VIDEO_SOURCE_YPBPR,
        VIDEO_SOURCE_SVIDEO,
        VIDEO_SOURCE_COMPOSITE,
        VIDEO_SOURCE_DSUB,
        VIDEO_SOURCE_TOTAL,
    };

    enum {
        AUDIO_SOURCE_HDMI               = 0,
        AUDIO_SOURCE_SPDIF,
        AUDIO_SOURCE_LINE,
        AUDIO_SOURCE_TOTAL,
    };

    enum
    {
        //  INFO
        SUBID_PC_GRABBER_VERSION_INFO       = 0x38380001,
        SUBID_PC_GRABBER_SOURCE_INFO,

        //  TS
        SUBID_PC_GRABBER_TS_SERVICE_NAME    = 0x38381001,
        SUBID_PC_GRABBER_TS_PROVIDER_NAME,
        SUBID_PC_GRABBER_TS_PROGRAM_NUMBER,
        SUBID_PC_GRABBER_TS_PMT_PID,
        SUBID_PC_GRABBER_TS_AUDIO_PID,
        SUBID_PC_GRABBER_TS_VIDEO_PID,
        SUBID_PC_GRABBER_TS_STUFFING_RATIO,

        //  Video
        SUBID_PC_GRABBER_VIDEO_CODEC        = 0x38382001,
        SUBID_PC_GRABBER_VIDEO_BITRATE,
        SUBID_PC_GRABBER_VIDEO_DIMENSION,
        SUBID_PC_GRABBER_VIDEO_FRAMERATE,
        SUBID_PC_GRABBER_VIDEO_MAX_FRAMERATE,
        SUBID_PC_GRABBER_VIDEO_GOP_SIZE,
        SUBID_PC_GRABBER_VIDEO_ASPECT_RATIO,
        SUBID_PC_GRABBER_VIDEO_ENCODER_PARAMS,
        SUBID_PC_GRABBER_VIDEO_MAX_FRAMERATE2,
        

        //  Audio
        SUBID_PC_GRABBER_AUDIO_CODEC        = 0x38383001,
        SUBID_PC_GRABBER_AUDIO_BITRATE,
        SUBID_PC_GRABBER_AUDIO_SAMPLE_RATE,
        SUBID_PC_GRABBER_AUDIO_ENCODER_PARAMS,

        //  misc.
        SUBID_PC_GRABBER_MODE               = 0x38384001,
        SUBID_PC_GRABBER_MIC_VOLUME,
        SUBID_PC_GRABBER_LINEIN_BOOST,
        SUBID_PC_GRABBER_BLINGRING_LED,
        SUBID_PC_GRABBER_RED_LED,
        SUBID_PC_GRABBER_GREEN_LED,
        SUBID_PC_GRABBER_BLUE_LED,
        SUBID_PC_GRABBER_I2C,
        SUBID_PC_GRABBER_FW,
        SUBID_PC_GRABBER_INTERRUPT_STATE,
        SUBID_PC_GRABBER_DATE,
        SUBID_PC_GRABBER_TIME,
        SUBID_PC_GRABBER_ROOT,
        SUBID_PC_GRABBER_DIGITAL_VOLUME,
        SUBID_PC_GRABBER_ENABLE_RECORD,
        SUBID_PC_GRABBER_DISABLE_HDCP,

        SUBID_PC_GRABBER_GENERIC_I2C        = 0x38383801,
    };

#define IT_USBD_PROPERTY_BASE   \
    unsigned int    size;       \
    unsigned int    id;         \
    unsigned int    flags;      \
    signed   int    status

    typedef struct
    {
        IT_USBD_PROPERTY_BASE;
    } property;

    typedef struct
    {
        it_usbd_property::property      property;
        unsigned int                    profile;
    } property_profile;

    typedef struct
    {
        it_usbd_property::property      property;
        unsigned int                    state;              // IT_USB_STATE_XXX
    } property_state;

    typedef struct
    {
        it_usbd_property::property      property;
        unsigned int                    audio_source;
        unsigned int                    video_source;
    } property_source;

    typedef struct
    {
        it_usbd_property::property      property;
        unsigned int                    size;
    } property_firmware_size;

    typedef struct
    {
        it_usbd_property::property      property;
        unsigned char                   data[512 - sizeof(it_usbd_property::property)];
    } property_firmware_data;

    typedef struct
    {
        it_usbd_property::property      property;
        unsigned int                    status;
    } property_firmware_status;

    typedef struct
    {
        it_usbd_property::property      property;
        int                             stream;
        int                             value;
    } property_video_procamp;

    typedef struct
    {
        it_usbd_property::property      property;
        int                             stream;
        int                             keyframe_rate;
        int                             quality;
    } property_video_compression_getinfo;

    typedef struct
    {
        it_usbd_property::property      property;
        int                             stream;
        int                             value;
    } property_video_compression;

#define IT_USBD_PC_GRABBER_PROPERTY_BASE    \
    union                                   \
    {                                       \
        struct                              \
        {                                   \
            IT_USBD_PROPERTY_BASE;          \
            union                           \
            {                               \
                signed int          subid;  \
            };                              \
        } __attribute__((aligned(8)));      \
        unsigned long long alignment;       \
    }

    typedef struct
    {
        IT_USBD_PC_GRABBER_PROPERTY_BASE;
    } property_pc_grabber;

    typedef struct
    {
        IT_USBD_PC_GRABBER_PROPERTY_BASE;
        union
        {
            unsigned char               u8[1];
            char                        i8[1];
            unsigned short              u16[1];
            short                       i16[1];
            unsigned int                u32[1];
            int                         i32[1];
            unsigned long long          u64[1];
            long long                   i64[1];
            char                        str[1];
            struct {
                unsigned int            input;
                unsigned int            width;
                unsigned int            height;
                unsigned int            frame_rate;
                unsigned int            interlaced;
                unsigned int            aspect_ratio;
                unsigned int            hdcp;
                unsigned int            hdmi_audio_format;
            } source_info;
            unsigned int                enabled;
            unsigned int                led_status;
            unsigned char               i2c_data[1];
            struct {
                unsigned char           addr;
                unsigned char           data[1];
            } fw;
            struct {
                unsigned int            year;
                unsigned int            month;
                unsigned int            day;
            } date;
            struct {
                unsigned int            hour;
                unsigned int            minute;
                unsigned int            second;
            } time;
            struct {
                unsigned char           addr;               /* bit[6:0] */
                unsigned char           num_bytes_to_write;
                unsigned char           num_bytes_to_read;
                unsigned char           num_bytes_written;
                unsigned char           num_bytes_read;
                unsigned char           data[1];
            } i2c;
            struct {
                unsigned int            source_index;
                unsigned int            mode_index;
                unsigned int            flags;
                unsigned int            width;
                unsigned int            height;
                unsigned int            kbit_rate;
                unsigned int            deinterlace_on;
                unsigned int            frame_double;
                unsigned int            frame_rate;
                unsigned int            gop_size;
                unsigned int            aspect_ratio;
                unsigned int            skip_mode;
                unsigned int            instance_num;
            } video_encoder_params;
            struct {
                unsigned int            codec_type;
                unsigned int            bit_rate;
                unsigned int            sample_rate;
            } audio_encoder_params;
        };
    } property_pc_grabber_vt;

    typedef struct
    {
        it_usbd_property::property      property;
        unsigned long                   time;
    } property_debug_query_time;

    typedef struct
    {
        it_usbd_property::property      property;
        unsigned int asyncdataSwitch;
    } property_asyncdata_switch;


    typedef struct {
        it_usbd_property::property      property;
		char msg2Logger[512 - sizeof(it_usbd_property::property)];
	} property_customer_demologger;


	typedef struct {
        it_usbd_property::property      property;
		char echo[512 - sizeof(it_usbd_property::property)];
	} property_customer_demoecho;

private:
    typedef struct {
        int                             id;
        int                             current_value;
        int                             default_value;
        int                             min_value;
        int                             max_value;
        const char*                     name;
    } property_range; 
    static property_range               m_property_ranges[
                                            ID_VIDEO_PROCAMP_END - ID_VIDEO_PROCAMP_START +
                                            ID_VIDEO_COMPRESSION_END - ID_VIDEO_COMPRESSION_START + 
                                            1];

    static unsigned int                 m_cmd_sequence;
    static unsigned int                 m_firmware_size_now;
    static unsigned int                 m_firmware_size_total;
    static unsigned char*               m_firmware_data;
    static unsigned char*               m_firmware_reboot;
    static unsigned int                 m_firmware_status;
    static          int                 m_video_procamp_brightness;
    static          int                 m_video_procamp_contrast;
    static          int                 m_video_procamp_hue;
    static          int                 m_video_procamp_saturation;
    static          int                 m_video_procamp_gain;
    static          int                 m_video_compression_keyframe_rate;
    static          int                 m_video_compression_quality;
    static unsigned long                m_my_time_base;

public:
    static char                         m_ts_service_name[256];
    static char                         m_ts_provider_name[256];
    static unsigned int                 m_ts_program_number;
    static unsigned int                 m_ts_pmt_pid;
    static unsigned int                 m_ts_video_pid;
    static unsigned int                 m_ts_audio_pid;
    static unsigned int                 m_ts_stuffing_ratio;
    static unsigned int                 m_video_codec;
    static unsigned int                 m_video_bitrate;
    static unsigned int                 m_video_framerate;
    static unsigned int                 m_video_max_framerate;
    static unsigned int                 m_video_gop_size;
    static unsigned int                 m_video_dimension_width;
    static unsigned int                 m_video_dimension_height;
    static unsigned int                 m_video_aspect_ratio;
    static unsigned int                 m_video_source;
    static unsigned int                 m_audio_codec;
    static unsigned int                 m_audio_bitrate;
    static unsigned int                 m_audio_source;
    static unsigned int                 m_audio_sampling_rate;
    static unsigned int                 m_mic_volume;
    static unsigned int                 m_linein_boost;
    static unsigned int                 m_pc_mode;
    static unsigned char                m_i2c_addr;
    static unsigned char                m_i2c_num_bytes_to_write;
    static unsigned char                m_i2c_num_bytes_to_read;
    static unsigned char                m_i2c_data[255];
    static unsigned int                 m_video_source_index;   // CAPTURE_VIDEO_SOURCE
    static unsigned int                 m_video_mode_index;     // VIDEO_ENCODER_INPUT_INFO
    static unsigned int                 m_video_max_frame_rate2_mode_index;
    static unsigned int                 m_video_max_frame_rate2_width;
    static unsigned int                 m_video_max_frame_rate2_height;
    
    virtual ~it_usbd_property()         = 0;

public:
    static int do_property(
        unsigned char                   (&data_in)[512],
        unsigned char                   (&data_out)[512]);

    static int is_pc_grabber_enabled() {
#if defined(CONFIG_USBD_HAVE_PCGRABBER)
        return 1;
#else
        return 0;
#endif
    }

#if defined(CONFIG_USBD_HAVE_PCGRABBER)
    static int do_pc_grabber(
        property&                       _property_in,
        property&                       _property_out);
#endif

    static void do_reboot(
        property&                       property_in,
        property&                       property_out);

    static int do_profile(
        property&                       _property_in,
        property&                       _property_out);

#if defined(CONFIG_USBD_HAVE_PCGRABBER)
    static int do_state(
        property_state&                 property_in,
        property_state&                 property_out);

    static int do_source(
        property_source&                property_in,
        property_source&                property_out);
#endif

    static int do_firmware_size(
        property_firmware_size&         property_in,
        property&                       property_out);

    static int do_firmware_data(
        property_firmware_data&         property_in,
        property&                       property_out);

    static int do_firmware_status(
        property&                       property_in,
        property_firmware_status&       property_out);

    static int do_reboot(
        property&                       property_in,
        property_firmware_status&       property_out);

#if defined(CONFIG_USBD_HAVE_PCGRABBER)
    static int do_video_compression_getinfo_property(
        property_video_compression_getinfo&    property_in,
        property_video_compression_getinfo&    property_out);

    static int do_video_compression_keyframe_rate_property(
        property_video_compression&     property_in,
        property_video_compression&     property_out);

    static int do_video_compression_quality_property(
        property_video_compression&     property_in,
        property_video_compression&     property_out);

    static int do_video_procamp_property(
        property_video_procamp&         property_in,
        property_video_procamp&         property_out,
        property_range&                 range);
#endif
    static int do_debug_query_time_property(
        property_debug_query_time&      property_in,
        property_debug_query_time&      property_out);
#if defined(CONFIG_USBD_HAVE_PCGRABBER)
    static int is_hw_grabber_enabled();

    static int do_hw_grabber_property(
        property&                       _property_in,
        property&                       _property_out);

    static int do_video_procamp_brightness_property(
        property_video_procamp&         property_in,
        property_video_procamp&         property_out);

    static int do_video_procamp_contrast_property(
        property_video_procamp&         property_in,
        property_video_procamp&         property_out);

    static int do_video_procamp_hue_property(
        property_video_procamp&         property_in,
        property_video_procamp&         property_out);

    static int do_video_procamp_saturation_property(
        property_video_procamp&         property_in,
        property_video_procamp&         property_out);

    static int do_video_procamp_gain_property(
        property_video_procamp&         property_in,
        property_video_procamp&         property_out);
#endif
};
#endif

#endif
