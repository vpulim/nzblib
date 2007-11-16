
INCLUDES = -I$(top_srcdir)/include

bin_PROGRAMS = nzb_leech

nzb_leech_SOURCES = main.c


nzb_leech_LDFLAGS = ${EXPAT_LDFLAGS}

nzb_leech_LDADD = ../src/libnzb_fetch.la ${EXPAT_LIBS} -lpthread
