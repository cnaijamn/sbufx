LIB=	sbufx
SHLIB_MAJOR?=	1
LDADD+=

SRCS=	sbufx.c
INCS=

WARNS=	3
CFLAGS+=
LDFLAGS+=

FILESGROUPS+=	SYSINCS
SYSINCS=	sbufx.h
SYSINCSDIR=	${INCLUDEDIR}/sys

FILESGROUPS+=	PKGCONF
PKGCONF=	libsbufx.pc
PKGCONFDIR=	${LIBDATADIR}/pkgconfig

#.include <bsd.opts.mk>
#
#HAS_TESTS=
#SUBDIR.${MK_TESTS}+=	tests

.include <bsd.lib.mk>
