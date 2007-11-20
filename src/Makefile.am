INCLUDES = -I$(top_srcdir)/include

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
        yenc.c \
        compat\reallocf.c \
        compat\asprintf.c \
        compat\vasprintf.c \
        compat\strsep.c
            
libnzb_fetch_la_CLFAGS = -include config.h
libnzb_fetch_la_LIBADD = ${EXPAT_LIB}

