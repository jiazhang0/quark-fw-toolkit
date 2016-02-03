/*
 * Byte stream buffer abstraction
 *
 * Copyright (c) 2015-2016 Wind River Systems, Inc.
 *
 * See "LICENSE" for license terms.
 *
 * Author: Lans Zhang <jia.zhang@windriver.com>
 */

#include <eee.h>
#include <err_status.h>
#include "buffer_stream.h"

static long
bs_abs_offset(buffer_stream_t *bs, long offset)
{
	if (offset < 0)
		return bs->buf_len + offset;

	return offset;
}

void
bs_init(buffer_stream_t *bs, void *buf, unsigned long buf_len)
{
	bs->buf = buf;
	bs->buf_len = buf_len;
	bs->current = 0;
}

unsigned long
bs_tell(buffer_stream_t *bs)
{
	return bs->current;
}

unsigned long
bs_size(buffer_stream_t *bs)
{
	return bs->buf_len;
}

unsigned long
bs_empty(buffer_stream_t *bs)
{
	return !bs_size(bs);
}

unsigned long
bs_remain(buffer_stream_t *bs)
{
	return bs->buf_len - bs->current;
}

void *
bs_head(buffer_stream_t *bs)
{
	return bs->buf;
}

err_status_t
bs_seek(buffer_stream_t *bs, long offset)
{
	long new_offset = (long)bs->current + offset;

	if (new_offset < 0 || new_offset >= (long)bs->buf_len)
		return BYTE_STREAM_ERR_INVALID_PARAMETER;

	bs->current += offset;

	return BYTE_STREAM_ERR_NONE;
}

err_status_t
bs_seek_at(buffer_stream_t *bs, long offset)
{
	offset = bs_abs_offset(bs, offset);
	if (offset < 0 || offset >= bs->buf_len)
		return BYTE_STREAM_ERR_OUT_OF_RANGE;

	bs->current = offset;

	return BYTE_STREAM_ERR_NONE;
}

err_status_t
bs_reserve_at(buffer_stream_t *bs, unsigned long ext_len, long offset)
{
	unsigned long out_buflen = offset + ext_len;

	if (out_buflen > bs->buf_len) {
		void *out_buf;

		out_buf = eee_malloc(out_buflen);
		// out_buf = eee_mrealloc_aligned(bs->buf, bs->buf_len, &out_buflen);
		if (!out_buf) {
			err(T("Failed to extend out buffer\n"));
			return BYTE_STREAM_ERR_OUT_OF_MEM;
		}

		eee_memcpy(out_buf, bs->buf, bs->buf_len);

		bs->buf = out_buf;
	}

	bs->buf_len = out_buflen;

	return bs_seek_at(bs, offset);
}

err_status_t
bs_reserve(buffer_stream_t *bs, unsigned long ext_len)
{
	return bs_reserve_at(bs, ext_len, bs->current);
}

static err_status_t
__bs_get_at(buffer_stream_t *bs, void **in, unsigned long in_len,
	    int post, long offset)
{
	offset = bs_abs_offset(bs, offset);
	if (offset < 0 || offset + in_len > bs->buf_len)
		return BYTE_STREAM_ERR_OUT_OF_RANGE;

	if (in)
		*in = bs->buf + offset;

	bs->current = offset;

	if (post)
		bs->current += in_len;

	return BYTE_STREAM_ERR_NONE;
}

err_status_t
bs_post_get_at(buffer_stream_t *bs, void **in, unsigned long in_len,
	       long offset)
{
	return __bs_get_at(bs, in, in_len, 1, offset);
}

err_status_t
bs_get_at(buffer_stream_t *bs, void **in, unsigned long in_len,
	  long offset)
{
	return __bs_get_at(bs, in, in_len, 0, offset);
}

err_status_t
bs_post_get(buffer_stream_t *bs, void **in, unsigned long in_len)
{
	return __bs_get_at(bs, in, in_len, 1, bs->current);
}

err_status_t
bs_get(buffer_stream_t *bs, void **in, unsigned long in_len)
{
	return __bs_get_at(bs, in, in_len, 0, bs->current);
}

static err_status_t
__bs_put_at(buffer_stream_t *bs, void *out, unsigned long out_len,
	    int post, long offset)
{
	offset = bs_abs_offset(bs, offset);
	if (offset < 0 || offset + out_len > bs->buf_len)
		return BYTE_STREAM_ERR_OUT_OF_RANGE;

	if (out)
		eee_memcpy(bs->buf + offset, out, out_len);

	bs->current = offset;

	if (post)
		bs->current += out_len;

	return BYTE_STREAM_ERR_NONE;
}

err_status_t
bs_post_put_at(buffer_stream_t *bs, void *out, unsigned long out_len,
	       long offset)
{
	return __bs_put_at(bs, out, out_len, 1, offset);
}

err_status_t
bs_put_at(buffer_stream_t *bs, void *out, unsigned long out_len,
	  long offset)
{
	return __bs_put_at(bs, out, out_len, 0, offset);
}

err_status_t
bs_post_put(buffer_stream_t *bs, void *out, unsigned long out_len)
{
	return __bs_put_at(bs, out, out_len, 1, bs->current);
}

err_status_t
bs_put(buffer_stream_t *bs, void *out, unsigned long out_len)
{
	return __bs_put_at(bs, out, out_len, 0, bs->current);
}