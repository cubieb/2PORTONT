#ifndef BMM_H
#define BMM_H

typedef struct {
    unsigned int            prev;
    unsigned int            next;
} bmm_blk_list_t;

typedef struct {
    unsigned char           lg_size:5;
    unsigned char           is_allocated:1;
    unsigned char           is_buddy_allocated:1;
} bmm_mcb_t;

typedef struct {
    unsigned char           lg_size;
    unsigned char           lg_unit;
    unsigned char           is_pool_prelocated:1;   // is the pool and mcb is preallocated
    unsigned char           is_mcb_prelocated:1;   // is the pool and mcb is preallocated
    unsigned char           is_list_prelocated:1;   // is the pool and mcb is preallocated
    unsigned char           fraction:5;
    unsigned int            unit;
    unsigned char           *pool;
    bmm_mcb_t               *mcb;
    bmm_blk_list_t          *blk_list;
} bmm_t;


// returning 0 means success
extern int bmm_init(bmm_t *bmm, 
    unsigned char lg_size,          // the total size of memory to manage is '1<<(lg_size+lg_unit)', where lg_size should >=1;
    unsigned char lg_unit,          // the unit-size in log
    unsigned char *pool,            // the memory pool to manager with 'size' bytes
    bmm_mcb_t *mcb,                 // memory control block, with '1<<(lg_size-1)' entries
    bmm_blk_list_t *blk_list        // a list of the same size of memory piece, should has (lg_size + 1) entries
    );

extern int bmm_simple_init(bmm_t *bmm, 
    unsigned char lg_size,          // the total size in lg
    unsigned char lg_unit);         // the unit-size in log


extern void bmm_set_fraction(bmm_t *bmm, unsigned char frac);

// to release the internal memory of bmm_t if necessary
extern void bmm_destory(bmm_t *bmm);

// get a piece of memory from given bmm_t
extern void *bmm_malloc(bmm_t *bmm, unsigned int sz);
extern int bmm_free(bmm_t *bmm, void *p);

extern int bmm_memisbuddy(bmm_t *bmm, void *p);

extern void *bmm_realloc(bmm_t *bmm, void *p,unsigned int new_size);
extern unsigned int bmm_get_mem_size(bmm_t *bmm, void *p) ;
void bmm_dump(bmm_t *bmm, int to_show_mcb);

/* memory sise configure */
#define LG_UNIT         3
#define UNIT            (1<<LG_UNIT)

#define LG_SYS_SIZE     13
#define SYS_SIZE        (1<<LG_SYS_SIZE)

#define WHOLE_SIZE      (1<<(LG_UNIT+LG_SYS_SIZE))
static unsigned long vlCounter = 10000L;



#endif//BMM_H


