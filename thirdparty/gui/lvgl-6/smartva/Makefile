include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk
include $(INCLUDE_DIR)/package.mk

PKG_NAME:=smartva
PKG_VERSION:=1.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

PKG_BUILD_DEPENDS:=PACKAGE_tslib:tslib
TARGET_PRODUCT = ${LICHEE_IC}_${LICHEE_BOARD}
define Package/$(PKG_NAME)
  SECTION:=gui
  SUBMENU:=Littlevgl-6
  CATEGORY:=Gui
  DEPENDS:= +libtmedia +alsa-utils +wifimanager-v2.0 +wifimanager-v2.0-lib +libspeexdsp +libfreetype +swupdate +libcurl +blkid +PACKAGE_tslib:tslib +zlib +libjpeg +libpng
  TITLE:=smartva for pdc-pd1 use lvgl-6.1.2
  MAINTAINER:=huangyixiu <huangyixiu@allwinnertech.com>
endef

define Package/$(PKG_NAME)/Default
endef

define Package/$(PKG_NAME)/description
  smartva for pdc-pd1
endef

define Package/$(PKG_NAME)/config
  source "$(SOURCE)/Config.in"
endef

ifeq ($(CONFIG_lvgl),y)
CONFIGURE_PRJ += ./../lvgl
endif

ifeq ($(CONFIG_lv_drivers),y)
CONFIGURE_PRJ += ./../lv_drivers
endif

ifeq ($(CONFIG_lv_examples),y)
CONFIGURE_PRJ += ./../lv_examples
endif

CONFIGURE_PRJ += ./src/common

#ifeq ($(CONFIG_lv_story),y)
CONFIGURE_PRJ += ./src/story
CONFIGURE_APP += ./src/story
CONFIGURE_BIN += story_bin
#endif

-include machinfo/$(LICHEE_IC)/config.mk

LOCAL_CFLAGS += -I$(LICHEE_KERN_DIR)/include/video
ifeq ($(CONFIG_RTP_USE),y)
TARGET_LDFLAGS += -lts
LOCAL_CFLAGS += -D__RTP_USE__
else ifeq ($(CONFIG_CTP_USE), y)
LOCAL_CFLAGS += -D__CTP_USE__
endif

LOCAL_CFLAGS+=-I$(STAGING_DIR)/usr/include/freetype2
LOCAL_CFLAGS+=-I$(STAGING_DIR)/usr/include/libpng16

TARGET_LDFLAGS+=-L$(STAGING_DIR)/usr/lib -L$(STAGING_DIR)/lib

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)/src
	$(CP) -u $(CONFIGURE_PRJ)/ $(PKG_BUILD_DIR)/src/
	$(CP) -u $(CONFIGURE_APP)/config/* $(PKG_BUILD_DIR)/src/
	$(CP) -u $(CONFIGURE_APP)/Makefile $(PKG_BUILD_DIR)/src/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src\
		ARCH="$(CONFIG_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		LOCAL_CFLAGS="$(LOCAL_CFLAGS)" \
		INSTALL_PREFIX="$(PKG_INSTALL_DIR)" \
		all
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/usr/res/
	$(INSTALL_DIR) $(1)/usr/res/image/
	$(INSTALL_DIR) $(1)/usr/res/font/

	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/$(CONFIGURE_BIN) $(1)/usr/bin/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/story/designer/image/res/* $(1)/usr/res/image/
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/story/designer/font/res/* $(1)/usr/res/font//
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
