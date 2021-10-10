/*
 * presentation.h : GeeXboX uShare UPnP Presentation Page headers.
 * Originally developped for the GeeXboX project.
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

#ifndef _PRESENTATION_H_
#define _PRESENTATION_H_

#define URENDER_PRESENTATION_PAGE "/web/info.html"
#define URENDER_MOBILE_PAGE "/web/mobile.html"
#define PRESENTATION_PAGE_CONTENT_TYPE "text/html"
#define URENDER_CGI "/web/info.cgi"
#define URENDER_DEVMODE_PAGE "/web/devmode.html"
#define URENDER_APLIST_XML "/web/aplist.xml"

int process_cgi(struct urender_t *ut, char *cgiargs);
int build_presentation_page (struct urender_t *ut, char *page_name);
int build_devmode_page (struct urender_t *ut);
int build_aplist_xml(struct urender_t *ut);
int setUrenderIni(char* ptIni,char* ptWeb);

#endif /* _PRESENTATION_H_ */


