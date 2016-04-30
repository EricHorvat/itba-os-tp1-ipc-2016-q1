GCC=gcc
CFLAGS=-I./ -I./server -I./client -Ilib/communication/headers -Ilib/serialization/headers -Ilib/sqlite/headers -Ilib/file_utils/headers
GCCFLAGS=-pthread

IMPORT_CFLAGS=$(shell pkg-config --cflags json-c)
IMPORT_LDFLAGS=$(shell pkg-config --libs json-c) $(shell pkg-config --libs yaml-0.1)


ifdef FIFO
ifdef SOCKET
$(error Cannot build FIFO and SOCKET. Select only one)
else
# fifo
LIBRARY_SOURCES=$(wildcard lib/communication/fifo/*.c) $(wildcard lib/serialization/*.c) $(wildcard lib/file_utils/*.c)
GCCMACROS=-D__FIFO__
endif
else
ifdef SOCKET
# socket
LIBRARY_SOURCES=$(wildcard lib/communication/socket/*.c) $(wildcard lib/serialization/*.c) $(wildcard lib/file_utils/*.c)
GCCMACROS=-D__SOCKET__
else
# fifo
$(info Neither FIFO nor SOCKET was chosen. Defaulting to FIFO)
LIBRARY_SOURCES=$(wildcard lib/communication/fifo/*.c) $(wildcard lib/serialization/*.c) $(wildcard lib/file_utils/*.c)
GCCMACROS=-D__FIFO__
endif
endif

LIBRARY_OBJECTS=$(LIBRARY_SOURCES:.c=.o)

CLIENT_ID=client
SERVER_ID=server

CLIENT=client.bin
CLIENT_SOURCES=$(wildcard client/*.c)
CLIENT_OBJECTS=$(CLIENT_SOURCES:.c=.o)


SERVER=server.bin
SERVER_SOURCES=$(wildcard server/*.c)
SERVER_OBJECTS=$(SERVER_SOURCES:.c=.o)



all: $(CLIENT_ID) $(SERVER_ID)

$(CLIENT_ID): $(CLIENT_OBJECTS) $(LIBRARY_OBJECTS)
	$(GCC) $(CFLAGS) $(IMPORT_CFLAGS) -o $(CLIENT) $(CLIENT_OBJECTS) $(LIBRARY_OBJECTS) $(IMPORT_LDFLAGS)

$(SERVER_ID): $(SERVER_OBJECTS) $(LIBRARY_OBJECTS)
	$(GCC) $(CFLAGS) $(IMPORT_CFLAGS) -o $(SERVER) $(SERVER_OBJECTS) $(LIBRARY_OBJECTS) $(IMPORT_LDFLAGS)

%.o: %.c
	$(GCC) $(GCCFLAGS) $(GCCMACROS) $(CFLAGS) $(IMPORT_CFLAGS) -c $< -o $@

wipe:
	rm -f /tmp/*.fifo /tmp/*.req /tmp/*.res

clean:
	rm -rfv *.o client/*.o server/*.o lib/communication/*/*.o lib/serialization/*.o lib/sqlite/*.o lib/file_utils/*.o
	rm -fv /tmp/*.fifo /tmp/*.req /tmp/*.res

.PHONY: all clean