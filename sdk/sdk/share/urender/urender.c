/*
 * ushare.c : GeeXboX uShare UPnP Media Server.
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

#include <stdio.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
//#include <unistd.h>
#include <errno.h>
//#include <getopt.h>

#if (defined(BSD) || defined(__FreeBSD__) || defined(__APPLE__))
#include <sys/socket.h>
#include <sys/sysctl.h>
#include <net/if_dl.h>
#endif

#if (defined(__APPLE__))
#include <net/route.h>
#endif

//#include <net/if.h>
//#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
//#include <stdbool.h>
#include <fcntl.h>

#ifdef HAVE_IFADDRS_H
#include <ifaddrs.h>
#endif

#if (defined(__unix__) || defined(unix)) && !defined(USG)
#include <sys/param.h>
#endif


#if (defined(HAVE_SETLOCALE) && defined(CONFIG_NLS))
# include <locale.h>
#endif

#include "config.h"
#include "urender.h"
#include "services.h"
#include "util_iconv.h"
#include "cfgparser.h"
#include "gettext.h"
#include "trace.h"
#include "buffer.h"
#include "http.h"
#include "iniparser/iniparser.h"
#include "ite/itp.h"



#define URENDER_INIFILENAME_MAXLEN              32
#define URENDER_NAME_MAXLEN                     32
#define URENDER_INIKEY_NAME                     "urender:friendlyName"

#define URENDER_SSID_MAXLEN                     64
#define URENDER_PASSWORD_MAXLEN                 256

urender_callback urender_cb;

struct urender_t *ut = NULL;
static char *description_xml = NULL;


#ifdef CFG_AUDIOLINK_URENDER_ONLY_ONE_PLAY
#define URENDER_ONLY_ONE_DEVICE_CAN_PLAY
#define URENDER_ONLY_ONE_DEVICE_CAN_PLAY_SECONDS CFG_AUDIOLINK_URENDER_ONLY_ONE_PLAY_SECONDS*1000
#define URENDER_ONLY_ONE_DEVICE_CAN_PLAY_SUSPEND_SECONDS CFG_AUDIOLINK_URENDER_ONLY_ONE_PLAY_SUSPEND_SECONDS*1000
#endif

#define SERVICE_RCS_ACTION_GET_VOLUME "GetVolume"

static pthread_mutex_t mutex_urender_service;

#ifdef URENDER_ONLY_ONE_DEVICE_CAN_PLAY
#define SERVICE_AVT_ACTION_SET_URI "SetAVTransportURI"
#define SERVICE_AVT_ACTION_PLAY "Play"
#define SERVICE_AVT_ACTION_STOP "Stop"
#define SERVICE_AVT_ACTION_PAUSE "Pause"
#define SERVICE_AVT_ACTION_SEEK "Seek"
#define SERVICE_RCS_ACTION_SET_MUTE "SetMute"
#define SERVICE_RCS_ACTION_SET_VOLUME "SetVolume"

#define SERVICE_RCS_ACTION_GET_MUTE "GetMute"
#define SERVICE_AVT_ACTION_GET_TS_INFO "GetTransportInfo"
#define SERVICE_AVT_ACTION_GET_POS_INFO "GetPositionInfo"

static pthread_mutex_t mutex_urender_one_device;

// keep android device owner ip
static char gAndDevOwnerIp[URENDER_ANDROID_DEVICE_IP_MAXLEN];
static char gAndDevCheckOwnerIp[URENDER_ANDROID_DEVICE_IP_MAXLEN];
static int    gnGetAndOwner = 0;
static int    gnGetAndOwnerStop = 0;
// keep stop time , need task ?
struct timeval gtAndOwnerStop = {0, 0};
// suspend time,keeping owner last query time 
struct timeval gtAndOwnerSuspend = {0, 0};

#endif

struct timeval gtVolumeCmd = {0, 0};

extern char giniName[URENDER_INIFILENAME_MAXLEN];

static struct urender_t *
urender_new (void)
{
    struct urender_t *ut = (struct urender_t *) malloc (sizeof (struct urender_t));
    if (!ut)
        return NULL;

    ut->name = strdup (PACKAGE_NAME);
    ut->interface_ = strdup (DEFAULT_URENDER_IFACE);
    ut->model_name = strdup (DEFAULT_URENDER_NAME);
    ut->starting_id = STARTING_ENTRY_ID_DEFAULT;
    ut->init = 0;
    ut->dev = 0;
    ut->udn = NULL;
    ut->uri = NULL;
    ut->uri_metadata = NULL;
    ut->ip = NULL;
    ut->port = 0; /* Randomly attributed by libupnp */
    ut->presentation = NULL;
    ut->use_presentation = TRUE;
    ut->reboot = FALSE;
    ut->upgrading = false;

#ifdef HAVE_DLNA
    ut->dlna_enabled = false;
    ut->dlna = NULL;
    ut->dlna_flags = DLNA_ORG_FLAG_STREAMING_TRANSFER_MODE |
        DLNA_ORG_FLAG_BACKGROUND_TRANSFERT_MODE |
        DLNA_ORG_FLAG_CONNECTION_STALL |
        DLNA_ORG_FLAG_DLNA_V15;
#endif /* HAVE_DLNA */
    ut->verbose = 0;
    ut->override_iconv_err = 0;
    ut->cfg_file = NULL;
    memset(&ut->avt, 0, sizeof(struct avt_t));
    memset(&ut->rcs, 0, sizeof(struct rcs_t));

    pthread_mutex_init (&ut->termination_mutex, NULL);
    pthread_mutex_init (&ut->render_mutex, NULL);
    pthread_cond_init (&ut->termination_cond, NULL);
#ifdef URENDER_ONLY_ONE_DEVICE_CAN_PLAY    
    pthread_mutex_init(&mutex_urender_one_device, NULL);
#endif
	gettimeofday(&gtVolumeCmd, NULL);
    return ut;
}

static void
urender_free (struct urender_t *ut)
{
    if (description_xml)
    {
        free(description_xml);
        description_xml = NULL;
    }

    if (!ut)
        return;

    if (ut->name)
        free (ut->name);
    if (ut->interface_)
        free (ut->interface_);
    if (ut->model_name)
        free (ut->model_name);
    if (ut->udn)
        free (ut->udn);
    if (ut->uri)
        free (ut->uri);
    if (ut->uri_metadata)
        free (ut->uri_metadata);
    if (ut->ip)
        free (ut->ip);
    if (ut->presentation)
        buffer_free (ut->presentation);
#ifdef HAVE_DLNA
    if (ut->dlna_enabled)
    {
        if (ut->dlna)
            dlna_uninit (ut->dlna);
        ut->dlna = NULL;
    }
#endif /* HAVE_DLNA */
    if (ut->cfg_file)
        free (ut->cfg_file);

    pthread_cond_destroy (&ut->termination_cond);
    pthread_mutex_destroy (&ut->render_mutex);
    pthread_mutex_destroy (&ut->termination_mutex);
#ifdef URENDER_ONLY_ONE_DEVICE_CAN_PLAY    
    pthread_mutex_destroy(&mutex_urender_one_device);
#endif

    free (ut);
}

static void
urender_signal_exit (void)
{
    pthread_mutex_lock (&ut->termination_mutex);
    pthread_cond_signal (&ut->termination_cond);
    pthread_mutex_unlock (&ut->termination_mutex);
}

static void
handle_action_request (struct Upnp_Action_Request *request)
{
    struct service_t *service;
    struct service_action_t *action;
    char val[256];
    unsigned int ip;
    struct timeval tv = {0, 0};

    if (!request || !ut)
        return;

    if (request->ErrCode != UPNP_E_SUCCESS)
        return;

    if (strcmp (request->DevUDN + 5, ut->udn))
        return;

    ip = ((struct sockaddr_in *)&request->CtrlPtIPAddr)->sin_addr.s_addr;
    ip = ntohl (ip);
    sprintf (val, "%d.%d.%d.%d",
        (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF);

    if (ut->verbose)
    {
        DOMString str = ixmlPrintDocument (request->ActionRequest);
        log_verbose ("***************************************************\n",0);
        log_verbose ("**             New Action Request                **\n",0);
        log_verbose ("***************************************************\n",0);
        log_verbose ("ServiceID: %s\n", request->ServiceID);
        log_verbose ("ActionName: %s\n", request->ActionName);
        log_verbose ("CtrlPtIP: %s\n", val);
        log_verbose ("Action Request:\n%s\n", str);
        ixmlFreeDOMString (str);
    }

    /* lock state mutex */

    if (find_service_action (request, &service, &action))
    {
        struct action_event_t event;
        //check if the device get power
#ifdef URENDER_ONLY_ONE_DEVICE_CAN_PLAY
        pthread_mutex_lock(&mutex_urender_one_device);

        gettimeofday(&tv, NULL);

        if (gnGetAndOwner==0 && (!strcmp(action->name, SERVICE_AVT_ACTION_SET_URI))){
            sprintf (gAndDevOwnerIp, "%d.%d.%d.%d",
                (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF);            
            printf("[Urender] device %s get the power #line %d\n",gAndDevOwnerIp,__LINE__);
            gnGetAndOwner = 1;
            gnGetAndOwnerStop = 0;            
        } if (gnGetAndOwner==0 && (!strcmp(action->name, SERVICE_AVT_ACTION_PLAY))){
            sprintf (gAndDevOwnerIp, "%d.%d.%d.%d",
                (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF);
            printf("[Urender] device %s get the power #line %d\n",gAndDevOwnerIp,__LINE__);
            gnGetAndOwner = 1;
            gnGetAndOwnerStop = 0;            
        } else if (gnGetAndOwner ==0) {
            // do nothing
        
        } else {
            sprintf (gAndDevCheckOwnerIp, "%d.%d.%d.%d",
                (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF);            
            if (!strcmp(gAndDevOwnerIp, gAndDevCheckOwnerIp)){
                 //printf("[Urender] handle_action_request same ip %s, %s \n",gAndDevOwnerIp,gAndDevCheckOwnerIp);
                 gettimeofday(&gtAndOwnerSuspend, NULL);
                 if (!strcmp(action->name, SERVICE_AVT_ACTION_STOP)){
                     printf("[Urender] handle_action_request owner stop \n");
                     gettimeofday(&gtAndOwnerStop, NULL);
                     gnGetAndOwnerStop = 1;
                 }                 
                 if (!strcmp(action->name, SERVICE_AVT_ACTION_SET_URI)){
                     printf("[Urender] handle_action_request owner clear stop \n");
                     gettimeofday(&gtAndOwnerStop, NULL);
                     gnGetAndOwnerStop = 0;
                 }
                 
            } else {
                 // differnet ip
                 printf("[Urender] handle_action_request diff ip %s, %s ,stop %d,suspend %d \n",gAndDevOwnerIp,gAndDevCheckOwnerIp,itpTimevalDiff(&gtAndOwnerStop, &tv),itpTimevalDiff(&gtAndOwnerSuspend, &tv));
                 if (itpTimevalDiff(&gtAndOwnerStop, &tv)>=URENDER_ONLY_ONE_DEVICE_CAN_PLAY_SECONDS && gnGetAndOwnerStop){
                     printf("[Urender] clear urender owner #line %d \n",__LINE__);
                     memset(gAndDevOwnerIp,0,sizeof(gAndDevOwnerIp));
                     gnGetAndOwner = 0;
                 }
                 if (itpTimevalDiff(&gtAndOwnerSuspend, &tv)>=URENDER_ONLY_ONE_DEVICE_CAN_PLAY_SUSPEND_SECONDS && gnGetAndOwnerStop==0){
                     printf("[Urender] urender owner suspend #line %d \n",__LINE__);
                     memset(gAndDevOwnerIp,0,sizeof(gAndDevOwnerIp));
                     gnGetAndOwner = 0;
                 }
                 
                 if (gnGetAndOwner == 1) {
                     // skip different ip send command
                     if (!strcmp(action->name, SERVICE_AVT_ACTION_SET_URI) || !strcmp(action->name, SERVICE_AVT_ACTION_PLAY) ||
                          !strcmp(action->name, SERVICE_AVT_ACTION_STOP) || !strcmp(action->name, SERVICE_AVT_ACTION_PAUSE) || 
                          !strcmp(action->name, SERVICE_AVT_ACTION_SEEK) || !strcmp(action->name, SERVICE_RCS_ACTION_SET_MUTE) || 
                          !strcmp(action->name, SERVICE_RCS_ACTION_SET_VOLUME) || !strcmp(action->name, SERVICE_RCS_ACTION_GET_VOLUME) || 
                          !strcmp(action->name, SERVICE_RCS_ACTION_GET_MUTE)/* || !strcmp(action->name, SERVICE_AVT_ACTION_GET_TS_INFO) || !strcmp(action->name, SERVICE_AVT_ACTION_GET_POS_INFO)*/){
                         printf("[Urender] skip command %s \n",action->name);
                         pthread_mutex_unlock(&mutex_urender_one_device);
                         return;                     
                     }
                 }

            }

        }
        pthread_mutex_unlock(&mutex_urender_one_device);           
#endif

        event.request = request;
        event.status = 1;
        event.service = service;
        gettimeofday(&tv, NULL);
#if 1
        if (!strcmp(action->name, SERVICE_RCS_ACTION_GET_VOLUME) && itpTimevalDiff(&gtVolumeCmd, &tv)<=200){
                if (action->function (&event) && event.status){
                    request->ErrCode = UPNP_E_SUCCESS;
                }
                printf("[Urender] delay command %s %d\n",action->name,itpTimevalDiff(&gtVolumeCmd, &tv));
                gettimeofday(&gtVolumeCmd, NULL);
                if (itpTimevalDiff(&gtVolumeCmd, &tv)<=40){
                    //gettimeofday(&gtVolumeCmd, NULL);
                    usleep(40000);
                }
                usleep(40000);
            } else {
            if (!strcmp(action->name, SERVICE_RCS_ACTION_GET_VOLUME)){
                printf("[Urender] %s %d\n",action->name,itpTimevalDiff(&gtVolumeCmd, &tv));
                gettimeofday(&gtVolumeCmd, NULL);
            }
            if (action->function (&event) && event.status)
                request->ErrCode = UPNP_E_SUCCESS;
        }
#else
        if (action->function (&event) && event.status)
            request->ErrCode = UPNP_E_SUCCESS;
#endif

        if (ut->verbose)
        {
            DOMString str = ixmlPrintDocument (request->ActionResult);
            log_verbose ("Action Result:\n%s", str);
            log_verbose ("***************************************************\n",0);
            log_verbose ("\n",0);
            ixmlFreeDOMString (str);
        }

        return;
    }

    if (service) /* Invalid Action name */
        strcpy (request->ErrStr, "Unknown Service Action");
    else /* Invalid Service name */
        strcpy (request->ErrStr, "Unknown Service ID");

    request->ActionResult = NULL;
    request->ErrCode = UPNP_SOAP_E_INVALID_ACTION;
}

static void
handle_event_subscription_request (struct Upnp_Subscription_Request *request)
{
    const char *serviceId = NULL;
    const char *sid = NULL;
    const char *udn = NULL;
    struct service_t *service;
    int i;

    serviceId = request->ServiceId;
    sid = request->Sid;
    udn = request->UDN;

    /* lock state mutex */
    pthread_mutex_lock(&ut->render_mutex);

    if (!request || !ut)
        return;

    if (strcmp (udn + 5, ut->udn))
        return;

    if (find_subscription_service (request, &service))
    {
        UpnpAcceptSubscription(ut->dev,
                               udn,
                               serviceId,
                               (const char **)&service->variables->var_name[0],
                               (const char **)&service->variables->var_str[0],
                               service->variables->var_count,
                               sid);
        for (i=0;i<service->variables->var_count;i++)
            if(service->variables->var_str[i]) {
                free(service->variables->var_str[i]);
                service->variables->var_str[i] = NULL;
            }
    }
    pthread_mutex_unlock(&ut->render_mutex);
}

static int
device_callback_event_handler (Upnp_EventType type, void *event,
                               void *cookie)
{
    switch (type)
    {
    case UPNP_CONTROL_ACTION_REQUEST:
        handle_action_request ((struct Upnp_Action_Request *) event);
        break;
    case UPNP_EVENT_SUBSCRIPTION_REQUEST:
        handle_event_subscription_request((struct Upnp_Subscription_Request *) event);
        break;
    case UPNP_CONTROL_GET_VAR_REQUEST:
    case UPNP_CONTROL_ACTION_COMPLETE:
        break;
    default:
        break;
    }

    return 0;
}

static int
finish_upnp (struct urender_t *ut)
{
    if (!ut)
        return -1;

    log_info ("Stopping UPnP Service ...\n",0);
    UpnpUnRegisterRootDevice (ut->dev);
    UpnpFinish ();

    return UPNP_E_SUCCESS;
}

static int
has_iface (char *interf)
{
    int sock;

    if (!interf)
        return 0;

    sock = socket (AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror ("socket");
        return 0;
    }

    closesocket(sock);
    return 1;
}

static char *
create_udn (char *interface)
{
    int sock = -1;
    char *buf;
    unsigned char *ptr;

#if (defined(BSD) || defined(__FreeBSD__) || defined(__APPLE__))
    int mib[6];
    size_t len;
    struct if_msghdr *ifm;
    struct sockaddr_dl *sdl;
#else /* Linux */
    struct ifreq ifr;
#endif

    if (!interface)
        return NULL;

#if (defined(BSD) || defined(__FreeBSD__) || defined(__APPLE__))
    mib[0] = CTL_NET;
    mib[1] = AF_ROUTE;
    mib[2] = 0;
    mib[3] = AF_LINK;
    mib[4] = NET_RT_IFLIST;

    mib[5] = if_nametoindex (interface);
    if (mib[5] == 0)
    {
        perror ("if_nametoindex");
        return NULL;
    }

    if (sysctl (mib, 6, NULL, &len, NULL, 0) < 0)
    {
        perror ("sysctl");
        return NULL;
    }

    buf = malloc (len);
    if (sysctl (mib, 6, buf, &len, NULL, 0) < 0)
    {
        perror ("sysctl");
        return NULL;
    }

    ifm = (struct if_msghdr *) buf;
    sdl = (struct sockaddr_dl*) (ifm + 1);
    ptr = (unsigned char *) LLADDR (sdl);
#else /* Linux */
    /* determine UDN according to MAC address */
    sock = socket (AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror ("socket");
        return NULL;
    }

    strcpy (ifr.ifr_name, interface);
    strcpy (ifr.ifr_hwaddr.sa_data, "");

    if (ioctl (sock, SIOCGIFHWADDR, &ifr) < 0)
    {
        perror ("ioctl");
        return NULL;
    }

    buf = (char *) malloc (64 * sizeof (char));
    memset (buf, 0, 64);
    ptr = (unsigned char *) ifr.ifr_hwaddr.sa_data;
#endif /* (defined(BSD) || defined(__FreeBSD__)) */

    snprintf (buf, 64, "%s-%02x%02x%02x%02x%02x%02x", DEFAULT_UUID,
             (ptr[0] & 0377), (ptr[1] & 0377), (ptr[2] & 0377),
             (ptr[3] & 0377), (ptr[4] & 0377), (ptr[5] & 0377));

    if (sock)
        close (sock);

    return buf;
}

static char *
get_iface_address (char *interface)
{
    int sock;
    uint32_t ip;
    struct ifreq ifr;
    char *val;

    if (!interface)
        return NULL;

    /* determine UDN according to MAC address */
    sock = socket (AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        perror ("socket");
        return NULL;
    }

    strcpy (ifr.ifr_name, interface);
    ifr.ifr_addr.sa_family = AF_INET;

    if (ioctl (sock, SIOCGIFADDR, &ifr) < 0)
    {
        perror ("ioctl");
        close (sock);
        return NULL;
    }

    val = (char *) malloc (16 * sizeof (char));
    ip = ((struct sockaddr_in *) &ifr.ifr_addr)->sin_addr.s_addr;
    ip = ntohl (ip);
    sprintf (val, "%d.%d.%d.%d",
        (ip >> 24) & 0xFF, (ip >> 16) & 0xFF, (ip >> 8) & 0xFF, ip & 0xFF);

    close (sock);

    return val;
}

static int
init_upnp (struct urender_t *ut)
{
    char *description = NULL;
    int res;
    size_t len;

    log_info ("Initializing UPnP subsystem ...\n",0);
    printf("[uRender]%s() L#%ld: ut->ip=%s, ut->port=%ld\r\n", __FUNCTION__, __LINE__, ut->ip, ut->port);
    res = UpnpInit ((ut->ip ? ut->ip : ""), ut->port);
    if (res != UPNP_E_SUCCESS)
    {
        log_error ("Cannot initialize UPnP subsystem\n",0);
        return -1;
    }

#ifdef URENDER_ONLY_ONE_DEVICE_CAN_PLAY   
    memset(gAndDevOwnerIp,0,sizeof(gAndDevOwnerIp));
    gnGetAndOwner = 0;
#endif    

    ut->udn = create_udn (ut->interface_);
    if (!ut->udn)
    {
        urender_free (ut);
        return EXIT_FAILURE;
    }

    //ut->ip = get_iface_address (ut->interface_);
    ut->ip = strdup (UpnpGetServerIpAddress());

    if (!ut->ip)
    {
        urender_free (ut);
        return EXIT_FAILURE;
    }

    if (!ut || !ut->name || !ut->udn || !ut->ip)
        return -1;

#ifdef HAVE_DLNA
    if (ut->dlna_enabled)
    {
        len = 0;
        description =
            dlna_dms_description_get (ut->name,
                                      "ITE Tech. Inc.",
                                      "http://www.ite.com.tw/",
                                      "DLNA Media Renderer",
                                      ut->model_name,
                                      "001",
                                      "http://www.ite.com.tw/",
                                      "CASTOR3-01",
                                      ut->udn,
                                      "/web/info.html",
                                      "/web/cms.xml",
                                      "/web/cms_control",
                                      "/web/cms_event",
                                      "/web/cds.xml",
                                      "/web/cds_control",
                                      "/web/cds_event");
        if (!description)
            return -1;
    }
    else
    {
#endif /* HAVE_DLNA */

        len = strlen (UPNP_DESCRIPTION)
            + strlen (ut->name)
            + strlen (CFG_NET_URENDER_MENUFACTURER)
            + strlen (CFG_NET_URENDER_MENUFACTURERURL)
            + strlen (CFG_NET_URENDER_MODELDESCRIPTION)
            + strlen (CFG_NET_URENDER_MODELNAME)
            + strlen (CFG_NET_URENDER_MODELNUMBER)
            + strlen (CFG_NET_URENDER_MODELURL)
            + strlen (CFG_NET_URENDER_SERIALNUMBER)
            + strlen (ut->udn) + 1;
        description = (char *) malloc (len * sizeof (char));
        memset (description, 0, len * sizeof (char));
        sprintf (description, UPNP_DESCRIPTION, ut->name,
                                                CFG_NET_URENDER_MENUFACTURER,
                                                CFG_NET_URENDER_MENUFACTURERURL,
                                                CFG_NET_URENDER_MODELDESCRIPTION,
                                                CFG_NET_URENDER_MODELNAME,
                                                CFG_NET_URENDER_MODELNUMBER,
                                                CFG_NET_URENDER_MODELURL,
                                                CFG_NET_URENDER_SERIALNUMBER,
                                                ut->udn);
        description_xml = strdup(description);

#ifdef HAVE_DLNA
    }
#endif /* HAVE_DLNA */

    if (UpnpSetMaxContentLength (UPNP_MAX_CONTENT_LENGTH) != UPNP_E_SUCCESS)
        log_info ("Could not set Max content UPnP\n",0);

#ifdef HAVE_DLNA
    if (ut->dlna_enabled)
    {
        log_info (_("Starting in DLNA compliant profile ...\n"));
        ut->dlna = dlna_init ();
        dlna_set_verbosity (ut->dlna, ut->verbose ? 1 : 0);
        dlna_set_extension_check (ut->dlna, 1);
        dlna_register_all_media_profiles (ut->dlna);
    }
#endif /* HAVE_DLNA */

    ut->port = UpnpGetServerPort();
    log_info ("UPnP MediaRender listening on %s:%d\n",
        UpnpGetServerIpAddress (), ut->port);

    res = SetVirtualDirCallbacks ();
    if (res != UPNP_E_SUCCESS)
    {
        log_error ("Cannot set virtual directory callbacks\n",0);
        free (description);
        return -1;
    }

    res = UpnpAddVirtualDir ("/");
    if (res != UPNP_E_SUCCESS)
    {
        log_error ("Cannot add virtual directory for web server\n",0);
        free (description);
        return -1;
    }

    res = UpnpAddVirtualDir (VIRTUAL_DIR);
    if (res != UPNP_E_SUCCESS)
    {
        log_error ("Cannot add virtual directory for web server\n",0);
        free (description);
        return -1;
    }

    res = UpnpRegisterRootDevice2 (UPNPREG_BUF_DESC, description, 0, 1,
                                   device_callback_event_handler,
                                   NULL, &(ut->dev));
    if (res != UPNP_E_SUCCESS)
    {
        log_error ("Cannot register UPnP device\n",0);
        free (description);
        return -1;
    }

    res = UpnpUnRegisterRootDevice (ut->dev);
    if (res != UPNP_E_SUCCESS)
    {
        log_error ("Cannot unregister UPnP device\n",0);
        free (description);
        return -1;
    }

    res = UpnpRegisterRootDevice2 (UPNPREG_BUF_DESC, description, 0, 1,
                                   device_callback_event_handler,
                                   NULL, &(ut->dev));
    if (res != UPNP_E_SUCCESS)
    {
        log_error ("Cannot register UPnP device\n",0);
        free (description);
        return -1;
    }

    log_info ("Sending UPnP advertisement for device ...\n",0);
    UpnpSendAdvertisement (ut->dev, 1800);

    log_info ("Listening for control point connections ...\n",0);

    if (description)
        free (description);

    return 0;
}

static int
restart_upnp (struct urender_t *ut)
{
    finish_upnp (ut);

    if (ut->udn)
        free (ut->udn);
    ut->udn = create_udn (ut->interface_);
    if (!ut->udn)
        return -1;

    if (ut->ip)
        free (ut->ip);
    ut->ip = get_iface_address (ut->interface_);
    if (!ut->ip)
        return -1;

    return (init_upnp (ut));
}

static void
display_headers (void)
{
    printf ("A lightweight UPnP A/V and DLNA Media Render.\n",0);

}

static void
setup_i18n(void)
{
#ifdef CONFIG_NLS
#ifdef HAVE_SETLOCALE
    setlocale (LC_ALL, "");
#endif
#if (!defined(BSD) && !defined(__FreeBSD__))
    bindtextdomain (PACKAGE, LOCALEDIR);
#endif
    textdomain (PACKAGE);
#endif
}

#define SHUTDOWN_MSG _("Render is shutting down: other clients will be notified soon, Bye bye ...\n")

static void
urender_kill ()
{
    urender_signal_exit ();
}

/*
Special characters check.
*/
//#define STR_QUOT        "&quot;"
#define STR_LT          "&lt;"
#define STR_GT          "&gt;"
//#define STR_APOS        "&apos;"
#define STR_AMP         "&amp;"

//2015.1.8 my.wei add
static void 
check_special_char(
const char *src, char *dst, uint len)
{
    int i,j;
    j = 0;
    for(i = 0; i < len; i++)
    {
        if(j == URENDER_NAME_MAXLEN)
            break;
        
    	dst[j] = src[i];
    	if(src[i]  == '&')
    	{
    		memcpy(&dst[j], STR_AMP, strlen(STR_AMP));
    		j += 4;
    	}
    	if(src[i]  == '<')
    	{
    		memcpy(&dst[j], STR_LT, strlen(STR_LT));
    		j += 3;
    	}
    	if(src[i]  == '>')
    	{
    		memcpy(&dst[j], STR_GT, strlen(STR_GT));
    		j += 3;
    	}
        
    	j++;
    }
    printf("[Urender] check_special_char: src = [%s], dst = [%s]\r\n", src, dst);

}

int
urender_set_callback(urender_callback *cb)
{
    memcpy(&urender_cb, cb, sizeof(urender_cb));

    return 0;
}


int
urender_init (int argc, char **argv)
{
    dictionary* ini = NULL;
    char ini_filename[URENDER_INIFILENAME_MAXLEN];
    char str_name[URENDER_NAME_MAXLEN];

    printf("[uRender]%s() L#%ld: Begin\r\n", __FUNCTION__, __LINE__);
    ut = urender_new ();
    if (!ut)
        return EXIT_FAILURE;

    setup_i18n ();
    setup_iconv ();

    if (parse_config_file (ut) < 0)
    {
        /* fprintf here, because syslog not yet ready */
        fprintf (stderr, _("Warning: can't parse file \"%s\".\n"),
            ut->cfg_file ? ut->cfg_file : URENDER_CONFIG_FILE);
    }

    // read the setting from ini
#ifdef CFG_USE_SD_INI
    snprintf(ini_filename, URENDER_INIFILENAME_MAXLEN, "%s:/%s", "C", giniName);
#else
    snprintf(ini_filename, URENDER_INIFILENAME_MAXLEN, "%s:/%s", CFG_PUBLIC_DRIVE, giniName);
#endif
    ini = iniparser_load(ini_filename);
	if (ini)
	{
// alan, 2015-01-04, support '%' and '\'
		char *name = iniparser_getstring(ini, URENDER_INIKEY_NAME, "AudioLink");
//        snprintf(str_name, URENDER_NAME_MAXLEN, iniparser_getstring(ini, URENDER_INIKEY_NAME, "AudioLink"));
		memset(&str_name, 0x0, URENDER_NAME_MAXLEN);
		check_special_char(name, str_name, strlen(name));//URENDER_NAME_MAXLEN);//2015.1.8 my.wei
		//memcpy(&str_name, iniparser_getstring(ini, URENDER_INIKEY_NAME, "AudioLink"), (URENDER_NAME_MAXLEN - 1));
		printf("[Urender] name = [%s], str_name = [%s]\r\n", name, str_name);

        if (strlen(str_name) > 0)
        {
            if (ut->name)
            {
                free(ut->name);
                ut->name = NULL;
            }
            ut->name = strdup(str_name);
        }
        iniparser_freedict(ini);
	}
    else
    {
        printf("[uRender]%s() L#%ld: Cannot load ini file. Do ugResetFactory() and reboot!\r\n", __FUNCTION__, __LINE__);
        ioctl(ITP_DEVICE_FAT, ITP_IOCTL_UNMOUNT, (void*)ITP_DISK_NOR);
        ioctl(ITP_DEVICE_FAT, ITP_IOCTL_FORCE_MOUNT, (void*)ITP_DISK_NOR);
        ioctl(ITP_DEVICE_FAT, ITP_IOCTL_FORMAT, (void*)1);
        ugResetFactory();
        i2s_CODEC_standby();
        exit(0);
        while (1);
    }

    if (!has_iface (ut->interface_))
    {
        printf("[uRender][Error] has no interface!\r\n");
        urender_free (ut);
        return EXIT_FAILURE;
    }
    printf("[uRender] ut->interface_=%s\r\n", ut->interface_);

    ut->udn = create_udn (ut->interface_);
    if (!ut->udn)
    {
        printf("[uRender][Error] create_udn() fail!\r\n");
        urender_free (ut);
        return EXIT_FAILURE;
    }
    printf("[uRender] ut->udn=%s\r\n", ut->udn);

    ut->ip = get_iface_address (ut->interface_);
    if (!ut->ip)
    {
        printf("[uRender][Error] get_iface_address() fail!\r\n");
        urender_free (ut);
        return EXIT_FAILURE;
    }
    printf("[uRender] ut->ip=%s\r\n", ut->ip);

    if (init_upnp (ut) < 0)
    {
        printf("[uRender][Error] init_upnp() fail!\r\n");
        finish_upnp (ut);
        urender_free (ut);
        return EXIT_FAILURE;
    }

    urender_service_init();

    display_headers ();

#if 0
    /* Let main sleep until it's time to die... */
    pthread_mutex_lock (&ut->termination_mutex);
    pthread_cond_wait (&ut->termination_cond, &ut->termination_mutex);
    pthread_mutex_unlock (&ut->termination_mutex);
#endif

    return EXIT_SUCCESS;
}



int
urender_terminate(void)
{
    finish_upnp (ut);
    urender_free (ut);
    finish_iconv ();
}



int
urender_reboot(void)
{
    return ut->reboot;
}



char *
urender_get_description(void)
{
    return description_xml;
}



int
urender_get_description_len(void)
{
    return strlen(description_xml);
}



