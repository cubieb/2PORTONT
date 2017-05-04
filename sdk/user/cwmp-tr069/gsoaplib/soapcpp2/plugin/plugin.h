/*	plugin.h

	Example gSOAP plug-in

	Copyright (C) 2000-2002 Robert A. van Engelen. All Rights Reserved.

*/

#include "stdsoap2.h"

#define PLUGIN_ID "PLUGIN-1.0" /* plugin identification */

struct plugin_data
{ int (*fsend)(struct soap*, const char*, size_t); /* example: to save and use send callback */
  size_t (*frecv)(struct soap*, char*, size_t); /* example: to save and use recv callback */
};

int plugin(struct soap *soap, struct soap_plugin *plugin, void *arg);
