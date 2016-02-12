bin_PROGRAMS = piproxy piagent pinative pitest

# PowerInsight API Tools
piproxy_SOURCES = proxy.c
piproxy_CFLAGS = -I$(top_srcdir)/src
piproxy_LDADD = $(top_builddir)/src/libpiapi.la -lpthread

piagent_SOURCES = agent.c
piagent_CFLAGS = -I$(top_srcdir)/src
piagent_LDADD = $(top_builddir)/src/libpiapi.la -lpthread

pinative_SOURCES = native.c
pinative_CFLAGS = -I$(top_srcdir)/src
pinative_LDADD = $(top_builddir)/src/libpiapi.la -lpthread

if HAVE_POWERINSIGHT
piagent_CFLAGS += $(POWERINSIGHT_CFLAGS)
piagent_LDFLAGS = $(POWERINSIGHT_LDFLAGS)
piagent_LDADD = $(POWERINSIGHT_LIBS)

pinative_CFLAGS += $(POWERINSIGHT_CFLAGS)
pinative_LDFLAGS = $(POWERINSIGHT_LDFLAGS)
pinative_LDADD = $(POWERINSIGHT_LIBS)
endif

pitest_SOURCES = test.c
pitest_CFLAGS = -I$(top_srcdir)/src
pitest_LDADD = $(top_builddir)/src/libpiapi.la -lpthread

man_MANS = pilogger.8 piproxy.8

bin_SCRIPTS = pilogger pimon piver

