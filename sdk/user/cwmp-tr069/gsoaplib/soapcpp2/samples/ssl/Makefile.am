####### This is the input file for automake, which will generate Makefile.in ##########
## not a GNU package. You can remove this line, if
## you have all needed files, that a GNU package needs
AUTOMAKE_OPTIONS = foreign 1.4

noinst_PROGRAMS=sslclient sslserver
SOAPHEADER=ssl.h

include ../Makefile.defines
SOAP_NS=$(SOAP_NS_DEF)
include ../Makefile.c_rules

sslclient_SOURCES=sslclient.c $(SOAPHEADER) $(SOAP_C_CLIENT)
sslserver_SOURCES=sslserver.c $(SOAPHEADER) $(SOAP_C_SERVER)
sslclient_LDADD=$(SOAP_C_LIB) $(SAMPLE_EXTRA_LIBS) -lssl -lcrypto
sslserver_LDADD=$(SOAP_C_LIB) $(SAMPLE_EXTRA_LIBS) -lssl -lcrypto
