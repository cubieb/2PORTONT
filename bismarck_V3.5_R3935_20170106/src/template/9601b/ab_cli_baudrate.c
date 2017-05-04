#include <cli/cli_util.h>
#include <cli/cli_parser.h>
#include <string.h>
#include <uart/uart.h>

cli_cmd_ret_t cli_mod_br(const void *user, u32_t argc, const char *argv[]) {
	u32_t target_br;

	if (argc != 3) {
		goto bad_command;
	}

	target_br = atoi(argv[2]);

	if ((target_br != 115200) &&
	    (target_br !=  57600) &&
	    (target_br !=  38400) &&
	    (target_br !=  19200) &&
	    (target_br !=  14400) &&
	    (target_br !=   9600)) {
		goto bad_command;
	}

	puts("II: Please modify baudrate of your terminal. Press '.' when ready...\n");
	/* This delay clears UART FIFO.
	   (1024*1024*8*16) is used to simulate the actual number (1000000*10*16) */
	udelay((1024*1024*8*16)/uart_baud_rate);
	uart_baud_rate = target_br;
	uart_init(uart_baud_rate, cg_query_freq(CG_DEV_LX));
	
	while (getc() != '.') {
		;
	}

	return CCR_OK;

 bad_command:
	puts("II: call mbr <115200/57600/38400/19200/14400/9600>");
	return CCR_INCOMPLETE_CMD;
}

cli_add_node(mbr, call, cli_mod_br);
cli_add_help(mbr, "call mbr <BAUDRATE>");
