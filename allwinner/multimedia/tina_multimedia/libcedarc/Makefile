include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=libcedarc
PKG_VERSION:=1.0.0
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)
SRC_CODE_DIR := $(LICHEE_PLATFORM_DIR)/allwinner/multimedia
PKG_FILE_DEPENDS += $(SRC_CODE_DIR)
PKG_BUILD_PARALLEL := 1
include $(INCLUDE_DIR)/package.mk
KERNEL_VERSION = CONF_KERNEL_VERSION_3_10
CEDARC_COMPILE_TOOL_CHAIN = arm-openwrt-linux
ifeq ($(KERNEL_PATCHVER),5.15)
CEDARC_LIBS_PATH = library/toolchain-sunxi-aarch64-glibc-gcc-v1020
else
CEDARC_LIBS_PATH = library/toolchain-sunxi-aarch64-glibc-gcc-v830
endif
KERNEL_VERSION_ION = CONF_KERNEL_CMA
CEDARC_LD_FLAGS = -ldl
CEDARC_LD_FLAGS += $(if $(CONFIG_USE_GLIBC),-lpthread)
CEDARC_EXTRA_CXXFLAGS =

-include ../machinfo/$(LICHEE_IC)/config.mk
ifeq ($(USE_MPP_LIBCEDARC_DIR),y)
    LIBCEDARC_DIR = mpp_libcedarc
    NEW_ENC_API = 1
else ifeq ($(ENABLE_CEDARC_VERSION_2_0_0),y)
    LIBCEDARC_DIR = libcedarc_v2.0
    NEW_ENC_API = 0
else
    LIBCEDARC_DIR = libcedarc
    NEW_ENC_API = 0
endif

ifeq ($(DISABLE_ISP_VENC_FLAG),y)
    ISP_VENC_FLAG = DISABLE_ENC_ISP
else
    ISP_VENC_FLAG = ENABLE_ENC_ISP
endif

ifeq ($(DISABLE_DEC_ATM_FLAG),y)
    DEC_ATM_FLAG = DISABLE_DEC_ATM
else
    DEC_ATM_FLAG = ENABLE_DEC_ATM
endif

ifeq ($(CONFIG_CEDARC_ENABLE_AFTERTREATMENT),y)
	CONFIG_AFTERTREATMENT += --enable-aftertreatment=yes
else
	CONFIG_AFTERTREATMENT += --enable-aftertreatment=no
endif

ifeq ($(CONFIG_CEDARC_ENABLE_VDECODERDEMO),y)
	CONFIG_VDECODERDEMO += --enable-vdecoderdemo=yes
else
	CONFIG_VDECODERDEMO += --enable-vdecoderdemo=no
endif

ifeq ($(CONFIG_CEDARC_ENABLE_VENCODERDEMO),y)
	CONFIG_VENCODERDEMO += --enable-vencoderdemo=yes
else
	CONFIG_VENCODERDEMO += --enable-vencoderdemo=no
endif

ifeq ($(CONFIG_CEDARC_VENC_DEFAULT_LIB),y)
	CONFIG_VENC_DEFAULT_LIB += --enable-venc-default-lib=yes
else
	CONFIG_VENC_DEFAULT_LIB += --enable-venc-default-lib=no
endif

ifeq ($(CONFIG_CEDARC_VENC_V_LINE_LIB),y)
	CONFIG_VENC_V_LINE_LIB += --enable-venc-v-line-lib=yes
else
	CONFIG_VENC_V_LINE_LIB += --enable-venc-v-line-lib=no
endif

TINA_LINUX_API = 1
VIDEO_DEC_LIBS := MPEG2 \
			 H264 \
			 AVS \
			 H265 \
			 MJPEG \
			 MPEG4BASE \
			 MPEG4DX \
			 MPEG4H263 \
			 MPEG4NORMAL \
			 MPEG4VP6 \
			 VP8 \
			 VP9HW \
			 VP6SOFT

LIBCEDARC_LIBS := SUPPORT_VIDEO_DEC_LIBS \
				   SUPPORT_VDEC_COMMON_LIBS \
				   SUPPORT_VIDEO_ENC_LIBS \
				   SUPPORT_VENC_COMMON_LIBS \
				   SUPPORT_OMX_LIBS
			

SUPPORT_VIDEO_DEC_LIBS = $(foreach c,$(VIDEO_DEC_LIBS), \
				$(if $(CONFIG_CEDAR_VDECODER_$(c)),libaw$(shell echo $(c) | tr '[A-Z]' '[a-z]').so))

ifneq ($(USE_MPP_LIBCEDARC_DIR),y)
SUPPORT_VIDEO_DEC_LIBS += $(if $(CONFIG_CEDAR_VDECODER_MJPEG),libawmjpegplus.so)
endif

SUPPORT_VIDEO_DEC_LIBS += $(if $(CONFIG_CEDAR_VDECODER_VP9),libawvp9HwAL.so)
ifeq ($(USE_MPP_LIBCEDARC_DIR),y)
SUPPORT_VDEC_COMMON_LIBS := $(if $(CONFIG_CEDAR_VIDEO_DECODER_COMMON_ALL),libcdc_base.so libMemAdapter.so libsbm.so libvdecoder.so libVE.so libvideoengine.so libvenc_codec.so)
else ifeq ($(ENABLE_V_LINE_ENC_LIB),y)
SUPPORT_VDEC_COMMON_LIBS := $(if $(CONFIG_CEDAR_VIDEO_DECODER_COMMON_ALL),libcdc_base.so libMemAdapter.so libfbm.so libsbm.so libvdecoder.so libVE.so libvideoengine.so)
else
SUPPORT_VDEC_COMMON_LIBS := $(if $(CONFIG_CEDAR_VIDEO_DECODER_COMMON_ALL),libcdc_base.so libMemAdapter.so libaftertreatment.so libfbm.so libsbm.so libscaledown.so libvdecoder.so libVE.so libvideoengine.so)
endif

ifeq ($(USE_MPP_LIBCEDARC_DIR),y)
SUPPORT_VIDEO_ENC_LIBS := $(if $(CONFIG_CEDAR_VIDEO_ENCODER_ALL),libvencoder.so libvenc_base.so)
else ifeq ($(ENABLE_V_LINE_ENC_LIB),y)
SUPPORT_VIDEO_ENC_LIBS := $(if $(CONFIG_CEDAR_VIDEO_ENCODER_ALL),libvencoder.so libvenc_base.so libvenc_codec.so)
else
SUPPORT_VIDEO_ENC_LIBS := $(if $(CONFIG_CEDAR_VIDEO_ENCODER_ALL),libvencoder.so libvenc_base.so libvenc_common.so libvenc_h264.so libvenc_h265.so libvenc_jpeg.so)
endif

SUPPORT_VENC_COMMON_LIBS := $(if $(CONFIG_CEDAR_VIDEO_ENCODER_COMMON_ALL), libcdc_base.so libMemAdapter.so libVE.so)

SUPPORT_OMX_LIBS := $(if $(CONFIG_OMX),libOmxCore.so libOmxVdec.so libOmxVenc.so)

SUPPORT_LIBCEDARC_LIBRARY_ALL := $(foreach c,$(LIBCEDARC_LIBS), $($c))
#config the kernel version,which depends on $(KERNEL_PATCHVER)
ifeq ($(KERNEL_PATCHVER),3.4)
    KERNEL_VERSION = CONF_KERNEL_VERSION_3_4
endif

ifeq ($(KERNEL_PATCHVER),3.10)
    KERNEL_VERSION = CONF_KERNEL_VERSION_3_10
endif

ifeq ($(KERNEL_PATCHVER),4.4)
    KERNEL_VERSION = CONF_KERNEL_VERSION_4_4
endif

ifeq ($(KERNEL_PATCHVER),4.9)
    KERNEL_VERSION = CONF_KERNEL_VERSION_4_9
endif

ifeq ($(KERNEL_PATCHVER),5.4)
    KERNEL_VERSION = CONF_KERNEL_VERSION_5_4
endif

ifeq ($(KERNEL_PATCHVER),5.15)
    KERNEL_VERSION = CONF_KERNEL_VERSION_5_15
endif

#config the toolchain,which depends on arm or arm64 or riscv
#notice:arm musl,arm glibc,arm9 musl,arm9 glibc all use the following toolchain prefix
ifeq ($(CONFIG_ARCH),"arm")
    CEDARC_COMPILE_TOOL_CHAIN = arm-openwrt-linux
endif
#notice:aarch64 musl,aarch64 glibc all use the following toolchain prefix
ifeq ($(CONFIG_ARCH),"aarch64")
    CEDARC_COMPILE_TOOL_CHAIN = aarch64-openwrt-linux
endif
#notice:riscv glibc all use the following toolchain prefix
ifeq ($(CONFIG_ARCH),"riscv")
    CEDARC_COMPILE_TOOL_CHAIN = riscv64-unknown-linux-gnu
endif
#end config the toolchain

#config the cma type
ifeq ($(CONFIG_SUNXI_ALLOC_CMA),y)
    KERNEL_VERSION_ION = CONF_KERNEL_CMA
endif
ifeq ($(CONFIG_SUNXI_ALLOC_IOMMU),y)
    KERNEL_VERSION_ION = CONF_KERNEL_IOMMU
endif

#config the arm libs path
ifeq ($(TARGET_CPU_VARIANT),cortex-a7)
    ifeq ($(CONFIG_LIBC),"musl")
        CEDARC_LIBS_PATH = library/toolchain-sunxi-arm-musl
	ifeq ($(CONFIG_TOOLCHAIN_GCC_VERSION),"8.4.0")
	ifneq ($(USE_MPP_LIBCEDARC_DIR),y)
        CEDARC_LIBS_PATH = library/toolchain-sunxi-musl-gcc-v830
	endif
        endif
        COMPILE_CEDARX_LEVEL = -O0
    endif
    ifeq ($(CONFIG_LIBC),"glibc")
        CEDARC_LIBS_PATH = library/toolchain-sunxi-glibc-gcc-v830
    endif
endif

#config the aarch64 libs path
ifeq ($(TARGET_CPU_VARIANT),cortex-a53)
    ifeq ($(CONFIG_COMPLILE_KERNEL64_USER32),y)
        CEDARC_COMPILE_TOOL_CHAIN = arm-openwrt-linux
        ifeq ($(CONFIG_LIBC),"musl")
            CEDARC_LIBS_PATH = library/toolchain-sunxi-arm-musl
        endif
        ifeq ($(CONFIG_LIBC),"glibc")
	    CEDARC_LIBS_PATH = library/toolchain-sunxi-glibc-gcc-v830
        endif
    else
        ifeq ($(CONFIG_LIBC),"musl")
            CEDARC_LIBS_PATH = library/toolchain-sunxi-aarch64-musl
	    ifeq ($(CONFIG_TOOLCHAIN_GCC_VERSION),"8.4.0")
                CEDARC_LIBS_PATH = library/toolchain-sunxi-aarch64-musl-gcc-v830
            endif
            COMPILE_CEDARX_LEVEL = -O0
        endif
        ifeq ($(CONFIG_LIBC),"glibc")
             CEDARC_LIBS_PATH = library/toolchain-sunxi-aarch64-glibc
	     ifeq ($(CONFIG_TOOLCHAIN_GCC_VERSION),"8.4.0")
             CEDARC_LIBS_PATH = library/toolchain-sunxi-aarch64-glibc-gcc-v830
     endif
        endif
    endif
endif

#config the arm9 libs path
ifeq ($(TARGET_CPU_VARIANT),arm926ej-s)
    ifeq ($(CONFIG_LIBC),"musl")
	CEDARC_LIBS_PATH = library/toolchain-sunxi-arm9-musl
    endif
    ifeq ($(CONFIG_LIBC),"glibc")
        CEDARC_LIBS_PATH = library/toolchain-sunxi-arm9-glibc
    endif
endif

#config the riscv libs path
ifeq ($(TARGET_CPU_VARIANT),c910)
    ifeq ($(CONFIG_LIBC),"glibc")
        CEDARC_LIBS_PATH = library/toolchain-sunxi-riscv-glibc
    endif
endif

#config the aarch64 libs path
ifeq ($(CONFIG_ARCH),"aarch64")
	ifeq ($(CONFIG_LIBC),"glibc")
		ifeq ($(CONFIG_TOOLCHAIN_GCC_VERSION),"11.3.0")
			CEDARC_LIBS_PATH = library/toolchain-sunxi-aarch64-glibc-gcc-v1130
		endif
		ifeq ($(CONFIG_TOOLCHAIN_GCC_VERSION),"10.2.0")
			CEDARC_LIBS_PATH = library/toolchain-sunxi-aarch64-glibc-gcc-v1020
		endif
		ifeq ($(CONFIG_TOOLCHAIN_GCC_VERSION),"13.2.0")
			CEDARC_LIBS_PATH = library/toolchain-sunxi-aarch64-glibc-gcc-v1320
		endif

	endif
endif
ifeq ($(MEDIA_TARGET_BOARD_PLATFORM), y)

define Package/$(PKG_NAME)/config
source "$(SOURCE)/../machinfo/$(LICHEE_IC)/$(PKG_NAME)/Config.in"
endef

define Package/$(PKG_NAME)
  SUBMENU:=Multimedia
  SECTION:=utils
  CATEGORY:=Allwinner
  TITLE:=$(PKG_NAME) for Allwinner
  DEPENDS:=+libstdcpp
endef

PKG_CONFIG_DEPENDS += CONFIG_CEDAR_VIDEO_DECODER_COMMON_ALL \
                      CONFIG_CEDAR_VIDEO_ENCODER_ALL

define Package/$(PKG_NAME)/Default
  TITLE:=$(PKG_NAME) for all
  URL:=http://www.allwinner.com/
endef

define Package/$(PKG_NAME)/description
	$(PKG_NAME)
endef

define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
ifeq ($(USE_MPP_LIBCEDARC_DIR),y)
	$(CP) -Lvr $(SRC_CODE_DIR)/libtmedia/$(LIBCEDARC_DIR)/* $(PKG_BUILD_DIR)/
else
	$(CP) -Lvr $(SRC_CODE_DIR)/$(LIBCEDARC_DIR)/* $(PKG_BUILD_DIR)/
endif
	$(CP) -r ../machinfo/$(LICHEE_IC)/$(PKG_NAME)/$(LICHEE_IC)_linux_cedarc.conf $(PKG_BUILD_DIR)/
endef

define Build/Configure
	@echo "********target_board_platform = $MEDIA_TARGET_BOARD_PLATFORM***********"
	@echo "********chip type = $(TINA_CHIP_TYPE)*********"
	@echo "********kernel version = $(KERNEL_PATCHVER)*********"
	@echo "********Adapting and Checking KERNEL_VERSION = $(KERNEL_VERSION)*********"
	@echo "********Adapting and Checking KERNEL_VERSION_ION = $(KERNEL_VERSION_ION)*********"
	@echo "********toolchain = $(CONFIG_ARCH)_$(CONFIG_LIBC)*********"
	@echo "********toolchain-version = $(CONFIG_TOOLCHAIN_GCC_VERSION)*******"
	@echo "********cpu arch = $(TARGET_CPU_VARIANT)*********"
	@echo "********Adapting and Checking cedarc_libs_path: $(CEDARC_LIBS_PATH)*********"
	@echo "********Adapting and Checking cedarc compile tool chain: $(CEDARC_COMPILE_TOOL_CHAIN)*********"
	@echo "********Adapting and Checking STAGING_DIR: $(STAGING_DIR)*********"
	cd $(PKG_BUILD_DIR)/; \
	./bootstrap; \
	./configure --prefix=$(PKG_BUILD_DIR)/install --host=$(CEDARC_COMPILE_TOOL_CHAIN) $(CONFIG_AFTERTREATMENT) $(CONFIG_VDECODERDEMO) $(CONFIG_VENCODERDEMO) $(CONFIG_VENC_DEFAULT_LIB) $(CONFIG_VENC_V_LINE_LIB) CFLAGS="-D__ENABLE_ZLIB__ -D$(KERNEL_VERSION) -D$(KERNEL_VERSION_ION) -D$(ISP_VENC_FLAG) -D$(DEC_ATM_FLAG) -Wno-format-overflow" CPPFLAGS="-D__ENABLE_ZLIB__ -D$(KERNEL_VERSION) -D$(KERNEL_VERSION_ION) -D$(ISP_VENC_FLAG) -D$(DEC_ATM_FLAG)" LDFLAGS="$(CEDARC_LD_FLAGS) -L$(PKG_BUILD_DIR)/$(CEDARC_LIBS_PATH)  -L$(PKG_BUILD_DIR)/install/lib"

endef

define Build/Compile
	$(MAKE) $(PKG_JOBS) -C $(PKG_BUILD_DIR) \
		ARCH="$(TARGET_ARCH)" \
		AR="$(TARGET_AR)" \
		CC="$(TARGET_CC)" \
		CPP="$(TARGET_CXX)" \
		CFLAGS="$(TARGET_CFLAGS) -D__ENABLE_ZLIB__ -D$(KERNEL_VERSION) -D$(KERNEL_VERSION_ION) -D$(ISP_VENC_FLAG) -D$(DEC_ATM_FLAG) -DTINA_LINUX_SUPPORT=$(TINA_LINUX_API) $(CEDARC_EXTRA_CXXFLAGS) -Wno-format-overflow -Wno-format -Wno-format-security" \
		CPPFLAGS="$(TARGET_CPPFLAGS) -D__ENABLE_ZLIB__ -D$(KERNEL_VERSION) -D$(KERNEL_VERSION_ION) -D$(ISP_VENC_FLAG) -D$(DEC_ATM_FLAG) $(CEDARC_EXTRA_CXXFLAGS)" \
		LDFLAGS="$(TARGET_LDFLAGS) $(CEDARC_LD_FLAGS) -L$(PKG_BUILD_DIR)/$(CEDARC_LIBS_PATH) -L$(PKG_BUILD_DIR)/install/lib" \
		install
endef

define Build/InstallDev
	mkdir -p $(PKG_INSTALL_DIR)
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/lib
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/bin
	$(INSTALL_DIR) $(PKG_INSTALL_DIR)/usr/include/allwinner
	## copy libraries
	$(CP) $(PKG_BUILD_DIR)/$(CEDARC_LIBS_PATH)/*.so							$(PKG_INSTALL_DIR)/usr/lib/
	$(CP) $(PKG_BUILD_DIR)/install/lib/*.so								$(PKG_INSTALL_DIR)/usr/lib/
	
	cd $(PKG_INSTALL_DIR)/usr/lib/
	## copy header files.
	
	## cdc_log.h
	$(CP) $(PKG_BUILD_DIR)/base/include/cdc_log.h								$(PKG_INSTALL_DIR)/usr/include/allwinner/

	## $(LIBCEDARC_DIR) vdecoder/vencoder
	-cp -r $(PKG_BUILD_DIR)/include											$(PKG_INSTALL_DIR)/usr/include/allwinner/
	-cp -r $(PKG_BUILD_DIR)/vencoder/base/include										$(PKG_INSTALL_DIR)/usr/include/allwinner/

	echo "**InstallDev path:$(1)***"
	$(CP) $(PKG_INSTALL_DIR)/usr $(1)/usr

endef

define Package/$(PKG_NAME)/install
	$(INSTALL_DIR) $(1)/usr/lib/
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/

	mkdir -p $(PKG_BUILD_DIR)/all_libs
	$(CP) $(PKG_BUILD_DIR)/$(CEDARC_LIBS_PATH)/*.so $(PKG_BUILD_DIR)/all_libs
	$(CP) $(PKG_BUILD_DIR)/install/lib/*.so $(PKG_BUILD_DIR)/all_libs
	
	echo "**install path:$(1)***"

	for n in $(SUPPORT_LIBCEDARC_LIBRARY_ALL); \
	do \
		$(CP) $(PKG_BUILD_DIR)/all_libs/$$$$n $(1)/usr/lib; \
	done

	$(CP) $(PKG_BUILD_DIR)/$(LICHEE_IC)_linux_cedarc.conf $(PKG_BUILD_DIR)/all_libs/cedarc.conf
	$(CP)          $(PKG_BUILD_DIR)/all_libs/cedarc.conf						 $(1)/etc/cedarc.conf

	cd $(1)/usr/lib/; rm -rf $(NOT_SUPPORT_VIDEO_LIBS)
endef
	
endif

ifeq ($(MEDIA_TARGET_BOARD_PLATFORM), y)
$(eval $(call BuildPackage,$(PKG_NAME)))
endif
