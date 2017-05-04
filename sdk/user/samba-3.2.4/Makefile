
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
	$(ROMFSINST) /etc/smb.conf

.PHONY: rcX
rcX:
	$(ROMFSINST) -s /var/group /etc/group
	$(ROMFSINST) -a "mkdir -p /var/samba/" $(RC_LEVEL)
	
clean:
	$(MAKE) -C $(SRC_DIRS) clean

