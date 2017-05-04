/* config.h.  Generated automatically by configure.  */
/*
 * "$Id: config.h.in,v 1.1 2011/02/22 15:28:56 cathy Exp $"
 *
 *   Configuration file for the Common UNIX Printing System (CUPS).
 *
 *   Copyright 1997-2000 by Easy Software Products.
 *
 *   These coded instructions, statements, and computer programs are the
 *   property of Easy Software Products and are protected by Federal
 *   copyright law.  Distribution and use rights are outlined in the file
 *   "LICENSE.txt" which should have been included with this file.  If this
 *   file is missing or damaged please contact Easy Software Products
 *   at:
 *
 *       Attn: CUPS Licensing Information
 *       Easy Software Products
 *       44141 Airport View Drive, Suite 204
 *       Hollywood, Maryland 20636-3111 USA
 *
 *       Voice: (301) 373-9603
 *       EMail: cups-info@cups.org
 *         WWW: http://www.cups.org
 */

/*
 * Version of software...
 */

#define CUPS_SVERSION	"CUPS v1.0.5"

/*
 * Where are files stored?
 */

#define CUPS_LOCALEDIR	"/usr/share/locale"
#define CUPS_SERVERROOT	"/var/cups"
#define CUPS_DATADIR	"/usr/share/cups"

/*
 * Do we have various image libraries?
 */

/* #undef HAVE_LIBPNG */
/* #undef HAVE_LIBZ */
/* #undef HAVE_LIBJPEG */
/* #undef HAVE_LIBTIFF */

/*
 * Does this machine store words in big-endian (MSB-first) order?
 */

#define WORDS_BIGENDIAN 1

/*
 * Which directory functions and headers do we use?
 */

#define HAVE_DIRENT_H 1
/* #undef HAVE_SYS_DIR_H */
/* #undef HAVE_SYS_NDIR_H */
/* #undef HAVE_NDIR_H */

/*
 * Do we have PAM stuff?
 */

#ifndef HAVE_LIBPAM
#define HAVE_LIBPAM 0
#endif /* !HAVE_LIBPAM */

/*
 * Do we have <shadow.h>?
 */

#define HAVE_SHADOW_H 1

/*
 * Do we have <crypt.h>?
 */

#define HAVE_CRYPT_H 1

/*
 * Do we have the strXXX() functions?
 */

#define HAVE_STRDUP 1
#define HAVE_STRCASECMP 1
#define HAVE_STRNCASECMP 1

/*
 * Do we have the (v)snprintf() functions?
 */

#define HAVE_SNPRINTF 1
#define HAVE_VSNPRINTF 1

/*
 * What signal functions to use?
 */

#define HAVE_SIGSET 1
#define HAVE_SIGACTION 1

/*
 * What wait functions to use?
 */

#define HAVE_WAITPID 1
#define HAVE_WAIT3 1

/*
 * End of "$Id: config.h.in,v 1.1 2011/02/22 15:28:56 cathy Exp $".
 */
