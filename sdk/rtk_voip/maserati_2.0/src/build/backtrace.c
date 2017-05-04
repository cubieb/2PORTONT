/*
 * backtrace.c: use -finstrument-functions to implement backtrace
 *
 * Authors: Rock <shaofu@realtek.com.tw>
 *
 */

#define MAX_CALL_DEPTH	64
#define CURRENT_STACK_FRAME  ({ char __csf; &__csf; })

struct log
{
  void *function;
  void *return_address;
};

static struct log logs[MAX_CALL_DEPTH] = {0};
static int call_depth = 0;

static void *stack_end = 0;
static int MaxStackUsage = 0;
static int MaxCallDepth = 0;

extern int main(void);

void __attribute__((__no_instrument_function__))
__cyg_profile_func_enter(void *this_func, void *call_site)
{
	if (this_func == main)
	{
		stack_end = CURRENT_STACK_FRAME;
	}

	if (call_depth < MAX_CALL_DEPTH)
	{
		logs[call_depth].function = this_func;
		logs[call_depth].return_address = call_site;
		call_depth++;

		if (call_depth > MaxCallDepth)
			MaxCallDepth = call_depth;

		if (stack_end && 
			stack_end - (void *) CURRENT_STACK_FRAME > MaxStackUsage)
		{
			MaxStackUsage = stack_end - (void *) CURRENT_STACK_FRAME;
		}
	}
	else
	{
		printf("Call depth is out of range (%d)\n", MAX_CALL_DEPTH);
		exit(0);
	}
}

void __attribute__((__no_instrument_function__))
__cyg_profile_func_exit(void *this_func, void *call_site)
{
	if (call_depth > 0)
	{
		call_depth--;
		logs[call_depth].function = 0;
		logs[call_depth].return_address = 0;
	}

	if (this_func == main)
	{
		printf("max call depth = %d, max stack usage = %d bytes\n",
			MaxCallDepth, MaxStackUsage);
	}
}

__attribute__((__no_instrument_function__))
void backtrace()
{
	struct log *current;
	int i;

	if (call_depth < 1)
		return;

	for (i=call_depth - 1; i>=0; i--)
	{
		current = &logs[i];
		printf("#%d %p in %p\n", 
			call_depth - i,
			current->return_address,
			current->function);
	}
}

