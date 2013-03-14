OBJS   = connection.o encode.o decode.o main.o
TARGET = piccrap
CFLAGS = -pedantic -Wall -Wextra -Werror -std=gnu99

$(TARGET): $(OBJS)
	$(CC) $(LDFLAGS) -o $(TARGET) $(OBJS)

all: $(TARGET)

clean:
	rm -f $(TARGET) $(OBJS)

.PHONY: clean
