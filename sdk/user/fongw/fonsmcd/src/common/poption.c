/*
 * Command line parser
 *
 * This file is part of FONUCS. Copyright (C) 2006,2007 FON Wireless Ltd.
 *
 * Created: 20050919 Pablo Martin <pablo.martin@fon.com>
 *
 * $Id: poption.c,v 1.1 2012/09/20 03:51:37 paula Exp $
 */

#include <getopt.h>
#include <libgen.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "poption.h"

#define BUG_REPORT_ADDRESS "pablo@fon.com"

static OP_ERROR OP_show_usage(OP_option *options_array, char *program_name);

OP_ERROR OP_parse(OP_option *options_array, int argc, char **argv)
{
	return OP_parse_full(options_array, argc, argv, NULL);
}

OP_ERROR OP_parse_full(OP_option *options_array, int argc, char **argv, int *next_argc)
{
	OP_ERROR return_value;
	char *program_name_full;
	char *program_name;
	struct option *long_options;
	char short_options[256];
	int i, j, k, num_options;
	int indexptr;
	int c;

	if ((program_name_full = strdup(argv[0])) == NULL)
		return OP_ERROR_NOMEM;
	program_name = basename(program_name_full);

	num_options = 1;
	for (i = 0; options_array[i].description != NULL; i++) {
		if (options_array[i].type != OP_TYPE_TITLE)
			num_options++;
	}
	long_options = (struct option *)malloc(sizeof(struct option) * (num_options + 1));
	for (i = 0; i <= num_options; i++)
		long_options[i].name = NULL;

	if (long_options == NULL) {
		free(program_name);
		return OP_ERROR_NOMEM;
	}

	return_value = OP_ERROR_OK;
	j = 0;
	k = 0;
	short_options[k++] = '+';
	for (i = 0; options_array[i].description != NULL; i++) {
		if ((options_array[i].type == OP_TYPE_TITLE) || (options_array[i].type == OP_TYPE_PARAMETERS))
			continue;
		if (options_array[i].long_option != NULL) {
			if (strlen(options_array[i].long_option) > 20) {
				return_value = OP_ERROR_VALUE;
				break;
			}
			long_options[j].name = strdup(options_array[i].long_option);
			if (options_array[i].type != OP_TYPE_FLAG) {
				long_options[j].has_arg = required_argument;
				long_options[j].flag = (int *)NULL;
				long_options[j].val = i;
			} else {
				long_options[j].has_arg = no_argument;
				long_options[j].flag = (int *)options_array[i].data;
				long_options[j].val = i;
			}
			j++;
		} else if (options_array[i].short_option == '\0') {
			return_value = OP_ERROR_VALUE;
			break;
		}
		if (options_array[i].short_option != '\0') {
			short_options[k++] = options_array[i].short_option;
			if (options_array[i].type != OP_TYPE_FLAG)
				short_options[k++] = ':';
		}
	}
	short_options[k++] = 'h';
	short_options[k] = '\0';
	long_options[j].name = strdup("help");
	long_options[j].has_arg = no_argument;
	long_options[j].flag = (int *)NULL;
	long_options[j].val = '?';
	j++;
	long_options[j].name = NULL;
	long_options[j].has_arg = 0;
	long_options[j].flag = (int *)NULL;
	long_options[j].val = 0;

	while ((c = getopt_long(argc, argv, short_options, long_options, &indexptr)) != -1) {
		if (c == 0) {
			c = long_options[indexptr].val;
		} else if ((c == '?') || (c == 'h')) {
			return_value = OP_show_usage(options_array, program_name);
			break;
		} else {
			for (i = 0; options_array[i].description != NULL; i++) {
				if (options_array[i].short_option == c) {
					c = i;
					break;
				}
			}
		}
		switch(options_array[c].type) {
			case OP_TYPE_STRING:
				*((char **)(options_array[c].data)) = (void *)strdup(optarg);
				if (*((void **)options_array[c].data) == NULL)
					return_value = OP_ERROR_NOMEM;
				break;
			case OP_TYPE_INT:
				*((int *)options_array[c].data) = atoi(optarg);
				break;
			case OP_TYPE_FLAG:
				*((int *)options_array[c].data) = 1;
				break;
			case OP_TYPE_CALLBACK:
				if (((OP_option_callback)(options_array[c].data))(optarg))
					return_value = OP_ERROR_UNKNOWN;
				break;
			default:
				break;
		}
		if (return_value != OP_ERROR_OK)
			break;
	}
	if (next_argc != NULL)
		*next_argc = optind;
	for (i = 0; i <= num_options; i++)
		if(long_options[i].name != NULL)
			free((char *)long_options[i].name);

	free(program_name_full);
	free(long_options);
	return return_value;
}

static OP_ERROR OP_show_usage(OP_option *options_array, char *program_name)
{
	int i, index_title = -1, index_parameters = -1;
	char buffer_long_option[256];
	char buffer_short_option[256];

	for (i = 0; options_array[i].description != NULL; i++) {
		if (options_array[i].type == OP_TYPE_TITLE) {
			index_title = i;
		} else if (options_array[i].type == OP_TYPE_PARAMETERS) {
			index_parameters = i;
		}
	}
	if (index_title == -1)
		return OP_ERROR_VALUE;
	printf("\n %s\n\n", options_array[index_title].description);
	if (index_parameters == -1)
		printf(" Usage: %s [OPTIONS]\n\n", program_name);
	else
		printf(" Usage: %s [OPTIONS] %s\n\n", program_name, options_array[index_parameters].description);
	for (i = 0; options_array[i].description != NULL; i++) {
		if ((options_array[i].type == OP_TYPE_TITLE) || (options_array[i].type == OP_TYPE_PARAMETERS))
			continue;
		if (options_array[i].short_option != '\0')
			sprintf(buffer_short_option, "-%c, ", options_array[i].short_option);
		else
			sprintf(buffer_short_option, "    ");
		if (options_array[i].long_option != NULL) {
			if (strlen(options_array[i].long_option) > 20)
				return OP_ERROR_VALUE;
			sprintf(buffer_long_option, "                     ");
			sprintf(buffer_long_option, "--%s", options_array[i].long_option);
			buffer_long_option[strlen(buffer_long_option)] = ' ';
		} else {
			sprintf(buffer_long_option, "                     ");
		}
		printf("  %s%s%s\n", buffer_short_option, buffer_long_option, options_array[i].description);
	}
	printf("  -h, --help               show this help and exit\n");
	printf("\n Report bugs to <%s>\n", BUG_REPORT_ADDRESS);
	return OP_ERROR_HELP;
}

