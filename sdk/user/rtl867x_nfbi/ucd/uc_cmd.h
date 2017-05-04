#ifndef INCLUDE_UC_CMD_H
#define INCLUDE_UC_CMD_H


struct uc_cmd_entry {                          
	int 	id;			// cmd id
	char	*name;		// cmd name in string                 			
	int		(*func)(char* data, int len, int maxlen);   //the fuction for act 
};           	


int uc_cmd_handle(int id , char *cmd ,int cmd_len, int maxlen);
int uc_cmd_action(void);

#endif /*INCLUDE_UC_CMD_H*/

