#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/*-- Local inlcude files --*/
#include "../webs.h"
#include "webform.h"
#include "mib.h"
#include "multilang.h"

#define START_SCRIPT "/var/config/start_script"
#define END_SCRIPT "/var/config/end_script"

#ifdef CONFIG_INIT_SCRIPTS
#define UPLOAD_SCT_MSG(url) { \
	boaHeader(wp); \
	boaWrite(wp, "<body><blockquote><h4>The script file was uploaded successfully!" \
                "<form><input type=button value=\"  OK  \" OnClick=window.location.replace(\"%s\")></form></blockquote></body>", url);\
	boaFooter(wp); \
	boaDone(wp, 200); \
}

//copy from fmmgmt.c
//find the start and end of the upload file.
FILE * uploadGetInitScript(request *wp, unsigned int *startPos, unsigned *endPos)
{
	FILE *fp = NULL;
	struct stat statbuf;
	unsigned char *buf;
	char boundary[80];
	int i, blen, c;

	if (wp->method == M_POST)
	{
		int i;

		fstat(wp->post_data_fd, &statbuf);
		lseek(wp->post_data_fd, SEEK_SET, 0);

		//printf("file size=%d\n",statbuf.st_size);
		fp = fopen(wp->post_file_name, "rb");
		if(fp == NULL) goto error;

		memset(boundary, 0, sizeof(boundary));
		if( fgets(boundary, 80, fp) == NULL ) goto error;

		if( boundary[0] != '-' || boundary[1] != '-' ) goto error;

		i = strlen(boundary) - 1;
		while(boundary[i] == '\r' || boundary[i] == '\n')
		{
			boundary[i]='\0';
			i--;
		}
		//printf("boundary=%s\n", boundary);
	}
	else goto error;

	while(1)
	{
		if(feof(fp))
		{
			printf("Cannot find start of file!\n");
			goto error;
		}

		if(fgetc(fp) != 0xd) continue;

		if(fgetc(fp) != 0xa) continue;

		if(fgetc(fp) != 0xd) continue;

		if(fgetc(fp) != 0xa) continue;

		break;
	}

	(*startPos) = ftell(fp);

	do
	{
		if(feof(fp))
		{
			printf("Cannot find end of file\n");
			goto error;
		}

		if(fgetc(fp) != 0xd) continue;

		if(fgetc(fp) != 0xa) continue;

		blen = strlen(boundary);
		for( i = 0; i < blen; i++)
		{
			c = fgetc(fp);
			if ( c != boundary[i])
			{
				ungetc(c, fp);
				break;
			}
		}

		if( i != blen ) continue;

		break;
	}while(1);

	(*endPos) = ftell(fp) - strlen(boundary) - 2;

	return fp;
error:
   	return NULL;
}

void formInitStartScript(request * wp, char *path, char *query)
{
	char	*strData;
	char tmpBuf[100];
	FILE	*fp = NULL, *fp_input;
	char *buf;
	unsigned int startPos, endPos, nLen, nRead, i, j;

	if ((fp = uploadGetInitScript(wp, &startPos, &endPos)) == NULL)
	{
		strcpy(tmpBuf, strUploaderror);
 		goto setErr_StartScript;
 	}

	nLen = endPos - startPos;
	//printf("filesize is %d\n", nLen);
	buf = malloc(nLen+1);
	if (!buf)
	{
		strcpy(tmpBuf, strMallocFail);
 		goto setErr_StartScript;
 	}

	fseek(fp, startPos, SEEK_SET);
	nRead = fread((void *)buf, 1, nLen, fp);

	buf[nRead]=0;
	if (nRead != nLen)
 		printf("Read %d bytes, expect %d bytes\n", nRead, nLen);

	if (!(fp_input = fopen(START_SCRIPT, "w")))
		printf("Create %s file fail!\n", START_SCRIPT);

	// Replace 0x0D0A with 0x0A
	for( i = 0, j = 0; i < nRead; i++, j++ )
	{
		if( buf[i] == 0x0D )
			i++;

		buf[j] = buf[i];
	}

	buf[j] = 0;

	fprintf(fp_input, "%s\n", buf);
	free(buf);
	fclose(fp_input);

//ccwei_flatfsd
#ifdef CONFIG_USER_FLATFSD_XXX
	if( va_cmd( "/bin/flatfsd", 1, 1, "-s" ) )
		printf( "[%d]:exec 'flatfsd -s' error!",__FILE__ );
#endif

	strData = boaGetVar(wp, "submit-url", "/init_script.asp");
	UPLOAD_SCT_MSG(strData);// display reconnect msg to remote
	return;

setErr_StartScript:
	ERR_MSG(tmpBuf);
}

void formInitStartScriptDel(request * wp, char *path, char *query)
{
	char	*strData;
	FILE *fp;

	if(strData = boaGetVar(wp, "start_delete", ""))
	{
		if(fp = fopen(START_SCRIPT, "w"))
		{
			fprintf(fp, "");
			fclose(fp);
#ifdef CONFIG_USER_FLATFSD_XXX
			if( va_cmd( "/bin/flatfsd", 1, 1, "-s" ) )
				printf( "[%d]:exec 'flatfsd -s' error!",__FILE__ );
#endif
			OK_MSG1(multilang(LANG_THE_INITIATINGSTART_SCRIPT_FILE_WAS_DELETED), "/init_script.asp")
		}
		else
			ERR_MSG(multilang(LANG_FAILED_TO_DELETE_INITIATINGSTART_SCRIPT_FILE));
	}
	else
		ERR_MSG(multilang(LANG_UNEXPECTED_ERROR));
}

void formInitEndScript(request * wp, char *path, char *query)
{
	char	*strData;
	char tmpBuf[100];
	FILE	*fp = NULL, *fp_input;
	char *buf;
	unsigned int startPos, endPos, nLen, nRead, i, j;

	if ((fp = uploadGetInitScript(wp, &startPos, &endPos)) == NULL)
	{
		strcpy(tmpBuf, strUploaderror);
 		goto setErr_StartScript;
 	}

	nLen = endPos - startPos;
	//printf("filesize is %d\n", nLen);
	buf = malloc(nLen+1);
	if (!buf)
	{
		strcpy(tmpBuf, strMallocFail);
 		goto setErr_StartScript;
 	}

	fseek(fp, startPos, SEEK_SET);
	nRead = fread((void *)buf, 1, nLen, fp);

	buf[nRead]=0;
	if (nRead != nLen)
 		printf("Read %d bytes, expect %d bytes\n", nRead, nLen);

	if (!(fp_input = fopen(END_SCRIPT, "w")))
		printf("Create %s file fail!\n", END_SCRIPT);

	// Replace 0x0D0A with 0x0A
	for( i = 0, j = 0; i < nRead; i++, j++ )
	{
		if( buf[i] == 0x0D )
			i++;

		buf[j] = buf[i];
	}

	buf[j] = 0;

	fprintf(fp_input, "%s\n", buf);
	free(buf);
	fclose(fp_input);

//ccwei_flatfsd
#ifdef CONFIG_USER_FLATFSD_XXX
	if( va_cmd( "/bin/flatfsd", 1, 1, "-s" ) )
		printf( "[%d]:exec 'flatfsd -s' error!",__FILE__ );
#endif

	strData = boaGetVar(wp, "submit-url", "/init_script.asp");
	UPLOAD_SCT_MSG(strData);// display reconnect msg to remote
	return;

setErr_StartScript:
	ERR_MSG(tmpBuf);
}

void formInitEndScriptDel(request * wp, char *path, char *query)
{
	char	*strData;
	FILE *fp;

	if(strData = boaGetVar(wp, "start_delete", ""))
	{
		if(fp = fopen(END_SCRIPT, "w"))
		{
			fprintf(fp, "");
			fclose(fp);
#ifdef CONFIG_USER_FLATFSD_XXX
			if( va_cmd( "/bin/flatfsd", 1, 1, "-s" ) )
				printf( "[%d]:exec 'flatfsd -s' error!",__FILE__ );
#endif
			OK_MSG1(multilang(LANG_THE_INITIATINGEND_SCRIPT_FILE_WAS_DELETED), "/init_script.asp")
		}
		else
			ERR_MSG(multilang(LANG_FAILED_TO_DELETE_INITIATINGEND_SCRIPT_FILE));
	}
	else
		ERR_MSG(multilang(LANG_UNEXPECTED_ERROR));
}
#endif

