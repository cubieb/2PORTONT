#ifndef PUSHBUTTON_H_
#define PUSHBUTTON_H_

enum PushButton_Type {
	PB_RESET,
	PB_FACTORY_RESET,
	PB_WPS,
	PB_WIFISW,
};

struct pushbutton_operations {
	char *name;
	void (*handle_init)(void);
	int (*handle_is_pushed)(int);
};

int pb_is_pushed(int which);
int pb_register_operations(struct pushbutton_operations *);

#endif

