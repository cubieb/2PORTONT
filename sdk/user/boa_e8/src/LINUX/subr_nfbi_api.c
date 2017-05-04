#include "utility.h"
#include "subr_nfbi_api.h"
#include "user/rtl867x_nfbi/ucd/uc_mib.c"
#include "user/rtl867x_nfbi/ucd/uc_udp.c"


/*************************************************************************/
static int slv_get_srvip(char *p)
{
	if(!p) return -1;
	strcpy(p, "127.0.0.1");
	return 0;
}


/** basic API*******************************************************************/
//return 1:ok, 0:failed
static char _adsl_slv_drv_get(unsigned int id, void *rValue, unsigned int len)
{
#ifdef EMBED
{
	char b[UC_MAX_SIZE], *data;
	UC_DATA_HDR *pdsl;
	int retlen,sendlen;
	char sysip4slv[16];

	//printf( "%s(): enter\n", __FUNCTION__ );
	if((len+UC_DATA_HDR_SIZE)>UC_DATA_SIZE)
	{
		printf("%s(): len is too large\n", __FUNCTION__);
		return 0;
	}
	if( (len>0)&&(rValue==NULL) )
	{
		printf("%s(): rValue==NULL, len=%d\n", __FUNCTION__, len);
		return 0;
	}
	data=b+UC_HDR_SIZE;

	slv_get_srvip(sysip4slv);
	pdsl=(UC_DATA_HDR*)data;
	pdsl->id=id;
	pdsl->len=len;
	if(len>0) memcpy( pdsl->data, rValue, len );
	sendlen=UC_DATA_HDR_SIZE+pdsl->len;
	retlen=uc_udp_sendrecv( sysip4slv, UC_CMD_DSL_IOC, b, sendlen, sizeof(b) );
	if(retlen<0)
	{
		//printf( "%s(): id=%x return error(%d)\n", __FUNCTION__, id, retlen );
		return 0;//fail
	}
	if(retlen!=sendlen)
	{
		printf( "%s(): return len is not the same(%d,%d)\n", __FUNCTION__, retlen, sendlen );
		return 0;//fail
	}
	if( (pdsl->id!=id) ||
		(pdsl->len!=len) )
	{
		printf( "%s(): check id(%d,%d) / len(%d,%d) error\n", __FUNCTION__, pdsl->id, id, pdsl->len, len);
		return 0;//fail
	}
	if(pdsl->len>0) memcpy( rValue, pdsl->data, pdsl->len );

	return 1;//ok
}
#endif
	return 0;
}

//return 1:ok, 0:failed
char adsl_slv_drv_get(unsigned int id, void *rValue, unsigned int len)
{
	char ret;
	void *new_rValue=rValue;
	unsigned int new_len=len;
#ifdef CONFIG_VDSL	
	MSGTODSL *m=rValue;
	DEF_MSGTODSL dm;
#endif /*CONFIG_VDSL*/


#ifdef CONFIG_VDSL
	if( (id==RLCM_UserSetDslData) || (id==RLCM_UserGetDslData) )
	{
		//printf( "id=RLCM_UserSetDslData/RLCM_UserGetDslData\n" );
		dm.message=m->message;
		memcpy( dm.intVal, m->intVal, sizeof(dm.intVal) );
		new_rValue=&dm;
		new_len=sizeof(dm);
	}
#endif /*CONFIG_VDSL*/

	ret=_adsl_slv_drv_get( id, new_rValue, new_len );

#ifdef CONFIG_VDSL
	if(ret) 
	{
		if( (id==RLCM_UserSetDslData) || (id==RLCM_UserGetDslData) )
		{
			memcpy( m->intVal, dm.intVal, sizeof(dm.intVal) );			
		}
	}
#endif /*CONFIG_VDSL*/

	return ret;
}

//return 1:ok, 0:failed
int mib_slv_get(int id, void *value)
{
	char b[UC_MAX_SIZE], *data;
	UC_DATA_HDR *p;
	int size,len, count, ret=0;
	char sysip4slv[16];

	//printf( "%s(): enter\n", __FUNCTION__ );
	data=b+UC_HDR_SIZE;
	slv_get_srvip(sysip4slv);
	size=uc_mib_get_size_by_id(id);
	if(size>0)
	{
		p=(UC_DATA_HDR*)data;
		p->id=id;
		p->len=0;
		len=UC_DATA_HDR_SIZE+p->len;
		count=uc_udp_sendrecv( sysip4slv, UC_CMD_GET_MIB_BYID, b, len, sizeof(b) );
		if( (count>=UC_DATA_HDR_SIZE) &&
			(p->id==id) &&
			(p->len==size) &&
			(count==(UC_DATA_HDR_SIZE+p->len)) )
		{
			memcpy( value, p->data, p->len );
			ret=1;
		}else{
			if(count>0)
			{
				printf( "%s(): check error count=%d, p->id=%u(%u), p->len=%u(%u)\n",
					__FUNCTION__, count, p->id, id, p->len, size );
			}
		}
	}else
		printf( "%s(): unknown ID=%d\n", __FUNCTION__, id );

	//printf( "%s(): return %d\n", __FUNCTION__, ret );
	return ret;
}

//return 1:ok, 0:failed
int mib_slv_set(int id, void *value)
{
	char b[UC_MAX_SIZE], *data;
	UC_DATA_HDR *p;
	int size, len, count, ret=0;
	char sysip4slv[16];

	//printf( "%s(): enter\n", __FUNCTION__ );
	data=b+UC_HDR_SIZE;
	slv_get_srvip(sysip4slv);
	size=uc_mib_get_size_by_id(id);
	if(size>0)
	{
		p=(UC_DATA_HDR*)data;
		p->id=id;
		p->len=size;
		memcpy( p->data, value, size );
		len=UC_DATA_HDR_SIZE+p->len;
		count=uc_udp_sendrecv( sysip4slv, UC_CMD_SET_MIB_BYID, b, len, sizeof(b) );
		if(count>=0)
		{
			ret=1;
		}
	}else
		printf( "%s(): unknown ID=%d\n", __FUNCTION__, id );

	//printf( "%s(): return %d\n", __FUNCTION__, ret );
	return ret;
}

//return 1:ok, 0:failed
int sys_slv_init( char *cmd )
{
	int len,count,ret=0;
	char b[UC_MAX_SIZE], *data;
	char sysip4slv[16];

	//printf( "%s(): enter cmd=%s\n", __FUNCTION__, (!cmd)?"":cmd );
	if(cmd==NULL) return ret;
	data=b+UC_HDR_SIZE;
	len = strlen(cmd);
	if( (len+UC_HDR_SIZE)>sizeof(b) ) return ret;

	slv_get_srvip(sysip4slv);
	sprintf(data,"%s",cmd);
	count=uc_udp_sendrecv( sysip4slv, UC_CMD_SYS_INIT, b, len, sizeof(b) );
	if(count>=0)
	{
		ret=1;
	}
	//printf( "%s(): return %d\n", __FUNCTION__, ret );
	return ret;
}


/***xdsl-related*******************************************************************/
#ifdef CONFIG_VDSL
/*pval: must be an int[4]-arrary pointer*/
static char dsl_slv_msg(unsigned int id, int msg, int *pval)
{
	MSGTODSL msg2dsl;
	char ret=0;

	msg2dsl.message=msg;
	msg2dsl.intVal=pval;
	ret=adsl_slv_drv_get(id, &msg2dsl, sizeof(MSGTODSL));

	return ret;
}

char dsl_slv_msg_set_array(int msg, int *pval)
{
	char ret=0;

	if(pval)
	{
		ret=dsl_slv_msg(RLCM_UserSetDslData, msg, pval);
	}
	return ret;
}

char dsl_slv_msg_set(int msg, int val)
{
	int tmpint[4];
	char ret=0;

	tmpint[0]=val;
	ret=dsl_slv_msg_set_array(msg, tmpint);
	return ret;
}

char dsl_slv_msg_get_array(int msg, int *pval)
{
	char ret=0;

	if(pval)
	{
		ret=dsl_slv_msg(RLCM_UserGetDslData, msg, pval);
	}
	return ret;
}

char dsl_slv_msg_get(int msg, int *pval)
{
	int tmpint[4];
	char ret=0;

	if(pval)
	{
		ret=dsl_slv_msg_get_array(msg, tmpint);
		if(ret) *pval=tmpint[0];
	}
	return ret;
}
#endif /*CONFIG_VDSL*/


/***mib-related*******************************************************************/
struct mibtbl_sync_mapping
{
	int	id;
	int id_slv;
};
static struct mibtbl_sync_mapping mibsync_sys[]=
{
	{MIB_DEVICE_TYPE,		UC_MIB_DEVICE_TYPE},
	{MIB_INIT_LINE,			UC_MIB_INIT_LINE},
	{0,						0}
};

static struct mibtbl_sync_mapping mibsync_dsl[]=
{
	{MIB_ADSL_MODE,					UC_MIB_ADSL_MODE},
	{MIB_ADSL_OLR,					UC_MIB_ADSL_OLR},
	{MIB_ADSL_TONE,					UC_MIB_ADSL_TONE},
	{MIB_ADSL_HIGH_INP,				UC_MIB_ADSL_HIGH_INP},
#ifdef FIELD_TRY_SAFE_MODE
	{MIB_ADSL_FIELDTRYSAFEMODE,		UC_MIB_ADSL_FIELDTRYSAFEMODE},
	{MIB_ADSL_FIELDTRYTESTPSDTIMES,	UC_MIB_ADSL_FIELDTRYTESTPSDTIMES},
	{MIB_ADSL_FIELDTRYCTRLIN,		UC_MIB_ADSL_FIELDTRYCTRLIN},
#endif /*FIELD_TRY_SAFE_MODE*/
#ifdef CONFIG_VDSL
	{MIB_VDSL2_PROFILE,				UC_MIB_VDSL2_PROFILE},
#endif /*CONFIG_VDSL*/
	{0,					0}
};

static int mib_slv_sync_table(struct mibtbl_sync_mapping *mibsync)
{
	int i=0;
	unsigned char b[256];

	//printf( "%s(): enter\n", __FUNCTION__ );	
	while( mibsync[i].id!=0 )
	{
		//printf( "%s(): sync id_slv=%d\n", __FUNCTION__, mibsync[i].id_slv );	
		memset( b, 0, sizeof(b) );
		mib_get( mibsync[i].id, b );
		mib_slv_set( mibsync[i].id_slv, b );

#if 0
		{//test mib_slv_get
			unsigned char tmp[256];
			int size;
			size=uc_mib_get_size_by_id( mibsync[i].id_slv );
			memset( tmp, 0, sizeof(tmp) );
			mib_slv_get( mibsync[i].id_slv, tmp );
			if( memcmp( b, tmp, size )!=0 )
				printf( "%s(): memcmp value error, id=%d\n", 
						__FUNCTION__, mibsync[i].id );
		}
#endif

		i++;
	}
	//printf( "%s(): return\n", __FUNCTION__ );
	return 0;
}


int mib_slv_sync_dsl(void)
{
	mib_slv_sync_table(mibsync_dsl);
	return 0;
}

int mib_slv_sync_all(void)
{
	mib_slv_sync_table(mibsync_sys);
	mib_slv_sync_table(mibsync_dsl);
	return 0;
}
/*************************************************************************/

