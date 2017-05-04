#ifndef _INC_STRDE_
#define _INC_STRDE_

#define _BC_FLAG_STRING		0x01
#define _BC_FLAG_CMPLX		0x02
#define _BC_FLAG_LIST		0x04
#define _BC_FLAG_CONST		0x80
#define _BC_MALLOC(t,n)		((t*)malloc(sizeof(t)*n))
#define _BC_MFREE(p)		free(p)

#define _BC_TRUE			1
#define _BC_FALSE			0

typedef struct _bitem
{
	char			flag;
    union 
	{
		struct
		{
	char *			key;
			union
			{
	char *			vs;
	float			vi;
			};
		};
		struct
		{
	int				itcnts;
	struct _bitem*	items;
		};
	};
	struct _bitem*	next;
}_bitem;

typedef struct _bcode
{
	int			flag;
	int			itcnt;
	_bitem*		items;
	_bitem*		itcur;
	char*		buf;
}_bcode;

#ifdef __cplusplus
extern "C"{
#endif

	_bcode*	bc_decode(const char* text);
	int		bc_seek(_bcode* bc, int index, int cnew);
	int		bc_pushs(_bcode* bc, char* key, const char* string);
	int		bc_pushf(_bcode* bc, char* key, float value);
	int		bc_pushi(_bcode* bc, char* key, int value);
	int		bc_encode(_bcode* bc, char* out, int max);
	void	bc_free(_bcode* bc);
	const char*	bc_gets(_bcode* bc, char* key);
	int		bc_getv(_bcode* bc, char* key, float* pv);

#ifdef __cplusplus
}
#endif

#endif /*_INC_STRDE_*/
