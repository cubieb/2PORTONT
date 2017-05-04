cmd_rtk_voip/voip_manager/voip_mgr_do_misc.o := /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/bin/rsdk-linux-gcc -Wp,-MD,rtk_voip/voip_manager/.voip_mgr_do_misc.o.d  -nostdinc -isystem /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/bin/../lib/gcc/mips-linux/4.4.6/include -Iinclude  -I/home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include -include include/linux/autoconf.h -Iinclude/soc -include /home/gangadhar/2PORTONT/sdk/autoconf.h -include include/soc/luna_cfg.h -D__KERNEL__ -D"VMLINUX_LOAD_ADDRESS=0x80000000" -D"LOADADDR=0x80000000" -D"DATAOFFSET=0" -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -fno-delete-null-pointer-checks -D__LUNA_KERNEL__ -Os -fno-inline -ffunction-sections -mno-check-zero-division -mabi=32 -G 0 -mno-abicalls -fno-pic -pipe -msoft-float -ffreestanding -EB -UMIPSEB -U_MIPSEB -U__MIPSEB -U__MIPSEB__ -UMIPSEL -U_MIPSEL -U__MIPSEL -U__MIPSEL__ -DMIPSEB -D_MIPSEB -D__MIPSEB -D__MIPSEB__ -Iinclude/asm-rlx -Iarch/rlx/bsp_rtl8686/ -Idrivers/net/rtl86900/sdk/include/ -Idrivers/net/rtl86900/sdk/system/include -I/home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/mach-generic -Wframe-larger-than=1024 -fno-stack-protector -fomit-frame-pointer -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fno-dwarf2-cfi-asm -Irtk_voip/voip_manager -Irtk_voip/include -Irtk_voip/voip_dsp/cp3 -Irtk_voip/voip_dsp/include -Irtk_voip/voip_dsp/dsp_r1/include -Irtk_voip/voip_dsp/v152 -Irtk_voip/voip_drivers -Irtk_voip/voip_ipc -DMODULE_NAME=\"MGR\"   -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(voip_mgr_do_misc)"  -D"KBUILD_MODNAME=KBUILD_STR(voip_manager)"  -c -o rtk_voip/voip_manager/voip_mgr_do_misc.o rtk_voip/voip_manager/voip_mgr_do_misc.c

deps_rtk_voip/voip_manager/voip_mgr_do_misc.o := \
  rtk_voip/voip_manager/voip_mgr_do_misc.c \
    $(wildcard include/config/defaults/kernel/2/6.h) \
    $(wildcard include/config/defaults/kernel/3/10.h) \
    $(wildcard include/config/rtk/voip/led.h) \
    $(wildcard include/config/rtk/voip/ipc/arch/is/host.h) \
    $(wildcard include/config/rtk/voip/ipc/arch.h) \
    $(wildcard include/config/rtk/voip/defer/snddev/init.h) \
    $(wildcard include/config/rtk/voip/drivers/pcm89xxc.h) \
    $(wildcard include/config/rtk/voip/drivers/pcm8672.h) \
    $(wildcard include/config/rtk/voip/drivers/pcm8676.h) \
    $(wildcard include/config/rtk/voip/drivers/pcm89xxd.h) \
    $(wildcard include/config/rtk/voip/platform/8686.h) \
    $(wildcard include/config/rtk/voip/ethernet/dsp.h) \
    $(wildcard include/config/nfbi/host.h) \
    $(wildcard include/config/rtk/voip/coprocess/dsp.h) \
  /home/gangadhar/2PORTONT/sdk/autoconf.h \
    $(wildcard include/config/defaults/realtek/luna.h) \
    $(wildcard include/config/luna/dual/linux.h) \
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
  include/linux/interrupt.h \
    $(wildcard include/config/generic/hardirqs.h) \
    $(wildcard include/config/lockdep.h) \
    $(wildcard include/config/pm/sleep.h) \
    $(wildcard include/config/smp.h) \
    $(wildcard include/config/generic/irq/probe.h) \
    $(wildcard include/config/proc/fs.h) \
    $(wildcard include/config/debug/shirq.h) \
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
  include/linux/preempt.h \
    $(wildcard include/config/debug/preempt.h) \
    $(wildcard include/config/preempt.h) \
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
  include/linux/irqreturn.h \
  include/linux/irqnr.h \
  include/linux/hardirq.h \
    $(wildcard include/config/virt/cpu/accounting.h) \
    $(wildcard include/config/no/hz.h) \
    $(wildcard include/config/classic/rcu.h) \
  include/linux/lockdep.h \
    $(wildcard include/config/lock/stat.h) \
    $(wildcard include/config/debug/lock/alloc.h) \
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
  include/linux/cache.h \
    $(wildcard include/config/arch/has/cache/line/size.h) \
  include/linux/spinlock.h \
    $(wildcard include/config/debug/spinlock.h) \
    $(wildcard include/config/generic/lockbreak.h) \
  include/linux/stringify.h \
  include/linux/bottom_half.h \
  include/linux/spinlock_types.h \
  include/linux/spinlock_types_up.h \
  include/linux/spinlock_up.h \
  include/linux/spinlock_api_up.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/atomic.h \
  include/asm-generic/atomic-long.h \
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
    $(wildcard include/config/modules.h) \
    $(wildcard include/config/hotplug.h) \
  include/linux/section-names.h \
  include/linux/seqlock.h \
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
    $(wildcard include/config/rtl/timer2.h) \
    $(wildcard include/config/preempt/bkl.h) \
    $(wildcard include/config/group/sched.h) \
    $(wildcard include/config/mm/owner.h) \
  include/linux/capability.h \
    $(wildcard include/config/security/file/capabilities.h) \
  include/linux/timex.h \
  include/linux/time.h \
  include/linux/math64.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/div64.h \
  include/asm-generic/div64.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/timex.h \
  include/linux/jiffies.h \
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
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/uaccess.h \
  include/net/sock.h \
    $(wildcard include/config/net/ns.h) \
    $(wildcard include/config/xfrm.h) \
    $(wildcard include/config/net/dma.h) \
  include/linux/list_nulls.h \
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
  include/linux/netdevice.h \
    $(wildcard include/config/dcb.h) \
    $(wildcard include/config/rtl865x/lanport/restriction.h) \
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
    $(wildcard include/config/ext/switch.h) \
    $(wildcard include/config/rtl867x/packet/processor.h) \
    $(wildcard include/config/rtl/hardware/nat.h) \
    $(wildcard include/config/compat/net/dev/ops.h) \
    $(wildcard include/config/net/dsa/tag/dsa.h) \
    $(wildcard include/config/net/dsa/tag/trailer.h) \
    $(wildcard include/config/rtl/local/public.h) \
    $(wildcard include/config/netpoll/trap.h) \
  include/linux/if.h \
    $(wildcard include/config/rtl/multi/eth/wan.h) \
  include/linux/socket.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/socket.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/sockios.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/ioctl.h \
  include/asm-generic/ioctl.h \
  include/linux/sockios.h \
    $(wildcard include/config/rtl867x/vlan/mapping.h) \
    $(wildcard include/config/port/mirror.h) \
    $(wildcard include/config/pppoe/proxy.h) \
  include/linux/hdlc/ioctl.h \
  include/linux/if_ether.h \
  include/linux/skbuff.h \
    $(wildcard include/config/imq.h) \
    $(wildcard include/config/rtk/vlan/support.h) \
    $(wildcard include/config/rtk/voip/qos.h) \
    $(wildcard include/config/nf/conntrack.h) \
    $(wildcard include/config/bridge/netfilter.h) \
    $(wildcard include/config/has/dma.h) \
    $(wildcard include/config/net/sched.h) \
    $(wildcard include/config/net/cls/act.h) \
    $(wildcard include/config/ipv6/ndisc/nodetype.h) \
    $(wildcard include/config/mac80211.h) \
    $(wildcard include/config/network/secmark.h) \
    $(wildcard include/config/rtl/hardware/multicast.h) \
    $(wildcard include/config/rtl/qos/8021p/support.h) \
    $(wildcard include/config/netfilter/xt/match/phyport.h) \
    $(wildcard include/config/rtl/8676hwnat.h) \
    $(wildcard include/config/rtl8676/static/acl.h) \
    $(wildcard include/config/pppoe/proxy/fastpath.h) \
    $(wildcard include/config/new/portmapping.h) \
    $(wildcard include/config/skb/pool/prealloc.h) \
    $(wildcard include/config/rtl865x/eth/priv/skb/adv.h) \
    $(wildcard include/config/rtl/eth/priv/skb/adv.h) \
    $(wildcard include/config/skb/pool/priv.h) \
    $(wildcard include/config/rtl/eth/priv/skb.h) \
    $(wildcard include/config/rtl865x/eth/priv/skb.h) \
    $(wildcard include/config/net/wireless/agn.h) \
    $(wildcard include/config/net/wireless/ag.h) \
    $(wildcard include/config/wireless.h) \
  include/linux/net.h \
  include/linux/random.h \
  include/linux/ioctl.h \
  include/linux/fcntl.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/fcntl.h \
  include/asm-generic/fcntl.h \
  include/linux/textsearch.h \
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
  include/net/netns/xfrm.h \
  include/linux/xfrm.h \
  include/linux/seq_file_net.h \
  include/linux/seq_file.h \
  include/net/dsa.h \
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
  rtk_voip/include/rtk_voip.h \
    $(wildcard include/config/rtk/voip/drivers/pcm865xc.h) \
    $(wildcard include/config/rtk/voip/drivers/pcm8972b/family.h) \
    $(wildcard include/config/audiocodes/voip.h) \
    $(wildcard include/config/rtk/voip/drivers/ip/phone.h) \
    $(wildcard include/config/rtk/voip/ipc/arch/is/dsp.h) \
    $(wildcard include/config/rtk/voip/wideband/support.h) \
    $(wildcard include/config/rtk/voip/g7231.h) \
    $(wildcard include/config/rtk/voip/g729ab.h) \
    $(wildcard include/config/rtk/voip/slic/si32176/nr.h) \
    $(wildcard include/config/rtk/voip/slic/si32176/cs/nr.h) \
    $(wildcard include/config/rtk/voip/slic/si32178/nr.h) \
    $(wildcard include/config/rtk/voip/slic/si32176/si32178/nr.h) \
    $(wildcard include/config/rtk/voip/slic/si3226/nr.h) \
    $(wildcard include/config/rtk/voip/slic/si3226x/nr.h) \
    $(wildcard include/config/rtk/voip/drivers/slic/le88221/nr.h) \
    $(wildcard include/config/rtk/voip/drivers/slic/le88111/nr.h) \
    $(wildcard include/config/rtk/voip/drivers/slic/le89116/nr.h) \
    $(wildcard include/config/rtk/voip/drivers/slic/le89316/nr.h) \
    $(wildcard include/config/rtk/voip/dect/dspg/hs/nr.h) \
    $(wildcard include/config/rtk/voip/dect/sitel/hs/nr.h) \
    $(wildcard include/config/rtk/voip/ip/phone/ch/nr.h) \
    $(wildcard include/config/rtk/voip/dsp/device/nr.h) \
    $(wildcard include/config/rtk/voip/slic/ch/nr/per/dsp.h) \
    $(wildcard include/config/rtk/voip/daa/ch/nr/per/dsp.h) \
    $(wildcard include/config/rtk/voip/drivers/mirror/slic/nr.h) \
    $(wildcard include/config/rtk/voip/drivers/mirror/daa/nr.h) \
    $(wildcard include/config/rtk/voip/drivers/daa/support.h) \
    $(wildcard include/config/rtk/voip/drivers/virtual/daa.h) \
    $(wildcard include/config/rtk/voip/drivers/virtual/daa/2/relay/support.h) \
    $(wildcard include/config/rtk/voip/con/ch/num.h) \
    $(wildcard include/config/rtk/voip/bus/pcm/ch/num.h) \
    $(wildcard include/config/rtk/voip/bus/iis/ch/num.h) \
    $(wildcard include/config/rtk/voip/drivers/8186v/router.h) \
    $(wildcard include/config/rtk/voip/wan/vlan.h) \
    $(wildcard include/config/rtk/voip/clone/mac.h) \
    $(wildcard include/config/rtk/voip/drivers/pcm8186.h) \
    $(wildcard include/config/rtk/voip/gpio/8962.h) \
    $(wildcard include/config/rtk/voip/drivers/pcm8671.h) \
    $(wildcard include/config/rtk/voip/gpio/8972b.h) \
    $(wildcard include/config/rtk/voip/gpio/8954c/v100.h) \
    $(wildcard include/config/rtk/voip/gpio/8954c/v200.h) \
    $(wildcard include/config/rtk/voip/slic/num/8.h) \
    $(wildcard include/config/rtk/voip/daa/num/8.h) \
    $(wildcard include/config/rtk/voip/drivers/fxo.h) \
    $(wildcard include/config/default/new/ec128.h) \
    $(wildcard include/config/rtk/voip/t38.h) \
    $(wildcard include/config/rtk/voip/ip/phone.h) \
    $(wildcard include/config/cwmp/tr069.h) \
    $(wildcard include/config/rtk/voip/g722/itu/use.h) \
    $(wildcard include/config/rtk/voip/rtp/redundant.h) \
    $(wildcard include/config/rtk/voip/g7111.h) \
    $(wildcard include/config/rtk/voip/drivers/si3050.h) \
  include/linux/version.h \
  rtk_voip/include/voip_debug.h \
  rtk_voip/include/voip_timer.h \
  rtk_voip/include/voip_feature.h \
  rtk_voip/include/rtk_voip.h \
  rtk_voip/include/voip_types.h \
    $(wildcard include/config/rtl865xb.h) \
  rtk_voip/include/voip_types.h \
  rtk_voip/include/voip_errno.h \
    $(wildcard include/config/range/err.h) \
  rtk_voip/include/voip_control.h \
    $(wildcard include/config/rtk/voip/ipv6/support.h) \
    $(wildcard include/config/rtk/voip/srtp.h) \
  rtk_voip/include/voip_params.h \
  rtk_voip/include/voip_params.h \
  rtk_voip/voip_manager/voip_mgr_define.h \
  rtk_voip/voip_manager/voip_mgr_netfilter.h \
  rtk_voip/voip_manager/../include/voip_types.h \
  rtk_voip/voip_manager/../include/voip_params.h \
  rtk_voip/voip_manager/../include/voip_control.h \
  rtk_voip/voip_manager/../include/rtk_voip.h \
  rtk_voip/voip_manager/../voip_dsp/rtp/Rtp.h \
  rtk_voip/voip_manager/../voip_dsp/rtp/rtpTypes.h \
    $(wildcard include/config/rtk/voip/rtcp/xr.h) \
  rtk_voip/voip_manager/../voip_dsp/rtp/NtpTime.h \
  rtk_voip/voip_manager/../voip_dsp/rtp/rtpTypesXR.h \
  rtk_voip/voip_dsp/include/codec_descriptor.h \
    $(wildcard include/config/rtk/voip/g722.h) \
    $(wildcard include/config/rtk/voip/g726.h) \
    $(wildcard include/config/rtk/voip/gsmfr.h) \
    $(wildcard include/config/rtk/voip/ilbc.h) \
    $(wildcard include/config/rtk/voip/amr/nb.h) \
    $(wildcard include/config/rtk/voip/speex/nb.h) \
    $(wildcard include/config/rtk/voip/pcm/linear/8k.h) \
    $(wildcard include/config/rtk/voip/pcm/linear/16k.h) \
    $(wildcard include/config/rtk/voip/silence.h) \
  rtk_voip/voip_dsp/include/../dsp_r1/include/dspcodec.h \
    $(wildcard include/config/done.h) \
  rtk_voip/voip_dsp/include/../dsp_r1/include/typedef.h \
  rtk_voip/include/codec_def.h \
  rtk_voip/voip_dsp/include/dsp_rtp_def.h \
  rtk_voip/voip_dsp/include/dsp_main.h \
  rtk_voip/voip_dsp/include/../rtp/RtpPacket.h \
  rtk_voip/voip_dsp/include/../rtp/rtpTypes.h \
  rtk_voip/voip_dsp/include/assert.h \
  rtk_voip/voip_dsp/include/../rtp/RtcpPacket.h \
  rtk_voip/voip_manager/../voip_dsp/rtp/RtpPacket.h \
  rtk_voip/voip_manager/../voip_dsp/rtp/RtpTransmitter.h \
  rtk_voip/voip_manager/../voip_dsp/rtp/RtpReceiver.h \
  rtk_voip/voip_manager/../voip_dsp/rtp/RtcpReceiver.h \
  rtk_voip/voip_manager/../voip_dsp/rtp/RtcpPacket.h \
  rtk_voip/voip_manager/../voip_dsp/rtp/rtpterm.h \
  rtk_voip/voip_manager/../voip_dsp/rtp/rtp_main.h \
  rtk_voip/voip_manager/../voip_dsp/rtp/Rtcp.h \
  rtk_voip/voip_manager/../voip_dsp/rtp/RtcpTransmitter.h \
  rtk_voip/voip_manager/../voip_dsp/rtp/rfc2833trunk.h \
  rtk_voip/voip_manager/../voip_dsp/dsp_r0/dspcodec_0.h \
  rtk_voip/voip_dsp/dsp_r1/include/dspcodec.h \
  rtk_voip/voip_manager/../voip_dsp/dsp_r1/include/dspcodec.h \
  rtk_voip/voip_manager/../voip_drivers/snd_mux_slic.h \
    $(wildcard include/config/rtk/voip/slic/zarlink/886/series.h) \
    $(wildcard include/config/rtk/voip/drivers/slic/linetest.h) \
    $(wildcard include/config/rtk/voip/drivers/silab/slic/linetest.h) \
    $(wildcard include/config/rtk/voip/drivers/slic/microsemi/linetest.h) \
  rtk_voip/voip_manager/voip_resource_check.h \
    $(wildcard include/config/rtk/voip/daa/num/1.h) \
    $(wildcard include/config/rtk/voip/daa/num/2.h) \
    $(wildcard include/config/rtk/voip/daa/num/3.h) \
    $(wildcard include/config/rtk/voip/daa/num/4.h) \
    $(wildcard include/config/rtk/voip/drivers/pcm8651.h) \
    $(wildcard include/config/rtk/voip/gpio/8651c.h) \
    $(wildcard include/config/rtk/voip/gpio/ipp/8952/v00.h) \
  rtk_voip/voip_drivers/con_defer_init.h \

rtk_voip/voip_manager/voip_mgr_do_misc.o: $(deps_rtk_voip/voip_manager/voip_mgr_do_misc.o)

$(deps_rtk_voip/voip_manager/voip_mgr_do_misc.o):
