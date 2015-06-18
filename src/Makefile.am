lib_LTLIBRARIES = libpiapi.la

include_HEADERS = piapi.h picommon.h

# Power API Framework
libpiapi_la_SOURCES = piapi.c piproxy.c piagent.c pinative.c piutil.c

libpiapi_la_CFLAGS = $(PIDEV_CFLAGS) -pthread
libpiapi_la_LDFLAGS = $(LDFLAGS) -version-info 2:0:1 $(PIDEV_LDFLAGS) -lpthread

