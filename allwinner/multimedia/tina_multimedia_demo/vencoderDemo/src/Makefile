TARGET := vencoderDemo
INCLUDES +=	-I$(STAGING_DIR)/usr/include/allwinner/include -I$(STAGING_DIR)/usr/include/allwinner
LIBS += -lvencoder -lVE -lvenc_common -lvenc_base -lcdc_base -lMemAdapter -lvenc_h264 -lvenc_h265 -lvenc_jpeg -lpthread -ldl -lm

SRCS = vencoderDemo.c \
       avtimer.c

OBJS = $(SRCS:.c=.o)

%.o: %.c
	$(CC) $(CFLAGS) -g $(INCLUDES) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS) $(LDFLAGS)

all:$(TARGET)


clean:
	rm -rf $(TARGET) *.o *.a *~
	cd common && rm -f *.o *.a *.bak *~ .depend
