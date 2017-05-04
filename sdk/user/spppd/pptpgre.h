/*
 * pptpgre.h
 *
 * Functions to handle the GRE en/decapsulation
 *
 * $Id: pptpgre.h,v 1.1 2012/08/27 07:44:58 ql Exp $
 */

#ifndef _PPTPD_PPTPGRE_H
#define _PPTPD_PPTPGRE_H

#include "if_sppp.h"

extern int pptp_gre_init(struct sppp *sp);

#endif	/* !_PPTPD_PPTPGRE_H */
