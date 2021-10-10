/*
 * cds.c : GeeXboX uShare Content Directory Service
 * Originally developped for the GeeXboX project.
 * Parts of the code are originated from GMediaServer from Oskar Liljeblad.
 * Copyright (C) 2005-2007 Benjamin Zores <ben@geexbox.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Library General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "upnp.h"
#include "upnptools.h"

#include "urender.h"
#include "services.h"
#include "buffer.h"

/* Represent the AVT SetAVTransportURI action. */
#define SERVICE_AVT_ACTION_SET_URI "SetAVTransportURI"

/* Represent the AVT GetMediaInfo action. */
#define SERVICE_AVT_ACTION_GET_MEDIA_INFO "GetMediaInfo"

/* Represent the AVT GetTransportInfo action. */
#define SERVICE_AVT_ACTION_GET_TS_INFO "GetTransportInfo"

/* Represent the AVT GetPositionInfo action. */
#define SERVICE_AVT_ACTION_GET_POS_INFO "GetPositionInfo"

/* Represent the AVT GetDeviceCapabilities action. */
#define SERVICE_AVT_ACTION_GET_DEV_CAPS "GetDeviceCapabilities"

/* Represent the AVT GetTransportSettings action. */
#define SERVICE_AVT_ACTION_GET_TS_SETTING "GetTransportSettings"

/* Represent the AVT Stop action. */
#define SERVICE_AVT_ACTION_STOP "Stop"

/* Represent the AVT Play action. */
#define SERVICE_AVT_ACTION_PLAY "Play"

/* Represent the AVT Pause action. */
#define SERVICE_AVT_ACTION_PAUSE "Pause"

/* Represent the AVT Record action. */
#define SERVICE_AVT_ACTION_RECORD "Record"

/* Represent the AVT Seek action. */
#define SERVICE_AVT_ACTION_SEEK "Seek"

/* Represent the AVT Next action. */
#define SERVICE_AVT_ACTION_NEXT "Next"

/* Represent the AVT Previous action. */
#define SERVICE_AVT_ACTION_PREVIOUS "Previous"

/* Represent the AVT SetPlayMode action. */
#define SERVICE_AVT_ACTION_SET_PLAY_MODE "SetPlayMode"

/* Represent the AVT GetCurrentTransportActions action. */
#define SERVICE_AVT_ACTION_GET_CUR_TS_ACT "GetCurrentTransportActions"


/* Represent the AVT TransportState variable. */
#define SERVICE_AVT_VAR_TS_STATE "TransportState"
char *TransportState[] = {
    "STOPPED",
    "PLAYING",
    "TRANSITIONING",
    "PAUSED_PLAYBACK",
    "PAUSED_RECORDING",
    "RECORDING",
    "NO_MEDIA_PRESENT",
};


/* Represent the AVT TransportStatus variable. */
#define SERVICE_AVT_VAR_TS_STATUS "TransportStatus"
char *TransportStatus[] = {
    "OK",
    "ERROR_OCCURRED",
};

typedef enum tagAVT_TRANSPORTSTATUS_E
{
    AVT_TRANSPORTSTATUS_OK = 0,
    AVT_TRANSPORTSTATUS_ERROR_OCCURRED,
} AVT_TRANSPORTSTATUS_E;


/* Represent the AVT PlaybackStorageMedium variable. */
#define SERVICE_AVT_VAR_PB_STOR_MEDIUM "PlaybackStorageMedium"

/* Represent the AVT RecordStorageMedium variable. */
#define SERVICE_AVT_VAR_REC_STOR_MEDIUM "RecordStorageMedium"

/* Represent the AVT PossiblePlaybackStorageMedia variable. */
#define SERVICE_AVT_VAR_POSSIB_PB_STOR_MEDIA "PossiblePlaybackStorageMedia"

/* Represent the AVT PossibleRecordStorageMedia variable. */
#define SERVICE_AVT_VAR_POSSIB_REC_STOR_MEDIA "PossibleRecordStorageMedia"

/* Represent the AVT CurrentPlayMode variable. */
#define SERVICE_AVT_VAR_CUR_PLAY_MODE "CurrentPlayMode"
char *CurrentPlayMode[] = {
    "NORMAL",
    "SHUFFLE",
    "REPEAT_ONE",
    "REPEAT_ALL",
    "RANDOM",
    "DIRECT_1",
    "INTRO",
};

typedef enum tagAVT_PLAYMODE_E
{
    AVT_PLAYMODE_NORMAL = 0,
    AVT_PLAYMODE_SHUFFLE,
    AVT_PLAYMODE_REPEAT_ONE,
    AVT_PLAYMODE_REPEAT_ALL,
    AVT_PLAYMODE_RANDOM,
    AVT_PLAYMODE_DIRECT_1,
    AVT_PLAYMODE_INTRO,
} AVT_PLAYMODE_E;


/* Represent the AVT TransportPlaySpeed variable. */
#define SERVICE_AVT_VAR_TS_PLAY_SPEED "TransportPlaySpeed"
char *TransportPlaySpeed[] = {
    "0",
    "1",
    "2",
    "3",
    "4",
};

typedef enum tagAVT_PLAYSPEED_E
{
    AVT_PLAYSPEED_0 = 0,
    AVT_PLAYSPEED_1,
    AVT_PLAYSPEED_2,
    AVT_PLAYSPEED_3,
    AVT_PLAYSPEED_4,
} AVT_PLAYSPEED_E;


/* Represent the AVT RecordMediumWriteStatus variable. */
#define SERVICE_AVT_VAR_REC_MEDIUM_WR_STATUS "RecordMediumWriteStatus"

/* Represent the AVT CurrentRecordQualityMode variable. */
#define SERVICE_AVT_VAR_CUR_REC_QUAL_MODE "CurrentRecordQualityMode"

/* Represent the AVT PossibleRecordQualityModes variable. */
#define SERVICE_AVT_VAR_POSSIB_REC_QUAL_MODES "PossibleRecordQualityModes"

/* Represent the AVT NumberOfTracks variable. */
#define SERVICE_AVT_VAR_NUM_OF_TRACKS "NumberOfTracks"

/* Represent the AVT CurrentTrack variable. */
#define SERVICE_AVT_VAR_CUR_TRACK "CurrentTrack"

/* Represent the AVT CurrentTrackDuration variable. */
#define SERVICE_AVT_VAR_CUR_TRACK_DURATION "CurrentTrackDuration"

/* Represent the AVT CurrentMediaDuration variable. */
#define SERVICE_AVT_VAR_CUR_MEDIA_DURATION "CurrentMediaDuration"

/* Represent the AVT CurrentTrackMetaData variable. */
#define SERVICE_AVT_VAR_CUR_TRACK_META_DATA "CurrentTrackMetaData"

/* Represent the AVT CurrentTrackURI variable. */
#define SERVICE_AVT_VAR_CUR_TRACK_URI "CurrentTrackURI"

/* Represent the AVT AVTransportURI variable. */
#define SERVICE_AVT_VAR_URI "AVTransportURI"

/* Represent the AVT AVTransportURIMetaData variable. */
#define SERVICE_AVT_VAR_URI_META_DATA "AVTransportURIMetaData"

/* Represent the AVT NextAVTransportURI variable. */
#define SERVICE_AVT_VAR_NEXT_URI "NextAVTransportURI"

/* Represent the AVT NextAVTransportURIMetaData variable. */
#define SERVICE_AVT_VAR_NEXT_URI_META_DATA "NextAVTransportURIMetaData"

/* Represent the AVT RelativeTimePosition variable. */
#define SERVICE_AVT_VAR_REL_TIME_POS "RelativeTimePosition"

/* Represent the AVT AbsoluteTimePosition variable. */
#define SERVICE_AVT_VAR_ABS_TIME_POS "AbsoluteTimePosition"

/* Represent the AVT RelativeCounterPosition variable. */
#define SERVICE_AVT_VAR_REL_CNT_POS "RelativeCounterPosition"

/* Represent the AVT AbsoluteCounterPosition variable. */
#define SERVICE_AVT_VAR_ABS_CNT_POS "AbsoluteCounterPosition"

/* Represent the AVT CurrentTransportActions variable. */
#define SERVICE_AVT_VAR_CUR_TS_ACT "CurrentTransportActions"

/* Represent the AVT LastChange variable. */
#define SERVICE_AVT_VAR_LAST_CHANGE "LastChange"

/* Represent the AVT ARG Unit */
#define SERVICE_AVT_ARG_UNIT "Unit"

/* Represent the AVT ARG Target */
#define SERVICE_AVT_ARG_TARGET "Target"

/* Represent the CurrentURI argument. */
#define SERVICE_AVT_ARG_CUR_URI "CurrentURI"

/* Represent the NextURI argument. */
#define SERVICE_AVT_ARG_NEXT_URI "NextURI"

/* Represent the CurrentURIMetaData argument. */
#define SERVICE_AVT_ARG_CUR_URI_META_DATA "CurrentURIMetaData"

/* Represent the NextURIMetaData argument. */
#define SERVICE_AVT_ARG_NEXT_URI_META_DATA "NextURIMetaData"

/* Represent the CurrentTransportState argument. */
#define SERVICE_AVT_ARG_CUR_TS_STATE "CurrentTransportState"

/* Represent the AVT CurrentTransportStatus argument. */
#define SERVICE_AVT_ARG_CUR_TS_STATUS "CurrentTransportStatus"

/* Represent the AVT CurrentSpeed argument. */
#define SERVICE_AVT_ARG_CUR_SPEED "CurrentSpeed"

/* Represent the AVT Speed argument. */
#define SERVICE_AVT_ARG_SPEED "Speed"

/* Represent the AVT NewPlayMode argument. */
#define SERVICE_AVT_ARG_NEW_PLAY_MODE "NewPlayMode"

/* Represent the AVT NrTracks argument. */
#define SERVICE_AVT_ARG_NR_TRACKS "NrTracks"

/* Represent the AVT MediaDuration argument. */
#define SERVICE_AVT_ARG_MEDIA_DURATION "MediaDuration"

/* Represent the AVT PlayMedium argument. */
#define SERVICE_AVT_ARG_PLAY_MEDIUM "PlayMedium"

/* Represent the AVT RecordMedium argument. */
#define SERVICE_AVT_ARG_REC_MEDIUM "RecordMedium"

/* Represent the AVT WriteStatus argument. */
#define SERVICE_AVT_ARG_WR_STATUS "WriteStatus"

/* Represent the AVT Track argument. */
#define SERVICE_AVT_ARG_TRACK "Track"

/* Represent the AVT TrackDuration argument. */
#define SERVICE_AVT_ARG_TRACK_DURATION "TrackDuration"

/* Represent the AVT TrackMetaData argument. */
#define SERVICE_AVT_ARG_TRACK_META_DATA "TrackMetaData"

/* Represent the AVT TrackURI argument. */
#define SERVICE_AVT_ARG_TRACK_URI "TrackURI"

/* Represent the AVT RelTime argument. */
#define SERVICE_AVT_ARG_REL_TIME "RelTime"

/* Represent the AVT AbsTime argument. */
#define SERVICE_AVT_ARG_ABS_TIME "AbsTime"

/* Represent the AVT RelCount argument. */
#define SERVICE_AVT_ARG_REL_CNT "RelCount"

/* Represent the AVT AbsCount argument. */
#define SERVICE_AVT_ARG_ABS_CNT "AbsCount"

/* Represent the AVT PlayMedia argument. */
#define SERVICE_AVT_ARG_PLAY_MEDIA "PlayMedia"

/* Represent the AVT RecMedia argument. */
#define SERVICE_AVT_ARG_REC_MEDIA "RecMedia"

/* Represent the AVT RecQualityModes argument. */
#define SERVICE_AVT_ARG_REC_QUAL_MODES "RecQualityModes"

/* Represent the AVT PlayMode argument. */
#define SERVICE_AVT_ARG_PLAY_MODE "PlayMode"

/* Represent the AVT RecQualityMode argument. */
#define SERVICE_AVT_ARG_REC_QUAL_MODE "RecQualityMode"

/* Represent the AVT Actions argument. */
#define SERVICE_AVT_ARG_ACTS "Actions"


/* Represent the AVT UNKNOWN */
#define SERVICE_AVT_UNKNOWN "UNKNOWN"

/* Represent the AVT NOT_IMPLEMENTED */
#define SERVICE_AVT_NOT_IMPLEMENTED "NOT_IMPLEMENTED"

/* Represent the AVT TRACK_NR */
#define SERVICE_AVT_TRACK_NR "TRACK_NR"

/* Represent the AVT NONE */
#define SERVICE_AVT_NONE "NONE"

/* Represent the AVT NETWORK */
#define SERVICE_AVT_NETWORK "NETWORK"

/* Represent the AVT HDD */
#define SERVICE_AVT_HDD "HDD"

/* Represent the AVT CD-DA */
#define SERVICE_AVT_CD_DA "CD-DA"

#define AVT_EVENT "Event"
#define AVT_NAMESPACE "xmlns=\"urn:schemas-upnp-org:metadata-1-0/AVT/\""
#define AVT_INSTANCE_ID "InstanceID"

/* Represent the LastChange service variable. */
#define SERVICE_VAR_LASTCHANGE "LastChange"

// AVT METADATA
#define SERVICE_AVT_META_TITLE "dc:title"
#define SERVICE_AVT_META_DATE "dc:date"
#define SERVICE_AVT_META_GENRE "upnp:genre"
#define SERVICE_AVT_META_ALBUM "upnp:album"
#define SERVICE_AVT_META_ARTIST "upnp:artist"
#define SERVICE_AVT_META_CREATOR "dc:creator"
#define SERVICE_AVT_META_ORIGINALTRACKNUMBER "upnp:originalTrackNumber"
#define SERVICE_AVT_META_SIZE "size"
#define SERVICE_AVT_META_DURATION "duration"
#define SERVICE_AVT_META_PROTOCOLINFO "protocolInfo"
#define SERVICE_AVT_META_ALBUMARTURI "upnp:albumArtURI"
#define SERVICE_AVT_META_CLASS "upnp:class"

#define SERVICE_AVT_MIME_AUDIO_MPEG                   "audio/mpeg"
#define SERVICE_AVT_MIME_AUDIO_FLAC                   "audio/flac"
#define SERVICE_AVT_MIME_AUDIO_XMSWMA                 "audio/x-ms-wma"
#define SERVICE_AVT_MIME_AUDIO_AAC                    "audio/aac"
// add "audio/wav" to fix win8 media player
#define SERVICE_AVT_MIME_AUDIO_WAV                   "audio/wav"
#define SERVICE_AVT_MIME_AUDIO_XWAV                 "audio/x-wav"

static int gnStopNotify = 0;

extern struct urender_t *ut;
extern urender_callback urender_cb;

static int gnSetURIStop = 0;

static int avt_var_init (void);

/* List of UPnP AVTransport Service variables */
struct service_variables_t avt_service_variables = {
    {SERVICE_VAR_LASTCHANGE, NULL, NULL},
    {NULL, NULL, NULL},
    1,
    avt_var_init,
};

static void
lastchange_add_header (struct buffer_t *out)
{
    buffer_appendf (out, "&lt;%s %s&gt;", AVT_EVENT, AVT_NAMESPACE);
    buffer_appendf (out, "&lt;%s val=\"%s\"&gt;", AVT_INSTANCE_ID, "0");
}

static void
lastchange_add_footer (struct buffer_t *out)
{
    buffer_appendf (out, "&lt;/%s&gt;", AVT_INSTANCE_ID);
    buffer_appendf (out, "&lt;/%s&gt;", AVT_EVENT);
}

static int
avt_var_init (void)
{
    struct buffer_t *out = NULL;

    out = buffer_new ();
    if (!out)
        return 0;

    ut->avt.transport_state = AVT_TRANSPORTSTATE_STOPPED;
    ut->avt.transport_status = AVT_TRANSPORTSTATUS_OK;
    ut->avt.play_speed = AVT_PLAYSPEED_1;
    ut->avt.play_mode = AVT_PLAYMODE_NORMAL;

    lastchange_add_header (out);
    buffer_appendf (out, "&lt;%s val=\"%s\"/&gt;", SERVICE_AVT_VAR_CUR_PLAY_MODE, CurrentPlayMode[ut->avt.play_mode]);
    buffer_appendf (out, "&lt;%s val=\"%s\"/&gt;", SERVICE_AVT_VAR_REC_STOR_MEDIUM, SERVICE_AVT_NOT_IMPLEMENTED);
    buffer_appendf (out, "&lt;%s val=\"%s\"/&gt;", SERVICE_AVT_VAR_CUR_TRACK_URI, "");
    buffer_appendf (out, "&lt;%s val=\"%s\"/&gt;", SERVICE_AVT_VAR_CUR_TRACK_DURATION, "00:00:00");
    buffer_appendf (out, "&lt;%s val=\"%s\"/&gt;", SERVICE_AVT_VAR_CUR_REC_QUAL_MODE, SERVICE_AVT_NOT_IMPLEMENTED);
    buffer_appendf (out, "&lt;%s val=\"%s\"/&gt;", SERVICE_AVT_VAR_CUR_MEDIA_DURATION, "00:00:00");
    buffer_appendf (out, "&lt;%s val=\"%s\"/&gt;", SERVICE_AVT_VAR_URI, "");
    buffer_appendf (out, "&lt;%s val=\"%s\"/&gt;", SERVICE_AVT_VAR_TS_STATE, TransportState[ut->avt.transport_state]);
    buffer_appendf (out, "&lt;%s val=\"%s\"/&gt;", SERVICE_AVT_VAR_CUR_TRACK_META_DATA, "");
    buffer_appendf (out, "&lt;%s val=\"%s\"/&gt;", SERVICE_AVT_VAR_NEXT_URI, SERVICE_AVT_NOT_IMPLEMENTED);
    buffer_appendf (out, "&lt;%s val=\"%s\"/&gt;", SERVICE_AVT_VAR_POSSIB_REC_QUAL_MODES, SERVICE_AVT_NOT_IMPLEMENTED);
    buffer_appendf (out, "&lt;%s val=\"%s\"/&gt;", SERVICE_AVT_VAR_CUR_TRACK, "0");
    buffer_appendf (out, "&lt;%s val=\"%s\"/&gt;", SERVICE_AVT_VAR_NEXT_URI_META_DATA, SERVICE_AVT_NOT_IMPLEMENTED);
    buffer_appendf (out, "&lt;%s val=\"%s\"/&gt;", SERVICE_AVT_VAR_PB_STOR_MEDIUM, SERVICE_AVT_NONE);
    buffer_appendf (out, "&lt;%s val=\"%s,%s,%s,%s,%s,%s\"/&gt;", SERVICE_AVT_VAR_CUR_TS_ACT,
                                                                  SERVICE_AVT_ACTION_PLAY,
                                                                  SERVICE_AVT_ACTION_PAUSE,
                                                                  SERVICE_AVT_ACTION_STOP,
                                                                  SERVICE_AVT_ACTION_SEEK,
                                                                  SERVICE_AVT_ACTION_NEXT,
                                                                  SERVICE_AVT_ACTION_PREVIOUS);
    buffer_appendf (out, "&lt;%s val=\"%s\"/&gt;", SERVICE_AVT_VAR_REC_MEDIUM_WR_STATUS, SERVICE_AVT_NOT_IMPLEMENTED);
    buffer_appendf (out, "&lt;%s val=\"%s,%s,%s,%s,%s\"/&gt;", SERVICE_AVT_VAR_POSSIB_PB_STOR_MEDIA,
                                                               SERVICE_AVT_NONE, SERVICE_AVT_NETWORK, SERVICE_AVT_HDD,
                                                               SERVICE_AVT_CD_DA, SERVICE_AVT_UNKNOWN);
    buffer_appendf (out, "&lt;%s val=\"%s\"/&gt;", SERVICE_AVT_VAR_URI_META_DATA, "");
    buffer_appendf (out, "&lt;%s val=\"%s\"/&gt;", SERVICE_AVT_VAR_NUM_OF_TRACKS, "0");
    buffer_appendf (out, "&lt;%s val=\"%s\"/&gt;", SERVICE_AVT_VAR_POSSIB_REC_STOR_MEDIA, SERVICE_AVT_NOT_IMPLEMENTED);
    buffer_appendf (out, "&lt;%s val=\"%s\"/&gt;", SERVICE_AVT_VAR_TS_STATUS, TransportStatus[ut->avt.transport_status]);
    buffer_appendf (out, "&lt;%s val=\"%s\"/&gt;", SERVICE_AVT_VAR_TS_PLAY_SPEED, TransportPlaySpeed[ut->avt.play_speed]);
    lastchange_add_footer (out);

    avt_service_variables.var_str[0] = strdup(out->buf);
    buffer_free (out);

    return 1;
}



static int
AVT_FreeMetadata(AVT_METADATA *metadata)
{
    int nRet = 0;

    if (metadata->title)
    {
        free(metadata->title);
        metadata->title = NULL;
    }
    if (metadata->date)
    {
        free(metadata->date);
        metadata->date = NULL;
    }
    if (metadata->genre)
    {
        free(metadata->genre);
        metadata->genre = NULL;
    }
    if (metadata->album)
    {
        free(metadata->album);
        metadata->album = NULL;
    }
    if (metadata->artist)
    {
        free(metadata->artist);
        metadata->artist = NULL;
    }
    if (metadata->creator)
    {
        free(metadata->creator);
        metadata->creator = NULL;
    }
    if (metadata->originalTrackNumber)
    {
        free(metadata->originalTrackNumber);
        metadata->originalTrackNumber = NULL;
    }
    if (metadata->size)
    {
        free(metadata->size);
        metadata->size = NULL;
    }
    if (metadata->duration)
    {
        free(metadata->duration);
        metadata->duration = NULL;
    }
    if (metadata->albumArtURI)
    {
        free(metadata->albumArtURI);
        metadata->albumArtURI = NULL;
    }
    if (metadata->upnpClass)
    {
        free(metadata->upnpClass);
        metadata->upnpClass = NULL;
    }
    if (metadata->res)
    {
        free(metadata->res);
        metadata->res = NULL;
    }
    if (metadata->protocolInfo)
    {
        free(metadata->protocolInfo);
        metadata->protocolInfo = NULL;
    }

    return nRet;
}



static int
AVT_ParseURIMetadata(struct urender_t *ut)
{
    int nRet = 0;
    IXML_Document *doc = NULL;
    IXML_Node *node = NULL, *child_node = NULL, *attr_node = NULL;
    IXML_NamedNodeMap *attr_map = NULL;
    char *str1 = NULL;
    int index = 0, attr_count = 0;
    const char *uri_metadata = NULL;
    AVT_METADATA *avt_metadata = NULL;
    int nShoutcast =0;
    int nShoutcastProtocol = 0;
    
    if (!ut)
    {
        nRet = -1;
        goto end;
    }

    uri_metadata = ut->uri_metadata;
    avt_metadata = &ut->avt.metadata;

    if (!uri_metadata || !avt_metadata)
    {
        nRet = -2;
        goto end;
    }

    nRet = AVT_FreeMetadata(avt_metadata);

    doc = ixmlParseBuffer(uri_metadata);
    if (!doc)
    {
        printf("[AVT]%s() L#%ld: Parse metadata error!\r\n", __FUNCTION__, __LINE__);
        goto end;
    }

    node = (IXML_Node *)doc;
    if (!node)
    {
        printf("[AVT]%s() L#%ld: Get root node error!\r\n", __FUNCTION__, __LINE__);
        goto end;
    }

    // <DIDL-Lite>
    node = ixmlNode_getFirstChild(node);
    if (!node)
    {
        printf("[AVT]%s() L#%ld: Get node <DIDL-Lite> error!\r\n", __FUNCTION__, __LINE__);
        goto end;
    }
    // <item>
    node = ixmlNode_getFirstChild(node);
    if (!node)
    {
        printf("[AVT]%s() L#%ld: Get node <item> error!\r\n", __FUNCTION__, __LINE__);
        goto end;
    }
    // <first child of item>
    node = ixmlNode_getFirstChild(node);
    if (!node)
    {
        printf("[AVT]%s() L#%ld: Get first node of <item> error!\r\n", __FUNCTION__, __LINE__);
        goto end;
    }

    for (; node; node = ixmlNode_getNextSibling (node))
    {
        child_node = ixmlNode_getFirstChild(node);
        if (child_node)
        {
            if (!strcmp(node->nodeName, SERVICE_AVT_META_TITLE))
            {
                if (avt_metadata->title)
                {
                    free(avt_metadata->title);
                }
                avt_metadata->title = strdup(child_node->nodeValue);
                printf("[AVT] avt_metadata->title = %s\r\n", avt_metadata->title);
            }
            else if (!strcmp(node->nodeName, SERVICE_AVT_META_DATE))
            {
                if (avt_metadata->date)
                {
                    free(avt_metadata->date);
                }
                avt_metadata->date = strdup(child_node->nodeValue);
                printf("[AVT] avt_metadata->date = %s\r\n", avt_metadata->date);
            }
            else if (!strcmp(node->nodeName, SERVICE_AVT_META_GENRE))
            {
                if (avt_metadata->genre)
                {
                    free(avt_metadata->genre);
                }
                avt_metadata->genre = strdup(child_node->nodeValue);
                printf("[AVT] avt_metadata->genre = %s\r\n", avt_metadata->genre);
            }
            else if (!strcmp(node->nodeName, SERVICE_AVT_META_ALBUM))
            {
                if (avt_metadata->album)
                {
                    free(avt_metadata->album);
                }
                avt_metadata->album = strdup(child_node->nodeValue);
                printf("[AVT] avt_metadata->album = %s\r\n", avt_metadata->album);
            }
            else if (!strcmp(node->nodeName, SERVICE_AVT_META_ARTIST))
            {
                if (avt_metadata->artist)
                {
                    free(avt_metadata->artist);
                }
                avt_metadata->artist = strdup(child_node->nodeValue);
                printf("[AVT] avt_metadata->artist = %s\r\n", avt_metadata->artist);
            }
            else if (!strcmp(node->nodeName, SERVICE_AVT_META_CREATOR))
            {
                if (avt_metadata->creator)
                {
                    free(avt_metadata->creator);
                }
                avt_metadata->creator = strdup(child_node->nodeValue);
                printf("[AVT] avt_metadata->creator = %s\r\n", avt_metadata->creator);
            }
            else if (!strcmp(node->nodeName, SERVICE_AVT_META_ORIGINALTRACKNUMBER))
            {
                if (avt_metadata->originalTrackNumber)
                {
                    free(avt_metadata->originalTrackNumber);
                }
                avt_metadata->originalTrackNumber = strdup(child_node->nodeValue);
                printf("[AVT] avt_metadata->originalTrackNumber = %s\r\n", avt_metadata->originalTrackNumber);
            }
            else if (!strcmp(node->nodeName, SERVICE_AVT_META_ALBUMARTURI))
            {
                if (avt_metadata->albumArtURI)
                {
                    free(avt_metadata->albumArtURI);
                }
                avt_metadata->albumArtURI = strdup(child_node->nodeValue);
                printf("[AVT] avt_metadata->albumArtURI = %s\r\n", avt_metadata->albumArtURI);
            }
            else if (!strcmp(node->nodeName, SERVICE_AVT_META_CLASS))
            {
                if (avt_metadata->upnpClass)
                {
                    free(avt_metadata->upnpClass);
                }
                avt_metadata->upnpClass = strdup(child_node->nodeValue);
                if (!strcmp(avt_metadata->upnpClass, UPNP_CLASS_AUDIOITEM_AUDIOBROADCAST)) {
                    // it is shoutcast
                    printf("[AVT] Shoutcast \n");
                    nShoutcast = 1;
                }                
                printf("[AVT] avt_metadata->upnpClass = %s\r\n", avt_metadata->upnpClass);
            }
            else if (!strcmp(node->nodeName, "res"))
            {
                if (avt_metadata->res)
                {
                    free(avt_metadata->res);
                }
                avt_metadata->res = strdup(child_node->nodeValue);
                printf("[AVT] avt_metadata->res = %s\r\n", avt_metadata->res);

                attr_map = ixmlNode_getAttributes(node);
                if (attr_map)
                {
                    attr_count = ixmlNamedNodeMap_getLength(attr_map);
                    for (index = 0; index < attr_count; index++)
                    {
                        attr_node = ixmlNamedNodeMap_item(attr_map, index);
                        if (attr_node)
                        {
                            if (!strcmp(attr_node->nodeName, SERVICE_AVT_META_SIZE))
                            {
                                if (avt_metadata->size)
                                {
                                    free(avt_metadata->size);
                                }
                                avt_metadata->size = strdup(attr_node->nodeValue);
                                printf("[AVT] avt_metadata->size = %s\r\n", avt_metadata->size);
                            }
                            else if (!strcmp(attr_node->nodeName, SERVICE_AVT_META_DURATION))
                            {
                                if (avt_metadata->duration)
                                {
                                    free(avt_metadata->duration);
                                }
                                avt_metadata->duration = strdup(attr_node->nodeValue);
                                printf("[AVT] avt_metadata->duration = %s\r\n", avt_metadata->duration);
                            }
                            else if (!strcmp(attr_node->nodeName, SERVICE_AVT_META_PROTOCOLINFO))
                            {
                                if (nShoutcast == 0) {
                                    if (avt_metadata->protocolInfo) {
                                        free(avt_metadata->protocolInfo);
                                    }
                                    avt_metadata->protocolInfo = strdup(attr_node->nodeValue);
                                    printf("[AVT] avt_metadata->protocolInfo = %s\r\n", avt_metadata->protocolInfo);
                                    if (strstr(avt_metadata->protocolInfo, SERVICE_AVT_MIME_AUDIO_MPEG) ||
                                        strstr(avt_metadata->protocolInfo, SERVICE_AVT_MIME_AUDIO_FLAC) ||
                                        strstr(avt_metadata->protocolInfo, SERVICE_AVT_MIME_AUDIO_XMSWMA) ||
                                        strstr(avt_metadata->protocolInfo, SERVICE_AVT_MIME_AUDIO_AAC) ||
                                        strstr(avt_metadata->protocolInfo, SERVICE_AVT_MIME_AUDIO_WAV) ||
                                        strstr(avt_metadata->protocolInfo, SERVICE_AVT_MIME_AUDIO_XWAV))
                                    {
                                        if (ut->uri) {
                                            free(ut->uri);
                                        }
                                        ut->uri = strdup(avt_metadata->res);
                                        printf("[AVT] Replace ut->uri to %s\r\n", ut->uri);
                                    }
                                } else if (nShoutcast == 1) {
                                    // now, keep 1st protocol info
                                    if (nShoutcastProtocol== 0){
                                        if (avt_metadata->protocolInfo) {
                                            free(avt_metadata->protocolInfo);
                                        }
                                        avt_metadata->protocolInfo = strdup(attr_node->nodeValue);
                                        printf("[AVT] avt_metadata->protocolInfo = %s\r\n", avt_metadata->protocolInfo);
                                        if (strstr(avt_metadata->protocolInfo, SERVICE_AVT_MIME_AUDIO_MPEG) ||
                                            strstr(avt_metadata->protocolInfo, SERVICE_AVT_MIME_AUDIO_FLAC) ||
                                            strstr(avt_metadata->protocolInfo, SERVICE_AVT_MIME_AUDIO_XMSWMA) ||
                                            strstr(avt_metadata->protocolInfo, SERVICE_AVT_MIME_AUDIO_AAC) ||
                                            strstr(avt_metadata->protocolInfo, SERVICE_AVT_MIME_AUDIO_WAV) ||
                                            strstr(avt_metadata->protocolInfo, SERVICE_AVT_MIME_AUDIO_XWAV))
                                        {
                                            if (ut->uri) {
                                                free(ut->uri);
                                            }
                                            ut->uri = strdup(avt_metadata->res);
                                            printf("[AVT] Replace ut->uri to %s\r\n", ut->uri);
                                        }
                                        nShoutcastProtocol++;
                                    }else {
                                        printf("[AVT] shoutcast by pass \n");
                                    }
                                }
                           }
                        }
                    }
                    ixmlNamedNodeMap_free(attr_map);
                    attr_map = NULL;
                }
            }
        }
    }

end:
    if (doc)
    {
        ixmlDocument_free(doc);
        doc = NULL;
    }

    return nRet;
}



static int
avt_set_uri (struct action_event_t *event)
{
    struct service_t *service;
    struct buffer_t *out = NULL;
    char udn[64];
    int res = 0;
    char *p = NULL;

    pthread_mutex_lock(&ut->render_mutex);
    printf("[AVT] %s() Begin %d \n", __FUNCTION__,gnSetURIStop);
    upnp_add_response (event, NULL, NULL);

    gnStopNotify = 0;
    if (gnSetURIStop==0){
        urender_cb.avt_stop();
    } else {
        gnSetURIStop = 0;
    }
    
    p = upnp_get_string(event->request, SERVICE_AVT_ARG_CUR_URI);
    if (strlen(p) == 0)
    {
        printf("[AVT]%s() L#%ld: uri == NULL!\r\n", __FUNCTION__, __LINE__);
        goto next_1;
    }

    if(ut->uri)
        free(ut->uri);
    ut->uri = p;
    printf("[AVT]%s() L#%ld: ut->uri=%s\r\n", __FUNCTION__, __LINE__, ut->uri);

next_1:
    p = upnp_get_string(event->request, SERVICE_AVT_ARG_CUR_URI_META_DATA);
    if (strlen(p) == 0)
    {
        printf("[AVT]%s() L#%ld: uri_metadata == NULL!\r\n", __FUNCTION__, __LINE__);
        AVT_FreeMetadata(&ut->avt.metadata);
        goto next_2;
    }

    if(ut->uri_metadata)
        free(ut->uri_metadata);
    ut->uri_metadata = p;
    printf("[AVT]%s() L#%ld: ut->uri_metadata=%s\r\n", __FUNCTION__, __LINE__, ut->uri_metadata);
    AVT_ParseURIMetadata(ut);

next_2:
    // For HomeDia, Pause and then change to next music, it does not stop first, so we needs to stop when set uri
    if (ut->avt.transport_state != AVT_TRANSPORTSTATE_STOPPED)
    {
        printf("[AVT] %s() set stop \n", __FUNCTION__);
    
        if (urender_cb.avt_stop)
        {
            res = urender_cb.avt_stop();
            ut->avt.transport_state = AVT_TRANSPORTSTATE_STOPPED;
        }
    }

    if (urender_cb.avt_setUri)
    {
        res = urender_cb.avt_setUri(ut->uri, &ut->avt.metadata);
    }
    
    service = get_service(SERVICE_AVT);
    sprintf (udn, "uuid:%s", ut->udn);

    out = buffer_new ();
    if (!out)
    {
        pthread_mutex_unlock(&ut->render_mutex);
        return 0;
    }

    lastchange_add_header (out);
    buffer_appendf (out, "&lt;%s val=\"%s\"/&gt;", SERVICE_AVT_VAR_URI, (ut->uri) ? ut->uri : "");
    //buffer_appendf (out, "&lt;%s val=\"%s\"/&gt;", SERVICE_AVT_VAR_URI_META_DATA, (ut->uri_metadata) ? ut->uri_metadata : "");
    //buffer_appendf (out, "&lt;%s val=\"%s\"/&gt;", SERVICE_AVT_VAR_URI, (ut->uri) ? "" :ut->uri);
    // for DLNA total meida player,HTC music
    buffer_appendf (out, "&lt;%s val=\"%s\"/&gt;", SERVICE_AVT_VAR_URI_META_DATA, (ut->uri_metadata) ? "" : "");
    
    lastchange_add_footer (out);

    avt_service_variables.var_str[0] = strdup(out->buf);
    buffer_free (out);

    // notify lastchange
    UpnpNotify(ut->dev,
               udn,
               service->id,
               (const char **)service->variables->var_name,
               (const char **)service->variables->var_str,
               service->variables->var_count);

    pthread_mutex_unlock(&ut->render_mutex);

    return event->status;
}

static int
avt_get_media_info (struct action_event_t *event)
{
    printf("[AVT] %s() Begin\n", __FUNCTION__);
    upnp_add_response (event, SERVICE_AVT_ARG_NR_TRACKS, "0");
    upnp_add_response (event, SERVICE_AVT_ARG_MEDIA_DURATION, (ut->avt.metadata.duration ? ut->avt.metadata.duration : "00:00:00"));
    upnp_add_response (event, SERVICE_AVT_ARG_CUR_URI, ut->uri);
    upnp_add_response (event, SERVICE_AVT_ARG_CUR_URI_META_DATA, ut->uri_metadata);
    upnp_add_response (event, SERVICE_AVT_ARG_NEXT_URI, SERVICE_AVT_NOT_IMPLEMENTED);
    upnp_add_response (event, SERVICE_AVT_ARG_NEXT_URI_META_DATA, SERVICE_AVT_NOT_IMPLEMENTED);
    upnp_add_response (event, SERVICE_AVT_ARG_PLAY_MEDIUM, SERVICE_AVT_NONE);
    upnp_add_response (event, SERVICE_AVT_ARG_REC_MEDIUM, SERVICE_AVT_NOT_IMPLEMENTED);
    upnp_add_response (event, SERVICE_AVT_ARG_WR_STATUS, SERVICE_AVT_NOT_IMPLEMENTED);

    return event->status;
}

static int
avt_get_ts_info (struct action_event_t *event)
{
    int res = 0;
    int play_state = 0;

    pthread_mutex_lock(&ut->render_mutex);
//    printf("[AVT] %s() Begin\n", __FUNCTION__);

    if (urender_cb.avt_get_transport_info)
    {
        res = urender_cb.avt_get_transport_info(&play_state);
        if (play_state != -1)
        {
            ut->avt.transport_state = play_state;
            ut->avt.transport_status = AVT_TRANSPORTSTATUS_OK;
        }
        else
        {
            ut->avt.transport_state = AVT_TRANSPORTSTATE_STOPPED;
            ut->avt.transport_status = AVT_TRANSPORTSTATUS_ERROR_OCCURRED;
        }
    }

//    printf("[AVT]%s() L#%ld: state=%s status=%s, speed=%s\r\n", __FUNCTION__, __LINE__, TransportState[ut->avt.transport_state], TransportStatus[ut->avt.transport_status], TransportPlaySpeed[ut->avt.play_speed]);
    upnp_add_response (event, SERVICE_AVT_ARG_CUR_TS_STATE, TransportState[ut->avt.transport_state]);
    upnp_add_response (event, SERVICE_AVT_ARG_CUR_TS_STATUS, TransportStatus[ut->avt.transport_status]);
    upnp_add_response (event, SERVICE_AVT_ARG_CUR_SPEED, TransportPlaySpeed[ut->avt.play_speed]);
    pthread_mutex_unlock(&ut->render_mutex);

    return event->status;
}

static int
avt_get_position_info (struct action_event_t *event)
{
    int res = 0;
    unsigned int duration = 0, curr_time = 0;
    unsigned int sec = 0, hh = 0, mm = 0, ss = 0;
    char str_dura[16], str_time[16],str_absTime[16];
    int play_state;
    struct service_t *service;
    struct buffer_t *out = NULL;
    char udn[64];

    pthread_mutex_lock(&ut->render_mutex);
//    printf("[AVT] %s() Begin\n", __FUNCTION__);

    memset(str_dura, 0, sizeof(str_dura));
    memset(str_time, 0, sizeof(str_time));
    if (urender_cb.avt_get_position_info) {
        res = urender_cb.avt_get_position_info(&duration, &curr_time);
        printf("[AVT]%s() L#%ld: duration=%ld, curr_time=%ld\r\n", __FUNCTION__, __LINE__, duration, curr_time);
        if (duration > 1000) {
            sec = duration / 1000;
            ss = sec % 60;
            mm = (sec / 60) % 60;
            hh = sec / 3600;
            sprintf(str_dura, "%02d:%02d:%02d", hh, mm, ss);
        } else {
            snprintf(str_dura, sizeof(str_dura), (ut->avt.metadata.duration ? ut->avt.metadata.duration : "00:00:00"));
        }
        sec = curr_time / 1000;
        ss = (sec % 60);
        mm = (sec / 60) % 60;
        hh = sec / 3600;
        sprintf(str_time, "%02d:%02d:%02d", hh, mm, ss);

//        printf("[AVT]%s() L#%ld: str_dura=%s, str_time=%s\r\n", __FUNCTION__, __LINE__, str_dura, str_time);
    }else {
        memcpy(str_dura, SERVICE_AVT_NOT_IMPLEMENTED, sizeof(SERVICE_AVT_NOT_IMPLEMENTED));
        memcpy(str_time, SERVICE_AVT_NOT_IMPLEMENTED, sizeof(SERVICE_AVT_NOT_IMPLEMENTED));
    }

    if (urender_cb.avt_get_transport_info) {
        res = urender_cb.avt_get_transport_info(&play_state);
        if (play_state != -1) {
            ut->avt.transport_state = play_state;
            ut->avt.transport_status = AVT_TRANSPORTSTATUS_OK;
        } else {
            ut->avt.transport_state = AVT_TRANSPORTSTATE_STOPPED;
            ut->avt.transport_status = AVT_TRANSPORTSTATUS_ERROR_OCCURRED;
        }
    }
    sec = 0, hh = 0, mm = 0, ss = 0;
    sscanf(ut->avt.metadata.duration , "%d:%d:%d", &hh, &mm, &ss);
    duration = (hh * 60 * 60) + (mm * 60) + ss;
    //printf(" duration=%d curr_time %d\r\n",duration,curr_time);

    if (play_state==AVT_TRANSPORTSTATE_STOPPED && (gnStopNotify >0 && gnStopNotify<=3) ) {        
        printf("[AVT] get posirion info statestop %d \n",gnStopNotify);
        gnStopNotify++;
        upnp_add_response (event, NULL, NULL);
        out = buffer_new ();
        if (!out) {
            pthread_mutex_unlock(&ut->render_mutex);
            return 0;
        }
        snprintf(str_absTime, sizeof(str_dura), "00:00:00");

        service = get_service(SERVICE_AVT);
        sprintf (udn, "uuid:%s", ut->udn);

        lastchange_add_header (out);
        buffer_appendf (out, "&lt;%s val=\"%s\"/&gt;", SERVICE_AVT_VAR_TS_STATE, TransportState[ut->avt.transport_state]);
        buffer_appendf (out, "&lt;%s val=\"%s\"/&gt;", SERVICE_AVT_VAR_TS_STATUS, TransportStatus[ut->avt.transport_status]);
        buffer_appendf (out, "&lt;%s val=\"%s\"/&gt;", SERVICE_AVT_ARG_TRACK, "1");        
        buffer_appendf (out, "&lt;%s val=\"%s\"/&gt;", SERVICE_AVT_ARG_TRACK_DURATION,str_absTime);        
        lastchange_add_footer (out);
        avt_service_variables.var_str[0] = strdup(out->buf);
        buffer_free (out);

        // notify lastchange
        UpnpNotify(ut->dev,
                   udn,
                   service->id,
                   (const char **)service->variables->var_name,
                   (const char **)service->variables->var_str,
                   service->variables->var_count);
    }else if (play_state==AVT_TRANSPORTSTATE_STOPPED) {
        gnStopNotify++;
        upnp_add_response (event, SERVICE_AVT_ARG_TRACK, "1");
        upnp_add_response (event, SERVICE_AVT_ARG_TRACK_DURATION, str_dura);
        upnp_add_response (event, SERVICE_AVT_ARG_TRACK_META_DATA, ut->uri_metadata);
        upnp_add_response (event, SERVICE_AVT_ARG_TRACK_URI, ut->uri);
        upnp_add_response (event, SERVICE_AVT_ARG_REL_TIME, str_dura);
        snprintf(str_absTime, sizeof(str_dura), "00:00:00");
        printf("[AVT]%s() L#%ld: str_dura=%s, str_time=%s\r\n", __FUNCTION__, __LINE__, str_dura, str_dura);        
        upnp_add_response (event, SERVICE_AVT_ARG_ABS_TIME, str_absTime);
        upnp_add_response (event, SERVICE_AVT_ARG_REL_CNT, "2147483647");
        upnp_add_response (event, SERVICE_AVT_ARG_ABS_CNT, "2147483647");
    } else {

        upnp_add_response (event, SERVICE_AVT_ARG_TRACK, "1");
        upnp_add_response (event, SERVICE_AVT_ARG_TRACK_DURATION, str_dura);
        upnp_add_response (event, SERVICE_AVT_ARG_TRACK_META_DATA, ut->uri_metadata);
        upnp_add_response (event, SERVICE_AVT_ARG_TRACK_URI, ut->uri);
        upnp_add_response (event, SERVICE_AVT_ARG_REL_TIME, str_time);
#if 0    
        upnp_add_response (event, SERVICE_AVT_ARG_ABS_TIME, str_time);
#else
        snprintf(str_absTime, sizeof(str_dura), "00:00:00");
        upnp_add_response (event, SERVICE_AVT_ARG_ABS_TIME, str_absTime);
#endif
        upnp_add_response (event, SERVICE_AVT_ARG_REL_CNT, "2147483647");
        upnp_add_response (event, SERVICE_AVT_ARG_ABS_CNT, "2147483647");
    }
    pthread_mutex_unlock(&ut->render_mutex);

    return event->status;
}

static int
avt_get_dev_caps (struct action_event_t *event)
{
    struct buffer_t *out = NULL;

    printf("[AVT] %s() Begin\n", __FUNCTION__);
    out = buffer_new ();
    if (!out)
        return 0;

    buffer_appendf (out, "%s,%s,%s,%s,%s", SERVICE_AVT_NONE, SERVICE_AVT_NETWORK, SERVICE_AVT_HDD, SERVICE_AVT_CD_DA, SERVICE_AVT_UNKNOWN);
    upnp_add_response (event, SERVICE_AVT_ARG_PLAY_MEDIA, out->buf);
    buffer_free (out);
    upnp_add_response (event, SERVICE_AVT_ARG_REC_MEDIA, SERVICE_AVT_NOT_IMPLEMENTED);
    upnp_add_response (event, SERVICE_AVT_ARG_REC_QUAL_MODES, SERVICE_AVT_NOT_IMPLEMENTED);

    return event->status;
}

static int
avt_get_ts_setting (struct action_event_t *event)
{
    printf("[AVT] %s() Begin\n", __FUNCTION__);
    upnp_add_response (event, SERVICE_AVT_ARG_PLAY_MODE, CurrentPlayMode[ut->avt.play_mode]);
    upnp_add_response (event, SERVICE_AVT_ARG_REC_QUAL_MODE, SERVICE_AVT_NOT_IMPLEMENTED);

    return event->status;
}

static int
avt_stop (struct action_event_t *event)
{
    struct service_t *service;
    struct buffer_t *out = NULL;
    char udn[64];
    int res = 0;

    pthread_mutex_lock(&ut->render_mutex);
    printf("[AVT] %s() Begin\n", __FUNCTION__);
    gnSetURIStop = 1;
    
    upnp_add_response (event, NULL, NULL);

    if (urender_cb.avt_stop)
    {
        res = urender_cb.avt_stop();
        ut->avt.transport_state = AVT_TRANSPORTSTATE_STOPPED;
        ut->avt.transport_status = AVT_TRANSPORTSTATUS_OK;
    }
    else
    {
        res = -1;
        ut->avt.transport_status = AVT_TRANSPORTSTATUS_ERROR_OCCURRED;
    }

    out = buffer_new ();
    if (!out)
    {
        pthread_mutex_unlock(&ut->render_mutex);
        return 0;
    }

    service = get_service(SERVICE_AVT);
    sprintf (udn, "uuid:%s", ut->udn);

    lastchange_add_header (out);
    buffer_appendf (out, "&lt;%s val=\"%s\"/&gt;", SERVICE_AVT_VAR_TS_STATE, TransportState[ut->avt.transport_state]);
    buffer_appendf (out, "&lt;%s val=\"%s\"/&gt;", SERVICE_AVT_VAR_TS_STATUS, TransportStatus[ut->avt.transport_status]);
    lastchange_add_footer (out);
    avt_service_variables.var_str[0] = strdup(out->buf);
    buffer_free (out);

    // notify lastchange
    UpnpNotify(ut->dev,
               udn,
               service->id,
               (const char **)service->variables->var_name,
               (const char **)service->variables->var_str,
               service->variables->var_count);

    pthread_mutex_unlock(&ut->render_mutex);

    return event->status;
}

static int
avt_play (struct action_event_t *event)
{
    struct service_t *service;
    struct buffer_t *out = NULL;
    char udn[64];
    int res = 0;
    int play_state = 0;

    pthread_mutex_lock(&ut->render_mutex);
    printf("[AVT] %s() Begin\n", __FUNCTION__);
    ut->avt.play_speed = upnp_get_ui4 (event->request, SERVICE_AVT_ARG_SPEED);
    printf("[AVT]%s() L#%ld: ut->avt.play_speed=%ld\r\n", __FUNCTION__, __LINE__, ut->avt.play_speed);
    upnp_add_response (event, NULL, NULL);
    printf("[AVT]%s() L#%ld: ut->uri=%s\r\n", __FUNCTION__, __LINE__, ut->uri);

    // Check the status of player before playing
    if (urender_cb.avt_get_transport_info)
    {
        res = urender_cb.avt_get_transport_info(&play_state);
        if (play_state != -1)
        {
            ut->avt.transport_state = play_state;
        }
        else
        {
            ut->avt.transport_state = AVT_TRANSPORTSTATE_STOPPED;
        }
    }
    
    if (urender_cb.avt_play)
    {
        if (ut->avt.transport_state == AVT_TRANSPORTSTATE_PAUSED_PLAYBACK)
        {
            // play from the paused point
            res = urender_cb.avt_play(NULL, NULL);
        }
        else
        {
            // play from beginning
            res = urender_cb.avt_play(ut->uri, &ut->avt.metadata);
        }
        ut->avt.transport_state = AVT_TRANSPORTSTATE_PLAYING;
        ut->avt.transport_status = AVT_TRANSPORTSTATUS_OK;
    }
    else
    {
        res = -1;
        ut->avt.transport_state = AVT_TRANSPORTSTATE_STOPPED;
        ut->avt.transport_status = AVT_TRANSPORTSTATUS_ERROR_OCCURRED;
    }

    out = buffer_new ();
    if (!out)
    {
        pthread_mutex_unlock(&ut->render_mutex);
        return 0;
    }

    service = get_service(SERVICE_AVT);
    sprintf (udn, "uuid:%s", ut->udn);

    lastchange_add_header (out);
    buffer_appendf (out, "&lt;%s val=\"%s\"/&gt;", SERVICE_AVT_VAR_TS_STATE, TransportState[ut->avt.transport_state]);
    buffer_appendf (out, "&lt;%s val=\"%s\"/&gt;", SERVICE_AVT_VAR_TS_STATUS, TransportStatus[ut->avt.transport_status]);
    lastchange_add_footer (out);
    avt_service_variables.var_str[0] = strdup(out->buf);
    buffer_free (out);

    // notify lastchange
    UpnpNotify(ut->dev,
               udn,
               service->id,
               (const char **)service->variables->var_name,
               (const char **)service->variables->var_str,
               service->variables->var_count);

    pthread_mutex_unlock(&ut->render_mutex);

exit:
    return event->status;
}

static int
avt_pause (struct action_event_t *event)
{
    struct service_t *service;
    struct buffer_t *out = NULL;
    char udn[64];
    int res = 0;

    pthread_mutex_lock(&ut->render_mutex);
    printf("[AVT] %s() Begin\n", __FUNCTION__);
    upnp_add_response (event, NULL, NULL);

    if (ut->avt.transport_state != AVT_TRANSPORTSTATE_PLAYING)
    {
        goto skip;
    }

    if (urender_cb.avt_pause)
    {
        res = urender_cb.avt_pause();
        ut->avt.transport_state = AVT_TRANSPORTSTATE_PAUSED_PLAYBACK;
        ut->avt.transport_status = AVT_TRANSPORTSTATUS_OK;
    }
    else
    {
        res = -1;
        ut->avt.transport_status = AVT_TRANSPORTSTATUS_ERROR_OCCURRED;
    }

skip:
    out = buffer_new ();
    if (!out)
    {
        pthread_mutex_unlock(&ut->render_mutex);
        return 0;
    }

    service = get_service(SERVICE_AVT);
    sprintf (udn, "uuid:%s", ut->udn);

    lastchange_add_header (out);
    buffer_appendf (out, "&lt;%s val=\"%s\"/&gt;", SERVICE_AVT_VAR_TS_STATE, TransportState[ut->avt.transport_state]);
    buffer_appendf (out, "&lt;%s val=\"%s\"/&gt;", SERVICE_AVT_VAR_TS_STATUS, TransportStatus[ut->avt.transport_status]);
    lastchange_add_footer (out);
    avt_service_variables.var_str[0] = strdup(out->buf);
    buffer_free (out);

    // notify lastchange
    UpnpNotify(ut->dev,
               udn,
               service->id,
               (const char **)service->variables->var_name,
               (const char **)service->variables->var_str,
               service->variables->var_count);

    pthread_mutex_unlock(&ut->render_mutex);

    return event->status;
}

static int
avt_record (struct action_event_t *event)
{
    printf("[AVT] %s() Begin\n", __FUNCTION__);
    upnp_add_response (event, NULL, NULL);
    return event->status;
}

static int
avt_seek (struct action_event_t *event)
{
    int res = 0;
    char *iid, *unit, *target;

    pthread_mutex_lock(&ut->render_mutex);
    printf("[AVT] %s() Begin\n", __FUNCTION__);
    upnp_add_response (event, NULL, NULL);
/*
    if (ut->avt.transport_state != AVT_TRANSPORTSTATE_PLAYING &&
        ut->avt.transport_state != AVT_TRANSPORTSTATE_PAUSED_PLAYBACK)
    {
        goto skip;
    }
*/
    iid    = upnp_get_string(event->request, AVT_INSTANCE_ID);
    unit   = upnp_get_string(event->request, SERVICE_AVT_ARG_UNIT);
    target = upnp_get_string(event->request, SERVICE_AVT_ARG_TARGET);

    printf("[AVT]%s() L#%ld: iid = %s, unit = %s, target = %s\r\n", __FUNCTION__, __LINE__, iid, unit, target);

    if (urender_cb.avt_seek)
    {
        res = urender_cb.avt_seek(unit, target);
    }

skip:
    pthread_mutex_unlock(&ut->render_mutex);

    return event->status;
}

static int
avt_next (struct action_event_t *event)
{
    printf("[AVT] %s() Begin\n", __FUNCTION__);
    upnp_add_response (event, NULL, NULL);
    return event->status;
}

static int
avt_previous (struct action_event_t *event)
{
    printf("[AVT] %s() Begin\n", __FUNCTION__);
    upnp_add_response (event, NULL, NULL);
    return event->status;
}

static int
avt_set_play_mode (struct action_event_t *event)
{
    printf("[AVT] %s() Begin\n", __FUNCTION__);
    ut->avt.play_mode = upnp_get_ui4 (event->request, SERVICE_AVT_ARG_NEW_PLAY_MODE);
    upnp_add_response (event, NULL, NULL);

    return event->status;
}

static int
avt_get_cur_ts_act (struct action_event_t *event)
{
    struct buffer_t *out = NULL;

    printf("[AVT] %s() Begin\n", __FUNCTION__);
    out = buffer_new ();
    if (!out)
        return 0;

    buffer_appendf (out, "%s, %s, %s, %s, %s, %s", SERVICE_AVT_ACTION_PLAY,
                                              SERVICE_AVT_ACTION_PAUSE,
                                              SERVICE_AVT_ACTION_STOP,
                                              SERVICE_AVT_ACTION_SEEK,
                                              SERVICE_AVT_ACTION_NEXT,
                                              SERVICE_AVT_ACTION_PREVIOUS);

    upnp_add_response (event, SERVICE_AVT_ARG_ACTS, out->buf);
    buffer_free (out);

    return event->status;
}

/* List of UPnP AVTransport Service actions */
struct service_action_t avt_service_actions[] = {
    { SERVICE_AVT_ACTION_SET_URI, avt_set_uri },
    { SERVICE_AVT_ACTION_GET_MEDIA_INFO, avt_get_media_info },
    { SERVICE_AVT_ACTION_GET_TS_INFO, avt_get_ts_info },
    { SERVICE_AVT_ACTION_GET_POS_INFO, avt_get_position_info },
    { SERVICE_AVT_ACTION_GET_DEV_CAPS, avt_get_dev_caps },
    { SERVICE_AVT_ACTION_GET_TS_SETTING, avt_get_ts_setting },
    { SERVICE_AVT_ACTION_STOP, avt_stop },
    { SERVICE_AVT_ACTION_PLAY, avt_play },
    { SERVICE_AVT_ACTION_PAUSE, avt_pause },
    { SERVICE_AVT_ACTION_RECORD, avt_record },
    { SERVICE_AVT_ACTION_SEEK, avt_seek },
    { SERVICE_AVT_ACTION_NEXT, avt_next },
    { SERVICE_AVT_ACTION_PREVIOUS, avt_previous },
    { SERVICE_AVT_ACTION_SET_PLAY_MODE, avt_set_play_mode },
    { SERVICE_AVT_ACTION_GET_CUR_TS_ACT, avt_get_cur_ts_act },
    { NULL, NULL }
};


