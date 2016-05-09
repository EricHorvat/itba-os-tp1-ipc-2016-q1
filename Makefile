GCC=gcc
CFLAGS=-I./ -I./server -I./client -I./logging/headers -Ilib/communication/headers -Ilib/serialization/headers -Ilib/sqlite/headers -Ilib/file_utils/headers
CFLAGS+=-Wall
GCCFLAGS=-pthread
MATHFLAGS=-lm -lrt

GCCMACROS=-D__DEBUG__

IMPORT_CFLAGS=$(shell pkg-config --cflags json-c)
IMPORT_LDFLAGS=$(shell pkg-config --libs json-c) $(shell pkg-config --libs yaml-0.1) $(shell pkg-config --libs sqlite3)


LIBRARY_SOURCES=$(wildcard lib/serialization/*.c) $(wildcard lib/sqlite/*.c) $(wildcard lib/file_utils/*.c)

ifdef FIFO
ifdef SOCKET
$(error Cannot build FIFO and SOCKET. Select only one)
else
# fifo
LIBRARY_SOURCES+= $(wildcard lib/communication/fifo/*.c)
GCCMACROS+=-D__FIFO__
endif
else
ifdef SOCKET
# socket
LIBRARY_SOURCES+= $(wildcard lib/communication/socket/*.c)
GCCMACROS+=-D__SOCKET__
else
# fifo
$(info Neither FIFO nor SOCKET was chosen. Defaulting to FIFO)
LIBRARY_SOURCES+= $(wildcard lib/communication/fifo/*.c)
GCCMACROS+=-D__FIFO__
endif
endif

LIBRARY_OBJECTS=$(LIBRARY_SOURCES:.c=.o)

CLIENT_ID=client
SERVER_ID=server
LIB_ID=lib
LOGGING_ID=logging

LIB_OUTPUT=lib/lib.a

CLIENT_OUTPUT=client.bin
CLIENT_SOURCES=$(wildcard client/*.c)
CLIENT_OBJECTS=$(CLIENT_SOURCES:.c=.o)


SERVER_OUTPUT=server.bin
SERVER_SOURCES=$(wildcard server/*.c)
SERVER_OBJECTS=$(SERVER_SOURCES:.c=.o)

LOGGING_OUTPUT=logging.bin
LOGGING_SOURCES=$(wildcard logging/*.c)
LOGGING_OBJECTS=$(LOGGING_SOURCES:.c=.o)

ROOT_SOURCES=$(wildcard ./*.c)
ROOT_OBJECTS=$(ROOT_SOURCES:.c=.o)



all: $(LIB_ID) $(CLIENT_ID) $(SERVER_ID) $(LOGGING_ID) 

$(CLIENT_ID): $(LIB_ID) $(CLIENT_OBJECTS) $(ROOT_OBJECTS)
	$(GCC) $(GCCFLAGS) $(CFLAGS) $(IMPORT_CFLAGS) -o $(CLIENT_OUTPUT) $(ROOT_OBJECTS) $(CLIENT_OBJECTS) $(LIB_OUTPUT) $(IMPORT_LDFLAGS) $(MATHFLAGS)

$(SERVER_ID):  $(LIB_ID) $(SERVER_OBJECTS) $(ROOT_OBJECTS)
	$(GCC) $(GCCFLAGS) $(CFLAGS) $(IMPORT_CFLAGS) -o $(SERVER_OUTPUT) $(ROOT_OBJECTS) $(SERVER_OBJECTS) $(LIB_OUTPUT) $(IMPORT_LDFLAGS) $(MATHFLAGS)

$(LOGGING_ID):  $(LOGGING_OBJECTS) 
	$(GCC) $(GCCFLAGS) $(CFLAGS) $(IMPORT_CFLAGS) -o $(LOGGING_OUTPUT) $(LOGGING_OBJECTS) $(MATHFLAGS) 

$(LIB_ID): $(LIBRARY_OBJECTS)
	ar rcs $(LIB_OUTPUT) $(LIBRARY_OBJECTS)

%.o: %.c
	$(GCC) $(GCCFLAGS) $(GCCMACROS) $(CFLAGS) $(IMPORT_CFLAGS) -c $< -o $@

wipe:
	rm -f /tmp/*.fifo /tmp/*.req /tmp/*.res

clean:
	rm -rfv *.o client/*.o server/*.o logging/*.o lib/communication/*/*.o lib/serialization/*.o lib/sqlite/*.o lib/file_utils/*.o lib/*.a
	rm -fv /tmp/*.fifo /tmp/*.req /tmp/*.res

.PHONY: all clean