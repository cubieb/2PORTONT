
.EXPORT_ALL_VARIABLES:

SAMBA_ROOT=$(shell pwd)
SRC_DIRS = source
RC_LEVEL=/etc/init.d/rc6

all:
	$(MAKE) -C $(SRC_DIRS)

romfs:
	$(ROMFSINST) source/bin/smbd /bin/smbd
	$(ROMFSINST) -e CONFIG_USER_NMBD source/bin/nmbd /bin/nmbd
	$(ROMFSINST) -e CONFIG_USER_SMBPASSWD source/bin/smbpasswd /bin/smbpasswd
	$(ROMFSINST) -s /var/samba /etc/samba
ifndef CONFIG_MULTI_SMBD_ACCOUNT
	$(ROMFSINST) /etc/smb.conf
	$(ROMFSINST) -e CONFIG_ARCH_LUNA_SLAVE smb_wlan1.conf /etc/smb.conf
else
	$(ROMFSINST) -s /var/smb.conf /etc/smb.conf
	$(ROMFSINST) -s /var/tmp/smbpasswd /etc/samba/smbpasswd
endif
	$(ROMFSINST) -e CONFIG_ARCH_LUNA_SLAVE smb_usr.sh /etc/smb_usr.sh

.PHONY: rcX
rcX:
	$(ROMFSINST) -s /var/group /etc/group
	$(ROMFSINST) -a "mkdir -p /var/samba/" $(RC_LEVEL)
ifdef CONFIG_ARCH_LUNA_SLAVE 
	$(ROMFSINST) -a "/bin/sh /etc/smb_usr.sh" $(RC_LEVEL)  
endif

clean:
	$(MAKE) -C $(SRC_DIRS) clean

