cmd_drivers/net/rtl86900/sdk/src//dal/rtl9602c/dal_rtl9602c_gpio.o := /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/bin/rsdk-linux-gcc -Wp,-MD,drivers/net/rtl86900/sdk/src//dal/rtl9602c/.dal_rtl9602c_gpio.o.d  -nostdinc -isystem /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/bin/../lib/gcc/mips-linux/4.4.6/include -Iinclude  -I/home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include -include include/linux/autoconf.h -Iinclude/soc -include /home/gangadhar/2PORTONT/sdk/autoconf.h -include include/soc/luna_cfg.h -D__KERNEL__ -D"VMLINUX_LOAD_ADDRESS=0x80000000" -D"LOADADDR=0x80000000" -D"DATAOFFSET=0" -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -fno-delete-null-pointer-checks -D__LUNA_KERNEL__ -Os -fno-inline -ffunction-sections -mno-check-zero-division -mabi=32 -G 0 -mno-abicalls -fno-pic -pipe -msoft-float -ffreestanding -EB -UMIPSEB -U_MIPSEB -U__MIPSEB -U__MIPSEB__ -UMIPSEL -U_MIPSEL -U__MIPSEL -U__MIPSEL__ -DMIPSEB -D_MIPSEB -D__MIPSEB -D__MIPSEB__ -Iinclude/asm-rlx -Iarch/rlx/bsp_rtl8686/ -Idrivers/net/rtl86900/sdk/include/ -Idrivers/net/rtl86900/sdk/system/include -I/home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/mach-generic -Wframe-larger-than=1024 -fno-stack-protector -fomit-frame-pointer -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fno-dwarf2-cfi-asm -DCONFIG_GPON_VERSION=2 -DRTK_X86_CLE -DNO_MIB_SEPARATE -DCONFIG_SOC_DEPEND_FEATURE -DCONFIG_SOC_MODE -DCONFIG_BEN_SINGLE_END -DCONFIG_CLASSFICATION_FEATURE -DCONFIG_GPON_VERSION=2 -DRTK_X86_CLE -DNO_MIB_SEPARATE -DCONFIG_SOC_DEPEND_FEATURE -DCONFIG_SOC_MODE -DCONFIG_SDK_RTL9602C -DCONFIG_BEN_SINGLE_END -DCONFIG_CLASSFICATION_FEATURE -DCONFIG_GPON_VERSION=2 -DRTK_X86_CLE -DNO_MIB_SEPARATE -DCONFIG_SOC_DEPEND_FEATURE -DCONFIG_SOC_MODE -DCONFIG_SDK_KERNEL_LINUX -DCONFIG_SDK_RTL9602C -DCONFIG_BEN_SINGLE_END -DCONFIG_CLASSFICATION_FEATURE -I/home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/include -I/home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/system/include -I/home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/include/hal/chipdef/apollo -I/home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net -I/home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/src/module/pkt_redirect -I/home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/include/hal/chipdef/rtl9602c   -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(dal_rtl9602c_gpio)"  -D"KBUILD_MODNAME=KBUILD_STR(rtk)"  -c -o drivers/net/rtl86900/sdk/src//dal/rtl9602c/dal_rtl9602c_gpio.o drivers/net/rtl86900/sdk/src//dal/rtl9602c/dal_rtl9602c_gpio.c

deps_drivers/net/rtl86900/sdk/src//dal/rtl9602c/dal_rtl9602c_gpio.o := \
  drivers/net/rtl86900/sdk/src//dal/rtl9602c/dal_rtl9602c_gpio.c \
  /home/gangadhar/2PORTONT/sdk/autoconf.h \
    $(wildcard include/config/defaults/realtek/luna.h) \
    $(wildcard include/config/luna/dual/linux.h) \
    $(wildcard include/config/defaults/kernel/2/6.h) \
    $(wildcard include/config/kernel/2/6/19.h) \
    $(wildcard include/config/kernel/2/6/30.h) \
    $(wildcard include/config/use/rsdk/wrapper.h) \
    $(wildcard include/config/rsdk/dir.h) \
    $(wildcard include/config/defaults/kernel.h) \
    $(wildcard include/config/defaults/vendor.h) \
    $(wildcard include/config/mconf/bzbox.h) \
    $(wildcard include/config/image/tiny.h) \
    $(wildcard include/config/voip/ipc/dsp/architecture.h) \
    $(wildcard include/config/plr/none.h) \
    $(wildcard include/config/use/preloader/parameters.h) \
    $(wildcard include/config/plr/3x.h) \
    $(wildcard include/config/luna.h) \
    $(wildcard include/config/luna/memory/auto/detection.h) \
    $(wildcard include/config/prd/signature.h) \
    $(wildcard include/config/prj/signature.h) \
  include/soc/luna_cfg.h \
  drivers/net/rtl86900/sdk/include/dal/rtl9602c/dal_rtl9602c.h \
  drivers/net/rtl86900/sdk/include/common/rt_type.h \
  drivers/net/rtl86900/sdk/system/include/common/type.h \
  drivers/net/rtl86900/sdk/include/common/rt_error.h \
  drivers/net/rtl86900/sdk/system/include/common/error.h \
  drivers/net/rtl86900/sdk/include/common/util/rt_bitop.h \
  drivers/net/rtl86900/sdk/include/common/util/rt_util.h \
    $(wildcard include/config/sdk/kernel/linux.h) \
  drivers/net/rtl86900/sdk/system/include/common/util.h \
  drivers/net/rtl86900/sdk/system/include/osal/lib.h \
  include/linux/ctype.h \
  include/linux/kernel.h \
    $(wildcard include/config/lbd.h) \
    $(wildcard include/config/preempt/voluntary.h) \
    $(wildcard include/config/debug/spinlock/sleep.h) \
    $(wildcard include/config/prove/locking.h) \
    $(wildcard include/config/printk.h) \
    $(wildcard include/config/panic/printk.h) \
    $(wildcard include/config/dynamic/debug.h) \
    $(wildcard include/config/ring/buffer.h) \
    $(wildcard include/config/tracing.h) \
    $(wildcard include/config/numa.h) \
    $(wildcard include/config/ftrace/mcount/record.h) \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/bin/../lib/gcc/mips-linux/4.4.6/include/stdarg.h \
  include/linux/linkage.h \
  include/linux/compiler.h \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
  include/linux/compiler-gcc.h \
    $(wildcard include/config/arch/supports/optimized/inlining.h) \
    $(wildcard include/config/optimize/inlining.h) \
  include/linux/compiler-gcc4.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/linkage.h \
  include/linux/stddef.h \
  include/linux/types.h \
    $(wildcard include/config/uid16.h) \
    $(wildcard include/config/phys/addr/t/64bit.h) \
    $(wildcard include/config/64bit.h) \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/types.h \
  include/asm-generic/int-ll64.h \
  include/asm-generic/bitsperlong.h \
  include/linux/posix_types.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/posix_types.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/sgidefs.h \
  include/linux/bitops.h \
    $(wildcard include/config/generic/find/first/bit.h) \
    $(wildcard include/config/generic/find/last/bit.h) \
    $(wildcard include/config/generic/find/next/bit.h) \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/bitops.h \
    $(wildcard include/config/cpu/has/llsc.h) \
    $(wildcard include/config/cpu/rlx4181.h) \
    $(wildcard include/config/cpu/rlx5181.h) \
    $(wildcard include/config/cpu/has/radiax.h) \
  include/linux/irqflags.h \
    $(wildcard include/config/trace/irqflags.h) \
    $(wildcard include/config/irqsoff/tracer.h) \
    $(wildcard include/config/preempt/tracer.h) \
    $(wildcard include/config/trace/irqflags/support.h) \
    $(wildcard include/config/rtl/819x.h) \
  include/linux/typecheck.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/irqflags.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/hazards.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/cpu-features.h \
    $(wildcard include/config/cpu/has/ejtag.h) \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/cpu.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/cpu-info.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/cache.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/mach-generic/kmalloc.h \
    $(wildcard include/config/dma/coherent.h) \
  arch/rlx/bsp_rtl8686/bspcpu.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/barrier.h \
    $(wildcard include/config/cpu/has/sync.h) \
    $(wildcard include/config/cpu/has/wb.h) \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/bug.h \
    $(wildcard include/config/bug.h) \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/break.h \
  include/asm-generic/bug.h \
    $(wildcard include/config/generic/bug.h) \
    $(wildcard include/config/generic/bug/relative/pointers.h) \
    $(wildcard include/config/debug/bugverbose.h) \
    $(wildcard include/config/smp.h) \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/byteorder.h \
    $(wildcard include/config/cpu/big/endian.h) \
  include/linux/byteorder/big_endian.h \
  include/linux/swab.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/swab.h \
  include/linux/byteorder/generic.h \
  include/asm-generic/bitops/non-atomic.h \
  include/asm-generic/bitops/fls64.h \
  include/asm-generic/bitops/ffz.h \
  include/asm-generic/bitops/find.h \
  include/asm-generic/bitops/sched.h \
  include/asm-generic/bitops/hweight.h \
  include/asm-generic/bitops/ext2-non-atomic.h \
  include/asm-generic/bitops/le.h \
  include/asm-generic/bitops/ext2-atomic.h \
  include/asm-generic/bitops/minix.h \
  include/linux/log2.h \
    $(wildcard include/config/arch/has/ilog2/u32.h) \
    $(wildcard include/config/arch/has/ilog2/u64.h) \
  include/linux/ratelimit.h \
  include/linux/param.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/param.h \
    $(wildcard include/config/hz.h) \
  include/linux/dynamic_debug.h \
  include/linux/string.h \
    $(wildcard include/config/binary/printf.h) \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/string.h \
  drivers/net/rtl86900/sdk/system/include/common/debug/rt_log.h \
    $(wildcard include/config/sdk/debug/log/level.h) \
    $(wildcard include/config/sdk/debug/log/level/mask.h) \
    $(wildcard include/config/sdk/debug/log/type/level/mask.h) \
    $(wildcard include/config/sdk/debug/log/mod/mask.h) \
    $(wildcard include/config/sdk/debug.h) \
    $(wildcard include/config/sdk/internal/param/chk/.h) \
  drivers/net/rtl86900/sdk/system/include/osal/print.h \
  drivers/net/rtl86900/sdk/system/include/osal/memory.h \
    $(wildcard include/config/sdk/kernel/linux/kernel/mode.h) \
  drivers/net/rtl86900/sdk/include/hal/chipdef/allmem.h \
    $(wildcard include/config/sdk/apollo.h) \
    $(wildcard include/config/sdk/apollomp.h) \
    $(wildcard include/config/sdk/rtl9601b.h) \
    $(wildcard include/config/sdk/rtl9602c.h) \
    $(wildcard include/config/sdk/rtl9607b.h) \
  drivers/net/rtl86900/sdk/include/hal/chipdef/allreg.h \
    $(wildcard include/config/sdk/reg/dflt/val.h) \
  drivers/net/rtl86900/sdk/include/hal/chipdef/rtl9602c/rtk_rtl9602c_table_struct.h \
    $(wildcard include/config/sdk/dump/table/with/name.h) \
  drivers/net/rtl86900/sdk/system/include/common/rt_autoconf.h \
  drivers/net/rtl86900/sdk/include/hal/chipdef/rtl9602c/rtk_rtl9602c_reg_struct.h \
    $(wildcard include/config/0r.h) \
    $(wildcard include/config/1r.h) \
    $(wildcard include/config/2r.h) \
    $(wildcard include/config/3r.h) \
    $(wildcard include/config/4r.h) \
    $(wildcard include/config/clk/usr.h) \
    $(wildcard include/config/clk/dsr.h) \
    $(wildcard include/config/dsr.h) \
    $(wildcard include/config/rtl9602c.h) \
    $(wildcard include/config/0/rtl9602c.h) \
    $(wildcard include/config/1/rtl9602c.h) \
    $(wildcard include/config/2/rtl9602c.h) \
    $(wildcard include/config/3/rtl9602c.h) \
    $(wildcard include/config/4/rtl9602c.h) \
    $(wildcard include/config/clk/us/rtl9602c.h) \
    $(wildcard include/config/clk/ds/rtl9602c.h) \
    $(wildcard include/config/ds/rtl9602c.h) \
    $(wildcard include/config/sdk/dump/reg/with/name.h) \
  drivers/net/rtl86900/sdk/include/hal/mac/mem.h \
  drivers/net/rtl86900/sdk/include/hal/mac/reg.h \
  drivers/net/rtl86900/sdk/include/hal/mac/drv.h \
  drivers/net/rtl86900/sdk/include/hal/chipdef/apollo/apollo_table_struct.h \
  drivers/net/rtl86900/sdk/include/hal/common/halctrl.h \
  drivers/net/rtl86900/sdk/include/hal/chipdef/driver.h \
  drivers/net/rtl86900/sdk/include/hal/phy/phydef.h \
  drivers/net/rtl86900/sdk/include/rtk/port.h \
  drivers/net/rtl86900/sdk/include/rtk/vlan.h \
  drivers/net/rtl86900/sdk/include/hal/chipdef/chip.h \
  drivers/net/rtl86900/sdk/include/hal/chipdef/rtl9602c/rtl9602c_def.h \
  drivers/net/rtl86900/sdk/include/dal/dal_common.h \
  drivers/net/rtl86900/sdk/include/rtk/default.h \
    $(wildcard include/config/sdk/fpga/platform.h) \
  drivers/net/rtl86900/sdk/include/rtk/gpio.h \
  drivers/net/rtl86900/sdk/include/dal/rtl9602c/dal_rtl9602c_gpio.h \
  drivers/net/rtl86900/sdk/system/include/ioal/mem32.h \
  drivers/net/rtl86900/sdk/system/include/soc/type.h \

drivers/net/rtl86900/sdk/src//dal/rtl9602c/dal_rtl9602c_gpio.o: $(deps_drivers/net/rtl86900/sdk/src//dal/rtl9602c/dal_rtl9602c_gpio.o)

$(deps_drivers/net/rtl86900/sdk/src//dal/rtl9602c/dal_rtl9602c_gpio.o):
