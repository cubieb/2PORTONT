cmd_rtk_voip/voip_drivers/dsp_rtk_mux.o := /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/bin/rsdk-linux-gcc -Wp,-MD,rtk_voip/voip_drivers/.dsp_rtk_mux.o.d  -nostdinc -isystem /rsdk-1.5.6-5281-EB-2.6.30-0.9.30.3-131105/bin/../lib/gcc/mips-linux/4.4.6/include -Iinclude  -I/home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include -include include/linux/autoconf.h -Iinclude/soc -include /home/gangadhar/2PORTONT/sdk/autoconf.h -include include/soc/luna_cfg.h -D__KERNEL__ -D"VMLINUX_LOAD_ADDRESS=0x80000000" -D"LOADADDR=0x80000000" -D"DATAOFFSET=0" -Wall -Wundef -Wstrict-prototypes -Wno-trigraphs -fno-strict-aliasing -fno-common -Werror-implicit-function-declaration -fno-delete-null-pointer-checks -D__LUNA_KERNEL__ -Os -fno-inline -ffunction-sections -mno-check-zero-division -mabi=32 -G 0 -mno-abicalls -fno-pic -pipe -msoft-float -ffreestanding -EB -UMIPSEB -U_MIPSEB -U__MIPSEB -U__MIPSEB__ -UMIPSEL -U_MIPSEL -U__MIPSEL -U__MIPSEL__ -DMIPSEB -D_MIPSEB -D__MIPSEB -D__MIPSEB__ -Iinclude/asm-rlx -Iarch/rlx/bsp_rtl8686/ -Idrivers/net/rtl86900/sdk/include/ -Idrivers/net/rtl86900/sdk/system/include -I/home/gangadhar/2PORTONT/sdk/linux-2.6.x/arch/rlx/include/asm/mach-generic -Wframe-larger-than=1024 -fno-stack-protector -fomit-frame-pointer -Wdeclaration-after-statement -Wno-pointer-sign -fno-strict-overflow -fno-dwarf2-cfi-asm -DREALTEK_PATCH_FOR_MICROSEMI -Irtk_voip/include -Iarch/rlx/bsp -Irtk_voip/voip_dsp/dsp_r1/common/basic_op -Irtk_voip/voip_dsp/dsp_r1/common/util -Irtk_voip/voip_dsp/include -Irtk_voip/voip_dsp/cp3 -Irtk_voip/voip_dsp/v152 -Irtk_voip/voip_drivers/ -Irtk_voip/voip_drivers/spi -Irtk_voip/voip_dsp/dsp_r1/include -Irtk_voip/voip_drivers/zarlink -Irtk_voip/voip_drivers/zarlink/api_lib-2.25.0/includes/ -Irtk_voip/voip_drivers/zarlink/api_lib-2.25.0/profiles/ -Irtk_voip/voip_drivers/zarlink/api_lib-2.25.0/arch/rtl89xxb/ -Irtk_voip/voip_drivers/zarlink/common/ -Irtk_voip/voip_drivers/zarlink/ve886/ -Irtk_voip/voip_drivers/zarlink/api_lib-2.25.0/vp886_api/ -DFEATURE_COP3_PROFILE -DFEATURE_COP3_PCMISR -DFEATURE_COP3_PCM_RX -DFEATURE_COP3_LEC -DMODULE_NAME=\"DRV\"   -D"KBUILD_STR(s)=\#s" -D"KBUILD_BASENAME=KBUILD_STR(dsp_rtk_mux)"  -D"KBUILD_MODNAME=KBUILD_STR(voip_drivers)"  -c -o rtk_voip/voip_drivers/dsp_rtk_mux.o rtk_voip/voip_drivers/dsp_rtk_mux.c

deps_rtk_voip/voip_drivers/dsp_rtk_mux.o := \
  rtk_voip/voip_drivers/dsp_rtk_mux.c \
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
  rtk_voip/include/voip_types.h \
    $(wildcard include/config/rtl865xb.h) \
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
  include/linux/version.h \
  rtk_voip/voip_drivers/con_register.h \
    $(wildcard include/config/rtk/voip/drivers/slic/linetest.h) \
    $(wildcard include/config/rtk/voip/drivers/silab/slic/linetest.h) \
    $(wildcard include/config/rtk/voip/drivers/slic/microsemi/linetest.h) \
    $(wildcard include/config/rtk/voip/slic/zarlink/886/series.h) \
  rtk_voip/voip_drivers/con_define.h \
  rtk_voip/voip_drivers/dsp_rtk_mux.h \
  rtk_voip/voip_drivers/dsp_rtk_define.h \

rtk_voip/voip_drivers/dsp_rtk_mux.o: $(deps_rtk_voip/voip_drivers/dsp_rtk_mux.o)

$(deps_rtk_voip/voip_drivers/dsp_rtk_mux.o):
