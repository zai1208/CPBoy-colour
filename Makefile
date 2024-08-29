# Compiler and linker settings
CC:=gcc
CFLAGS:=-Wall -O2
LDFLAGS:=-Wl,-Map,output.map

# Source files
SRCS:=main.c utils.c
OBJS:=$(SRCS:.c=.o)

# Output files
TARGET:=output.elf

# Build rules
all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET) $(LDFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) output.map
