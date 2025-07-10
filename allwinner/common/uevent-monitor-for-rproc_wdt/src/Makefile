TARGET = uevent-monitor-for-rproc_wdt

$(TARGET): main.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^

all:$(TARGET) install

install:
	mkdir -p $(CONFIG_PREFIX)/usr/bin
	cp $(TARGET) $(CONFIG_PREFIX)/usr/bin/

clean:
	rm -rf $(TARGET) $(OBJS)
