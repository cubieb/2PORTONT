# Copyright (c) 2010-2012 Helsinki Institute for Information Technology
# and University of Helsinki.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

include $(TOPDIR)/rules.mk

# Name and release number of this package
PKG_NAME:=y1731
PKG_VERSION:=0.0.1
PKG_RELEASE:=1

# The filename of the original sources
PKG_SOURCE:=$(PKG_NAME)-$(PKG_VERSION).tar.gz

# How to decompress the sources (zcat, bzcat, unzip)
PKG_CAT:=zcat

# This specifies the directory where we're going to build the program.  
# The root build directory, $(BUILD_DIR), is by default the build_mipsel 
# directory in your OpenWrt SDK directory
PKG_BUILD_DIR:=$(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)

PKG_INSTALL_DIR:=$(PKG_BUILD_DIR)/ipkg-install

include $(INCLUDE_DIR)/package.mk

# Specify package information for this program. 
define Package/y1731
	SECTION:=net
	CATEGORY:=Network
	TITLE:=Y.1731 daemon
endef

define Package/y1731/description
 The daemon for Y.1731
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR) $(TARGET_CONFIGURE_OPTS)
endef

define Package/y1731/install
	$(INSTALL_DIR) $(1)/usr/local/etc/y1731/
	$(INSTALL_DIR) $(1)/usr/local/sbin/

	$(INSTALL_DATA) ./files/config.xml $(1)/usr/local/etc/y1731/config.xml
	$(INSTALL_DATA) ./files/meg.xml $(1)/usr/local/etc/y1731/meg.xml

	$(INSTALL_BIN)  $(PKG_BUILD_DIR)/oamd $(1)/usr/local/sbin/
	$(INSTALL_BIN)  $(PKG_BUILD_DIR)/oamtool $(1)/usr/local/sbin/
	$(INSTALL_BIN)  $(PKG_BUILD_DIR)/oamping $(1)/usr/local/sbin/
	$(INSTALL_BIN)  $(PKG_BUILD_DIR)/ccontrold $(1)/usr/local/sbin/
endef

define Package/y1731/conffiles
/usr/local/etc/y1731/config.xml
/usr/local/etc/y1731/meg.xml
endef

$(eval $(call BuildPackage,y1731))
