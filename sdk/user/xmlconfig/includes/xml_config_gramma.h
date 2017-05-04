#define MAX_TOKEN_STRING    512
#define MAX_NAME_STRING		128
#define MAX_VALUE_STRING	4098
#define	MAX_DEPTH			5

typedef enum {
    CFG_OK=0,
    CFG_NO_SUCH_FILE,
    CFG_READ_FILE_ERR,
    CFG_SET_VALUE_ERR,
    CFG_UNEXPECTED_TOKEN,
    CFG_SYNTAX_ERROR
} cfg_err_t;

void gramma_init(void);
void stoken_init(void);
//int config_parser(const char *filename);
cfg_err_t g_program(void);

extern char *g_config;

