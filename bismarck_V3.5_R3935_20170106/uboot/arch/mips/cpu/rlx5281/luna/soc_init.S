#include <preloader.h>
#include <mips-generic.h>

  .section preloader.uc, "ax", @progbits
  .align  4
  .set    noreorder
  .global soc_init
  .ent    soc_init
soc_init:
/* Turn off cache Write allocation */
  mfc0    t0, $20
  li      t1, 0xFFFF7F7F
  and     t0, t0, t1
  mtc0    t0, $20
  nop
  li      t1, 0x00008000 # bit 15 of COP0 reg 20, select 0
  or      t0, t0, t1
  mtc0    t0, $20
  nop

/* Bypass SRAM BIST (Patch obtained from YUMC)
   NOTE: This is a patch for SRAM BIST issue.
   SRAM BIST is only enabled by bonding.
   The patch should be removed after SRAM BIST is disabled by ponding for all ICs */
/* delay 350us */
  li      t0, 3500
1:
  addiu   t0, t0, -1
  bne     zero, t0, 1b
  nop

  li      t0, 0xBB000078
  li      t1, 0x100
  lw      t2, 0(t0)
  and     t2, t2, t1
  bne     zero, t2, 1f
  nop

#Disable PON BIST
  li      t0, 0xBB00007C
  li      t1, 0x3
  sw      t1, 0(t0)
  nop
1: /* skip */

#include "preloader/platform/current/lplr_sram.H"

/* memcpy(SRAM_BASE, preloader_uclma, preloader_sz) */
  la      t0, preloader_sram_start
  li      t1, 0x20000000
  or      t0, t0, t1

  la      t1, UCSRAM_BASE

  la      t2, preloader_sram_size
  addu    t2, t0, t2
1:
  lw      t3, 0(t0)
  addi    t0, t0, 4
  sw      t3, 0(t1)
  bne     t0, t2, 1b
  addi    t1, t1, 4
  nop

  REBASED_CALL(cpu_init, UCSRAM_BASE)

  la      t0, soc_init_in_cache
  jr      t0
  nop
  .end    soc_init

GFUNC(soc_init_in_cache)
  la      sp, c_sp_pos
  lw      sp, 0(sp)

  la      t9, c_start
  jal     t9
  nop

  li      sp, CONFIG_SYS_TEXT_BASE - 8
  la      t9, board_init_f
  j       t9
  nop
END(soc_init_in_cache)
