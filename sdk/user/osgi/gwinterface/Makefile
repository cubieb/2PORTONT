JAVA_HOME=/usr/local/java/jdk1.6.0_45
CFLAGS=-I$(JAVA_HOME)/include -I$(JAVA_HOME)/include/linux/
LFLAGS=-shared -fPIC
JAVA_FILES=Gateway_jni.java testGateway.java

all: clean java lib

java:
	@echo
	@echo Compiling Java codes: ${JAVA_FILES}
	@javac ${JAVA_FILES}
	@echo
	@echo Generating header file for JNI
	@javah -jni Gateway_jni

lib:
	@echo
	@echo Compiling the native codes and generate shared library
	@$(CC) ${CFLAGS} $(LFLAGS) gwinterface.c -o libgwinterface.so

exec:
	@echo
	@echo Execute testGateway
	@echo
	@java -Djava.library.path=`pwd` testGateway

clean:
	@rm -f *.o
	@rm -f *.so
	@rm -f *.class
