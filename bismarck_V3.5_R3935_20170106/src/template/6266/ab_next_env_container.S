#include <soc.h>


  .section .next_env, "a"
  .global next_env
next_env:
#if (BUILDTEST != 1)
  .incbin "../../uboot/u-boot.bin"
#endif
