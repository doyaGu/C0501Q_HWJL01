/****************************************************************************
 *
 *  $Id: commands.c,v 1.39 2008/07/03 15:08:34 smorlat Exp $
 *
 *  Copyright (C) 2006-2009  Sandro Santilli <strk@keybit.net>
 *  Copyright (C) 2004  Simon MORLAT <simon.morlat@linphone.org>
 *
****************************************************************************
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 ****************************************************************************/

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#ifndef _WIN32_WCE
#include <errno.h>
#include <unistd.h>
#endif /*_WIN32_WCE*/
#include <limits.h>
#include <ctype.h>
#include <linphonecore.h>
#include "linphonec.h"
#include "lpconfig.h"

#if 1 //ndef WIN32
#include <sys/wait.h>
#endif

#include "coreapi/private.h"
#include "mediastreamer2/mediastream.h"
#include "linphone_castor3.h"
#include "ite/ite_codec.h"
#if (CFG_CHIP_FAMILY == 9910)
#include "video_encoder/video_encoder_it9910.h"
#else
#include "video_encoder/video_encoder.h"
#endif

#define AUDIO 0
#define VIDEO 1

#ifdef ENABLE_AUDIO_ENGENEER_MODEL
#define SOUND "Esound"
#else
#define SOUND "sound"
#endif    

/***************************************************************************
 *
 *  Forward declarations
 *
 ***************************************************************************/

extern char *lpc_strip_blanks(char *input);

/* Command handlers */
static int lpc_cmd_help(LinphoneCore *, char *);
static int lpc_cmd_proxy(LinphoneCore *, char *);
static int lpc_cmd_call(LinphoneCore *, char *);
static int lpc_cmd_calls(LinphoneCore *, char *);
static int lpc_cmd_chat(LinphoneCore *, char *);
static int lpc_cmd_answer(LinphoneCore *, char *);
static int lpc_cmd_autoanswer(LinphoneCore *, char *);
static int lpc_cmd_terminate(LinphoneCore *, char *);
static int lpc_cmd_redirect(LinphoneCore *, char *);
static int lpc_cmd_call_logs(LinphoneCore *, char *);
static int lpc_cmd_ipv6(LinphoneCore *, char *);
static int lpc_cmd_transfer(LinphoneCore *, char *);
static int lpc_cmd_quit(LinphoneCore *, char *);
static int lpc_cmd_nat(LinphoneCore *, char *);
static int lpc_cmd_stun(LinphoneCore *, char *);
static int lpc_cmd_firewall(LinphoneCore *, char *);
static int lpc_cmd_friend(LinphoneCore *, char*);
static int lpc_cmd_soundcard(LinphoneCore *, char *);
static int lpc_cmd_webcam(LinphoneCore *, char *);
static int lpc_cmd_staticpic(LinphoneCore *, char *);
static int lpc_cmd_play(LinphoneCore *, char *);
static int lpc_cmd_record(LinphoneCore *, char *);
static int lpc_cmd_register(LinphoneCore *, char *);
static int lpc_cmd_unregister(LinphoneCore *, char *);
static int lpc_cmd_duration(LinphoneCore *lc, char *args);
static int lpc_cmd_status(LinphoneCore *lc, char *args);
static int lpc_cmd_ports(LinphoneCore *lc, char *args);
static int lpc_cmd_param(LinphoneCore *lc, char *args);
static int lpc_cmd_speak(LinphoneCore *lc, char *args);
static int lpc_cmd_acodec(LinphoneCore *lc, char *args);
static int lpc_cmd_vcodec(LinphoneCore *lc, char *args);
static int lpc_cmd_codec(int type, LinphoneCore *lc, char *args);
static int lpc_cmd_echocancellation(LinphoneCore *lc, char *args);
static int lpc_cmd_echolimiter(LinphoneCore *lc, char *args);
static int lpc_cmd_pause(LinphoneCore *lc, char *args);
static int lpc_cmd_resume(LinphoneCore *lc, char *args);
static int lpc_cmd_mute_mic(LinphoneCore *lc, char *args);
static int lpc_cmd_unmute_mic(LinphoneCore *lc, char *args);
static int lpc_cmd_playback_gain(LinphoneCore *lc, char *args);
static int lpc_cmd_rtp_no_xmit_on_audio_mute(LinphoneCore *lc, char *args);
#ifdef VIDEO_ENABLED
static int lpc_cmd_camera(LinphoneCore *lc, char *args);
static int lpc_cmd_video_window(LinphoneCore *lc, char *args);
static int lpc_cmd_preview_window(LinphoneCore *lc, char *args);
static int lpc_cmd_snapshot(LinphoneCore *lc, char *args);
static int lpc_cmd_vfureq(LinphoneCore *lc, char *arg);
#endif
static int lpc_cmd_states(LinphoneCore *lc, char *args);
static int lpc_cmd_identify(LinphoneCore *lc, char *args);
static int lpc_cmd_ringback(LinphoneCore *lc, char *args);
static int lpc_cmd_conference(LinphoneCore *lc, char *args);
static int lpc_cmd_zrtp_verified(LinphoneCore *lc, char *args);
static int lpc_cmd_zrtp_unverified(LinphoneCore *lc, char *args);

static int lpc_cmd_castor3_set_ring(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_play_ring(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_set_play_level(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_set_rec_level(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_set_ring_level(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_play_ringsound(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_play_keysound(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_play_voicesound(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_play_warnsound(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_add_friend(LinphoneCore *lc, char *args);
int lpc_cmd_castor3_get_friends(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_update_friend(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_delete_friend(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_clear_friends(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_clear_call_logs_missed(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_clear_call_logs_received(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_clear_call_logs_sent(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_delete_call_log(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_play_video_msg(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_call(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_terminate_others(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_stop_sound(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_record_video_msg(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_set_do_not_disturb(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_set_presence(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_watch_camera(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_stop_camera(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_start_voice_memo_record(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_stop_voice_memo_record(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_start_voice_memo_play(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_stop_voice_memo_play(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_start_ipcam_streaming(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_stop_ipcam_streaming(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_start_ipcam_record(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_stop_ipcam_record(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_ipcam_snapshot(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_start_video_memo_record(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_stop_video_memo_record(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_start_audiostream_memo_record(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_stop_audiostream_memo_record(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_start_video_memo_play(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_stop_video_memo_play(LinphoneCore *lc, char *args);
static int lpc_cmd_bind_ipcam(LinphoneCore *lc, char *args);
static int lpc_cmd_cancel_bind_ipcam(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_start_adv_play(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_stop_adv_play(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_start_media_video_play(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_stop_media_video_play(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_start_media_audio_play(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_stop_media_audio_play(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_start_playrec(LinphoneCore *lc);
static int lpc_cmd_castor3_stop_playrec(LinphoneCore *lc);
static int lpc_cmd_castor3_start_camera_playback(LinphoneCore *lc, char *args);
static int lpc_cmd_castor3_stop_camera_playback(LinphoneCore *lc, char *args);

/* Command handler helpers */
static void linphonec_proxy_add(LinphoneCore *lc);
static void linphonec_proxy_display(LinphoneProxyConfig *lc);
static void linphonec_proxy_list(LinphoneCore *lc);
static void linphonec_proxy_remove(LinphoneCore *lc, int index);
static  int linphonec_proxy_use(LinphoneCore *lc, int index);
static void linphonec_proxy_show(LinphoneCore *lc,int index);
static void linphonec_friend_display(LinphoneFriend *fr);
static int linphonec_friend_list(LinphoneCore *lc, char *arg);
static void linphonec_display_command_help(LPC_COMMAND *cmd);
static int linphonec_friend_call(LinphoneCore *lc, unsigned int num);
#if 1 //ndef WIN32
static int linphonec_friend_add(LinphoneCore *lc, const char *name, const char *addr);
#endif
static int linphonec_friend_delete(LinphoneCore *lc, int num);
static int linphonec_friend_delete(LinphoneCore *lc, int num);
static void linphonec_codec_list(int type, LinphoneCore *lc);
static void linphonec_codec_enable(int type, LinphoneCore *lc, int index);
static void linphonec_codec_disable(int type, LinphoneCore *lc, int index);
static void lpc_display_call_states(LinphoneCore *lc);

/* Command table management */
static LPC_COMMAND *lpc_find_command(const char *name);

void linphonec_out(const char *fmt,...);

VideoParams lpc_video_params={-1,-1,-1,-1,0,TRUE};
VideoParams lpc_preview_params={-1,-1,-1,-1,0,TRUE};

/***************************************************************************
 *
 *  Global variables
 *
 ***************************************************************************/

/*
 * Commands table.
 */
static LPC_COMMAND commands[] = {
    { "help", lpc_cmd_help, "Print commands help.",
        "'help <command>'\t: displays specific help for command.\n"
        "'help advanced'\t: shows advanced commands.\n"
    },
    { "call", lpc_cmd_call, "Call a SIP uri or number",
#ifdef VIDEO_ENABLED
        "'call <sip-url or number>  [options]' \t: initiate a call to the specified destination.\n"
        "Options can be:\n"
        "--audio-only : initiate the call without video.\n"
        "--early-media : sends audio and video stream immediately when remote proposes early media.\n"
#else
        "'call <sip-url or number>' \t: initiate a call to the specified destination.\n"
#endif
        },
    { "calls", lpc_cmd_calls, "Show all the current calls with their id and status.",
        NULL
        },
    { "chat", lpc_cmd_chat, "Chat with a SIP uri",
        "'chat <sip-url> \"message\"' "
        ": send a chat message \"message\" to the specified destination."
        },
    { "terminate", lpc_cmd_terminate, "Terminate a call",
        "'terminate' : Terminate the current call\n"
        "'terminate <call id>' : Terminate the call with supplied id\n"
        "'terminate <all>' : Terminate all the current calls\n"
        },
    { "answer", lpc_cmd_answer, "Answer a call",
        "'answer' : Answer the current incoming call\n"
        "'answer <call id>' : Answer the call with given id\n"
    },
    { "pause", lpc_cmd_pause, "pause a call",
        "'pause' : pause the current call\n"},
    { "resume", lpc_cmd_resume, "resume a call",
        "'resume' : resume the unique call\n"
        "'resume <call id>' : hold off the call with given id\n"},
    { "transfer", lpc_cmd_transfer,
        "Transfer a call to a specified destination.",
        "'transfer <sip-uri>' : transfers the current active call to the destination sip-uri\n"
        "'transfer <call id> <sip-uri>': transfers the call with 'id' to the destination sip-uri\n"
        "'transfer <call id1> --to-call <call id2>': transfers the call with 'id1' to the destination of call 'id2' (attended transfer)\n"
    },
    { "conference", lpc_cmd_conference, "Create and manage an audio conference.",
        "'conference add <call id> : join the call with id 'call id' into the audio conference."
        "'conference rm <call id> : remove the call with id 'call id' from the audio conference."
    },
    { "mute", lpc_cmd_mute_mic,
      "Mute microphone and suspend voice transmission."},
#ifdef VIDEO_ENABLED
    { "camera", lpc_cmd_camera, "Send camera output for current call.",
        "'camera on'\t: allow sending of local camera video to remote end.\n"
        "'camera off'\t: disable sending of local camera's video to remote end.\n"},
#endif
    { "unmute", lpc_cmd_unmute_mic,
          "Unmute microphone and resume voice transmission."},
    { "playbackgain", lpc_cmd_playback_gain,
          "Adjust playback gain."},
    { "duration", lpc_cmd_duration, "Print duration in seconds of the last call.", NULL },

    { "autoanswer", lpc_cmd_autoanswer, "Show/set auto-answer mode",
        "'autoanswer'       \t: show current autoanswer mode\n"
        "'autoanswer enable'\t: enable autoanswer mode\n"
        "'autoanswer disable'\t: disable autoanswer mode?\n"},
    { "proxy", lpc_cmd_proxy, "Manage proxies",
        "'proxy list' : list all proxy setups.\n"
        "'proxy add' : add a new proxy setup.\n"
        "'proxy remove <index>' : remove proxy setup with number index.\n"
        "'proxy use <index>' : use proxy with number index as default proxy.\n"
        "'proxy unuse' : don't use a default proxy.\n"
        "'proxy show <index>' : show configuration and status of the proxy numbered by index.\n"
        "'proxy show default' : show configuration and status of the default proxy.\n"
    },
    { "soundcard", lpc_cmd_soundcard, "Manage soundcards",
        "'soundcard list' : list all sound devices.\n"
        "'soundcard show' : show current sound devices configuration.\n"
        "'soundcard use <index>' : select a sound device.\n"
        "'soundcard use files' : use .wav files instead of soundcard\n"
    },
    { "webcam", lpc_cmd_webcam, "Manage webcams",
        "'webcam list' : list all known devices.\n"
        "'webcam use <index>' : select a video device.\n"
    },
    { "ipv6", lpc_cmd_ipv6, "Use IPV6",
        "'ipv6 status' : show ipv6 usage status.\n"
        "'ipv6 enable' : enable the use of the ipv6 network.\n"
        "'ipv6 disable' : do not use ipv6 network."
    },
    { "nat", lpc_cmd_nat, "Set nat address",
        "'nat'        : show nat settings.\n"
        "'nat <addr>' : set nat address.\n"
    },
    { "stun", lpc_cmd_stun, "Set stun server address",
        "'stun'        : show stun settings.\n"
        "'stun <addr>' : set stun server address.\n"
    },
    { "firewall", lpc_cmd_firewall, "Set firewall policy",
        "'firewall'        : show current firewall policy.\n"
        "'firewall none'   : use direct connection.\n"
        "'firewall nat'    : use nat address given with the 'nat' command.\n"
        "'firewall stun'   : use stun server given with the 'stun' command.\n"
    },
    { "call-logs", lpc_cmd_call_logs, "Calls history", NULL },
    { "friend", lpc_cmd_friend, "Manage friends",
        "'friend list [<pattern>]'    : list friends.\n"
        "'friend call <index>'        : call a friend.\n"
        "'friend add <name> <addr>'   : add friend, <name> must be quoted to include\n"
        "                               spaces, <addr> has \"sip:\" added if it isn't\n"
        "                               there.  Don't use '<' '>' around <addr>.\n"
        "'friend delete <index>'      : remove friend, 'all' removes all\n"
    },
    { "play", lpc_cmd_play, "play a wav file",
        "This command has two roles:\n"
        "Plays a file instead of capturing from soundcard - only available in file mode (see 'help soundcard')\n"
        "Specifies a wav file to be played to play music to far end when putting it on hold (pause)\n"
        "'play <wav file>'    : play a wav file."
    },
    { "record", lpc_cmd_record, "record to a wav file",
        "This feature is available only in file mode (see 'help soundcard')\n"
        "'record <wav file>'    : record into wav file."
    },
    { "quit", lpc_cmd_quit, "Exit linphonec", NULL },
    { "castor3-set-ring", lpc_cmd_castor3_set_ring, "", NULL },
    { "castor3-play-ring", lpc_cmd_castor3_play_ring, "", NULL },
    { "castor3-set-play-level", lpc_cmd_castor3_set_play_level, "", NULL },
    { "castor3-set-rec-level", lpc_cmd_castor3_set_rec_level, "", NULL },
    { "castor3-set-ring-level", lpc_cmd_castor3_set_ring_level, "", NULL },
    { "castor3-play-keysound", lpc_cmd_castor3_play_keysound, "", NULL },
    { "castor3-play-voicesound", lpc_cmd_castor3_play_voicesound, "", NULL },
    { "castor3-play-warnsound", lpc_cmd_castor3_play_warnsound, "", NULL },
    { "castor3-add-friend", lpc_cmd_castor3_add_friend, "", NULL },
    { "castor3-get-friends", lpc_cmd_castor3_get_friends, "", NULL },
    { "castor3-update-friend", lpc_cmd_castor3_update_friend, "", NULL },
    { "castor3-delete-friend", lpc_cmd_castor3_delete_friend, "", NULL },
    { "castor3-clear-friends", lpc_cmd_castor3_clear_friends, "", NULL },
    { "castor3-clear-call-logs-missed", lpc_cmd_castor3_clear_call_logs_missed, "", NULL },
    { "castor3-clear-call-logs-received", lpc_cmd_castor3_clear_call_logs_received, "", NULL },
    { "castor3-clear-call-logs-sent", lpc_cmd_castor3_clear_call_logs_sent, "", NULL },
    { "castor3-delete-call-log", lpc_cmd_castor3_delete_call_log, "", NULL },
    { "castor3-play-video-msg", lpc_cmd_castor3_play_video_msg, "", NULL },
    { "castor3-call", lpc_cmd_castor3_call, "", NULL },
    { "castor3-terminate-others", lpc_cmd_castor3_terminate_others, "", NULL },
    { "castor3-stop-sound", lpc_cmd_castor3_stop_sound, "", NULL },
    { "castor3-record-video-msg", lpc_cmd_castor3_record_video_msg, "", NULL },
    { "castor3-set-do-not-disturb", lpc_cmd_castor3_set_do_not_disturb, "", NULL },
    { "castor3-set-presence", lpc_cmd_castor3_set_presence, "", NULL },
    { "castor3-watch-camera", lpc_cmd_castor3_watch_camera, "", NULL },
    { "castor3-stop-camera", lpc_cmd_castor3_stop_camera, "", NULL },
    { "castor3-start-voice-memo-record", lpc_cmd_castor3_start_voice_memo_record, "", NULL },
    { "castor3-stop-voice-memo-record", lpc_cmd_castor3_stop_voice_memo_record, "", NULL },
    { "castor3-start-voice-memo-play", lpc_cmd_castor3_start_voice_memo_play, "", NULL },
    { "castor3-stop-voice-memo-play", lpc_cmd_castor3_stop_voice_memo_play, "", NULL },
    { "castor3-start-video-memo-record", lpc_cmd_castor3_start_video_memo_record, "", NULL },
    { "castor3-stop-video-memo-record", lpc_cmd_castor3_stop_video_memo_record, "", NULL },
    { "castor3-start-audiostream-memo-record", lpc_cmd_castor3_start_audiostream_memo_record, "", NULL },
    { "castor3-stop-audiostream-memo-record", lpc_cmd_castor3_stop_audiostream_memo_record, "", NULL },  
    { "castor3-start-video-memo-play", lpc_cmd_castor3_start_video_memo_play, "", NULL },
    { "castor3-stop-video-memo-play", lpc_cmd_castor3_stop_video_memo_play, "", NULL },
    { "castor3-bind-ipcam", lpc_cmd_bind_ipcam, "", NULL },
    { "castor3-cancel-bind-ipcam", lpc_cmd_cancel_bind_ipcam, "", NULL },
    { "castor3-start-adv-play", lpc_cmd_castor3_start_adv_play, "", NULL },
    { "castor3-stop-adv-play", lpc_cmd_castor3_stop_adv_play, "", NULL },
    { "castor3-start-media-video-play", lpc_cmd_castor3_start_media_video_play, "", NULL },
    { "castor3-stop-media-video-play", lpc_cmd_castor3_stop_media_video_play, "", NULL },
    { "castor3-start-media-audio-play", lpc_cmd_castor3_start_media_audio_play, "", NULL },
    { "castor3-stop-media-audio-play", lpc_cmd_castor3_stop_media_audio_play, "", NULL },
    { "castor3-start-ipcam-streaming", lpc_cmd_castor3_start_ipcam_streaming, "", NULL },
    { "castor3-stop-ipcam-streaming", lpc_cmd_castor3_stop_ipcam_streaming, "", NULL },
    { "castor3-start-ipcam-record", lpc_cmd_castor3_start_ipcam_record, "", NULL },
    { "castor3-stop-ipcam-record", lpc_cmd_castor3_stop_ipcam_record, "", NULL },
    { "castor3-ipcam-snapshot", lpc_cmd_castor3_ipcam_snapshot, "", NULL },
#if ENABLE_AUDIO_ENGENEER_MODEL
    { "castor3-start-playrec", lpc_cmd_castor3_start_playrec, "", NULL },
    { "castor3-stop-playrec", lpc_cmd_castor3_stop_playrec, "", NULL },
#endif
    { "castor3-start-camera-playback", lpc_cmd_castor3_start_camera_playback, "", NULL },
    { "castor3-stop-camera-playback", lpc_cmd_castor3_stop_camera_playback, "", NULL },
    { (char *)NULL, (lpc_cmd_handler)NULL, (char *)NULL, (char *)NULL }
};


static LPC_COMMAND advanced_commands[] = {
     { "codec", lpc_cmd_acodec, "Audio codec configuration",
            "'codec list' : list audio codecs\n"
            "'codec enable <index>' : enable available audio codec\n"
            "'codec disable <index>' : disable audio codec" },
    { "vcodec", lpc_cmd_vcodec, "Video codec configuration",
            "'vcodec list' : list video codecs\n"
            "'vcodec enable <index>' : enable available video codec\n"
            "'vcodec disable <index>' : disable video codec" },
    { "ec", lpc_cmd_echocancellation, "Echo cancellation",
        "'ec on [<delay>] [<tail>] [<framesize>]' : turn EC on with given delay, tail length and framesize\n"
        "'ec off' : turn echo cancellation (EC) off\n"
        "'ec show' : show EC status" },
    { "el", lpc_cmd_echolimiter, "Echo limiter",
        "'el on turns on echo limiter (automatic half duplex, for cases where echo canceller cannot work)\n"
        "'el off' : turn echo limiter off\n"
        "'el show' : show echo limiter status" },
    { "nortp-on-audio-mute", lpc_cmd_rtp_no_xmit_on_audio_mute,
          "Set the rtp_no_xmit_on_audio_mute configuration parameter",
          "   If set to 1 then rtp transmission will be muted when\n"
          "   audio is muted , otherwise rtp is always sent."},
#ifdef VIDEO_ENABLED
    { "vwindow", lpc_cmd_video_window, "Control video display window",
        "'vwindow show': shows video window\n"
        "'vwindow hide': hides video window\n"
        "'vwindow pos <x> <y>': Moves video window to x,y pixel coordinates\n"
        "'vwindow size <width> <height>': Resizes video window\n"
        "'vwindow id <window id>': embeds video display into supplied window id."
    },
    { "pwindow", lpc_cmd_preview_window, "Control local camera video display (preview window)",
        "'pwindow show': shows the local camera video display\n"
        "'pwindow hide': hides the local camera video display\n"
        "'pwindow pos <x> <y>': Moves preview window to x,y pixel coordinates\n"
        "'pwindow size <width> <height>': Resizes preview window\n"
        "'pwindow id <window id>': embeds preview display into supplied window id.\n"
        "'pwindow integrated': integrate preview display within the video window of current call.\n"
        "'pwindow standalone': use standalone window for preview display."
    },
    { "snapshot", lpc_cmd_snapshot, "Take a snapshot of currently received video stream",
        "'snapshot <file path>': take a snapshot and records it in jpeg format into the supplied path\n"
    },
    { "vfureq", lpc_cmd_vfureq, "Request the other side to send VFU for the current call"},
#endif
    { "states", lpc_cmd_states, "Show internal states of liblinphone, registrations and calls, according to linphonecore.h definitions",
        "'states global': shows global state of liblinphone \n"
        "'states calls': shows state of calls\n"
        "'states proxies': shows state of proxy configurations"
    },
    { "register", lpc_cmd_register, "Register in one line to a proxy" , "register <sip identity> <sip proxy> <password>"},
    { "unregister", lpc_cmd_unregister, "Unregister from default proxy", NULL   },
    { "status", lpc_cmd_status, "Print various status information",
            "'status register'  \t: print status concerning registration\n"
            "'status autoanswer'\t: tell whether autoanswer mode is enabled\n"
            "'status hook'      \t: print hook status\n" },
    { "ports", lpc_cmd_ports, "Network ports configuration",
            "'ports'  \t: prints current used ports.\n"
            "'ports sip <port number>'\t: Sets the sip port.\n" },
    { "param", lpc_cmd_param, "parameter set or read as normally given in .linphonerc",
            "'param <section> <parameter> [<value>]'  \t: reads [sets] given parameter.\n"
            "NOTES: - changes may become effective after (re)establishing a sip connection.\n"
            "       - upon exit, .linphonerc will reflect the updated state.\n" },
    { "speak", lpc_cmd_speak, "Speak a sentence using espeak TTS engine",
            "This feature is available only in file mode. (see 'help soundcard')\n"
            "'speak <voice name> <sentence>'    : speak a text using the specified espeak voice.\n"
            "Example for english voice: 'speak default Hello my friend !'"
    },
    { "staticpic", lpc_cmd_staticpic, "Manage static pictures when nowebcam",
        "'staticpic set' : Set path to picture that should be used.\n"
        "'staticpic fps' : Get/set frames per seconds for picture emission.\n"
    },
    { "identify", lpc_cmd_identify, "Returns the user-agent string of far end",
        "'identify' \t: returns remote user-agent string for current call.\n"
        "'identify <id>' \t: returns remote user-agent string for call with supplied id.\n"
    },
    { "ringback", lpc_cmd_ringback, "Specifies a ringback tone to be played to remote end during incoming calls",
        "'ringback <path of mono .wav file>'\t: Specifies a ringback tone to be played to remote end during incoming calls\n"
        "'ringback disable'\t: Disable playing of ringback tone to callers\n"
    },
    { "redirect", lpc_cmd_redirect, "Redirect an incoming call",
        "'redirect <redirect-uri>'\t: Redirect all pending incoming calls to the <redirect-uri>\n"
    },
    { "zrtp-set-verified", lpc_cmd_zrtp_verified,"Set ZRTP SAS verified.",
        "'Set ZRTP SAS verified'\n"
    },
    { "zrtp-set-unverified", lpc_cmd_zrtp_unverified,"Set ZRTP SAS not verified.",
        "'Set ZRTP SAS not verified'\n"
    },
    {   NULL,NULL,NULL,NULL}
};



/***************************************************************************
 *
 *  Public interface
 *
 ***************************************************************************/

/*
 * Main command dispatcher.
 * WARNING: modifies second argument!
 *
 * Always return 1 currently.
 */
int
linphonec_parse_command_line(LinphoneCore *lc, char *cl)
{
    char *ptr=cl;
    char *args=NULL;
    LPC_COMMAND *cmd;

    /* Isolate first word and args */
    while(*ptr && !isspace(*ptr)) ++ptr;
    if (*ptr)
    {
        *ptr='\0';
        /* set args to first nonblank */
        args=ptr+1;
        while(*args && isspace(*args)) ++args;
    }

    /* Handle DTMF */
    if ( isdigit(*cl) || *cl == '#' || *cl == '*' )
    {
        while ( isdigit(*cl) || *cl == '#' || *cl == '*' )
        {
            linphone_core_send_dtmf(lc, *cl);
            linphone_core_play_dtmf (lc,*cl,100);
            ms_sleep(1); // be nice
            ++cl;
        }

        // discard spurious trailing chars
        return 1;
    }

    /* Handle other kind of commands */
    cmd=lpc_find_command(cl);
    if ( !cmd )
    {
        linphonec_out("'%s': Cannot understand this.\n", cl);
        return 1;
    }

    if ( ! cmd->func(lc, args) )
    {
        linphonec_out("Syntax error.\n");
        linphonec_display_command_help(cmd);
    }

    return 1;
}

/*
 * Generator function for command completion.
 * STATE let us know whether to start from scratch;
 * without any state (STATE==0), then we start at the
 * top of the list.
 */
char *
linphonec_command_generator(const char *text, int state)
{
    static int index, len, adv;
    char *name;

    if ( ! state )
    {
        index=0;
        adv=0;
        len=strlen(text);
    }
    /*
     * Return the next name which partially matches
     * from the commands list
     */
    if (adv==0){
        while ((name=commands[index].name))
        {
            ++index; /* so next call get next command */

            if (strncmp(name, text, len) == 0)
            {
                return ortp_strdup(name);
            }
        }
        adv=1;
        index=0;
    }
    if (adv==1){
        while ((name=advanced_commands[index].name))
        {
            ++index; /* so next call get next command */

            if (strncmp(name, text, len) == 0)
            {
                return ortp_strdup(name);
            }
        }
    }
    return NULL;
}


/***************************************************************************
 *
 *  Command handlers
 *
 ***************************************************************************/

static int
lpc_cmd_help(LinphoneCore *lc, char *arg)
{
    int i=0;
    LPC_COMMAND *cmd;

    if (!arg || !*arg)
    {
        linphonec_out("Commands are:\n");
        linphonec_out("---------------------------\n");

        while (commands[i].help)
        {
            linphonec_out("%10.10s\t%s\n", commands[i].name,
                commands[i].help);
            i++;
        }

        linphonec_out("---------------------------\n");
        linphonec_out("Type 'help <command>' for more details or\n");
        linphonec_out("     'help advanced' to list additional commands.\n");

        return 1;
    }

    if (strcmp(arg,"advanced")==0){
        linphonec_out("Advanced commands are:\n");
        linphonec_out("---------------------------\n");
        i=0;
        while (advanced_commands[i].help)
        {
            linphonec_out("%10.10s\t%s\n", advanced_commands[i].name,
                advanced_commands[i].help);
            i++;
        }

        linphonec_out("---------------------------\n");
        linphonec_out("Type 'help <command>' for more details.\n");

        return 1;
    }

    cmd=lpc_find_command(arg);
    if ( !cmd )
    {
        linphonec_out("No such command.\n");
        return 1;
    }

    linphonec_display_command_help(cmd);
    return 1;

}

static char callee_name[256]={0};
static char caller_name[256]={0};


static int
lpc_cmd_call(LinphoneCore *lc, char *args)
{
#ifdef CFG_SIP_SERVER_TEST
	/*who want to call*/
	char temp_cut[] = "@";  //eason
	char *args_check;

	args_check = strstr(args, temp_cut)+1; //args = toto@xxx...
#endif

    if ( ! args || ! *args )
    {
        return 0;
    }
    {
        LinphoneCall *call;
        LinphoneCallParams *cp;
        char *opt1,*opt2, *opt3, *opt4, *opt5;
#ifdef CFG_SIP_SERVER_TEST
		char temp_indoor[sizeof(CFG_REGISTER_DOMAIN)+sizeof(args_check)+5],
			 temp_old[sizeof(CFG_REGISTER_DOMAIN)+sizeof(CFG_SIP_CALLOUT)+5];

#ifdef CFG_SIP_INDOOR_TEST
if (memcmp(args_check, "192.168", 7) != 0){
		if (memcmp(args_check, "9", 2) == 0){
			strcpy(temp_old, "sip:");
			strcat(temp_old, CFG_SIP_CALLOUT);
			strcat(temp_old, "@");
			strcat(temp_old, CFG_REGISTER_DOMAIN);

			args = temp_old;
		}else{
			strcpy(temp_indoor, "sip:");
			strcat(temp_indoor, args_check);
			strcat(temp_indoor, "@");
			strcat(temp_indoor, CFG_REGISTER_DOMAIN);

		args = temp_indoor;
	}
}
#elif CFG_SIP_LOBBY_TEST
		strcpy(temp_old, "sip:");
		strcat(temp_old, CFG_SIP_CALLOUT);
		strcat(temp_old, "@");
		strcat(temp_old, CFG_REGISTER_DOMAIN);
		args = temp_old;
#endif
#endif

        if ( linphone_core_in_call(lc) )
        {
            linphonec_out("Terminate or hold on the current call first.\n");
            return 1;
        }
        cp=linphone_core_create_default_call_parameters (lc);
        opt1=strstr(args,"--audio-only");
        opt2=strstr(args,"--early-media");
        opt3=strstr(args,"--play-video-msg");
        opt4=strstr(args,"--video-from-ipcam");
        opt5=strstr(args,"--audio_only_send");
        if (opt1){
            opt1[0]='\0';
            linphone_call_params_enable_video (cp,FALSE);
        }
        if (opt2){
            opt2[0]='\0';
            linphone_call_params_enable_early_media_sending(cp,TRUE);
        }
        if (opt3){
            opt3[0]='\0';
            linphone_call_params_enable_play_video_msg(cp,TRUE);
        }
        if (opt4){
            opt4[0]='\0';
            linphone_call_params_enable_video_from_ipcam(cp, TRUE);
        }
        if (opt5){
            opt5[0]='\0';
            linphone_call_paprms_disable_audio_recive_graph(cp,FALSE);
        }
        if ( NULL == (call=linphone_core_invite_with_params(lc, args,cp)) )
        {
            linphonec_out("Error from linphone_core_invite.\n");
        }
        else
        {
            snprintf(callee_name,sizeof(callee_name),"%s",args);
        }
        linphone_call_params_destroy(cp);
    }
    //if(lc->presence_mode != LinphoneStatusDoNotDisturb)
    lc->presence_mode = LinphoneStatusBusy;
    return 1;
}

static int
lpc_cmd_calls(LinphoneCore *lc, char *args){
    const MSList *calls = linphone_core_get_calls(lc);
    if(calls)
    {
        lpc_display_call_states(lc);
    }else
    {
        linphonec_out("No active call.\n");
    }
    return 1;
}


static int
lpc_cmd_chat(LinphoneCore *lc, char *args)
{
    char *arg1 = args;
    char *arg2 = NULL;
    char *ptr = args;
    LinphoneChatRoom *cr;

    if (!args) return 0;

    /* Isolate first and second arg */
    while(*ptr && !isspace(*ptr)) ++ptr;
    if ( *ptr )
    {
        *ptr='\0';
        arg2=ptr+1;
        while(*arg2 && isspace(*arg2)) ++arg2;
    }
    else
    {
        /* missing one parameter */
        return 0;
    }
    cr = linphone_core_create_chat_room(lc,arg1);
    linphone_chat_room_send_message(cr,arg2);
    linphone_chat_room_destroy(cr);

    return 1;
}

const char *linphonec_get_callee(){
    return callee_name;
}

const char *linphonec_get_caller(){
    return caller_name;
}

void linphonec_set_caller(const char *caller){
    snprintf(caller_name,sizeof(caller_name)-1,"%s",caller);
}

static int
lpc_cmd_transfer(LinphoneCore *lc, char *args)
{
    if (args){
        LinphoneCall *call;
        LinphoneCall *call2;
        const char *refer_to=NULL;
        char arg1[256]={0};
        char arg2[266]={0};
        long id2=0;
        int n=sscanf(args,"%s %s %li",arg1,arg2,&id2);
        if (n==1 || isalpha(*arg1)){
            call=linphone_core_get_current_call(lc);
            if (call==NULL && ms_list_size(linphone_core_get_calls(lc))==1){
                call=(LinphoneCall*)linphone_core_get_calls(lc)->data;
            }
            refer_to=args;
            if (call==NULL){
                linphonec_out("No active call, please specify a call id among the ones listed by 'calls' command.\n");
                return 0;
            }
            linphone_core_transfer_call(lc, call, refer_to);
        }else if (n==2){
            long id=atoi(arg1);
            refer_to=args+strlen(arg1)+1;
            call=linphonec_get_call(id);
            if (call==NULL) return 0;
            linphone_core_transfer_call(lc, call, refer_to);
        }else if (n==3){
            long id=atoi(arg1);
            call=linphonec_get_call(id);
            call2=linphonec_get_call(id2);
            if (call==NULL || call2==NULL) return 0;
            if (strcmp(arg2,"--to-call")!=0){
                return 0;
            }
            linphonec_out("Performing attended transfer of call %i to call %i",id,id2);
            linphone_core_transfer_call_to_another (lc,call,call2);
        }else return 0;
    }else{
        linphonec_out("Transfer command requires at least one argument\n");
        return 0;
    }
    return 1;
}

static int
lpc_cmd_terminate(LinphoneCore *lc, char *args)
{
    if (linphone_core_get_calls(lc)==NULL){
        linphonec_out("No active calls\n");
        return 1;
    }
    if (!args)
    {
        if ( -1 == linphone_core_terminate_call(lc, NULL) ){
            linphonec_out("Could not stop the active call.\n");
        }
        return 1;
    }

    if(strcmp(args,"all")==0){
        linphonec_out("We are going to stop all the calls.\n");
        linphone_core_terminate_all_calls(lc);
        return 1;
    }else{
        /*the argument is a linphonec call id */
        long id=atoi(args);
        LinphoneCall *call=linphonec_get_call(id);
        if (call){
            if (linphone_core_terminate_call(lc,call)==-1){
                linphonec_out("Could not stop the call with id %li\n",id);
            }
        }else return 0;
        return 1;
    }
    return 0;

}

static int
lpc_cmd_redirect(LinphoneCore *lc, char *args){
    const MSList *elem;
    int didit=0;
    if (!args) return 0;
    if ((elem=linphone_core_get_calls(lc))==NULL){
        linphonec_out("No active calls.\n");
        return 1;
    }
    while(elem!=NULL){
        LinphoneCall *call=(LinphoneCall*)elem->data;
        if (linphone_call_get_state(call)==LinphoneCallIncomingReceived ||
            linphone_call_get_state(call)==LinphoneCallIncomingEarlyMedia){
            linphone_core_redirect_call(lc,call,args);
            didit=1;
            /*as the redirection closes the call, we need to re-check the call list that is invalidated.*/
            elem=linphone_core_get_calls(lc);
        }else elem=elem->next;
    }
    if (didit==0){
        linphonec_out("There is no pending incoming call to redirect.");
    }
    return 1;
}

static int
lpc_cmd_answer(LinphoneCore *lc, char *args){
    if (!args)
    {
        int nb=ms_list_size(linphone_core_get_calls(lc));
        if (nb==1){
            //if just one call is present answer the only one in passing NULL to the linphone_core_accept_call ...
            if ( -1 == linphone_core_accept_call(lc, NULL) )
            {
                linphonec_out("Fail to accept incoming call\n");
            }
        }else if (nb==0){
            linphonec_out("There are no calls to answer.\n");
        }else{
            linphonec_out("Multiple calls in progress, please specify call id.\n");
            return 0;
        }
        return 1;
    }else{
        long id;
        if (sscanf(args,"%li",&id)==1){
            LinphoneCall *call=linphonec_get_call (id);
            if (linphone_core_accept_call (lc,call)==-1){
                linphonec_out("Fail to accept call %i\n",id);
            }
        }else return 0;
        return 1;
    }
    return 0;
}

static int
lpc_cmd_autoanswer(LinphoneCore *lc, char *args)
{
    if ( ! args )
    {
        if ( linphonec_get_autoanswer() ) {
            linphonec_out("Auto answer is enabled. Use 'autoanswer disable' to disable.\n");
        } else {
            linphonec_out("Auto answer is disabled. Use 'autoanswer enable' to enable.\n");
        }
        return 1;
    }

    if (strstr(args,"enable")){
        linphonec_set_autoanswer(TRUE);
        linphonec_out("Auto answer enabled.\n");
    }else if (strstr(args,"disable")){
        linphonec_set_autoanswer(FALSE);
        linphonec_out("Auto answer disabled.\n");
    }else return 0;
    return 1;
}

static int
lpc_cmd_quit(LinphoneCore *lc, char *args)
{
    linphonec_main_loop_exit();
    return 1;
}

static int
lpc_cmd_nat(LinphoneCore *lc, char *args)
{
    bool_t use;
    const char *nat;

    if ( args ) args=lpc_strip_blanks(args);

    if ( args && *args )
    {
        linphone_core_set_nat_address(lc, args);
        /* linphone_core_set_firewall_policy(lc,LINPHONE_POLICY_USE_NAT_ADDRESS); */
    }

    nat = linphone_core_get_nat_address(lc);
    use = linphone_core_get_firewall_policy(lc)==LinphonePolicyUseNatAddress;
    linphonec_out("Nat address: %s%s\n", nat ? nat : "unspecified" , use ? "" : " (disabled - use 'firewall nat' to enable)");

    return 1;
}

static int
lpc_cmd_stun(LinphoneCore *lc, char *args)
{
    bool_t use;
    const char *stun;

    if ( args ) args=lpc_strip_blanks(args);

    if ( args && *args )
    {
        linphone_core_set_stun_server(lc, args);
        /* linphone_core_set_firewall_policy(lc,LINPHONE_POLICY_USE_STUN); */
    }

    stun = linphone_core_get_stun_server(lc);
    use = linphone_core_get_firewall_policy(lc)==LinphonePolicyUseStun;
    linphonec_out("Stun server: %s%s\n", stun ? stun : "unspecified" , use? "" : " (disabled - use 'firewall stun' to enable)");

    return 1;
}

static int
lpc_cmd_firewall(LinphoneCore *lc, char *args)
{
    const char* setting=NULL;

    if ( args ) args=lpc_strip_blanks(args);

    if ( args && *args )
    {
        if (strcmp(args,"none")==0)
        {
            linphone_core_set_firewall_policy(lc,LinphonePolicyNoFirewall);
        }
        else if (strcmp(args,"stun")==0)
        {
            setting = linphone_core_get_stun_server(lc);
            if ( ! setting )
            {
                linphonec_out("No stun server address is defined, use 'stun <address>' first\n");
                return 1;
            }
            linphone_core_set_firewall_policy(lc,LinphonePolicyUseStun);
        }
        else if (strcmp(args,"nat")==0)
        {
            setting = linphone_core_get_nat_address(lc);
            if ( ! setting )
            {
                linphonec_out("No nat address is defined, use 'nat <address>' first");
                return 1;
            }
            linphone_core_set_firewall_policy(lc,LinphonePolicyUseNatAddress);
        }
    }

    switch(linphone_core_get_firewall_policy(lc))
    {
        case LinphonePolicyNoFirewall:
            linphonec_out("No firewall\n");
            break;
        case LinphonePolicyUseStun:
            linphonec_out("Using stun server %s to discover firewall address\n", setting ? setting : linphone_core_get_stun_server(lc));
            break;
        case LinphonePolicyUseNatAddress:
            linphonec_out("Using supplied nat address %s.\n", setting ? setting : linphone_core_get_nat_address(lc));
            break;
    }
    return 1;
}

#if 1 //ndef WIN32
/* Helper function for processing freind names */
static int
lpc_friend_name(char **args, char **name)
{
    /* Use space as a terminator unless quoted */
    if (('"' == **args) || ('\'' == **args)){
        char *end;
        char delim = **args;
        (*args)++;
        end = (*args);
        while ((delim != *end) && ('\0' != *end)) end++;
        if ('\0' == *end) {
            fprintf(stderr, "Mismatched quotes\n");
            return 0;
        }
        *name = *args;
        *end = '\0';
        *args = ++end;
    } else {
        *name = strsep(args, " ");

        if (NULL == *args) { /* Means there was no separator */
            fprintf(stderr, "Either name or address is missing\n");
            return 0;
        }
        if (NULL == *name) return 0;
    }
    return 1;
}
#endif

static int
lpc_cmd_friend(LinphoneCore *lc, char *args)
{
    int friend_num;

    if ( args ) args=lpc_strip_blanks(args);

    if ( ! args || ! *args ) return 0;

    if ( !strncmp(args, "list", 4) )
    {
        return linphonec_friend_list(lc, args+4);
        return 1;
    }
    else if ( !strncmp(args, "call", 4) )
    {
        args+=4;
        if ( ! *args ) return 0;
        friend_num = strtol(args, NULL, 10);
#ifndef _WIN32_WCE
        if ( errno == ERANGE ) {
            linphonec_out("Invalid friend number\n");
            return 0;
        }
#endif /*_WIN32_WCE*/
        linphonec_friend_call(lc, friend_num);
        return 1;
    }
    else if ( !strncmp(args, "delete", 6) )
    {
        args+=6;
        if ( ! *args ) return 0;
        while (*args == ' ') args++;
        if ( ! *args ) return 0;
        if (!strncmp(args, "all", 3))
        {
            friend_num = -1;
        }
        else
        {
            friend_num = strtol(args, NULL, 10);
#ifndef _WIN32_WCE
            if ( errno == ERANGE ) {
                linphonec_out("Invalid friend number\n");
                return 0;
            }
#endif /*_WIN32_WCE*/
        }
        linphonec_friend_delete(lc, friend_num);
        return 1;
    }
    else if ( !strncmp(args, "add", 3) )
    {
#if 1 //ndef WIN32
        char  *name;
        char  addr[80];
        char *addr_p = addr;
        char *addr_orig;

        args+=3;
        if ( ! *args ) return 0;
        while (*args == ' ') args++;
        if ( ! *args ) return 0;

        if (!lpc_friend_name(&args,  &name)) return 0;

        while (*args == ' ') args++;
        if ( ! *args ) return 0;
        if (isdigit(*args)) {
            strcpy (addr, "sip:");
            addr_p = addr + strlen("sip:");
        }
        addr_orig = strsep(&args, " ");
        if (1 >= strlen(addr_orig)) {
            fprintf(stderr, "A single-digit address is not valid\n");
            return 0;
        }
        strcpy(addr_p, addr_orig);
        linphonec_friend_add(lc, name, addr);
#else
        LinphoneFriend *new_friend;
        new_friend = linphone_friend_new_with_addr(args);
        linphone_core_add_friend(lc, new_friend);
#endif
        return 1;
    }
    return 0;
}

static int lpc_cmd_play(LinphoneCore *lc, char *args){
    if ( args ) args=lpc_strip_blanks(args);
    if ( ! args || ! *args ) return 0;
    linphone_core_set_play_file(lc,args);
    return 1;
}

static int lpc_cmd_record(LinphoneCore *lc, char *args){
    if ( args ) args=lpc_strip_blanks(args);
    if ( ! args || ! *args ) return 0;
    linphone_core_set_record_file(lc,args);
    return 1;
}

static int lpc_cmd_bind_ipcam(LinphoneCore *lc, char *args){
    if ( args ) args=lpc_strip_blanks(args);
    if ( ! args || ! *args ) return 0;
    linphone_core_set_ipcam_address(lc,args);
    return 1;
}

static int lpc_cmd_cancel_bind_ipcam(LinphoneCore *lc, char *args){
    if (lc->ipcam_address!=NULL){
        ms_free(lc->ipcam_address);
        lc->ipcam_address=NULL;
    }
    return 1;
}

/*
 * Modified input
 */
static int
lpc_cmd_proxy(LinphoneCore *lc, char *args)
{
    char *arg1 = args;
    char *arg2 = NULL;
    char *ptr = args;
    int proxynum;

    if ( ! arg1 ) return 0;

    /* Isolate first and second arg */
    while(*ptr && !isspace(*ptr)) ++ptr;
    if ( *ptr )
    {
        *ptr='\0';
        arg2=ptr+1;
        while(*arg2 && isspace(*arg2)) ++arg2;
    }

    if (strcmp(arg1,"add")==0)
    {
#ifdef HAVE_READLINE
        rl_inhibit_completion=1;
#endif
        linphonec_proxy_add(lc);
#ifdef HAVE_READLINE
        rl_inhibit_completion=0;
#endif
    }
    else if (strcmp(arg1,"list")==0)
    {
        linphonec_proxy_list(lc);
    }
    else if (strcmp(arg1,"remove")==0)
    {
        linphonec_proxy_remove(lc,atoi(arg2));
    }
    else if (strcmp(arg1,"use")==0)
    {
        if ( arg2 && *arg2 )
        {
            proxynum=atoi(arg2);
            if ( linphonec_proxy_use(lc, proxynum) )
                linphonec_out("Default proxy set to %d.\n", proxynum);
        }
        else
        {
            proxynum=linphone_core_get_default_proxy(lc, NULL);
            if ( proxynum == -1 ) linphonec_out("No default proxy.\n");
            else linphonec_out("Current default proxy is %d.\n", proxynum);
        }
    }else if (strcmp(arg1, "unuse")==0){
        linphone_core_set_default_proxy(lc, NULL);
        linphonec_out("Use no proxy.\n");
    }

    else if (strcmp(arg1, "show")==0)
    {
        if (arg2 && *arg2)
        {
            if (strstr(arg2,"default"))
            {
        proxynum=linphone_core_get_default_proxy(lc, NULL);
        if ( proxynum < 0 ) {
            linphonec_out("No default proxy defined\n");
            return 1;
        }
        linphonec_proxy_show(lc,proxynum);
            }
            else
            {
        linphonec_proxy_show(lc, atoi(arg2));
            }
        }
        else return 0; /* syntax error */
    }

    else
    {
        return 0; /* syntax error */
    }

    return 1;
}

static int
lpc_cmd_call_logs(LinphoneCore *lc, char *args)
{
    const MSList *elem=linphone_core_get_call_logs(lc);
    for (;elem!=NULL;elem=ms_list_next(elem))
    {
        LinphoneCallLog *cl=(LinphoneCallLog*)elem->data;
        char *str=linphone_call_log_to_str(cl);
        linphonec_out("%s\n",str);
        ms_free(str);
    }
    return 1;
}

static int
lpc_cmd_ipv6(LinphoneCore *lc, char *arg1)
{
    if ( ! arg1 )
    {
        return 0; /* syntax error */
    }

    if (strcmp(arg1,"status")==0)
    {
        linphonec_out("ipv6 use enabled: %s\n",linphone_core_ipv6_enabled(lc) ? "true":"false");
    }
    else if (strcmp(arg1,"enable")==0)
    {
        linphone_core_enable_ipv6(lc,TRUE);
        linphonec_out("ipv6 use enabled.\n");
    }
    else if (strcmp(arg1,"disable")==0)
    {
        linphone_core_enable_ipv6(lc,FALSE);
        linphonec_out("ipv6 use disabled.\n");
    }
    else
    {
        return 0; /* syntax error */
    }
    return 1;
}

static int devname_to_index(LinphoneCore *lc, const char *devname){
    const char **p;
    int i;
    for(i=0,p=linphone_core_get_sound_devices(lc);*p!=NULL;++p,++i){
        if (strcmp(devname,*p)==0) return i;
    }
    return -1;
}

static const char *index_to_devname(LinphoneCore *lc, int index){
    const char **p;
    int i;
    for(i=0,p=linphone_core_get_sound_devices(lc);*p!=NULL;++p,++i){
        if (i==index) return *p;
    }
    return NULL;
}

static int lpc_cmd_soundcard(LinphoneCore *lc, char *args)
{
    int i, index;
    const char **dev;
    char *arg1 = args;
    char *arg2 = NULL;
    char *ptr = args;

    if (!args) return 0; /* syntax error */

    /* Isolate first and second arg */
    while(*ptr && !isspace(*ptr)) ++ptr;
    if ( *ptr )
    {
        *ptr='\0';
        arg2=ptr+1;
        while(*arg2 && isspace(*arg2)) ++arg2;
    }

    if (strcmp(arg1, "list")==0)
    {
        dev=linphone_core_get_sound_devices(lc);
        for(i=0; dev[i]!=NULL; ++i){
            linphonec_out("%i: %s\n",i,dev[i]);
        }
        return 1;
    }

    if (strcmp(arg1, "show")==0)
    {
        linphonec_out("Ringer device: %s\n",
            linphone_core_get_ringer_device(lc));
        linphonec_out("Playback device: %s\n",
            linphone_core_get_playback_device(lc));
        linphonec_out("Capture device: %s\n",
            linphone_core_get_capture_device(lc));
        return 1;
    }

    if (strcmp(arg1, "use")==0 && arg2)
    {
        if (strcmp(arg2, "files")==0)
        {
            linphonec_out("Using wav files instead of soundcard.\n");
            linphone_core_use_files(lc,TRUE);
            return 1;
        }

        dev=linphone_core_get_sound_devices(lc);
        index=atoi(arg2); /* FIXME: handle not-a-number */
        for(i=0;dev[i]!=NULL;i++)
        {
            if (i!=index) continue;

            linphone_core_set_ringer_device(lc,dev[i]);
            linphone_core_set_playback_device(lc,dev[i]);
            linphone_core_set_capture_device(lc,dev[i]);
            linphonec_out("Using sound device %s\n",dev[i]);
            return 1;
        }
        linphonec_out("No such sound device\n");
        return 1;
    }
    if (strcmp(arg1, "capture")==0)
    {
        const char *devname=linphone_core_get_capture_device(lc);
        if (!arg2){
            linphonec_out("Using capture device #%i (%s)\n",
                    devname_to_index(lc,devname),devname);
        }else{
            index=atoi(arg2); /* FIXME: handle not-a-number */
            devname=index_to_devname(lc,index);
            if (devname!=NULL){
                linphone_core_set_capture_device(lc,devname);
                linphonec_out("Using capture sound device %s\n",devname);
                return 1;
            }
            linphonec_out("No such sound device\n");
        }
        return 1;
    }
    if (strcmp(arg1, "playback")==0)
    {
        const char *devname=linphone_core_get_playback_device(lc);
        if (!arg2){
            linphonec_out("Using playback device #%i (%s)\n",
                    devname_to_index(lc,devname),devname);
        }else{
            index=atoi(arg2); /* FIXME: handle not-a-number */
            devname=index_to_devname(lc,index);
            if (devname!=NULL){
                linphone_core_set_playback_device(lc,devname);
                linphonec_out("Using playback sound device %s\n",devname);
                return 1;
            }
            linphonec_out("No such sound device\n");
        }
        return 1;
    }
    if (strcmp(arg1, "ring")==0)
    {
        const char *devname=linphone_core_get_ringer_device(lc);
        if (!arg2){
            linphonec_out("Using ring device #%i (%s)\n",
                    devname_to_index(lc,devname),devname);
        }else{
            index=atoi(arg2); /* FIXME: handle not-a-number */
            devname=index_to_devname(lc,index);
            if (devname!=NULL){
                linphone_core_set_ringer_device(lc,devname);
                linphonec_out("Using ring sound device %s\n",devname);
                return 1;
            }
            linphonec_out("No such sound device\n");
        }
        return 1;
    }
    return 0; /* syntax error */
}

static int lpc_cmd_webcam(LinphoneCore *lc, char *args)
{
    int i, index;
    const char **dev;
    char *arg1 = args;
    char *arg2 = NULL;
    char *ptr = args;

    if (!args) return 0; /* syntax error */

    /* Isolate first and second arg */
    while(*ptr && !isspace(*ptr)) ++ptr;
    if ( *ptr )
    {
        *ptr='\0';
        arg2=ptr+1;
        while(*arg2 && isspace(*arg2)) ++arg2;
    }

    if (strcmp(arg1, "list")==0)
    {
        dev=linphone_core_get_video_devices(lc);
        for(i=0; dev[i]!=NULL; ++i){
            linphonec_out("%i: %s\n",i,dev[i]);
        }
        return 1;
    }

    if (strcmp(arg1, "use")==0 && arg2)
    {
        dev=linphone_core_get_video_devices(lc);
        index=atoi(arg2); /* FIXME: handle not-a-number */
        for(i=0;dev[i]!=NULL;i++)
        {
            if (i!=index) continue;

            linphone_core_set_video_device(lc, dev[i]);
            linphonec_out("Using video device %s\n",dev[i]);
            return 1;
        }
        linphonec_out("No such video device\n");
        return 1;
    }
    return 0; /* syntax error */
}

static int
lpc_cmd_staticpic(LinphoneCore *lc, char *args)
{
    char *arg1 = args;
    char *arg2 = NULL;
    char *ptr = args;

    if (!args) return 0;  /* Syntax error */

    /* Isolate first and second arg */
    while(*ptr && !isspace(*ptr)) ++ptr;
    if ( *ptr )
    {
        *ptr='\0';
        arg2=ptr+1;
        while(*arg2 && isspace(*arg2)) ++arg2;
    }

    if (strcmp(arg1, "set")==0 && arg2) {
        linphone_core_set_static_picture(lc, arg2);
        return 1;
    }

    if (strcmp(arg1, "fps")==0) {
      if (arg2) {
            float fps = atof(arg2); /* FIXME: Handle not-a-float */
        linphone_core_set_static_picture_fps(lc, fps);
        return 1;
      } else {
        float fps;
        fps = linphone_core_get_static_picture_fps(lc);
        linphonec_out("Current FPS %f\n", fps);
        return 1;
      }
    }

    return 0; /* Syntax error */
}

static int lpc_cmd_pause(LinphoneCore *lc, char *args){

    if(linphone_core_in_call(lc))
    {
        linphone_core_pause_call(lc,linphone_core_get_current_call(lc));
        return 1;
    }
    linphonec_out("you can only pause when a call is in process\n");
    return 0;
}

static int lpc_cmd_resume(LinphoneCore *lc, char *args){

    if(linphone_core_in_call(lc))
    {
        linphonec_out("There is already a call in process pause or stop it first");
        return 1;
    }
    if (args)
    {
        long id;
        int n = sscanf(args, "%li", &id);
        if (n == 1){
            LinphoneCall *call=linphonec_get_call (id);
            if (call){
                if(linphone_core_resume_call(lc,call)==-1){
                    linphonec_out("There was a problem to resume the call check the remote address you gave %s\n",args);
                }
            }
            return 1;
        }else return 0;
    }
    else
    {
        const MSList *calls = linphone_core_get_calls(lc);
        int nbcalls=ms_list_size(calls);
        if( nbcalls == 1)
        {
            if(linphone_core_resume_call(lc,calls->data) < 0)
            {
                linphonec_out("There was a problem to resume the unique call.\n");
            }
            return 1;
        }else if (nbcalls==0){
            linphonec_out("There is no calls at this time.\n");
            return 1;
        }else{
            linphonec_out("There are %i calls at this time, please specify call id as given with 'calls' command.\n");
        }
    }
    return 0;

}

static int lpc_cmd_conference(LinphoneCore *lc, char *args){
    long id;
    char subcommand[32]={0};
    int n;
    if (args==NULL) return 0;
    n=sscanf(args, "%31s %li", subcommand,&id);
    if (n == 2){
        LinphoneCall *call=linphonec_get_call(id);
        if (call==NULL) return 1;
        if (strcmp(subcommand,"add")==0){
            linphone_core_add_to_conference(lc,call);
            return 1;
        }else if (strcmp(subcommand,"rm")==0){
            linphone_core_remove_from_conference(lc,call);
            return 1;
        }else if (strcmp(subcommand,"enter")==0){
            linphone_core_enter_conference(lc);
            return 1;
        }else if (strcmp(subcommand,"leave")==0){
            linphone_core_leave_conference(lc);
            return 1;
        }
    }
    return 0;
}

/***************************************************************************
 *
 *  Commands helper functions
 *
 ***************************************************************************/


static void
linphonec_proxy_add(LinphoneCore *lc)
{
    bool_t enable_register=FALSE;
    LinphoneProxyConfig *cfg;

    linphonec_out("Adding new proxy setup. Hit ^D to abort.\n");

    /*
     * SIP Proxy address
     */
    while (1)
    {
        char *input=linphonec_readline("Enter proxy sip address: ");
        char *clean;

        if ( ! input ) {
            linphonec_out("Aborted.\n");
            return;
        }

        /* Strip blanks */
        clean=lpc_strip_blanks(input);
        if ( ! *clean ) {
            free(input);
            continue;
        }

        cfg=linphone_proxy_config_new();
        if (linphone_proxy_config_set_server_addr(cfg,clean)<0)
        {
            linphonec_out("Invalid sip address (sip:sip.domain.tld).\n");
            free(input);
            linphone_proxy_config_destroy(cfg);
            continue;
        }
        free(input);
        break;
    }

    /*
     * SIP Proxy identity
     */
    while (1)
    {
        char *input=linphonec_readline("Your identity for this proxy: ");
        char *clean;

        if ( ! input ) {
            linphonec_out("Aborted.\n");
            linphone_proxy_config_destroy(cfg);
            return;
        }

        /* Strip blanks */
        clean=lpc_strip_blanks(input);
        if ( ! *clean ) {
            free(input);
            continue;
        }

        linphone_proxy_config_set_identity(cfg, clean);
        if ( ! linphone_proxy_config_get_identity (cfg))
        {
            linphonec_out("Invalid identity (sip:name@sip.domain.tld).\n");
            free(input);
            continue;
        }
        free(input);
        break;
    }

    /*
     * SIP Proxy enable register
     */
    while (1)
    {
        char *input=linphonec_readline("Do you want to register on this proxy (yes/no): ");
        char *clean;

        if ( ! input ) {
            linphonec_out("Aborted.\n");
            linphone_proxy_config_destroy(cfg);
            return;
        }

        /* Strip blanks */
        clean=lpc_strip_blanks(input);
        if ( ! *clean ) {
            free(input);
            continue;
        }

        if ( ! strcmp(clean, "yes") ) enable_register=TRUE;
        else if ( ! strcmp(clean, "no") ) enable_register=FALSE;
        else {
            linphonec_out("Please answer with 'yes' or 'no'\n");
            free(input);
            continue;
        }
        linphone_proxy_config_enableregister(cfg, enable_register);
        free(input);
        break;
    }

    /*
     * SIP Proxy registration expiration
     */
    if ( enable_register==TRUE )
    {
        long int expires=0;
        while (1)
        {
            char *input=linphonec_readline("Specify register expiration time"
                " in seconds (default is 600): ");

            if ( ! input ) {
                linphonec_out("Aborted.\n");
                linphone_proxy_config_destroy(cfg);
                return;
            }

            expires=strtol(input, (char **)NULL, 10);
            if ( expires == LONG_MIN || expires == LONG_MAX )
            {
                linphonec_out("Invalid value: %s\n", strerror(errno));
                free(input);
                continue;
            }

            linphone_proxy_config_expires(cfg, expires);
            linphonec_out("Expiration: %d seconds\n", linphone_proxy_config_get_expires (cfg));

            free(input);
            break;
        }
    }

    /*
     * SIP proxy route
     */
    while (1)
    {
        char *input=linphonec_readline("Specify route if needed: ");
        char *clean;

        if ( ! input ) {
            linphonec_out("Aborted.\n");
            linphone_proxy_config_destroy(cfg);
            return;
        }

        /* Strip blanks */
        clean=lpc_strip_blanks(input);
        if ( ! *clean ) {
            free(input);
            linphonec_out("No route specified.\n");
            break;
        }

        linphone_proxy_config_set_route(cfg, clean);
        if ( ! linphone_proxy_config_get_route(cfg) )
        {
            linphonec_out("Invalid route.\n");
            free(input);
            continue;
        }

        free(input);
        break;
    }

    /*
     * Final confirmation
     */
    while (1)
    {
        char *input;
        char *clean;

        linphonec_out("--------------------------------------------\n");
        linphonec_proxy_display(cfg);
        linphonec_out("--------------------------------------------\n");
        input=linphonec_readline("Accept the above proxy configuration (yes/no) ?: ");


        if ( ! input ) {
            linphonec_out("Aborted.\n");
            linphone_proxy_config_destroy(cfg);
            return;
        }

        /* Strip blanks */
        clean=lpc_strip_blanks(input);
        if ( ! *clean ) {
            free(input);
            continue;
        }

        if ( ! strcmp(clean, "yes") ) break;
        else if ( ! strcmp(clean, "no") )
        {
            linphonec_out("Declined.\n");
            linphone_proxy_config_destroy(cfg);
            free(input);
            return;
        }

        linphonec_out("Please answer with 'yes' or 'no'\n");
        free(input);
        continue;
    }


    linphone_core_add_proxy_config(lc,cfg);

    /* automatically set the last entered proxy as the default one */
    linphone_core_set_default_proxy(lc,cfg);

    linphonec_out("Proxy added.\n");
}

static void
linphonec_proxy_display(LinphoneProxyConfig *cfg)
{
    const char *route=linphone_proxy_config_get_route(cfg);
    const char *identity=linphone_proxy_config_get_identity(cfg);
    linphonec_out("sip address: %s\nroute: %s\nidentity: %s\nregister: %s\nexpires: %i\nregistered: %s\n",
            linphone_proxy_config_get_addr(cfg),
            (route!=NULL)? route:"",
            (identity!=NULL)?identity:"",
            linphone_proxy_config_register_enabled (cfg)?"yes":"no",
            linphone_proxy_config_get_expires (cfg),
            linphone_proxy_config_is_registered(cfg) ? "yes" : "no");
}

static void linphonec_proxy_show(LinphoneCore *lc, int index)
{
    const MSList *elem;
    int i;
    for(elem=linphone_core_get_proxy_config_list(lc),i=0;elem!=NULL;elem=elem->next,++i){
        if (index==i){
            LinphoneProxyConfig *cfg=(LinphoneProxyConfig *)elem->data;
            linphonec_proxy_display(cfg);
            return;
        }
    }
    linphonec_out("No proxy with index %i\n", index);
}

static void
linphonec_proxy_list(LinphoneCore *lc)
{
    const MSList *proxies;
    int n;
    int def=linphone_core_get_default_proxy(lc,NULL);

    proxies=linphone_core_get_proxy_config_list(lc);
    for(n=0;proxies!=NULL;proxies=ms_list_next(proxies),n++){
        if (n==def)
            linphonec_out("****** Proxy %i - this is the default one - *******\n",n);
        else
            linphonec_out("****** Proxy %i *******\n",n);
        linphonec_proxy_display((LinphoneProxyConfig*)proxies->data);
    }
    if ( ! n ) linphonec_out("No proxies defined\n");
}

static void
linphonec_proxy_remove(LinphoneCore *lc, int index)
{
    const MSList *proxies;
    LinphoneProxyConfig *cfg;
    proxies=linphone_core_get_proxy_config_list(lc);
    cfg=(LinphoneProxyConfig*)ms_list_nth_data(proxies,index);
    if (cfg==NULL){
        linphonec_out("No such proxy.\n");
        return;
    }
    linphone_core_remove_proxy_config(lc,cfg);
    linphonec_out("Proxy %s removed.\n", linphone_proxy_config_get_addr(cfg));
}

static int
linphonec_proxy_use(LinphoneCore *lc, int index)
{
    const MSList *proxies;
    LinphoneProxyConfig *cfg;
    proxies=linphone_core_get_proxy_config_list(lc);
    cfg=(LinphoneProxyConfig*)ms_list_nth_data(proxies,index);
    if (cfg==NULL){
        linphonec_out("No such proxy (try 'proxy list').");
        return 0;
    }
    linphone_core_set_default_proxy(lc,cfg);
    return 1;
}

static void
linphonec_friend_display(LinphoneFriend *fr)
{
    LinphoneAddress *uri=linphone_address_clone(linphone_friend_get_address(fr));
    char *str;

    linphonec_out("name: %s\n", linphone_address_get_display_name(uri));
    linphone_address_set_display_name(uri,NULL);
    str=linphone_address_as_string(uri);
    linphonec_out("address: %s\n", str);
}

static int
linphonec_friend_list(LinphoneCore *lc, char *pat)
{
    const MSList *friend;
    int n;

    if (pat) {
        pat=lpc_strip_blanks(pat);
        if (!*pat) pat = NULL;
    }

    friend = linphone_core_get_friend_list(lc);
    for(n=0; friend!=NULL; friend=ms_list_next(friend), ++n )
    {
        if ( pat ) {
            const char *name = linphone_address_get_display_name(
                linphone_friend_get_address((LinphoneFriend*)friend->data));
            if (name && ! strstr(name, pat) ) continue;
        }
        linphonec_out("****** Friend %i *******\n",n);
        linphonec_friend_display((LinphoneFriend*)friend->data);
    }

    return 1;
}

static int
linphonec_friend_call(LinphoneCore *lc, unsigned int num)
{
    const MSList *friend = linphone_core_get_friend_list(lc);
    unsigned int n;
    char *addr;

    for(n=0; friend!=NULL; friend=ms_list_next(friend), ++n )
    {
        if ( n == num )
        {
            int ret;
            addr = linphone_address_as_string(linphone_friend_get_address((LinphoneFriend*)friend->data));
            ret=lpc_cmd_call(lc, addr);
            ms_free(addr);
            return ret;
        }
    }
    linphonec_out("No such friend %u\n", num);
    return 1;
}

#if 1 //ndef WIN32
static int
linphonec_friend_add(LinphoneCore *lc, const char *name, const char *addr)
{
    LinphoneFriend *newFriend;

    char url[PATH_MAX];

    snprintf(url, PATH_MAX, "%s <%s>", name, addr);
    newFriend = linphone_friend_new_with_addr(url);
    linphone_core_add_friend(lc, newFriend);
    return 0;
}
#endif

static int
linphonec_friend_delete(LinphoneCore *lc, int num)
{
    const MSList *friend = linphone_core_get_friend_list(lc);
    unsigned int n;

    for(n=0; friend!=NULL; friend=ms_list_next(friend), ++n )
    {
        if ( n == num )
        {
            linphone_core_remove_friend(lc, friend->data);
            return 0;
        }
    }

    if (-1 == num)
    {
        unsigned int i;
        for (i = 0 ; i < n ; i++)
            linphonec_friend_delete(lc, 0);
        return 0;
    }

    linphonec_out("No such friend %u\n", num);
    return 1;
}

static void
linphonec_display_command_help(LPC_COMMAND *cmd)
{
    if ( cmd->doc ) linphonec_out ("%s\n", cmd->doc);
    else linphonec_out("%s\n", cmd->help);
}


static int lpc_cmd_register(LinphoneCore *lc, char *args){
    char identity[512];
    char proxy[512];
    char passwd[512];
    LinphoneProxyConfig *cfg;
    const MSList *elem;

    if (!args)
        {
            /* it means that you want to register the default proxy */
            LinphoneProxyConfig *cfg=NULL;
            linphone_core_get_default_proxy(lc,&cfg);
            if (cfg)
            {
                if(!linphone_proxy_config_is_registered(cfg)) {
                linphone_proxy_config_enable_register(cfg,TRUE);
                linphone_proxy_config_done(cfg);
            }else{
                linphonec_out("default proxy already registered\n");
            }
            }else{
                linphonec_out("we do not have a default proxy\n");
                return 0;
            }
            return 1;
        }
    passwd[0]=proxy[0]=identity[0]='\0';
    sscanf(args,"%s %s %s",identity,proxy,passwd);
    if (proxy[0]=='\0' || identity[0]=='\0'){
        linphonec_out("Missing parameters, see help register\n");
        return 1;
    }
    if (passwd[0]!='\0'){
        LinphoneAddress *from;
        LinphoneAuthInfo *info;
        if ((from=linphone_address_new(identity))!=NULL){
            char realm[128];
            snprintf(realm,sizeof(realm)-1,"\"%s\"",linphone_address_get_domain(from));
            info=linphone_auth_info_new(linphone_address_get_username(from),NULL,passwd,NULL,NULL);
            linphone_core_add_auth_info(lc,info);
            linphone_address_destroy(from);
            linphone_auth_info_destroy(info);
        }
    }
    elem=linphone_core_get_proxy_config_list(lc);
    if (elem) {
        cfg=(LinphoneProxyConfig*)elem->data;
        linphone_proxy_config_edit(cfg);
    }
    else cfg=linphone_proxy_config_new();
    linphone_proxy_config_set_identity(cfg,identity);
    linphone_proxy_config_set_server_addr(cfg,proxy);
    linphone_proxy_config_enable_register(cfg,TRUE);
    if (elem) linphone_proxy_config_done(cfg);
    else linphone_core_add_proxy_config(lc,cfg);
    linphone_core_set_default_proxy(lc,cfg);
    return 1;
}

static int lpc_cmd_unregister(LinphoneCore *lc, char *args){
    LinphoneProxyConfig *cfg=NULL;
    linphone_core_get_default_proxy(lc,&cfg);
    if (cfg && linphone_proxy_config_is_registered(cfg)) {
        linphone_proxy_config_edit(cfg);
        linphone_proxy_config_enable_register(cfg,FALSE);
        linphone_proxy_config_done(cfg);
    }else{
        linphonec_out("unregistered\n");
    }
    return 1;
}

static int lpc_cmd_duration(LinphoneCore *lc, char *args){
    LinphoneCallLog *cl;
    const MSList *elem=linphone_core_get_call_logs(lc);
    for(;elem!=NULL;elem=elem->next){
        if (elem->next==NULL){
            cl=(LinphoneCallLog*)elem->data;
            linphonec_out("%i seconds\n",cl->duration);
        }
    }
    return 1;
}

static int lpc_cmd_status(LinphoneCore *lc, char *args)
{
    LinphoneProxyConfig *cfg;

    if ( ! args ) return 0;
    linphone_core_get_default_proxy(lc,&cfg);
    if (strstr(args,"register"))
    {
        if (cfg)
        {
            if (linphone_proxy_config_is_registered(cfg)){
                linphonec_out("registered, identity=%s duration=%i\n",
                    linphone_proxy_config_get_identity(cfg),
                    linphone_proxy_config_get_expires(cfg));
            }else if (linphone_proxy_config_register_enabled(cfg)){
                linphonec_out("registered=-1\n");
            }else linphonec_out("registered=0\n");
        }
        else linphonec_out("registered=0\n");
    }
    else if (strstr(args,"autoanswer"))
    {
        if (cfg && linphone_proxy_config_is_registered(cfg))
            linphonec_out("autoanswer=%i\n",linphonec_get_autoanswer());
        else linphonec_out("unregistered\n");
    }
    else if (strstr(args,"hook"))
    {
        LinphoneCall *call=linphone_core_get_current_call (lc);
        LinphoneCallState call_state=LinphoneCallIdle;
        if (call) call_state=linphone_call_get_state(call);

        switch(call_state){
            case LinphoneCallOutgoingInit:
                linphonec_out("hook=outgoing_init sip:%s\n",linphonec_get_callee());
            break;
            case LinphoneCallOutgoingProgress:
                linphonec_out("hook=dialing sip:%s\n",linphonec_get_callee());
            break;
            case LinphoneCallOutgoingRinging:
                linphonec_out("hook=ringing sip:%s\n",linphonec_get_callee());
            break;
            case LinphoneCallPaused:
                linphonec_out("hook=paused sip:%s\n",linphonec_get_callee());
            break;
            case LinphoneCallIdle:
                linphonec_out("hook=offhook\n");
            break;
            case LinphoneCallStreamsRunning:
            case LinphoneCallConnected:
                if (linphone_call_get_dir(call)==LinphoneCallOutgoing){
                    linphonec_out("Call out, hook=%s duration=%i, muted=%s rtp-xmit-muted=%s\n", linphonec_get_callee(),
                          linphone_core_get_current_call_duration(lc),
                          linphone_core_is_mic_muted (lc) ? "yes" : "no",
                          linphone_core_is_rtp_muted(lc) ? "yes"  : "no");
                }else{
                    linphonec_out("hook=answered duration=%i %s\n" ,
                        linphone_core_get_current_call_duration(lc), linphonec_get_caller());
                }
                break;
            case LinphoneCallIncomingReceived:
                linphonec_out("Incoming call from %s\n",linphonec_get_caller());
                break;
            default:
                break;
        }

    }
    else return 0;

    return 1;
}

static int lpc_cmd_ports(LinphoneCore *lc, char *args)
{
    int port;
    if ( ! args ){
        linphonec_out("sip port = %i\naudio rtp port = %i\nvideo rtp port = %i\n",
            linphone_core_get_sip_port(lc),
            linphone_core_get_audio_port(lc),
            linphone_core_get_video_port(lc));
        return 1;
    }
    if (sscanf(args,"sip %i",&port)==1){
        linphonec_out("Setting sip port to %i\n",port);
        linphone_core_set_sip_port(lc,port);
    }else return 0;

    return 1;
}

static int lpc_cmd_param(LinphoneCore *lc, char *args)
{
    char section[20], param[20], value[50];
    const char *string;

    if (args == NULL) {
        return 0;
    }
    switch (sscanf(args,"%s %s %s",section,param,value)) {
        // case 1 might show all current settings under a section
        case 2:
            string = lp_config_get_string(linphone_core_get_config(lc), section, param, "(undef)");
            linphonec_out("current value: %s\n", string);
            break;
        case 3:
            if (lp_config_get_string(linphone_core_get_config(lc), section, param, NULL) != NULL) {
                lp_config_set_string(linphone_core_get_config(lc), section, param, value);
            // no indication of existence
                linphonec_out("updated value: %s\n", value);
            } else {
                linphonec_out("only update of existing variables are allowed\n");
            }
            break;
        default:
            return 0;
    }
    return 1;
}

static int lpc_cmd_speak(LinphoneCore *lc, char *args){
#if 0 //ndef WIN32
    char voice[64];
    char *sentence;
    char cl[128];
    char *wavfile;
    int status;
    FILE *file;

    if (!args) return 0;
    memset(voice,0,sizeof(voice));
    sscanf(args,"%s63",voice);
    sentence=args+strlen(voice);
    wavfile=tempnam("/tmp/","linphonec-espeak-");
    snprintf(cl,sizeof(cl),"espeak -v %s -s 100 -w %s --stdin",voice,wavfile);
    file=popen(cl,"w");
    if (file==NULL){
        ms_error("Could not open pipe to espeak !");
        return 1;
    }
    fprintf(file,"%s",sentence);
    status=pclose(file);
    if (WEXITSTATUS(status)==0){
        linphone_core_set_play_file(lc,wavfile);
    }else{
        linphonec_out("espeak command failed.");
    }
#else
    linphonec_out("Sorry, this command is not implemented in windows version.");
#endif
    return 1;
}

static int lpc_cmd_acodec(LinphoneCore *lc, char *args){
    return lpc_cmd_codec(AUDIO, lc, args);
}

static int lpc_cmd_vcodec(LinphoneCore *lc, char *args){
    return lpc_cmd_codec(VIDEO, lc, args);
}

static int lpc_cmd_codec(int type, LinphoneCore *lc, char *args){
    char *arg1 = args;
    char *arg2 = NULL;
    char *ptr = args;

    if (!args) return 0;

    /* Isolate first and second arg */
    while(*ptr && !isspace(*ptr)) ++ptr;
    if ( *ptr )
    {
        *ptr='\0';
        arg2=ptr+1;
        while(*arg2 && isspace(*arg2)) ++arg2;
    }

    if (strcmp(arg1,"enable")==0)
    {
#ifdef HAVE_READLINE
        rl_inhibit_completion=1;
#endif
        if (!strcmp(arg2,"all")) linphonec_codec_enable(type,lc,-1);
        else linphonec_codec_enable(type,lc,atoi(arg2));
#ifdef HAVE_READLINE
        rl_inhibit_completion=0;
#endif
    }
    else if (strcmp(arg1,"list")==0)
    {
        linphonec_codec_list(type,lc);
    }
    else if (strcmp(arg1,"disable")==0)
    {
        if (!strcmp(arg2,"all")) linphonec_codec_disable(type,lc,-1);
        else linphonec_codec_disable(type,lc,atoi(arg2));
    }
    else
    {
        return 0; /* syntax error */
    }

    return 1;
}

static void linphonec_codec_list(int type, LinphoneCore *lc){
    PayloadType *pt;
    int index=0;
    const MSList *node=NULL;

    if (type == AUDIO) {
      node=linphone_core_get_audio_codecs(lc);
    } else if(type==VIDEO) {
      node=linphone_core_get_video_codecs(lc);
    }

    for(;node!=NULL;node=ms_list_next(node)){
        pt=(PayloadType*)(node->data);
        linphonec_out("%2d: %s (%d) %s\n", index, pt->mime_type, pt->clock_rate,
            linphone_core_payload_type_enabled(lc,pt) ? "enabled" : "disabled");
        index++;
    }
}

static void linphonec_codec_enable(int type, LinphoneCore *lc, int sel_index){
    PayloadType *pt;
    int index=0;
    const MSList *node=NULL;

    if (type == AUDIO) {
        node=linphone_core_get_audio_codecs(lc);
    } else if(type==VIDEO) {
        node=linphone_core_get_video_codecs(lc);
    }

    for(;node!=NULL;node=ms_list_next(node)){
        if (index == sel_index || sel_index == -1) {
            pt=(PayloadType*)(node->data);
            linphone_core_enable_payload_type (lc,pt,TRUE);
            linphonec_out("%2d: %s (%d) %s\n", index, pt->mime_type, pt->clock_rate, "enabled");
        }
        index++;
    }
}

static void linphonec_codec_disable(int type, LinphoneCore *lc, int sel_index){
    PayloadType *pt;
    int index=0;
    const MSList *node=NULL;

    if (type == AUDIO) {
        node=linphone_core_get_audio_codecs(lc);
    } else if(type==VIDEO) {
        node=linphone_core_get_video_codecs(lc);
    }

    for(;node!=NULL;node=ms_list_next(node)){
        if (index == sel_index || sel_index == -1) {
            pt=(PayloadType*)(node->data);
            linphone_core_enable_payload_type (lc,pt,FALSE);
            linphonec_out("%2d: %s (%d) %s\n", index, pt->mime_type, pt->clock_rate, "disabled");
        }
        index++;
    }
}

static int lpc_cmd_echocancellation(LinphoneCore *lc, char *args){
    char *arg1 = args;
    char *arg2 = NULL;
    char *ptr = args;
    LpConfig *config=linphone_core_get_config(lc);

    if (!args) return 0;

    /* Isolate first and second arg */
    while(*ptr && !isspace(*ptr)) ++ptr;
    if ( *ptr )
    {
        *ptr='\0';
        arg2=ptr+1;
        while(*arg2 && isspace(*arg2)) ++arg2;
    }

    if (strcmp(arg1,"on")==0){
        int delay, tail_len, frame_size;
        int n;

        linphone_core_enable_echo_cancellation(lc,1);

        if (arg2 != 0) {
            n = sscanf(arg2, "%d %d %d", &delay, &tail_len, &frame_size);

            if (n == 1) {
                lp_config_set_int(config,SOUND,"ec_delay",delay);
            }
            else if (n == 2) {
                lp_config_set_int(config,SOUND,"ec_delay",delay);
                lp_config_set_int(config,SOUND,"ec_tail_len",tail_len);
            }
            else if (n == 3) {
                lp_config_set_int(config,SOUND,"ec_delay",delay);
                lp_config_set_int(config,SOUND,"ec_tail_len",tail_len);
                lp_config_set_int(config,SOUND,"ec_framesize",frame_size);
            }
        }
    }
    else if (strcmp(arg1,"off")==0){
        linphone_core_enable_echo_cancellation(lc,0);
    }
    else if (strcmp(arg1,"show")==0){
        linphonec_out("echo cancellation is %s; delay %d, tail length %d, frame size %d\n",
            linphone_core_echo_cancellation_enabled(lc) ? "on" : "off",
            lp_config_get_int(config,SOUND,"ec_delay",0),
            lp_config_get_int(config,SOUND,"ec_tail_len",0),
            lp_config_get_int(config,SOUND,"ec_framesize",0));
    }
    else {
        return 0;
    }

    return 1;
}

static int lpc_cmd_echolimiter(LinphoneCore *lc, char *args){
    if (args){
        if (strcmp(args,"on")==0){
            linphone_core_enable_echo_limiter (lc,TRUE);
        }else if (strcmp(args,"off")==0){
            linphone_core_enable_echo_limiter (lc,FALSE);
        }
    }
    linphonec_out("Echo limiter is now %s.\n",linphone_core_echo_limiter_enabled (lc) ? "on":"off");
    return 1;
}

static int lpc_cmd_mute_mic(LinphoneCore *lc, char *args)
{
    linphone_core_mute_mic(lc, 1);
    return 1;
}

static int lpc_cmd_unmute_mic(LinphoneCore *lc, char *args){
    linphone_core_mute_mic(lc, 0);
    return 1;
}

static int lpc_cmd_playback_gain(LinphoneCore *lc, char *args)
{
    if (args){
            linphone_core_set_playback_gain_db(lc, atof(args));
            return 1;
    }
    return 0;
}

static int lpc_cmd_rtp_no_xmit_on_audio_mute(LinphoneCore *lc, char *args)
{
    bool_t rtp_xmit_off=FALSE;
    char *status;

    if(args){
        if(strstr(args,"1"))rtp_xmit_off=TRUE;
        if(linphone_core_get_current_call (lc)==NULL)
            linphone_core_set_rtp_no_xmit_on_audio_mute(lc,rtp_xmit_off);
        else
            linphonec_out("nortp-on-audio-mute: call in progress - cannot change state\n");
    }
    rtp_xmit_off=linphone_core_get_rtp_no_xmit_on_audio_mute(lc);
    if (rtp_xmit_off) status="off";
    else status="on";
    linphonec_out("rtp transmit %s when audio muted\n",status);
    return 1;
}

#ifdef VIDEO_ENABLED
static int _lpc_cmd_video_window(LinphoneCore *lc, char *args, bool_t is_preview){
    char subcommand[64];
    int a,b;
    int err;
    VideoParams *params=is_preview ? &lpc_preview_params : &lpc_video_params;

    if (!args) return 0;
    err=sscanf(args,"%s %i %i",subcommand,&a,&b);
    if (err>=1){
        if (strcmp(subcommand,"pos")==0){
            if (err<3) return 0;
            params->x=a;
            params->y=b;
            params->refresh=TRUE;
        }else if (strcmp(subcommand,"size")==0){
            if (err<3) return 0;
            params->w=a;
            params->h=b;
            params->refresh=TRUE;
        }else if (strcmp(subcommand,"show")==0){
            params->show=TRUE;
            params->refresh=TRUE;
            if (is_preview) linphone_core_enable_video_preview (lc,TRUE);
        }else if (strcmp(subcommand,"hide")==0){
            params->show=FALSE;
            params->refresh=TRUE;
            if (is_preview) linphone_core_enable_video_preview (lc,FALSE);
        }else if (strcmp(subcommand,"id")==0){
            if (err == 1){
                linphonec_out("vwindow id: 0x%x\n",is_preview ? linphone_core_get_native_preview_window_id (lc) :
                              linphone_core_get_native_video_window_id (lc));
                return 1;
            } else if (err != 2) return 0;
            params->wid=a;
            if (is_preview)
                linphone_core_set_native_preview_window_id (lc,a);
            else
                linphone_core_set_native_video_window_id(lc,a);
        }else if (is_preview==TRUE){
            if (strcmp(subcommand,"integrated")==0){
                linphone_core_use_preview_window (lc,FALSE);
            }else if (strcmp(subcommand,"standalone")==0){
                linphone_core_use_preview_window(lc,TRUE);
            }else return 0;
        }else return 0;
    }
    return 1;
}

static int lpc_cmd_video_window(LinphoneCore *lc, char *args){
    return _lpc_cmd_video_window(lc, args, FALSE);
}

static int lpc_cmd_preview_window(LinphoneCore *lc, char *args){
    return _lpc_cmd_video_window(lc, args, TRUE);
}
#endif

static void lpc_display_global_state(LinphoneCore *lc){
    linphonec_out("Global liblinphone state\n%s\n",
                  linphone_global_state_to_string(linphone_core_get_global_state(lc)));
}

static void lpc_display_call_states(LinphoneCore *lc){
    LinphoneCall *call;
    const MSList *elem;
    char *tmp;
    linphonec_out("Call states\n"
                  "Id |            Destination              |      State      |    Flags   |\n"
                  "------------------------------------------------------------------------\n");
    elem=linphone_core_get_calls(lc);
    if (elem==NULL){
        linphonec_out("(empty)\n");
    }else{
        for(;elem!=NULL;elem=elem->next){
            const char *flag;
            bool_t in_conference;
            call=(LinphoneCall*)elem->data;
            in_conference=linphone_call_params_local_conference_mode(linphone_call_get_current_params(call));
            tmp=linphone_call_get_remote_address_as_string (call);
            flag=in_conference ? "conferencing" : "";
            flag=linphone_call_has_transfer_pending(call) ? "transfer pending" : flag;
            linphonec_out("%-2i | %-35s | %-15s | %s\n",(int)(long)linphone_call_get_user_pointer(call),
                          tmp,linphone_call_state_to_string(linphone_call_get_state(call))+strlen("LinphoneCall"),flag);
            ms_free(tmp);
        }
    }
}

static void lpc_display_proxy_states(LinphoneCore *lc){
    const MSList *elem;
    linphonec_out("Proxy registration states\n"
                  "           Identity                      |      State\n"
                  "------------------------------------------------------------\n");
    elem=linphone_core_get_proxy_config_list (lc);
    if (elem==NULL) linphonec_out("(empty)\n");
    else {
        for(;elem!=NULL;elem=elem->next){
            LinphoneProxyConfig *cfg=(LinphoneProxyConfig*)elem->data;
            linphonec_out("%-40s | %s\n",linphone_proxy_config_get_identity (cfg),
                          linphone_registration_state_to_string(linphone_proxy_config_get_state(cfg)));
        }
    }
}

static int lpc_cmd_states(LinphoneCore *lc, char *args){
    if (args==NULL) {
        lpc_display_global_state(lc);
        lpc_display_call_states(lc);
        lpc_display_proxy_states(lc);
        return 1;
    }
    if (strcmp(args,"global")==0){
        lpc_display_global_state(lc);
        return 1;
    }
    if (strcmp(args,"proxies")==0){
        lpc_display_proxy_states(lc);
        return 1;
    }
    if (strcmp(args,"calls")==0){
        lpc_display_call_states(lc);
        return 1;
    }
    return 0;
}

#ifdef VIDEO_ENABLED
static int lpc_cmd_camera(LinphoneCore *lc, char *args){
    LinphoneCall *call=linphone_core_get_current_call(lc);
    bool_t activated=FALSE;

    if (linphone_core_video_enabled (lc)==FALSE){
        linphonec_out("Video is disabled, re-run linphonec with -V option.");
        return 1;
    }

    if (args){
        if (strcmp(args,"on")==0)
            activated=TRUE;
        else if (strcmp(args,"off")==0)
            activated=FALSE;
        else
            return 0;
    }

    if (call==NULL){
        if (args){
            linphonec_camera_enabled=activated;
        }
        if (linphonec_camera_enabled){
            linphonec_out("Camera is enabled. Video stream will be setup immediately for outgoing and incoming calls.\n");
        }else{
            linphonec_out("Camera is disabled. Calls will be established with audio-only, with the possibility to later add video using 'camera on'.\n");
        }
    }else{
        const LinphoneCallParams *cp=linphone_call_get_current_params (call);
        if (args){
            linphone_call_enable_camera(call,activated);
            if ((activated && !linphone_call_params_video_enabled (cp))){
                /*update the call to add the video stream*/
                LinphoneCallParams *ncp=linphone_call_params_copy(cp);
                linphone_call_params_enable_video(ncp,TRUE);
                linphone_core_update_call(lc,call,ncp);
                linphone_call_params_destroy (ncp);
                linphonec_out("Trying to bring up video stream...\n");
            }
        }
        if (linphone_call_camera_enabled (call))
                linphonec_out("Camera is allowed for current call.\n");
        else linphonec_out("Camera is dis-allowed for current call.\n");
    }
    return 1;
}

static int lpc_cmd_snapshot(LinphoneCore *lc, char *args){
    LinphoneCall *call;
    if (!args) return 0;
    call=linphone_core_get_current_call(lc);
    if (call!=NULL){
        linphone_call_take_video_snapshot(call,args);
        linphonec_out("Taking video snapshot in file %s\n", args);
    }else linphonec_out("There is no active call.\n");
    return 1;
}

static int lpc_cmd_vfureq(LinphoneCore *lc, char *arg){
    LinphoneCall *call;
    call=linphone_core_get_current_call(lc);
    if (call!=NULL){
        linphone_call_send_vfu_request(call);
        linphonec_out("VFU request sent\n");
    }else linphonec_out("There is no active call.\n");
    return 1;
}
#endif

static int lpc_cmd_identify(LinphoneCore *lc, char *args){
    LinphoneCall *call;
    const char *remote_ua;
    if (args==NULL){
        call=linphone_core_get_current_call(lc);
        if (call==NULL) {
            linphonec_out("There is currently running call. Specify call id.\n");
            return 0;
        }
    }else{
        call=linphonec_get_call(atoi(args));
        if (call==NULL){
            return 0;
        }
    }
    remote_ua=linphone_call_get_remote_user_agent(call);
    if (remote_ua){
        linphonec_out("Remote user agent string is: %s\n",remote_ua);
    }
    return 1;
}

static int lpc_cmd_ringback(LinphoneCore *lc, char *args){
    if (!args) return 0;
    if (strcmp(args,"disable")==0){
        linphone_core_set_remote_ringback_tone(lc,NULL);
        linphonec_out("Disabling ringback tone.\n");
        return 1;
    }
    linphone_core_set_remote_ringback_tone (lc,args);
    linphonec_out("Using %s as ringback tone to be played to callers.",args);
    return 1;
}

static int zrtp_set_verified(LinphoneCore *lc, char *args, bool_t verified){
    LinphoneCall *call=linphone_core_get_current_call(lc);
    if (linphone_call_params_get_media_encryption(linphone_call_get_current_params(call))==LinphoneMediaEncryptionZRTP){
        linphone_call_set_authentication_token_verified(call,verified);
    }
    return 1;
}
static int lpc_cmd_zrtp_verified(LinphoneCore *lc, char *args){
    return zrtp_set_verified(lc,args,TRUE);
}
static int lpc_cmd_zrtp_unverified(LinphoneCore *lc, char *args){
    return zrtp_set_verified(lc,args,FALSE);
}

static int lpc_cmd_castor3_set_ring(LinphoneCore *lc, char *args)
{
#if CFG_MP3_RING
    linphonec_out("Enable mp3 ring , not set ring \n");
    return 0;
#endif
    if (!args) return 0;
    linphone_core_set_ring(lc,args);
    linphonec_out("Using %s as ring.",args);
    return 1;
}

static int lpc_cmd_castor3_play_ring(LinphoneCore *lc, char *args)
{
    MSSndCard *ringcard=lc->sound_conf.lsd_card ?lc->sound_conf.lsd_card : lc->sound_conf.ring_sndcard;
#if (CFG_CHIP_FAMILY == 9910)
    if (!args) return 1;
#else
    if (!args || linphone_core_get_current_call(lc)) return 1;
#endif
    if (lc->ringstream!=NULL){
        ring_stop(lc->ringstream);
        lc->ringstream=NULL;
        lc->dmfs_playing_start_time=0;
    }
    if (ringcard) {
        ms_snd_card_set_level(ringcard,MS_SND_CARD_PLAYBACK,linphoneCastor3.ring_level);
        lc->ringstream=ring_start(args,-1,ringcard);
        lc->dmfs_playing_start_time=time(NULL)+1;
    }
    return 1;
}

static int lpc_cmd_castor3_set_play_level(LinphoneCore *lc, char *args)
{
    LpConfig *config=linphone_core_get_config(lc);
    linphone_core_set_play_level(lc, linphoneCastor3.play_level);
    lp_config_set_int(config,SOUND,"play_lev",linphoneCastor3.play_level);
    return 1;
}

static int lpc_cmd_castor3_set_rec_level(LinphoneCore *lc, char *args)
{
    LpConfig *config=linphone_core_get_config(lc);
    linphone_core_set_rec_level(lc, linphoneCastor3.rec_level);
    lp_config_set_int(config,SOUND,"rec_lev",linphoneCastor3.rec_level);
    return 1;
}

static int lpc_cmd_castor3_set_ring_level(LinphoneCore *lc, char *args)
{
    LpConfig *config=linphone_core_get_config(lc);

    if (args && strcmp(args,"active")==0){
        linphone_core_set_ring_level(lc, linphoneCastor3.ring_level);
    } else {
        lc->sound_conf.ring_lev=linphoneCastor3.ring_level;
    }
    lp_config_set_int(config,SOUND,"ring_lev",linphoneCastor3.ring_level);
    return 1;
}

static int lpc_cmd_castor3_play_voicesound(LinphoneCore *lc, char *args)
{
    MSSndCard *ringcard=lc->sound_conf.lsd_card ?lc->sound_conf.lsd_card : lc->sound_conf.ring_sndcard;
    int n=0;
    int duration=0;
    char file_path[256];
    LinphoneCall* call=linphone_core_get_current_call(lc);
    if (!args || (call && call->audiostream->soundwrite)){
        AudioStream *st;
        if (call && (st=call->audiostream)){
            n=sscanf(args, "%s %li", file_path, &duration);
            voice_mix_flag (st,file_path);
        }
        return 1;
    }
    n=sscanf(args, "%s %li", file_path, &duration);
    if (lc->ringstream!=NULL){
        ring_stop(lc->ringstream);
        lc->ringstream=NULL;
        lc->dmfs_playing_start_time=0;
    }
    //printf("f:%s, n:%d, t:%d\n", file_path, n, duration);
    if (ringcard) {
        ms_snd_card_set_level(ringcard,MS_SND_CARD_PLAYBACK,linphoneCastor3.key_level);
        lc->ringstream=ring_start(file_path,-1,ringcard);
        lc->dmfs_playing_start_time=time(NULL)+duration;
        lc->ringstream_autorelease=TRUE;
    }
    return 1;
}

static int lpc_cmd_castor3_play_keysound(LinphoneCore *lc, char *args)
{
    MSSndCard *ringcard=lc->sound_conf.lsd_card ?lc->sound_conf.lsd_card : lc->sound_conf.ring_sndcard;
    if (!args || linphone_core_get_current_call(lc) || lc->voice_memo_play_stream) return 1;

    if (lc->isVideoMemoPlaying) return 1;
    if (lc->isADVPlaying)
    {
        lc->isADVPlaying = FALSE;
#ifdef CFG_RTSP_CLIENT_ENABLE
        stopRTSPClient();
#endif
        castor3snd_reinit_for_video_memo_play();
    }

    if (lc->ringstream!=NULL){
        ring_stop(lc->ringstream);
        lc->ringstream=NULL;
        lc->dmfs_playing_start_time=0;
    }
    if (ringcard) {
        int duration = 1;
        ms_snd_card_set_level(ringcard,MS_SND_CARD_PLAYBACK,linphoneCastor3.key_level);
        lc->ringstream=ring_start(args,-1,ringcard);
        duration += get_wav_time(lc->ringstream);
        lc->dmfs_playing_start_time=time(NULL)+duration;
        lc->ringstream_autorelease=TRUE;
    }
    return 1;
}

static int lpc_cmd_castor3_play_warnsound(LinphoneCore *lc, char *args)
{
    MSSndCard *ringcard=lc->sound_conf.lsd_card ?lc->sound_conf.lsd_card : lc->sound_conf.ring_sndcard;
    if (!args || linphone_core_get_current_call(lc)) return 1;
    if(lc->isMediaVideoPlaying)  
    {
        linphone_core_stop_media_video_play(lc);
    }        
    
    if(lc->isVideoMemoPlaying)
    {
        linphone_core_stop_video_memo_play(lc);
    }
    if (lc->voice_memo_record_stream!=NULL){
        voice_memo_stop_record(lc->voice_memo_record_stream);
        lc->voice_memo_record_stream=NULL;
    }
    
    if (lc->voice_memo_play_stream!=NULL){
        voice_memo_stop_play(lc->voice_memo_play_stream);
        lc->voice_memo_play_stream=NULL;
    }    

    if (lc->ringstream!=NULL){
        ring_stop(lc->ringstream);
        lc->ringstream=NULL;
        lc->dmfs_playing_start_time=0;
    }
    if (ringcard) {
        ms_snd_card_set_level(ringcard,MS_SND_CARD_PLAYBACK,linphoneCastor3.warn_level);
        lc->ringstream=ring_start(args,100,ringcard);
        lc->dmfs_playing_start_time=0;
    }
    return 1;
}

int linphone_friend_set_name(LinphoneFriend *lf, const char *name);

static int lpc_cmd_castor3_add_friend(LinphoneCore *lc, char *args)
{
    char *arg1 = args;
    char *ptr = args;
    LinphoneFriend *new_friend;

    if (!args) return 0;

    /* Isolate first and second arg */
    while(*ptr && !isspace(*ptr)) ++ptr;
    if ( *ptr )
    {
        *ptr='\0';
    }

    new_friend = linphone_core_get_friend_by_ref_key(lc, linphoneCastor3.friend_addr);
    if (new_friend)
    {
        if (*arg1 == '1')
            linphone_friend_set_inc_subscribe_policy(new_friend, LinphoneSPDeny);
        else
            linphone_friend_set_inc_subscribe_policy(new_friend, LinphoneSPAccept);

        linphone_friend_set_name(new_friend, linphoneCastor3.friend_comment);
    }
    else
    {
        new_friend = linphone_friend_new_with_addr(linphoneCastor3.friend_sip);
        if (!new_friend)
            return 0;

        if (*arg1 == '1')
            linphone_friend_set_inc_subscribe_policy(new_friend, LinphoneSPDeny);

        linphone_friend_set_name(new_friend, linphoneCastor3.friend_comment);
        linphone_friend_set_ref_key(new_friend, linphoneCastor3.friend_addr);
        linphone_core_add_friend(lc, new_friend);
    }
    lpc_cmd_castor3_get_friends(lc, NULL);
    return 1;
}

int lpc_cmd_castor3_get_friends(LinphoneCore *lc, char *args)
{
    MSList *elem;
    int i, count;

    count = linphoneCastor3.friend_count;
    if (count > 0)
    {
        linphoneCastor3.friend_count = 0;

        for (i = 0; i < count; i++)
        {
            LinphoneCastor3Friend* fr = &linphoneCastor3.friends[i];
            free(fr->uri);
        }
        free(linphoneCastor3.friends);
        linphoneCastor3.friends = NULL;
    }

    count = ms_list_size(elem=lc->friends);
    if (count == 0)
        return 1;

    linphoneCastor3.friends = calloc(count, sizeof (LinphoneCastor3Friend));
    if (!linphoneCastor3.friends)
        return 1;

    for (elem=lc->friends,i=0; elem!=NULL; elem=ms_list_next(elem),i++)
    {
        LinphoneFriend* lf = (LinphoneFriend*)elem->data;
        LinphoneCastor3Friend* fr = &linphoneCastor3.friends[i];
        const LinphoneAddress* la = linphone_friend_get_address(lf);

        fr->refkey = (char*)linphone_friend_get_ref_key(lf);
        fr->uri = linphone_address_as_string_uri_only(la);
        fr->comment = (char*)linphone_address_get_display_name(la);

        if (linphone_friend_get_inc_subscribe_policy(lf) == LinphoneSPDeny)
            fr->blacklist = 1;
        else
            fr->blacklist = 0;
    }
    linphoneCastor3.friend_count = count;
    return 1;
}

static int lpc_cmd_castor3_update_friend(LinphoneCore *lc, char *args)
{
    char *arg1 = args;
    char *arg2 = NULL;
    char *ptr = args;
    LinphoneFriend *lf;

    if (!args) return 0;

    /* Isolate first and second arg */
    while(*ptr && !isspace(*ptr)) ++ptr;
    if ( *ptr )
    {
        *ptr='\0';
        arg2=ptr+1;
        while(*arg2 && isspace(*arg2)) ++arg2;
    }
    else
    {
        /* missing one parameter */
        return 0;
    }

    lf = linphone_core_get_friend_by_ref_key(lc, arg1);
    if (lf)
    {
        if (*arg2 == '1')
            linphone_friend_set_inc_subscribe_policy(lf, LinphoneSPDeny);
        else
            linphone_friend_set_inc_subscribe_policy(lf, LinphoneSPAccept);

        linphone_friend_set_name(lf, linphoneCastor3.friend_comment);

        lpc_cmd_castor3_get_friends(lc, NULL);
    }
    return 1;
}

static int lpc_cmd_castor3_delete_friend(LinphoneCore *lc, char *args)
{
    LinphoneFriend *lf;

    if (!args) return 0;

    lf = linphone_core_get_friend_by_ref_key(lc, args);
    if (lf)
    {
        linphone_core_remove_friend(lc, lf);
        lpc_cmd_castor3_get_friends(lc, NULL);
    }
    return 1;
}

static int lpc_cmd_castor3_clear_friends(LinphoneCore *lc, char *args)
{
    ms_list_for_each(lc->friends,(void (*)(void*))linphone_friend_destroy);
    lc->friends=ms_list_free(lc->friends);
    linphone_core_write_friends_config(lc);
    lpc_cmd_castor3_get_friends(lc, NULL);
    return 1;
}

static int lpc_cmd_castor3_clear_call_logs_missed(LinphoneCore *lc, char *args)
{
    MSList *elem;

    for (elem=lc->call_logs; elem!=NULL;)
    {
        MSList* next_elem = ms_list_next(elem);
        LinphoneCallLog* cl = (LinphoneCallLog*)elem->data;
        if (cl->dir == LinphoneCallIncoming && (cl->status == LinphoneCallMissed || cl->status == LinphoneCallAborted))
        {
            linphone_call_log_set_ref_key(cl, "D");
        }
        elem = next_elem;
    }
    call_logs_write_to_config_file(lc);
    return 1;
}

static int lpc_cmd_castor3_clear_call_logs_received(LinphoneCore *lc, char *args)
{
    MSList *elem;

    for (elem=lc->call_logs; elem!=NULL;)
    {
        MSList* next_elem = ms_list_next(elem);
        LinphoneCallLog* cl = (LinphoneCallLog*)elem->data;
        if (cl->dir == LinphoneCallIncoming && cl->status == LinphoneCallSuccess)
        {
            linphone_call_log_set_ref_key(cl, "D");
        }
        elem = next_elem;
    }
    call_logs_write_to_config_file(lc);
    return 1;
}

static int lpc_cmd_castor3_clear_call_logs_sent(LinphoneCore *lc, char *args)
{
    MSList *elem;

    for (elem=lc->call_logs; elem!=NULL;)
    {
        MSList* next_elem = ms_list_next(elem);
        LinphoneCallLog* cl = (LinphoneCallLog*)elem->data;
        if (cl->dir == LinphoneCallOutgoing)
        {
            linphone_call_log_set_ref_key(cl, "D");
        }
        elem = next_elem;
    }
    call_logs_write_to_config_file(lc);
    return 1;
}

static int lpc_cmd_castor3_delete_call_log(LinphoneCore *lc, char *args)
{
    MSList *elem;

    if (!args) return 0;

    for(elem=lc->call_logs;elem!=NULL;elem=elem->next){
        LinphoneCallLog* cl = (LinphoneCallLog*)elem->data;
        if (cl->refkey!=NULL && strcmp(cl->refkey,args)==0){

            linphone_call_log_set_ref_key(cl, "D");
            call_logs_write_to_config_file(lc);
            return 1;
        }
    }
    return 1;
}

static int lpc_cmd_castor3_play_video_msg(LinphoneCore *lc, char *args)
{
    LinphoneProxyConfig *cfg=linphone_proxy_config_new();
    if (cfg)
    {
        const char* addr = linphone_core_get_primary_contact(lc);

        if (linphone_proxy_config_set_server_addr(cfg, addr) == 0)
        {
            linphone_proxy_config_set_identity(cfg, linphoneCastor3.video_msg_uri);
            if (linphone_proxy_config_get_identity (cfg))
            {
                linphone_core_add_proxy_config(lc,cfg);
                linphone_core_set_default_proxy(lc,cfg);
                lpc_cmd_call(lc, args);
                linphonec_proxy_remove(lc, 0);
            }
            else
                linphone_proxy_config_destroy(cfg);
        }
        else
            linphone_proxy_config_destroy(cfg);
    }
    return 1;
}

static int lpc_cmd_castor3_call(LinphoneCore *lc, char *args)
{
    int i;
    bool_t audio_only = FALSE, early_media = FALSE, call_mobile = FALSE, video_from_ipcam = FALSE, other_only_ring = FALSE;
    char *opt1,*opt2, *opt3, *opt4, *opt5, buf[64];
    opt1=strstr(args,"--audio-only");
    opt2=strstr(args,"--early-media");
	opt3=strstr(args,"--call-mobile");
    opt4=strstr(args,"--video-from-ipcam");
    opt5=strstr(args,"--other-only-ring");

    if ( linphone_core_in_call(lc) )
    {
        linphonec_out("Terminate or hold on the current call first.\n");
        return 1;
    }

    if (opt1){
        opt1[0]='\0';
        audio_only = TRUE;
    }
    if (opt2){
        opt2[0]='\0';
        early_media = TRUE;
    }
	if (opt3){
        opt3[0]='\0';
        call_mobile = TRUE;
    }
    if (opt4){
        opt4[0]='\0';
        video_from_ipcam = TRUE;
    }
    if (opt5){
        opt5[0]='\0';
        other_only_ring = TRUE;
    }
    for (i = 0; i < linphoneCastor3.calling_ip_count; i++)
    {
        LinphoneCall *call;
        LinphoneCallParams *cp=linphone_core_create_default_call_parameters (lc);

        if(other_only_ring && i >= MAX_VIDEO_USER_NUM)
        {
            linphone_call_params_enable_call_only_ring(cp,TRUE);
        }
        if (audio_only){
            linphone_call_params_enable_video (cp,FALSE);
        }
        if (early_media){
            linphone_call_params_enable_early_media_sending(cp,TRUE);
        }
        if (video_from_ipcam){
            linphone_call_params_enable_video_from_ipcam(cp, TRUE);
        }
#if 0 //def CFG_SIP_OUTDOOR_TEST
		if (i == 0){
			strcpy(buf, "sip:");
			strcat(buf, CFG_SIP_CALLOUT);
			strcat(buf, "@");
			strcat(buf, CFG_REGISTER_DOMAIN);
		}
#else
        if (i == 0)
            strcpy(buf, "sip:toto@");
        else
            strcpy(buf, "sip:ext@");

        strcat(buf, linphoneCastor3.calling_ip_array[i]);
#endif

        if ( NULL == (call=linphone_core_invite_with_params(lc, buf,cp)) )
        {
            linphonec_out("Error from linphone_core_invite.\n");
        }
        else
        {
            snprintf(callee_name,sizeof(callee_name),"%s",buf);
        }
        linphone_call_params_destroy(cp);
    }
    if (linphoneCastor3.sip_server_uri[0] != '\0')
    {
        LinphoneCall *call;
        LinphoneCallParams *cp=linphone_core_create_default_call_parameters (lc);

        linphone_call_params_enable_video (cp,TRUE);
        linphone_call_params_enable_early_media_sending(cp,FALSE);

        if ( NULL == (call=linphone_core_invite_with_params(lc, linphoneCastor3.sip_server_uri,cp)) )
        {
            linphonec_out("Error from linphone_core_invite.\n");
        }
        else
        {
            snprintf(callee_name,sizeof(callee_name),"%s",linphoneCastor3.sip_server_uri);
        }
        linphone_call_params_destroy(cp);
        linphoneCastor3.calling_ip_count++;
    }
    for (i = 0; i < linphoneCastor3.calling_mobile_count; i++)
    {
        LinphoneCall *call;
        LinphoneCallParams *cp=linphone_core_create_default_call_parameters (lc);

        if (audio_only){
            linphone_call_params_enable_video (cp,FALSE);
        }
        linphone_call_params_enable_early_media_sending(cp,TRUE);

		if(call_mobile)
        	linphone_call_params_enable_call_mobile(cp,TRUE);

#if defined(CFG_SIP_LOBBY_TEST) || defined(CFG_SIP_OUTDOOR_TEST)
        strcpy(buf, "sip:");
        strcat(buf, CFG_SIP_CALLOUT);
        strcat(buf, "@");
        strcat(buf, CFG_REGISTER_DOMAIN);

#else
        strcpy(buf, "sip:mobile@");
        strcat(buf, linphoneCastor3.calling_mobile_array[i]);
#endif

#ifdef CFG_P2P
        strcpy(buf, "sip:");
        strcat(buf, CFG_P2P_CALLOUT);
        strcat(buf, "@");
        strcat(buf, CFG_P2P_DOMAIN);
#endif

#ifdef CFG_SIP_PROXD_TEST
		// sip proxy testing, add 5066 port to second calling......
        strcat(buf, ":5066"); //eason, call to register_name@indoor_ip:5066
        printf("[lpc_cmd_castor3_call] Call to %s\n", buf);
#endif

        if ( NULL == (call=linphone_core_invite_with_params(lc, buf,cp)) )
        {
            linphonec_out("Error from linphone_core_invite.\n");
        }
        else
        {
            snprintf(callee_name,sizeof(callee_name),"%s",buf);
        }
        linphone_call_params_destroy(cp);
        linphoneCastor3.calling_ip_count++;
    }
    return 1;
}

static int lpc_cmd_castor3_terminate_others(LinphoneCore *lc, char *args)
{
	uint32_t start_cnt = 0;
    long id=atoi(args);
    const MSList *sip_calls=linphone_core_get_calls(lc);
	const MSList *calls=linphone_core_get_calls(lc);

    if (sip_calls==NULL || calls==NULL){
        linphonec_out("No active calls\n");
        return 1;
    }
	
	start_cnt = itpGetTickCount();
	while(sip_calls) {
        LinphoneCall *c=(LinphoneCall*)sip_calls->data;
        sip_calls=sip_calls->next;
        if (linphone_call_get_user_pointer (c)!=(void*)id){
            linphone_core_terminate_call_sip_only(lc,c);
        }
    }
    printf("[debug]terminate sip time = %d ms\n", itpGetTickDuration(start_cnt));
	
	start_cnt = itpGetTickCount();

    while(calls) {
        LinphoneCall *c=(LinphoneCall*)calls->data;
        calls=calls->next;
        if (linphone_call_get_user_pointer (c)!=(void*)id){
            linphone_core_terminate_call(lc,c);
        }
    }
	printf("[debug]terminate media time = %d ms\n", itpGetTickDuration(start_cnt));
    return 1;
}

static int lpc_cmd_castor3_stop_sound(LinphoneCore *lc, char *args)
{
    MSSndCard *ringcard=lc->sound_conf.lsd_card ?lc->sound_conf.lsd_card : lc->sound_conf.ring_sndcard;
    if (linphone_core_get_current_call(lc)) return 1;
    if (lc->ringstream!=NULL){
        ring_stop(lc->ringstream);
        lc->ringstream=NULL;
        lc->dmfs_playing_start_time=0;
    }
    return 1;
}

static int lpc_cmd_castor3_record_video_msg(LinphoneCore *lc, char *args)
{
    LinphoneProxyConfig *cfg=linphone_proxy_config_new();
    if (cfg)
    {
        const char* addr = linphone_core_get_primary_contact(lc);

        if (linphone_proxy_config_set_server_addr(cfg, addr) == 0)
        {
            linphone_proxy_config_set_identity(cfg, linphoneCastor3.video_msg_uri);
            if (linphone_proxy_config_get_identity (cfg))
            {
                linphone_core_add_proxy_config(lc,cfg);
                linphone_core_set_default_proxy(lc,cfg);
                lpc_cmd_call(lc, args);
                linphonec_proxy_remove(lc, 0);
            }
            else
                linphone_proxy_config_destroy(cfg);
        }
        else
            linphone_proxy_config_destroy(cfg);
    }
    return 1;
}

static int lpc_cmd_castor3_set_do_not_disturb(LinphoneCore *lc, char *args)
{
    if (!args) return 0;
    if (strcmp(args,"on")==0){
        linphone_core_set_presence_info(lc, 0, NULL, LinphoneStatusDoNotDisturb);
        lc->b_DoNotDisturb = TRUE;
    } else if (strcmp(args,"off")==0){
        linphone_core_set_presence_info(lc, 0, NULL, LinphoneStatusOnline);
        lc->b_DoNotDisturb = FALSE;
    }
    return 1;
}

static int lpc_cmd_castor3_set_presence(LinphoneCore *lc, char *args)
{
    if (!args) return 0;
    if (strcmp(args,"online")==0){
        linphone_core_set_presence_info(lc, 0, NULL, LinphoneStatusOnline);
    } else if (strcmp(args,"busy")==0){
        linphone_core_set_presence_info(lc, 0, NULL, LinphoneStatusBusy);
    }
    return 1;
}

static int lpc_cmd_castor3_watch_camera(LinphoneCore *lc, char *args)
{
    if ( ! args || ! *args )
    {
        return 0;
    }
    {
        LinphoneCall *call;
        LinphoneCallParams *cp;
        int id;
        if ( linphone_core_in_call(lc) )
        {
            linphonec_out("Terminate or hold on the current call first.\n");
            return 1;
        }
        cp=linphone_core_create_default_call_parameters (lc);
        linphone_call_params_enable_early_media_sending(cp,TRUE);
        if ( NULL == (call=linphone_core_invite_with_params(lc, args,cp)) )
        {
            linphonec_out("Error from linphone_core_invite.\n");
            return 0;
        }
        else
        {
            snprintf(callee_name,sizeof(callee_name),"%s",args);
        }
        linphone_call_params_destroy(cp);
        id=(long)linphone_call_get_user_pointer (call);
        linphoneCastor3.camera_id = id;
        	lc->presence_mode = LinphoneStatusWatchingCamera;
    }
    return 1;
}

static int lpc_cmd_castor3_stop_camera(LinphoneCore *lc, char *args)
{
    if (linphone_core_get_calls(lc)==NULL){
        linphonec_out("No active camera\n");
        return 1;
    }

    if(linphoneCastor3.camera_id==0){
        return 1;
    }else{
        LinphoneCall *call=linphonec_get_call(linphoneCastor3.camera_id);
        linphoneCastor3.camera_id = 0;
        if (call){
            if (linphone_core_terminate_call(lc,call)==-1){
                linphonec_out("Could not stop camera id %li\n",linphoneCastor3.camera_id);
            }
        }else return 0;
        return 1;
    }
    return 0;
}

static int lpc_cmd_castor3_start_voice_memo_record(LinphoneCore *lc, char *args)
{
    MSSndCard *sndcard=lc->sound_conf.lsd_card ?lc->sound_conf.lsd_card : lc->sound_conf.ring_sndcard;
    if (!args) return 1;
    if (lc->voice_memo_record_stream!=NULL){
        voice_memo_stop_record(lc->voice_memo_record_stream);
        lc->voice_memo_record_stream=NULL;
    }
    if (sndcard) {
        lc->voice_memo_record_stream=voice_memo_start_record(args,sndcard);
    }
    return 1;
}

static int lpc_cmd_castor3_stop_voice_memo_record(LinphoneCore *lc, char *args)
{
    MSSndCard *sndcard=lc->sound_conf.lsd_card ?lc->sound_conf.lsd_card : lc->sound_conf.ring_sndcard;
    if (lc->voice_memo_record_stream!=NULL){
        voice_memo_stop_record(lc->voice_memo_record_stream);
        lc->voice_memo_record_stream=NULL;
    }
    linphoneCastor3.sync = 1;
    return 1;
}

static int lpc_cmd_castor3_start_voice_memo_play(LinphoneCore *lc, char *args)
{
    MSSndCard *sndcard=lc->sound_conf.lsd_card ?lc->sound_conf.lsd_card : lc->sound_conf.ring_sndcard;
    if (!args) return 1;
    if (lc->ringstream!=NULL){
        ring_stop(lc->ringstream);
        lc->ringstream=NULL;
        lc->dmfs_playing_start_time=0;
    }
    if (lc->voice_memo_play_stream!=NULL){
        voice_memo_stop_play(lc->voice_memo_play_stream);
        lc->voice_memo_play_stream=NULL;
    }
    if (sndcard) {
        lc->voice_memo_play_stream=voice_memo_start_play(args,sndcard);
    }
    return 1;
}

static int lpc_cmd_castor3_stop_voice_memo_play(LinphoneCore *lc, char *args)
{
    MSSndCard *sndcard=lc->sound_conf.lsd_card ?lc->sound_conf.lsd_card : lc->sound_conf.ring_sndcard;
    if (lc->voice_memo_play_stream!=NULL){
        voice_memo_stop_play(lc->voice_memo_play_stream);
        lc->voice_memo_play_stream=NULL;
    }
	linphoneCastor3.sync = 1;
    return 1;
}

#if ENABLE_AUDIO_ENGENEER_MODEL
static int lpc_cmd_castor3_start_playrec(LinphoneCore *lc)
{
    MSSndCard *sndcard=lc->sound_conf.play_sndcard;
    MSSndCard *captcard=lc->sound_conf.capt_sndcard;
    
    if (lc->play_rec_stream!=NULL){
        voice_playrec_stop(lc->play_rec_stream);
        lc->play_rec_stream=NULL;
    }
    if (sndcard) {
        lc->play_rec_stream=voice_playrec_start(sndcard,captcard);
    }
    return 1;
}

static int lpc_cmd_castor3_stop_playrec(LinphoneCore *lc)
{
    if (lc->play_rec_stream!=NULL){
        voice_playrec_stop(lc->play_rec_stream);
        lc->play_rec_stream=NULL;
    }
    linphoneCastor3.sync = 1;
    return 1;
}
#endif

static void linphonec_call_ipcam_streaming_status_callback_default(char bStatus)
{
    return;
}
void (*linphonec_call_ipcam_streaming_status_callback)(char bStatus) = linphonec_call_ipcam_streaming_status_callback_default;
static int lpc_cmd_castor3_start_ipcam_streaming(LinphoneCore *lc, char *args)
{
    if (lc->ipcamstream == NULL)
    {
        lc->ipcamstream = ipcam_streaming_start();
        lc->isIPCamWatching = TRUE;
    }
    linphonec_call_ipcam_streaming_status_callback(1);
    return 1;
}

static int lpc_cmd_castor3_stop_ipcam_streaming(LinphoneCore *lc, char *args)
{
    if (lc->ipcamstream != NULL)
    {
        ipcam_streaming_stop(lc->ipcamstream);
        lc->ipcamstream = NULL;
        lc->isIPCamWatching = FALSE;
    }
    linphonec_call_ipcam_streaming_status_callback(0);
    return 1;
}

static void linphonec_call_ipcam_record_status_callback_default(char bStatus)
{
    return;
}
void (*linphonec_call_ipcam_record_status_callback)(char bStatus) = linphonec_call_ipcam_record_status_callback_default;
static int lpc_cmd_castor3_start_ipcam_record(LinphoneCore *lc, char *args)
{
    if ( args ) args=lpc_strip_blanks(args);
    if ( ! args || ! *args ) return 0;

    if (lc->ipcamstream != NULL)
    {
        ipcam_stream_recorder_start(lc->ipcamstream, args);
        //linphonec_call_ipcam_record_status_callback(1);
    }
    return 1;
}

static int lpc_cmd_castor3_stop_ipcam_record(LinphoneCore *lc, char *args)
{
    if (lc->ipcamstream != NULL)
    {
        ipcam_stream_recorder_stop(lc->ipcamstream);
        //linphonec_call_ipcam_record_status_callback(0);
    }
    return 1;
}

static int lpc_cmd_castor3_ipcam_snapshot(LinphoneCore *lc, char *args){
    if ( args ) args=lpc_strip_blanks(args);
    if ( ! args || ! *args ) return 0;
    if (lc->ipcamstream != NULL)
    {
        ipcam_stream_snapshot(lc->ipcamstream, args);
        linphonec_out("Taking ipcam snapshot in file %s\n", args);
    }
    return 1;
}

static void linphonec_call_capture_record_status_callback_default(char bStatus)
{
    return;
}
void (*linphonec_call_capture_record_status_callback)(char bStatus) = linphonec_call_capture_record_status_callback_default;
static int lpc_cmd_castor3_start_video_memo_record(LinphoneCore *lc, char *args)
{
    if ( args ) args=lpc_strip_blanks(args);
    if ( ! args || ! *args ) return 0;
    linphone_core_start_video_memo_record(lc,args);
    linphonec_call_capture_record_status_callback(1);
    return 1;
}

static int lpc_cmd_castor3_stop_video_memo_record(LinphoneCore *lc, char *args)
{
    linphone_core_stop_video_memo_record(lc);
    linphonec_call_capture_record_status_callback(0);
    return 1;
}

static int lpc_cmd_castor3_start_audiostream_memo_record(LinphoneCore *lc, char *args)
{
    if ( args ) args=lpc_strip_blanks(args);
    if ( ! args || ! *args ) return 0;
    linphone_core_start_audio_memo_record(lc,args);
    return 1;
}

static int lpc_cmd_castor3_stop_audiostream_memo_record(LinphoneCore *lc, char *args)
{
    linphone_core_stop_audio_memo_record(lc);
    return 1;
}

static int lpc_cmd_castor3_start_video_memo_play(LinphoneCore *lc, char *args)
{
    if ( args ) args=lpc_strip_blanks(args);
    if ( ! args || ! *args ) return 0;
    if (lc->ringstream!=NULL){
        ring_stop(lc->ringstream);
        lc->ringstream=NULL;
        lc->dmfs_playing_start_time=0;
    }
    linphone_core_set_play_level(lc,linphoneCastor3.play_level);
    linphone_core_start_video_memo_play(lc,args);
    return 1;
}

static int lpc_cmd_castor3_stop_video_memo_play(LinphoneCore *lc, char *args)
{
    linphone_core_stop_video_memo_play(lc);
    return 1;
}

static int lpc_cmd_castor3_start_adv_play(LinphoneCore *lc, char *args)
{
    lc->isADVPlaying = TRUE;
    return 1;
}

static int lpc_cmd_castor3_stop_adv_play(LinphoneCore *lc, char *args)
{
    if(lc->isADVPlaying)
    {
        lc->isADVPlaying = FALSE;
#ifdef CFG_RTSP_CLIENT_ENABLE
        stopRTSPClient();
#endif
        castor3snd_reinit_for_video_memo_play();
    }
    return 1;
}

static int lpc_cmd_castor3_start_media_video_play(LinphoneCore *lc, char *args)
{
    lc->isMediaVideoPlaying = TRUE;
    return 1;
}

static int lpc_cmd_castor3_stop_media_video_play(LinphoneCore *lc, char *args)
{
    //linphone_core_stop_media_video_play(lc);
    lc->isMediaVideoPlaying = FALSE;
    return 1;
}

static int lpc_cmd_castor3_start_media_audio_play(LinphoneCore *lc, char *args)
{
    lc->isMediaAudioPlaying = TRUE;
    return 1;
}

static int lpc_cmd_castor3_stop_media_audio_play(LinphoneCore *lc, char *args)
{
    //linphone_core_stop_media_audio_play(lc);
    lc->isMediaAudioPlaying = FALSE;
    return 1;
}

static int lpc_cmd_castor3_start_camera_playback(LinphoneCore *lc, char *args)
{
    if (lc->cam_pb_stream == NULL)
    {
        lc->cam_pb_stream = cam_playback_start();
    }
    return 1;
}

static int lpc_cmd_castor3_stop_camera_playback(LinphoneCore *lc, char *args)
{
    if (lc->cam_pb_stream != NULL)
    {
        cam_playback_stop(lc->cam_pb_stream);
        lc->cam_pb_stream = NULL;
    }
    return 1;
}


/***************************************************************************
 *
 *  Command table management funx
 *
 ***************************************************************************/

/*
 * Find a command given its name
 */
static LPC_COMMAND *
lpc_find_command(const char *name)
{
    int i;

    for (i=0; commands[i].name; ++i)
    {
        if (strcmp(name, commands[i].name) == 0)
            return &commands[i];
    }

    for (i=0; advanced_commands[i].name; ++i)
    {
        if (strcmp(name, advanced_commands[i].name) == 0)
            return &advanced_commands[i];
    }

    return (LPC_COMMAND *)NULL;
}


/****************************************************************************
 *
 * $Log: commands.c,v $
 * Revision 1.39  2008/07/03 15:08:34  smorlat
 * api cleanups, interface in progress.
 *
 * Revision 1.38  2008/06/17 20:38:59  smorlat
 * added missing file.
 *
 * Revision 1.37  2008/04/09 09:26:00  smorlat
 * merge various patches
 * H264 support.
 *
 * Revision 1.36  2007/08/01 14:47:53  strk
 *         * console/commands.c: Clean up commands 'nat', 'stun'
 *           and 'firewall' to be more intuitive.
 *
 * Revision 1.35  2007/06/27 09:01:25  smorlat
 * logging improvements.
 *
 * Revision 1.34  2007/02/20 10:17:13  smorlat
 * linphonec friends patch2
 *
 * Revision 1.31  2006/09/22 07:22:47  smorlat
 * linphonecore api changes.
 *
 * Revision 1.30  2006/09/08 15:32:57  smorlat
 * support for using files instead of soundcard (used by linphonec only)
 *
 * Revision 1.29  2006/08/28 14:29:07  smorlat
 * fix bug.
 *
 * Revision 1.28  2006/08/21 12:49:59  smorlat
 * merged several little patches.
 *
 * Revision 1.27  2006/07/17 18:45:00  smorlat
 * support for several event queues in ortp.
 * glib dependency removed from coreapi/ and console/
 *
 * Revision 1.26  2006/04/14 15:16:36  smorlat
 * soundcard use did nothing !
 *
 * Revision 1.25  2006/04/06 20:09:33  smorlat
 * add linphonec command to see and select sound devices.
 *
 * Revision 1.24  2006/03/04 11:17:10  smorlat
 * mediastreamer2 in progress.
 *
 * Revision 1.23  2006/02/20 21:14:01  strk
 * Handled syntax errors with 'friend' command
 *
 * Revision 1.22  2006/02/20 10:20:29  strk
 * Added substring-based filter support for command 'friend list'
 *
 * Revision 1.21  2006/02/02 15:39:18  strk
 * - Added 'friend list' and 'friend call' commands
 * - Allowed for multiple DTFM send in a single line
 * - Added status-specific callback (bare version)
 *
 * Revision 1.20  2006/01/26 11:54:34  strk
 * More robust 'nat' command handler (strip blanks in args)
 *
 * Revision 1.19  2006/01/26 09:48:05  strk
 * Added limits.h include
 *
 * Revision 1.18  2006/01/26 02:18:05  strk
 * Added new commands 'nat use' and 'nat unuse'.
 * These will required a pending patch to linphonecore.c
 * in order to work.
 *
 * Revision 1.17  2006/01/20 14:12:33  strk
 * Added linphonec_init() and linphonec_finish() functions.
 * Handled SIGINT and SIGTERM to invoke linphonec_finish().
 * Handling of auto-termination (-t) moved to linphonec_finish().
 * Reworked main (input read) loop to not rely on 'terminate'
 * and 'run' variable (dropped). configfile_name allocated on stack
 * using PATH_MAX limit. Changed print_usage signature to allow
 * for an exit_status specification.
 *
 * Revision 1.16  2006/01/18 09:25:32  strk
 * Command completion inhibited in proxy addition and auth request prompts.
 * Avoided use of linphonec_readline's internal filename completion.
 *
 * Revision 1.15  2006/01/14 13:29:32  strk
 * Reworked commands interface to use a table structure,
 * used by command line parser and help function.
 * Implemented first level of completion (commands).
 * Added notification of invalid "answer" and "terminate"
 * commands (no incoming call, no active call).
 * Forbidden "call" intialization when a call is already active.
 * Cleaned up all commands, adding more feedback and error checks.
 *
 * Revision 1.14  2006/01/13 13:00:29  strk
 * Added linphonec.h. Code layout change (added comments, forward decl,
 * globals on top, copyright notices and Logs). Handled out-of-memory
 * condition on history management. Removed assumption on sizeof(char).
 * Fixed bug in authentication prompt (introduced by linphonec_readline).
 * Added support for multiple authentication requests (up to MAX_PENDING_AUTH).
 *
 *
 ****************************************************************************/
