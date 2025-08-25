CC=gcc
CFLAGS=-Iinclude -Wall -Wextra -O2
LDFLAGS=-lssh2 -lncurses -fopenmp
SRCS=src/main.c src/config.c src/logging.c src/utils.c src/ssh.c src/update.c src/ui.c src/ini.c
OBJS=$(SRCS:.c=.o)
TARGET=esr_uploader
all:$(TARGET)
$(TARGET):$(OBJS)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)
clean:
	rm -f $(OBJS) $(TARGET)