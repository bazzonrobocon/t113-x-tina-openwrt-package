include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=uevent-monitor-for-rproc_wdt
PKG_VERSION:=1
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

# SRC_CODE_DIR := $(LICHEE_CBBPKG_DIR)/allwinner/utils/uevent-monitor/src

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)/Default
  SUBMENU:=Other
  SECTION:=utils
  CATEGORY:=Allwinner
endef

define Package/$(PKG_NAME)
  $(call Package/$(PKG_NAME)/Default)
  TITLE:=tina uevent monitor for rproc_wdt
endef

define Package/$(PKG_NAME)/description
  tina uevent monitor demo for rproc_wdt, contains function:
listen uevent
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)/src/
	$(CP) -r ./src/* $(PKG_BUILD_DIR)/src/
endef

define Build/Configure
endef

$(warning "KERNEL: $(KERNEL_PATCHVER)")
ifneq ($(filter 4.4 4.9, $(KERNEL_PATCHVER)), )
  LOCAL_CFLAGS=-DGADGET_CONFIGFS
endif

define Build/Compile
	#compile demo
	$(MAKE) -C $(PKG_BUILD_DIR)/src \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS) $(LOCAL_CFLAGS)"\
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all \
		V=1
endef

define Build/InstallDev
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/* $(1)/usr/bin/
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
