/*
linphone
Copyright (C) 2000  Simon MORLAT (simon.morlat@free.fr)

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

/* enum lookup code */

#include "enum.h"

#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <errno.h>
#include <string.h>

#define DNS_ANSWER_MAX_SIZE 2048

#undef res_search
#undef ns_initparse
#undef ns_parserr

gchar *create_enum_domain(const gchar *number){
	int len=strlen(number);
	gchar *domain=g_malloc((len*2)+10);
	int i,j;
	
	for (i=0,j=len-1;j>=0;j--){
		domain[i]=number[j];
		i++;
		domain[i]='.';
		i++;
	}
	strcpy(&domain[i],"e164.arpa");
	g_message("enum domain for %s is %s\n",number,domain);
	return domain;
}


gboolean is_a_number(const char *str){
	char *p=(char *)str;
	gboolean res=FALSE;
	gboolean space_found=FALSE;
	for(;;p++){
		switch(p[0]){
			case '9':
			case '8':
			case '7':
			case '6':
			case '5':
			case '4':
			case '3':
			case '2':
			case '1':
			case '0':
				res=TRUE;
				if (space_found) return FALSE; /* avoid splited numbers */
				break;
			case '\0':
				return res;
				break;
			case ' ':
				space_found=TRUE;
				break;
			default:
				return FALSE;
		}
	}
	return FALSE;
}
//4970072278724
gboolean is_enum(const gchar *sipaddress, char **enum_domain){
	char *p;
	p=strstr(sipaddress,"sip:");
	if (p==NULL) return FALSE; /* enum should look like sip:4369959250*/
	else p+=4;
	if (is_a_number(p)){
		if (enum_domain!=NULL){
			*enum_domain=create_enum_domain(p);
		}
		return TRUE;
	}
	return FALSE;
}



gint enum_lookup(const gchar *enum_domain, enum_lookup_res_t **res){
	int err;
	//char dns_answer[DNS_ANSWER_MAX_SIZE];
	char *begin,*end;
	char *host_result, *command;
	int i;
	gboolean forkok;
	/*
	ns_msg handle;
	int count;
	
	memset(&handle,0,sizeof(handle));
	*res=NULL;
	g_message("Resolving %s...",enum_domain);
	
	err=res_search(enum_domain,ns_c_in,ns_t_naptr,dns_answer,DNS_ANSWER_MAX_SIZE);
	if (err<0){
		g_warning("Error resolving enum:",herror(h_errno));
		return -1;
	}
	ns_initparse(dns_answer,DNS_ANSWER_MAX_SIZE,&handle);
	count=ns_msg_count(handle,ns_s_an);
	
	for(i=0;i<count;i++){
		ns_rr rr;
		memset(&rr,0,sizeof(rr));
		ns_parserr(&handle,ns_s_an,i,&rr);
		g_message("data=%s",ns_rr_rdata(rr));
	}
	*/
	command=g_strdup_printf("host -t naptr %s",enum_domain);
	forkok=g_spawn_command_line_sync(command,&host_result,NULL,&err,NULL);
	g_free(command);
	if (forkok){
		if (err!=0){
			g_warning("Host exited with %i error status.\n",err);
			return -1;
		}
	}else{
		g_warning("Could not spawn the \'host\' command.\n");
		return -1;
	}		
	g_message("Answer received from dns (err=%i): %s\n",err,host_result);
	
	begin=strstr(host_result,"sip:");
	if (begin==0) {
		g_warning("No sip address found in dns naptr answer.\n");
		return -1;
	}
	*res=g_malloc0(sizeof(enum_lookup_res_t));
	err=0;
	for(i=0;i<MAX_ENUM_LOOKUP_RESULTS;i++){
		end=strstr(begin,"!");
		if (end==NULL) goto parse_error;
		end[0]='\0';
		(*res)->sip_address[i]=g_strdup(begin);
		err++;
		begin=strstr(end+1,"sip:");
		if (begin==NULL) break;
	}
	g_free(host_result);
	return err;

	parse_error:
		g_free(*res);
		g_free(host_result);
		*res=NULL;
		g_warning("Parse error in enum_lookup().\n");
		return -1;
}

void enum_lookup_res_free(enum_lookup_res_t *res){
	int i;
	for (i=0;i<MAX_ENUM_LOOKUP_RESULTS;i++){
		if (res->sip_address[i]!=NULL) g_free(res->sip_address[i]);
	}
	g_free(res);
}
