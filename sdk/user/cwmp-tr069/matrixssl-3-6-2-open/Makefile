# Recursively build the directories below
# 66615a4 (HEAD, tag: MATRIXSSL-3-6-2-OPEN, tag: MATRIXSSL-3-6-2-COMM, origin/master, origin/HEAD, master)
all:
	$(MAKE) --directory=core
	$(MAKE) --directory=crypto
	$(MAKE) --directory=matrixssl
	
gold:
	$(MAKE) gold --directory=core
	$(MAKE) gold --directory=crypto
	$(MAKE) gold --directory=matrixssl
	
clean:
	$(MAKE) clean --directory=core
	$(MAKE) clean --directory=crypto
	$(MAKE) clean --directory=matrixssl
