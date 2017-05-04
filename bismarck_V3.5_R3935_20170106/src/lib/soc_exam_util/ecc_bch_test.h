#ifndef __ECC_BCH_TEST_H__
#define __ECC_BCH_TEST_H__

#include <ecc/ecc_ctrl.h>


extern void ecc_all_one_test(u32_t bch_ability);
extern int ecc_destroy_pattern1(u32_t bch_ability);
extern int ecc_destroy_pattern2(u32_t bch_ability);
extern int ecc_destroy_pattern3(u32_t bch_ability);
extern void bch_encode_decode_test(u32_t bch_ability);
extern void ecc_controller_test(void);

#endif //__ECC_BCH_TEST_H__

