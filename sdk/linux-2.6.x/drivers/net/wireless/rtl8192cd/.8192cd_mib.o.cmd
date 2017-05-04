cmd_drivers/net/wireless/rtl8192cd/8192cd_mib.o := /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/bin/rsdk-linux-gcc -Wp,-MD,drivers/net/wireless/rtl8192cd/.8192cd_mib.o.d  -nostdinc -isystem /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/bin/../lib/gcc/mips-linux/4.4.6/include -Iinclude  -I/home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include -include include/linux/autoconf.h -Iinclude/soc -include /home/gangadhar/2PORTONT/sdk/autoconf.h -include include/soc/luna_cfg.h -D__KERNEL__ -D"VMLINUX_LOAD_ADDRESS=0x80000000" -D"LOADADDR=0x80000000" -D"DATAOFFSET=0" -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -fno-delete-null-pointer-checks -D__LUNA_KERNEL__ -Os -fno-inline -ffunction-sections -mno-check-zero-division -mabi=32 -G 0 -mno-abicalls -fno-pic -pipe -msoft-float -ffreestanding -EB -UMIPSEB -U_MIPSEB -U__MIPSEB -U__MIPSEB__ -UMIPSEL -U_MIPSEL -U__MIPSEL -U__MIPSEL__ -DMIPSEB -D_MIPSEB -D__MIPSEB -D__MIPSEB__ -Iinclude/asm-rlx -Iarch/rlx/bsp_rtl8686/ -Idrivers/net/rtl86900/sdk/include/ -Idrivers/net/rtl86900/sdk/system/include -I/home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/mach-generic -Wframe-larger-than=1024 -fno-stack-protector -fomit-frame-pointer -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fno-dwarf2-cfi-asm -Idrivers/net/wireless/rtl8192cd -Idrivers/net/wireless/rtl8192cd/OUTSRC -Idrivers/net/wireless/rtl8192cd -Idrivers/net/wireless/rtl8192cd/WlanHAL/ -Idrivers/net/wireless/rtl8192cd/WlanHAL/Include -Idrivers/net/wireless/rtl8192cd/WlanHAL/HalHeader -Idrivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX -Idrivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E/RTL8192EE -Idrivers/net/wireless/rtl8192cd/WlanHAL/RTL88XX/RTL8192E -DCONFIG_RTL8196B -DCONFIG_RTL8196C -DCONFIG_RTL8196B_GW -DCONFIG_RTL8196C_TESTCHIP_PATCH -DCONFIG_COMPAT_NET_DEV_OPS -D_MP_TELNET_SUPPORT_ -I/home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx -D'SVN_REV=""'   -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(8192cd_mib)"  -D"KBUILD_MODNAME=KBUILD_STR(rtl8192cd)"  -c -o drivers/net/wireless/rtl8192cd/8192cd_mib.o drivers/net/wireless/rtl8192cd/8192cd_mib.c

deps_drivers/net/wireless/rtl8192cd/8192cd_mib.o := \
  drivers/net/wireless/rtl8192cd/8192cd_mib.c \
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
  include/linux/module.h \
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/modversions.h) \
    $(wildcard include/config/unused/symbols.h) \
    $(wildcard include/config/generic/bug.h) \
    $(wildcard include/config/kallsyms.h) \
    $(wildcard include/config/markers.h) \
    $(wildcard include/config/tracepoints.h) \
    $(wildcard include/config/tracing.h) \
    $(wildcard include/config/module/unload.h) \
    $(wildcard include/config/smp.h) \
    $(wildcard include/config/sysfs.h) \
  include/linux/list.h \
    $(wildcard include/config/debug/list.h) \
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
  include/linux/poison.h \
  include/linux/prefetch.h \
  include/linux/types.h \
    $(wildcard include/config/uid16.h) \
    $(wildcard include/config/lbd.h) \
    $(wildcard include/config/phys/addr/t/64bit.h) \
    $(wildcard include/config/64bit.h) \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/types.h \
  include/asm-generic/int-ll64.h \
  include/asm-generic/bitsperlong.h \
  include/linux/posix_types.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/posix_types.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/sgidefs.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/processor.h \
    $(wildcard include/config/cpu/has/sleep.h) \
  include/linux/cpumask.h \
    $(wildcard include/config/disable/obsolete/cpumask/functions.h) \
    $(wildcard include/config/hotplug/cpu.h) \
    $(wildcard include/config/cpumask/offstack.h) \
    $(wildcard include/config/debug/per/cpu/maps.h) \
  include/linux/kernel.h \
    $(wildcard include/config/preempt/voluntary.h) \
    $(wildcard include/config/debug/spinlock/sleep.h) \
    $(wildcard include/config/prove/locking.h) \
    $(wildcard include/config/printk.h) \
    $(wildcard include/config/panic/printk.h) \
    $(wildcard include/config/dynamic/debug.h) \
    $(wildcard include/config/ring/buffer.h) \
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
  include/linux/stat.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/stat.h \
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
  include/linux/math64.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/div64.h \
  include/asm-generic/div64.h \
  include/linux/kmod.h \
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
    $(wildcard include/config/rtl8672.h) \
    $(wildcard include/config/rtl8681/ptm.h) \
    $(wildcard include/config/ethwan.h) \
    $(wildcard include/config/usb.h) \
    $(wildcard include/config/usb/rtl8187su/softap.h) \
    $(wildcard include/config/usb/rtl8192su/softap.h) \
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
  include/linux/errno.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/errno.h \
  include/asm-generic/errno-base.h \
  include/linux/mutex.h \
    $(wildcard include/config/debug/mutexes.h) \
  include/linux/rwsem.h \
    $(wildcard include/config/rwsem/generic/spinlock.h) \
  include/linux/rwsem-spinlock.h \
  include/linux/srcu.h \
  include/linux/topology.h \
    $(wildcard include/config/sched/smt.h) \
    $(wildcard include/config/sched/mc.h) \
  include/linux/smp.h \
    $(wildcard include/config/use/generic/smp/helpers.h) \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/topology.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/mach-generic/topology.h \
  include/asm-generic/topology.h \
  include/linux/mmdebug.h \
    $(wildcard include/config/debug/vm.h) \
    $(wildcard include/config/debug/virtual.h) \
  include/linux/elf.h \
  include/linux/elf-em.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/elf.h \
  include/linux/kobject.h \
  include/linux/sysfs.h \
  include/linux/kref.h \
  include/linux/moduleparam.h \
    $(wildcard include/config/alpha.h) \
    $(wildcard include/config/ia64.h) \
    $(wildcard include/config/ppc64.h) \
  include/linux/marker.h \
  include/linux/tracepoint.h \
  include/linux/rcupdate.h \
    $(wildcard include/config/classic/rcu.h) \
    $(wildcard include/config/tree/rcu.h) \
    $(wildcard include/config/preempt/rcu.h) \
  include/linux/completion.h \
  include/linux/rcuclassic.h \
    $(wildcard include/config/rcu/cpu/stall/detector.h) \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/local.h \
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
  include/linux/kmalloc_sizes.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/percpu.h \
  include/asm-generic/percpu.h \
    $(wildcard include/config/have/setup/per/cpu/area.h) \
  include/linux/percpu-defs.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/module.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/uaccess.h \
  drivers/net/wireless/rtl8192cd/./8192cd_cfg.h \
    $(wildcard include/config/rtl/ulinker/brsc.h) \
    $(wildcard include/config/rtl/wlan/hal/not/exist.h) \
    $(wildcard include/config/wlan/not/hal/exist.h) \
    $(wildcard include/config/net/pci.h) \
    $(wildcard include/config/pci.h) \
    $(wildcard include/config/rtl/proc/new.h) \
    $(wildcard include/config/rtl8190/priv/skb.h) \
    $(wildcard include/config/rtl8196b.h) \
    $(wildcard include/config/rtl8196c.h) \
    $(wildcard include/config/rtl8198.h) \
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
    $(wildcard include/config/wlan/hal/8881a.h) \
    $(wildcard include/config/rtl/92c/support.h) \
    $(wildcard include/config/rtk/voip/board.h) \
    $(wildcard include/config/slot/0/92d.h) \
    $(wildcard include/config/rtl/8197d.h) \
    $(wildcard include/config/rtl/8197dl.h) \
    $(wildcard include/config/rtl/8198c.h) \
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
    $(wildcard include/config/pci/hci.h) \
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
    $(wildcard include/config/sdio/hci.h) \
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
    $(wildcard include/config/rtl/vap/support.h) \
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
    $(wildcard include/config/rtl/hw/wapi/support.h) \
    $(wildcard include/config/rtl865x/cmo.h) \
    $(wildcard include/config/wireless/lan.h) \
    $(wildcard include/config/rtl/delay/refill.h) \
    $(wildcard include/config/slot/0/ext/pa.h) \
    $(wildcard include/config/slot/1/ext/pa.h) \
    $(wildcard include/config/slot/0/ext/lna.h) \
    $(wildcard include/config/slot/1/ext/lna.h) \
    $(wildcard include/config/rtk/vlc/speedup/support.h) \
    $(wildcard include/config/veriwave/check.h) \
    $(wildcard include/config/rtl/tx/early/mode/support.h) \
    $(wildcard include/config/rtl/8196d.h) \
    $(wildcard include/config/high/power/ext/pa.h) \
    $(wildcard include/config/txpwr/lmt.h) \
    $(wildcard include/config/wlan/hal/8192ee.h) \
    $(wildcard include/config/wlan/hal/8814ae.h) \
    $(wildcard include/config/wlan/hal.h) \
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
    $(wildcard include/config/arch/luna/slave.h) \
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
    $(wildcard include/config/rtl8686/dsp/mem/base.h) \
    $(wildcard include/config/usb/hci.h) \
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
    $(wildcard include/config/sdio/tx/interrupt.h) \
    $(wildcard include/config/sdio/tx/in/interrupt.h) \
    $(wildcard include/config/sdio/reserve/massive/public/page.h) \
    $(wildcard include/config/sdio/tx/filter/by/pri.h) \
    $(wildcard include/config/p2p/rtk/support.h) \
  include/linux/version.h \
  include/linux/jiffies.h \
    $(wildcard include/config/rtl/timer2.h) \
  include/linux/timex.h \
    $(wildcard include/config/no/hz.h) \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/timex.h \
  drivers/net/wireless/rtl8192cd/./typedef.h \

drivers/net/wireless/rtl8192cd/8192cd_mib.o: $(deps_drivers/net/wireless/rtl8192cd/8192cd_mib.o)

$(deps_drivers/net/wireless/rtl8192cd/8192cd_mib.o):
