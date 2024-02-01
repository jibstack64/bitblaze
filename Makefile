
CC = gcc
CFLAGS = -Wall -DPLATFORM_DESKTOP

LDFLAGS = -lm -lncurses

SRC = main.c
TARGET = bitblaze

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

clean:
	rm -f $(TARGET)

