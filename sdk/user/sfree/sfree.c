/* This is system call api and it's utility
 */

#include <linux/unistd.h>

_syscall0(void, sfree)

int main()
{
	sfree();
	return 0;
}

