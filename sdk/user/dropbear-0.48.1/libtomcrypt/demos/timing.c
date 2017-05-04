#include <tomcrypt_test.h>

int main(void)
{
init_timer();
reg_algs();
time_keysched();
time_cipher();
time_cipher2();
time_cipher3();
time_hash();
time_macs();
time_encmacs();
time_prng();
time_mult();
time_sqr();
time_rsa();
time_ecc();
time_dh();
return EXIT_SUCCESS;

}

/* $Source: /usr/local/dslrepos/uClinux-dist/user/dropbear-0.48.1/libtomcrypt/demos/timing.c,v $ */
/* $Revision: 1.1 $ */
/* $Date: 2006/06/08 13:36:24 $ */
