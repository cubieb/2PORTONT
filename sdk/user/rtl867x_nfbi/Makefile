#rtl867x_nfbi makefile

.EXPORT_ALL_VARIABLES:

RC_LEVEL=/etc/init.d/rc24

all: 
ifdef CONFIG_USER_NFBI_MASTER
	$(MAKE) -C nfbi
endif #CONFIG_USER_NFBI_MASTER
ifdef CONFIG_USER_NFBI
	$(MAKE) -C ucd
endif #CONFIG_USER_NFBI
ifdef CONFIG_USER_NFBI_SLAVE
	$(MAKE) -C mdio
endif #CONFIG_USER_NFBI_SLAVE


romfs:
ifdef CONFIG_USER_NFBI_MASTER
	$(MAKE) -C nfbi romfs
endif #CONFIG_USER_NFBI_MASTER
ifdef CONFIG_USER_NFBI
	$(MAKE) -C ucd romfs
endif #CONFIG_USER_NFBI
ifdef CONFIG_USER_NFBI_SLAVE
	$(MAKE) -C mdio romfs
endif #CONFIG_USER_NFBI_SLAVE
	
.PHONY: rcX
rcX:
ifdef CONFIG_USER_NFBI_SLAVE
	$(ROMFSINST) -a "/bin/ucd&" $(RC_LEVEL)
endif	
	
clean:

	$(MAKE) -C nfbi clean
	$(MAKE) -C ucd clean
	$(MAKE) -C mdio clean

