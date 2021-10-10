#include <sys/ioctl.h>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "lwip/ip.h"
#include "ite/itp.h"
#include "platform.h"
#include "microhttpd.h"
#include "libxml/xpath.h"
#include "ite/ug.h"
#include "airconditioner.h"
#include "scene.h"

#define PAGE "<html><head><meta http-equiv=\"pragma\" content=\"no-cache\"/><meta http-equiv=\"expires\" content=\"0\"/><meta http-equiv=\"refresh\" content=\"0; URL=/setting.html\"/></head></html>"
#define DENIED "<html><head><title>Access denied</title></head><body>Access denied</body></html>"

/**
 * Data kept per request.
 */
struct Request
{
    struct MHD_PostProcessor *pp;
    struct MHD_Connection *connection;
    char* key;
    uint8_t* filebuf;
    uint32_t filesize;
};

struct MHD_Daemon* webServerDaemon;
static uint8_t* fwFileBuf;
static uint32_t fwFileSize;

static int
iterate_post (void *cls,
	       enum MHD_ValueKind kind,
	       const char *key,
	       const char *filename,
	       const char *content_type,
	       const char *transfer_encoding,
	       const char *data, uint64_t off, size_t size)
{
    struct Request *request = cls;

    if (request->key && 0 != strcmp (key, request->key))
        return MHD_NO;

    if (!request->filebuf)
    {
        const char *clen = MHD_lookup_connection_value (request->connection,
                                      MHD_HEADER_KIND,
                                      MHD_HTTP_HEADER_CONTENT_LENGTH);

        printf("filesize %s\n", clen);

        while (!UpgradeIsReady())
            usleep(1000);

        request->filesize = 0;
        request->filebuf = malloc(strtoul (clen, NULL, 10));
        if (!request->filebuf)
        {
            printf("alloc filebuf fail\n");
            return MHD_NO;
        }
    }
    memcpy(request->filebuf + off, data, size);
    request->filesize += size;
    printf("recv file %lu bytes\n", request->filesize);

    return MHD_YES;
}

static ssize_t
file_reader (void *cls, uint64_t pos, char *buf, size_t max)
{
  FILE *file = cls;

  (void) fseek (file, pos, SEEK_SET);
  return fread (buf, 1, max, file);
}

static void
file_free_callback (void *cls)
{
  FILE *file = cls;
  fclose (file);
}

static int
create_response (void *cls,
          struct MHD_Connection *connection,
          const char *url,
          const char *method,
          const char *version,
          const char *upload_data, size_t *upload_data_size, void **ptr)
{
    struct MHD_Response *response;
    struct Request *request;
    int ret;
    char *user;
    char *pass;
    int fail;

    if ((0 != strcmp (method, MHD_HTTP_METHOD_GET)) && (0 != strcmp (method, MHD_HTTP_METHOD_POST)))
        return MHD_NO;              /* unexpected method */

    request = *ptr;
    if (NULL == request)
    {
        request = calloc (1, sizeof (struct Request));
        if (NULL == request)
        {
	        fprintf (stderr, "calloc error: %s\n", strerror (errno));
	        return MHD_NO;
	    }
        *ptr = request;

        if (0 == strcmp (method, MHD_HTTP_METHOD_POST))
        {
            request->connection = connection;
            request->filebuf = NULL;
            request->filesize = 0;

            if (strcmp(url, "/dev/info.cgi") == 0)
            {
                SceneQuit(QUIT_UPGRADE_WEB);
                request->key = "filename";
            }
            request->pp = MHD_create_post_processor (connection, 1024, &iterate_post, request);
            if (NULL == request->pp)
            {
                printf ("Failed to setup post processor for '%s'\n", url);
                return MHD_NO; /* internal error */
            }
        }

        /* do never respond on first call */
        return MHD_YES;
    }

    pass = NULL;
    user = MHD_basic_auth_get_username_password (connection, &pass);
    fail = ( (user == NULL) || (0 != strcmp (user, theConfig.user_id)) || (0 != strcmp (pass, theConfig.user_password) ) );
    if (fail)
    {
        response = MHD_create_response_from_buffer (strlen (DENIED),
				          (void *) DENIED, 
				          MHD_RESPMEM_PERSISTENT);
        ret = MHD_queue_basic_auth_fail_response (connection,"Realm",response);
        MHD_destroy_response (response);
        return ret;
    }

    if (strcmp(url, "/dev/info.cgi") == 0)
    {
        if (0 == strcmp (method, MHD_HTTP_METHOD_GET))
        {
            const char* val;

            *ptr = NULL;                  /* reset when done */

            val = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "action");
            if (val == NULL)
                return MHD_NO;

            if (strcmp(val, "upgrade") == 0)
            {
                puts("ready to upgrade.");

				ret = MHD_HTTP_INTERNAL_SERVER_ERROR;

                // upgrade firmware
                if (fwFileSize > 0 && fwFileBuf)
                {
                    ITCArrayStream arrayStream;

                    itcArrayStreamOpen(&arrayStream, fwFileBuf, fwFileSize);

                    if (ugCheckCrc(&arrayStream.stream, NULL))
                    {
                        puts("Upgrade failed.");
                    }
                    else
                    {
                        UpgradeSetStream(&arrayStream.stream);

                        while (!UpgradeIsFinished())
                            sleep(1);

                        ret = UpgradeGetResult();
                        if (ret)
                        {
                            printf("Upgrade failed: %d\n", ret);
							ret = MHD_HTTP_INTERNAL_SERVER_ERROR;
                        }
                        else
                        {
                            puts("Upgrade finished.");
                            ret = MHD_HTTP_OK;
						#if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
							ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
						#endif							
                        }
                    }
                }
                response = MHD_create_response_from_buffer (0, NULL, MHD_RESPMEM_PERSISTENT);
                ret = MHD_queue_response (connection, ret, response);
                MHD_destroy_response (response);

                return ret;
            }
            else if (strcmp(val, "reboot") == 0)
            {
                printf("REBOOT!");
                sleep(2);
                itp_codec_standby();
                exit(0);
            }

            if (SceneGetQuitValue() == QUIT_UPGRADE_WEB)
                return MHD_NO;

            if (strcmp(val, "reset_factory") == 0)
            {
                SceneQuit(QUIT_RESET_FACTORY);

                response = MHD_create_response_from_buffer (0, NULL, MHD_RESPMEM_PERSISTENT);
                ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
                MHD_destroy_response (response);

                return ret;
            }
            else if (strcmp(val, "macaddr") == 0)
            {
                const char* str;
                char macaddr[6];

                str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "macaddr0");
                if (str)
                {
                    macaddr[0] = strtol(str, NULL, 16);

                    str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "macaddr1");
                    if (str)
                    {
                        macaddr[1] = strtol(str, NULL, 16);

                        str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "macaddr2");
                        if (str)
                        {
                            macaddr[2] = strtol(str, NULL, 16);

                            str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "macaddr3");
                            if (str)
                            {
                                macaddr[3] = strtol(str, NULL, 16);

                                str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "macaddr4");
                                if (str)
                                {
                                    macaddr[4] = strtol(str, NULL, 16);

                                    str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "macaddr5");
                                    if (str)
                                    {
                                        macaddr[5] = strtol(str, NULL, 16);
                                        ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_WIRTE_MAC, macaddr);

                                        response = MHD_create_response_from_buffer (0, NULL, MHD_RESPMEM_PERSISTENT);
                                        ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
                                        MHD_destroy_response (response);

                                        return ret;
                                    }
                                }
                            }
                        }
                    }
                }
            }
            return MHD_NO;
        }
        else if (0 == strcmp (method, MHD_HTTP_METHOD_POST))
        {
            if (ScreenSaverIsScreenSaving())
                ScreenSaverRefresh();

            /* evaluate POST data */
            if (0 != *upload_data_size)
            {
                MHD_post_process (request->pp, upload_data, *upload_data_size);
                *upload_data_size = 0;
                return MHD_YES;
            }

            puts("upload finish.");

            fwFileBuf = request->filebuf;
            fwFileSize = request->filesize;
            request->filebuf = NULL;

            /* done with POST data, serve response */
            MHD_destroy_post_processor (request->pp);
            request->pp = NULL;

            response = MHD_create_response_from_buffer (strlen (PAGE), (void *) PAGE, MHD_RESPMEM_PERSISTENT);
            ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
            MHD_destroy_response (response);

            return ret;
        }
        /* unsupported HTTP method */
        return MHD_NO;
    }

    if (SceneGetQuitValue() == QUIT_UPGRADE_WEB)
        return MHD_NO;

    if (!strcmp(url, "/setting.html") || !strcmp(url, "/"))
    {
        xmlDocPtr       doc;
        xmlXPathContext *xpathCtx;
        xmlXPathObject  *xpathObj;
        xmlChar         *xmlbuff;
        int             buffersize;
        char            buf[32];
    #ifdef CFG_NET_WIFI
        ITPWifiInfo     netInfo;
    #elif defined(CFG_NET_ETHERNET)
        ITPEthernetInfo netInfo;
    #endif
        char*           ip;
        struct timeval tv;
        struct tm *tm;

        *ptr = NULL;                  /* reset when done */

        gettimeofday(&tv, NULL);
        tm = localtime(&tv.tv_sec);

        doc      = xmlParseFile(CFG_PRIVATE_DRIVE ":/web/setting.html");
        xpathCtx = xmlXPathNewContext(doc);

    #ifdef CFG_NET_WIFI
        ioctl(ITP_DEVICE_WIFI, ITP_IOCTL_GET_INFO, &netInfo);
        ip = ipaddr_ntoa((const ip_addr_t*)&netInfo.address);
    #elif defined(CFG_NET_ETHERNET)
        netInfo.index = 0;
        ioctl(ITP_DEVICE_ETHERNET, ITP_IOCTL_GET_INFO, &netInfo);
        ip = ipaddr_ntoa((const ip_addr_t*)&netInfo.address);
    #else
        ip = theConfig.ipaddr;
    #endif // CFG_NET_WIFI

        // macaddr
        xpathObj = xmlXPathEvalExpression(BAD_CAST "//*[@id='macaddr']", xpathCtx);
        if (!xmlXPathNodeSetIsEmpty(xpathObj->nodesetval))
        {
            xmlNode* node = xpathObj->nodesetval->nodeTab[0];
            xmlNode* text;

            buf[0] = '\0';
        #ifdef CFG_NET_ENABLE
            sprintf(buf, "%02x:%02x:%02x:%02x:%02x:%02x", 
                (uint8_t)netInfo.hardwareAddress[0], 
                (uint8_t)netInfo.hardwareAddress[1], 
                (uint8_t)netInfo.hardwareAddress[2], 
                (uint8_t)netInfo.hardwareAddress[3], 
                (uint8_t)netInfo.hardwareAddress[4], 
                (uint8_t)netInfo.hardwareAddress[5]);
        #endif // CFG_NET_ENABLE
            text = xmlNewText(BAD_CAST buf);
            xmlAddChild(node, text);
        }
        xmlXPathFreeObject(xpathObj);

        // ipaddr
        xpathObj = xmlXPathEvalExpression(BAD_CAST "//*[@id='ipaddr']", xpathCtx);
        if (!xmlXPathNodeSetIsEmpty(xpathObj->nodesetval))
        {
            xmlNode* node = xpathObj->nodesetval->nodeTab[0];
            xmlNode* text = xmlNewText(BAD_CAST ip);
            xmlAddChild(node, text);
        }
        xmlXPathFreeObject(xpathObj);

        // sw_ver
        xpathObj = xmlXPathEvalExpression(BAD_CAST "//*[@id='sw_ver']", xpathCtx);
        if (!xmlXPathNodeSetIsEmpty(xpathObj->nodesetval))
        {
            xmlNode* node = xpathObj->nodesetval->nodeTab[0];
            xmlNode* text = xmlNewText(BAD_CAST CFG_VERSION_MAJOR_STR "." CFG_VERSION_MINOR_STR "." CFG_VERSION_PATCH_STR "." CFG_VERSION_CUSTOM_STR "." CFG_VERSION_TWEAK_STR);
            xmlAddChild(node, text);
        }
        xmlXPathFreeObject(xpathObj);

        // date
        xpathObj = xmlXPathEvalExpression(BAD_CAST "//*[@id='date_year']", xpathCtx);
        if (!xmlXPathNodeSetIsEmpty(xpathObj->nodesetval))
        {
            xmlNode* node = xpathObj->nodesetval->nodeTab[0];
            xmlNode* text;
            sprintf(buf, "%d", tm->tm_year + 1900);
            text = xmlNewText(BAD_CAST buf);
            xmlAddChild(node, text);
        }
        xmlXPathFreeObject(xpathObj);

        xpathObj = xmlXPathEvalExpression(BAD_CAST "//*[@id='date_month']", xpathCtx);
        if (!xmlXPathNodeSetIsEmpty(xpathObj->nodesetval))
        {
            xmlNode* node = xpathObj->nodesetval->nodeTab[0];
            xmlNode* text;
            sprintf(buf, "%d", tm->tm_mon + 1);
            text = xmlNewText(BAD_CAST buf);
            xmlAddChild(node, text);
        }
        xmlXPathFreeObject(xpathObj);

        xpathObj = xmlXPathEvalExpression(BAD_CAST "//*[@id='date_day']", xpathCtx);
        if (!xmlXPathNodeSetIsEmpty(xpathObj->nodesetval))
        {
            xmlNode* node = xpathObj->nodesetval->nodeTab[0];
            xmlNode* text;
            sprintf(buf, "%d", tm->tm_mday);
            text = xmlNewText(BAD_CAST buf);
            xmlAddChild(node, text);
        }
        xmlXPathFreeObject(xpathObj);

        // output
        xmlDocDumpFormatMemory(doc, &xmlbuff, &buffersize, 1);

        response = MHD_create_response_from_buffer (buffersize, xmlbuff, MHD_RESPMEM_MUST_FREE);
        if (response == NULL)
        {
            xmlFree (xmlbuff);
            xmlXPathFreeContext(xpathCtx);
            xmlFreeDoc(doc);
            return MHD_NO;
        }
        ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
        MHD_destroy_response (response);

        xmlXPathFreeContext(xpathCtx);
        xmlFreeDoc(doc);

        return ret;
    }
    else if (strncmp(url, "/public/", 8) == 0)
    {
        struct stat buf;
        FILE *file;
        char fullpath[PATH_MAX];

        strcpy(fullpath, CFG_PUBLIC_DRIVE ":");
        strcat(fullpath, strrchr(url, '/'));

        if (stat (fullpath, &buf))
        {
            *ptr = NULL;                  /* reset when done */
            return MHD_NO;
        }

        file = fopen (fullpath, "rb");
        if (file == NULL)
        {
            *ptr = NULL;                  /* reset when done */
            return MHD_NO;
        }
        else
        {
            response = MHD_create_response_from_callback (buf.st_size, 32 * 1024,     /* 32k PAGE_NOT_FOUND size */
                                                    &file_reader, file,
                                                    &file_free_callback);
            if (response == NULL)
            {
                fclose (file);
                return MHD_NO;
            }
            ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
            MHD_destroy_response (response);

            return ret;
        }
        return MHD_NO;
    }
    else
    {
        struct stat buf;
        FILE *file;
        char fullpath[PATH_MAX];

        strcpy(fullpath, CFG_PRIVATE_DRIVE ":/web");
        strcat(fullpath, strrchr(url, '/'));

        if (stat (fullpath, &buf))
        {
            *ptr = NULL;                  /* reset when done */
            return MHD_NO;
        }

        file = fopen (fullpath, "rb");
        if (file == NULL)
        {
            *ptr = NULL;                  /* reset when done */
            return MHD_NO;
        }
        else
        {
            response = MHD_create_response_from_callback (buf.st_size, 32 * 1024,     /* 32k PAGE_NOT_FOUND size */
                                                    &file_reader, file,
                                                    &file_free_callback);
            if (response == NULL)
	        {
	            fclose (file);
	            return MHD_NO;
	        }
            ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
            MHD_destroy_response (response);

            return ret;
        }
    }
}

static void
request_completed_callback (void *cls,
			    struct MHD_Connection *connection,
			    void **con_cls,
			    enum MHD_RequestTerminationCode toe)
{
    struct Request *request = *con_cls;

    if (NULL == request)
        return;

    if (NULL != request->pp)
        MHD_destroy_post_processor (request->pp);

    free (request->filebuf);
    free (request);
}

void WebServerInit(void)
{
    printf("Start Web Server...\n");

    webServerDaemon = MHD_start_daemon(MHD_USE_SELECT_INTERNALLY | MHD_USE_DEBUG, CFG_WEBSERVER_PORT, NULL, NULL,
                         &create_response, NULL,
                         MHD_OPTION_NOTIFY_COMPLETED, &request_completed_callback, NULL,
                         NULL, MHD_OPTION_END);
    if (webServerDaemon == NULL)
        printf("Start Web Server fail\n");
}

void WebServerExit(void)
{
    printf("Stop Web Server...\n");

    MHD_stop_daemon(webServerDaemon);
}
