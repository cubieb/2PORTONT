/* files.h */
#ifndef _FILES_H
#define _FILES_H

struct config_keyword {
/*star:20090209 START some option may be longer than 14*/
//	char keyword[14];
	char keyword[30];
/*star:20090209 END*/
	int (*handler)(char *line, void *var);
	void *var;
	char def[30];
};


int read_config(char *file);
void write_leases(void);
void read_leases(char *file);
void update_force_portal(struct dhcpOfferedAddr *pstLease);
void remove_force_portal();
void update_client_limit(struct dhcpOfferedAddr *pstLease);

#endif
