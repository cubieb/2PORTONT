/* $Id: auth2-pam.h,v 1.1.1.1 2003/08/18 05:40:18 kaohj Exp $ */

#include "includes.h"
#ifdef USE_PAM

int	auth2_pam(Authctxt *authctxt);

#endif /* USE_PAM */
