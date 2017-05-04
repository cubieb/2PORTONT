#include <tomcrypt_test.h>

int main(void)
{
   int x;
   reg_algs();
   printf("build == \n%s\n", crypt_build_settings);
   printf("\nstore_test...."); fflush(stdout); x = store_test();       printf(x ? "failed" : "passed");if (x) exit(EXIT_FAILURE);
   printf("\ncipher_test..."); fflush(stdout); x = cipher_hash_test(); printf(x ? "failed" : "passed");if (x) exit(EXIT_FAILURE);
   printf("\nmodes_test...."); fflush(stdout); x = modes_test();       printf(x ? "failed" : "passed");if (x) exit(EXIT_FAILURE);
   printf("\nder_test......"); fflush(stdout); x = der_tests();        printf(x ? "failed" : "passed");if (x) exit(EXIT_FAILURE);
   printf("\nmac_test......"); fflush(stdout); x = mac_test();         printf(x ? "failed" : "passed");if (x) exit(EXIT_FAILURE);
   printf("\npkcs_1_test..."); fflush(stdout); x = pkcs_1_test();      printf(x ? "failed" : "passed");if (x) exit(EXIT_FAILURE);
   printf("\nrsa_test......"); fflush(stdout); x = rsa_test();         printf(x ? "failed" : "passed");if (x) exit(EXIT_FAILURE);
   printf("\necc_test......"); fflush(stdout); x = ecc_tests();        printf(x ? "failed" : "passed");if (x) exit(EXIT_FAILURE);
   printf("\ndsa_test......"); fflush(stdout); x = dsa_test();         printf(x ? "failed" : "passed");if (x) exit(EXIT_FAILURE);
   printf("\ndh_test......."); fflush(stdout); x = dh_tests();         printf(x ? "failed" : "passed");if (x) exit(EXIT_FAILURE);
   printf("\n");
   return EXIT_SUCCESS;
}

/* $Source: /usr/local/dslrepos/uClinux-dist/user/dropbear-0.48.1/libtomcrypt/demos/test.c,v $ */
/* $Revision: 1.1 $ */
/* $Date: 2006/06/08 13:36:24 $ */
