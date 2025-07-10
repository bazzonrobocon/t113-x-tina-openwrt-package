Target = backplaydemo

CFLAGS += -I.
SourceIncludePath := -I$(STAGING_DIR)/usr/include/allwinner/include -I$(STAGING_DIR)/usr/include/allwinner
CompileFlags = $(CFLAGS) $(SourceIncludePath)

ifeq ($(ONLY_ENABLE_AUDIO),y)
CompileFlags += -DONLY_ENABLE_AUDIO
endif

LoadFlags +=  -ltplayer -lcdx_common -lcdx_base -lrt -lstdc++

$(Target): backplaydemo.c backplay.c
	$(CC) -o $@ $^ $(CompileFlags) $(LDFLAGS) $(LoadFlags)
