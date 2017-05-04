PROG := nfs-utils-1.2.8

all:
	cd $(PROG) ; \
	[ -f Makefile ] || ./configure --host=mips-linux --target=mips-linux --disable-nfsv4 --disable-nfsv41 \
	--without-tcp-wrappers --disable-tirpc --disable-gss --disable-uuid; \
	make

clean:
	make -C $(PROG) clean

romfs:
	$(ROMFSINST) $(PROG)/utils/exportfs/exportfs /bin/
	$(ROMFSINST) $(PROG)/utils/mountd/mountd /bin/
	$(ROMFSINST) $(PROG)/utils/nfsd/nfsd /bin/
	$(ROMFSINST) $(PROG)/utils/showmount/showmount /bin/
	$(ROMFSINST) $(PROG)/utils/nfsstat/nfsstat /bin/