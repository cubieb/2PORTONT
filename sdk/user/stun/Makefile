
# if you chnage this version, change in stun.h too 
VERSION=0.97

#CXXFLAGS+=-O2
#LDFLAGS+=-O2 -lssl
#STUNLIB=libstun.a

#
# Alternatively, for debugging.
#
CXXFLAGS+=-g -O -Wall
LDFLAGS+=-g -O -Wall
# for solaris
#LDFLAGS+= -lnsl -lsocket

EXEC = stun_client
all: stun_client 

clean:
	- rm *.o stun_client

stun_client: client.o stun.o udp.o 
	$(CXX) $(LDFLAGS) -o $@  $^

%.o:%.cxx
	$(CXX)  -c $(CPPFLAGS) $(CXXFLAGS) $< -o $@

libstun.a: stun.o udp.o
	ar rv $@ $<

romfs:
	$(ROMFSINST) /bin/$(EXEC)

%:RCS/%
	co $@

# Dependancies
client.o: stun.h udp.h 
stun.o: stun.h udp.h
udp.o: stun.h udp.h 
