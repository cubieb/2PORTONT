#include <soc.h>

typedef void (void_func)(void);

extern u32_t next_env;
extern void start_of_next_env, end_of_next_env;

__attribute__ ((noinline))
void chsp_jump(const u32_t sp, const void * entry) {
        u32_t cur_sp;

        asm volatile ("move %0, $29": "=r"(cur_sp));

        asm volatile ("move $29, %0": : "r"(sp));

        ((void_func *)entry)();

        asm volatile ("move $29, %0": : "r"(cur_sp));

        return;
}

#ifdef STANDALONE_UBOOT

extern void board_init_f(unsigned long bootflag);
extern void uboot_vmabase;
extern void uboot_start;
extern void uboot_size;

void exit_preloader(void) {
        printf("DD: %08x <- %08x (%d B)... ",
               &uboot_vmabase, &uboot_start, &uboot_size);

        inline_memcpy(&uboot_vmabase, &uboot_start, (u32_t)&uboot_size);
        _soc.bios.dcache_writeback_invalidate_all();
        puts(" done\n");

        chsp_jump((const u32_t)(&uboot_vmabase-8), board_init_f);

        return;
}
#else

#define UBOOT_VMA_BASE 0x83c00000
static void * prep_next_env(void) {
	printf("DD: %08x <- %08x (%d B)...",
	       UBOOT_VMA_BASE, &next_env, &end_of_next_env - &start_of_next_env);
	inline_memcpy(UBOOT_VMA_BASE, &next_env, &end_of_next_env - &start_of_next_env);
	//__asm__ __volatile__ ("sdbbp;");
	_soc.bios.dcache_writeback_invalidate_all();
	puts(" done\n");

	return (void *)UBOOT_VMA_BASE;
}

void exit_preloader(void) {
	void * entry;

	entry = prep_next_env();

	chsp_jump(0x81000000, entry);

	return;
}
#endif
REG_INIT_FUNC(exit_preloader, 30);
