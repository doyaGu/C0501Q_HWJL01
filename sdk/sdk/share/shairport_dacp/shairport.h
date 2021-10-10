#ifndef __SHAIRPORT_H__
#define __SHAIRPORT_H__

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <openssl/bio.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "socketlib.h"
#include <regex.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <regex.h>
#include "../dns_sd/mDNSShared/dns_sd.h"

#define HWID_SIZE 6
#define SHAIRPORT_LOG 1
#define LOG_INFO     1
#define LOG_DEBUG    5
#define LOG_DEBUG_V  6
#define LOG_DEBUG_VV 7

#define kDACPBonjourServiceDomain				    "local."
#define kRAOPBonjourServiceType						"_raop._tcp."
#define kDACPBonjourServiceType                     "_dacp._tcp"
#define kRAOPBonjourServiceDomain					"local."
#define kDACPBonjourServiceNamePrefix               "iTunes_Ctrl_"
#define kDACPCommandStr_BeginFastFwd			"beginff"
#define kDACPCommandStr_BeginRewind				"beginrew"
#define kDACPCommandStr_DeviceVolumeLegacy		"devicevolume=" // OBSOLETE: Remove after Denon updates their firmware.
#define kDACPCommandStr_GetProperty				"getproperty"
#define kDACPCommandStr_MuteToggle				"mutetoggle"
#define kDACPCommandStr_NextChapter				"nextchapter"
#define kDACPCommandStr_NextContainer			"nextcontainer"
#define kDACPCommandStr_NextGroup				"nextgroup"
#define kDACPCommandStr_NextItem				"nextitem"
#define kDACPCommandStr_Pause					"pause"
#define kDACPCommandStr_Play					"play"
#define kDACPCommandStr_PlayPause				"playpause"
#define kDACPCommandStr_PlayResume				"playresume"
#define kDACPCommandStr_PlaySpecified			"playspec"
#define kDACPCommandStr_PrevChapter				"prevchapter"
#define kDACPCommandStr_PrevContainer			"prevcontainer"
#define kDACPCommandStr_PrevGroup				"prevgroup"
#define kDACPCommandStr_PrevItem				"previtem"
#define kDACPCommandStr_RepeatAdvance			"repeatadv"
#define kDACPCommandStr_RestartItem				"restartitem"
#define kDACPCommandStr_SetProperty				"setproperty?"
#define kDACPCommandStr_ShuffleSongs			"shufflesongs"
#define kDACPCommandStr_ShuffleToggle			"shuffletoggle"
#define kDACPCommandStr_Stop					"stop"
#define kDACPCommandStr_VolumeDown				"volumedown"
#define kDACPCommandStr_VolumeUp				"volumeup"

// Properties
#define kDACPProperty_DevicePreventPlayback		"dmcp.device-prevent-playback"
#define kDACPProperty_DeviceVolume				"dmcp.device-volume"
#define kDACPProperty_DeviceBusy        		"dmcp.device-busy"

#define	kRAOPTXTKey_AppleModel						"am" // [String] Model of device (e.g. Device1,1). Must be globally unique.
#define	kRAOPTXTKey_Channels						"ch" // [Integer] Number of audio channels (e.g. "2").
#define	kRAOPTXTKey_CompressionTypes				"cn" // [Bit list][AirPlayCompressionType] Supported compression types (e.g. "0,1" for none and Apple Lossless).
#define	kRAOPTXTKey_EncryptionKeyIndex				"ek" // [Integer] Index of the encryption key to use. Currently "1".
#define	kRAOPTXTKey_EncryptionTypes					"et" // [Bit list][AirPlayEncryptionType] Supported encryption types (e.g. "0,1" for none and AES).
#define	kRAOPTXTKey_Features						"ft" // [Hex Integer][AirPlayFeatures] Supported features bits.
#define	kRAOPTXTKey_FirmwareSourceVersion			"fv" // [String] Firmware Source Version (e.g. 74000.2).
#define	kRAOPTXTKey_MACAddress						"ma" // [String] MAC address of the endpoint (e.g. 00:11:22:33:44:55). Usually not published via Bonjour.
#define kRAOPTXTKey_MetaDataTypes					"md" // [Bit list][AirTunesMetaDataType] Supported meta data types (e.g. "0,1" for text and graphics). AirTunes 2.1 and later.
#define	kRAOPTXTKey_Name							"nm" // [String] UTF-8 name of the endpoint. Usually not published via Bonjour.
#define	kRAOPTXTKey_NeedsSoftwareMute				"sm" // [Boolean] "true" if device needs sender to mute the volume before sending.
#define	kRAOPTXTKey_NeedsSoftVolume					"sv" // [Boolean] "true" if device needs sender to adjust the volume before sending.
#define kRAOPTXTKey_OutputBufferSize				"ob" // [Integer] Number of bytes in the device's output buffer (e.g. "176400").
#define kRAOPTXTKey_Password						"pw" // [Boolean] "true" if a password is required to use play to the device, "false" otherwise.
#define kRAOPTXTKey_PublicKey						"pk" // [Hex string] 32-byte Ed25519 public key.
#define kRAOPTXTKey_ProtocolVersion					"vn" // [16.16 version string] AirTunes protocol version supported by the device (e.g. "65536" for 1.0).
#define	kRAOPTXTKey_RFC2617DigestAuth				"da" // [Boolean] "true" if device supports RFC-2617-style digest authentication.
#define	kRAOPTXTKey_SampleRate						"sr" // [Integer] Audio sample rate used by the device (e.g. "44100").
#define	kRAOPTXTKey_SampleSize						"ss" // [Integer] Bit size of each audio sample (e.g. "16").
#define kRAOPTXTKey_Seed							"sd" // [integer] Config seed number. Increment on any software config change.
#define kRAOPTXTKey_SourceVersion					"vs" // [String] AirTunes source version string (e.g. "100.8").
#define	kRAOPTXTKey_StatusFlags						"sf" // [Hex integer][AirPlayStatusFlags] System flags.
#define kRAOPTXTKey_TransportTypes					"tp" // [String] Comma-separated list of supported audio transports (e.g. "TCP,UDP").
#define RAOP_AUTHENTICATE "WWW-Authenticate: Digest realm=\""
#define RAOP_NONCE "\", nonce=\"573674E90C155258179CAA445F66B2AB\""
#define RAOP_AUTH  "Authorization"
#define RAOP_RESPONSE "response"

typedef enum SHAIRPORT_STATE_TAG
{
    SHAIRPORT_NONE = 0,
    SHAIRPORT_PLAY,
    SHAIRPORT_STOP,
    HAIRTUNE_STOP,    
    SHAIRPORT_SHUTDOWN
} SHAIRPORT_STATE;

struct shairbuffer
{
  char *data;
  int   current;
  int   maxsize;
  int   marker;
};

struct keyring
{
  char *aeskey;
  char *aesiv;
  char *fmt;
};

struct comms
{
  int  in[2];
  int  out[2];
};

struct connection
{
  struct shairbuffer  recv;
  struct shairbuffer  resp;
  struct keyring      *keys; // Does not point to malloc'd memory.
  struct comms        *hairtunes;
  int                 clientSocket;
  char                *password;
};

typedef struct SHAIRPORT_PARAM_TAG
{
    int (*set_playstate)(const int new_state);
    int (*get_playstate)(int *cur_state);
    int (*stop_upnp)(void);
    int nCurrState;
    int nAirPlayPlaying;    // set AL_PLAYSTATE_AIRPLAY_PLAYING
    int nUpnpPlaying;    // set AL_PLAYSTATE_UPNP_PLAYING    
    int nStop;    // set AL_PLAYSTATE_STOP
    char* pName;
    char* pHWIDHex;
    int nPort;
    pthread_t taskmDNSResponder;
    pthread_t taskShairport;
    pthread_t taskAcceptClient;
    struct addrinfo* pAddrInfo;
    struct connection*  ptConn;
    int nClientSock;
    int nServerSock;
    int (*set_exception)(const int nException);
    int (*get_exception)();
    unsigned int sinaddr;
    int nWifiMode;
#ifdef CFG_AUDIOLINK_LOCAL_PLAYER_ENABLE
    int (*stop_local)(void);
    int nLocalPlaying;    // set AL_PLAYSTATE_LOCAL_PLAYER_PLAYING    
#endif    
}SHAIRPORT_PARAM;


void sim(int pLevel, char *pValue1, char *pValue2);

int setShairportStatus(int nStatus);
int shairportGetALStatus();
int setAudioLinkStatus(SHAIRPORT_PARAM *pShairport);
int startmDNSResponder();
int getShairportPort();
void setShairportThreadID(pthread_t tshairport);
pthread_t getShairportThreadID();
pthread_t getmDNSResponderThreadID();
pthread_t getAcceptClientThreadID();
void closeShairport(int nClose);
void closeAcceptClient();
int shairportSetException(int nException);
int shairportGetException();
int shairportChangeSpeed(int nSpeed);
int shairportStartAcceptClient();
unsigned int getShairportsaddr();
int getShairportWifiMode();
int shairportgetOtherProtocolPlaying();
int setShairportIni(char* ptIni);
// 1:play pause 2:next item 3: prev item
//    AUDIOLINK_AIRPLAY_PLAY_PAUSE, //  1
//    AUDIOLINK_AIRPLAY_NEXT_ITEM,  //  2
//    AUDIOLINK_AIRPLAY_PREV_ITEM,  //  3
//    AUDIOLINK_AIRPLAY_VOLUME_UP,  //  4
//    AUDIOLINK_AIRPLAY_VOLUME_DOWN,  // 5
//    AUDIOLINK_AIRPLAY_BUSY_ON,  // 6
//    AUDIOLINK_AIRPLAY_PREVENT_PLAYBACK_ON,  // 7
//    AUDIOLINK_AIRPLAY_PREVENT_PLAYBACK_OFF,  //8
//    AUDIOLINK_AIRPLAY_BUSY_OFF,  // 9    
int setShairportDACP(int cmd);

int shairportRegistRAOP(int nPassword);
int shairportClose();
int shairport_update_password();
double shairportGetVolume();

#endif
