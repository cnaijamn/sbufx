TESTSDIR=.
MK_MAKE_CHECK_USE_SANDBOX=no
#TESTS_LD_LIBRARY_PATH+=${.CURDIR}/..

ATF_TESTS_C+=	sbufx_test

LDADD+=		-lsbufx -lsbuf
CFLAGS+=	-I.. `pkgconf --cflags atf-c`
LDFLAGS+=	-L.. `pkgconf --libs atf-c`

WARNS?=		3

.include <bsd.test.mk>
