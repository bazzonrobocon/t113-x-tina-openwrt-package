include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

# Name and release number of this package
PKG_NAME := pqd
PKG_VERSION := 0.0.1
PKG_RELEASE := 1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
SRC_CODE_DIR := $(LICHEE_PLATFORM_DIR)/allwinner/display/$(PKG_NAME)
PKG_FILE_DEPENDS += $(SRC_CODE_DIR)
TARGET_CFLAGS += -std=gnu99 -Wall -Wno-unused-function -I./hardwares/de20x/ -I./ -Iutils -g -DFCM_10BIT_ENABLE
TARGET_LDFLAGS += -lpthread -lm
#TARGET_LDFLAGS += -lasan
#TARGET_CFLAGS += -fsanitize=leak -fsanitize=address -fno-omit-frame-pointer -g

include $(INCLUDE_DIR)/package.mk

define Package/$(PKG_NAME)
  SUBMENU:=Display
  CATEGORY := Allwinner
  TITLE := pqd
  DEPENDS := +libc
endef

define Package/$(PKG_NAME)/description
  tigerLCD application
endef

define Build/Prepare
	@$(INSTALL_DIR) $(PKG_BUILD_DIR)/src
	@$(CP) $(SRC_CODE_DIR)/* $(PKG_BUILD_DIR)/src
endef

define Build/Compile
	@$(MAKE) -C $(PKG_BUILD_DIR)/src \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CXX="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		BUILD_DIR="$(PKG_BUILD_DIR)" \
		PREFIX="$(PREFIX)" \
		all
endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/bin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/pqd $(1)/usr/bin
endef

$(eval $(call BuildPackage,$(PKG_NAME)))
