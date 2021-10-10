/*
 * services.h : GeeXboX uShare UPnP services handler header.
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

#ifndef _SERVICES_H_
#define _SERVICES_H_

#include "urender.h"

#define MAX_VARIABLES 3 // only LastChange
#define SERVICE_AVT  0
#define SERVICE_CMS  1
#define SERVICE_RCS  2

struct service_variables_t {
    char *var_name[MAX_VARIABLES];
    char *var_str[MAX_VARIABLES];
    int var_count;
    int (*init) (void);
};

struct service_action_t {
    char *name;
    int (*function) (struct action_event_t *);
};

struct service_t {
    char *id;
    char *type;
    struct service_variables_t *variables;
    struct service_action_t *actions;
};

#define SERVICE_CONTENT_TYPE "text/xml"

struct service_t *
get_service(int id);

int
find_subscription_service (struct Upnp_Subscription_Request *request,
                           struct service_t **service);

int find_service_action (struct Upnp_Action_Request *request,
                          struct service_t **service,
                          struct service_action_t **action);

int upnp_add_response (struct action_event_t *event,
                        char *key, const char *value);

char * upnp_get_string (struct Upnp_Action_Request *request, const char *key);

int upnp_get_ui4 (struct Upnp_Action_Request *request, const char *key);

int urender_service_init(void);

#endif /* _SERVICES_H_ */
