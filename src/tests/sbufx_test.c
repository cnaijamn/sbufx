/*-
 * BSD 3-Clause License
 *
 * Copyright (c) 2020, 2021, 2023 Chez Naijamn
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "../sbufx.h"

#include <atf-c.h>

ATF_TC_WITHOUT_HEAD(sbufx_attach_test);
ATF_TC_BODY(sbufx_attach_test, tc)
{
	struct sbuf *s;
	char *buf1, *buf2;
	int ret;

	s = sbuf_new_auto();
	ATF_REQUIRE(s != NULL);

	buf1 = malloc(4);
	ATF_REQUIRE(buf1 != NULL);

	ret = sbufx_attach(s, buf1, 0, 4);
	ATF_CHECK_MSG(ret == 0, "ret = %d", ret);
	ATF_CHECK(buf1 == s->s_buf);
	ATF_CHECK(s->s_len == 0);
	ATF_CHECK(s->s_size == 4);

	buf2 = malloc(4);
	ATF_REQUIRE(buf2 != NULL);
	strcpy(buf2, "abc");

	ret = sbufx_attach(s, buf2, 3, 4);
	ATF_CHECK_MSG(ret == 0, "ret = %d", ret);
	ATF_CHECK(buf2 == s->s_buf);
	ATF_CHECK(s->s_len == 3);
	ATF_CHECK(s->s_size == 4);

	ret = sbuf_cat(s, "def");
	ATF_CHECK_MSG(ret == 0, "ret = %d", ret);
	ATF_CHECK_MSG(strcmp(sbuf_data(s), "abcdef") == 0, "sbuf_data = %s", sbuf_data(s));

	sbuf_delete(s);
}

ATF_TC_WITHOUT_HEAD(sbufx_detach_test);
ATF_TC_BODY(sbufx_detach_test, tc)
{
	struct sbuf *s;
	char *buf;
	size_t len, sz;

	s = sbuf_new_auto();
	ATF_REQUIRE(s != NULL);
	sbuf_cat(s, "abcdef");

	buf = sbufx_detach(s, &len, &sz);
	ATF_CHECK_MSG(strcmp(buf, "abcdef") == 0, "buf = %s", buf);
	ATF_CHECK_MSG(len == 6, "len = %zu", len);
	ATF_CHECK_MSG(sz == 16, "sz = %zu", sz);
	ATF_CHECK_MSG(s->s_buf != buf, "s->s_buf = %p", s->s_buf);
	ATF_CHECK_MSG(s->s_len == 0, "s->s_len = %zu", s->s_len);
	ATF_CHECK_MSG(s->s_size == 16, "s->s_size = %zu", s->s_size);

	sbuf_delete(s);
}

ATF_TC_WITHOUT_HEAD(sbufx_ltrim_test);
ATF_TC_BODY(sbufx_ltrim_test, tc)
{
	struct sbuf *s;
	int ret;

	s = sbuf_new_auto();
	ATF_REQUIRE(s != NULL);

	/* empty */
	ret = sbufx_ltrim(s);
	sbuf_finish(s);
	ATF_CHECK_MSG(ret == 0, "ret = %d", ret);
	ATF_CHECK_MSG(s->s_len == 0, "s->s_len = %zu", s->s_len);

	/* not trim */
	sbuf_cpy(s, "abc def");
	ret = sbufx_ltrim(s);
	sbuf_finish(s);
	ATF_CHECK_MSG(ret == 0, "ret = %d", ret);
	ATF_CHECK(strcmp(sbuf_data(s), "abc def") == 0);
	ATF_CHECK_MSG(s->s_len == 7, "s->s_len = %zu", s->s_len);

	/* trim */
	sbuf_cpy(s, "  abc def  ");
	ret = sbufx_ltrim(s);
	sbuf_finish(s);
	ATF_CHECK_MSG(ret == 0, "ret = %d", ret);
	ATF_CHECK(strcmp(sbuf_data(s), "abc def  ") == 0);
	ATF_CHECK_MSG(s->s_len == 9, "s->s_len = %zu", s->s_len);

	/* empty after trimmed */
	sbuf_cpy(s, "  ");
	ret = sbufx_ltrim(s);
	sbuf_finish(s);
	ATF_CHECK_MSG(ret == 0, "ret = %d", ret);
	ATF_CHECK(strcmp(sbuf_data(s), "") == 0);
	ATF_CHECK_MSG(s->s_len == 0, "s->s_len = %zu", s->s_len);

	sbuf_delete(s);
}

ATF_TC_WITHOUT_HEAD(sbufx_both_trim_test);
ATF_TC_BODY(sbufx_both_trim_test, tc)
{
	struct sbuf *s;
	int ret;

	s = sbuf_new_auto();
	ATF_REQUIRE(s != NULL);

	sbuf_cpy(s, "  abc def  ");
	ret = sbufx_both_trim(s);
	sbuf_finish(s);
	ATF_CHECK_MSG(ret == 0, "ret = %d", ret);
	ATF_CHECK(strcmp(sbuf_data(s), "abc def") == 0);
	ATF_CHECK_MSG(s->s_len == 7, "s->s_len = %zu", s->s_len);
}

ATF_TC_WITHOUT_HEAD(sbufx_starts_with_test);
ATF_TC_BODY(sbufx_starts_with_test, tc)
{
	struct sbuf *s;
	int ret;

	s = sbuf_new_auto();
	ATF_REQUIRE(s != NULL);

	ret = sbufx_starts_with(s, "abc");
	ATF_CHECK_MSG(ret == 0, "ret = %d", ret);

	sbuf_cpy(s, "abc");
	ret = sbufx_starts_with(s, "abc");
	ATF_CHECK_MSG(ret != 0, "ret = %d", ret);

	sbuf_cpy(s, "abcd");
	ret = sbufx_starts_with(s, "abc");
	ATF_CHECK_MSG(ret != 0, "ret = %d", ret);

	sbuf_cpy(s, "xabcd");
	ret = sbufx_starts_with(s, "abc");
	ATF_CHECK_MSG(ret == 0, "ret = %d", ret);

	sbuf_delete(s);
}

ATF_TC_WITHOUT_HEAD(sbufx_ends_with_test);
ATF_TC_BODY(sbufx_ends_with_test, tc)
{
	struct sbuf *s;
	int ret;

	s = sbuf_new_auto();
	ATF_REQUIRE(s != NULL);

	ret = sbufx_ends_with(s, "abc");
	ATF_CHECK_MSG(ret == 0, "ret = %d", ret);

	sbuf_cpy(s, "abc");
	ret = sbufx_ends_with(s, "abc");
	ATF_CHECK_MSG(ret != 0, "ret = %d", ret);

	sbuf_cpy(s, "abcd");
	ret = sbufx_ends_with(s, "bcd");
	ATF_CHECK_MSG(ret != 0, "ret = %d", ret);

	sbuf_cpy(s, "abcdx");
	ret = sbufx_ends_with(s, "bcd");
	ATF_CHECK_MSG(ret == 0, "ret = %d", ret);

	sbuf_delete(s);
}

ATF_TC_WITHOUT_HEAD(sbufx_contain_test);
ATF_TC_BODY(sbufx_contain_test, tc)
{
	struct sbuf *s;
	int ret;

	s = sbuf_new_auto();
	ATF_REQUIRE(s != NULL);

	ret = sbufx_contain(s, "abc");
	ATF_CHECK_MSG(ret == 0, "ret = %d", ret);

	sbuf_cpy(s, "abc");
	ret = sbufx_contain(s, "abc");
	ATF_CHECK_MSG(ret != 0, "ret = %d", ret);

	sbuf_cpy(s, "abcd");
	ret = sbufx_contain(s, "abc");
	ATF_CHECK_MSG(ret != 0, "ret = %d", ret);
	ret = sbufx_contain(s, "bcd");
	ATF_CHECK_MSG(ret != 0, "ret = %d", ret);
	ret = sbufx_contain(s, "bc");
	ATF_CHECK_MSG(ret != 0, "ret = %d", ret);
	ret = sbufx_contain(s, "xxx");
	ATF_CHECK_MSG(ret == 0, "ret = %d", ret);

	sbuf_delete(s);
}

ATF_TC_WITHOUT_HEAD(sbufx_fread_test);
ATF_TC_BODY(sbufx_fread_test, tc)
{
	struct sbuf *s;
	int ret;
	FILE *fp;
	static char str[] = "abcdefg\nhijklmn";

	s = sbuf_new_auto();
	ATF_REQUIRE(s != NULL);

	fp = fmemopen(str, strlen(str), "r");
	ret = sbufx_fread(s, fp);
	ATF_CHECK_MSG(ret == 0, "ret = %d", ret);
	ATF_CHECK_MSG(strcmp(sbuf_data(s), str) == 0, "sbuf_data = %s", sbuf_data(s));

	fclose(fp);
	sbuf_delete(s);
}

ATF_TP_ADD_TCS(tp)
{
	ATF_TP_ADD_TC(tp, sbufx_attach_test);
	ATF_TP_ADD_TC(tp, sbufx_detach_test);
	ATF_TP_ADD_TC(tp, sbufx_ltrim_test);
	ATF_TP_ADD_TC(tp, sbufx_both_trim_test);
	ATF_TP_ADD_TC(tp, sbufx_starts_with_test);
	ATF_TP_ADD_TC(tp, sbufx_ends_with_test);
	ATF_TP_ADD_TC(tp, sbufx_contain_test);
	ATF_TP_ADD_TC(tp, sbufx_fread_test);

	return (atf_no_error());
}
