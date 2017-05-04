#include  <tomcrypt_test.h>

int base64_test(void)
{
   unsigned char in[64], out[256], tmp[64];
   unsigned long x, l1, l2;
   
   for  (x = 0; x < 64; x++) {
       yarrow_read(in, x, &yarrow_prng);
       l1 = sizeof(out);
       DO(base64_encode(in, x, out, &l1));
       l2 = sizeof(tmp);
       DO(base64_decode(out, l1, tmp, &l2));
       if (l2 != x || memcmp(tmp, in, x)) {
           fprintf(stderr, "base64  failed %lu %lu %lu", x, l1, l2);
           return 1;
       }
   }
   return 0;
}

/* $Source: /usr/local/dslrepos/uClinux-dist/user/dropbear-0.48.1/libtomcrypt/testprof/base64_test.c,v $ */
/* $Revision: 1.1 $ */
/* $Date: 2006/06/08 13:51:48 $ */
