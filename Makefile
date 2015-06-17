DBG ?= n
LVL ?= 1
SPI ?= n
CNT ?= y

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

