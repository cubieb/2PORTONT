cmd_drivers/net/rtl86900/sdk/unittest/sdk/sdk_test.o := /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/bin/rsdk-linux-gcc -Wp,-MD,drivers/net/rtl86900/sdk/unittest/sdk/.sdk_test.o.d  -nostdinc -isystem /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/bin/../lib/gcc/mips-linux/4.4.6/include -Iinclude  -I/home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include -include include/linux/autoconf.h -Iinclude/soc -include /home/gangadhar/2PORTONT/sdk/autoconf.h -include include/soc/luna_cfg.h -D__KERNEL__ -D"VMLINUX_LOAD_ADDRESS=0x80000000" -D"LOADADDR=0x80000000" -D"DATAOFFSET=0" -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -fno-delete-null-pointer-checks -D__LUNA_KERNEL__ -Os -fno-inline -ffunction-sections -mno-check-zero-division -mabi=32 -G 0 -mno-abicalls -fno-pic -pipe -msoft-float -ffreestanding -EB -UMIPSEB -U_MIPSEB -U__MIPSEB -U__MIPSEB__ -UMIPSEL -U_MIPSEL -U__MIPSEL -U__MIPSEL__ -DMIPSEB -D_MIPSEB -D__MIPSEB -D__MIPSEB__ -Iinclude/asm-rlx -Iarch/rlx/bsp_rtl8686/ -Idrivers/net/rtl86900/sdk/include/ -Idrivers/net/rtl86900/sdk/system/include -I/home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/mach-generic -Wframe-larger-than=1024 -fno-stack-protector -fomit-frame-pointer -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fno-dwarf2-cfi-asm -DCONFIG_GPON_VERSION=2 -DRTK_X86_CLE -DNO_MIB_SEPARATE -DCONFIG_SOC_DEPEND_FEATURE -DCONFIG_SOC_MODE -DCONFIG_BEN_SINGLE_END -DCONFIG_CLASSFICATION_FEATURE -DCONFIG_GPON_VERSION=2 -DRTK_X86_CLE -DNO_MIB_SEPARATE -DCONFIG_SOC_DEPEND_FEATURE -DCONFIG_SOC_MODE -DCONFIG_SDK_RTL9602C -DCONFIG_BEN_SINGLE_END -DCONFIG_CLASSFICATION_FEATURE -DCONFIG_GPON_VERSION=2 -DRTK_X86_CLE -DNO_MIB_SEPARATE -DCONFIG_SOC_DEPEND_FEATURE -DCONFIG_SOC_MODE -DCONFIG_SDK_KERNEL_LINUX -DCONFIG_SDK_RTL9602C -DCONFIG_BEN_SINGLE_END -DCONFIG_CLASSFICATION_FEATURE -I/home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/include -I/home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/system/include -I/home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/system/linux -I/home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/unittest   -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(sdk_test)"  -D"KBUILD_MODNAME=KBUILD_STR(sdktest)"  -c -o drivers/net/rtl86900/sdk/unittest/sdk/sdk_test.o drivers/net/rtl86900/sdk/unittest/sdk/sdk_test.c

deps_drivers/net/rtl86900/sdk/unittest/sdk/sdk_test.o := \
  drivers/net/rtl86900/sdk/unittest/sdk/sdk_test.c \
    $(wildcard include/config/sdk/apollo.h) \
    $(wildcard include/config/sdk/rtl9602c.h) \
    $(wildcard include/config/sdk/rtl9601b.h) \
    $(wildcard include/config/gpon/version.h) \
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
  drivers/net/rtl86900/sdk/system/include/common/debug/rt_log.h \
    $(wildcard include/config/sdk/kernel/linux.h) \
    $(wildcard include/config/sdk/debug/log/level.h) \
    $(wildcard include/config/sdk/debug/log/level/mask.h) \
    $(wildcard include/config/sdk/debug/log/type/level/mask.h) \
    $(wildcard include/config/sdk/debug/log/mod/mask.h) \
    $(wildcard include/config/sdk/debug.h) \
    $(wildcard include/config/sdk/internal/param/chk/.h) \
  drivers/net/rtl86900/sdk/system/include/common/error.h \
  drivers/net/rtl86900/sdk/system/include/common/type.h \
  drivers/net/rtl86900/sdk/system/include/osal/print.h \
  /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/bin/../lib/gcc/mips-linux/4.4.6/include/stdarg.h \
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
  drivers/net/rtl86900/sdk/system/include/osal/lib.h \
  include/linux/ctype.h \
  include/linux/string.h \
    $(wildcard include/config/binary/printf.h) \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/string.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/unittest/common/unittest_util.h \
  drivers/net/rtl86900/sdk/include/common/rt_type.h \
  drivers/net/rtl86900/sdk/include/common/rt_error.h \
  drivers/net/rtl86900/sdk/include/hal/mac/mac_probe.h \
  include/linux/random.h \
  include/linux/ioctl.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/ioctl.h \
  include/asm-generic/ioctl.h \
  include/linux/irqnr.h \
    $(wildcard include/config/generic/hardirqs.h) \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/unittest/sdk/sdk_test.h \
  drivers/net/rtl86900/sdk/include/rtk/init.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/unittest/hal/hal_api_test_case.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/unittest/hal/hal_reg_test_case.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/unittest/dal/dal_gpon_test_case.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/unittest/dal/dal_l34_test_case.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/unittest/dal/dal_qos_test_case.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/unittest/dal/dal_svlan_test_case.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/unittest/dal/dal_acl_test_case.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/unittest/dal/dal_oam_test_case.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/unittest/dal/dal_ponmac_test_case.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/unittest/dal/dal_classf_test_case.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/unittest/dal/dal_stp_test_case.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/unittest/dal/dal_rate_test_case.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/unittest/dal/dal_sec_test_case.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/unittest/dal/dal_led_test_case.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/unittest/dal/dal_switch_test_case.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/unittest/dal/dal_vlan_test_case.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/unittest/dal/dal_cpu_test_case.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/unittest/dal/dal_mirror_test_case.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/unittest/dal/dal_trunk_test_case.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/unittest/dal/dal_port_test_case.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/unittest/dal/dal_l2_test_case.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/unittest/dal/dal_rldp_test_case.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/unittest/dal/dal_stat_test_case.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/unittest/dal/dal_trap_test_case.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/unittest/dal/dal_dot1x_test_case.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/unittest/dal/dal_intr_test_case.h \

drivers/net/rtl86900/sdk/unittest/sdk/sdk_test.o: $(deps_drivers/net/rtl86900/sdk/unittest/sdk/sdk_test.o)

$(deps_drivers/net/rtl86900/sdk/unittest/sdk/sdk_test.o):
