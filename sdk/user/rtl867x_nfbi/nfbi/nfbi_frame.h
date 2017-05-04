#ifndef INCLUDE_NFBI_FRAME_H
#define INCLUDE_NFBI_FRAME_H
#if 0
//--------------------------------------------------------------------------------
//mode
#define	NC_R		0x01
#define	NC_W		0x02
#define	NC_RW		(NC_R|NC_W)
//type
#define	NC_BYTE		0x01
#define	NC_WORD		0x02
#define	NC_DWORD	0x03
#define	NC_STR		0x04
#define	NC_ARRAY	0x05

struct nc_cmd_table_t
{
	unsigned char	*name;
    unsigned char 	id;
    unsigned char	mode;
    unsigned char	type;
    unsigned char	size;
};

extern void nc_print_cmd_list(unsigned char m);
extern int nc_handle_setcmd(char *name, unsigned char *value);
extern int nc_handle_getcmd(char *name);

#endif
#endif /*INCLUDE_NFBI_FRAME_H*/

