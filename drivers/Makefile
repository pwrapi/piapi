PREFIX = /usr
#CROSS_COMPILE = arm-linux-gnueabihf-

PIVER = 2

CC = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -O3 -fpic

TARGET = getRawPower
LIBNAME = pidev

SHAREDLIB = lib$(LIBNAME).so

ifeq ($(PIVER),2)
CFLAGS += -MMD -O -I/usr/include/lua-5.1 -I$(PWD)/../../powerinsight
LDFLAGS += -L$(PWD)/../../powerinsight -lpidev -lm -llua-5.1
OBJS = pidev2.o
else
OBJS = pidev.o
endif

TESTOBJS = $(TARGET).o

all: $(LIBNAME) $(TESTOBJS)
	$(CC) $(CFLAGS) $(TARGET).c $(LDFLAGS) -L. -l$(LIBNAME) -o $(TARGET)

clean:
	rm -f $(TARGET) $(SHAREDLIB) $(OBJS) $(TESTOBJS)

$(LIBNAME): $(OBJS)
	$(CC) -shared -o $(SHAREDLIB) $(OBJS)
	
install:
	mkdir -p $(PREFIX)
	mkdir -p $(PREFIX)/include
	cp pidev.h $(PREFIX)/include
	mkdir -p $(PREFIX)/lib
	cp $(SHAREDLIB) $(PREFIX)/lib
	mkdir -p $(PREFIX)/bin
	cp $(TARGET) $(PREFIX)/bin
	cp .config $(PREFIX)/bin
	ln -s $(PREFIX)/bin /etc/powerinsight.conf
