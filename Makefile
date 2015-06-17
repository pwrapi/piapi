PREFIX = install
DBG ?= n
LVL ?= 1
XC ?= n
SPI ?= n
CNT ?= y
PIVER ?= 2
SHARED ?= y

LIBNAME = piapi

ifeq ($(XC),y)
CROSS_COMPILE = arm-linux-gnueabihf-
SPI = y
endif

CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar
CFLAGS = -Wall -O3 -pthread
LDFLAGS = -L. -l$(LIBNAME)

ifeq ($(SHARED),y)
CFLAGS += -fpic
LIB = lib$(LIBNAME).so
else
LIB = lib$(LIBNAME).a
endif

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
CFLAGS += -I$(PWD)/drivers/v$(PIVER) -DPIAPI_SPI
LDFLAGS += -L$(PWD)/drivers/v$(PIVER) -lpidev
endif

ifeq ($(CNT),y)
CFLAGS += -DPIAPI_COUNTERS
endif

OBJS = 	src/piapi.o     \
	src/piproxy.o   \
	src/piagent.o   \
	src/pinative.o  \
	src/piutil.o

TESTOBJS = tools/proxy.o  \
	tools/agent.o     \
	tools/native.o    \
	tools/test.o

all: $(LIBNAME) $(TESTOBJS)
	$(CC) $(CFLAGS) tools/test.c $(LDFLAGS) -o pitest
	$(CC) $(CFLAGS) tools/proxy.c $(LDFLAGS) -o piproxy 
	$(CC) $(CFLAGS) tools/agent.c $(LDFLAGS) -o piagent 
	$(CC) $(CFLAGS) tools/native.c $(LDFLAGS) -o pinative

clean:
	rm -f pitest piproxy piagent pinative $(LIB) $(OBJS) $(TESTOBJS)

ifeq ($(SHARED),y)
$(LIBNAME): $(OBJS)
	$(CC) -shared -o $(LIB) $(OBJS)
else
$(LIBNAME): $(OBJS)
	$(AR) rcs $(LIB) $(OBJS)
endif	

install:
	mkdir -p $(PREFIX)
	mkdir -p $(PREFIX)/include
	cp src/picommon.h piapi.h $(PREFIX)/include
	mkdir -p $(PREFIX)/lib
	cp $(LIB) $(PREFIX)/lib
	mkdir -p $(PREFIX)/bin
	cp pitest piproxy piagent pinative pilogger pimon piver $(PREFIX)/bin
	mkdir -p $(PREFIX)/man
	cp tools/piproxy.8 tools/pilogger.8 $(PREFIX)/man
