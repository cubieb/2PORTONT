#include <soc.h>

typedef void (void_func)(void);

extern u32_t next_env;
extern void start_of_next_env, end_of_next_env;

static void * prep_next_env(void) {
	printf("DD: %08x <- %08x (%d B)...",
	       0x87c00000, &next_env, &end_of_next_env - &start_of_next_env);
	inline_memcpy(0x87c00000, &next_env, &end_of_next_env - &start_of_next_env);
	_soc.bios.dcache_writeback_invalidate_all();
	printf(" done\n");

	return (void *)0x87c00000;
}

__attribute__ ((noinline))
void chsp_jump(const u32_t sp, const void * entry) {
	u32_t cur_sp;

	asm volatile ("move %0, $29": "=r"(cur_sp));

	asm volatile ("move $29, %0": : "r"(sp));

	((void_func *)entry)();

	asm volatile ("move $29, %0": : "r"(cur_sp));

	return;
}

void exit_preloader(void) {
	void * entry;

	entry = prep_next_env();

	chsp_jump(0x81000000, entry);

	return;
}
REG_INIT_FUNC(exit_preloader, 30);
