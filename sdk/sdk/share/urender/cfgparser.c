/*
 * cfgparser.c : GeeXboX uShare config file parser.
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
//#include <stdbool.h>
#include <limits.h>

//#include "config.h"
#include "gettext.h"
#include "cfgparser.h"
#include "urender.h"
#include "trace.h"
#include "osdep.h"



static int
ignore_line (const char *line)
{
  int i;
  size_t len;

  /* commented line */
  if (line[0] == '#' )
    return 1;

  len = strlen (line);

  for (i = 0 ; i < (int) len ; i++ )
    if (line[i] != ' ' && line[i] != '\t' && line[i] != '\n')
      return 0;

  return 1;
}

static char *
strdup_trim (const char *s)
{
  size_t begin, end;
  char *r = NULL;

  if (!s)
    return NULL;

  end = strlen (s) - 1;

  for (begin = 0 ; begin < end ; begin++)
    if (s[begin] != ' ' && s[begin] != '\t' && s[begin] != '"')
      break;

  for (; begin < end ; end--)
    if (s[end] != ' ' && s[end] != '\t' && s[end] != '"' && s[end] != '\n')
      break;

  r = strndup (s + begin, end - begin + 1);

  return r;
}

static void
urender_set_name (struct urender_t *ut, const char *name)
{
  if (!ut || !name)
    return;

  if (ut->name)
  {
    free (ut->name);
    ut->name = NULL;
  }

  ut->name = strdup_trim (name);
}

static void
urender_set_interface (struct urender_t *ut, const char *iface)
{
  if (!ut || !iface)
    return;

  if (ut->interface_)
  {
    free (ut->interface_);
    ut->interface_ = NULL;
  }

#if defined(CFG_NET_WIFI)
#if defined(CFG_NET_ETHERNET)
	if ( iteEthGetLink() )
	{
		ut->interface_ = strdup("eth0");
	}
	else
#endif
	{
		ut->interface_ = strdup("wlan0");
	}
#else
  ut->interface_ = strdup_trim (iface);
#endif
}

static void
urender_set_cfg_file (struct urender_t *ut, const char *file)
{
  if (!ut || !file)
    return;

  ut->cfg_file = strdup (file);
}

static void
urender_set_port (struct urender_t *ut, const char *port)
{
  if (!ut || !port)
    return;

  ut->port = atoi (port);
/*  if (ut->port < 49152)
  {
    fprintf (stderr,
             _("Warning: port doesn't fit IANA port assignements.\n"));

    fprintf (stderr, _("Warning: Only Dynamic or Private Ports can be used "
                       "(from 49152 through 65535)\n"));
    ut->port = 0;
  }
  */
}

static void
urender_use_dlna (struct urender_t *ut, const char *val)
{
  if (!ut || !val)
    return;

#ifdef HAVE_DLNA
  ut->dlna_enabled = (!strcmp (val, "yes")) ? true : false;
#endif /* HAVE_DLNA */
}

static void
urender_set_override_iconv_err (struct urender_t *ut, const char *arg)
{
  if (!ut)
    return;

  ut->override_iconv_err = 0;

  if (!strcmp (arg, "yes")
      || !strcmp (arg, "true")
      || !strcmp (arg, "1"))
    ut->override_iconv_err = 1;
}

static u_configline_t configline[] = {
  { URENDER_NAME,                 urender_set_name                },
  { URENDER_IFACE,                urender_set_interface           },
  { URENDER_PORT,                 urender_set_port                },
  { URENDER_OVERRIDE_ICONV_ERR,   urender_set_override_iconv_err  },
  { URENDER_ENABLE_DLNA,          urender_use_dlna                },
  { NULL,                        NULL                           },
};

static void
parse_config_line (struct urender_t *ut, const char *line,
                   u_configline_t *configline)
{
  char *s = NULL;

  s = strchr (line, '=');
  if (s && s[1] != '\0')
  {
    int i;
    for (i=0 ; configline[i].name ; i++)
    {
      if (!strncmp (line, configline[i].name, strlen (configline[i].name)))
      {
        configline[i].set_var (ut, s + 1);
        break;
      }
    }
  }
}

int
parse_config_file (struct urender_t *ut)
{
  char filename[PATH_MAX];
  FILE *conffile;
  char *line = NULL;
  size_t size = 0;
  size_t read;

  if (!ut)
    return -1;

  if (!ut->cfg_file)
    snprintf (filename, PATH_MAX, "%s/%s", SYSCONFDIR, URENDER_CONFIG_FILE);
  else
    snprintf (filename, PATH_MAX, "%s", ut->cfg_file);

  conffile = fopen (filename, "r");
  if (!conffile)
    return -1;

  while ((read = getline (&line, &size, conffile)) != -1)
  {
    if (ignore_line (line))
      continue;

    if (line[read-1] == '\n')
      line[read-1] = '\0';

    while (line[0] == ' ' || line[0] == '\t')
      line++;

    parse_config_line (ut, line, configline);
  }

  fclose (conffile);

  if (line)
    free (line);

  return 0;
}
