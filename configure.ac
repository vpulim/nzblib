AC_PREREQ(2.59)
AC_INIT([libnzbfetch], [trunk], [michaelvantellingen@gmail.com])
AM_INIT_AUTOMAKE(libnzbfetch, 0.1)
AM_CONFIG_HEADER(config.h)


AC_LANG(C)

AM_INIT_AUTOMAKE


AC_GNU_SOURCE
AC_PROG_CC


##
# Check for expat
save_CFLAGS="${CFLAGS}"
save_LDFLAGS="${LDFLAGS}"
save_LIBS="${LIBS}"

LIBS=""

AC_ARG_WITH([expat], [  --with-expat=path       specify that expat is installed at path], [
    if test -n "$withval"; then
        CPPFLAGS="$CPPFLAGS -I$withval/include"
        LDFLAGS="$LDFLAGS -L$withval/lib"
    fi
])

AC_CHECK_LIB(expat, XML_Parse,, [
                    AC_MSG_ERROR([C compiler can't find <expat.h>])
                ])

AC_CHECK_HEADER([expat.h],, [
                    AC_MSG_ERROR([C compiler can't find <expat.h>])
                ])
                
EXPAT_LIBS="${LIBS}"

CLFAGS="${save_CFLAGS}"
LDFLAGS="${save_LDFLAGS}"
LIBS="${save_LIBS}"

AC_SUBST(EXPAT_LIBS)



##
# Other header files
AC_CHECK_HEADER([assert.h],, AC_MSG_ERROR(["Missing include file"]))
AC_CHECK_HEADER([netdb.h],, AC_MSG_ERROR(["Missing include file"]))
AC_CHECK_HEADER([netinet/in.h],, AC_MSG_ERROR(["Missing include file"]))
AC_CHECK_HEADER([pthread.h],, AC_MSG_ERROR(["Missing include file"]))
AC_CHECK_HEADER([stdarg.h],, AC_MSG_ERROR(["Missing include file"]))
AC_CHECK_HEADER([stdio.h],, AC_MSG_ERROR(["Missing include file"]))
AC_CHECK_HEADER([sys/param.h],, AC_MSG_ERROR(["Missing include file"]))
AC_CHECK_HEADER([sys/socket.h],, AC_MSG_ERROR(["Missing include file"]))

AC_CONFIG_FILES([
    Makefile
])
AC_OUTPUT
