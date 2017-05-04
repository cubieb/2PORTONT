cmd_drivers/net/rtl86900/sdk/system/linux/rtcore/../../linux/dev_config.o := /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/bin/rsdk-linux-gcc -Wp,-MD,drivers/net/rtl86900/sdk/system/linux/rtcore/../../linux/.dev_config.o.d  -nostdinc -isystem /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/bin/../lib/gcc/mips-linux/4.4.6/include -Iinclude  -I/home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include -include include/linux/autoconf.h -Iinclude/soc -include /home/gangadhar/2PORTONT/sdk/autoconf.h -include include/soc/luna_cfg.h -D__KERNEL__ -D"VMLINUX_LOAD_ADDRESS=0x80000000" -D"LOADADDR=0x80000000" -D"DATAOFFSET=0" -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -fno-delete-null-pointer-checks -D__LUNA_KERNEL__ -Os -fno-inline -ffunction-sections -mno-check-zero-division -mabi=32 -G 0 -mno-abicalls -fno-pic -pipe -msoft-float -ffreestanding -EB -UMIPSEB -U_MIPSEB -U__MIPSEB -U__MIPSEB__ -UMIPSEL -U_MIPSEL -U__MIPSEL -U__MIPSEL__ -DMIPSEB -D_MIPSEB -D__MIPSEB -D__MIPSEB__ -Iinclude/asm-rlx -Iarch/rlx/bsp_rtl8686/ -Idrivers/net/rtl86900/sdk/include/ -Idrivers/net/rtl86900/sdk/system/include -I/home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/mach-generic -Wframe-larger-than=1024 -fno-stack-protector -fomit-frame-pointer -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fno-dwarf2-cfi-asm -DCONFIG_SDK_KERNEL_LINUX -DCONFIG_GPON_VERSION=2 -DRTK_X86_CLE -DNO_MIB_SEPARATE -DCONFIG_SOC_DEPEND_FEATURE -DCONFIG_SOC_MODE -DCONFIG_BEN_SINGLE_END -DCONFIG_CLASSFICATION_FEATURE -DCONFIG_GPON_VERSION=2 -DRTK_X86_CLE -DNO_MIB_SEPARATE -DCONFIG_SOC_DEPEND_FEATURE -DCONFIG_SOC_MODE -DCONFIG_SDK_RTL9602C -DCONFIG_BEN_SINGLE_END -DCONFIG_CLASSFICATION_FEATURE -DCONFIG_GPON_VERSION=2 -DRTK_X86_CLE -DNO_MIB_SEPARATE -DCONFIG_SOC_DEPEND_FEATURE -DCONFIG_SOC_MODE -DCONFIG_SDK_KERNEL_LINUX -DCONFIG_SDK_RTL9602C -DCONFIG_BEN_SINGLE_END -DCONFIG_CLASSFICATION_FEATURE -I/home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/include -I/home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/system/include -I/home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/system/linux   -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(dev_config)"  -D"KBUILD_MODNAME=KBUILD_STR(rtcore)"  -c -o drivers/net/rtl86900/sdk/system/linux/rtcore/../../linux/dev_config.o drivers/net/rtl86900/sdk/system/linux/rtcore/../../linux/dev_config.c

deps_drivers/net/rtl86900/sdk/system/linux/rtcore/../../linux/dev_config.o := \
  drivers/net/rtl86900/sdk/system/linux/rtcore/../../linux/dev_config.c \
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
  /home/gangadhar/2PORTONT/sdk/linux-2.6.x/drivers/net/rtl86900/sdk/system/linux/dev_config.h \
  drivers/net/rtl86900/sdk/include/common/rt_type.h \
  drivers/net/rtl86900/sdk/system/include/common/type.h \

drivers/net/rtl86900/sdk/system/linux/rtcore/../../linux/dev_config.o: $(deps_drivers/net/rtl86900/sdk/system/linux/rtcore/../../linux/dev_config.o)

$(deps_drivers/net/rtl86900/sdk/system/linux/rtcore/../../linux/dev_config.o):
