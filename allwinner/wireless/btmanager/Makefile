include $(TOPDIR)/rules.mk

PKG_NAME:=btmanager-v4.0
PKG_VERSION:=4.0.0
PKG_RELEASE:=1
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

SRC_CODE_DIR := $(LICHEE_CBBPKG_DIR)/allwinner/wireless/btmanager

PKG_CONFIG_DEPENDS:= \
	CONFIG_USES_BCMDHD \
	CONFIG_USES_XR829 \
	CONFIG_USES_XR819S \
	CONFIG_USES_REALTEK \
	CONFIG_USES_ESP8089

PKG_FILE_DEPENDS += $(SRC_CODE_DIR)

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)/Default
  SECTION:=utils
  CATEGORY:=Allwinner
  SUBMENU:=Wireless
endef

define Package/$(PKG_NAME)
  $(call Package/$(PKG_NAME)/Default)
  TITLE:=bluetooth manager core
  DEPENDS:=+alsa-lib +bluez-daemon +glib2 +sbc +dbus +bluez-alsa +libjson-c +ell +wirelesscommon
  KCONFIG:=\
	  CONFIG_HFP_OVER_PCM=y
endef

define Package/$(PKG_NAME)/config
config BTMG_ENABLE_DEMO
	bool "Enable btmanager demo support"
	depends on PACKAGE_btmanager-v4.0
	default y

config BTMG_ENABLE_MESH
	bool "Enable xradio mesh support"
	depends on PACKAGE_btmanager-v4.0
	default n

choice
	prompt "select serial for Bluetooth"
	default ttyS1
config BTMG_ENABLE_TTYS1
	bool "ttyS1"
config BTMG_ENABLE_TTYAS1
	bool "ttyAS1"
config BTMG_ENABLE_TTYS2
	bool "ttyS2"
config BTMG_ENABLE_TTYAS2
	bool "ttyAS2"
endchoice
endef

define Package/$(PKG_NAME)/description
	bluetooth manager
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) -r $(SRC_CODE_DIR)/src $(PKG_BUILD_DIR)/
	$(CP) -r $(SRC_CODE_DIR)/demo $(PKG_BUILD_DIR)/
endef

TARGET_LDFLAGS += $(if $(CONFIG_USE_GLIBC),-lpthread -lz -lrt -lm -ldl)

PKG_CONFIG_DEPENDS += CONFIG_BTMG_ENABLE_MESH
PKG_CONFIG_DEPENDS += CONFIG_BTMG_ENABLE_DEMO
PKG_CONFIG_DEPENDS += CONFIG_BTMG_ENABLE_TTYS1
PKG_CONFIG_DEPENDS += CONFIG_BTMG_ENABLE_TTYAS1
PKG_CONFIG_DEPENDS += CONFIG_BTMG_ENABLE_TTYS2
PKG_CONFIG_DEPENDS += CONFIG_BTMG_ENABLE_TTYAS2

ifeq ($(CONFIG_TOOLCHAIN_GCC_VERSION), "8.3.0")
CONFIG_LIBC="glibc-gcc8_3_0"
else ifeq ($(CONFIG_TOOLCHAIN_GCC_VERSION), "10.2.0")
CONFIG_LIBC="glibc-gcc10_2_0"
else ifeq ($(CONFIG_TOOLCHAIN_GCC_VERSION), "11.3.0")
CONFIG_LIBC="glibc-gcc11_3_0"
endif

define Build/Configure
endef
ifeq ($(CONFIG_PACKAGE_$(PKG_NAME)),y)
define Build/Compile/$(PKG_NAME)
	$(MAKE) -C $(PKG_BUILD_DIR)/src/ \
		PKG_BUILD_DIR="$(PKG_BUILD_DIR)" \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		BT_EXT_CFLAG="$(BT_EXT_CFLAG)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		STAGING_DIR="$(STAGING_DIR)" \
		CONFIG_LIBC="$(CONFIG_LIBC)$(if $(CONFIG_EXT_TOOLCHAIN_GCC_8_3_0),-gcc8_3_0)" \
		CPU="$(ARCH)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
ifeq ($(CONFIG_BTMG_ENABLE_DEMO), y)
	$(MAKE) -C $(PKG_BUILD_DIR)/demo/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		STAGING_DIR="$(STAGING_DIR)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endif
ifeq ($(CONFIG_BTMG_ENABLE_MESH), y)
	$(MAKE) -C $(PKG_BUILD_DIR)/src/mesh \
		PKG_BUILD_DIR="$(PKG_BUILD_DIR)" \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		BT_EXT_CFLAG="$(BT_EXT_CFLAG)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		STAGING_DIR="$(STAGING_DIR)" \
		CONFIG_LIBC="$(CONFIG_LIBC)$(if $(CONFIG_EXT_TOOLCHAIN_GCC_8_3_0),-gcc8_3_0)" \
		CPU="$(ARCH)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
	$(MAKE) -C $(PKG_BUILD_DIR)/demo/mesh/ \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		CONFIG_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endif
endef
endif

define Build/Compile
	$(Build/Compile/$(PKG_NAME))
endef

define Build/InstallDev
	$(INSTALL_DIR) $(1)/usr/include
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/include/*.h $(1)/usr/include
	$(INSTALL_DIR) $(1)/usr/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/usr/lib
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/lib
	$(INSTALL_DATA) $(PKG_INSTALL_DIR)/usr/lib/* $(1)/lib
	$(INSTALL_DIR) $(1)/etc/bluetooth
	cp $(SRC_CODE_DIR)/config/bt_init.sh $(SRC_CODE_DIR)/config/bt_init.sh.tmp
ifeq ($(CONFIG_USES_XR819S), y)
	sed -i 's/xradio/xr819s/g' $(SRC_CODE_DIR)/config/bt_init.sh.tmp
	sed -i 's/# xr819s_stop/xr819s_stop/g' $(SRC_CODE_DIR)/config/bt_init.sh.tmp
else ifeq ($(CONFIG_USES_REALTEK), y)
	sed -i 's/bt_hciattach="hciattach"/bt_hciattach="rtk_hciattach"/g' $(SRC_CODE_DIR)/config/bt_init.sh.tmp
	sed -i 's/# reset_bluetooth_power/reset_bluetooth_power/g' $(SRC_CODE_DIR)/config/bt_init.sh.tmp
	sed -i 's/ttyS1 xradio/-s 115200 \/dev\/ttyS1 rtk_h5/g' $(SRC_CODE_DIR)/config/bt_init.sh.tmp
else ifeq ($(CONFIG_PACKAGE_aic8800-firmware), y)
	sed -i '/# reset_bluetooth_power/i\	echo 1 \> \/proc\/bluetooth\/sleep\/btwrite' $(SRC_CODE_DIR)/config/bt_init.sh.tmp
	sed -i 's/# reset_bluetooth_power/reset_bluetooth_power/g' $(SRC_CODE_DIR)/config/bt_init.sh.tmp
	sed -i 's/xradio/aic/g' $(SRC_CODE_DIR)/config/bt_init.sh.tmp
endif

ifeq ($(CONFIG_BTMG_ENABLE_TTYAS1), y)
	sed -i 's/ttyS1/ttyAS1/g' $(SRC_CODE_DIR)/config/bt_init.sh.tmp
else ifeq ($(CONFIG_BTMG_ENABLE_TTYS2), y)
	sed -i 's/ttyS1/ttyS2/g' $(SRC_CODE_DIR)/config/bt_init.sh.tmp
else ifeq ($(CONFIG_BTMG_ENABLE_TTYAS2), y)
	sed -i 's/ttyS1/ttyAS2/g' $(SRC_CODE_DIR)/config/bt_init.sh.tmp
endif

	$(INSTALL_BIN) $(SRC_CODE_DIR)/config/bt_init.sh.tmp $(1)/etc/bluetooth/bt_init.sh
	rm -rf $(SRC_CODE_DIR)/config/bt_init.sh.tmp
	$(INSTALL_DATA) $(SRC_CODE_DIR)/config/bluetooth.json $(1)/etc/bluetooth/bluetooth.json

ifeq ($(CONFIG_BTMG_ENABLE_DEMO), y)
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/bt_test $(1)/usr/bin
endif

ifeq ($(CONFIG_BTMG_ENABLE_MESH), y)
	$(INSTALL_BIN) $(PKG_INSTALL_DIR)/usr/bin/bt_mesh_test $(1)/usr/bin
endif
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
