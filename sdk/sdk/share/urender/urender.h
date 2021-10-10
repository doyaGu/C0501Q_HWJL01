/*
 * ushare.h : GeeXboX uShare UPnP Media Server header.
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

#ifndef _USHARE_H_
#define _USHARE_H_

#include "upnp/upnp.h"
#include "upnp/upnptools.h"
//#include <stdbool.h>
#include "pthread.h"
#include "ite/ith.h"

#ifdef HAVE_DLNA
#include <dlna.h>
#endif /* HAVE_DLNA */

#include "avt.h"
#include "rcs.h"

#define VIRTUAL_DIR "/web"
#define XBOX_MODEL_NAME "Windows Media Connect Compatible"
#define DEFAULT_UUID "898f9738-d930-4db4-a3cf"

#define UPNP_MAX_CONTENT_LENGTH 4096*3

#define STARTING_ENTRY_ID_DEFAULT 0
#define STARTING_ENTRY_ID_XBOX360 100000

#ifndef PATH_MAX
    #define PATH_MAX 512
#endif


#define UPNP_CLASS_AUDIOITEM                    "object.item.audioItem"
#define UPNP_CLASS_AUDIOITEM_MUSICTRACK         "object.item.audioItem.musicTrack"
#define UPNP_CLASS_AUDIOITEM_AUDIOBROADCAST     "object.item.audioItem.audioBroadcast"
#define UPNP_CLASS_AUDIOITEM_AUDIOBOOK          "object.item.audioItem.audioBook"
#define UPNP_CLASS_VIDEOITEM                    "object.item.videoItem"
#define UPNP_CLASS_VIDEOITEM_MOVIE              "object.item.videoItem.movie"
#define UPNP_CLASS_VIDEOITEM_VIDEOBROADCAST     "object.item.videoItem.videoBroadcast"
#define UPNP_CLASS_VIDEOITEM_MUSICVIDEOCLIP     "object.item.videoItem.musicVideoClip"
#define UPNP_CLASS_IMAGEITEM                    "object.item.imageItem"
#define UPNP_CLASS_IMAGEITEM_PHOTO              "object.item.imageItem.photo"
#define UPNP_CLASS_PLAYLISTITEM                 "object.item.playlistItem"
#define UPNP_CLASS_TEXTITEM                     "object.item.textItem"


#define UPNP_DESCRIPTION \
"<?xml version=\"1.0\" encoding=\"utf-8\"?>" \
"<root xmlns=\"urn:schemas-upnp-org:device-1-0\" xmlns:dlna=\"urn:schemas-dlna-org:device-1-0\">" \
    "<specVersion>" \
        "<major>1</major>" \
        "<minor>0</minor>" \
    "</specVersion>" \
    "<device>" \
        "<deviceType>urn:schemas-upnp-org:device:MediaRenderer:1</deviceType>" \
        "<friendlyName>%s</friendlyName>" \
        "<manufacturer>%s</manufacturer>" \
        "<manufacturerURL>%s</manufacturerURL>" \
        "<modelDescription>%s</modelDescription>" \
        "<modelName>%s</modelName>" \
        "<modelNumber>%s</modelNumber>" \
        "<modelURL>%s</modelURL>" \
        "<serialNumber>%s</serialNumber>" \
        "<UDN>uuid:%s</UDN>" \
        "<dlna:X_DLNADOC xmlns:dlna=\"urn:schemas-dlna-org:device-1-0\">DMR-1.50</dlna:X_DLNADOC>" \
        "<serviceList>" \
            "<service>" \
                "<serviceType>urn:schemas-upnp-org:service:AVTransport:1</serviceType>" \
                "<serviceId>urn:upnp-org:serviceId:AVTransport</serviceId>" \
                "<SCPDURL>/web/avt.xml</SCPDURL>" \
                "<controlURL>/web/avt_control.xml</controlURL>" \
                "<eventSubURL>/web/avt_event.xml</eventSubURL>" \
            "</service>" \
            "<service>" \
                "<serviceType>urn:schemas-upnp-org:service:ConnectionManager:1</serviceType>" \
                "<serviceId>urn:upnp-org:serviceId:ConnectionManager</serviceId>" \
                "<SCPDURL>/web/cms.xml</SCPDURL>" \
                "<controlURL>/web/cms_control.xml</controlURL>" \
                "<eventSubURL>/web/cms_event.xml</eventSubURL>" \
            "</service>" \
            "<service>" \
                "<serviceType>urn:schemas-upnp-org:service:RenderingControl:1</serviceType>" \
                "<serviceId>urn:upnp-org:serviceId:RenderingControl</serviceId>" \
                "<SCPDURL>/web/rcs.xml</SCPDURL>" \
                "<controlURL>/web/rcs_control.xml</controlURL>" \
                "<eventSubURL>/web/rcs_event.xml</eventSubURL>" \
            "</service>" \
        "</serviceList>" \
        "<presentationURL>/web/info.html</presentationURL>" \
    "</device>" \
"</root>"


#define DESCRIPTION_LOCATION "/description.xml"

#define URENDER_ANDROID_DEVICE_IP_MAXLEN     256


struct urender_t {
  char *name;
  char *interface_;
  char *model_name;
  int starting_id;
  int init;
  UpnpDevice_Handle dev;
  char *udn;
  char *uri;
  char *uri_metadata;
  char *ip;
  unsigned short port;
  struct buffer_t *presentation;
  int use_presentation;
#ifdef HAVE_DLNA
  int dlna_enabled;
  dlna_t *dlna;
  dlna_org_flags_t dlna_flags;
#endif /* HAVE_DLNA */
  int verbose;
  int override_iconv_err;
  char *cfg_file;
  pthread_mutex_t termination_mutex;
  pthread_cond_t termination_cond;
  pthread_mutex_t render_mutex;

  struct avt_t avt;
  struct rcs_t rcs;

  bool reboot;
  bool upgrading;
};

struct action_event_t {
  struct Upnp_Action_Request *request;
  int status;
  struct service_t *service;
};



typedef struct urender_callback_t
{
    int (*avt_setUri)(const char *uri, const AVT_METADATA *metadata);
    int (*avt_play)(const char *uri, const AVT_METADATA *metadata);
    int (*avt_stop)(void);
    int (*avt_pause)(void);
    int (*avt_get_position_info)(unsigned int *duration, unsigned int *curr_time);
    int (*avt_get_transport_info)(int *play_state);
    int (*avt_seek)(const char *unit, const char *target);
    int (*rcs_get_volume)(int *volume);
    int (*rcs_set_volume)(const int volume);
    int (*rcs_get_mute)(int *mute);
    int (*rcs_set_mute)(const int mute);
} urender_callback;



int urender_main (int argc, char **argv);
int urender_set_callback(urender_callback *cb);
int urender_terminate(void);
int urender_reboot(void);
char *urender_get_description(void);
int urender_get_description_len(void);


#endif /* _USHARE_H_ */
