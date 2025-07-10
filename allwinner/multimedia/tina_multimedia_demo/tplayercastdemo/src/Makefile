target			= tplayer_cast_demo

INCLUDES        += -I. -I./include -I$(STAGING_DIR)/usr/include/allwinner/include -I$(STAGING_DIR)/usr/include/allwinner
STATIC_LIB		:= -lm
DYNAMIC_LIB		:= -lc -lpthread -ltplayer -law_dlna -lcdx_base
SRCDIRS = $(shell find . -maxdepth 5 -type d)

SRCS := $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.c))
SRCS += $(foreach dir,$(SRCDIRS),$(wildcard $(dir)/*.cpp))
OBJS = $(SRCS:.c=.o)
LOCAL_LDFLAGS :=

ifdef STATIC_LIB
	LOCAL_LDFLAGS += -Wl,-Bstatic -Xlinker "-(" $(STATIC_LIB) -Xlinker "-)"
endif

ifdef DYNAMIC_LIB
	LOCAL_LDFLAGS += -Wl,-Bdynamic $(DYNAMIC_LIB)
else
	LOCAL_LDFLAGS += -static
endif

install:
	-@mkdir -p $(CONFIG_PREFIX)/usr/bin

%.o: %.c
	$(CC) $(INCLUDES) $(CFLAGS) $(LOCAL_CFLAGS) -c -o $@ $<

$(target): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS) -L../dlna/lib $(LOCAL_LDFLAGS)
	mkdir -p $(CONFIG_PREFIX)/usr/bin
	@cp $@ $(CONFIG_PREFIX)/usr/bin

all: $(target)
clean:
	rm -rf $(target)
