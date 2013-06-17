# CROSS_COMPILE=arm-linux-gnueabihf-

CC = $(CROSS_COMPILE)gcc
AR = $(CROSS_COMPILE)ar
CFLAGS = -O2

TARGET = getRawPower
LIBNAME = piapi

TESTOBJS = $(TARGET).o
STATICLIB = lib$(LIBNAME).a
OBJS = piapi.o pidev.o

all: lib $(TESTOBJS)
	$(CC) $(CFLAGS) $(TESTOBJS) -L. -l$(LIBNAME) -o $(TARGET)

clean:
	rm -f out/* $(TARGET) $(TESTOBJS) $(STATICLIB) $(OBJS)

lib: $(OBJS)
	$(AR) rcs $(STATICLIB) $(OBJS)
	
