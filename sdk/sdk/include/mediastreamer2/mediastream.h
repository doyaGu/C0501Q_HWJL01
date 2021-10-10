/*
mediastreamer2 library - modular sound and video processing and streaming
Copyright (C) 2006  Simon MORLAT (simon.morlat@linphone.org)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/


#ifndef MEDIASTREAM_H
#define MEDIASTREAM_H

#include <ortp/ortp.h>
#include <ortp/event.h>
#include <ortp/zrtp.h>
#include <ortp/ortp_srtp.h>

#include <ortp/zrtp.h>

#include <mediastreamer2/msfilter.h>
#include <mediastreamer2/msticker.h>
#include <mediastreamer2/mssndcard.h>
#include <mediastreamer2/mswebcam.h>
#include <mediastreamer2/msvideo.h>
#include <mediastreamer2/bitratecontrol.h>
#include <mediastreamer2/qualityindicator.h>
#include <mediastreamer2/ice.h>


#define PAYLOAD_TYPE_FLAG_CAN_RECV  PAYLOAD_TYPE_USER_FLAG_1
#define PAYLOAD_TYPE_FLAG_CAN_SEND  PAYLOAD_TYPE_USER_FLAG_2


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @addtogroup ring_api
 * @{
**/

typedef enum _LinphoneAudioStreamFlow
{
    AudioFromRtpRecv,
    AudioFromSoundRead,
    AudioFromUdpRecv
} LinphoneAudioStreamFlow;

struct _RingStream
{
    MSTicker *ticker;
    MSFilter *source;
    MSFilter *gendtmf;
    MSFilter *write_resampler;
    MSFilter *sndwrite;
    MSFilter *decoder; 
    MSFilter *mixer;
};

typedef struct _RingStream RingStream;

MS2_PUBLIC RingStream *ring_start (const char * file, int interval, MSSndCard *sndcard);
MS2_PUBLIC RingStream *ring_start_with_cb(const char * file, int interval, MSSndCard *sndcard, MSFilterNotifyFunc func, void * user_data);
MS2_PUBLIC void ring_stop (RingStream * stream);

/**
 * @}
**/

/**
 * @addtogroup mkv player api
 * @{
**/

typedef struct {
    MSFilter    *player;
    MSFilter    *audioDecoder;
    MSFilter    *videoDecoder;
    MSFilter    *audioSink;
    MSFilter    *videoSink;
    MSTicker    *ticker;
    char        *filename;
    bool_t      eof;
    uint64_t    origTime;
    bool_t      origTimeIsSet;
    bool_t      firstVideoImage;
} MKVPlayerStream;

MKVPlayerStream * mkvplayer_stream_init(const char *filename);
void mkvplayer_stream_uninit(MKVPlayerStream *obj);
void mkvplayer_stream_start(MKVPlayerStream *obj);
void mkvplayer_stream_stop(MKVPlayerStream *obj);
uint64_t mkvplayer_stream_get_time(const MKVPlayerStream *obj);
void mkvplayer_stream_set_native_window_id(const MKVPlayerStream *obj, unsigned long id);

/**
 * @}
**/


typedef enum StreamType {
    AudioStreamType,
    VideoStreamType
} StreamType;

/**
 * The MediaStream is an object describing a stream (one of AudioStream or VideoStream).
**/
typedef struct _MediaStream MediaStream;

/*
 * internal cb to process rtcp stream
 * */
typedef  void (*media_stream_process_rtcp)(MediaStream *stream, mblk_t *m);

struct _MSMediaStreamSessions{
    RtpSession *rtp_session;
    srtp_t srtp_session;
    OrtpZrtpContext *zrtp_context;
    MSTicker *ticker;
};

typedef struct _MSMediaStreamSessions MSMediaStreamSessions;

/**
 * Base struct for both AudioStream and VideoStream structure.
**/
struct _MediaStream {
    StreamType type;
    MSMediaStreamSessions sessions;
    OrtpEvQueue *evq;
    MSFilter *rtprecv;
    MSFilter *rtpsend;
	MSFilter *udprecv;
    MSFilter *udpsend;
	unsigned short udp_port;
    MSFilter *encoder;
    MSFilter *decoder;
    MSBitrateController *rc;
    MSQualityIndicator *qi;
    time_t start_time;
    bool_t use_rc;
    bool_t is_beginning;
    bool_t pad[2];
};

MS2_PUBLIC void media_stream_set_rtcp_information(MediaStream *stream, const char *cname, const char *tool);

MS2_PUBLIC void media_stream_enable_adaptive_bitrate_control(MediaStream *stream, bool_t enabled);
/* *
 * returns a realtime indicator of the stream quality between 0 and 5
 * */
MS2_PUBLIC float media_stream_get_quality_rating(MediaStream *stream);

MS2_PUBLIC float media_stream_get_average_quality_rating(MediaStream *stream);

typedef enum EchoLimiterType{
    ELInactive,
    ELControlMic,
    ELControlFull
} EchoLimiterType;

struct _AudioStream
{
    MediaStream ms;
    MSFilter *soundread;
    MSFilter *soundwrite;
    MSFilter *dtmfgen;
    MSFilter *dtmfgen_rtp;
    MSFilter *ec;/*echo canceler*/
    MSFilter *volsend,*volrecv; /*MSVolumes*/
    MSFilter *read_resampler;
    MSFilter *write_resampler;
    MSFilter *equalizerSPK;
    MSFilter *equalizerMIC;
    MSFilter *teeforrecord;  //for MKV record   
    MSFilter *itcsink;       //for MKV record
    MSFilter *mixvoice;
    struct {
        MSFilter *tee;             
        MSFilter *itcsink;
        MSFilter *audio_input;
        MSFilter *recorder;
        MSFilter *encoder;
        MSTicker *ticker;
    } a_recorder;//for audiostream rec
    uint64_t last_packet_count;
    time_t last_packet_time;
    EchoLimiterType el_type; /*use echo limiter: two MSVolume, measured input level controlling local output level*/
    bool_t eq_SPK;//filter speaker equalizer
    bool_t eq_MIC;//filter mic equalizer
    bool_t use_ec;//filter acoustic echo cancellation
    bool_t use_mix;//filter mix sound
    bool_t use_volsend;//filter volsend dsp
    bool_t use_volrecv;//filter volrecv dsp
    bool_t play_dtmfs;
    bool_t use_gc;//gain control function
    bool_t use_agc;//acoustic gain control function
    bool_t use_ng;//noise gate function
    bool_t receive_graph;//
    bool_t send_graph;
};

/**
 * The AudioStream holds all resources to create and run typical VoIP audiostream.
**/
typedef struct _AudioStream AudioStream;

struct _VoiceMemoRecordStream
{
    MediaStream ms;
    MSFilter *sndread;
    MSFilter *filewrite;
};

typedef struct _VoiceMemoRecordStream VoiceMemoRecordStream;

struct _VoiceMemoPlayStream
{
    MediaStream ms;
    MSFilter *fileread;
    MSFilter *sndwrite;
};

typedef struct _VoiceMemoPlayStream VoiceMemoPlayStream;

#if ENABLE_AUDIO_ENGENEER_MODEL
struct _PlayRecstream
{
    MediaStream ms;
    MSFilter *fileread;
    MSFilter *sndwrite;
    MSFilter *filewrite;
    MSFilter *sndread;
    MSFilter *ec;/*echo canceler*/
    MSFilter *volsend,*volrecv; /*MSVolumes*/
    MSFilter *equalizerMIC,*equalizerSPK;/*msequalizer*/
};

typedef struct _PlayRecstream Playrecstream;
#endif

/* start a thread that does sampling->encoding->rtp_sending|rtp_receiving->decoding->playing */
MS2_PUBLIC AudioStream *audio_stream_start (RtpProfile * prof, int locport, const char *remip,
                 int remport, int payload_type, int jitt_comp, bool_t echo_cancel);

MS2_PUBLIC AudioStream *audio_stream_start_with_sndcards(RtpProfile * prof, int locport, const char *remip4, int remport, int payload_type, int jitt_comp, MSSndCard *playcard, MSSndCard *captcard, bool_t echocancel);


MS2_PUBLIC int audio_stream_start_with_files (AudioStream * stream, RtpProfile * prof,
                        const char *remip, int remport, int rem_rtcp_port,
                        int pt, int jitt_comp,
                        const char * infile,  const char * outfile);

/**
 * Starts an audio stream from/to local wav files or soundcards.
 *
 * This method starts the processing of the audio stream, that is playing from wav file or soundcard, voice processing, encoding,
 * sending through RTP, receiving from RTP, decoding, voice processing and wav file recording or soundcard playback.
 *
 *
 * @param stream an AudioStream previously created with audio_stream_new().
 * @param profile a RtpProfile containing all PayloadType possible during the audio session.
 * @param rem_rtp_ip remote IP address where to send the encoded audio.
 * @param rem_rtp_port remote IP port where to send the encoded audio.
 * @param rem_rtcp_ip remote IP address for RTCP.
 * @param rem_rtcp_port remote port for RTCP.
 * @param payload payload type index to use for the sending stream. This index must point to a valid PayloadType in the RtpProfile.
 * @param jitt_comp Nominal jitter buffer size in milliseconds.
 * @param infile path to wav file to play out (can be NULL)
 * @param outfile path to wav file to record into (can be NULL)
 * @param playcard The soundcard to be used for playback (can be NULL)
 * @param captcard The soundcard to be used for capture. (can be NULL)
 * @param use_ec whether echo cancellation is to be performed.
 * @returns 0 if successful, -1 otherwise.
**/
MS2_PUBLIC int audio_stream_start_full(AudioStream *stream, RtpProfile *profile, const char *rem_rtp_ip,int rem_rtp_port,
    const char *rem_rtcp_ip, int rem_rtcp_port, int payload,int jitt_comp, const char *infile, const char *outfile,
    MSSndCard *playcard, MSSndCard *captcard, bool_t use_ec, LinphoneAudioStreamFlow select_flow);


MS2_PUBLIC void audio_stream_play(AudioStream *st, const char *name);
MS2_PUBLIC void audio_stream_record(AudioStream *st, const char *name);

static MS2_INLINE void audio_stream_set_rtcp_information(AudioStream *st, const char *cname, const char *tool) {
    media_stream_set_rtcp_information(&st->ms, cname, tool);
}

MS2_PUBLIC void audio_stream_play_received_dtmfs(AudioStream *st, bool_t yesno);

/**
 * Creates an AudioStream object listening on a RTP port.
 * @param loc_rtp_port the local UDP port to listen for RTP packets.
 * @param loc_rtcp_port the local UDP port to listen for RTCP packets
 * @param ipv6 TRUE if ipv6 must be used.
 * @returns a new AudioStream.
**/
MS2_PUBLIC AudioStream *audio_stream_new(int loc_rtp_port, int loc_rtcp_port, bool_t ipv6);

/**
 * Starts an audio stream from local soundcards.
 *
 * This method starts the processing of the audio stream, that is capture from soundcard, voice processing, encoding,
 * sending through RTP, receiving from RTP, decoding, voice processing and soundcard playback.
 *
 * @param stream an AudioStream previously created with audio_stream_new().
 * @param prof a RtpProfile containing all PayloadType possible during the audio session.
 * @param remip remote IP address where to send the encoded audio.
 * @param remport remote IP port where to send the encoded audio
 * @param rem_rtcp_port remote port for RTCP.
 * @param payload_type payload type index to use for the sending stream. This index must point to a valid PayloadType in the RtpProfile.
 * @param jitt_comp Nominal jitter buffer size in milliseconds.
 * @param playcard The soundcard to be used for playback
 * @param captcard The soundcard to be used for capture.
 * @param echo_cancel whether echo cancellation is to be performed.
**/
MS2_PUBLIC int audio_stream_start_now(AudioStream * stream, RtpProfile * prof,  const char *remip, int remport, int rem_rtcp_port, int payload_type, int jitt_comp,MSSndCard *playcard, MSSndCard *captcard, bool_t echo_cancel);
MS2_PUBLIC void audio_stream_set_relay_session_id(AudioStream *stream, const char *relay_session_id);
/*returns true if we are still receiving some data from remote end in the last timeout seconds*/
MS2_PUBLIC bool_t audio_stream_alive(AudioStream * stream, int timeout);

/**
 * Executes background low priority tasks related to audio processing (RTP statistics analysis).
 * It should be called periodically, for example with an interval of 100 ms or so.
 */
MS2_PUBLIC void audio_stream_iterate(AudioStream *stream);

/**
 * enable echo-limiter dispositve: one MSVolume in input branch controls a MSVolume in the output branch
 * */
MS2_PUBLIC void audio_stream_enable_echo_limiter(AudioStream *stream, EchoLimiterType type);

/**
 * enable gain control, to be done before start()
 * */
MS2_PUBLIC void audio_stream_enable_gain_control(AudioStream *stream, bool_t val);

/**
 * enable automatic gain control, to be done before start()
 * */
MS2_PUBLIC void audio_stream_enable_automatic_gain_control(AudioStream *stream, bool_t val);

/**
 * to be done before start
 *  */
MS2_PUBLIC void audio_stream_set_echo_canceller_params(AudioStream *st, int tail_len_ms, int delay_ms, int framesize);

/**
 * enable adaptive rate control
 * */
static MS2_INLINE void audio_stream_enable_adaptive_bitrate_control(AudioStream *stream, bool_t enabled) {
    media_stream_enable_adaptive_bitrate_control(&stream->ms, enabled);
}


MS2_PUBLIC void audio_stream_set_mic_gain(AudioStream *stream, float gain);

/**
 *  enable/disable rtp stream
 *  */
MS2_PUBLIC void audio_stream_mute_rtp(AudioStream *stream, bool_t val);

/**
 * enable noise gate, must be done before start()
 * */
MS2_PUBLIC void audio_stream_enable_noise_gate(AudioStream *stream, bool_t val);

/**
 * enable parametric equalizer in the stream that goes to the speaker
 * */
MS2_PUBLIC void audio_stream_enable_equalizer(AudioStream *stream, bool_t enabled);

MS2_PUBLIC void audio_stream_equalizer_set_gain(AudioStream *stream, int frequency, float gain, int freq_width);

/**
 *  stop the audio streaming thread and free everything
 *  */
MS2_PUBLIC void audio_stream_stop (AudioStream * stream, LinphoneAudioStreamFlow select_flow);

/**
 *  send a dtmf
 *  */
MS2_PUBLIC int audio_stream_send_dtmf (AudioStream * stream, char dtmf);

MS2_PUBLIC void audio_stream_set_default_card(int cardindex);

/* retrieve RTP statistics*/
MS2_PUBLIC void audio_stream_get_local_rtp_stats(AudioStream *stream, rtp_stats_t *stats);

/* returns a realtime indicator of the stream quality between 0 and 5 */
MS2_PUBLIC float audio_stream_get_quality_rating(AudioStream *stream);

/* returns the quality rating as an average since the start of the streaming session.*/
MS2_PUBLIC float audio_stream_get_average_quality_rating(AudioStream *stream);

/* enable ZRTP on the audio stream */
MS2_PUBLIC void audio_stream_enable_zrtp(AudioStream *stream, OrtpZrtpParams *params);

/* enable SRTP on the audio stream */
MS2_PUBLIC bool_t audio_stream_enable_strp(AudioStream* stream, enum ortp_srtp_crypto_suite_t suite, const char* snd_key, const char* rcv_key);

MS2_PUBLIC VoiceMemoRecordStream *voice_memo_start_record (const char * file, MSSndCard *sndcard);
MS2_PUBLIC void voice_memo_stop_record (VoiceMemoRecordStream * stream);
MS2_PUBLIC VoiceMemoPlayStream *voice_memo_start_play (const char * file, MSSndCard *sndcard);
MS2_PUBLIC void voice_memo_stop_play (VoiceMemoPlayStream * stream);
MS2_PUBLIC void voice_mix_flag (AudioStream* stream,const char * file);
#if ENABLE_AUDIO_ENGENEER_MODEL
MS2_PUBLIC Playrecstream *voice_playrec_start(MSSndCard *sndcard, MSSndCard *captcard);
MS2_PUBLIC void voice_playrec_stop(Playrecstream * stream);
#endif
/**
 * @}
**/


/**
 * @addtogroup video_stream_api
 * @{
**/

typedef void (*VideoStreamRenderCallback)(void *user_pointer, const MSPicture *local_view, const MSPicture *remote_view);
typedef void (*VideoStreamEventCallback)(void *user_pointer, const MSFilter *f, const unsigned int event_id, const void *args);


typedef enum _VideoStreamDir{
    VideoStreamSendRecv,
    VideoStreamSendOnly,
    VideoStreamRecvOnly
}VideoStreamDir;

struct _VideoStream
{
    MediaStream ms;
    MSFilter *source;
    MSFilter *pixconv;
    MSFilter *sizeconv;
    MSFilter *tee;
    MSFilter *output;
    MSFilter *tee2;
    MSFilter *jpegwriter;
    MSFilter *filewriter;
    MSFilter *itcsinkforfilewriter;
    MSFilter *itcsourceforfilewriter;
    MSTicker *tickerforfilewriter;
    MSFilter *sourceforIPCamStreaming;
    MSTicker *tickerforIPCamStreaming;
    MSTicker *tickerforCamPlayback;
    MSFilter *output2;
    MSFilter *teeforrecord;
    MSFilter *itcsink;
    struct {
        MSFilter *audio_input;
        MSFilter *video_input;
        MSFilter *recorder;

        MSTicker *ticker;
    } av_recorder;
    struct _AudioStream *audiostream;/*the stream with which this videostream is paired*/
    MSVideoSize sent_vsize;
    int corner; /*for selfview*/
    VideoStreamRenderCallback rendercb;
    void *render_pointer;
    VideoStreamEventCallback eventcb;
    void *event_pointer;
    char *display_name;
    unsigned long window_id;
    unsigned long preview_window_id;
    VideoStreamDir dir;
    MSWebCam *cam;
    bool_t use_preview_window;
    int device_orientation; /* warning: meaning of this variable depends on the platform (Android, iOS, ...) */
};

typedef struct _VideoStream VideoStream;

MS2_PUBLIC VideoStream * ipcam_streaming_start();
MS2_PUBLIC void ipcam_streaming_stop(VideoStream *stream);
MS2_PUBLIC void ipcam_stream_recorder_start(VideoStream *stream, const char *file);
MS2_PUBLIC void ipcam_stream_recorder_stop(VideoStream *stream);
MS2_PUBLIC void ipcam_stream_snapshot(VideoStream *stream, const char *file);

MS2_PUBLIC VideoStream *video_stream_new(int loc_rtp_port, int loc_rtcp_port, bool_t use_ipv6, bool_t call_mobile);
MS2_PUBLIC VideoStream *video_stream_new_with_sessions(const MSMediaStreamSessions *sessions);
MS2_PUBLIC VideoStream *video_stream_udp_new(unsigned short loc_port, bool_t use_ipv6,bool_t call_mobile);
MS2_PUBLIC void video_stream_set_direction(VideoStream *vs, VideoStreamDir dir);
static MS2_INLINE void video_stream_enable_adaptive_bitrate_control(VideoStream *stream, bool_t enabled) {
    media_stream_enable_adaptive_bitrate_control(&stream->ms, enabled);
}
MS2_PUBLIC void video_stream_set_render_callback(VideoStream *s, VideoStreamRenderCallback cb, void *user_pointer);
MS2_PUBLIC void video_stream_set_event_callback(VideoStream *s, VideoStreamEventCallback cb, void *user_pointer);
MS2_PUBLIC void video_stream_set_display_filter_name(VideoStream *s, const char *fname);
MS2_PUBLIC int video_stream_start_with_source (VideoStream *stream, RtpProfile *profile, const char *rem_rtp_ip, int rem_rtp_port,
    const char *rem_rtcp_ip, int rem_rtcp_port, int payload, int jitt_comp, bool_t mobile_call, bool_t video_from_ipcam, MSWebCam* cam, MSFilter* source);
MS2_PUBLIC int video_stream_start(VideoStream * stream, RtpProfile *profile, const char *rem_rtp_ip, int rem_rtp_port, const char *rem_rtcp_ip, int rem_rtcp_port,
    int payload, int jitt_comp, bool_t mobile_call, bool_t video_from_ipcam, MSWebCam *device);
MS2_PUBLIC int video_stream_udp_start(VideoStream * stream, const char *rem_ip, int rem_port,
    bool_t mobile_call, bool_t video_from_ipcam, MSWebCam *device);

MS2_PUBLIC void video_stream_set_relay_session_id(VideoStream *stream, const char *relay_session_id);
static MS2_INLINE void video_stream_set_rtcp_information(VideoStream *st, const char *cname, const char *tool) {
    media_stream_set_rtcp_information(&st->ms, cname, tool);
}
MS2_PUBLIC void video_stream_change_camera(VideoStream *stream, MSWebCam *cam);
/* Calling video_stream_set_sent_video_size() or changing the bitrate value in the used PayloadType during a stream is running does nothing.
The following function allows to take into account new parameters by redrawing the sending graph*/
MS2_PUBLIC void video_stream_update_video_params(VideoStream *stream);
/*function to call periodically to handle various events */
MS2_PUBLIC void video_stream_iterate(VideoStream *stream);

/**
 * Ask the video stream to send a Full-Intra Request.
 * @param[in] stream The videostream object.
 */
MS2_PUBLIC void video_stream_send_vfu(VideoStream *stream);

/**
 * Ask the video stream to generate a Video Fast Update (generally after receiving a Full-Intra Request.
 * @param[in] stream The videostream object.
 */
MS2_PUBLIC void video_stream_stop(VideoStream * stream);
MS2_PUBLIC void video_stream_udp_stop(VideoStream * stream);
MS2_PUBLIC void video_stream_set_sent_video_size(VideoStream *stream, MSVideoSize vsize);

/**
 * Returns the name of the video display filter on the current platform.
**/
MS2_PUBLIC const char *video_stream_get_default_video_renderer(void);

MS2_PUBLIC void video_stream_enable_self_view(VideoStream *stream, bool_t val);
MS2_PUBLIC unsigned long video_stream_get_native_window_id(VideoStream *stream);
MS2_PUBLIC void video_stream_set_native_window_id(VideoStream *stream, unsigned long id);
MS2_PUBLIC void video_stream_set_native_preview_window_id(VideoStream *stream, unsigned long id);
MS2_PUBLIC unsigned long video_stream_get_native_preview_window_id(VideoStream *stream);
MS2_PUBLIC void video_stream_use_preview_video_window(VideoStream *stream, bool_t yesno);
MS2_PUBLIC void video_stream_set_device_rotation(VideoStream *stream, int orientation);
MS2_PUBLIC void video_stream_show_video(VideoStream *stream, bool_t show);

/*provided for compatibility, use video_stream_set_direction() instead */
MS2_PUBLIC int video_stream_recv_only_start(VideoStream *videostream, RtpProfile *profile, const char *addr, int port, int used_pt, int jitt_comp);
MS2_PUBLIC int video_stream_send_only_start(VideoStream *videostream,
                RtpProfile *profile, const char *addr, int port, int rtcp_port,
                int used_pt, int  jitt_comp, MSWebCam *device);
MS2_PUBLIC void video_stream_recv_only_stop(VideoStream *vs);
MS2_PUBLIC void video_stream_send_only_stop(VideoStream *vs);

MS2_PUBLIC void video_stream_start_record_mkv(VideoStream *st, const char *name);
MS2_PUBLIC void video_stream_stop_record_mkv(VideoStream *st);
MS2_PUBLIC void video_stream_start_play_mkv(VideoStream *st, const char *name);
MS2_PUBLIC void video_stream_stop_play_mkv(VideoStream *st);

/* enable ZRTP on the video stream using information from the audio stream */
MS2_PUBLIC void video_stream_enable_zrtp(VideoStream *vstream, AudioStream *astream, OrtpZrtpParams *param);

/* enable SRTP on the video stream */
MS2_PUBLIC bool_t video_stream_enable_strp(VideoStream* stream, enum ortp_srtp_crypto_suite_t suite, const char* snd_key, const char* rcv_key);

/**
 * Small API to display a local preview window.
**/

typedef VideoStream VideoPreview;

MS2_PUBLIC VideoPreview * video_preview_new(void);
#define video_preview_set_size(p,s)                             video_stream_set_sent_video_size(p,s)
#define video_preview_set_display_filter_name(p,dt) video_stream_set_display_filter_name(p,dt)
#define video_preview_set_native_window_id(p,id)        video_stream_set_native_preview_window_id (p,id)
#define video_preview_get_native_window_id(p)           video_stream_get_native_preview_window_id (p)
MS2_PUBLIC void video_preview_start(VideoPreview *stream, MSWebCam *device);
MS2_PUBLIC void video_preview_stop(VideoPreview *stream);

/**
 * @}
**/

MS2_PUBLIC bool_t ms_is_ipv6(const char *address);
void castor3snd_deinit_for_video_memo_play(void);
void castor3snd_reinit_for_video_memo_play(void);


#ifdef __cplusplus
}
#endif

#endif
