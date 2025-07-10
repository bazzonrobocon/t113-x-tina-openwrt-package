Target = tplayerdemo

SourceIncludePath := -I$(STAGING_DIR)/usr/include/allwinner/include -I$(STAGING_DIR)/usr/include/allwinner
CompileFlags = $(CFLAGS) $(SourceIncludePath)

ifeq ($(ONLY_ENABLE_AUDIO),y)
CompileFlags += -DONLY_ENABLE_AUDIO
endif

ifeq ($(ONLY_DISABLE_AUDIO),y)
CompileFlags += -DONLY_DISABLE_AUDIO
endif
LoadFlags +=  -ltplayer -lcdx_base -lrt -lstdc++
target_src := tplayerdemo.c


#############################################################
#mode that for suspend are temporary.
#todo: communicate with kernel to get the suspend status.
#############################################################
#############################################################
#TPLAYER_POWER_MONITOR:for suspend while playing multi-videos
#############################################################
ifneq ($(filter TPLAYER_POWER_MONITOR, $(GET_EXTRA_DEMO_CONFIG)),)
#cdx_common for parsing the cedarx.conf and set the ms for suspend.
LoadFlags += -lcdx_common
target_src += power_monitor/power_monitor.c
CompileFlags += -I./power_monitor
CompileFlags += -DTPLAYER_POWER_MONITOR
endif

############################################################
#TPLAYER_SUSPEND_TEST:a mode of tplayerdemo to test suspend
#                     while playing multi-videos.
############################################################
ifneq ($(filter TPLAYER_SUSPEND_TEST, $(GET_EXTRA_DEMO_CONFIG)),)
target_src += suspend_test/tplayer_suspend_test.c
CompileFlags += -I./suspend_test -I.
CompileFlags += -DTPLAYERDEMO_SUSPEND_TEST
endif


$(Target): $(target_src)
	$(CC) -o $@ $^ $(CompileFlags) $(LDFLAGS) $(LoadFlags)
