bin_PROGRAMS = piproxy piagent pinative pitest pilogger pimon piver

# PowerInsight API Tools
piproxy_SOURCES = proxy.c
piproxy_CFLAGS = -I$(top_srcdir)/src
piproxy_LDADD = $(top_builddir)/src/libpiapi.la

piagent_SOURCES = agent.c
piagent_CFLAGS = -I$(top_srcdir)/src
piagent_LDADD = $(top_builddir)/src/libpiapi.la

pinative_SOURCES = native.c
pinative_CFLAGS = -I$(top_srcdir)/src
pinative_LDADD = $(top_builddir)/src/libpiapi.la

pitest_SOURCES = test.c
pitest_CFLAGS = -I$(top_srcdir)/src
pitest_LDADD = $(top_builddir)/src/libpiapi.la

man_MANS = pilogger.8 piproxy.8

