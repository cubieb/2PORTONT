
all:
	make -C osgi_cmd
	make -C gwinterface
	
clean:
	make -C osgi_cmd clean
	make -C gwinterface clean

romfs:
	$(ROMFSINST) osgi_cmd/osgi /bin/osgi
	$(ROMFSINST) gwinterface/libgwinterface.so /usr/local/classpath/lib/classpath/libgwinterface.so
	$(ROMFSINST) gwinterface/Gateway_jni.class /usr/local/class/Gateway_jni.class
	$(ROMFSINST) gwinterface/testGateway.class /usr/local/class/testGateway.class
	
	
