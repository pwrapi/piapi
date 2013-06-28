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
	$(CC) $(CFLAGS) $(TESTOBJS) -L. -l$(LIBNAME) -o $(TARGET)
	$(CC) $(CFLAGS) proxy.o -L. -l$(LIBNAME) -o proxy 
	$(CC) $(CFLAGS) agent.o -L. -l$(LIBNAME) -o agent 
	$(CC) $(CFLAGS) native.o -L. -l$(LIBNAME) -o native 

clean:
	rm -f out/* $(TARGET) $(STATICLIB) $(OBJS)

lib: $(OBJS)
	$(AR) rcs $(STATICLIB) $(OBJS)
	
