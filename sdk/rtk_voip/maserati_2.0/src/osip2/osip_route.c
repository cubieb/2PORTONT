/*
  The oSIP library implements the Session Initiation Protocol (SIP -rfc3261-)
  Copyright (C) 2001,2002,2003,2004,2005,2006,2007 Aymeric MOIZARD jack@atosc.org
  
  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  
  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
  
  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include <stdlib.h>
#include <stdio.h>

#include <osipparser2/osip_port.h>
#include <osipparser2/osip_message.h>
#include <osipparser2/osip_parser.h>

#ifndef MINISIZE
int osip_route_init(osip_route_t ** route)
{
	return osip_from_init((osip_from_t **) route);
}
#endif

/* adds the route header to message.         */
/* INPUT : const char *hvalue | value of header.    */
/* OUTPUT: osip_message_t *sip | structure to save results.  */
/* returns -1 on error. */
int osip_message_set_route(osip_message_t * sip, const char *hvalue)
{
	osip_route_t *route;
	int i;

	if (hvalue == NULL || hvalue[0] == '\0')
		return OSIP_SUCCESS;

#ifdef __VXWORKS_OS__
	i = osip_route_init2(&route);
#else
	i = osip_route_init(&route);
#endif
	if (i != 0)
		return i;
	i = osip_route_parse(route, hvalue);
	if (i != 0) {
		osip_route_free(route);
		return i;
	}
	sip->message_property = 2;
	osip_list_add(&sip->routes, route, -1);
	return OSIP_SUCCESS;
}

#ifndef MINISIZE
/* returns the route header.    */
/* INPUT : osip_message_t *sip | sip message.   */
/* returns null on error. */
int
osip_message_get_route(const osip_message_t * sip, int pos, osip_route_t ** dest)
{
	osip_route_t *route;

	*dest = NULL;
	if (osip_list_size(&sip->routes) <= pos)
		return OSIP_UNDEFINED_ERROR;	/* does not exist */
	route = (osip_route_t *) osip_list_get(&sip->routes, pos);
	*dest = route;
	return pos;
}
#endif

#ifndef MINISIZE
int osip_route_parse(osip_route_t * route, const char *hvalue)
{
	return osip_from_parse((osip_from_t *) route, hvalue);
}

/* returns the route header as a string.          */
/* INPUT : osip_route_t *route | route header.  */
/* returns null on error. */
int osip_route_to_str(const osip_route_t * route, char **dest)
{
	/* we can't use osip_from_to_str(): route and record_route */
	/* always use brackets. */
	return osip_record_route_to_str((osip_record_route_t *) route, dest);
}

/* deallocates a osip_route_t structure.  */
/* INPUT : osip_route_t *route | route header. */
void osip_route_free(osip_route_t * route)
{
	osip_from_free((osip_from_t *) route);
}
#endif

//[SD6, bohungwu, exosip 3.5 integration
int osip_routes_init(osip_list_t **routes)
{
	*routes = (osip_list_t *) osip_malloc (sizeof (osip_list_t));
	return osip_list_init((*routes));
}

int osip_routes_set(osip_list_t *routes, const char *hvalue)
{
	osip_route_t *route;
	int i;

	if (hvalue == NULL || hvalue[0] == '\0')
		return 0;

	i = osip_route_init(&route);
	if (i != 0)
		return -1;

	i = osip_route_parse(route, hvalue);
	if (i != 0)
	{
		osip_route_free(route);
		return -1;
	}

	return osip_list_add(routes, route, -1); // insert route at the end
}

int osip_routes_get(osip_list_t *routes, int pos, osip_route_t **dest)
{
	osip_route_t *route;

	*dest = NULL;
	if (osip_list_size(routes) <= pos)
		return -1;			/* does not exist */

	route = (osip_route_t *) osip_list_get(routes, pos);
	*dest = route;
	return pos;
}

void osip_routes_free(osip_list_t *routes)
{
	int pos;
	osip_route_t *route;

	pos = 0;
	while (!osip_list_eol(routes, pos))
	{
		route = (osip_route_t *) osip_list_get(routes, pos);
		osip_list_remove(routes, pos);
		osip_route_free(route);
	}

	osip_free(routes);
}
//]
#if 1 //service_route test

/* adds the route header to message.         */
/* INPUT : const char *hvalue | value of header.    */
/* OUTPUT: osip_message_t *sip | structure to save results.  */
/* returns -1 on error. */
int
osip_service_route_clone(const osip_service_route_t * service_route,
						 osip_service_route_t ** dest)
{
	int i;
	osip_service_route_t *au;
	printf("osip_service_route_clone\n");
	*dest = NULL;
	if (service_route == NULL)
		return OSIP_BADPARAMETER;

	i = osip_content_length_init(&au);
	if (i == -1)				/* allocation failed */
		return i;
	if (service_route->value != NULL) {
		au->value = osip_strdup(service_route->value);
		printf("osip_service_route_clone au->value=%s\n",au->value);
		if (au->value == NULL) {
			osip_content_length_free(au);
			return OSIP_NOMEM;
		}
	}
	

	*dest = au;
	return OSIP_SUCCESS;
}

int osip_message_set_service_route(osip_message_t * sip, const char *hvalue)
{
	osip_service_route_t *service_route;
	int i;
	printf("osip_message_set_service_route =%s\n", hvalue);
	if (hvalue == NULL || hvalue[0] == '\0')
		return OSIP_SUCCESS;

	i = osip_content_length_init(&service_route);

	if (i != 0)
		return i;
	i = osip_content_length_parse(service_route, hvalue);
	if (i != 0) {
		printf("osip_message_set_service_route i=%d\n",i);
		osip_content_length_free(service_route);
		return i;
	}
	sip->message_property = 2;
	osip_list_add(&sip->service_routes, service_route, -1);
	return OSIP_SUCCESS;
}

int osip_service_routes_to_str(const osip_service_route_t * st, char **dest)
{
	if (st == NULL)
		return OSIP_BADPARAMETER;
	*dest = osip_strdup(st->value);
	if (*dest == NULL)
		return OSIP_NOMEM;
	return OSIP_SUCCESS;
}


void osip_service_routes_free(osip_service_route_t * service_route)
{
printf("osip_service_routes_free\n");
	if (service_route == NULL)
		return;
	osip_free(service_route->value);
	osip_free(service_route);
}
#if 0
int osip_service_routes_set(osip_list_t *routes, const char *hvalue)
{
	osip_route_t *service_route;
	int i;

	if (hvalue == NULL || hvalue[0] == '\0')
		return 0;

	i = osip_service_route_init(&service_route);
	if (i != 0)
		return -1;

	i = osip_service_route_parse(service_route, hvalue);
	if (i != 0)
	{
		osip_service_route_free(service_route);
		return -1;
	}

	return osip_list_add(routes, service_route, -1); // insert route at the end
}

int osip_service_routes_get(osip_list_t *routes, int pos, osip_route_t **dest)
{
	osip_route_t *service_route;

	*dest = NULL;
	if (osip_list_size(routes) <= pos)
		return -1;			/* does not exist */

	service_route = (osip_route_t *) osip_list_get(routes, pos);
	*dest = service_route;
	return pos;
}

void osip_service_routes_free(osip_list_t *routes)
{
	int pos;
	osip_route_t *service_route;

	pos = 0;
	while (!osip_list_eol(routes, pos))
	{
		service_route = (osip_route_t *) osip_list_get(routes, pos);
		osip_list_remove(routes, pos);
		osip_route_free(service_route);
	}

	osip_free(routes);
}
//]
#endif
#endif
