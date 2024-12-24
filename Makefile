# Try make eink
CC := $(CROSS_PREFIX)cc
CFLAGS += -Wall -O2
LDFLAGS +=
OBJS = fbpad.o term.o pad.o draw.o font.o isdw.o scrsnap.o

ifdef EINK
OBJS += eink.o
CFLAGS += -DEINK -std=gnu99 -pthread -I$(shell pwd)/../build/include
LDFLAGS += -L$(shell pwd)/../build/lib -lfbink -pthread
endif

all: fbpad
fbpad.o: conf.h
term.o: conf.h
pad.o: conf.h
fbpad: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)
%.o: %.c
	$(CC) -c $(CFLAGS) $<
clean:
	rm -f *.o fbpad
