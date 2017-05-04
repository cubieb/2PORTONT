/*
  The oSIP library implements the Session Initiation Protocol (SIP -rfc2543-)
  Copyright (C) 2001  Aymeric MOIZARD jack@atosc.org
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


#ifndef _OSIP_REPLACES_H_
#define _OSIP_REPLACES_H_

/**
 * @file osip_replaces.h
 * @brief oSIP osip_replaces header definition.
 */

/**
 * @defgroup oSIP_REPLACES oSIP replaces header definition.
 * @ingroup oSIP_HEADERS
 * @{
 */

/**
 * Structure for replaces headers.
 * @var osip_replaces_t
 */
  typedef struct osip_replaces osip_replaces_t;

/**
 * Definition of the Call-Id header.
 * @struct osip_replaces
 */
  struct osip_replaces
  {
    osip_call_id_t  *call_id;
    osip_list_t     *gen_params;
  };

#ifdef __cplusplus
extern "C"
{
#endif

/**
 * Allocate a Replaces element.
 * @param header The element to work on.
 */
  int osip_replaces_init (osip_replaces_t ** header);
/**
 * Free a Replaces element.
 * @param header The element to work on.
 */
  void osip_replaces_free (osip_replaces_t * header);
/**
 * Parse a Replaces element.
 * @param header The element to work on.
 * @param hvalue The string to parse.
 */
  int osip_replaces_parse (osip_replaces_t * header, const char *hvalue);
/**
 * Get a string representation of a Replaces element.
 * @param header The element to work on.
 * @param dest A pointer on the new allocated string.
 */
  int osip_replaces_to_str (const osip_replaces_t * header, char **dest);
/**
 * Clone a Replaces element.
 * @param header The element to work on.
 * @param dest A pointer on the copy of the element.
 */
  int osip_replaces_clone (const osip_replaces_t * header, osip_replaces_t ** 
dest);
/**
 * Set the Call-Id in the Replaces element.
 * @param header The element to work on.
 * @param value The value of the element.
 */
  void osip_replaces_set_call_id (osip_replaces_t * header, osip_call_id_t *
value);
/**
 * Get the Call-Id from a Replaces header.
 * @param header The element to work on.
 */
  osip_call_id_t *osip_replaces_get_call_id(osip_replaces_t * header);
/**
 * Set the to_tag value in the Replaces element.
 * @param header The element to work on.
 * @param value The value of the element.
 */
  void osip_replaces_set_to_tag(osip_replaces_t * header, const char *value);
/**
 * Set the from_tag value in the Replaces element.
 * @param header The element to work on.
 * @param value The value of the element.
 */
  void osip_replaces_set_from_tag(osip_replaces_t * header, const char *value);
/**
 * Get the from_tag from a Replaces header.
 * @param header The element to work on.
 */
  char *osip_replaces_get_from_tag (osip_replaces_t * header);

/**
 * Get the to_tag from a Replaces header.
 * @param header The element to work on.
 */
  char *osip_replaces_get_to_tag (osip_replaces_t * header);



#ifdef __cplusplus
}
#endif

/** @} */

#endif

