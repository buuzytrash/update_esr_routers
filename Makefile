CC=gcc
CFLAGS=-Wall -Iinclude
OBJ=src/main.o src/uploader.o

all: uploader

uploader: $(OBJ)
	$(CC) -o uploader $(OBJ)

clean:
	rm -f uploader $(OBJ)
