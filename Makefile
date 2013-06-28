#CROSS_COMPILE=arm-linux-gnueabihf-

CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar
CFLAGS = -Wall -O0 -pthread

TARGET = getRawPower
LIBNAME = piapi

TESTOBJS = $(TARGET).o
STATICLIB = lib$(LIBNAME).a
OBJS = piapi.o piproxy.o piagent.o pinative.o piutil.o pidev.o proxy.o agent.o native.o

all: lib $(TESTOBJS)
	$(CC) $(CFLAGS) $(TESTOBJS) -L../lib -l$(LIBNAME) -o ../bin/$(TARGET)
	$(CC) $(CFLAGS) proxy.o -L../lib -l$(LIBNAME) -o ../bin/proxy 
	$(CC) $(CFLAGS) agent.o -L../lib -l$(LIBNAME) -o ../bin/agent 
	$(CC) $(CFLAGS) native.o -L../lib -l$(LIBNAME) -o ../bin/native 

clean:
	rm -f out/* ../bin/$(TARGET) ../lib/$(STATICLIB) $(OBJS)

lib: $(OBJS)
	$(AR) rcs ../lib/$(STATICLIB) $(OBJS)
	
