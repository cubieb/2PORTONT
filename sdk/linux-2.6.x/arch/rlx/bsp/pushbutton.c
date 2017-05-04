#include "pushbutton.h"

static struct pushbutton_operations *pb_op = 0;

int pb_register_operations(struct pushbutton_operations *p) {
	if (!pb_op) {
		pb_op = p;
		p->handle_init();
		return 0;
	}

	return -1;
}

int pb_is_pushed(int which) {
	if (pb_op)
		return pb_op->handle_is_pushed(which);

	return 0;
}