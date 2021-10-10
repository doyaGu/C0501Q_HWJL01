#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "lwip/ip.h"
#include "ite/itp.h"
#include "platform.h"
#include "microhttpd.h"
#include "libxml/xpath.h"
#include "ite/ug.h"
#include "ctrlboard.h"
#include "scene.h"
#ifdef CFG_NET_FTP_SERVER
#include "ftpd.h"
#endif

#define PAGE "<html><head><meta http-equiv=\"pragma\" content=\"no-cache\"/><meta http-equiv=\"expires\" content=\"0\"/><meta http-equiv=\"refresh\" content=\"0; URL=/setting.html\"/></head></html>"

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

            if (strcmp(val, "setting") == 0)
            {
                const char* str;
                int lang = theConfig.lang, wifi_mode = theConfig.wifi_mode;

                // brightness
                str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "brightness");
                if (str)
                {
                    theConfig.brightness = atoi(str);
                    ScreenSetBrightness(theConfig.brightness);
                }

                // screensaver_time
                str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "screensaver_time");
                if (str)
                {
                    theConfig.screensaver_time = atoi(str);
                }

                // screensaver_type
                str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "screensaver_type");
                if (str)
                {
                    theConfig.screensaver_type = atoi(str);
                }

                // lang
                str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "lang");
                if (str)
                {
                    if (strcmp(str, "cht") == 0)
                        lang = LANG_CHT;
                    else if (strcmp(str, "chs") == 0)
                        lang = LANG_CHS;
                    else if (strcmp(str, "eng") == 0)
                        lang = LANG_ENG;

                    if (theConfig.lang != lang)
                    {
                        theConfig.lang = lang;
                        SceneChangeLanguage();
                    }
                }

				// wifi mode
                str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "wifi_mode");
				printf("###[web server] str = %s, theConfig.wifi_mode = %d\n", str, theConfig.wifi_mode);
                if (str)
                {
                    if (strcmp(str, "client") == 0)
                        wifi_mode = WIFI_CLIENT;
                    else if (strcmp(str, "softap") == 0)
                        wifi_mode = WIFI_SOFTAP;

                    if (theConfig.wifi_mode!= wifi_mode)
                    {
                        theConfig.wifi_mode = wifi_mode;
#ifdef CFG_NET_WIFI
                        NetworkWifiModeSwitch();
#endif
                    }
                }

                // keylevel
                str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "keylevel");
                if (str)
                {
                    AudioSetKeyLevel(atoi(str));
                }

                // keysound_type
                str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "keysound_type");
                if (str)
                {
                    strncpy(theConfig.keysound, str, sizeof (theConfig.keysound) - 1);
                }

                // play_lev
                str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "play_lev");
                if (str)
                {
                    AudioSetVolume(atoi(str));
                }

                ConfigSave();

                response = MHD_create_response_from_buffer (strlen (PAGE), (void *) PAGE, MHD_RESPMEM_PERSISTENT);
                ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
                MHD_destroy_response (response);

                return ret;
            }
#ifdef CFG_NET_FTP_SERVER
			else if (strcmp(val, "account") == 0)
			{
				const char* str;
				bool configModified = false;

				// old_password
				str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "old_password");
				if (str)
				{
					if (strcmp(theConfig.user_password, str) == 0)
					{
						// user_id
						str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "user_id");
						if (str)
						{
							if (strcmp(theConfig.user_id, str))
							{
								strcpy(theConfig.user_id, str);
								configModified = true;
							}
						}

						// user_password
						str = MHD_lookup_connection_value(connection, MHD_GET_ARGUMENT_KIND, "user_password");
						if (str)
						{
							if (strcmp(theConfig.user_password, str))
							{
								strcpy(theConfig.user_password, str);
								configModified = true;
							}
						}

						if (configModified)
						{
							ftpd_setlogin(theConfig.user_id, theConfig.user_password);
							//SceneUpdateByConfig();
							ConfigSave();
						}

						response = MHD_create_response_from_buffer (strlen (PAGE), (void *) PAGE, MHD_RESPMEM_PERSISTENT);
						ret = MHD_queue_response (connection, MHD_HTTP_OK, response);
						MHD_destroy_response (response);

						return ret;
					}
				}
				return MHD_NO;
			}
#endif
            else if (strcmp(val, "reset_factory") == 0)
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
            if (ScreenSaverIsScreenSaving() && theConfig.screensaver_type == SCREENSAVER_BLANK)
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

        *ptr = NULL;                  /* reset when done */

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

        // hw_ver
        xpathObj = xmlXPathEvalExpression(BAD_CAST "//*[@id='hw_ver']", xpathCtx);
        if (!xmlXPathNodeSetIsEmpty(xpathObj->nodesetval))
        {
            xmlNode* node = xpathObj->nodesetval->nodeTab[0];
            xmlNode* text = xmlNewText(BAD_CAST CFG_HW_VERSION);
            xmlAddChild(node, text);
        }
        xmlXPathFreeObject(xpathObj);

#ifdef SIP_SERVER_TEST
		// sip_account
        xpathObj = xmlXPathEvalExpression(BAD_CAST "//*[@id='sip_account']", xpathCtx);
        if (!xmlXPathNodeSetIsEmpty(xpathObj->nodesetval))
        {
            xmlNode* node = xpathObj->nodesetval->nodeTab[0];
            xmlNode* text = xmlNewText(BAD_CAST CFG_REGISTER_ACCOUNT);
            xmlAddChild(node, text);
        }
        xmlXPathFreeObject(xpathObj);

		// sip_password
        xpathObj = xmlXPathEvalExpression(BAD_CAST "//*[@id='sip_password']", xpathCtx);
        if (!xmlXPathNodeSetIsEmpty(xpathObj->nodesetval))
        {
            xmlNode* node = xpathObj->nodesetval->nodeTab[0];
            xmlNode* text = xmlNewText(BAD_CAST CFG_REGISTER_DOMAIN);
            xmlAddChild(node, text);
        }
        xmlXPathFreeObject(xpathObj);

		// sip_domain
        xpathObj = xmlXPathEvalExpression(BAD_CAST "//*[@id='sip_domain']", xpathCtx);
        if (!xmlXPathNodeSetIsEmpty(xpathObj->nodesetval))
        {
            xmlNode* node = xpathObj->nodesetval->nodeTab[0];
            xmlNode* text = xmlNewText(BAD_CAST CFG_REGISTER_PWD);
            xmlAddChild(node, text);
        }
        xmlXPathFreeObject(xpathObj);
#endif

        // brightness
        xpathObj = xmlXPathEvalExpression(BAD_CAST "//*[@id='brightness']", xpathCtx);
        if (!xmlXPathNodeSetIsEmpty(xpathObj->nodesetval))
        {
            xmlNode* node = xpathObj->nodesetval->nodeTab[0];
            sprintf(buf, "%d", theConfig.brightness);
            xmlNewProp(node, BAD_CAST "value", BAD_CAST buf);
        }
        xmlXPathFreeObject(xpathObj);

        // screensaver_time
        xpathObj = xmlXPathEvalExpression(BAD_CAST "//*[@id='screensaver_time']", xpathCtx);
        if (!xmlXPathNodeSetIsEmpty(xpathObj->nodesetval))
        {
            xmlNode* node = xpathObj->nodesetval->nodeTab[0];
            sprintf(buf, "%d", theConfig.screensaver_time);
            xmlNewProp(node, BAD_CAST "value", BAD_CAST buf);
        }
        xmlXPathFreeObject(xpathObj);

        // screensaver_type
        if (theConfig.screensaver_type == SCREENSAVER_NONE)
        {
            xpathObj = xmlXPathEvalExpression(BAD_CAST "//*[@id='screensaver_none']", xpathCtx);
            if (!xmlXPathNodeSetIsEmpty(xpathObj->nodesetval))
            {
                xmlNode* node = xpathObj->nodesetval->nodeTab[0];
                xmlNewProp(node, BAD_CAST "selected", BAD_CAST "selected");
            }
            xmlXPathFreeObject(xpathObj);
        }
        else if (theConfig.screensaver_type == SCREENSAVER_CLOCK)
        {
            xpathObj = xmlXPathEvalExpression(BAD_CAST "//*[@id='screensaver_clock']", xpathCtx);
            if (!xmlXPathNodeSetIsEmpty(xpathObj->nodesetval))
            {
                xmlNode* node = xpathObj->nodesetval->nodeTab[0];
                xmlNewProp(node, BAD_CAST "selected", BAD_CAST "selected");
            }
            xmlXPathFreeObject(xpathObj);
        }
        else if (theConfig.screensaver_type == SCREENSAVER_BLANK)
        {
            xpathObj = xmlXPathEvalExpression(BAD_CAST "//*[@id='screensaver_blank']", xpathCtx);
            if (!xmlXPathNodeSetIsEmpty(xpathObj->nodesetval))
            {
                xmlNode* node = xpathObj->nodesetval->nodeTab[0];
                xmlNewProp(node, BAD_CAST "selected", BAD_CAST "selected");
            }
            xmlXPathFreeObject(xpathObj);
        }
        else if (theConfig.screensaver_type == SCREENSAVER_PHOTO)
        {
            xpathObj = xmlXPathEvalExpression(BAD_CAST "//*[@id='screensaver_photo']", xpathCtx);
            if (!xmlXPathNodeSetIsEmpty(xpathObj->nodesetval))
            {
                xmlNode* node = xpathObj->nodesetval->nodeTab[0];
                xmlNewProp(node, BAD_CAST "selected", BAD_CAST "selected");
            }
            xmlXPathFreeObject(xpathObj);
        }

        // lang
        if (theConfig.lang == LANG_CHT)
        {
            xpathObj = xmlXPathEvalExpression(BAD_CAST "//*[@id='lang_cht']", xpathCtx);
            if (!xmlXPathNodeSetIsEmpty(xpathObj->nodesetval))
            {
                xmlNode* node = xpathObj->nodesetval->nodeTab[0];
                xmlNewProp(node, BAD_CAST "selected", BAD_CAST "selected");
            }
            xmlXPathFreeObject(xpathObj);
        }
        else if (theConfig.lang == LANG_CHS)
        {
            xpathObj = xmlXPathEvalExpression(BAD_CAST "//*[@id='lang_chs']", xpathCtx);
            if (!xmlXPathNodeSetIsEmpty(xpathObj->nodesetval))
            {
                xmlNode* node = xpathObj->nodesetval->nodeTab[0];
                xmlNewProp(node, BAD_CAST "selected", BAD_CAST "selected");
            }
            xmlXPathFreeObject(xpathObj);
        }
        else if (theConfig.lang == LANG_ENG)
        {
            xpathObj = xmlXPathEvalExpression(BAD_CAST "//*[@id='lang_eng']", xpathCtx);
            if (!xmlXPathNodeSetIsEmpty(xpathObj->nodesetval))
            {
                xmlNode* node = xpathObj->nodesetval->nodeTab[0];
                xmlNewProp(node, BAD_CAST "selected", BAD_CAST "selected");
            }
            xmlXPathFreeObject(xpathObj);
        }

		// wifi mode
        if (theConfig.wifi_mode == WIFI_CLIENT)
        {
            xpathObj = xmlXPathEvalExpression(BAD_CAST "//*[@id='wifi_client']", xpathCtx);
            if (!xmlXPathNodeSetIsEmpty(xpathObj->nodesetval))
            {
                xmlNode* node = xpathObj->nodesetval->nodeTab[0];
                xmlNewProp(node, BAD_CAST "selected", BAD_CAST "selected");
            }
            xmlXPathFreeObject(xpathObj);
        }
        else if (theConfig.wifi_mode == WIFI_SOFTAP)
        {
            xpathObj = xmlXPathEvalExpression(BAD_CAST "//*[@id='wifi_softap']", xpathCtx);
            if (!xmlXPathNodeSetIsEmpty(xpathObj->nodesetval))
            {
                xmlNode* node = xpathObj->nodesetval->nodeTab[0];
                xmlNewProp(node, BAD_CAST "selected", BAD_CAST "selected");
            }
            xmlXPathFreeObject(xpathObj);
        }

        // keylevel
        xpathObj = xmlXPathEvalExpression(BAD_CAST "//*[@id='keylevel']", xpathCtx);
        if (!xmlXPathNodeSetIsEmpty(xpathObj->nodesetval))
        {
            xmlNode* node = xpathObj->nodesetval->nodeTab[0];
            sprintf(buf, "%d", theConfig.keylevel);
            xmlNewProp(node, BAD_CAST "value", BAD_CAST buf);
        }
        xmlXPathFreeObject(xpathObj);

        // keysound_type
		if (strcmp(theConfig.keysound, "key1.wav") == 0)
        {
            xpathObj = xmlXPathEvalExpression(BAD_CAST "//*[@id='key_1']", xpathCtx);
            if (!xmlXPathNodeSetIsEmpty(xpathObj->nodesetval))
            {
                xmlNode* node = xpathObj->nodesetval->nodeTab[0];
                xmlNewProp(node, BAD_CAST "selected", BAD_CAST "selected");
            }
            xmlXPathFreeObject(xpathObj);
        }
        else if (strcmp(theConfig.keysound, "key2.wav") == 0)
        {
            xpathObj = xmlXPathEvalExpression(BAD_CAST "//*[@id='key_2']", xpathCtx);
            if (!xmlXPathNodeSetIsEmpty(xpathObj->nodesetval))
            {
                xmlNode* node = xpathObj->nodesetval->nodeTab[0];
                xmlNewProp(node, BAD_CAST "selected", BAD_CAST "selected");
            }
            xmlXPathFreeObject(xpathObj);
        }
        else if (strcmp(theConfig.keysound, "key3.wav") == 0)
        {
            xpathObj = xmlXPathEvalExpression(BAD_CAST "//*[@id='key_3']", xpathCtx);
            if (!xmlXPathNodeSetIsEmpty(xpathObj->nodesetval))
            {
                xmlNode* node = xpathObj->nodesetval->nodeTab[0];
                xmlNewProp(node, BAD_CAST "selected", BAD_CAST "selected");
            }
            xmlXPathFreeObject(xpathObj);
        }

        // play_lev
        xpathObj = xmlXPathEvalExpression(BAD_CAST "//*[@id='play_lev']", xpathCtx);
        if (!xmlXPathNodeSetIsEmpty(xpathObj->nodesetval))
        {
            xmlNode* node = xpathObj->nodesetval->nodeTab[0];
            sprintf(buf, "%d", theConfig.audiolevel);
            xmlNewProp(node, BAD_CAST "value", BAD_CAST buf);
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
