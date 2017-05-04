#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

int main(int argc, char **argv)
{
	int  bftp;
	char *p_temp;
	char *p_URL;
	char *p_user;
	char *p_password;
	char *continue_opt="-c";
	char p_dir[258];
	struct stat fstate;
#ifdef CONFIG_E8B
	int port;
#endif
	int  i_res;
	char *arg_array[8];
	char execute_filename[] = "/bin/wget";
	int i;
	char p_p_char;
	char *p_path;
	
	printf("wget_manager:Wget manage; argc=%d\n", argc);
	if (argc < 2)
	{
		printf("Too little args!\n");
		return -1;
	}

	for (i=0; i<argc; i++)
	{
		printf("argv[%d]=%s\n", i, argv[i]);
	}

#if 0
	// check the programe name
	if ( strcmp(argv[0], "wget_manage") && strcmp(argv[0], "/bin/wget_manage") )
	{
		printf("Wrong executive file name!\n");
		return -2;
	}
#endif
#ifdef CONFIG_E8B
	//get port
	for (i=1; i<argc; i++)
	{
		if ( ! strncmp(argv[i], "port=", 5) )
		{
			break;
		}
	}

	if (i<argc)
	{
		port = atoi(&(argv[i][5]));
		printf("port=%d!\n", port);
	}
	else
	{
		printf("Fail to get port!\n");		
		return -3;
	}
#endif
	// get url
	bftp = -1;
	for (i=1; i<argc; i++)
	{
		if ( ! strncmp(argv[i], "ftp://", 6) )
		{
			bftp = 1;
			break;
		}
		else if ( ! strncmp(argv[i], "http://", 7) )
		{
			bftp = 0;
			break;
		}
	}

	if (bftp == -1)
	{
		printf("Fail to get URL!\n");
		return bftp;
	}
	else
	{
#ifdef CONFIG_E8B
		p_URL = malloc(strlen(argv[i])+32);
#else
		p_URL = malloc(strlen(argv[i])+1);
#endif
		if ( p_URL != NULL )
		{
#ifdef CONFIG_E8B
			char *p;
			if(bftp==1)
				p=strstr(argv[i]+6,"/");
			else if(bftp==0)
				p=strstr(argv[i]+7,"/");
			if(!p) {
				free(p_URL);
				return -3;
			}
			printf("p=%s\n",p);
			memcpy(p_URL,argv[i],p-argv[i]);			 
			if(port)
				sprintf(p_URL,"%s:%d%s",p_URL,port,p);
			else
				sprintf(p_URL,"%s%s",p_URL,p);		 	
#else
			strcpy(p_URL, argv[i]);
#endif
			printf("URL=%s!\n", p_URL);
		}
		else
		{
			bftp = -10;
			printf("Fail to get memory!\n");
			return bftp;
		}
	}

	// get user
	for (i=1; i<argc; i++)
	{
		if ( ! strncmp(argv[i], "user=", 5) )
		{
			break;
		}
	}

	if (i<argc)
	{
		p_user = &(argv[i][5]);
		printf("username=%s!\n", p_user);
	}
	else
	{
		printf("Fail to get user!\n");
		free(p_URL);
		return -2;
	}

	// get password
	for (i=1; i<argc; i++)
	{
		if ( ! strncmp(argv[i], "passwd=", 7) )
		{
			break;
		}
	}

	if (i<argc)
	{
		p_password = &(argv[i][7]);
		printf("password=%s!\n", p_password);
	}
	else
	{
		printf("Fail to get password!\n");
		free(p_URL);
		return -3;
	}

#if 0
	// get path
	i_res = stat("/var/wget_path", &fstate);

	if ( i_res == 0 )
	{
		if ( S_ISLNK(fstate.st_mode) )
		{
			printf("Success to get path a link!\n");
		}
		else if ( S_ISDIR(fstate.st_mode) )
		{
			printf("Success to get path a dir!\n");
		}
		else
		{
			printf("Fail to get path!\n");
			free(p_URL);
			bftp = -9;
			return bftp;
		}
	}
	else if ( i_res == ENOENT )
	{
		printf("Fail to get path not exist!\n");
		free(p_URL);
		bftp = -9;
		return bftp;
	}
	else
	{
		printf("Fail to get path other!\n");
		free(p_URL);
		bftp = -9;
		return bftp;
	}

	printf("sprintf!\n");
#endif

	// get path 2
	for (i=1; i<argc; i++)
	{
		if ( ! strncmp(argv[i], "path=", 5) )
		{
			printf("Success to get path from arg!\n");
			break;
		}
	}

	if (i<argc)
	{
		printf("Success to get path from arg (i<argc)!\n");
		p_path = &(argv[i][5]);
		printf("path=%s!\n", p_path);
	}
	else
	{
		printf("Fail to get path!\n");
		free(p_URL);
		return -3;
	}

	sprintf(p_dir, "--directory-prefix=%s", p_path);

	// rebuild user
	if ( bftp == 1 )
	{
		p_temp = malloc( strlen(p_URL) + 1 + strlen(p_user) + 1 + strlen(p_password) + 1 );
		if ( p_temp != NULL )
		{
			printf("Rebuild ftp URL...\n");
			strncpy(p_temp, p_URL, 6);
			p_temp[6] = 0;
			strcat(p_temp, p_user);
			if ( strcmp(p_user, "") )
			{
				if ( strcmp(p_password, "") )
				{
					strcat(p_temp, ":");
					strcat(p_temp, p_password);
				}
				strcat(p_temp, "@");
			}
			strcat(p_temp, &(p_URL[6]));
			free(p_URL);
			p_URL = p_temp;
			printf("URL=%s!\n", p_URL);
		}
		else
		{
			bftp = -10;
			printf("Fail to get memory!\n");
			free(p_URL);
			return bftp;
		}
	}
	else if ( bftp == 0 )
	{
		printf("Rebuild http URL...\n");
		p_temp = malloc( strlen(p_user) + 13 );
		if ( p_temp != NULL )
		{
			strcpy(p_temp, "--http-user=");
			strcat(p_temp, p_user);
			p_user = p_temp;
			printf("user:%s!\n", p_user);
		}
		else
		{
			bftp = -10;
			printf("Fail to get memory!\n");
			free(p_URL);
			return bftp;
		}

		p_temp = malloc( strlen(p_password) + 15 );
		if ( p_temp != NULL )
		{
			strcpy(p_temp, "--http-passwd=");
			strcat(p_temp, p_password);
			p_password = p_temp;
			printf("password:%s!\n", p_password);
		}
		else
		{
			bftp = -10;
			printf("Fail to get memory!\n");
			free(p_user);
			free(p_URL);
			return bftp;
		}
	}

	arg_array[0] = execute_filename;
	if (bftp == 1)
	{
		printf("Print args ftp\n");
		arg_array[1] = p_dir;
		arg_array[2] = continue_opt;
#ifdef CONFIG_E8B
		arg_array[3] = "--passive-ftp";
		arg_array[4] = p_URL;
		arg_array[5] = NULL;
#else
		arg_array[3] = p_URL;
		arg_array[4] = NULL;
#endif
#ifdef CONFIG_E8B
		for (i=0; i<5; i++)
#else
		for (i=0; i<4; i++)
#endif
		{
			printf("arg_array[%d]=%s\n", i, arg_array[i]);
		}
	}
	else
	{
		printf("Print args http\n");
		arg_array[1] = p_user;
		arg_array[2] = p_password;
		arg_array[3] = p_dir;
		arg_array[4] = continue_opt;
		arg_array[5] = p_URL;
		arg_array[6] = NULL;

		for (i=0; i<6; i++)
		{
			printf("arg_array[%d]=%s\n", i, arg_array[i]);
		}
	}

	printf("execve ! \n");
	if (execv(execute_filename, arg_array) == -1 )
	{
		perror("execve");
		return -8;
	}

	return 0;
}

