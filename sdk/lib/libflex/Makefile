
objs-libfl := libmain.o libyywrap.o

all: libfl.a

libfl.a: $(objs-libfl)
	-rm -f libfl.a
	$(AR) -rcuv libfl.a $(objs-libfl)	
	$(RANLIB) libfl.a
	#$(TMPFSINST) /libflex/libfl.a

clean: ; -rm -f *.a *.o *.so

romfs:



