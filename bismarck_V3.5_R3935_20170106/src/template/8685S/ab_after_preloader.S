#include <soc.h>


  .section .next_env, "a"
  .global next_env
next_env:
  .incbin "../../uboot/u-boot.pimg"
