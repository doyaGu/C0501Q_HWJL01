/** @file
 * Castor3 Extension for Linphone.
 *
 * @author Jim Tan
 * @version 1.0
 * @date 2013
 * @copyright ITE Tech. Inc. All Rights Reserved.
 */
/** @defgroup linphone Linphone Library
 *  @{
 */
#ifndef LINPHONE_CASTOR3_H
#define LINPHONE_CASTOR3_H

#include <sys/time.h>
#include <mqueue.h>

#ifdef __cplusplus
extern "C"
{
#endif

/** @defgroup linphone_castor3 Castor3 Extension
 *  @{
 */
// Definitions
#define PROMPT_MAX_LEN 256	            ///< Maximum length of prompt string
#define LINPHONE_CASTOR3_MAX_IP_COUNT 5 ///< Maximum calling number on the same time

/**
 * Contact entry definition
 */
typedef struct
{
    char* uri;          ///< SIP address
    char* comment;      ///< Comment
    int blacklist;      ///< In blacklist or not
    char* refkey;       ///< Room address
} LinphoneCastor3Friend;

/**
 * SIP dual registration definition
 */
typedef struct
{
    int sip_register_status;                                    ///<SIP register status.
    int sip_account_1;                                          ///<SIP account 1 register index.
    int sip_account_2;                                          ///<SIP account 2 register index.
    int sip_register_order;                                     ///<SIP account register order.
} LinphoneCastor3DualSIP;

/**
 * LinphoneRegistrationState describes proxy registration states.
**/
typedef enum _LinphoneCastor3RegistrationState{
	Linphone_RegistrationNone, /**<Initial state for registrations */
	Linphone_RegistrationProgress, /**<Registration is in progress */
	Linphone_RegistrationOk,	/**< Registration is successful */
	Linphone_RegistrationCleared, /**< Unregistration succeeded */
	Linphone_RegistrationFailed	/**<Registration failed */
}LinphoneCastor3RegistrationState;

/**
 * Global structure definition used to exchange data between linphone and application.
 */
typedef struct
{
    int play_level;                                             ///< Voice sound level
    int rec_level;                                              ///< MIC sound level
    int ring_level;                                             ///< Ring sound level
    int key_level;                                              ///< Key sound level
    int warn_level;                                             ///< Warn sound level
    char friend_addr[32];                                       ///< Contact room address, used to add a new contact.
    char friend_sip[32];                                        ///< Contact SIP address, used to add a new contact.
    char* friend_comment;                                       ///< Contact comment, used to add a new contact.
    int friend_count;                                           ///< Total count of contacts.
    LinphoneCastor3Friend* friends;                             ///< Contacts array to export to application.
    char video_msg_uri[128];                                    ///< SIP address used to watch video message on center.
    int calling_ip_count;                                       ///< The count of calling on the same time.
    char calling_ip_array[LINPHONE_CASTOR3_MAX_IP_COUNT][16];   ///< Array of IP addresses of calling on the same time.
    int initialized;                                            ///< Linphone is initialized or not.
    int quit;                                                   ///< Linphone is quiting or not.
    mqd_t mq;                                                   ///< Message queue to receive commands from application.
    int camera_id;                                              ///< Current call ID of camera, used to stop watching camera.
    int sync;                                                   ///< Sync with command.
    char sip_server_uri[128];                                   ///< SIP address used to call user registered on SIP server.
    int calling_mobile_count;
    char calling_mobile_array[LINPHONE_CASTOR3_MAX_IP_COUNT][16];
    int calllog_refkey;                                         ///< Call log reference key index.
    LinphoneCastor3DualSIP DualSIP;
} LinphoneCastor3;

// Global variables
extern LinphoneCastor3 linphoneCastor3;         ///< Global structure variable used to exchange data between linphone and application.
extern unsigned int linphonec_camera_enabled;   ///< Enables camera device or not.

// Functions
/**
 * Callback to notify a call is ended.
 *
 * @param addr the IP address of target.
 * @param id the call ID.
 */
extern void (*linphonec_call_end_notify_callback)(char* addr, int id);

/**
 * Callback to notify a call is incoming.
 *
 * @param username the user name of target.
 * @param addr the IP address of target.
 * @param id the call ID.
 * @param video the call includes video stream or not.
 */
extern void (*linphonec_call_imcoming_received_notify_callback)(char* username, char* addr, int id, int video);

/**
 * Callback to notify a call is connected.
 *
 * @param addr the IP address of target.
 * @param id the call ID.
 */
extern void (*linphonec_call_connected_notify_callback)(char* addr, int id);

/**
 * Callback to notify a call is error.
 *
 * @param addr the IP address of target.
 * @param id the call ID.
 * @param code the SIP error code.
 */
extern void (*linphonec_call_error_notify_callback)(char* addr, int id, int code);

/**
 * Callback to get the room address of a IP address. This is used to get blacklist information when a call is received.
 *
 * @param addr the IP address of target.
 * @param id the call ID.
 * @return the room address.
 */
extern char* (*linphonec_call_get_addr_callback)(char* ip);

/**
 * Callback to notify configuration file is saved. This is used to gave application change to update CRC data.
 *
 * @param filename the file name of configuration.
 */
extern void (*lp_config_sync_notify_callback)(char* filename);

/**
 * Callback when receiving a call is on do not disturb mode. This is used to determine the behavior after receiving this call.
 *
 * @param addr the IP address of target.
 * @return 1 to receive this call; 0 to hang up this call with SIP error code 480; 2 to receive this call silently, the application need to redirect it; -1 to hang up this call with SIP error code 600.
 */
extern int (*linphonec_call_on_do_not_disturb_callback)(char* ip);

/**
 * Callback when receiving a call on busy state. This is used to determine the behavior after receiving this call.
 *
 * @param addr the IP address of target.
 * @return 1 to receive this call; 0 to hang up this call.
 */
extern int (*linphonec_call_allow_call_waiting_callback)(char* ip);

/**
 * Callback when receiving a call and to know whether the application is watching camera or video message. This is used to determine the behavior after receiving this call.
 *
 * @return 1 if the application is watching camera or video message; 0 otherwise.
 */
extern int (*linphonec_call_is_camera_viewing_callback)(void);

/**
 * Callback when calling. This is used to determine the behavior after a call is on calling.
 *
 * @return 1 if the application is calling; 0 is no calling IDLE.
 */
extern int (*linphonec_call_is_calling_callback)(void);

/**
 * Callback to notify this library is initialized. This is used to update UI.
 */
extern void (*linphonec_initialized_notify_callback)(void);

/**
 * Callback to notify a new log is created. This is used to sync time with snapshot.
 *
 * @param start_time the create time of log.
 */
extern void (*linphonec_new_log_notify_callback)(time_t start_time);

/**
 * Callback when receiving a call and to know whether allowing the call to be spotted. This is used to determine the behavior after receiving this call.
 *
 * @return 1 if the call being spotted; 0 otherwise.
 */
extern int (*linphonec_call_is_allowing_spotted_callback)(char* addr, int id, int video);

/**
 * Callback to add a call log.
 *
 * type     ///< Call log type
 * ip       ///< IP address of target
 * time     ///< Calling time
 * duration ///< Calling duration, in seconds.
 * refkey   ///< Index number as reference key
 * init     ///< Called on init stage or not
 */
extern void (*linphonec_call_log_add_callback)(int type, char* ip, char* time, int duration, int refkey, int init);
extern void (*linphonec_call_camera_connect_error_callback)(char bError);
extern void (*linphonec_call_ipcam_streaming_status_callback)(char bStatus);
extern void (*linphonec_call_ipcam_record_status_callback)(char bStatus);
extern void (*linphonec_call_capture_record_status_callback)(char bStatus);
extern void (*linphonec_call_first_snapshot_callback)(char* addr);
extern void (*linphonec_call_stop_audio_play_callback)(void);
extern void (*linphonec_call_stop_video_play_callback)(void);

#if !defined(CFG_SENSOR_ENABLE) && defined(ENABLE_VIDEO_MULTICAST)
typedef char* (*BUF_CALLBACK)(char *remote_addr);
#endif
/**
 * The entry point of linphone task. The application need to create a task to run this function.
 *
 * @param argc the count of arguments.
 * @param argv the array of of arguments.
 * @return unused, always 0.
 */
int linphonec_main(int argc, char *argv[]);

/** @} */ // end of linphone_castor3

#if !defined(CFG_SENSOR_ENABLE) && defined(ENABLE_VIDEO_MULTICAST)
void Group_SetBufCallBack(BUF_CALLBACK bufCallback);
#endif

#ifdef __cplusplus
}
#endif

#endif // LINPHONE_CASTOR3_H
/** @} */ // end of linphone