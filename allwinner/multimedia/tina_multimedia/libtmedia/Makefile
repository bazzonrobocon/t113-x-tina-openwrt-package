include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_VERSION:=0.1
PKG_RELEASE:=1
PKG_NAME:=libtmedia
PKG_BUILD_PARALLEL := 1
PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
SRC_CODE_DIR := $(LICHEE_PLATFORM_DIR)/allwinner/multimedia
PKG_FILE_DEPENDS += $(SRC_CODE_DIR)

include $(INCLUDE_DIR)/package.mk

#init the default value

KERNEL_VERSION_ION = CONF_KERNEL_CMA
COMPILE_CEDARX_LEVEL =
CONF_ONLY_DISABLE_AUDIO =
TARGET_CFLAGS += -I$(STAGING_DIR)/usr/include/allwinner/include -I$(STAGING_DIR)/usr/include/allwinner 
-include ../machinfo/$(LICHEE_IC)/config.mk
ifeq ($(USE_MPP_LIBCEDARC_DIR),y)
    LIBCEDARC_DIR = libtmedia/mpp_libcedarc
    NEW_ENC_API = 1
else
    LIBCEDARC_DIR = libcedarc
    NEW_ENC_API = 0
endif
TINA_LBC_SUPPORT = 1

TPLAYER_MULTIMEDIA_LIBS := SUPPORT_TPLAYER_COMMON \
				   SUPPORT_TRECORDER_COMMON \
				   SUPPORT_TMETADATA \
				   SUPPORT_JPEG_DECODER 



SUPPORT_TPLAYER_COMMON := $(if $(CONFIG_TPLAYER),libtplayer.so)


SUPPORT_TRECORDER_COMMON := $(if $(CONFIG_TRECORDER),libtrecorder.so)

## TMETA related libraries
SUPPORT_TMETADATA := $(if $(CONFIG_TMETADATARETRIVER),libtmetadataretriever.so)

## TJPEGDECODER related libraries
SUPPORT_JPEG_DECODER := $(if $(CONFIG_JPEGDECODER),libjpegdecode.so)

TPLAYER_SUPPORT_LIBRARY_ALL := $(foreach c,$(TPLAYER_MULTIMEDIA_LIBS), $($c))

#config the cma type
ifeq ($(CONFIG_SUNXI_ALLOC_CMA),y)
    KERNEL_VERSION_ION = CONF_KERNEL_CMA
endif
ifeq ($(CONFIG_SUNXI_ALLOC_IOMMU),y)
    KERNEL_VERSION_ION = CONF_KERNEL_IOMMU
endif

ifeq ($(MEDIA_TARGET_BOARD_PLATFORM), y)

define Package/$(PKG_NAME)/config
source "$(SOURCE)/../machinfo/$(LICHEE_IC)/$(PKG_NAME)/Config.in"
endef

define Package/$(PKG_NAME)
	SUBMENU:=Multimedia
	SECTION:=utils
	CATEGORY:=Allwinner
	TITLE:=$(PKG_NAME)  for tplayer and trecoder
	DEPENDS:=+libcedarx \
	   +TR_YES_USE_VIN_ISP:libAWIspApi \
	   +TR_USE_GLES_KC:libgpu +TR_USE_GLES_KC:libkeystone
endef

define Package/$(PKG_NAME)/Default
  TITLE:=$(PKG_NAME) for all
  URL:=http://www.allwinner.com/
endef

define Package/$(PKG_NAME)/description
	$(PKG_NAME)
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) $(SRC_CODE_DIR)/libtmedia/tina_multimedia_version.h $(PKG_BUILD_DIR)/
	$(CP) -r $(SRC_CODE_DIR)/libtmedia/tplayer $(PKG_BUILD_DIR)/tplayer
	$(CP) -r $(SRC_CODE_DIR)/libtmedia/trecorder $(PKG_BUILD_DIR)/trecorder
	$(CP) -r $(SRC_CODE_DIR)/libtmedia/jpegdecode $(PKG_BUILD_DIR)/jpegdecode
	$(CP) -r $(SRC_CODE_DIR)/libtmedia/tmetadataretriever $(PKG_BUILD_DIR)/tmetadataretriever
endef

define Build/Configure

endef

define Build/Compile

	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/tplayer \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CPP="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS) -D$(KERNEL_VERSION_ION) -DTINA_LINUX_SUPPORT=$(TINA_LBC_SUPPORT)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		BOARD_PLATFORM="$(MEDIA_TARGET_BOARD_PLATFORM)" \
		CPU_TYPE="$(TARGET_CPU_VARIANT)" \
		C_LIB_TYPE="$(CONFIG_LIBC)" \
		KERNEL64_USER32="$(CONFIG_COMPLILE_KERNEL64_USER32)" \
		CONFIG_TOOLCHAIN_GCC_VERSION=$(CONFIG_TOOLCHAIN_GCC_VERSION) \
		CONFIG_ARCH="$(CONFIG_ARCH)" \
		TPLAYER_LIBCEDARC_DIR=$(LIBCEDARC_DIR) \
		ONLY_ENABLE_AUDIO="$(CONFIG_ONLY_ENABLE_AUDIO)" \
		ONLY_DISABLE_AUDIO="$(CONFIG_ONLY_DISABLE_AUDIO)"

	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/trecorder \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CPP="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS) -DMPP_ENC_API=$(NEW_ENC_API)" \
		LDFLAGS="$(TARGET_LDFLAGS)"	\
		BOARD_PLATFORM="$(MEDIA_TARGET_BOARD_PLATFORM)" \
		USE_VIN_ISP="$(CONFIG_TR_YES_USE_VIN_ISP)" \
		USE_GLES_KC="$(CONFIG_TR_USE_GLES_KC)" \
		DEFAULT_SOUNDCARD=$(CONFIG_DEFAULT_SOUNDCARD) \
		TRECORDER_LIBCEDARC_DIR=$(LIBCEDARC_DIR) \
		DEFAULT_DEVICE=$(CONFIG_DEFAULT_DEVICE)

	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/jpegdecode \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CPP="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		KERNEL64_USER32="$(CONFIG_COMPLILE_KERNEL64_USER32)" \
		CPU_TYPE="$(TARGET_CPU_VARIANT)" \
		JPEG_LIBCEDARC_DIR=$(LIBCEDARC_DIR) \
		C_LIB_TYPE="$(CONFIG_LIBC)"

	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR)/tmetadataretriever \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CPP="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS)" \
		KERNEL64_USER32="$(CONFIG_COMPLILE_KERNEL64_USER32)" \
                CONFIG_TOOLCHAIN_GCC_VERSION=$(CONFIG_TOOLCHAIN_GCC_VERSION) \
		CONFIG_ARCH="$(CONFIG_ARCH)" \
		CPU_TYPE="$(TARGET_CPU_VARIANT)" \
		TMETADATA_LIBCEDARC_DIR=$(LIBCEDARC_DIR) \
		C_LIB_TYPE="$(CONFIG_LIBC)" \
		ONLY_DISABLE_AUDIO="$(CONFIG_ONLY_DISABLE_AUDIO)"
endef

define Build/InstallDev
	mkdir -p $(PKG_INSTALL_DIR)
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/lib
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/bin
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/include/allwinner

	$(CP) $(PKG_BUILD_DIR)/tplayer/libtplayer.so									$(PKG_INSTALL_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/trecorder/libtrecorder.so								$(PKG_INSTALL_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/jpegdecode/libjpegdecode.so							$(PKG_INSTALL_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/tmetadataretriever/libtmetadataretriever.so					$(PKG_INSTALL_DIR)/usr/lib/
	cd $(PKG_INSTALL_DIR)/usr/lib/

	## copy header files.
	## tplayer
	-cp $(PKG_BUILD_DIR)/tplayer/tplayer.h									$(PKG_INSTALL_DIR)/usr/include/allwinner/
	-cp $(PKG_BUILD_DIR)/tplayer/awsink/tlayer_ctrl.h									$(PKG_INSTALL_DIR)/usr/include/allwinner/
	## trecorder
	-cp $(PKG_BUILD_DIR)/trecorder/Trecorder.h									$(PKG_INSTALL_DIR)/usr/include/allwinner/
	-cp $(PKG_BUILD_DIR)/trecorder/dataqueue.h									$(PKG_INSTALL_DIR)/usr/include/allwinner/
	## jpegdecode
	-cp $(PKG_BUILD_DIR)/jpegdecode/jpegdecode.h									$(PKG_INSTALL_DIR)/usr/include/allwinner/

	## tmetadataretriever
	-cp $(PKG_BUILD_DIR)/tmetadataretriever/tmetadataretriever.h									$(PKG_INSTALL_DIR)/usr/include/allwinner/
	echo "**InstallDev path:$(1)***"
	$(CP) $(PKG_INSTALL_DIR)/usr $(1)/usr

endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/

	mkdir -p $(PKG_BUILD_DIR)/all_libs
	$(CP) $(PKG_BUILD_DIR)/tplayer/libtplayer.so $(PKG_BUILD_DIR)/all_libs
	$(CP) $(PKG_BUILD_DIR)/trecorder/libtrecorder.so $(PKG_BUILD_DIR)/all_libs
	$(CP) $(PKG_BUILD_DIR)/jpegdecode/libjpegdecode.so $(PKG_BUILD_DIR)/all_libs
	$(CP) $(PKG_BUILD_DIR)/tmetadataretriever/libtmetadataretriever.so $(PKG_BUILD_DIR)/all_libs

	$(CP) $(PKG_BUILD_DIR)/trecorder/recorder.cfg									$(1)/etc/

	echo "**install path:$(1)***"

	for n in $(TPLAYER_SUPPORT_LIBRARY_ALL); \
	do \
		$(CP) $(PKG_BUILD_DIR)/all_libs/$$$$n $(1)/usr/lib; \
	done
	cd $(1)/usr/lib/; rm -rf $(NOT_SUPPORT_VIDEO_LIBS)
endef

endif

ifeq ($(MEDIA_TARGET_BOARD_PLATFORM), y)
$(eval $(call BuildPackage,$(PKG_NAME)))
endif
