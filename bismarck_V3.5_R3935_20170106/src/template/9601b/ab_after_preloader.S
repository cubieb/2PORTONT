#include <soc.h>

#if (BUILDTEST != 1)
#ifndef AFTER_PRELOADER_IMGQ
  #error EE: AFTER_PRELOADER_IMG(Q) is not defined in info.in
#endif
#endif

  .section .next_env, "a"
  .global next_env
next_env:
#if (BUILDTEST != 1)
  .incbin AFTER_PRELOADER_IMGQ
#endif
