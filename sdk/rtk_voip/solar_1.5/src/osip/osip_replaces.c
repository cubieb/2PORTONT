/*
  The oSIP library implements the Session Initiation Protocol (SIP -rfc3261-)
  Copyright (C) 2001,2002,2003  Aymeric MOIZARD jack@atosc.org
            (C) 2005  Vadim Lebedev vadim@mbdsys.com

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

#include "osip_port.h"
#include "osip_message.h"
#include "osip_parser.h"

/* fills the replaces of message.                    */
/* INPUT : const char *hvalue | value of header.    */
/* OUTPUT: osip_message_t *sip | structure to save results.  */
/* returns -1 on error. */
int
osip_message_set_replaces (osip_message_t * sip, const char *hvalue)
{
  int i;

  if (hvalue == NULL || hvalue[0] == '\0')
    return 0;

  if (sip->replaces != NULL)
    return -1;
  i = osip_replaces_init (&(sip->replaces));
  if (i != 0)
    return -1;
  sip->message_property = 2;
  i = osip_replaces_parse (sip->replaces, hvalue);
  if (i != 0)
    {
      osip_replaces_free (sip->replaces);
      sip->replaces = NULL;
      return -1;
    }
  return 0;
}

/* returns the replaces.               */
/* INPUT : osip_message_t *sip | sip message.  */
osip_replaces_t *
osip_message_get_replaces (const osip_message_t * sip)
{
  return sip->replaces;
}

int
osip_replaces_init (osip_replaces_t ** rep)
{
  *rep = (osip_replaces_t *) osip_malloc (sizeof (osip_replaces_t));
  if (*rep == NULL)
    return -1;
  (*rep)->call_id = NULL;

  (*rep)->gen_params = (osip_list_t *) osip_malloc (sizeof (osip_list_t));
  if ((*rep)->gen_params == NULL)
    {
      osip_free (*rep);
      *rep = NULL;
    }
  osip_list_init ((*rep)->gen_params);


  return 0;
}


/* deallocates a osip_replaces_t structure. */
/* INPUT : osip_replaces_t *| rep.       */
void
osip_replaces_free (osip_replaces_t * rep)
{
  if (rep == NULL)
    return;
  osip_call_id_free (rep->call_id);
  osip_generic_param_freelist (rep->gen_params);

  osip_free (rep);
}


extern int
__osip_generic_param_parseall (osip_list_t * gen_params, const char *params);  /* in osip_from.c */

/* fills the replaces structure.                           */
/* INPUT : char *hvalue | value of header.                */
/* OUTPUT: osip_replaces_t *rep | structure to save results.  */
/* returns -1 on error. */

int
osip_replaces_parse (osip_replaces_t * rep, const char *hvalue)
{
  char *params;
  char *copy = osip_strdup(hvalue);
  int i;

  params = strchr(copy, ';');
  if (params)
    {
      if (__osip_generic_param_parseall (rep->gen_params, params) == -1)
        {
	  osip_free(copy);
	  return -1;
        }
      *params = 0;

    }

  osip_call_id_init(&rep->call_id);
  i = osip_call_id_parse(rep->call_id, copy);
  osip_free(copy);
  return i;

}

/* returns the replaces as a string.          */
/* INPUT : osip_replaces_t *rep | rep.  */
/* returns null on error. */
int
osip_replaces_to_str (const osip_replaces_t * rep, char **dest)
{
  char *buf;
  int len;


  *dest = NULL;
  if ((rep == NULL) || (rep->call_id == NULL))
    return -1;

  osip_call_id_to_str(rep->call_id, &buf);
  len = strlen(buf)+3;
  buf = osip_realloc(buf, len);


  {
    int pos = 0;
    osip_generic_param_t *u_param;
    size_t plen;
    char *tmp;

    while (!osip_list_eol (rep->gen_params, pos))
      {
	u_param =
	  (osip_generic_param_t *) osip_list_get (rep->gen_params, pos);

	if (u_param->gvalue == NULL)
	  plen = strlen (u_param->gname) + 2;
	else
	  plen = strlen (u_param->gname) + strlen (u_param->gvalue) + 3;
	len = len + plen;
	buf = (char *) osip_realloc (buf, len);
	tmp = buf;
	tmp = tmp + strlen (tmp);
	if (u_param->gvalue == NULL)
	  sprintf (tmp, ";%s", u_param->gname);
	else
	  sprintf (tmp, ";%s=%s", u_param->gname, u_param->gvalue);
	pos++;
      }
  }

  *dest = buf;

  return 0;
}

osip_call_id_t *
osip_replaces_get_call_id (osip_replaces_t * rep)
{
  if (rep == NULL)
    return NULL;
  return rep->call_id;
}

void
osip_replaces_set_call_id (osip_replaces_t * rep, osip_call_id_t *call_id)
{
  rep->call_id = call_id;
}


int
osip_replaces_clone (const osip_replaces_t * rep, osip_replaces_t ** dest)
{
  int i;
  osip_replaces_t *clone;

  *dest = NULL;
  if (rep == NULL)
    return -1;
  if (rep->call_id == NULL)
    return -1;

  i = osip_replaces_init (&clone);
  if (i == -1)			/* allocation failed */
    return -1;

  if (-1 == osip_call_id_clone(rep->call_id, &clone->call_id))
    {
    osip_replaces_free(clone);
    return -1;
    }


  {
    int pos = 0;
    osip_generic_param_t *u_param;
    osip_generic_param_t *dest_param;

    while (!osip_list_eol (rep->gen_params, pos))
      {
	u_param =
	  (osip_generic_param_t *) osip_list_get (rep->gen_params, pos);
	i = osip_generic_param_clone (u_param, &dest_param);
	if (i != 0)
	  {
	    osip_replaces_free (clone);
	    return -1;
	  }
	osip_list_add (clone->gen_params, dest_param, -1);
	pos++;
      }
  }


  *dest = clone;
  return 0;
}


void osip_replaces_set_to_tag(osip_replaces_t * header, const char *value)
{
  osip_generic_param_add(header->gen_params,osip_strdup("to-tag"),osip_strdup(value));
}


void osip_replaces_set_from_tag(osip_replaces_t * header, const char *value)
{
  osip_generic_param_add(header->gen_params,osip_strdup("from-tag"),osip_strdup(value));
}


char *osip_replaces_get_from_tag (osip_replaces_t * header)
{
  osip_generic_param_t *p;

  osip_generic_param_get_byname((header)->gen_params,"from-tag",&p);
  if (!p)
    return NULL;

  return p->gvalue;

}

char *osip_replaces_get_to_tag (osip_replaces_t * header)
{
  osip_generic_param_t *p;

  osip_generic_param_get_byname((header)->gen_params,"to-tag",&p);
  if (!p)
    return NULL;

  return p->gvalue;

}
