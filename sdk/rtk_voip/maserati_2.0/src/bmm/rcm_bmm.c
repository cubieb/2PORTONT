#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include "rcm_bmm.h"

#define EVEN_INDEX(idx) ((idx)&~1)
#define TOTAL_SIZE(bmm) (1<<( (bmm)->lg_size + (bmm)->lg_unit ))
#define MCB_SIZE(bmm) (1<<((bmm)->lg_size-1))
#define LIST_SIZE(bmm) ((bmm)->lg_size+1)
#define MINUS_ONE   (0xffffffff)
#define HEAD_INDEX(mcb) (MINUS_ONE - (mcb->lg_size))
#define IS_BLK_LIST_INDEX(idx) (((unsigned int)(idx))>=(MINUS_ONE-32))
#define IS_BLK_LIST_EMPTY(blk_list) (IS_BLK_LIST_INDEX((list)->prev))
#define REPRESENT_BUDDY(idx1, idx2) (((idx1)<(idx2))?(idx1):(idx2))

static inline unsigned char *get_memory(bmm_t *bmm, unsigned int index) {
    return IS_BLK_LIST_INDEX(index)?NULL:(bmm->pool+(index*bmm->unit));
}
static unsigned int get_index(bmm_t *bmm, void *p) {
    unsigned int index;
    unsigned char *q=(unsigned char *)p;
    if (q<bmm->pool) return MINUS_ONE;
    index=q-bmm->pool;
    index>>=bmm->lg_unit;
    if (index>=(1<<bmm->lg_size)) return MINUS_ONE;
    return index;
}
static inline bmm_blk_list_t *get_free_list(bmm_t *bmm, unsigned char lg_size) {
    return bmm->blk_list + lg_size;
}
static inline bmm_blk_list_t *get_free_list_by_mcb(bmm_t *bmm, bmm_mcb_t *mcb) {
    return bmm->blk_list + (mcb->lg_size);
}
static inline bmm_blk_list_t *get_blk_list(bmm_t *bmm, unsigned int index) {
    if (IS_BLK_LIST_INDEX(index)) {
        index=MINUS_ONE-index;
        return bmm->blk_list + index;
    } else {
        return (bmm_blk_list_t *)(bmm->pool+(index*bmm->unit));
    }
}
static inline bmm_mcb_t *get_mcb(bmm_t *bmm, unsigned int index) {
    return bmm->mcb + (index>>1);
}
static inline unsigned char get_mem_lg_size(bmm_t *bmm, unsigned int index) {
    bmm_mcb_t *mcb=get_mcb(bmm, index);
    return (index&0x01)?0:mcb->lg_size;
}
static inline unsigned int find_buddy_index(bmm_t *bmm, unsigned int index) {
    bmm_mcb_t *mcb=get_mcb(bmm, index);
    unsigned buddy_index=index^(1<<mcb->lg_size);
    return buddy_index;
}

static inline void remove_from_blk_list(bmm_t *bmm, unsigned int index) {
    bmm_blk_list_t *pre, *nxt, *cur;
    unsigned int cur_prev, cur_next;
    
    //assert(!IS_BLK_LIST_INDEX(index));
    
    cur=get_blk_list(bmm, index);
    pre=get_blk_list(bmm, cur_prev=cur->prev);
    nxt=get_blk_list(bmm, cur_next=cur->next);
    pre->next=cur_next;
    nxt->prev=cur_prev;
    cur->next=index;
    cur->prev=index;
}
static inline void add_to_blk_list(bmm_t *bmm, unsigned int index) {
    bmm_mcb_t *mcb=get_mcb(bmm, index);
    bmm_blk_list_t *head=get_free_list_by_mcb(bmm, mcb);
    bmm_blk_list_t *nxt, *cur;
    unsigned int head_next;
    
    nxt=get_blk_list(bmm, head_next=head->next);
    cur=get_blk_list(bmm, index);
    cur->next=head_next;
    cur->prev=nxt->prev;
    nxt->prev=index;
    head->next=index;
}
static void set_mem_allocated(bmm_t *bmm, unsigned int index) {
    bmm_mcb_t *mcb=get_mcb(bmm, index);
    if (index & 0x01) {
        mcb->is_buddy_allocated=1;
    } else {
        mcb->is_allocated=1;
    }
}
static unsigned int split_mem(bmm_t *bmm, unsigned int index) {
    bmm_mcb_t *mcb=get_mcb(bmm, index), *buddy_mcb;
    unsigned int buddy_index;
    unsigned char new_lg_size;
    
    // cannot split
    if (index&0x01) return MINUS_ONE;
    if (mcb->lg_size==0) return MINUS_ONE;
    
    new_lg_size=--(mcb->lg_size);
    buddy_index=find_buddy_index(bmm, index);
    
    buddy_mcb=get_mcb(bmm, buddy_index);
    if ((buddy_index&0x01)==0) {
        buddy_mcb->lg_size=new_lg_size;
        buddy_mcb->is_allocated=0;
    }
    buddy_mcb->is_buddy_allocated=0;
    
    return buddy_index;
}
static unsigned int allocate_a_piece_in_big(bmm_t *bmm, 
    unsigned int big_piece_index, unsigned int lg_size) {
    unsigned int mem_lg_size=get_mem_lg_size(bmm, big_piece_index);
    unsigned int buddy_index;
    
    if (mem_lg_size<lg_size) return MINUS_ONE;

    remove_from_blk_list(bmm, big_piece_index);
    while (mem_lg_size>lg_size) {
        buddy_index=split_mem(bmm, big_piece_index);
        // assert(!IS_BLK_LIST_INDEX(buddy_index))
        add_to_blk_list(bmm, buddy_index);
        --mem_lg_size;
    }
    set_mem_allocated(bmm, big_piece_index);
    return big_piece_index;
}
static unsigned int find_fit_mem(bmm_t *bmm, unsigned int lg_size) {
    unsigned char max=bmm->lg_size;
    bmm_blk_list_t *list=bmm->blk_list+lg_size;
    unsigned int index;
    
    while(lg_size<=max){
        if (!IS_BLK_LIST_EMPTY(list)) {
            index=list->next;
            //assert(!IS_BLK_LIST_INDEX(index));
            return index;
        }
        ++lg_size;
        ++list;
    }
    return MINUS_ONE;
}
static unsigned int merge_free_mem(bmm_t *bmm, unsigned int index) {
    bmm_mcb_t *mcb=get_mcb(bmm, index),*buddy_mcb, *rep_mcb;
    unsigned int buddy_index=find_buddy_index(bmm, index), rep;
    rep=REPRESENT_BUDDY(index, buddy_index);
    if (mcb->lg_size==0) {
        if (mcb->is_allocated||mcb->is_buddy_allocated) return MINUS_ONE;
        rep_mcb=mcb;
    } else {
        if (mcb->is_allocated) return MINUS_ONE;
        buddy_mcb=get_mcb(bmm, buddy_index);
        if (buddy_mcb->is_allocated) return MINUS_ONE;
        if (mcb->lg_size!=buddy_mcb->lg_size) return MINUS_ONE;
        rep_mcb=(rep==index)?mcb:buddy_mcb;
    }

    remove_from_blk_list(bmm, buddy_index);
    ++(rep_mcb->lg_size);
    return rep;
}
static unsigned int bmm_get_size(bmm_t *bmm, unsigned int index) {
    bmm_mcb_t *mcb=get_mcb(bmm, index);
    unsigned int s=0;
    
    if (mcb==NULL) return 0;
    if (mcb->lg_size==0) return 1<<(bmm->lg_unit);
    while(1) {
        s +=  1 << mcb->lg_size;
        if (mcb->is_buddy_allocated) {
            index=find_buddy_index(bmm, index);
            mcb=get_mcb(bmm, index);
        } else {
            return s << (bmm->lg_unit);
        }
    }
}

//--------------------------------------------------------------------------
//                  API to release
//--------------------------------------------------------------------------
int bmm_init(bmm_t *bmm, unsigned char lg_size, 
    unsigned char lg_unit, 
    unsigned char *pool, 
    bmm_mcb_t *mcb, 
    bmm_blk_list_t *blk_list) {
    int i;

    if (lg_unit<3) lg_unit=3;
    if (lg_size<=1) return -1;

    bzero(bmm, sizeof(bmm_t));
    bmm->lg_size=lg_size;
    bmm->lg_unit=lg_unit;
    bmm->unit=1<<lg_unit;
    bmm->is_pool_prelocated=1;
    bmm->is_mcb_prelocated=1;
    bmm->is_list_prelocated=1;
    bmm->fraction=0;

    if (pool==NULL) {
        if ((pool=malloc(TOTAL_SIZE(bmm)))==NULL) 
            return -1;
        bmm->is_pool_prelocated=0;
    }
    bmm->pool=pool;
    
    if (mcb==NULL) {
        if ((mcb=malloc(sizeof(bmm_mcb_t)*MCB_SIZE(bmm)))==NULL) {
            bmm_destory(bmm);
            return -1;
        }
        bmm->is_mcb_prelocated=0;
    }
    bmm->mcb=mcb;
    bzero(mcb, sizeof(bmm_mcb_t)*MCB_SIZE(bmm));
    bmm->mcb[0].lg_size=lg_size;
    bmm->mcb[0].is_allocated=0;
    bmm->mcb[0].is_buddy_allocated=0;
    
    if (blk_list==NULL) {
        if ((blk_list=malloc(sizeof(bmm_blk_list_t)*LIST_SIZE(bmm)))==NULL) {
            bmm_destory(bmm);
            return -1;
        }
        bmm->is_list_prelocated=0;
    }
    bmm->blk_list=blk_list;
    for (i=0;i<LIST_SIZE(bmm);++i) {
        blk_list[i].prev=MINUS_ONE-i;
        blk_list[i].next=MINUS_ONE-i;
    }
    add_to_blk_list(bmm, 0);

    return 0;
}

int bmm_simple_init(bmm_t *bmm, unsigned char lg_size, 
    unsigned char lg_unit) {
    return bmm_init(bmm, lg_size, lg_unit, NULL, NULL, NULL);
}

void bmm_set_fraction(bmm_t *bmm, unsigned char frac) {
    bmm->fraction=frac;
}

void bmm_destory(bmm_t *bmm) {
    if (bmm->is_pool_prelocated==0) {
        free(bmm->pool);
    }
    if (bmm->is_mcb_prelocated==0) {
        free(bmm->mcb);
    }
    if (bmm->is_list_prelocated==0) {
        free(bmm->blk_list);
    }
}
void *bmm_malloc(bmm_t *bmm, unsigned int sz) {
    unsigned int f=bmm->fraction;
    unsigned int lg_sel=bmm->lg_size, sel;
    unsigned int cand, to_split, buddy_index;
    bmm_mcb_t *mcb, *buddy_mcb;
    
    // cannot malloc
    if (sz==0) return NULL;
    if (sz>TOTAL_SIZE(bmm)) {
		return NULL;
    }
    
    // adjust sz
    sz += bmm->unit - 1;
    sz >>= bmm->lg_unit;
    
    sel=1<<lg_sel;
    while(1) {
        if (sel==sz) break;
        if (sel<sz) {
            ++lg_sel;
            sel<<=1;
            break;
        }
        --lg_sel;
        sel>>=1;
    }
    
    cand=find_fit_mem(bmm, lg_sel);
    if (cand==MINUS_ONE) {
		printf("**** bmm_malloc fail 001\n");
//		printf("**** vlCounter is %d\n",vlCounter);

		bmm_dump(bmm,0);
		return NULL;
    }
    cand=allocate_a_piece_in_big(bmm, cand, lg_sel);
    if (cand==MINUS_ONE) {
		printf("**** bmm_malloc fail 002\n");
		
		return NULL;
    }

    if (sel>sz) {
        to_split=cand;
        while ((f--)&&(lg_sel>1)) {
            buddy_index=split_mem(bmm, to_split);
            
            --lg_sel;
            sel>>=1;
            if (sz>sel) {
                mcb=get_mcb(bmm, to_split);
                mcb->is_buddy_allocated=1;
                buddy_mcb=get_mcb(bmm, buddy_index);
                buddy_mcb->is_allocated=1;
                sz-=sel;
                to_split=buddy_index;
            } else {
                add_to_blk_list(bmm, buddy_index);
                if (sz==sel) break;
            }
        }
    }
//	vlCounter++;
	//printf("vlCounter is %d\n",vlCounter);
    return get_memory(bmm, cand);
}

int bmm_free(bmm_t *bmm, void *p) {
    unsigned int index=get_index(bmm, p), merged_index;
    bmm_mcb_t *mcb;
    if (index==MINUS_ONE) return -1;

	//vlCounter--;
//	printf("bmm_free size %d , counter %d\n",bmm_get_mem_size(bmm,p),vlCounter);
    mcb=get_mcb(bmm, index);
    if (index&0x01) {
        if (mcb->is_buddy_allocated==0) return -1;
        mcb->is_buddy_allocated=0;
    } else {
        if (mcb->is_allocated==0) return -1;
        mcb->is_allocated=0;
    }
    
    while ((mcb->is_buddy_allocated)&&(mcb->lg_size>0)) {
        mcb->is_buddy_allocated=0;
        add_to_blk_list(bmm, index);
        index=find_buddy_index(bmm, index);
        mcb=get_mcb(bmm, index);
        mcb->is_allocated=0;
    }
    
    while ((merged_index=merge_free_mem(bmm, index))!=MINUS_ONE)
        index=merged_index;
    
    add_to_blk_list(bmm, index);
    return 0;
}
void *bmm_realloc(bmm_t *bmm, void *p, unsigned int new_size) {
    unsigned int index=get_index(bmm, p);
    unsigned int org_size;
    void *q;
    
    if (index==MINUS_ONE) return NULL;
    if ((org_size=bmm_get_size(bmm, index))>=new_size) return p;
    if ((q=bmm_malloc(bmm, new_size))==NULL) return NULL;
    memcpy(q, p, org_size);
    bmm_free(bmm, p);
    return q;
}
int bmm_memisbuddy(bmm_t *bmm, void *p) {
    unsigned int index=0;
    
    index=get_index(bmm, p);

	
//    printf("bmm_is Buff inPool index is %d\n",index);

	//return -1 not in buddy memory
    return index;
}  


//--------------------------------------------------------------------------
//                  debug functions
//--------------------------------------------------------------------------
void bmm_dump(bmm_t *bmm, int to_show_mcb) {
    bmm_blk_list_t *list;
    unsigned int lg_size, next, i;
    printf("[free list]\n");
    for (lg_size=0;lg_size<=bmm->lg_size;++lg_size) {
        list=get_free_list(bmm, lg_size);
        printf(" [%d]:", lg_size);
        next=list->next;
        while(!IS_BLK_LIST_INDEX(next)) {
            printf("%d ", next);
            list=get_blk_list(bmm, next);
            next=list->next;
        }
        printf("\n");
    }
    if (to_show_mcb==0) return;
    printf("[MCB]\n");
    for (i=0;i<MCB_SIZE(bmm);++i) {
        printf("  [%4d]:sz=%d, a=%d, ba=%d", i*2, bmm->mcb[i].lg_size, 
            bmm->mcb[i].is_allocated, bmm->mcb[i].is_buddy_allocated);
        if ((i%4)==3) printf("\n");
    }
}

unsigned int bmm_get_mem_size(bmm_t *bmm, void *p) {
    unsigned int index=get_index(bmm, p);
    return bmm_get_size(bmm, index);
}