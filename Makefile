GCC=gcc
CFLAGS=-I./ -I./server -I./client -I./logging/headers -I./monitor/headers -Ilib/communication/headers -Ilib/serialization/headers -Ilib/sqlite/headers -Ilib/file_utils/headers
CFLAGS+=-Wall -Wextra -Wstrict-prototypes -Wold-style-definition -Wmissing-prototypes -Wmissing-declarations -Wdeclaration-after-statement
GCCFLAGS=-pthread
MATHFLAGS=-lm -lncurses
ifdef LOGGING
MATHFLAGS+=-lrt
endif


# GCCMACROS=-D__DEBUG__ 
ifdef LOGGING
GCCMACROS+=-D__LOGGING__
endif

IMPORT_CFLAGS=$(shell pkg-config --cflags json-c)
IMPORT_LDFLAGS=$(shell pkg-config --libs json-c) $(shell pkg-config --libs sqlite3)


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
MONITOR_ID=monitor

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

MONITOR_OUTPUT=monitor.bin
MONITOR_SOURCES=$(wildcard monitor/*.c)
MONITOR_OBJECTS=$(MONITOR_SOURCES:.c=.o)

ROOT_SOURCES=$(wildcard ./*.c)
ROOT_OBJECTS=$(ROOT_SOURCES:.c=.o)



ifdef LOGGING
all: $(LIB_ID) $(CLIENT_ID) $(SERVER_ID) $(LOGGING_ID) $(MONITOR_ID)
else
all: $(LIB_ID) $(CLIENT_ID) $(SERVER_ID) $(MONITOR_ID)
endif

$(CLIENT_ID): $(LIB_ID) $(CLIENT_OBJECTS) $(ROOT_OBJECTS)
	$(GCC) $(GCCFLAGS) $(CFLAGS) $(IMPORT_CFLAGS) -o $(CLIENT_OUTPUT) $(ROOT_OBJECTS) $(CLIENT_OBJECTS) $(LIB_OUTPUT) $(IMPORT_LDFLAGS) $(MATHFLAGS)

$(SERVER_ID):  $(LIB_ID) $(SERVER_OBJECTS) $(ROOT_OBJECTS)
	$(GCC) $(GCCFLAGS) $(CFLAGS) $(IMPORT_CFLAGS) -o $(SERVER_OUTPUT) $(ROOT_OBJECTS) $(SERVER_OBJECTS) $(LIB_OUTPUT) $(IMPORT_LDFLAGS) $(MATHFLAGS)

$(LOGGING_ID): $(LOGGING_OBJECTS) 
	$(GCC) $(GCCFLAGS) $(CFLAGS) $(IMPORT_CFLAGS) -o $(LOGGING_OUTPUT) $(LOGGING_OBJECTS) $(MATHFLAGS)

$(MONITOR_ID): $(MONITOR_OBJECTS) $(LIB_ID)
	$(GCC) $(GCCFLAGS) $(CFLAGS) $(IMPORT_CFLAGS) -o $(MONITOR_OUTPUT) $(MONITOR_OBJECTS) $(LIB_OUTPUT) $(IMPORT_LDFLAGS) $(MATHFLAGS) 

$(LIB_ID): $(LIBRARY_OBJECTS)
	ar rcs $(LIB_OUTPUT) $(LIBRARY_OBJECTS)

%.o: %.c
	$(GCC) $(GCCFLAGS) $(GCCMACROS) $(CFLAGS) $(IMPORT_CFLAGS) -c $< -o $@

wipe:
	rm -f /tmp/*.fifo /tmp/*.req /tmp/*.res

clean:
	rm -rfv *.o client/*.o server/*.o logging/*.o monitor/*.o lib/communication/*/*.o lib/serialization/*.o lib/sqlite/*.o lib/file_utils/*.o lib/*.a *.bin
	rm -fv /tmp/*.fifo /tmp/*.req /tmp/*.res

.PHONY: all wipe clean
	