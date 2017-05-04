SUBDIRS = src src/dlisten



all:
	${MAKE} -C src
	${MAKE} -C src/dlisten

clean:
	${MAKE} -C src clean
	${MAKE} -C src/dlisten clean

romfs:
	for x in $(SUBDIRS); do make -C $$x $@; done
