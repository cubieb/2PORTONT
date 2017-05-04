/*
 * Access and get random values from /dev/urandom
 *
 * This file is part of FONUCS. Copyright (C) 2007 Fon Wireless Ltd.
 *
 * Created: 20070407 Pablo Martín Medrano <pablo@fon.com>
 * 
 * $Id: urandom.c,v 1.1 2012/09/20 03:52:23 paula Exp $
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "urandom.h"

typedef struct {
	int fd;
} Ur;

/* UR_start */
UR_HANDLE UR_start(void)
{
	Ur *ur;

	ur = (Ur *)malloc(sizeof(Ur));
	if (ur == NULL)
		return NULL;
	ur->fd = open("/dev/urandom", O_RDONLY);
	if (ur->fd == -1) {
		free(ur);
		return NULL;
	}
	return ur;
}

/* UR_get */
UR_ERROR UR_get(UR_HANDLE handle, char *buffer, size_t size)
{
	ssize_t sread;
	Ur *ur = (Ur *)handle;
	sread = read(ur->fd, (void *)buffer, size);
	if (sread == (ssize_t)(-1))
		return UR_READ;
	if ((size_t)sread != size)
		return UR_READ;
	return UR_OK;
}

/* UR_stop */
UR_ERROR UR_stop(UR_HANDLE handle)
{
	Ur *ur = (Ur *)handle;
	close(ur->fd);
	free(ur);
	return UR_OK;
}

