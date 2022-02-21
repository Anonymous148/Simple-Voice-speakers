LIBS = $(wildcard lib/*)
CC = arm-none-linux-gnueabi-gcc

TARGET:=voicectl
SRCS:=$(wildcard ./src/*.c)
OBJS:=$(patsubst %.c,%.o,$(SRCS))

CPPFLAGS += -I ./inc
CPPFLAGS += -I ./inc/libxml2
LDFLAGS  += -L ./lib


LDFLAGS += -Wl,-rpath=. -Wl,-rpath=./lib

$(TARGET):$(OBJS)
	$(CC) $^ -o $@ $(CPPFLAGS) $(LDFLAGS) -Wall
%.o:%.c
	$(CC) -c $^ -o $@ $(CPPFLAGS) $(LDFLAGS) -Wall

clean:
	rm $(OBJS) $(TARGET) -rf
