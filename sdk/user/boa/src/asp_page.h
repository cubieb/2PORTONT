#include "globals.h"
#ifdef SUPPORT_ASP
#define ERROR -1
#define FAILED -1
#define SUCCESS 0
#define TRUE 1
#define FALSE 0

#define MAX_QUERY_TEMP_VAL_SIZE 2048


typedef int (*asp_funcptr)(int eid, request * req,  int argc, char **argv);
typedef void (*form_funcptr)(request * req, char* path, char* query);

typedef struct asp_name_s
{
	const char *pagename;
	asp_funcptr function;
	struct asp_name_s *next;
} asp_name_t;

typedef struct form_name_s
{
	const char *pagename;
#ifdef MULTI_USER_PRIV
	unsigned char privilege;	/* page privilege level */
#endif
	form_funcptr function;
	struct form_name_s *next;
} form_name_t;

typedef struct temp_mem_s
{
	char *str;
	struct temp_mem_s *next;
} temp_mem_t;

#ifdef MULTI_USER_PRIV
struct formPrivilege{
	const char* pagename;
	unsigned char privilege;	/* page privilege level */
};
extern struct formPrivilege formPrivilegeList[];
#endif

int boaWriteBlock(request *req, char *buf, int nChars);
//ANDREW #define boaWrite(req,format, arg...)  {	char *temp=(char *)malloc(1024); sprintf(temp,format, ##arg); boaWriteBlock(req,temp,strlen(temp)); free(temp); }

char *boaGetVar_adv(request *req, char *var, char *defaultGetValue, int index);
char *boaGetVar(request *req, char *var, char *defaultGetValue);
#endif

