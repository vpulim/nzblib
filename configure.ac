AC_PREREQ(2.59)
AC_INIT([libnzbfetch], [trunk], [michaelvantellingen@gmail.com])
AM_INIT_AUTOMAKE(libnzbfetch, 0.1)
AM_CONFIG_HEADER(config.h)


AC_LANG(C)

AM_INIT_AUTOMAKE


m4_include([aclocal/libtool.m4])

AC_GNU_SOURCE
AC_PROG_CC
AC_PROG_LIBTOOL

CFLAGS="${CFLAGS} -std=c99 -Wall -Winline"

##
# Check for expat
save_LIBS="${LIBS}"

LIBS=""

AC_ARG_WITH([expat], [  --with-expat=path       specify that expat is installed at path], [
    if test -n "$withval"; then
        CFLAGS="$CFLAGS -I$withval/include"
        LDFLAGS="$LDFLAGS -L$withval/lib"
    fi
])

AC_CHECK_LIB(expat, XML_Parse,, [
                    AC_MSG_ERROR([expat not found, please use --with-expat])
                ])

AC_CHECK_HEADER([expat.h],, [
                    AC_MSG_ERROR([expat not found, please use --with-expat])
                ])
                
EXPAT_LIBS="${LIBS}"


LIBS="${save_LIBS}"

AC_SUBST(EXPAT_LIBS)


##
# Check for reallocf (FreeBSD api)
AC_CHECK_FUNCS([reallocf])



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
    include/Makefile
    src/Makefile
    example/Makefile
])
AC_OUTPUT
