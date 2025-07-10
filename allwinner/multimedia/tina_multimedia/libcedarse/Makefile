#
# Copyright (C) 2012 OpenWrt.org
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=libcedarse
PKG_VERSION:=2.9.4
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
include $(INCLUDE_DIR)/package.mk

PKG_LIBCEDARSE_CBB_SRC_DIR:=$(LICHEE_CBBPKG_DIR)/allwinner/multimedia/libcedarse
PKG_FILE_DEPENDS += $(PKG_LIBCEDARSE_CBB_SRC_DIR)

define Package/libcedarse
  CATEGORY:=Allwinner
  SECTION:=allwinner
  SUBMENU:=Audio
  TITLE:= cedarse library
  DEPENDS:= +librpbuf
endef

define Package/cedarSE_parm_adj
  CATEGORY:=Allwinner
  SECTION:=allwinner
  SUBMENU:=Audio
  TITLE:= cedarSE parm adjust
endef

define Package/cedarSE_parm_adj/description
  cedarSE parm adjust
endef

define Package/cedarSE_demo
  CATEGORY:=Allwinner
  SECTION:=allwinner
  SUBMENU:=Audio
  TITLE:= cedarSE demo
  DEPENDS:= +librpbuf +libcedarse
endef

define Package/cedarSE_demo/description
  cedarSE demo
endef

define Build/Prepare
	$(CP) -r $(LICHEE_CBBPKG_DIR)/allwinner/multimedia/libcedarse/lib $(PKG_BUILD_DIR)/
	$(CP) -r $(LICHEE_CBBPKG_DIR)/allwinner/multimedia/libcedarse/config $(PKG_BUILD_DIR)/
	$(CP) -r $(LICHEE_CBBPKG_DIR)/allwinner/multimedia/libcedarse/include $(PKG_BUILD_DIR)/
	$(CP) -r $(LICHEE_CBBPKG_DIR)/allwinner/multimedia/libcedarse/adjust $(PKG_BUILD_DIR)/
	$(CP) -r $(LICHEE_CBBPKG_DIR)/allwinner/multimedia/libcedarse/demo $(PKG_BUILD_DIR)/
endef

TARGET_CFLAGS+=-Werror

define Build/Compile/libcedarse
	$(MAKE) -C $(PKG_BUILD_DIR)/lib \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
	$(MAKE) -C $(PKG_BUILD_DIR)/config \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/Compile/cedarSE_parm_adj
	$(MAKE) -C $(PKG_BUILD_DIR)/adjust \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/Compile/cedarSE_demo
	$(MAKE) -C $(PKG_BUILD_DIR)/demo \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Build/Compile
	$(Build/Compile/libcedarse)
ifeq ($(CONFIG_PACKAGE_cedarSE_parm_adj),y)
	$(Build/Compile/cedarSE_parm_adj)
endif
ifeq ($(CONFIG_PACKAGE_cedarSE_demo),y)
	$(Build/Compile/cedarSE_demo)
endif
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/*.so $(1)/usr/lib/
endef

define Package/libcedarse/install
	mkdir -p  $(1)/etc/cedarSE
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/lib/libcedarse.so $(1)/usr/lib/
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/config/cedarSE/* $(1)/etc/cedarSE/
endef

define Package/cedarSE_parm_adj/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/cedarSE_parm_adj $(1)/usr/bin/
endef

define Package/cedarSE_demo/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/cedarSE_demo $(1)/usr/bin/
endef

$(eval $(call BuildPackage,libcedarse))
$(eval $(call BuildPackage,cedarSE_parm_adj))
$(eval $(call BuildPackage,cedarSE_demo))
