PREFIX = ../install
CROSS_COMPILE = arm-linux-gnueabihf-

CC = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -O3 -fpic

TARGET = getRawPower
LIBNAME = pidev

SHAREDLIB = lib$(LIBNAME).so

TESTOBJS = $(TARGET).o

all: $(TESTOBJS)
	$(CC) $(CFLAGS) $(TARGET).c $(LDFLAGS) -L. -l$(LIBNAME) -o $(TARGET)

clean:
	rm -f $(TARGET) $(TESTOBJS)

install:
	mkdir -p $(PREFIX)
	mkdir -p $(PREFIX)/include
	cp pidev.h $(PREFIX)/include
	mkdir -p $(PREFIX)/lib
	cp $(SHAREDLIB) $(PREFIX)/lib
	mkdir -p $(PREFIX)/bin
	cp $(TARGET) $(PREFIX)/bin
	cp amd.config $(PREFIX)/bin/pidev.conf
