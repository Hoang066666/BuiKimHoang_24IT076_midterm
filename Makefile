CC      = gcc
CFLAGS  = -Wall -Wextra -std=c11 -pedantic
TARGET  = ls
SRCS    = main.c options.c fileinfo.c format.c sort.c display.c
OBJS    = $(SRCS:.c=.o)

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET)
