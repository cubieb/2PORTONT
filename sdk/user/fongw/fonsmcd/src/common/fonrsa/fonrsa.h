/*
 * FONSM RSA handling library, used by fonsmcd and foncheckrsa
 *
 * This file is part of FONUCS. Copyright (C) 2007 FON Wireless Ltd.
 *
 * This library is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Created: 20070306 Pablo Martin Medrano <pablo@fon.com>
 *
 * $Id: fonrsa.h,v 1.1 2012/09/20 03:52:09 paula Exp $
 */
#ifndef _FONRSA_H
#define _FONRSA_H

typedef enum {
	FONRSA_OK = 0,
	FONRSA_VERIFICATION_FAILURE = 1,
	FONRSA_OPENKEY = 2,
	FONRSA_SIZE = 3,
	FONRSA_LOADFILE = 4,
	FONRSA_CRYPT = 5,
	FONRSA_DECRYPT = 6,
	FONRSA_SAVEFILE = 7,
	FONRSA_NOSYS = 8,
	FONRSA_VERIFY = 9
} FONRSA_ERROR;

void *FR_init(char *public_key_path);
FONRSA_ERROR FR_end(void *handle);
FONRSA_ERROR FR_verify_buffer(void *handler, unsigned char *text, unsigned int size_text,
	unsigned char *signature, unsigned int size_signature);
FONRSA_ERROR FR_decrypt_buffer(void *handler, unsigned char *cryptext, int cryptext_size,
	unsigned char *plaintext, int plaintext_buffer_size, int *plaintext_size);
FONRSA_ERROR FR_verify_file(void *handler, char *file_path, char *signature_file_path);
FONRSA_ERROR FR_decrypt_file(void *handler, char *crypted_file_path, char *plaintext_file_path);

#endif

