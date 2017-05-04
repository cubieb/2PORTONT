/*
 * Command line parser
 *
 * This file is part of FONUCS. Copyright (C) 2006,2007 FON Wireless Ltd.
 *
 * Created: 20050919 Pablo Martin <pablo.martin@fon.com>
 *
 * $Id: poption.h,v 1.1 2012/09/20 03:51:37 paula Exp $
 */

#ifndef _POPTION_H
#define _POPTION_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
	OP_TYPE_STRING,
	OP_TYPE_INT, 
	OP_TYPE_FLAG,
	OP_TYPE_TITLE,
	OP_TYPE_PARAMETERS,
	OP_TYPE_CALLBACK
} OP_type;

typedef struct {
	const char *long_option; 
	char short_option;
	OP_type type; 
	const char *description;
	void *data; 
} OP_option;

typedef enum {
	OP_ERROR_OK,
	OP_ERROR_VALUE,
	OP_ERROR_PARSE,
	OP_ERROR_NOMEM,
	OP_ERROR_UNKNOWN,
	OP_ERROR_HELP
} OP_ERROR;

typedef int (*OP_option_callback) (const char *value);

OP_ERROR OP_parse(OP_option *options_array, int argc, char **argv);

OP_ERROR OP_parse_full(OP_option *options_array, int argc, char **argv, int *next_argc);
#ifdef __cplusplus
}
#endif

#endif /* #ifdef POPTION_H */


