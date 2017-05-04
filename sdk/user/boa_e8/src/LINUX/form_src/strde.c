/*-- System inlcude files --*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include "strde.h"

#define true 1
#define false 0

#define _sntprintf	snprintf

#define	_BC_ROOT(bc)	(*(((bc)->flag & _BC_FLAG_LIST) ? &((bc)->itcur->items) : &((bc)->items)))

_bcode*	bc_decode(const char* text);
int		bc_pushs(_bcode* bc, char* key, const char* string);
int		bc_pushsx(_bcode* bc, char* key, char* string, char iscnst);
int		bc_pushi(_bcode* bc, char* key, int value);
int		bc_pushix(_bcode* bc, char* key, int value, char iscnst);
int		bc_pushf(_bcode* bc, char* key, float value);
int		bc_pushfx(_bcode* bc, char* key, float value, char iscnst);
int		bc_encode(_bcode* bc, char* out, int max);
void	bc_free(_bcode* bc);

const char*	bc_gets(_bcode* bc, char* key);
int		bc_getv(_bcode* bc, char* key, float* pv);

_bcode*	bc_decode(const char* text)
{
	char*	key = NULL;
	char*	string = NULL;
	char*	token = NULL;
	char*	pos = NULL;
	char	bf = 0;
	char	cval = 0;
	char	type = 0;
	char	cnt = 0;
	float	val = 0.0f;
	int		vlen = 0;
	int		tlen = 0;
	int		itcnt = 0;
	_bcode*	bc = _BC_MALLOC(_bcode, 1);

	if(bc == NULL)return NULL;
	memset(bc, 0, sizeof(_bcode));

	if(text == NULL)goto finish;

	tlen = strlen(text);
	if(tlen < 4) goto finish;

	bc->buf = _BC_MALLOC(char, tlen + 2);
	if((token = bc->buf) == NULL)goto finish;
	memcpy(bc->buf, text, tlen);
	bc->buf[tlen] = 0;

	if(*token == 'l') // is list
	{
		bc->flag |= _BC_FLAG_LIST;
		token++;
		if(*token != 'd') {goto finish;}// not dict
		if(bc_seek(bc, 0, 1) < 0) {goto finish;}//create a null node
	}
	else
	{
		if(*token != 'd') {goto finish;}// not dict
		token++;
	}

	itcnt = 0;
	while((type = *token) != 0)
	{
		pos = token;
		if(type == 'd')
		{
			if(!(bc->flag & _BC_FLAG_LIST))goto finish;
			if(bc_seek(bc, itcnt++, 1) < 0) goto finish;
			type = *token++;
			pos = token;
		}
		else if(type == 'e')
		{
			*token++ = 0;
			continue;
		}
		if(type == 'i') break;//error, key name must't be a number
		while(*pos && *pos != ':') {if(!isdigit(*pos)) {goto finish;} pos++;}
		if(*pos == 0)break;//error
		*pos++ = 0;
		vlen = atoi(token);
		*token++ = 0;	//so previous identifier have a terminator
		if((vlen + (pos - bc->buf)) > tlen)break;//error, too long
		key = pos;
		pos += vlen;
		token = pos;
		if((type = *token) == 0)break;//error, value is missing
		else if(type == 'i')//is number
		{
			*token++ = 0;	//so previous identifier have a terminator
			pos = token;
			bf = 0;
			while(*pos && *pos != 'e') {if(*pos == '.') bf = true; pos++;}
			if(*pos == 0)break;//error, 'e' is missing
			*pos++ = 0;
			val = (float)atof(token);
			if(bf)bc_pushfx(bc, key, val, _BC_TRUE);//without error detecting
			else bc_pushix(bc, key, (int)val, _BC_TRUE);//without error detecting
		}
		else
		{
			while(*pos && *pos != ':') pos++;
			if(*pos == 0)break;//error
			*pos++ = 0;
			vlen = atoi(token);
			*token++ = 0;	//so previous identifier have a terminator
			if((vlen + (pos - bc->buf)) > tlen)break;//error, too long
			string = pos;
			pos += vlen;
			bc_pushsx(bc, key, string, _BC_TRUE);//without error detecting
		}
		token = pos;
	}

finish:

	return bc;
}

int		bc_seek(_bcode* bc, int index, int cnew)
{
	_bitem*	itcur = NULL;
	_bitem*	it = NULL;
	int	cnt = 0;

	if(bc == NULL || !(bc->flag & _BC_FLAG_LIST) || index < 0)return -1;

	itcur = bc->items;
	while(cnt < index && itcur && itcur->next) {itcur = itcur->next; cnt++;}
	if(itcur == NULL || cnt != index)
	{
		if(cnew == 0)return -1;
		it = _BC_MALLOC(_bitem, 1);
		memset(it, 0, sizeof(_bitem));
		it->flag |= _BC_FLAG_LIST;
		if(itcur)itcur->next = it;
		else bc->items = it;
		bc->itcur = it;
		bc->itcnt++;
		index = itcur ? (cnt + 1) : 0;
	}
	else
	{
		bc->itcur = itcur;
	}

	return index;
}

int		bc_pushs(_bcode* bc, char* key, const char* string)
{
	char* strdpk = NULL, *strdps = NULL;

	if(key == NULL || *key == 0 || string == NULL)return _BC_FALSE;
	strdpk = strdup(key);
	if(strdpk == NULL)return _BC_FALSE;
	strdps = strdup(string);
	if(strdps == NULL)return _BC_FALSE;
	if(bc_pushsx(bc, strdpk, strdps, _BC_FALSE) == _BC_TRUE) return _BC_TRUE;
	_BC_MFREE(strdpk);
	_BC_MFREE(strdps);
	return _BC_FALSE;
}

int	bc_pushsx(_bcode* bc, char* key, char* string, char iscnst)
{
	_bitem*	itprv = NULL;
	_bitem*	itcur = NULL;
	_bitem*	it = NULL;
	int		cmpval = 0;

	if(bc == NULL || key == NULL || *key == 0 || string == NULL)return _BC_FALSE;

	it = _BC_MALLOC(_bitem, 1);
	memset(it, 0, sizeof(_bitem));
	it->flag |= _BC_FLAG_STRING;
	if(iscnst) it->flag |= _BC_FLAG_CONST;
	it->key = key;
	it->vs = string;

	if(_BC_ROOT(bc) == NULL)
	{
		_BC_ROOT(bc) = it;
	}
	else
	{
		itcur = _BC_ROOT(bc);
		while(itcur && (cmpval = strcmp(it->key, itcur->key)) >= 0)
		{
			if(cmpval == 0)//existed, ignored
			{
				_BC_MFREE(it);
				return _BC_FALSE;
			}
			itprv = itcur;
			itcur = itcur->next;
		}
		if(itprv == NULL)
		{
			it->next = _BC_ROOT(bc);
			_BC_ROOT(bc) = it;
		}
		else
		{
			it->next = itprv->next;
			itprv->next = it;
		}
	}
	if(bc->flag & _BC_FLAG_LIST)bc->itcur->itcnts++;
	else bc->itcnt++;

	return _BC_TRUE;
}

int		bc_pushf(_bcode* bc, char* key, float value)
{
	char* strdpk = NULL;
	if(key == NULL || *key == 0 )return _BC_FALSE;
	strdpk = strdup(key);
	if(strdpk == NULL)return _BC_FALSE;
	if(bc_pushfx(bc, strdpk, value, _BC_FALSE) == _BC_TRUE) return _BC_TRUE;
	_BC_MFREE(strdpk);
	return _BC_FALSE;
}

int		bc_pushfx(_bcode* bc, char* key, float value, char iscnst)
{
	_bitem*	itprv = NULL;
	_bitem*	itcur = NULL;
	_bitem*	it = NULL;
	int		cmpval = 0;
	if(bc == NULL || key == NULL || *key == 0)return _BC_FALSE;

	it = _BC_MALLOC(_bitem, 1);
	memset(it, 0, sizeof(_bitem));
	it->flag |= _BC_FLAG_CMPLX;
	if(iscnst) it->flag |= _BC_FLAG_CONST;
	it->key = key;
	it->vi = value;

	if(_BC_ROOT(bc) == NULL)
	{
		_BC_ROOT(bc) = it;
	}
	else
	{
		itcur = _BC_ROOT(bc);
		while(itcur && (cmpval = strcmp(it->key, itcur->key)) >= 0)
		{
			if(cmpval == 0)//existed, ignored
			{
				_BC_MFREE(it);
				return _BC_FALSE;
			}
			itprv = itcur;
			itcur = itcur->next;
		}
		if(itprv == NULL)
		{
			it->next = _BC_ROOT(bc);
			_BC_ROOT(bc) = it;
		}
		else
		{
			it->next = itprv->next;
			itprv->next = it;
		}
	}
	if(bc->flag & _BC_FLAG_LIST)bc->itcur->itcnts++;
	else bc->itcnt++;

	return _BC_TRUE;
}

int		bc_pushi(_bcode* bc, char* key, int value)
{
	char* strdpk = NULL;
	if(key == NULL || *key == 0 )return _BC_FALSE;
	strdpk = strdup(key);
	if(strdpk == NULL)return _BC_FALSE;
	if(bc_pushix(bc, strdpk, value, _BC_FALSE) == _BC_TRUE) return _BC_TRUE;
	_BC_MFREE(strdpk);
	return _BC_FALSE;
}

int		bc_pushix(_bcode* bc, char* key, int value, char iscnst)
{
	_bitem*	itprv = NULL;
	_bitem*	itcur = NULL;
	_bitem*	it = NULL;
	int		cmpval = 0;
	if(bc == NULL || key == NULL || *key == 0)return _BC_FALSE;

	it = _BC_MALLOC(_bitem, 1);
	memset(it, 0, sizeof(_bitem));
	if(iscnst) it->flag |= _BC_FLAG_CONST;
	it->key = key;
	it->vi = (float)value;

	if(_BC_ROOT(bc) == NULL)
	{
		_BC_ROOT(bc) = it;
	}
	else
	{
		itcur = _BC_ROOT(bc);
		while(itcur && (cmpval = strcmp(it->key, itcur->key)) >= 0)
		{
			if(cmpval == 0)//existed, ignored
			{
				_BC_MFREE(it);
				return _BC_FALSE;
			}
			itprv = itcur;
			itcur = itcur->next;
		}
		if(itprv == NULL)
		{
			it->next = _BC_ROOT(bc);
			_BC_ROOT(bc) = it;
		}
		else
		{
			it->next = itprv->next;
			itprv->next = it;
		}
	}
	if(bc->flag & _BC_FLAG_LIST)bc->itcur->itcnts++;
	else bc->itcnt++;

	return _BC_TRUE;
}

int		bc_encode(_bcode* bc, char* out, int max)
{
	_bitem*	it = NULL, *it1 = NULL;
	int		wx = 0;

	if(bc == NULL || out == NULL || max <= 0)return _BC_FALSE;

	if(bc->flag & _BC_FLAG_LIST)
	{
		wx += _sntprintf(out + wx, max - wx, "l");
		if(wx == max)return _BC_TRUE;	//is not enough space to fill more content
	}

	it = bc->items;
next_map:
	if(bc->flag & _BC_FLAG_LIST)
	{
		if(it == NULL) goto final; 
		it1 = it->items;
	}
	else
	{
		it1 = bc->items;
	}

	wx += _sntprintf(out + wx, max - wx, "d");
	if(wx == max)return _BC_TRUE;	//is not enough space to fill more content

	while(it1)
	{
		wx += _sntprintf(out + wx, max - wx, "%d:%s", strlen(it1->key), it1->key);
		if(wx == max)return _BC_TRUE;	//is not enough space to fill more content
		if(it1->flag & _BC_FLAG_STRING)
		{
			wx += _sntprintf(out + wx, max - wx, "%d:%s", strlen(it1->vs), it1->vs);
			if(wx == max)return _BC_TRUE;	//is not enough space to fill more content
		}
		else if(it1->flag & _BC_FLAG_CMPLX)
		{
			wx += _sntprintf(out + wx, max - wx, "i%0.2fe", it1->vi);
			if(wx == max)return _BC_TRUE;	//is not enough space to fill more content
		}
		else
		{
			wx += _sntprintf(out + wx, max - wx, "i%de", (int)(it1->vi));
			if(wx == max)return _BC_TRUE;	//is not enough space to fill more content
		}
		it1 = it1->next;
	}

	wx += _sntprintf(out + wx, max - wx, "e");
	if(wx == max)return _BC_TRUE;	//is not enough space to fill more content

final:
	if(bc->flag & _BC_FLAG_LIST)
	{
		if(it) it = it->next;
		if(it) goto next_map;
		wx += _sntprintf(out + wx, max - wx, "e");
		if(wx == max)return _BC_TRUE;	//is not enough space to fill more content
	}

	return _BC_TRUE;
}

void	bc_free(_bcode* bc)
{
	_bitem*	it = NULL, *it1 = NULL;

	if(bc == NULL)return;

	while(it = bc->items)
	{
		bc->items = bc->items->next;
		if(it->flag & _BC_FLAG_LIST)
		{
			while(it1 = it->items)
			{
				it->items = it->items->next;
				if(!(it1->flag & _BC_FLAG_CONST))
				{
					_BC_MFREE(it1->key);
					if(it1->flag & _BC_FLAG_STRING)_BC_MFREE(it1->vs);
				}
				_BC_MFREE(it1);
			}
		}
		else if(!(it->flag & _BC_FLAG_CONST))
		{
			_BC_MFREE(it->key);
			if(it->flag & _BC_FLAG_STRING)_BC_MFREE(it->vs);
		}
		_BC_MFREE(it);
	}
	if(bc->buf)_BC_MFREE(bc->buf);
	_BC_MFREE(bc);
}

const char*	bc_gets(_bcode* bc, char* key)
{
	_bitem*	it = NULL;
	int		cmpval = 0;

	if(bc == NULL || key == NULL || *key == 0)return NULL;

	it = _BC_ROOT(bc);
	while(it)
	{
		if((cmpval = strcmp(it->key, key)) == 0)return (it->flag & _BC_FLAG_STRING ? it->vs : NULL);
		if(cmpval > 0)break;
		it = it->next;
	}
	return NULL;
}

int		bc_getv(_bcode* bc, char* key, float* pv)
{
	_bitem*	it = NULL;
	int		cmpval = 0;

	if(bc == NULL || key == NULL || *key == 0 || pv == NULL)return _BC_FALSE;

	it = _BC_ROOT(bc);
	while(it)
	{
		if((cmpval = strcmp(it->key, key)) == 0) {if(it->flag & _BC_FLAG_STRING)return _BC_FALSE; *pv = it->vi; return _BC_TRUE;}
		if(cmpval > 0)break;
		it = it->next;
	}
	return _BC_FALSE;
}

