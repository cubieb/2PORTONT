/*
 * Copyright (C) 2009 Realtek Semiconductor Corp.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * $Revision: 39101 $
 * $Date: 2013-05-03 17:35:27 +0800 (Fri, 03 May 2013) $
 *
 * Purpose : Realtek Switch SDK Rtusr API Module
 *
 * Feature : The file have include the following module and sub-modules
 *           1) utility
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include <common/rt_type.h>
#include <common/rt_error.h>


#define SETSOCKOPT(optid, varptr, vartype, qty) \
	{ \
		unsigned int ret; \
		int	sockfd; \
		if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) == -1) { \
			return -1; \
		} \
		if ((ret=setsockopt(sockfd, IPPROTO_IP, optid, (void *)varptr, sizeof(vartype)*qty)) != 0) { \
			close(sockfd); \
			return ret; \
		} \
		close(sockfd); \
	}

#define GETSOCKOPT(optid, varptr, vartype, qty) \
	{ \
		unsigned int ret; \
		int	sockfd; \
		int	len; \
		if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_RAW)) == -1) { \
			return -1; \
		} \
		len = sizeof(vartype) * qty; \
		if ((ret=getsockopt(sockfd, IPPROTO_IP, optid, (void *)varptr, &len)) != 0) { \
			close(sockfd); \
			return ret; \
		} \
		close(sockfd); \
	}
	


