TARGET = camerademo
INCLUDES += -I$(STAGING_DIR)/usr/include/allwinner/include -I$(STAGING_DIR)/usr/include/allwinner
LIBS +=  -lpthread -ldl -lrt \

ifeq ($(USE_VIN_ISP),y)
	CompileFlags += -D__USE_VIN_ISP__

	LIBS += -lm -lisp -lisp_ini -lAWIspApi
endif

	SRCS = camerademo.c convert.c common.c
ifeq ($(USE_ION),y)
	CompileFlags += -D__USE_ION__

	SRCS += ion_alloc.c
endif

ifeq ($(SAVE_RV_YUV),y)
	CompileFlags += -D__SAVE_RV_YUV__
endif
	OBJS = $(SRCS:.c=.o)

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDES) $(CompileFlags) -c -o $@ $<

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS) $(LDFLAGS)

all:$(TARGET)

clean:
	rm -rf $(TARGET) *.o *~
