
/*
linphone
Copyright (C) 2000  Simon MORLAT (simon.morlat@free.fr)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include "uglib.h"

#if defined(HAVE_DLFCN_H) && defined(HAVE_DLOPEN)
#include <dlfcn.h>

typedef int (*function_t) (void);

static int no_artsc = 0;
static void *artsc_handle;
static function_t __arts_init = NULL;
static function_t __arts_free = NULL;
static function_t __arts_suspend = NULL;
static function_t __arts_suspended = NULL;
static int suspended = 0;

int
load_artsc ()
{
	artsc_handle = dlopen ("libartsc.so.0", RTLD_NOW);
	if (artsc_handle == NULL)
	{
		no_artsc = 1;
		g_message ("Could not load libartsc.so.0\n");
		return -1;
	}
	__arts_init = dlsym (artsc_handle, "arts_init");
	if (__arts_init == NULL)
	{
		g_warning ("Could not resolv arts_init\n");
		return -1;
	}
	__arts_free = dlsym (artsc_handle, "arts_free");
	if (__arts_free == NULL)
	{
		g_warning ("Could not resolv arts_free\n");
		return -1;
	}
	__arts_suspend = dlsym (artsc_handle, "arts_suspend");
	if (__arts_suspend == NULL)
	{
		g_warning ("Could not resolv arts_suspend\n");
		return -1;
	}
	__arts_suspended = dlsym (artsc_handle, "arts_suspended");
	if (__arts_suspended == NULL)
	{
		g_warning ("Could not resolv arts_suspended\n");
		return -1;
	}
	return 0;
}

int
linphone_arts_suspend ()
{
	if (no_artsc == 0)
	{
		if (__arts_init == NULL)
		{
			/*try to dlopen libartsc.so */
			if (load_artsc () < 0)
				return -1;
		}
	}
	else
		return -1;	/* we are unable to suspend arts */
	__arts_init ();
	__arts_suspend ();
	if (__arts_suspended () == 0)
		return -1;
	suspended = 1;
	return 0;
}

int
linphone_arts_restore ()
{
	if (__arts_free != NULL)
	{
		if (suspended==0) return -1;
		__arts_free ();
		return 0;
	}
	return -1;
}


#else
int
linphone_arts_suspend ()
{
	return -1;
}

int
linphone_arts_restore ()
{
	return -1;
}

#endif
