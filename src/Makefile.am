lib_LTLIBRARIES = libpiapi.la

include_HEADERS = piapi.h picommon.h

# Power API Framework
libpiapi_la_SOURCES = piapi.c piproxy.c piagent.c pinative.c piutil.c

libpiapi_la_CFLAGS = -pthread
libpiapi_la_LDFLAGS = -version-info 2:1:1 -lpthread

if HAVE_POWERINSIGHT
libpiapi_la_CFLAGS += $(POWERINSIGHT_CFLAGS)
libpiapi_la_LDFLAGS += $(POWERINSIGHT_LDFLAGS)
libpiapi_la_LDADD = $(POWERINSIGHT_LIBS)
endif
