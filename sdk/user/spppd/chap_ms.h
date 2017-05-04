/*
 * chap.h - Challenge Handshake Authentication Protocol definitions.
 *
 * Copyright (c) 1995 Eric Rosenquist, Strata Software Limited.
 * http://www.strataware.com/
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms are permitted
 * provided that the above copyright notice and this paragraph are
 * duplicated in all such forms and that any documentation,
 * advertising materials, and other materials related to such
 * distribution and use acknowledge that the software was developed
 * by Eric Rosenquist.  The name of the author may not be used to
 * endorse or promote products derived from this software without
 * specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED ``AS IS'' AND WITHOUT ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, WITHOUT LIMITATION, THE IMPLIED
 * WARRANTIES OF MERCHANTIBILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 *
 * $Id: chap_ms.h,v 1.1 2011/11/30 11:17:05 czpinging Exp $
 */

#ifndef __CHAPMS_INCLUDE__
#include "chap.h"
typedef struct {
    u_char LANManResp[24];
    u_char NTResp[24];
    u_char UseNT;		/* If 1, ignore the LANMan response field */
} MS_ChapResponse;
/* We use MS_CHAP_RESPONSE_LEN, rather than sizeof(MS_ChapResponse),
   in case this struct gets padded. */

typedef struct {
    u_char PeerChallenge[16];
    u_char Reserved[8];
    u_char NTResp[24];
    u_char Flags;
} MS_ChapResponse_v2;

 
# if defined (__GNUC__) || (defined (__STDC__) && __STDC__) || defined (__cplusplus)
#  define	__P(args)	args	/* Use prototypes.  */
#else
#  define	__P(args)	()	/* No prototypes.  */
#endif


void ChapMS __P((struct  sppp  *, char *, int, char *, int));
//int  ChapMS_Resp __P((chap_state *, char *, int, u_char *));
void ChapMS_v2 __P((struct  sppp  *, char *, int, char *, int));
int  ChapMS_v2_Resp __P((struct  sppp  *, char *, int, u_char *, char *));
 

void ChapMS_v2_Auth __P((struct  sppp  *, char *, int, u_char *, char *));

//void ChapMS __P((chap_state *, char *, int, char *, int));
/*

void ChapMS(
    chap_state *cstate,
    char *rchallenge,
    int rchallenge_len,
    char *secret,
    int secret_len
    );
    
int  ChapMS_Resp(
    chap_state *cstate,
        char *secret,
        int secret_len,
        u_char *remmd
);
void ChapMS_v2(struct  sppp *sppp,char *AuthenticatorChallenge,int AuthenticatorChallengeLen,char *Password,int PasswordLen);
int  ChapMS_v2_Resp(
    struct  sppp *,
       char *Password,
       int PasswordLen,
       u_char *remmd,
       char *UserName
       );
       
void ChapMS_v2_Auth(
    struct  sppp *,
        char *Password,
        int  PasswordLen,
        u_char *remmd,
        char *UserName
        );
*/

//int reqchapms(char **);
//int nochapms(char **);
//int reqchapms_v2(char **);
//int nochapms_v2(char **);

#define __CHAPMS_INCLUDE__
#endif /* __CHAPMS_INCLUDE__ */
