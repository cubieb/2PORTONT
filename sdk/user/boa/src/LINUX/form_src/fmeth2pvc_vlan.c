/*
 *      Web server handler routines for interface-vlan mapping.
 *
 */

/*-- System inlcude files --*/
#include <signal.h>
#ifdef EMBED
#include <linux/config.h>
#else
#include "../../../../include/linux/autoconf.h"
#endif

/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "utility.h"

#include "options.h"

