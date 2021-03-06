CC=gcc
CFLAGS=-std=c89 \
	-O3 -Wall -Wextra -Wundef -Wfloat-equal -Wpointer-arith \
	-Wcast-align -Wstrict-prototypes -Wno-missing-field-initializers \
	-Warray-bounds -Wwrite-strings -pedantic -fstrict-aliasing \
	-Wshadow -Wswitch-default -Wswitch-enum -g
LINKS=-lpthread -lssl -lcrypto
INCLUDES=-I. -I./src

LIB_OBJS =
LIB_OBJS += build/objs/bserve.o
LIB_OBJS += build/objs/config.o
LIB_OBJS += build/objs/error.o
LIB_OBJS += build/objs/ipc.o
LIB_OBJS += build/objs/request.o
LIB_OBJS += build/objs/response.o

EXEC=bserve

all: $(EXEC)
	@echo ""

$(EXEC): $(LIB_OBJS)
	@$(CC) $(CFLAGS) -o $@ $(LIB_OBJS) $(INCLUDES) $(LINKS)

$(LIB_OBJS): build/objs/%.o: src/%.c
	@echo "cc: $<"
	@$(CC) $(CFLAGS) -c $< $(INCLUDES) $(LINKS) -o $@

clean:
	find . -type f -name '*.o' -delete

install:
	sudo cp bserve /usr/local/bin
