#
# FONRSA & FONSIGN libraries unit testing
# 
# This file is part of FONUCS. Copyright (C) 2007 FON Wireless Ltd.
#
# Execute in this directory
#
# Created: 20070422 Pablo Martin Medrano <pablo@fon.com>
#
# $Id: Makefile,v 1.1 2012/09/20 03:52:09 paula Exp $
#
# FIXME: Put this in the main Makefile.am
# 
all: fonrsa fonsign foncryp foncheckrsa

fonsign: ../fonsign/sign.c
	gcc -g ../fonsign/sign.c -D__MAINTEST__ -o fonsign -lssl

foncryp: ../fonsign/sign.c
	gcc -g ../fonsign/sign.c -D__MAINTEST__ -o foncryp -lssl

foncheckrsa: ../../client/foncheckrsa/foncheckrsa.c ../base/base64.c
	gcc -g *.c ../base/base64.c ../fonsm/log.c ../../client/foncheckrsa/foncheckrsa.c -o foncheckrsa -I..

fonrsa: fonrsa.c private_fon_rsa_key.pem private_sm_rsa_key.pem ../base/base64.c
	gcc -g *.c ../base/base64.c ../fonsm/log.c -o fonrsa -D__MAINTEST__ -I..

private_fon_rsa_key.pem:
	openssl genrsa -out private_fon_rsa_key.pem 4096
	openssl rsa -in private_fon_rsa_key.pem -pubout -out public_fon_rsa_key.pem

private_sm_rsa_key.pem:
	openssl genrsa -out private_sm_rsa_key.pem 2048
	openssl rsa -in private_sm_rsa_key.pem -pubout -out public_sm_rsa_key.pem

# Data files
data256:
	dd if=/dev/urandom of=data256 bs=1 count=256
#	dd if=/dev/zero of=data256 bs=1 count=256
#	dd if=/dev/urandom of=data256 bs=1 count=256

data512:
	dd if=/dev/urandom of=data512 bs=1 count=512
#	dd if=/dev/zero of=data512 bs=1 count=512
#	dd if=/dev/urandom of=data512 bs=1 count=512

data4096:
	dd if=/dev/urandom of=data4096 bs=1 count=4096

# Encrypted with sign.c (OpenSSL)
data512.e: data512 foncryp
	./foncryp encrypt private_fon_rsa_key.pem data512 data512.e

data256.e: data256 foncryp
	./foncryp encrypt private_sm_rsa_key.pem data256 data256.e

# Decrypted with fonrsa
data512.d: data512.e fonrsa
	./fonrsa public_fon_rsa_key.pem data512.e data512.d

data256.d: data256.e fonrsa
	./fonrsa public_sm_rsa_key.pem data256.e data256.d

# Decrypted with sign.c (redundant)
data512.f: data512.e foncryp
	./foncryp decrypt public_fon_rsa_key.pem data512.e data512.f

data256.f: data256.e foncryp
	./foncryp decrypt public_sm_rsa_key.pem data256.e data256.f

# Signed data4096 with OpenSSL 
data4096.s: data4096
	openssl dgst -rmd160 -sign private_fon_rsa_key.pem  data4096 > data4096.s 

# Signed data4096 with crypt.c  
# FIXME: do this

clean: cleantest
	-rm fonsign foncheckrsa foncryp fonrsa *.pem

cleantest:
	-rm data*

test256: all data256.d data256.f
	diff data256 data256.f
	diff data256 data256.d

test512: all data512.d data512.f
	diff data512 data512.f
	diff data512 data512.d

testsign: all data4096.s
	openssl dgst -rmd160 -verify public_fon_rsa_key.pem -signature data4096.s data4096
	./foncheckrsa public_fon_rsa_key.pem data4096.s data4096

test: test256 test512 testsign
	
