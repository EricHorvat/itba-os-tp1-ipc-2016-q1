GCC=gcc
CFLAGS=-I./ -I./server -I./client -Ilib/communication/headers -Ilib/serialization/headers -Ilib/sqlite/headers -Ilib/file_utils/headers
GCCFLAGS=-pthread -pthread

IMPORT_CFLAGS=$(shell pkg-config --cflags glib-2.0) $(shell pkg-config --cflags json-c)
IMPORT_LDFLAGS=$(shell pkg-config --libs glib-2.0) $(shell pkg-config --libs json-c) $(shell pkg-config --libs yaml-0.1)



LIBRARY_SOURCES=$(wildcard lib/communication/*.c) $(wildcard lib/serialization/*.c) $(wildcard lib/file_utils/*.c)
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
	$(GCC) $(GCCFLAGS) $(CFLAGS) $(IMPORT_CFLAGS) -o $(CLIENT) $(CLIENT_OBJECTS) $(LIBRARY_OBJECTS) $(IMPORT_LDFLAGS)

$(SERVER_ID): $(SERVER_OBJECTS) $(LIBRARY_OBJECTS)
	$(GCC) $(GCCFLAGS) $(CFLAGS) $(IMPORT_CFLAGS) -o $(SERVER) $(SERVER_OBJECTS) $(LIBRARY_OBJECTS) $(IMPORT_LDFLAGS)

%.o: %.c
	$(GCC) $(GCCFLAGS) $(CFLAGS) $(IMPORT_CFLAGS) -c $< -o $@

wipe:
	rm -f /tmp/*.fifo /tmp/*.req /tmp/*.res

clean:
	rm -rf *.o client/*.o server/*.o lib/communication/*.o lib/serialization/*.o lib/sqlite/*.o
	rm -f /tmp/*.fifo /tmp/*.req /tmp/*.res

.PHONY: all clean