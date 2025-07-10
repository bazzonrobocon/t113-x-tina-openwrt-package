PKG_NAME:=mali-utgard-km
#Make sure TINA_TARGET_PLAT is null to avoid build error for Mali Utgard Device Driver
TINA_TARGET_PLAT=
GPU_NEW_DIR := $(LINUX_DIR)/modules/gpu/mali-utgard
GPU_OLD_DIR := $(GPU_NEW_DIR)/kernel_mode
ifneq (wildcard $(GPU_OLD_DIR)/driver/src/devicedrv/mali/.version),)
GPU_BUILD_DIR := $(GPU_OLD_DIR)/driver/src/devicedrv/mali
else
GPU_BUILD_DIR := $(GPU_NEW_DIR)/driver/src/devicedrv/mali
endif
GPU_KO_DIR := $(GPU_BUILD_DIR)/mali.ko