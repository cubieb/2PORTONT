cmd_arch/rlx/kernel/rlx-switch.o := /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/bin/rsdk-linux-gcc -Wp,-MD,arch/rlx/kernel/.rlx-switch.o.d  -nostdinc -isystem /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/bin/../lib/gcc/mips-linux/4.4.6/include -Iinclude  -I/home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include -include include/linux/autoconf.h -Iinclude/soc -include /home/gangadhar/2PORTONT/sdk/autoconf.h -include include/soc/luna_cfg.h -D__KERNEL__ -D"VMLINUX_LOAD_ADDRESS=0x80000000" -D"LOADADDR=0x80000000" -D"DATAOFFSET=0" -D__ASSEMBLY__ -ffunction-sections  -mno-check-zero-division -mabi=32 -G 0 -mno-abicalls -fno-pic -pipe -msoft-float -ffreestanding -EB -UMIPSEB -U_MIPSEB -U__MIPSEB -U__MIPSEB__ -UMIPSEL -U_MIPSEL -U__MIPSEL -U__MIPSEL__ -DMIPSEB -D_MIPSEB -D__MIPSEB -D__MIPSEB__ -Iinclude/asm-rlx -Iarch/rlx/bsp_rtl8686/ -Idrivers/net/rtl86900/sdk/include/ -Idrivers/net/rtl86900/sdk/system/include  -I/home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/mach-generic       -c -o arch/rlx/kernel/rlx-switch.o arch/rlx/kernel/rlx-switch.S

deps_arch/rlx/kernel/rlx-switch.o := \
  arch/rlx/kernel/rlx-switch.S \
    $(wildcard include/config/cpu/has/llsc.h) \
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
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/asm.h \
    $(wildcard include/config/printk.h) \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/sgidefs.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/cachectl.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/rlxregs.h \
    $(wildcard include/config/cpu/rlx5281.h) \
    $(wildcard include/config/cpu/rlx4281.h) \
  include/linux/linkage.h \
  include/linux/compiler.h \
    $(wildcard include/config/trace/branch/profiling.h) \
    $(wildcard include/config/profile/all/branches.h) \
    $(wildcard include/config/enable/must/check.h) \
    $(wildcard include/config/enable/warn/deprecated.h) \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/linkage.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/hazards.h \
  include/asm/asm-offsets.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/page.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/mach-generic/spaces.h \
    $(wildcard include/config/32bit.h) \
    $(wildcard include/config/64bit.h) \
    $(wildcard include/config/dma/noncoherent.h) \
  include/linux/const.h \
  include/asm-generic/memory_model.h \
    $(wildcard include/config/flatmem.h) \
    $(wildcard include/config/discontigmem.h) \
    $(wildcard include/config/sparsemem/vmemmap.h) \
    $(wildcard include/config/sparsemem.h) \
  include/asm-generic/getorder.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/regdef.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/stackframe.h \
    $(wildcard include/config/cpu/has/radiax.h) \
  include/linux/threads.h \
    $(wildcard include/config/nr/cpus.h) \
    $(wildcard include/config/base/small.h) \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/asmmacro.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/asmmacro-32.h \
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/thread_info.h \
    $(wildcard include/config/kernel/stack/size/order.h) \
    $(wildcard include/config/debug/stack/usage.h) \

arch/rlx/kernel/rlx-switch.o: $(deps_arch/rlx/kernel/rlx-switch.o)

$(deps_arch/rlx/kernel/rlx-switch.o):
