#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>
#include <string.h>
#include "udpechoserverlib.h"

#define SHM_PROJ_ID	'S'

int initShmem( void **t, int s, char *name )
{
	key_t key;
	int shm_id;

	if(t==NULL || s<=0 || name==NULL)
		return -1;
		
	key = ftok(name, SHM_PROJ_ID);
	if(key==-1)
	{
		//perror("ftok");
		return -1;
	}
	
	shm_id=shmget(key, s, IPC_CREAT);
	if(shm_id==-1)
	{
		//perror("shmget");
		return -1;		
	}
	
	*t=shmat(shm_id,NULL,0);
	if( *t==(void *)-1 )
	{
		//perror("shmget");
		return -1;		
	}
	
	memset( *t, 0, s );
	
	return 0;
	
}

int getShmem( void **t, int s, char *name )
{
	key_t key;
	int shm_id;

	if(t==NULL || s<=0 || name==NULL)
		return -1;

	key = ftok(name, SHM_PROJ_ID);
	if(key==-1)
	{
		//perror("ftok");
		return -1;
	}
	
	shm_id=shmget(key, s , 0);
	if(shm_id==-1)
	{
		//perror("shmget");
		return -1;		
	}
	
	*t=shmat(shm_id,NULL,0);
	if( *t==(void *)-1 )
	{
		//perror("shmget");
		return -1;		
	}
	
	return 0;
	
}

int detachShmem( void *t )
{
	if( shmdt(t)==-1 )
	{
		//perror( "shmdt" );
		return -1;
	}
	return 0;
}
