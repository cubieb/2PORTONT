OUTPUT_FORMAT("elf32-tradbigmips", "elf32-tradbigmips", "elf32-tradlittlemips")
OUTPUT_ARCH(mips)
ENTRY(_start)
SECTIONS
{
 . = 0x00000000;
 . = ALIGN(4);
 .text :
 {
   *(.text*)
 }
 . = ALIGN(4);
 .rodata : { *(SORT_BY_ALIGNMENT(SORT_BY_NAME(.rodata*))) }
 . = ALIGN(4);
 .data : { *(.data*) }
 . = .;
 _gp = ALIGN(16) + 0x7ff0;
 .got : {
   __got_start = .;
   *(.got)
   __got_end = .;
 }
 . = ALIGN(4);
 .sdata : { *(.sdata*) }
 . = ALIGN(4);
    .soft_patch : {
        LS_sw_patch_start = .;
        KEEP(*(.soft_patch.0));
        KEEP(*(.soft_patch.1));
        KEEP(*(.soft_patch.2));
        KEEP(*(.soft_patch.3));
        KEEP(*(.soft_patch.4));
        KEEP(*(.soft_patch.5));
        KEEP(*(.soft_patch.6));
        KEEP(*(.soft_patch.7));
        KEEP(*(.soft_patch.8));
        KEEP(*(.soft_patch.9));
        KEEP(*(.soft_patch.10));
        KEEP(*(.soft_patch.11));
        KEEP(*(.soft_patch.12));
        KEEP(*(.soft_patch.13));
        KEEP(*(.soft_patch.14));
        KEEP(*(.soft_patch.15));
        KEEP(*(.soft_patch.16));
        KEEP(*(.soft_patch.17));
        KEEP(*(.soft_patch.18));
        KEEP(*(.soft_patch.19));
        KEEP(*(.soft_patch.20));
        KEEP(*(.soft_patch.21));
        KEEP(*(.soft_patch.22));
        KEEP(*(.soft_patch.23));
        LS_bootm_stack = .;
        KEEP(*(.soft_patch.99));
        LS_sw_patch_end = .;
    }
    . = ALIGN(4);
    .nor_spif_probe_func : {
        LS_start_of_nor_spif_probe_func = .;
        *(.nor_spif_probe_func);
        LS_end_of_nor_spif_probe_func = .;
    }
 . = ALIGN(4);
 .u_boot_cmd : {
   __u_boot_cmd_start = .;
   *(.u_boot_cmd)
   __u_boot_cmd_end = .;
 }
 . = ALIGN(4);
 uboot_end_data = .;
 . = ALIGN(4);
 .sbss (NOLOAD) : { *(.sbss*) }
 . = ALIGN(4);
 .bss (NOLOAD) : { *(.bss*) }
 uboot_end = .;
}
