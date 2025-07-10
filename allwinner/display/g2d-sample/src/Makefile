CC = gcc
CFLAGS = -Wall
LDFLAGS =
OBJS = g2d_bld_dmabuf.o g2d_fill_dmabuf.o g2d_lbc_rot.o g2d_mixer_task.o g2d_performance.o

all: g2d_bld_dmabuf g2d_fill_dmabuf g2d_lbc_rot g2d_mixer_task g2d_performance

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

yuview: $(OBJS)
	$(CC) -o $@ $(OBJS)

clean:
	rm -f rbcfg *.o
