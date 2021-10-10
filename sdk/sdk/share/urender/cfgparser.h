/*
 * cfgparser.c : GeeXboX uShare config file parser headers.
 * Originally developped for the GeeXboX project.
 * Copyright (C) 2005-2007 Alexis Saettler <asbin@asbin.org>
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

#ifndef _CONFIG_PARSER_H_
#define _CONFIG_PARSER_H_

#include "config.h"
#include "urender.h"

#define URENDER_NAME               "URENDER_NAME"
#define URENDER_IFACE              "URENDER_IFACE"
#define URENDER_PORT               "URENDER_PORT"
#define URENDER_TELNET_PORT        "URENDER_TELNET_PORT"
#define URENDER_DIR                "URENDER_DIR"
#define URENDER_OVERRIDE_ICONV_ERR "URENDER_OVERRIDE_ICONV_ERR"
#define URENDER_ENABLE_DLNA        "URENDER_ENABLE_DLNA"

#define URENDER_CONFIG_FILE        CFG_NET_URENDER_CONFIG ".conf"
#define DEFAULT_URENDER_NAME       "ITE UPnP Render"

#if (defined(BSD) || defined(__FreeBSD__))
#define DEFAULT_URENDER_IFACE      "lnc0"
#else /* Linux */
#define DEFAULT_URENDER_IFACE      "eth0"
#endif

int parse_config_file (struct urender_t *ut);

typedef struct {
  char *name;
  void (*set_var) (struct urender_t*, const char*);
} u_configline_t;

#endif /* _CONFIG_PARSER_H_ */
