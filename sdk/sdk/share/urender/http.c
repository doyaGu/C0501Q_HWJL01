/*
 * http.c : GeeXboX uShare Web Server handler.
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

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
//#include <unistd.h>
#include <errno.h>
#include <signal.h>

#include "upnp.h"
#include "upnptools.h"

#include "services.h"
#include "avt.h"
#include "cms.h"
#include "rcs.h"
#include "http.h"
#include "minmax.h"
#include "trace.h"
#include "presentation.h"
#include "osdep.h"
#include "mime.h"
#include "buffer.h"
#include "urender.h"
#include "ite/itp.h"


#define PROTOCOL_TYPE_PRE_SZ  11   /* for the str length of "http-get:*:" */
#define PROTOCOL_TYPE_SUFF_SZ 2    /* for the str length of ":*" */

struct web_file_t {
    char *fullpath;
    off_t pos;

    enum {
        FILE_LOCAL,
        FILE_MEMORY,
        FILE_UPGRADE,
    } type;
    union {
        struct {
            FILE *fd;
            struct upnp_entry_t *entry;
        } local;
        struct {
            char *contents;
            off_t len;
        } memory;
    } detail;
};


static void
set_info_file (struct File_Info *info, const size_t length,
               const char *content_type)
{
    info->file_length = length;
    info->last_modified = 0;
    info->is_directory = 0;
    info->is_readable = 1;
    info->content_type = ixmlCloneDOMString (content_type);
}

static int
http_get_info (const char *filename, struct File_Info *info)
{
	extern struct urender_t *ut;
    int len = 0;

    printf("[uRender HTTP] http_get_info() Begin filename=%s\r\n", filename);

    if (!filename || !info)
        return -1;

    log_verbose ("http_get_info, filename : %s\n", filename);

    if (!strcmp (filename, DESCRIPTION_LOCATION))
    {
        set_info_file (info, urender_get_description_len(), SERVICE_CONTENT_TYPE);
        return 0;
    }

    if (!strcmp (filename, AVT_LOCATION))
    {
        set_info_file (info, AVT_DESCRIPTION_LEN, SERVICE_CONTENT_TYPE);
        return 0;
    }

    if (!strcmp (filename, CMS_LOCATION))
    {
        set_info_file (info, CMS_DESCRIPTION_LEN, SERVICE_CONTENT_TYPE);
        return 0;
    }

    if (!strcmp (filename, RCS_LOCATION))
    {
        set_info_file (info, RCS_DESCRIPTION_LEN, SERVICE_CONTENT_TYPE);
        return 0;
    }

	if (ut->use_presentation && (!strcmp (filename, URENDER_PRESENTATION_PAGE) || !strcmp (filename, "/")))
	{
        printf("[uRender HTTP] PresentationPage filename=%s\r\n", filename);
		if (build_presentation_page (ut, URENDER_PRESENTATION_PAGE) < 0)
		{
            printf("[uRender HTTP]%s() L#%ld: [ERROR] build presentation page fail!\r\n", __FUNCTION__, __LINE__);
			return -1;
		}

		set_info_file (info, ut->presentation->len, PRESENTATION_PAGE_CONTENT_TYPE);
		return 0;
	}

	if (ut->use_presentation && !strcmp (filename, URENDER_DEVMODE_PAGE))
	{
		if (build_devmode_page (ut) < 0)
			return -1;

		set_info_file (info, ut->presentation->len, PRESENTATION_PAGE_CONTENT_TYPE);
		return 0;
	}

	if (ut->use_presentation && !strcmp (filename, URENDER_MOBILE_PAGE))
	{
        printf("[uRender HTTP] PresentationPage filename=%s\r\n", filename);
		if (build_presentation_page (ut, URENDER_MOBILE_PAGE) < 0)
		{
            printf("[uRender HTTP]%s() L#%ld: [ERROR] build presentation page fail!\r\n", __FUNCTION__, __LINE__);
			return -1;
		}

		set_info_file (info, ut->presentation->len, PRESENTATION_PAGE_CONTENT_TYPE);
		return 0;
	}

	if (ut->use_presentation && !strncmp (filename, URENDER_CGI, strlen (URENDER_CGI)))
	{
        printf("[uRender HTTP] CGI filename=%s\r\n", filename);
		if (process_cgi (ut, (char *) (filename + strlen (URENDER_CGI) + 1)) < 0)
		{
            printf("[uRender HTTP]%s() L#%ld: [ERROR] process cgi fail!\r\n", __FUNCTION__, __LINE__);
			return -1;
		}

        if (ut->presentation)
        {
		    set_info_file (info, ut->presentation->len, PRESENTATION_PAGE_CONTENT_TYPE);
        }

		return 0;
	}

	if (ut->use_presentation && !strncmp (filename, URENDER_APLIST_XML, strlen (URENDER_APLIST_XML)))
	{
        printf("[uRender HTTP] APListXML filename=%s\r\n", filename);
		if (build_aplist_xml (ut) < 0)
		{
            printf("[uRender HTTP]%s() L#%ld: [ERROR] build_aplist_page() fail!\r\n", __FUNCTION__, __LINE__);
			return -1;
		}

        if (ut->presentation)
        {
		    set_info_file (info, ut->presentation->len, SERVICE_CONTENT_TYPE);
        }

		return 0;
	}

    len = strlen(filename);
    if (len > 4)
    {
	    if ( strncmp(&filename[len - 4], ".css", 4) == 0  )
	    {
		    set_info_file(info, -1, "text/css");
		    return 0;
	    }

	    if ( strncmp(&filename[len - 4], ".gif", 4) == 0  )
	    {
		    set_info_file(info, -1, "image/gif");
		    return 0;
	    }

	    if ( strncmp(&filename[len - 5], ".jpeg", 5) == 0 || strncmp(&filename[len - 4], ".jpg", 4) == 0 )
	    {
		    set_info_file(info, -1, "image/jpeg");
		    return 0;
	    }

	    if ( strncmp(&filename[len - 4], ".png", 4) == 0  )
	    {
		    set_info_file(info, -1, "image/png");
		    return 0;
	    }

	    if ( strncmp(&filename[len - 4], ".ico", 4) == 0  )
	    {
		    set_info_file(info, -1, "image/ico");
		    return 0;
	    }

        if ( strncmp(&filename[len - 3], ".js", 3) == 0  )
	    {
		    set_info_file(info, -1, "text/javascript");
		    return 0;
	    }
    }

    return 0;
}

static UpnpWebFileHandle
get_file_memory (const char *fullpath, const char *description,
                 const size_t length)
{
    struct web_file_t *file;

    file = malloc (sizeof (struct web_file_t));
    if (fullpath) {
        file->fullpath = strdup (fullpath);
    } else {
        file->fullpath = NULL;
    }
    file->pos = 0;
    file->type = FILE_MEMORY;
    if (description) {
        file->detail.memory.contents = strdup (description);
    } else {
        file->detail.memory.contents = NULL;
    }
    file->detail.memory.len = length;

    return ((UpnpWebFileHandle) file);
}


extern size_t upnp_content_length;
extern void HandleSigInt(int sigraised);

static UpnpWebFileHandle
http_open (
	struct SOCK_INFO*	  sockInfo,
	const char*           filename,
	enum UpnpOpenFileMode mode)
{
	extern struct urender_t *ut;

	struct upnp_entry_t* entry          = NULL;
	struct web_file_t*	 file           = NULL;
    FILE*                fd             = 0;
    int                  len = 0;

    if (!filename)
        return NULL;

    log_verbose ("http_open, filename : %s\n", filename);

#ifdef CFG_UPGRADE_FILENAME
    if ( mode == UPNP_WRITE
        && ut->use_presentation
        && !strncmp(filename, URENDER_CGI, strlen (URENDER_CGI)) )
    {
        file = malloc (sizeof (struct web_file_t));
        if (!file)
        {
    	    return NULL;
        }

        file->fullpath = strdup (CFG_PRIVATE_DRIVE ":/" CFG_UPGRADE_FILENAME);
        file->pos = 0;

#ifdef CFG_UPGRADE_ONLINE
        file->type = FILE_UPGRADE;
        file->detail.memory.contents = malloc(upnp_content_length);
        if (file->detail.memory.contents == NULL)
        {
            printf("out of memory: %d\n", upnp_content_length);
            return NULL;
        }
        file->detail.memory.len = upnp_content_length;
        upnp_content_length = 0;
        ut->upgrading = true;
        //raise(SIGINT);

    #ifdef CFG_NET_SHAIRPORT_DACP
    #else
        HandleSigInt(SIGINT);
    #endif
#else
        fd = fopen (CFG_PRIVATE_DRIVE ":/" CFG_UPGRADE_FILENAME, "w");
        if (fd == NULL)
        {
    	    return NULL;
        }

        file->type = FILE_LOCAL;
        file->detail.local.entry = entry;
        file->detail.local.fd = fd;
#endif // CFG_UPGRADE_ONLINE

        return ((UpnpWebFileHandle)file);
    }
#endif // CFG_UPGRADE_FILENAME

    if (mode != UPNP_READ)
        return NULL;

    if (!strcmp (filename, DESCRIPTION_LOCATION))
        return get_file_memory (DESCRIPTION_LOCATION, urender_get_description(), urender_get_description_len());

    if (!strcmp (filename, AVT_LOCATION))
        return get_file_memory (AVT_LOCATION, AVT_DESCRIPTION, AVT_DESCRIPTION_LEN);

    if (!strcmp (filename, CMS_LOCATION))
        return get_file_memory (CMS_LOCATION, CMS_DESCRIPTION, CMS_DESCRIPTION_LEN);

    if (!strcmp (filename, RCS_LOCATION))
        return get_file_memory (RCS_LOCATION, RCS_DESCRIPTION, RCS_DESCRIPTION_LEN);

	if (ut->use_presentation
	    && (   !strcmp(filename, URENDER_PRESENTATION_PAGE)
	        || !strcmp(filename, "/")
	        || !strncmp(filename, URENDER_CGI, strlen (URENDER_CGI))) )
	{
		return get_file_memory(URENDER_PRESENTATION_PAGE, ut->presentation->buf, ut->presentation->len);
	}

	if (ut->use_presentation && !strcmp(filename, URENDER_MOBILE_PAGE))
	{
		return get_file_memory(URENDER_MOBILE_PAGE, ut->presentation->buf, ut->presentation->len);
	}

    if (   ut->use_presentation
	    && (   !strcmp(filename, URENDER_DEVMODE_PAGE)) )
	{
		return get_file_memory(URENDER_DEVMODE_PAGE, ut->presentation->buf, ut->presentation->len);
	}

    if (ut->use_presentation && !strncmp(filename, URENDER_APLIST_XML, strlen(URENDER_APLIST_XML)))
    {
		return get_file_memory(URENDER_APLIST_XML, ut->presentation->buf, ut->presentation->len);
    }

    len = strlen(filename);
    if (len > 4)
    {
	    if ( !strcmp(&filename[len - 4], ".css") || !strcmp(&filename[len - 4], ".gif") || !strcmp(&filename[len - 5], ".jpeg") || !strcmp(&filename[len - 4], ".jpg") || !strcmp(&filename[len - 4], ".png")  || !strcmp(&filename[len - 4], ".ico") || !strcmp(&filename[len - 3], ".js") )
	    {
            char* fullpath = malloc(len + 3);
            if (!fullpath)
                return NULL;

#ifdef CFG_USE_SD_INI
            strcpy(fullpath, "C:");
#else
            strcpy(fullpath, CFG_PRIVATE_DRIVE ":");
#endif
            strcat(fullpath, strrchr(filename, '/'));
	        fd = fopen (fullpath, "r");
	        if (fd == NULL)
	        {
                free(fullpath);
                fullpath = NULL;
		        return NULL;
	        }

	        file = malloc (sizeof (struct web_file_t));
            if (!file)
            {
                fclose(fd);
                free(fullpath);
                fullpath = NULL;
		        return NULL;
            }
	        file->fullpath = fullpath;
	        file->pos = 0;
	        file->type = FILE_LOCAL;
	        file->detail.local.entry = entry;
	        file->detail.local.fd = fd;

	        return ((UpnpWebFileHandle)file);
	    }
    }

    return NULL;
}

static int
http_read (UpnpWebFileHandle fh, char *buf, size_t buflen)
{
    struct web_file_t *file = (struct web_file_t *) fh;
    size_t len = -1;

    log_verbose ("http_read\n", 0);

    if (!file)
        return -1;

    switch (file->type)
    {
    case FILE_LOCAL:
        log_verbose ("Read local file.\n", 0);
        len = fread (buf, 1, buflen, file->detail.local.fd);
        break;
    case FILE_MEMORY:
        log_verbose ("Read file from memory.\n",0);
        len = (size_t) MIN (buflen, file->detail.memory.len - file->pos);
        memcpy (buf, file->detail.memory.contents + file->pos, (size_t) len);
        break;
    default:
        log_verbose ("Unknown file type.\n",0);
        break;
    }

    if (len >= 0)
        file->pos += len;

    log_verbose ("Read %zd bytes.\n", len);

    return len;
}

static int
http_write (
	UpnpWebFileHandle fh __attribute__((unused)),
	char*             buf __attribute__((unused)),
	size_t            buflen __attribute__((unused)))
{
    struct web_file_t *file = (struct web_file_t *) fh;
    ssize_t len = -1;
    char *ptr, *ptr2 = NULL;
    size_t len2;
    static char boundary[128];
    static size_t boundary_len;

    log_verbose ("http write\n",0);

	if (!file)
		return -1;

	switch (file->type)
	{
    case FILE_UPGRADE:
		log_verbose ("Write file to memory.\n");
        if (file->pos == EOF)
            break;

        if (file->pos == 0)
        {
            strcpy(boundary, "\r\n");
            ptr = strstr(buf, "\r\n");
            boundary_len = ptr - buf;
            strncat(boundary, buf, boundary_len);
            ptr = strstr(ptr, "\r\n\r\n");
            ptr += 4;
            len2 = buflen - (ptr - buf);
        }
        else
        {
            ptr = buf;
            len2 = buflen;
        }

        ptr2 = memmem(ptr, len2, boundary, boundary_len);
        if (ptr2)
        {
            len2 = (ptr2 - ptr);
        }

        len = (size_t) MIN (len2, (size_t)(file->detail.memory.len - file->pos));
		memcpy (file->detail.memory.contents + file->pos, ptr, (size_t) len);
        upnp_content_length += len;
        if (ptr2)
        {
            ptr = memmem(file->detail.memory.contents, upnp_content_length, boundary, boundary_len);
            if (ptr)
                upnp_content_length = (ptr - file->detail.memory.contents);
        }
        printf("[uRender HTTP]%s() L#%ld: upnp_content_length = %d\r\n", __FUNCTION__, __LINE__, upnp_content_length);
        break;

	default:
		log_verbose ("Unknown file type.\n");
		break;
    }

	if (len >= 0)
		file->pos += len;

    log_verbose ("Write %zd bytes.\n", len);

    if (ptr2)
        file->pos = EOF;

#if defined(__OPENRTOS__)
        usleep(1000);
#endif

	return buflen;
}

static int
http_seek (UpnpWebFileHandle fh, off_t offset, int origin)
{
    struct web_file_t *file = (struct web_file_t *) fh;
    off_t newpos = -1;

    log_verbose ("http_seek\n",0);

    if (!file)
        return -1;

    switch (origin)
    {
    case SEEK_SET:
        log_verbose ("Attempting to seek to %lld (was at %lld) in %s\n",
            offset, file->pos, file->fullpath);
        newpos = offset;
        break;
    case SEEK_CUR:
        log_verbose ("Attempting to seek by %lld from %lld in %s\n",
            offset, file->pos, file->fullpath);
        newpos = file->pos + offset;
        break;
    case SEEK_END:
        log_verbose ("Attempting to seek by %lld from end (was at %lld) in %s\n",
            offset, file->pos, file->fullpath);

        if (file->type == FILE_LOCAL)
        {
            struct stat sb;
            if (stat (file->fullpath, &sb) < 0)
            {
                log_verbose ("%s: cannot stat: %s\n",
                    file->fullpath, strerror (errno));
                return -1;
            }
            newpos = sb.st_size + offset;
        }
        else if (file->type == FILE_MEMORY)
            newpos = file->detail.memory.len + offset;
        break;
    }

    switch (file->type)
    {
    case FILE_LOCAL:
        /* Just make sure we cannot seek before start of file. */
        if (newpos < 0)
        {
            log_verbose ("%s: cannot seek: %s\n", file->fullpath, strerror (EINVAL));
            return -1;
        }

        /* Don't seek with origin as specified above, as file may have
        changed in size since our last stat. */
        if (fseek (file->detail.local.fd, newpos, SEEK_SET) == -1)
        {
            log_verbose ("%s: cannot seek: %s\n", file->fullpath, strerror (errno));
            return -1;
        }
        break;
    case FILE_MEMORY:
        if (newpos < 0 || newpos > file->detail.memory.len)
        {
            log_verbose ("%s: cannot seek: %s\n", file->fullpath, strerror (EINVAL));
            return -1;
        }
        break;
    }

    file->pos = newpos;

    return 0;
}


char* urender_upgrade_content;


static int
http_close (
	struct SOCK_INFO* sockInfo,
	UpnpWebFileHandle fh)
{
    struct web_file_t *file = (struct web_file_t *) fh;

    log_verbose ("http_close\n",0);

    if (!file)
        return -1;

    switch (file->type)
    {
    case FILE_LOCAL:
        fclose (file->detail.local.fd);
#if defined(CFG_NOR_ENABLE) && CFG_NOR_CACHE_SIZE > 0
        ioctl(ITP_DEVICE_WATCHDOG, ITP_IOCTL_DISABLE, NULL);
        ioctl(ITP_DEVICE_NOR, ITP_IOCTL_FLUSH, NULL);
        ioctl(ITP_DEVICE_WATCHDOG, ITP_IOCTL_ENABLE, NULL);
#endif
        break;
    case FILE_MEMORY:
        /* no close operation */
        if (file->detail.memory.contents)
            free (file->detail.memory.contents);
        break;
    case FILE_UPGRADE:
		if (upnp_content_length > 0 && file->detail.memory.contents)
        {
            printf("[Dav]%s() L#%ld: F1\r\n", __FUNCTION__, __LINE__);
            free (urender_upgrade_content);
		    urender_upgrade_content = file->detail.memory.contents;
        }
        break;
    default:
        log_verbose ("Unknown file type.\n",0);
        break;
    }

    if (file->fullpath)
        free (file->fullpath);
    free (file);

    return 0;
}

int
SetVirtualDirCallbacks(void)
{
    int res = 0;
    res = UpnpVirtualDir_set_GetInfoCallback(http_get_info);
    if(res)
        return res;
    res = UpnpVirtualDir_set_OpenCallback(http_open);
    if(res)
        return res;
    res = UpnpVirtualDir_set_ReadCallback(http_read);
    if(res)
        return res;
    res = UpnpVirtualDir_set_WriteCallback(http_write);
    if(res)
        return res;
    res = UpnpVirtualDir_set_SeekCallback(http_seek);
    if(res)
        return res;
    res = UpnpVirtualDir_set_CloseCallback(http_close);
    if(res)
        return res;
    return 0;
}
