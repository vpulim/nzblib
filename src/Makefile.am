INCLUDES = -I$(top_srcdir)/include
SUBDIRS = compat

lib_LTLIBRARIES = libnzb_fetch.la
libnzb_fetch_la_SOURCES = \
        crc32.c \
        file.c \
        libnzb_fetch.c \
        net.c \
        nttp_connection.c \
        parse_nzb.c \
        post.c \
        process.c \
        queue.c \
        segment.c \
        server.c \
        yenc.c 


            
libnzb_fetch_la_CFLAGS = -include config.h -Wall -pedantic -Werror-implicit-function-declaration -Wconversion
libnzb_fetch_la_LIBADD = ${EXPAT_LIBS} ${SSL_LIBS} compat/libcompat.la
