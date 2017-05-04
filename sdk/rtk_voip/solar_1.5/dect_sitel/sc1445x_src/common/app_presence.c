/*
 *-----------------------------------------------------------------------------
 *                                                                             
 *               @@@@@@@      *   @@@@@@@@@           *                                     
 *              @       @             @               *                            
 *              @             *       @      ****     *                                 
 *               @@@@@@@      *       @     *    *    *                              
 *        ___---        @     *       @     ******    *                                 
 *  ___---      @       @     *       @     *         *                             
 *   -_          @@@@@@@  _   *       @      ****     *                               
 *     -_                 _ -                                                     
 *       -_          _ -                                                       
 *         -_   _ -        s   e   m   i   c   o   n   d   u   c   t   o   r 
 *           -                                                                    
 *                                                                              
 * (C) Copyright SiTel Semiconductor BV, unpublished work.file: 
 * This computer program includes Confidential, Proprietary Information and
 * is a Trade Secret of SiTel Semiconductor BV.
 * All use, disclosure, and/or reproduction is prohibited unless authorized
 * in writing. All Rights Reserved.
 *
 *-----------------------------------------------------------------------------
 * File:		 	app_presence.c
 * Purpose:		 		 
 * Created:		 	Oct 2009
 * By:		 		 YP
 * Country:		 	Greece
 *
 *-----------------------------------------------------------------------------
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <si_print_api.h>
#include "app_presence.h"
#include "../../RSSFeed/RSSReader.h"  

appl_presence *sc1445x_app_presence_notify_parser(char*info)
{
	mxml_node_t *tree;
	appl_presence *presence ;
	osip_uri_t *client;
	char *note;
	char *basicstatus;
	char *contact;

	if (info==NULL) return NULL;
	tree= mxmlLoadString(NULL, info,MXML_TEXT_CALLBACK);
	if (tree==NULL) return NULL;
    note = sc1445x_app_presence_getelement(tree,"note");
    basicstatus = sc1445x_app_presence_getelement(tree,"basic");
	contact = sc1445x_app_presence_getelement(tree,"contact");
  	if (contact){
		int ret;
		char *urlstr=contact; 
		while (urlstr[0]==' ' && strlen(urlstr)>0)
			urlstr++;
 		osip_uri_init(&client);
		ret = osip_uri_parse(client, urlstr);
  	}
	presence = (appl_presence *) malloc(sizeof(appl_presence));
	if (presence)
	{
		presence->note = note; 
		presence->basicstatus = basicstatus;
		presence->username = strdup(client->username);
		presence->host = strdup(client->host);
	}
	if (client) osip_uri_free(client);
	if (contact) free(contact);
	if (tree) mxmlDelete (tree);
	if (info) free(info);
	return presence;
}
char *sc1445x_app_presence_getelement(mxml_node_t *tree, char *element)
{
	mxml_node_t *envelope;
	if (tree==NULL || element==NULL)
		return NULL;

 	if ((tree->type==MXML_ELEMENT)&&(strcmp(tree->value.element.name,element)==0))
	{
 		envelope=tree;
	}
	else
	{
 		 envelope = mxmlFindElement (tree,tree, element,NULL,NULL,MXML_DESCEND);
		 if (!strcmp(envelope->value.element.name,element))
			 return concatenateXMLText(envelope);
 		 if (envelope==NULL)
		 {
 			 mxmlDelete(tree);
			 tree=NULL;
			 return NULL;
		 }
	}
 	return NULL;
}
