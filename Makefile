# mininet - MINIX 3 network monitor

CC = clang

PREFIX = /usr/local

CFLAGS = -Wall -Wextra -O2 \
         -D_XOPEN_SOURCE_EXTENDED=1 \
         -I/usr/pkg/include/ncursesw \
         -I/usr/pkg/include

LDFLAGS = -L/usr/pkg/lib \
          -Wl,-rpath,/usr/pkg/lib

CITRUS_LIB ?= ../citrus-compat/libcitrus-compat.a

LDLIBS = -Wl,--whole-archive $(CITRUS_LIB) \
         -Wl,--no-whole-archive \
         -lncursesw

TARGET = mininet
OBJS = main.o collector.o history.o ui.o

all: $(TARGET)

$(TARGET): $(OBJS) $(CITRUS_LIB)
	$(CC) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)

main.o: main.c collector.h history.h ui.h
collector.o: collector.c collector.h
history.o: history.c history.h collector.h
ui.o: ui.c ui.h collector.h history.h

clean:
	rm -f $(TARGET) $(OBJS)

install: $(TARGET)
	mkdir -p $(DESTDIR)$(PREFIX)/bin
	cp $(TARGET) $(DESTDIR)$(PREFIX)/bin/
	chmod 755 $(DESTDIR)$(PREFIX)/bin/$(TARGET)

.PHONY: all clean install
