// small demo app that just includes a cipher/hash/prng
#include <tomcrypt.h>

int main(void)
{
   register_cipher(&rijndael_enc_desc);
   register_prng(&yarrow_desc);
   register_hash(&sha256_desc);
   return 0;
}

/* $Source: /usr/local/dslrepos/uClinux-dist/user/dropbear-0.48.1/libtomcrypt/demos/small.c,v $ */
/* $Revision: 1.1 $ */
/* $Date: 2006/06/08 13:36:24 $ */
