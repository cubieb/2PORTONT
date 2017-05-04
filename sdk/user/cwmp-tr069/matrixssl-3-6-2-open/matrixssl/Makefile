#
#	Makefile for MatrixSSL dynamic and static library
#	'make' builds debug (Default).
#	'make gold' builds optimized. 
#
#	Copyright (c) INSIDE Secure, 2002-2014. All Rights Reserved.
#

include ../common.mk

LIBRARY	= libmatrixssl$(SO)
STATIC	= libmatrixsslstatic$(A)

CORE_DYN = -L../core/ -lcore
CRYPTO_DYN = -L../crypto -lpscrypto

OBJECTS	= \
		prf$(O) \
		sslv3$(O) \
		tls$(O) \
		psk$(O) \
		cipherSuite$(O) \
		hsHash$(O) \
		matrixssl$(O) \
		matrixsslApi$(O) \
		sslDecode$(O) \
		sslSocket$(O) \
		sslEncode$(O)

#
#	Compile options
#
CFLAGS += -I./ -I../ 

all: compile

compile: $(OBJECTS) $(STATIC) $(LIBRARY)

*.o: *.h *.c

#
# Build the library
#
$(LIBRARY): $(OBJECTS)
	$(CC) $(SHARED) -o $@ $^ $(CORE_DYN) $(CRYPTO_DYN) $(LDFLAGS)

#
# Build the static library
#
$(STATIC): $(OBJECTS)
	$(AR) -rcuv $@ $^ 2>/dev/null

#
#	Clean up all generated files
#
clean:
	rm -f $(LIBRARY) $(STATIC) $(OBJECTS)
