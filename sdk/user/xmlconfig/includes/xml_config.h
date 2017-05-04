#ifdef CONFIG_E8B
#include "mib_e8.h"
#include "../../boa_e8/src/msgq.h"
#else
#include "mib.h"
#include "../../boa/src/msgq.h"
#endif
//#define	DEBUG
#ifdef DEBUG
#define XML_DBG(...) printf(__VA_ARGS__)
#else
#define XML_DBG(...)
#endif

/* xml_get() output max size. */
#define OUT_STR_LEN	(4096+200) // value:4096 + spare:200

/* LUNA assertion */
#if LUNA_EXTRA_CHK == 1
#define XSTR(s) #s
#define luna_assert(EX, MSG) \
	do { \
		if (!EX) { \
			fprintf (stderr, "Luna Assertion Failed: \'%s\' from %s(), line:%d, %s\n", \
				XSTR(EX), __func__, __LINE__, __FILE__); \
			fprintf (stderr, "Result in: %s\n", MSG); \
			abort (); \
		} \
	} while (0)
#else
#define luna_assert(EX, MSG) do {} while (0)
#endif

/* For XML_ENTRY_ENCRYPT_STRING2 use. */
#undef CONFIG_MIB_STRING_ENCRYPT
//#define CONFIG_MIB_STRING_ENCRYPT


/* child-sibling */
typedef enum {
	XML_OK=0,
	XML_OUT_OF_RANGE,
	XML_REACH_MAX,
	XML_FAIL,
	XML_GET,
	XML_SET
} XML_OPR_t;	//Operation Result

typedef enum {
	BYTE_ARRAY = 0,
	INT_ARRAY
} ARRAY_TYPE;

struct entry_st;
typedef XML_OPR_t (*xml_set_t)(struct entry_st *, const char *);
typedef XML_OPR_t (*xml_mcp_set_t)(struct entry_st *, const char *);
typedef XML_OPR_t (*xml_get_t)(struct entry_st *, char *, unsigned int *byte_count);
typedef XML_OPR_t (*xml_mcp_get_t)(struct entry_st *, char *, unsigned int *byte_count);
typedef void (*xml_init_index_t) (void);
typedef XML_OPR_t (*xml_dir_visit_t) (void);
typedef int (*xml_dir_index_t) (void);

/* directory structure */
typedef struct dir_st {
	struct dir_st *parent;
	struct dir_st *sibling;
	struct dir_st *firstChild;
	const char *name;
	const char *matchname;
	struct entry_st *firstEntry;
	int id;             // For chain id
	int *index;
	int index_max;      // Define the max index number
	int empty_chain;    // Flag to identify the chain without entry
	int final_idx;      // Final available index number of DIR
} dir_t;

/* entry structure */
typedef struct entry_st {
	struct dir_st *dir;
	struct entry_st *sibling;
	const char *name;
	const char *matchname;
	int id;	//for mib table id

	// setting
	xml_set_t set;
	xml_mcp_set_t mcp_set;

	// getting
	xml_get_t get;
	xml_mcp_get_t mcp_get;	

	// initial index
	xml_init_index_t init_index;
} entry_t;

/* string replace structure */
typedef struct xml_pre_st {
	char find_ent[10];
	char rep_str[10];
} xmlent_t;

void *get_last_dir_ptr(void);
void *get_last_entry_ptr(void);
int config_parser(const char *filename);
int printTree(dir_t *dir, int depth);
int gen_file(CONFIG_DATA_T xml_type, const char *filename);

int getPositionById(unsigned int ID, char **buf);
#if 0
XML_OPR_t string_to_hex(unsigned char *dst, const char *src);
#else
XML_OPR_t string_to_hex(void *dst, const char *src, ARRAY_TYPE type);
#endif
char *str_replace (const char *source, xmlent_t *ent, int ent_nr);
#ifdef CONFIG_MIB_STRING_ENCRYPT
int str_encrypt (const char *de_str, char *en_str);
int str_decrypt (const char *en_str, char *de_str);
#endif

