/*
 * Handle abstraction
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

#include <err_status.h>
#include <cln_fw.h>
#include <eee.h>
#include "internal.h"
#include "bcll.h"

err_status_t
cln_fw_handle_open(cln_fw_handle_t *handle, void *fw, unsigned long fw_len)
{
	cln_fw_parser_t *parser;
	err_status_t err;

	if (!handle || !fw || !fw_len)
		return CLN_FW_ERR_INVALID_PARAMETER;

	err = cln_fw_parser_create(fw, fw_len, &parser);
	if (is_err_status(err))
		return err;

	err = cln_fw_parser_parse(parser);
	if (is_err_status(err)) {
		eee_mfree(parser);
		return err;
	}

	*handle = (cln_fw_handle_t)parser;

	return CLN_FW_ERR_NONE;
}

void
cln_fw_handle_close(cln_fw_handle_t handle)
{
	if (!handle)
		return;

	cln_fw_parser_destroy((cln_fw_parser_t *)handle);
}

void
cln_fw_handle_show_all(cln_fw_handle_t handle)
{
	cln_fw_parser_t *parser;
	buffer_stream_t *bs;

	if (!handle)
		return;

	parser = (cln_fw_parser_t *)handle;
	bs = &parser->mfh;
	if (!bs_empty(bs))
		mfh_show(bs_head(bs), bs_size(bs));

	bs = &parser->pdata;
	if (!bs_empty(bs))
		platform_data_show(bs_head(bs), bs_size(bs));
}

err_status_t
cln_fw_handle_embed_key(cln_fw_handle_t handle, cln_fw_sb_key_t key,
			void *in, unsigned long in_len)
{
	cln_fw_parser_t *parser;
	buffer_stream_t *pdata;
	err_status_t err;
	const tchar_t *key_name[] = {
		T("PK"), T("KEK"), T("DB"), T("DBX")
	};

	if (!handle || !in || !in_len || key >= CLN_FW_SB_KEY_MAX)
		return CLN_FW_ERR_INVALID_PARAMETER;

	dbg(T("Embedding %s ...\n"), key_name[key]);

	parser = (cln_fw_parser_t *)handle;
	pdata = &parser->pdata;
	if (bs_empty(pdata)) {
		/* Try to parse the firmware if it was not parsed */
		err = cln_fw_parser_parse(parser);
		if (is_err_status(err))
			return err;

		if (bs_empty(pdata)) {
			err(T("Not found platform data in firmware for ")
			    T("embedding %s\n"), key_name[key]);
			return CLN_FW_ERR_NO_PDATA;
		}
	}

	if (cln_fw_verbose()) {
		dbg(T("Showing platform data before embedding %s ...\n"),
		    key_name[key]);
		platform_data_show(bs_head(pdata), bs_size(pdata));
	}

	err = cln_fw_parser_embed_key(parser, key, in, in_len);
	if (is_err_status(err))
		return err;

#if 0
	if (cln_fw_verbose()) {
		dbg(T("Showing platform data after embedding %s ...\n"),
		    key_name[key]);
		platform_data_show(bs_head(pdata), bs_size(pdata));
	}
#endif

	return CLN_FW_ERR_NONE;
}

err_status_t
cln_fw_handle_flush(cln_fw_handle_t handle, void **out,
		    unsigned long *out_len)
{
	cln_fw_parser_t *parser;
	unsigned long fw_buf_len;
	void *fw_buf;
	err_status_t err;

	if (!handle || !out || !out_len)
		return CLN_FW_ERR_INVALID_PARAMETER;

	parser = (cln_fw_parser_t *)handle;
	fw_buf_len = bs_size(&parser->firmware);
	fw_buf = eee_malloc(fw_buf_len);
	if (!fw_buf)
		return CLN_FW_ERR_OUT_OF_MEM;

	eee_memcpy(fw_buf, bs_head(&parser->firmware), fw_buf_len);

	err = cln_fw_parser_flush(parser, fw_buf, fw_buf_len);
	if (is_err_status(err)) {
		eee_mfree(fw_buf);
		return err;
	}

	*out = fw_buf;
	*out_len = fw_buf_len;

	return CLN_FW_ERR_NONE;
}