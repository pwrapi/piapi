PREFIX = install
DBG ?= n
LVL ?= 1
XC ?= n
SPI ?= n
CNT ?= y

ifeq ($(XC),y)
CROSS_COMPILE = arm-linux-gnueabihf-
SPI = y
endif

CC = $(CROSS_COMPILE)gcc
CFLAGS = -Wall -O3 -pthread -fpic

ifeq ($(DBG),y)
CFLAGS += -g
ifeq ($(LVL),0)
CFLAGS += -DPIAPI_DEBUG
endif
ifeq ($(LVL),1)
CFLAGS += -DPIAPI_PROXY_DEBUG -DPIAPI_AGENT_DEBUG
endif
ifeq ($(LVL),2)
CFLAGS += -DPIAPI_PROXY_DEBUG -DPIAPI_AGENT_DEBUG -DPIAPI_NATIVE_DEBUG
endif
endif

ifeq ($(SPI),y)
CFLAGS += -DPIAPI_SPI
LDFLAGS += -lpidev
endif

ifeq ($(CNT),y)
CFLAGS += -DPIAPI_COUNTERS
endif

TARGET = test
LIBNAME = piapi

SHAREDLIB = lib$(LIBNAME).so

OBJS = 	piapi.o     \
	piproxy.o   \
	piagent.o   \
	pinative.o  \
	piutil.o

TESTOBJS = proxy.o  \
	agent.o     \
	native.o    \
	$(TARGET).o

all: $(LIBNAME) $(TESTOBJS)
	$(CC) $(CFLAGS) $(TARGET).c -L. -l$(LIBNAME) -o $(TARGET)
	$(CC) $(CFLAGS) proxy.c -L. -l$(LIBNAME) -o piproxy 
	$(CC) $(CFLAGS) agent.c -L. -l$(LIBNAME) -o piagent 
	$(CC) $(CFLAGS) native.c -L. -l$(LIBNAME) -o pinative

clean:
	rm -f $(TARGET) piproxy piagent pinative $(SHAREDLIB) $(OBJS) $(TESTOBJS)

$(LIBNAME): $(OBJS)
	$(CC) -shared -o $(SHAREDLIB) $(OBJS)
	
install:
	mkdir -p $(PREFIX)
	mkdir -p $(PREFIX)/include
	cp picommon.h piapi.h $(PREFIX)/include
	mkdir -p $(PREFIX)/lib
	cp $(SHAREDLIB) $(PREFIX)/lib
	mkdir -p $(PREFIX)/bin
	cp $(TARGET) piproxy piagent pinative pilogger pimon piver $(PREFIX)/bin
	mkdir -p $(PREFIX)/man
	cp piproxy.8 pilogger.8 $(PREFIX)/man
