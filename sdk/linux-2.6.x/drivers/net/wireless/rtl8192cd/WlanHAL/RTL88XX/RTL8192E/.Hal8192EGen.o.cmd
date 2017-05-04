cmd_drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/Hal8192EGen.o := /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/bin/rsdk-linux-gcc -Wp,-MD,drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/.Hal8192EGen.o.d  -nostdinc -isystem /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/bin/../lib/gcc/mips-linux/4.4.6/include -Iinclude  -I/home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include -include include/linux/autoconf.h -Iinclude/soc -include /home/gangadhar/2PORTONT/sdk/autoconf.h -include include/soc/luna_cfg.h -D__KERNEL__ -D"VMLINUX_LOAD_ADDRESS=0x80000000" -D"LOADADDR=0x80000000" -D"DATAOFFSET=0" -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -fno-delete-null-pointer-checks -D__LUNA_KERNEL__ -Os -fno-inline -ffunction-sections -mno-check-zero-division -mabi=32 -G 0 -mno-abicalls -fno-pic -pipe -msoft-float -ffreestanding -EB -UMIPSEB -U_MIPSEB -U__MIPSEB -U__MIPSEB__ -UMIPSEL -U_MIPSEL -U__MIPSEL -U__MIPSEL__ -DMIPSEB -D_MIPSEB -D__MIPSEB -D__MIPSEB__ -Iinclude/asm-rlx -Iarch/rlx/bsp_rtl8686/ -Idrivers/net/rtl86900/sdk/include/ -Idrivers/net/rtl86900/sdk/system/include -I/home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/mach-generic -Wframe-larger-than=1024 -fno-stack-protector -fomit-frame-pointer -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fno-dwarf2-cfi-asm -Idrivers/net/wireless/rtl8192cd -Idrivers/net/wireless/rtl8192cd/OUTSRC -Idrivers/net/wireless/rtl8192cd -Idrivers/net/wireless/rtl8192cd/WlanHAL/ -Idrivers/net/wireless/rtl8192cd/WlanHAL/Include -Idrivers/net/wireless/rtl8192cd/WlanHAL/HalHeader -Idrivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX -Idrivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/RTL8192EE -Idrivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E -DCONFIG_RTL8196B -DCONFIG_RTL8196C -DCONFIG_RTL8196B_GW -DCONFIG_RTL8196C_TESTCHIP_PATCH -DCONFIG_COMPAT_NET_DEV_OPS -D_MP_TELNET_SUPPORT_ -I/home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx -D'SVN_REV=""'   -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(Hal8192EGen)"  -D"KBUILD_MODNAME=KBUILD_STR(rtl8192cd)"  -c -o drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/Hal8192EGen.o drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/Hal8192EGen.c

deps_drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/Hal8192EGen.o := \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/Hal8192EGen.c \
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
  drivers/net/wireless/rtl8192cd/WlanHAL/HalPrecomp.h \
  drivers/net/wireless/rtl8192cd/WlanHAL/Include/PlatformDef.h \
  drivers/net/wireless/rtl8192cd/WlanHAL/Include/GeneralDef.h \
  drivers/net/wireless/rtl8192cd/WlanHAL/HalCfg.h \
    $(wildcard include/config/wlan/hal/8881a.h) \
    $(wildcard include/config/wlan/hal/8192ee.h) \
    $(wildcard include/config/pci/hci.h) \
    $(wildcard include/config/usb/hci.h) \
    $(wildcard include/config/sdio/hci.h) \
    $(wildcard include/config/wlan/hal/8814ae.h) \
    $(wildcard include/config/rtl/vap/support.h) \
    $(wildcard include/config/rtl/hw/wapi/support.h) \
    $(wildcard include/config/sdio/tx/interrupt.h) \
    $(wildcard include/config/rtl8672.h) \
    $(wildcard include/config/rtl/8198c.h) \
    $(wildcard include/config/arch/luna/slave.h) \
    $(wildcard include/config/wlan/hal.h) \
    $(wildcard include/config/luna/slave/phymem/offset/hal.h) \
    $(wildcard include/config/rtl8686/dsp/mem/base.h) \
    $(wildcard include/config/net/pci.h) \
    $(wildcard include/config/8881a/hp.h) \
  drivers/net/wireless/rtl8192cd/Wlan_TypeDef.h \
    $(wildcard include/config/operation/.h) \
    $(wildcard include/config/operation.h) \
  drivers/net/wireless/rtl8192cd/Wlan_QoSType.h \
  drivers/net/wireless/rtl8192cd/8192cd_cfg.h \
    $(wildcard include/config/rtl/ulinker/brsc.h) \
    $(wildcard include/config/rtl/wlan/hal/not/exist.h) \
    $(wildcard include/config/wlan/not/hal/exist.h) \
    $(wildcard include/config/pci.h) \
    $(wildcard include/config/rtl/proc/new.h) \
    $(wildcard include/config/rtl8190/priv/skb.h) \
    $(wildcard include/config/rtl8196b.h) \
    $(wildcard include/config/rtl8196c.h) \
    $(wildcard include/config/rtl8198.h) \
    $(wildcard include/config/rtl/819x.h) \
    $(wildcard include/config/rtl/8198.h) \
    $(wildcard include/config/rtl/8196c.h) \
    $(wildcard include/config/rtl/819x/ecos.h) \
    $(wildcard include/config/openwrt/sdk.h) \
    $(wildcard include/config/rtl/custom/passthru.h) \
    $(wildcard include/config/rtl/custom/passthru/pppoe.h) \
    $(wildcard include/config/rtl/92d/support.h) \
    $(wildcard include/config/rtl/8812/support.h) \
    $(wildcard include/config/rtl/8881a.h) \
    $(wildcard include/config/use/pcie/slot/0.h) \
    $(wildcard include/config/use/pcie/slot/1.h) \
    $(wildcard include/config/rtl/dual/pcieslot/biwlan/d.h) \
    $(wildcard include/config/rtl/dual/pcieslot/biwlan.h) \
    $(wildcard include/config/rtl/92d/dmdp.h) \
    $(wildcard include/config/rtl/92c/support.h) \
    $(wildcard include/config/rtk/voip/board.h) \
    $(wildcard include/config/slot/0/92d.h) \
    $(wildcard include/config/rtl/8197d.h) \
    $(wildcard include/config/rtl/8197dl.h) \
    $(wildcard include/config/rtl/mesh/support.h) \
    $(wildcard include/config/rtk/mesh.h) \
    $(wildcard include/config/rtl8196b/ap/root.h) \
    $(wildcard include/config/rtl8196b/tr.h) \
    $(wildcard include/config/rtl8196b/gw.h) \
    $(wildcard include/config/rtl/8196c/gw.h) \
    $(wildcard include/config/rtl/8198/gw.h) \
    $(wildcard include/config/rtl8196b/kld.h) \
    $(wildcard include/config/rtl8196b/tld.h) \
    $(wildcard include/config/rtl8196c/ap/root.h) \
    $(wildcard include/config/rtl8196c/ap/hcm.h) \
    $(wildcard include/config/rtl8198/ap/root.h) \
    $(wildcard include/config/rtl/8198/ap/root.h) \
    $(wildcard include/config/rtl8196c/client/only.h) \
    $(wildcard include/config/rtl/8198/nfbi/board.h) \
    $(wildcard include/config/rtl8196c/kld.h) \
    $(wildcard include/config/rtl8196c/ec.h) \
    $(wildcard include/config/rtl/8196c/inic.h) \
    $(wildcard include/config/rtl/8198/inband/ap.h) \
    $(wildcard include/config/rtl/819xd.h) \
    $(wildcard include/config/rtl/8196e.h) \
    $(wildcard include/config/rtl/8198b.h) \
    $(wildcard include/config/command.h) \
    $(wildcard include/config/latency.h) \
    $(wildcard include/config/base0.h) \
    $(wildcard include/config/base1.h) \
    $(wildcard include/config/proc/fs.h) \
    $(wildcard include/config/rtl8196b/gw/8m.h) \
    $(wildcard include/config/pcie/power/saving.h) \
    $(wildcard include/config/rtl/8196cs.h) \
    $(wildcard include/config/rtl/88e/support.h) \
    $(wildcard include/config/slot0h.h) \
    $(wildcard include/config/slot0s.h) \
    $(wildcard include/config/slot1h.h) \
    $(wildcard include/config/slot1s.h) \
    $(wildcard include/config/rtl/wds/support.h) \
    $(wildcard include/config/pacp/support.h) \
    $(wildcard include/config/rtl/client/mode/support.h) \
    $(wildcard include/config/rtl/multi/clone/support.h) \
    $(wildcard include/config/rtl/support/multi/profile.h) \
    $(wildcard include/config/rtl8196b/gw/mp.h) \
    $(wildcard include/config/rtl/fastbridge.h) \
    $(wildcard include/config/rtl8672/bridge/fastpath.h) \
    $(wildcard include/config/rtl/no/br/shortcut.h) \
    $(wildcard include/config/rps.h) \
    $(wildcard include/config/mem/limitation.h) \
    $(wildcard include/config/rtl/nfjrom/mp.h) \
    $(wildcard include/config/enable/efuse.h) \
    $(wildcard include/config/rtl/new/autoch.h) \
    $(wildcard include/config/rtl/new/iqk.h) \
    $(wildcard include/config/rtl/noise/control/92c.h) \
    $(wildcard include/config/rtl/repeater/mode/support.h) \
    $(wildcard include/config/rtl/dfs/support.h) \
    $(wildcard include/config/rtl/hs2/support.h) \
    $(wildcard include/config/rtl/hostapd/support.h) \
    $(wildcard include/config/rtl/p2p/support.h) \
    $(wildcard include/config/rtl/comapi/wltools.h) \
    $(wildcard include/config/rtl/tdls/support.h) \
    $(wildcard include/config/wext/priv.h) \
    $(wildcard include/config/rtl/sdram/ge/32m.h) \
    $(wildcard include/config/rtl/tx/reserve/desc.h) \
    $(wildcard include/config/ant/switch.h) \
    $(wildcard include/config/rtl/8881a/ant/switch.h) \
    $(wildcard include/config/slot/0/ant/switch.h) \
    $(wildcard include/config/slot/1/ant/switch.h) \
    $(wildcard include/config/rtl/wapi/support.h) \
    $(wildcard include/config/rtl8192cd.h) \
    $(wildcard include/config/rtl865x/cmo.h) \
    $(wildcard include/config/usb.h) \
    $(wildcard include/config/wireless/lan.h) \
    $(wildcard include/config/rtl/delay/refill.h) \
    $(wildcard include/config/slot/0/ext/pa.h) \
    $(wildcard include/config/slot/1/ext/pa.h) \
    $(wildcard include/config/slot/0/ext/lna.h) \
    $(wildcard include/config/slot/1/ext/lna.h) \
    $(wildcard include/config/rtk/vlc/speedup/support.h) \
    $(wildcard include/config/veriwave/check.h) \
    $(wildcard include/config/smp.h) \
    $(wildcard include/config/rtl/tx/early/mode/support.h) \
    $(wildcard include/config/rtl/8196d.h) \
    $(wildcard include/config/high/power/ext/pa.h) \
    $(wildcard include/config/txpwr/lmt.h) \
    $(wildcard include/config/support/client/mixed/security.h) \
    $(wildcard include/config/bt/coexist/92ee.h) \
    $(wildcard include/config/offload/function.h) \
    $(wildcard include/config/8814/ap/mac/veri.h) \
    $(wildcard include/config/rtl865x/ac.h) \
    $(wildcard include/config/compat/net/dev/ops.h) \
    $(wildcard include/config/msc.h) \
    $(wildcard include/config/rtl8196c/revision/b.h) \
    $(wildcard include/config/rtl8686.h) \
    $(wildcard include/config/rtl/noise/control.h) \
    $(wildcard include/config/rtl/92d/int/pa.h) \
    $(wildcard include/config/rtl/odm/wlan/driver.h) \
    $(wildcard include/config/rtl865x/kld.h) \
    $(wildcard include/config/rtl/ulinker/wlan/delay/init.h) \
    $(wildcard include/config/rlx.h) \
    $(wildcard include/config/rtl/8812ar/vn/support.h) \
    $(wildcard include/config/slot/0/tx/beamforming.h) \
    $(wildcard include/config/slot/1/tx/beamforming.h) \
    $(wildcard include/config/rtl/11w/support.h) \
    $(wildcard include/config/ieee80211w.h) \
    $(wildcard include/config/ieee80211w/cmd.h) \
    $(wildcard include/config/rtl/11r/support.h) \
    $(wildcard include/config/ieee80211r.h) \
    $(wildcard include/config/rtl/ac2g/256qam.h) \
    $(wildcard include/config/rtl/8198/nfbi/rtk/inband/ap.h) \
    $(wildcard include/config/usb/power/bus.h) \
    $(wildcard include/config/rtl865x/eth/priv/skb.h) \
    $(wildcard include/config/rtl/eth/priv/skb.h) \
    $(wildcard include/config/rtk/vlan/support.h) \
    $(wildcard include/config/rtl/tpt/thread.h) \
    $(wildcard include/config/rtl/dot11k/support.h) \
    $(wildcard include/config/luna/slave/phymem/offset.h) \
    $(wildcard include/config/usb/vendor/req/mutex.h) \
    $(wildcard include/config/vendor/req/retry.h) \
    $(wildcard include/config/usb/vendor/req/buffer/prealloc.h) \
    $(wildcard include/config/usb/vendor/req/buffer/dynamic/allocate.h) \
    $(wildcard include/config/use/vmalloc.h) \
    $(wildcard include/config/usb/tx/aggregation.h) \
    $(wildcard include/config/tcp/ack/tx/aggregation.h) \
    $(wildcard include/config/tcp/ack/merge.h) \
    $(wildcard include/config/netdev/multi/tx/queue.h) \
    $(wildcard include/config/tx/recycle/early.h) \
    $(wildcard include/config/use/usb/buffer/alloc/rx.h) \
    $(wildcard include/config/prealloc/recv/skb.h) \
    $(wildcard include/config/usb/rx/aggregation.h) \
    $(wildcard include/config/error/detect.h) \
    $(wildcard include/config/usb/interrupt/in/pipe.h) \
    $(wildcard include/config/support/usb/int.h) \
    $(wildcard include/config/interrupt/based/txbcn.h) \
    $(wildcard include/config/sdio/tx/aggregation.h) \
    $(wildcard include/config/sdio/tx/in/interrupt.h) \
    $(wildcard include/config/sdio/reserve/massive/public/page.h) \
    $(wildcard include/config/sdio/tx/filter/by/pri.h) \
    $(wildcard include/config/p2p/rtk/support.h) \
  include/linux/version.h \
  include/linux/jiffies.h \
    $(wildcard include/config/rtl/timer2.h) \
  include/linux/math64.h \
  include/linux/types.h \
    $(wildcard include/config/uid16.h) \
    $(wildcard include/config/lbd.h) \
    $(wildcard include/config/phys/addr/t/64bit.h) \
    $(wildcard include/config/64bit.h) \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/types.h \
  include/asm-generic/int-ll64.h \
  include/asm-generic/bitsperlong.h \
  include/linux/posix_types.h \
  include/linux/stddef.h \
  include/linux/compiler.h \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
  include/linux/compiler-gcc.h \
    $(wildcard include/config/arch/supports/optimized/inlining.h) \
    $(wildcard include/config/optimize/inlining.h) \
  include/linux/compiler-gcc4.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/posix_types.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/sgidefs.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/div64.h \
  include/asm-generic/div64.h \
  include/linux/kernel.h \
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
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/linkage.h \
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
  include/linux/time.h \
  include/linux/cache.h \
    $(wildcard include/config/arch/has/cache/line/size.h) \
  include/linux/seqlock.h \
  include/linux/spinlock.h \
    $(wildcard include/config/debug/spinlock.h) \
    $(wildcard include/config/generic/lockbreak.h) \
    $(wildcard include/config/preempt.h) \
    $(wildcard include/config/debug/lock/alloc.h) \
  include/linux/preempt.h \
    $(wildcard include/config/debug/preempt.h) \
    $(wildcard include/config/preempt/notifiers.h) \
  include/linux/thread_info.h \
    $(wildcard include/config/compat.h) \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/thread_info.h \
    $(wildcard include/config/kernel/stack/size/order.h) \
    $(wildcard include/config/debug/stack/usage.h) \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/processor.h \
    $(wildcard include/config/cpu/has/sleep.h) \
  include/linux/cpumask.h \
    $(wildcard include/config/disable/obsolete/cpumask/functions.h) \
    $(wildcard include/config/hotplug/cpu.h) \
    $(wildcard include/config/cpumask/offstack.h) \
    $(wildcard include/config/debug/per/cpu/maps.h) \
  include/linux/threads.h \
    $(wildcard include/config/nr/cpus.h) \
    $(wildcard include/config/base/small.h) \
  include/linux/bitmap.h \
  include/linux/string.h \
    $(wildcard include/config/binary/printf.h) \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/string.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/cachectl.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/rlxregs.h \
    $(wildcard include/config/cpu/rlx5281.h) \
    $(wildcard include/config/cpu/rlx4281.h) \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/system.h \
    $(wildcard include/config/cpu/has/tls.h) \
    $(wildcard include/config/cpu/has/watch.h) \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/addrspace.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/mach-generic/spaces.h \
    $(wildcard include/config/32bit.h) \
    $(wildcard include/config/dma/noncoherent.h) \
  include/linux/const.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/cmpxchg.h \
  include/linux/list.h \
    $(wildcard include/config/debug/list.h) \
  include/linux/poison.h \
  include/linux/prefetch.h \
  include/linux/stringify.h \
  include/linux/bottom_half.h \
  include/linux/spinlock_types.h \
  include/linux/spinlock_types_up.h \
  include/linux/lockdep.h \
    $(wildcard include/config/lockdep.h) \
    $(wildcard include/config/lock/stat.h) \
    $(wildcard include/config/generic/hardirqs.h) \
  include/linux/spinlock_up.h \
  include/linux/spinlock_api_up.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/atomic.h \
  include/asm-generic/atomic-long.h \
  include/linux/timex.h \
    $(wildcard include/config/no/hz.h) \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/timex.h \
  drivers/net/wireless/rtl8192cd/typedef.h \
  drivers/net/wireless/rtl8192cd/wifi.h \
    $(wildcard include/config/ieee80211w/cli.h) \
  drivers/net/wireless/rtl8192cd/8192cd.h \
    $(wildcard include/config/rtl/alp.h) \
    $(wildcard include/config/rtl/wlan/status.h) \
    $(wildcard include/config/tcp/ack/txagg.h) \
    $(wildcard include/config/rtl8190/throughput.h) \
    $(wildcard include/config/1rcca/rf/power/saving.h) \
    $(wildcard include/config/rtl/wlan/dos/filter.h) \
    $(wildcard include/config/rtl865x/wtdog.h) \
    $(wildcard include/config/rtl/wtdog.h) \
    $(wildcard include/config/rtl8186/kb.h) \
    $(wildcard include/config/rtl/simple/config.h) \
    $(wildcard include/config/rtk/vlan/new/feature.h) \
    $(wildcard include/config/rtl/vlan/support.h) \
    $(wildcard include/config/rtl/bridge/vlan/support.h) \
  include/linux/interrupt.h \
    $(wildcard include/config/pm/sleep.h) \
    $(wildcard include/config/generic/irq/probe.h) \
    $(wildcard include/config/debug/shirq.h) \
  include/linux/irqreturn.h \
  include/linux/irqnr.h \
  include/linux/hardirq.h \
    $(wildcard include/config/virt/cpu/accounting.h) \
    $(wildcard include/config/classic/rcu.h) \
  include/linux/ftrace_irq.h \
    $(wildcard include/config/ftrace/nmi/enter.h) \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/hardirq.h \
  include/linux/irq.h \
    $(wildcard include/config/s390.h) \
    $(wildcard include/config/irq/per/cpu.h) \
    $(wildcard include/config/irq/release/method.h) \
    $(wildcard include/config/intr/remap.h) \
    $(wildcard include/config/generic/pending/irq.h) \
    $(wildcard include/config/sparse/irq.h) \
    $(wildcard include/config/numa/migrate/irq/desc.h) \
    $(wildcard include/config/generic/hardirqs/no//do/irq.h) \
    $(wildcard include/config/cpumasks/offstack.h) \
  include/linux/smp.h \
    $(wildcard include/config/use/generic/smp/helpers.h) \
  include/linux/errno.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/errno.h \
  include/asm-generic/errno-base.h \
  include/linux/gfp.h \
    $(wildcard include/config/zone/dma.h) \
    $(wildcard include/config/zone/dma32.h) \
    $(wildcard include/config/highmem.h) \
  include/linux/mmzone.h \
    $(wildcard include/config/force/max/zoneorder.h) \
    $(wildcard include/config/memory/hotplug.h) \
    $(wildcard include/config/sparsemem.h) \
    $(wildcard include/config/arch/populates/node/map.h) \
    $(wildcard include/config/discontigmem.h) \
    $(wildcard include/config/flat/node/mem/map.h) \
    $(wildcard include/config/cgroup/mem/res/ctlr.h) \
    $(wildcard include/config/have/memory/present.h) \
    $(wildcard include/config/need/node/memmap/size.h) \
    $(wildcard include/config/need/multiple/nodes.h) \
    $(wildcard include/config/have/arch/early/pfn/to/nid.h) \
    $(wildcard include/config/flatmem.h) \
    $(wildcard include/config/sparsemem/extreme.h) \
    $(wildcard include/config/nodes/span/other/nodes.h) \
    $(wildcard include/config/holes/in/zone.h) \
    $(wildcard include/config/arch/has/holes/memorymodel.h) \
  include/linux/wait.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/current.h \
  include/linux/numa.h \
    $(wildcard include/config/nodes/shift.h) \
  include/linux/init.h \
    $(wildcard include/config/rtl8681/ptm.h) \
    $(wildcard include/config/ethwan.h) \
    $(wildcard include/config/usb/rtl8187su/softap.h) \
    $(wildcard include/config/usb/rtl8192su/softap.h) \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/hotplug.h) \
  include/linux/section-names.h \
  include/linux/nodemask.h \
  include/linux/pageblock-flags.h \
    $(wildcard include/config/hugetlb/page.h) \
    $(wildcard include/config/hugetlb/page/size/variable.h) \
  include/linux/bounds.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/page.h \
  include/linux/pfn.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/io.h \
  include/asm-generic/iomap.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/pgtable-bits.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/mach-generic/ioremap.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/mach-generic/mangle-port.h \
    $(wildcard include/config/swap/io/space.h) \
  include/asm-generic/memory_model.h \
    $(wildcard include/config/sparsemem/vmemmap.h) \
  include/asm-generic/getorder.h \
  include/linux/memory_hotplug.h \
    $(wildcard include/config/have/arch/nodedata/extension.h) \
    $(wildcard include/config/memory/hotremove.h) \
  include/linux/notifier.h \
  include/linux/mutex.h \
    $(wildcard include/config/debug/mutexes.h) \
  include/linux/rwsem.h \
    $(wildcard include/config/rwsem/generic/spinlock.h) \
  include/linux/rwsem-spinlock.h \
  include/linux/srcu.h \
  include/linux/topology.h \
    $(wildcard include/config/sched/smt.h) \
    $(wildcard include/config/sched/mc.h) \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/topology.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/mach-generic/topology.h \
  include/asm-generic/topology.h \
  include/linux/mmdebug.h \
    $(wildcard include/config/debug/vm.h) \
    $(wildcard include/config/debug/virtual.h) \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/irq.h \
    $(wildcard include/config/rtk/voip.h) \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/mach-generic/irq.h \
    $(wildcard include/config/i8259.h) \
    $(wildcard include/config/irq/cpu.h) \
    $(wildcard include/config/irq/cpu/rm7k.h) \
    $(wildcard include/config/irq/cpu/rm9k.h) \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/ptrace.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/isadep.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/irq_regs.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/hw_irq.h \
  include/linux/irq_cpustat.h \
  include/linux/sched.h \
    $(wildcard include/config/sched/debug.h) \
    $(wildcard include/config/detect/softlockup.h) \
    $(wildcard include/config/detect/hung/task.h) \
    $(wildcard include/config/core/dump/default/elf/headers.h) \
    $(wildcard include/config/bsd/process/acct.h) \
    $(wildcard include/config/taskstats.h) \
    $(wildcard include/config/audit.h) \
    $(wildcard include/config/inotify/user.h) \
    $(wildcard include/config/epoll.h) \
    $(wildcard include/config/posix/mqueue.h) \
    $(wildcard include/config/keys.h) \
    $(wildcard include/config/user/sched.h) \
    $(wildcard include/config/sysfs.h) \
    $(wildcard include/config/schedstats.h) \
    $(wildcard include/config/task/delay/acct.h) \
    $(wildcard include/config/fair/group/sched.h) \
    $(wildcard include/config/rt/group/sched.h) \
    $(wildcard include/config/blk/dev/io/trace.h) \
    $(wildcard include/config/preempt/rcu.h) \
    $(wildcard include/config/x86/ptrace/bts.h) \
    $(wildcard include/config/sysvipc.h) \
    $(wildcard include/config/auditsyscall.h) \
    $(wildcard include/config/rt/mutexes.h) \
    $(wildcard include/config/task/xacct.h) \
    $(wildcard include/config/cpusets.h) \
    $(wildcard include/config/cgroups.h) \
    $(wildcard include/config/futex.h) \
    $(wildcard include/config/fault/injection.h) \
    $(wildcard include/config/latencytop.h) \
    $(wildcard include/config/function/graph/tracer.h) \
    $(wildcard include/config/have/unstable/sched/clock.h) \
    $(wildcard include/config/preempt/bkl.h) \
    $(wildcard include/config/group/sched.h) \
    $(wildcard include/config/mm/owner.h) \
  include/linux/capability.h \
    $(wildcard include/config/security/file/capabilities.h) \
  include/linux/rbtree.h \
  include/linux/mm_types.h \
    $(wildcard include/config/split/ptlock/cpus.h) \
    $(wildcard include/config/want/page/debug/flags.h) \
    $(wildcard include/config/mmu.h) \
    $(wildcard include/config/mmu/notifier.h) \
  include/linux/auxvec.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/auxvec.h \
  include/linux/prio_tree.h \
  include/linux/completion.h \
  include/linux/page-debug-flags.h \
    $(wildcard include/config/page/poisoning.h) \
    $(wildcard include/config/page/debug/something/else.h) \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/mmu.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/cputime.h \
  include/asm-generic/cputime.h \
  include/linux/sem.h \
  include/linux/ipc.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/ipcbuf.h \
  include/linux/kref.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/sembuf.h \
  include/linux/rcupdate.h \
    $(wildcard include/config/tree/rcu.h) \
  include/linux/rcuclassic.h \
    $(wildcard include/config/rcu/cpu/stall/detector.h) \
  include/linux/signal.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/signal.h \
    $(wildcard include/config/trad/signals.h) \
  include/asm-generic/signal-defs.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/sigcontext.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/siginfo.h \
  include/asm-generic/siginfo.h \
  include/linux/path.h \
  include/linux/pid.h \
  include/linux/percpu.h \
    $(wildcard include/config/have/dynamic/per/cpu/area.h) \
  include/linux/slab.h \
    $(wildcard include/config/slab/debug.h) \
    $(wildcard include/config/debug/objects.h) \
    $(wildcard include/config/slub.h) \
    $(wildcard include/config/slob.h) \
    $(wildcard include/config/debug/slab.h) \
  include/linux/slab_def.h \
    $(wildcard include/config/kmemtrace.h) \
  include/trace/kmemtrace.h \
  include/linux/tracepoint.h \
    $(wildcard include/config/tracepoints.h) \
  include/linux/kmalloc_sizes.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/percpu.h \
  include/asm-generic/percpu.h \
    $(wildcard include/config/have/setup/per/cpu/area.h) \
  include/linux/percpu-defs.h \
  include/linux/proportions.h \
  include/linux/percpu_counter.h \
  include/linux/seccomp.h \
    $(wildcard include/config/seccomp.h) \
  include/linux/rtmutex.h \
    $(wildcard include/config/debug/rt/mutexes.h) \
  include/linux/plist.h \
    $(wildcard include/config/debug/pi/list.h) \
  include/linux/resource.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/resource.h \
  include/asm-generic/resource.h \
  include/linux/timer.h \
    $(wildcard include/config/timer/stats.h) \
    $(wildcard include/config/debug/objects/timers.h) \
  include/linux/ktime.h \
    $(wildcard include/config/ktime/scalar.h) \
  include/linux/debugobjects.h \
    $(wildcard include/config/debug/objects/free.h) \
  include/linux/hrtimer.h \
    $(wildcard include/config/high/res/timers.h) \
  include/linux/task_io_accounting.h \
    $(wildcard include/config/task/io/accounting.h) \
  include/linux/kobject.h \
  include/linux/sysfs.h \
  include/linux/latencytop.h \
  include/linux/cred.h \
    $(wildcard include/config/security.h) \
  include/linux/key.h \
    $(wildcard include/config/sysctl.h) \
  include/linux/sysctl.h \
    $(wildcard include/config/rtl/nf/conntrack/garbage/new.h) \
  include/linux/aio.h \
    $(wildcard include/config/aio.h) \
  include/linux/workqueue.h \
  include/linux/aio_abi.h \
  include/linux/uio.h \
  include/linux/skbuff.h \
    $(wildcard include/config/imq.h) \
    $(wildcard include/config/rtk/voip/qos.h) \
    $(wildcard include/config/nf/conntrack.h) \
    $(wildcard include/config/bridge/netfilter.h) \
    $(wildcard include/config/has/dma.h) \
    $(wildcard include/config/xfrm.h) \
    $(wildcard include/config/net/sched.h) \
    $(wildcard include/config/net/cls/act.h) \
    $(wildcard include/config/ipv6/ndisc/nodetype.h) \
    $(wildcard include/config/mac80211.h) \
    $(wildcard include/config/net/dma.h) \
    $(wildcard include/config/network/secmark.h) \
    $(wildcard include/config/rtl/hardware/multicast.h) \
    $(wildcard include/config/rtl865x/lanport/restriction.h) \
    $(wildcard include/config/rtl/qos/8021p/support.h) \
    $(wildcard include/config/netfilter/xt/match/phyport.h) \
    $(wildcard include/config/rtl/local/public.h) \
    $(wildcard include/config/rtl/8676hwnat.h) \
    $(wildcard include/config/rtl8676/static/acl.h) \
    $(wildcard include/config/pppoe/proxy.h) \
    $(wildcard include/config/pppoe/proxy/fastpath.h) \
    $(wildcard include/config/new/portmapping.h) \
    $(wildcard include/config/skb/pool/prealloc.h) \
    $(wildcard include/config/rtl865x/eth/priv/skb/adv.h) \
    $(wildcard include/config/rtl/eth/priv/skb/adv.h) \
    $(wildcard include/config/skb/pool/priv.h) \
    $(wildcard include/config/net/wireless/agn.h) \
    $(wildcard include/config/net/wireless/ag.h) \
    $(wildcard include/config/wireless.h) \
  include/linux/net.h \
  include/linux/socket.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/socket.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/sockios.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/ioctl.h \
  include/asm-generic/ioctl.h \
  include/linux/sockios.h \
    $(wildcard include/config/rtl/multi/eth/wan.h) \
    $(wildcard include/config/rtl867x/vlan/mapping.h) \
    $(wildcard include/config/port/mirror.h) \
  include/linux/random.h \
  include/linux/ioctl.h \
  include/linux/fcntl.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/fcntl.h \
  include/asm-generic/fcntl.h \
  include/linux/textsearch.h \
  include/linux/module.h \
    $(wildcard include/config/modversions.h) \
    $(wildcard include/config/unused/symbols.h) \
    $(wildcard include/config/kallsyms.h) \
    $(wildcard include/config/markers.h) \
    $(wildcard include/config/module/unload.h) \
  include/linux/stat.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/stat.h \
  include/linux/kmod.h \
  include/linux/elf.h \
  include/linux/elf-em.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/elf.h \
  include/linux/moduleparam.h \
    $(wildcard include/config/alpha.h) \
    $(wildcard include/config/ia64.h) \
    $(wildcard include/config/ppc64.h) \
  include/linux/marker.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/local.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/module.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/uaccess.h \
  include/linux/err.h \
  include/net/checksum.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/checksum.h \
  include/linux/in6.h \
  include/linux/dmaengine.h \
    $(wildcard include/config/dma/engine.h) \
    $(wildcard include/config/async/tx/dma.h) \
  include/linux/device.h \
    $(wildcard include/config/debug/devres.h) \
  include/linux/ioport.h \
  include/linux/klist.h \
  include/linux/pm.h \
  include/linux/semaphore.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/device.h \
  include/asm-generic/device.h \
  include/linux/pm_wakeup.h \
    $(wildcard include/config/pm.h) \
  include/linux/dma-mapping.h \
    $(wildcard include/config/have/dma/attrs.h) \
  include/linux/dma-attrs.h \
  include/linux/bug.h \
  include/linux/scatterlist.h \
    $(wildcard include/config/debug/sg.h) \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/scatterlist.h \
  include/linux/mm.h \
    $(wildcard include/config/stack/growsup.h) \
    $(wildcard include/config/swap.h) \
    $(wildcard include/config/shmem.h) \
    $(wildcard include/config/debug/pagealloc.h) \
    $(wildcard include/config/hibernation.h) \
  include/linux/debug_locks.h \
    $(wildcard include/config/debug/locking/api/selftests.h) \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/pgtable.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/pgtable-32.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/fixmap.h \
  include/asm-generic/pgtable-nopmd.h \
  include/asm-generic/pgtable-nopud.h \
  include/asm-generic/pgtable.h \
  include/linux/page-flags.h \
    $(wildcard include/config/pageflags/extended.h) \
    $(wildcard include/config/have/mlocked/page/bit.h) \
    $(wildcard include/config/ia64/uncached/allocator.h) \
  include/linux/vmstat.h \
    $(wildcard include/config/vm/event/counters.h) \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/dma-mapping.h \
  include/linux/imq.h \
  include/linux/netdevice.h \
    $(wildcard include/config/dcb.h) \
    $(wildcard include/config/wlan/80211.h) \
    $(wildcard include/config/ax25.h) \
    $(wildcard include/config/mac80211/mesh.h) \
    $(wildcard include/config/tr.h) \
    $(wildcard include/config/net/ipip.h) \
    $(wildcard include/config/net/ipgre.h) \
    $(wildcard include/config/ipv6/sit.h) \
    $(wildcard include/config/ipv6/tunnel.h) \
    $(wildcard include/config/netpoll.h) \
    $(wildcard include/config/net/poll/controller.h) \
    $(wildcard include/config/fcoe.h) \
    $(wildcard include/config/wireless/ext.h) \
    $(wildcard include/config/net/dsa.h) \
    $(wildcard include/config/net/ns.h) \
    $(wildcard include/config/ext/switch.h) \
    $(wildcard include/config/rtl867x/packet/processor.h) \
    $(wildcard include/config/rtl/hardware/nat.h) \
    $(wildcard include/config/net/dsa/tag/dsa.h) \
    $(wildcard include/config/net/dsa/tag/trailer.h) \
    $(wildcard include/config/netpoll/trap.h) \
  include/linux/if.h \
  include/linux/hdlc/ioctl.h \
  include/linux/if_ether.h \
  include/linux/if_packet.h \
  include/linux/delay.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/delay.h \
  include/net/net_namespace.h \
    $(wildcard include/config/ipv6.h) \
    $(wildcard include/config/ip/dccp.h) \
    $(wildcard include/config/netfilter.h) \
    $(wildcard include/config/net.h) \
  include/net/netns/core.h \
  include/net/netns/mib.h \
    $(wildcard include/config/xfrm/statistics.h) \
  include/net/snmp.h \
  include/linux/snmp.h \
  include/net/netns/unix.h \
  include/net/netns/packet.h \
  include/net/netns/ipv4.h \
    $(wildcard include/config/ip/multiple/tables.h) \
    $(wildcard include/config/ip/mroute.h) \
    $(wildcard include/config/ip/pimsm/v1.h) \
    $(wildcard include/config/ip/pimsm/v2.h) \
  include/net/inet_frag.h \
  include/net/netns/ipv6.h \
    $(wildcard include/config/ipv6/multiple/tables.h) \
    $(wildcard include/config/ipv6/mroute.h) \
    $(wildcard include/config/ipv6/pimsm/v2.h) \
  include/net/netns/dccp.h \
  include/net/netns/x_tables.h \
  include/linux/netfilter.h \
    $(wildcard include/config/netfilter/debug.h) \
    $(wildcard include/config/nf/nat/needed.h) \
  include/linux/in.h \
  include/net/flow.h \
  include/linux/proc_fs.h \
    $(wildcard include/config/proc/devicetree.h) \
    $(wildcard include/config/proc/kcore.h) \
  include/linux/fs.h \
    $(wildcard include/config/dnotify.h) \
    $(wildcard include/config/quota.h) \
    $(wildcard include/config/inotify.h) \
    $(wildcard include/config/debug/writecount.h) \
    $(wildcard include/config/file/locking.h) \
    $(wildcard include/config/block.h) \
    $(wildcard include/config/fs/xip.h) \
    $(wildcard include/config/migration.h) \
  include/linux/limits.h \
  include/linux/kdev_t.h \
  include/linux/dcache.h \
  include/linux/rculist.h \
  include/linux/radix-tree.h \
  include/linux/fiemap.h \
  include/linux/quota.h \
  include/linux/dqblk_xfs.h \
  include/linux/dqblk_v1.h \
  include/linux/dqblk_v2.h \
  include/linux/dqblk_qtree.h \
  include/linux/nfs_fs_i.h \
  include/linux/nfs.h \
  include/linux/sunrpc/msg_prot.h \
  include/linux/magic.h \
  include/net/netns/conntrack.h \
    $(wildcard include/config/nf/conntrack/events.h) \
  include/linux/list_nulls.h \
  include/net/netns/xfrm.h \
  include/linux/xfrm.h \
  include/linux/seq_file_net.h \
  include/linux/seq_file.h \
  include/net/dsa.h \
  include/../net/bridge/br_private.h \
    $(wildcard include/config/rtl/igmp/snooping.h) \
    $(wildcard include/config/bridge/igmpv3/snooping.h) \
    $(wildcard include/config/brnf/defaut/off.h) \
    $(wildcard include/config/rtl/hw/stp.h) \
    $(wildcard include/config/rtl867x/combo/portmapping.h) \
    $(wildcard include/config/net/wireless.h) \
  include/linux/if_bridge.h \
  include/net/route.h \
  include/net/dst.h \
    $(wildcard include/config/net/cls/route.h) \
  include/linux/rtnetlink.h \
  include/linux/netlink.h \
    $(wildcard include/config/rtl/nlmsg/protocol.h) \
    $(wildcard include/config/rtl/nl/protocol.h) \
  include/linux/if_link.h \
  include/linux/if_addr.h \
  include/linux/neighbour.h \
  include/net/neighbour.h \
  include/net/rtnetlink.h \
  include/net/netlink.h \
  include/net/inetpeer.h \
  include/net/inet_sock.h \
  include/linux/jhash.h \
  include/net/sock.h \
  include/linux/security.h \
    $(wildcard include/config/security/path.h) \
    $(wildcard include/config/security/network.h) \
    $(wildcard include/config/security/network/xfrm.h) \
    $(wildcard include/config/securityfs.h) \
  include/linux/binfmts.h \
  include/linux/shm.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/shmparam.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/shmbuf.h \
  include/linux/msg.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/msgbuf.h \
    $(wildcard include/config/cpu/little/endian.h) \
  include/linux/filter.h \
  include/linux/rculist_nulls.h \
  include/net/request_sock.h \
  include/net/netns/hash.h \
  include/linux/in_route.h \
  include/linux/route.h \
  include/linux/ip.h \
  drivers/net/wireless/rtl8192cd/./8192cd_cfg.h \
  drivers/net/wireless/rtl8192cd/./ieee802_mib.h \
  drivers/net/wireless/rtl8192cd/./wifi.h \
  drivers/net/wireless/rtl8192cd/./Wlan_TypeDef.h \
  drivers/net/wireless/rtl8192cd/./Wlan_QoSType.h \
  drivers/net/wireless/rtl8192cd/./8192cd_security.h \
    $(wildcard include/config/rtl/comapi/cfgfile.h) \
  drivers/net/wireless/rtl8192cd/././8192cd_cfg.h \
  drivers/net/wireless/rtl8192cd/./8192cd_hw.h \
    $(wildcard include/config/.h) \
  drivers/net/wireless/rtl8192cd/././8192c_reg.h \
    $(wildcard include/config/shift.h) \
    $(wildcard include/config/mask.h) \
  drivers/net/wireless/rtl8192cd/././8192e_reg.h \
  drivers/net/wireless/rtl8192cd/././8812_reg.h \
    $(wildcard include/config/usedk.h) \
    $(wildcard include/config/no/usedk.h) \
  drivers/net/wireless/rtl8192cd/././wifi.h \
  drivers/net/wireless/rtl8192cd/././8192cd_phyreg.h \
  drivers/net/wireless/rtl8192cd/OUTSRC/phydm_types.h \
    $(wildcard include/config/hw/antenna/diversity.h) \
    $(wildcard include/config/sw/antenna/diversity.h) \
    $(wildcard include/config/no/2g/diversity.h) \
    $(wildcard include/config/2g5g/cg/trx/diversity/8881a.h) \
    $(wildcard include/config/2g/cgcs/rx/diversity.h) \
    $(wildcard include/config/2g/cg/trx/diversity.h) \
    $(wildcard include/config/2g/cg/smart/ant/diversity.h) \
    $(wildcard include/config/no/5g/diversity/8881a.h) \
    $(wildcard include/config/no/5g/diversity.h) \
    $(wildcard include/config/5g/cgcs/rx/diversity/8881a.h) \
    $(wildcard include/config/5g/cgcs/rx/diversity.h) \
    $(wildcard include/config/5g/cg/trx/diversity/8881a.h) \
    $(wildcard include/config/5g/cg/trx/diversity.h) \
    $(wildcard include/config/2g5g/cg/trx/diversity.h) \
    $(wildcard include/config/5g/cg/smart/ant/diversity.h) \
    $(wildcard include/config/not/support/antdiv.h) \
    $(wildcard include/config/2g/support/antdiv.h) \
    $(wildcard include/config/5g/support/antdiv.h) \
    $(wildcard include/config/2g5g/support/antdiv.h) \
    $(wildcard include/config/gspi/hci.h) \
    $(wildcard include/config/little/endian.h) \
    $(wildcard include/config/big/endian.h) \
  drivers/net/wireless/rtl8192cd/OUTSRC/../typedef.h \
  drivers/net/wireless/rtl8192cd/./odm_inc.h \
    $(wildcard include/config/wlan/hal/8195a.h) \
  drivers/net/wireless/rtl8192cd/OUTSRC/phydm.h \
    $(wildcard include/config/bb/rf.h) \
    $(wildcard include/config/bt/coexist.h) \
    $(wildcard include/config/phy/reg/pg/value/type.h) \
    $(wildcard include/config/antenna/diversity.h) \
    $(wildcard include/config/bb/phy/reg.h) \
    $(wildcard include/config/bb/agc/tab.h) \
    $(wildcard include/config/bb/agc/tab/2g.h) \
    $(wildcard include/config/bb/agc/tab/5g.h) \
    $(wildcard include/config/bb/phy/reg/pg.h) \
    $(wildcard include/config/bb/phy/reg/mp.h) \
    $(wildcard include/config/bb/agc/tab/diff.h) \
    $(wildcard include/config/rf/radio.h) \
    $(wildcard include/config/rf/txpwr/lmt.h) \
    $(wildcard include/config/fw/nic.h) \
    $(wildcard include/config/fw/nic/2.h) \
    $(wildcard include/config/fw/ap.h) \
    $(wildcard include/config/fw/mp.h) \
    $(wildcard include/config/fw/wowlan.h) \
    $(wildcard include/config/fw/wowlan/2.h) \
    $(wildcard include/config/fw/ap/wowlan.h) \
    $(wildcard include/config/fw/bt.h) \
  drivers/net/wireless/rtl8192cd/OUTSRC/phydm_DIG.h \
    $(wildcard include/config/special/setting/for/funai/tv.h) \
  drivers/net/wireless/rtl8192cd/OUTSRC/phydm_EdcaTurboCheck.h \
  drivers/net/wireless/rtl8192cd/OUTSRC/phydm_PathDiv.h \
  drivers/net/wireless/rtl8192cd/OUTSRC/phydm_DynamicBBPowerSaving.h \
  drivers/net/wireless/rtl8192cd/OUTSRC/phydm_RaInfo.h \
  drivers/net/wireless/rtl8192cd/OUTSRC/phydm_DynamicTxPower.h \
  drivers/net/wireless/rtl8192cd/OUTSRC/phydm_CfoTracking.h \
  drivers/net/wireless/rtl8192cd/OUTSRC/phydm_ACS.h \
  drivers/net/wireless/rtl8192cd/OUTSRC/phydm_PowerTracking.h \
  drivers/net/wireless/rtl8192cd/OUTSRC/PhyDM_Adaptivity.h \
  drivers/net/wireless/rtl8192cd/./8192cd_psk.h \
    $(wildcard include/config/rtl8186/kb/n.h) \
  drivers/net/wireless/rtl8192cd/WlanHAL/Output/HalLib.h \
  drivers/net/wireless/rtl8192cd/8192cd_util.h \
  include/linux/pci.h \
    $(wildcard include/config/pci/iov.h) \
    $(wildcard include/config/pcieaspm.h) \
    $(wildcard include/config/pci/msi.h) \
    $(wildcard include/config/pci/legacy.h) \
    $(wildcard include/config/ht/irq.h) \
    $(wildcard include/config/pci/domains.h) \
    $(wildcard include/config/pci/mmconfig.h) \
  include/linux/pci_regs.h \
  include/linux/mod_devicetable.h \
  include/linux/io.h \
    $(wildcard include/config/has/ioport.h) \
  include/linux/pci_ids.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/pci.h \
    $(wildcard include/config/dma/need/pci/map/state.h) \
  include/asm-generic/pci-dma-compat.h \
  include/linux/circ_buf.h \
  drivers/net/wireless/rtl8192cd/./8192cd.h \
  drivers/net/wireless/rtl8192cd/8192cd_headers.h \
  include/linux/wireless.h \
  drivers/net/wireless/rtl8192cd/./8192cd_tx.h \
    $(wildcard include/config/rtl8671.h) \
  drivers/net/wireless/rtl8192cd/./WlanHAL/RTL88XX/RTL8192E/RTL8192EE/Hal8192EEDef.h \
  drivers/net/wireless/rtl8192cd/Beamforming.h \
  drivers/net/wireless/rtl8192cd/OUTSRC/phydm_precomp.h \
    $(wildcard include/config/intel/proxim.h) \
  drivers/net/wireless/rtl8192cd/OUTSRC/phydm_types.h \
  drivers/net/wireless/rtl8192cd/OUTSRC/../8192cd_cfg.h \
  drivers/net/wireless/rtl8192cd/OUTSRC/../odm_inc.h \
  drivers/net/wireless/rtl8192cd/OUTSRC/../8192cd.h \
  drivers/net/wireless/rtl8192cd/OUTSRC/../8192cd_util.h \
  drivers/net/wireless/rtl8192cd/OUTSRC/../8192cd_headers.h \
  drivers/net/wireless/rtl8192cd/OUTSRC/../8192cd_debug.h \
  drivers/net/wireless/rtl8192cd/OUTSRC/phydm.h \
  drivers/net/wireless/rtl8192cd/OUTSRC/phydm_HWConfig.h \
  drivers/net/wireless/rtl8192cd/OUTSRC/phydm_debug.h \
  drivers/net/wireless/rtl8192cd/OUTSRC/phydm_RegDefine11AC.h \
  drivers/net/wireless/rtl8192cd/OUTSRC/phydm_RegDefine11N.h \
    $(wildcard include/config/anta/11n.h) \
  drivers/net/wireless/rtl8192cd/OUTSRC/phydm_AntDect.h \
  drivers/net/wireless/rtl8192cd/OUTSRC/phydm_interface.h \
  drivers/net/wireless/rtl8192cd/OUTSRC/phydm_reg.h \
  drivers/net/wireless/rtl8192cd/OUTSRC/phydm_AntDiv.h \
  drivers/net/wireless/rtl8192cd/OUTSRC/HalPhyRf.h \
  drivers/net/wireless/rtl8192cd/OUTSRC/rtl8192e/HalPhyRf_8192e.h \
  drivers/net/wireless/rtl8192cd/OUTSRC/rtl8192e/../HalPhyRf.h \
  drivers/net/wireless/rtl8192cd/OUTSRC/rtl8192e/phydm_RTL8192E.h \
  drivers/net/wireless/rtl8192cd/WlanHAL/HalPrecompInc.h \
  drivers/net/wireless/rtl8192cd/WlanHAL/Include/StatusCode.h \
  drivers/net/wireless/rtl8192cd/WlanHAL/HalDbgCmd.h \
  drivers/net/wireless/rtl8192cd/WlanHAL/HalDef.h \
    $(wildcard include/config/para/.h) \
    $(wildcard include/config/para.h) \
  drivers/net/wireless/rtl8192cd/WlanHAL/HalHeader/HalHWCfg.h \
    $(wildcard include/config/wlanreg/support.h) \
  drivers/net/wireless/rtl8192cd/WlanHAL/HalHeader/HalComTXDesc.h \
  drivers/net/wireless/rtl8192cd/WlanHAL/HalHeader/HalHWCfg.h \
  drivers/net/wireless/rtl8192cd/WlanHAL/HalHeader/HalComRXDesc.h \
  drivers/net/wireless/rtl8192cd/WlanHAL/HalHeader/HalComBit.h \
  drivers/net/wireless/rtl8192cd/WlanHAL/HalHeader/HalComReg.h \
  drivers/net/wireless/rtl8192cd/WlanHAL/HalHeader/HalComPhyBit.h \
  drivers/net/wireless/rtl8192cd/WlanHAL/HalHeader/HalComPhyReg.h \
    $(wildcard include/config/pmpd/anta/ac.h) \
    $(wildcard include/config/anta/ac.h) \
    $(wildcard include/config/antb/ac.h) \
    $(wildcard include/config/pmpd/antb/ac.h) \
  drivers/net/wireless/rtl8192cd/WlanHAL/HalCommon.h \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/Hal88XXPwrSeqCmd.h \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/../../HalPwrSeqCmd.h \
    $(wildcard include/config/rtl/92e/support.h) \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/Hal88XXReg.h \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/Hal88XXDesc.h \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/Hal88XXTxDesc.h \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/Hal88XXRxDesc.h \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/Hal88XXFirmware.h \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/Hal88XXIsr.h \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/Hal88XXDebug.h \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/Hal88XXPhyCfg.h \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/Hal88XXDM.h \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/Hal8192EPwrSeqCmd.h \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/Hal8192EDef.h \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/Hal8192EPhyCfg.h \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/RTL8192EE/Hal8192EEDef.h \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/Hal88XXDef.h \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/../../../data_AGC_TAB_8192E.c \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/../../../data_MAC_REG_8192E.c \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/../../../data_PHY_REG_8192E.c \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/../../../data_PHY_REG_MP_8192E.c \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/../../../data_PHY_REG_PG_8192E.c \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/../../../data_PHY_REG_PG_8192E_new.c \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/../../../data_RadioA_8192E.c \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/../../../data_RadioB_8192E.c \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/../../../data_rtl8192Efw.c \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/../../../data_MAC_REG_8192Eb.c \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/../../../data_PHY_REG_8192Eb.c \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/../../../data_RadioA_8192Eb.c \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/../../../data_RadioB_8192Eb.c \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/../../../data_AGC_TAB_8192Emp.c \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/../../../data_RadioA_8192Emp.c \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/../../../data_RadioB_8192Emp.c \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/../../../data_RadioA_8192EmpA.c \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/../../../data_RadioB_8192EmpA.c \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/../../../data_MAC_REG_8192Emp.c \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/../../../data_PHY_REG_8192Emp.c \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/../../../data_PHY_REG_MP_8192Emp.c \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/../../../data_PHY_REG_PG_8192Emp.c \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/../../../data_rtl8192EfwMP.c \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/../../../data_TxPowerTrack_AP.c \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/../../../data_MACDM_def_high_8192E.c \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/../../../data_MACDM_def_low_8192E.c \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/../../../data_MACDM_def_normal_8192E.c \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/../../../data_MACDM_gen_high_8192E.c \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/../../../data_MACDM_gen_low_8192E.c \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/../../../data_MACDM_gen_normal_8192E.c \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/../../../data_MACDM_txop_high_8192E.c \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/../../../data_MACDM_txop_low_8192E.c \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/../../../data_MACDM_txop_normal_8192E.c \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/../../../data_MACDM_state_criteria_8192E.c \
  drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/../../../data_TXPWR_LMT_92EE_new.c \

drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/Hal8192EGen.o: $(deps_drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/Hal8192EGen.o)

$(deps_drivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/Hal8192EGen.o):
