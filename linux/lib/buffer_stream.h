/*
 * Byte stream buffer abstraction
 *
 * Copyright (c) 2015, Lans Zhang <jia.zhang@windriver.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __BUFFER_STREAM_H__
#define __BUFFER_STREAM_H__

#include <err_status.h>

struct __buffer_stream;

struct __buffer_stream {
	void *buf;
	unsigned long buf_len;
	unsigned long current;
	void *in;
	unsigned long in_len;
	void *out;
	unsigned long out_len;
	struct __buffer_stream *parent;
};

typedef struct __buffer_stream			buffer_stream_t;

#ifndef BYTE_STREAM_ERROR_BASE
#define BYTE_STREAM_ERROR_BASE			0
#endif

#define BYTE_STREAM_ERR(err)	\
	(BYTE_STREAM_ERROR_BASE + (err))
#define BYTE_STREAM_ERR_NONE			BYTE_STREAM_ERR(0)
#define BYTE_STREAM_ERR_INVALID_PARAMETER	BYTE_STREAM_ERR(1)
#define BYTE_STREAM_ERR_OUT_OF_RANGE		BYTE_STREAM_ERR(2)
#define BYTE_STREAM_ERR_OUT_OF_MEM		BYTE_STREAM_ERR(3)

err_status_t
bs_alloc(buffer_stream_t **bs);

void
bs_init(buffer_stream_t *bs, void *buf, unsigned long buf_len);

unsigned long
bs_tell(buffer_stream_t *bs);

unsigned long
bs_remain(buffer_stream_t *bs);

unsigned long
bs_size(buffer_stream_t *bs);

unsigned long
bs_empty(buffer_stream_t *bs);

err_status_t
bs_seek(buffer_stream_t *bs, long offset);

err_status_t
bs_seek_at(buffer_stream_t *bs, long offset);

void *
bs_head(buffer_stream_t *bs);

err_status_t
bs_reserve_at(buffer_stream_t *bs, unsigned long ext_len, long offset);

err_status_t
bs_reserve(buffer_stream_t *bs, unsigned long ext_len);

err_status_t
bs_shrink_at(buffer_stream_t *bs, unsigned long len, long offset);

err_status_t
bs_post_get_at(buffer_stream_t *bs, void **in, unsigned long in_len,
	       long offset);

err_status_t
bs_get_at(buffer_stream_t *bs, void **in, unsigned long in_len,
	  long offset);

err_status_t
bs_post_get(buffer_stream_t *bs, void **in, unsigned long in_len);

err_status_t
bs_get(buffer_stream_t *bs, void **in, unsigned long in_len);

err_status_t
bs_post_put_at(buffer_stream_t *bs, void *out, unsigned long out_len,
	       long offset);

err_status_t
bs_put_at(buffer_stream_t *bs, void *out, unsigned long out_len,
	  long offset);

err_status_t
bs_post_put(buffer_stream_t *bs, void *out, unsigned long out_len);

err_status_t
bs_put(buffer_stream_t *bs, void *out, unsigned long out_len);

#endif	/* __BUFFER_STREAM_H__ */