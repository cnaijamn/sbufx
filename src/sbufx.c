/*-
 * BSD 3-Clause License
 *
 * Copyright (c) 2020, 2021 Chez Naijamn
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

#include <ctype.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "sbufx.h"

#define	KASSERT(e, m)
#define	SBMALLOC(size)		calloc(1, size)
#define	SBFREE(buf)		free(buf)

/*
 * Predicates
 */
#define	SBUF_ISDYNAMIC(s)w	((s)->s_flags & SBUF_DYNAMIC)
#define	SBUF_ISDYNSTRUCT(s)	((s)->s_flags & SBUF_DYNSTRUCT)
#define	SBUF_ISFINISHED(s)	((s)->s_flags & SBUF_FINISHED)
#define	SBUF_HASROOM(s)		((s)->s_len < (s)->s_size - 1)
#define	SBUF_FREESPACE(s)	((s)->s_size - ((s)->s_len + 1))
#define	SBUF_CANEXTEND(s)	((s)->s_flags & SBUF_AUTOEXTEND)
#define	SBUF_ISSECTION(s)	((s)->s_flags & SBUF_INSECTION)
#define	SBUF_NULINCLUDED(s)	((s)->s_flags & SBUF_INCLUDENUL)
#define	SBUF_ISDRAINTOEOR(s)	((s)->s_flags & SBUF_DRAINTOEOR)
#define	SBUF_DODRAINTOEOR(s)	(SBUF_ISSECTION(s) && SBUF_ISDRAINTOEOR(s))

/*
 * Debugging support
 */
#define	assert_sbuf_integrity(s) do { } while (0)
#define	assert_sbuf_state(s, i)	 do { } while (0)

int
sbufx_attach(struct sbuf *s, char *buf, size_t len, size_t sz)
{
	if (buf == NULL || len > sz || (s->s_flags & SBUF_AUTOEXTEND) == 0)
		return (-1);

	sbuf_clear(s);
	SBFREE(s->s_buf);

	s->s_buf = buf;
	s->s_len = len;
	s->s_size = sz;
	return (0);
}

char *
sbufx_detach(struct sbuf *s, size_t *len, size_t *sz)
{
	char *buf, *newbuf;

	if ((s->s_flags & SBUF_AUTOEXTEND) == 0)
		return (NULL);
	if (sbuf_finish(s) != 0)
		return (NULL);

	newbuf = SBMALLOC(0);
	if (newbuf == NULL)
		return (NULL);

	buf = s->s_buf;
	*len = s->s_len;
	*sz = s->s_size;
	s->s_buf = newbuf;
	sbuf_clear(s);
	return (buf);
}

int
sbufx_ltrim(struct sbuf *s)
{
	ssize_t off;

	assert_sbuf_integrity(s);
	assert_sbuf_state(s, 0);
	KASSERT(s->s_drain_func == NULL,
	    ("%s makes no sense on sbuf %p with drain", __func__, s));
	KASSERT(!SBUF_ISSECTION(s),
	    ("attempt to trim left when in a section"));

	if (s->s_error != 0)
		return (-1);
	if (SBUF_ISSECTION(s))
		return (-1);
	if (s->s_len == 0)
		return (0);

	off = 0;
	while (off < s->s_len && isspace(s->s_buf[off]))
		++off;

	if (off > 0) {
		s->s_len -= off;
		(void)memmove(s->s_buf, s->s_buf + off, s->s_len);
	}

	return (0);
}

int
sbufx_both_trim(struct sbuf *s)
{
	if (sbuf_trim(s) == 0 && sbufx_ltrim(s) == 0)
		return (0);
	return (-1);
}

int
sbufx_starts_with(struct sbuf *s, const char *str)
{
	size_t len;

	len = strlen(str);
	return (strncmp(s->s_buf, str, len) == 0 ? 1 : 0);
}

int
sbufx_ends_with(struct sbuf *s, const char *str)
{
	size_t len;

	len = strlen(str);
	if ((size_t)s->s_len < len)
		return (0);
	return (strncmp(&s->s_buf[s->s_len - len], str, len) == 0 ? 1 : 0);
}

int
sbufx_contain(struct sbuf *s, const char *str)
{
	return (strnstr(s->s_buf, str, s->s_len) != NULL ? 1 : 0);
}

int
sbufx_fread(struct sbuf *s, FILE *in)
{
	char buf[1024];
	size_t n;

	sbuf_clear(s);
	while ((n = fread(buf, 1, 1024, in)) > 0)
		sbuf_bcat(s, buf, n);
	sbuf_finish(s);
	return (ferror(in) ? -1 : 0);
}
