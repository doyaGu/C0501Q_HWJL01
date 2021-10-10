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

/* Represent the RCS GetMute action. */
#define SERVICE_RCS_ACTION_GET_MUTE "GetMute"

/* Represent the RCS GetVolume action. */
#define SERVICE_RCS_ACTION_GET_VOLUME "GetVolume"

/* Represent the RCS GetVolumeDB action. */
#define SERVICE_RCS_ACTION_GET_VOLUME_DB "GetVolumeDB"

/* Represent the RCS GetVolumeDBRange action. */
#define SERVICE_RCS_ACTION_GET_VOLUME_DB_RANGE "GetVolumeDBRange"

/* Represent the RCS ListPresets action. */
#define SERVICE_RCS_ACTION_LIST_PRESETS "ListPresets"

/* Represent the RCS SelectPreset action. */
#define SERVICE_RCS_ACTION_SELECT_PRESET "SelectPreset"

/* Represent the RCS SetMute action. */
#define SERVICE_RCS_ACTION_SET_MUTE "SetMute"

/* Represent the RCS SetVolume action. */
#define SERVICE_RCS_ACTION_SET_VOLUME "SetVolume"


/* Represent the RCS LastChange argument. */
#define SERVICE_RCS_ARG_LASTCHANGE "LastChange"

/* Represent the RCS PresetNameList argument. */
#define SERVICE_RCS_ARG_PRESET_NAME_LIST "PresetNameList"

/* Represent the RCS Volume argument. */
#define SERVICE_RCS_ARG_VOLUME "Volume"

/* Represent the RCS CurrentVolume argument. */
#define SERVICE_RCS_ARG_CURRENT_VOLUME "CurrentVolume"

/* Represent the RCS DesiredVolume argument. */
#define SERVICE_RCS_ARG_DESIRED_VOLUME "DesiredVolume"

/* Represent the RCS VolumeDB argument. */
#define SERVICE_RCS_ARG_VOLUME_DB "VolumeDB"

/* Represent the RCS Mute argument. */
#define SERVICE_RCS_ARG_MUTE "Mute"

/* Represent the RCS CurrentMute argument. */
#define SERVICE_RCS_ARG_CURRENT_MUTE "CurrentMute"

/* Represent the RCS DesiredMute argument. */
#define SERVICE_RCS_ARG_DESIRED_MUTE "DesiredMute"

/* Represent the RCS A_ARG_TYPE_InstanceID argument. */
#define SERVICE_RCS_ARG_INSTANCE_ID "A_ARG_TYPE_InstanceID"


#define RCS_EVENT "Event"
#define RCS_NAMESPACE "xmlns=\"urn:schemas-upnp-org:metadata-1-0/RCS/\""
#define RCS_INSTANCE_ID "InstanceID"

/* Represent the LastChange service variable. */
#define SERVICE_VAR_LASTCHANGE "LastChange"

#define SERVICE_RCS_VOLUME_MAX 100
#define SERVICE_RCS_VOLUME_MIN 0


extern struct urender_t *ut;
extern urender_callback urender_cb;

static int rcs_var_init (void);

int rcs_notify_volume();

/* List of UPnP AVTransport Service variables */
struct service_variables_t rcs_service_variables = {
    {SERVICE_VAR_LASTCHANGE, NULL, NULL},
    {NULL, NULL, NULL},
    1,
    rcs_var_init,
};

static void
lastchange_add_header_new (struct buffer_t *out)
{
//    buffer_appendf (out, "&lt;?xml version=\"1.0\" encoding=\"UTF-8\"?&gt;");
    buffer_appendf (out, "&lt;%s %s&gt;", RCS_EVENT, RCS_NAMESPACE);
    buffer_appendf (out, "&lt;%s val=\"%s\"&gt;", RCS_INSTANCE_ID, "0");
}

static void
lastchange_add_footer_new (struct buffer_t *out)
{
    buffer_appendf (out, "&lt;/%s&gt;", RCS_INSTANCE_ID);
    buffer_appendf (out, "&lt;/%s&gt;", RCS_EVENT);
}

static void
lastchange_add_header (struct buffer_t *out)
{
    buffer_appendf (out, "<%s %s>", RCS_EVENT, RCS_NAMESPACE);
    buffer_appendf (out, "<%s val=\"%s\"/>", RCS_INSTANCE_ID, "0");
}


static void
lastchange_add_footer (struct buffer_t *out)
{
    buffer_appendf (out, "</%s>", RCS_INSTANCE_ID);
    buffer_appendf (out, "</%s>", RCS_EVENT);
}

static rcs_check_volume_validation(void)
{
    if (ut->rcs.volume > SERVICE_RCS_VOLUME_MAX)
    {
        ut->rcs.volume = SERVICE_RCS_VOLUME_MAX;
    }
    if (ut->rcs.volume < SERVICE_RCS_VOLUME_MIN)
    {
        ut->rcs.volume = SERVICE_RCS_VOLUME_MIN;
    }
}


static int
rcs_get_mute (struct action_event_t *event)
{
    pthread_mutex_lock(&ut->render_mutex);
    printf("[RCS] %s() Begin\n", __FUNCTION__);
    if (urender_cb.rcs_get_mute)
    {
        urender_cb.rcs_get_mute(&ut->rcs.mute);
    }
    upnp_add_response (event, SERVICE_RCS_ARG_CURRENT_MUTE, (ut->rcs.mute) ? "1" : "0");
    pthread_mutex_unlock(&ut->render_mutex);
    return event->status;
}


static int
rcs_set_mute (struct action_event_t *event)
{
    char *mute_str = NULL;

    char vol[8];
    struct service_t *service;
    struct buffer_t *out = NULL;
    char udn[64];
    int nResult = 0;

    pthread_mutex_lock(&ut->render_mutex);
    printf("[RCS] %s() Begin\n", __FUNCTION__);
    mute_str = upnp_get_string(event->request, SERVICE_RCS_ARG_DESIRED_MUTE);
    printf("[RCS]%s() L#%ld: %s=%s\r\n", __FUNCTION__, __LINE__, SERVICE_RCS_ARG_DESIRED_MUTE, mute_str);
    if (mute_str && strlen(mute_str) > 0)
    {
        ut->rcs.mute = atoi(mute_str);
        if (urender_cb.rcs_set_mute)
        {
            urender_cb.rcs_set_mute(ut->rcs.mute);
        }
        free(mute_str);
    }    
    upnp_add_response (event, NULL, NULL);
    service = get_service(SERVICE_RCS);
    sprintf (udn, "uuid:%s", ut->udn);

    out = buffer_new ();
    snprintf(vol, sizeof(vol), "%ld", ut->rcs.mute);

    lastchange_add_header_new (out);
    buffer_appendf (out, "&lt;%s Channel=\"Master\" val=\"%s\"/&gt;", SERVICE_RCS_ARG_MUTE, vol);
    
    lastchange_add_footer_new (out);

    rcs_service_variables.var_str[0] = strdup(out->buf);
    buffer_free (out);

    // notify lastchange
    nResult = UpnpNotify(ut->dev,
               udn,
               service->id,
               (const char **)service->variables->var_name,
               (const char **)service->variables->var_str,
               service->variables->var_count);

    //printf("rcs notify  %s %d\n",vol,nResult);

    pthread_mutex_unlock(&ut->render_mutex);
    return event->status;
}

static int
rcs_get_volume (struct action_event_t *event)
{
    char vol[8];

    pthread_mutex_lock(&ut->render_mutex);
    printf("[RCS] %s() Begin\n", __FUNCTION__);
    if (urender_cb.rcs_get_volume)
    {
        urender_cb.rcs_get_volume(&ut->rcs.volume);
    }
    rcs_check_volume_validation();
    snprintf(vol, sizeof(vol), "%ld", ut->rcs.volume);
    printf("[RCS]%s() L#%ld: ut->rcs.volume=%ld\r\n", __FUNCTION__, __LINE__, ut->rcs.volume);
    upnp_add_response (event, SERVICE_RCS_ARG_CURRENT_VOLUME, vol);
    pthread_mutex_unlock(&ut->render_mutex);
    return event->status;
}

static int
rcs_set_volume (struct action_event_t *event)
{
    char *volume_str = NULL;
    char vol[8];
    struct service_t *service;
    struct buffer_t *out = NULL;
    char udn[64];
    int nResult = 0;

    pthread_mutex_lock(&ut->render_mutex);
    printf("[RCS] %s() Begin\n", __FUNCTION__);
    volume_str = upnp_get_string(event->request, SERVICE_RCS_ARG_DESIRED_VOLUME);
    printf("[RCS]%s() L#%ld: %s=%s\r\n", __FUNCTION__, __LINE__, SERVICE_RCS_ARG_DESIRED_VOLUME, volume_str);
    if (volume_str && strlen(volume_str) > 0)
    {
        ut->rcs.volume = atoi(volume_str);
        rcs_check_volume_validation();
        if (urender_cb.rcs_set_volume)
        {
            urender_cb.rcs_set_volume(ut->rcs.volume);
        }
        free(volume_str);
    }
    upnp_add_response (event, NULL, NULL);

    service = get_service(SERVICE_RCS);
    sprintf (udn, "uuid:%s", ut->udn);

    out = buffer_new ();
    snprintf(vol, sizeof(vol), "%ld", ut->rcs.volume);
    ut->rcs.volumeDB = (15206*ut->rcs.volume)/100;

    lastchange_add_header_new (out);
    buffer_appendf (out, "&lt;%s Channel=\"Master\" val=\"%s\"/&gt;", SERVICE_RCS_ARG_VOLUME, vol);
    buffer_appendf (out, "&lt;%s Channel=\"Master\" val=\"%d\"/&gt;", SERVICE_RCS_ARG_VOLUME_DB, ut->rcs.volumeDB);
    
    lastchange_add_footer_new (out);

    rcs_service_variables.var_str[0] = strdup(out->buf);
    buffer_free (out);

    // notify lastchange
    nResult = UpnpNotify(ut->dev,
               udn,
               service->id,
               (const char **)service->variables->var_name,
               (const char **)service->variables->var_str,
               service->variables->var_count);

    printf("rcs notify %d %s %d\n",ut->rcs.volume,vol,nResult);
    
    pthread_mutex_unlock(&ut->render_mutex);
    return event->status;
}


static int
rcs_get_volume_db (struct action_event_t *event)
{
    printf("[RCS] %s() Begin\n", __FUNCTION__);
    upnp_add_response (event, SERVICE_RCS_ARG_VOLUME_DB, "1");
    return event->status;
}


static int
rcs_get_volume_db_range (struct action_event_t *event)
{
    printf("[RCS] %s() Begin\n", __FUNCTION__);
    upnp_add_response (event, SERVICE_RCS_ARG_VOLUME_DB, "1");
    return event->status;
}


static int
rcs_list_presets (struct action_event_t *event)
{
    printf("[RCS] %s() Begin\n", __FUNCTION__);
    upnp_add_response (event, NULL, NULL);
    return event->status;
}


static int
rcs_select_preset (struct action_event_t *event)
{
    printf("[RCS] %s() Begin\n", __FUNCTION__);
    upnp_add_response (event, NULL, NULL);
    return event->status;
}


/* List of UPnP AVTransport Service actions */
struct service_action_t rcs_service_actions[] = {
    { SERVICE_RCS_ACTION_GET_MUTE, rcs_get_mute },
    { SERVICE_RCS_ACTION_GET_VOLUME, rcs_get_volume },
    { SERVICE_RCS_ACTION_GET_VOLUME_DB, rcs_get_volume_db },
    { SERVICE_RCS_ACTION_GET_VOLUME_DB_RANGE, rcs_get_volume_db_range },
    { SERVICE_RCS_ACTION_LIST_PRESETS, rcs_list_presets },
    { SERVICE_RCS_ACTION_SELECT_PRESET, rcs_select_preset },
    { SERVICE_RCS_ACTION_SET_MUTE, rcs_set_mute },
    { SERVICE_RCS_ACTION_SET_VOLUME, rcs_set_volume },
    { NULL, NULL }
};

int rcs_notify_volume()
{
#if 0
    rcs_var_init();    
#else
    char *volume_str = NULL;
    char vol[8];
    struct service_t *service;
    struct buffer_t *out = NULL;
    char udn[64];

    pthread_mutex_lock(&ut->render_mutex);
    printf("[RCS] %s() Begin\n", __FUNCTION__);


    service = get_service(SERVICE_RCS);
    sprintf (udn, "uuid:%s", ut->udn);

    out = buffer_new ();
    snprintf(vol, sizeof(vol), "%ld", ut->rcs.volume);

    lastchange_add_header_new (out);
    buffer_appendf (out, "&lt;%s Channel=\"Master\" val=\"%s\"/&gt;", SERVICE_RCS_ARG_VOLUME, "100");
    buffer_appendf (out, "&lt;%s Channel=\"Master\" val=\"%d\"/&gt;", SERVICE_RCS_ARG_MUTE, ut->rcs.mute);
    buffer_appendf (out, "&lt;%s val=\"%s\"/&gt;", SERVICE_RCS_ARG_PRESET_NAME_LIST, "FactoryDefaults");    
    buffer_appendf (out, "&lt;%s Channel=\"Master\" val=\"%s\"/&gt;", SERVICE_RCS_ARG_VOLUME_DB, "0");
    
    lastchange_add_footer_new (out);

    rcs_service_variables.var_str[0] = strdup(out->buf);
    buffer_free (out);
printf("rcs notify %d %s\n",ut->rcs.volume,vol);
    // notify lastchange
    UpnpNotify(ut->dev,
               udn,
               service->id,
               (const char **)service->variables->var_name,
               (const char **)service->variables->var_str,
               service->variables->var_count);

    pthread_mutex_unlock(&ut->render_mutex);
#endif

}


static int
rcs_var_init (void)
{
    struct buffer_t *out = NULL;
    char vol[8];

    out = buffer_new ();
    if (!out)
        return 0;

    if (urender_cb.rcs_get_volume)
    {
        urender_cb.rcs_get_volume(&ut->rcs.volume);
    }
    else
    {
        ut->rcs.volume = 100;
    }
    printf("[RCS]%s() L#%ld: ut->rcs.volume=%ld\r\n", __FUNCTION__, __LINE__, ut->rcs.volume);
    if (urender_cb.rcs_get_mute)
    {
        urender_cb.rcs_get_mute(&ut->rcs.mute);
    }
    else
    {
        ut->rcs.mute = 0;
    }
    printf("[RCS]%s() L#%ld: ut->rcs.mute=%ld\r\n", __FUNCTION__, __LINE__, ut->rcs.mute);
    //ut->rcs.volumeDB = 15206;
    ut->rcs.volumeDB = (15206*ut->rcs.volume)/100;
    snprintf(vol, sizeof(vol), "%ld", ut->rcs.volume);

    lastchange_add_header (out);
    buffer_appendf (out, "<%s Channel=\"Master\" val=\"%s\"/>", SERVICE_RCS_ARG_VOLUME, vol);
    buffer_appendf (out, "<%s Channel=\"Master\" val=\"%d\"/>", SERVICE_RCS_ARG_MUTE, ut->rcs.mute);
    buffer_appendf (out, "<%s val=\"%s\"/>", SERVICE_RCS_ARG_PRESET_NAME_LIST, "FactoryDefaults");
    buffer_appendf (out, "<%s Channel=\"Master\" val=\"%s\"/>", SERVICE_RCS_ARG_VOLUME_DB, "0");
    lastchange_add_footer (out);

    rcs_service_variables.var_str[0] = strdup(out->buf);
    buffer_free (out);

    return 1;
}

