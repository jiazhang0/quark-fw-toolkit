/*
 * Generic firmware parser APIs
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

#include <eee.h>
#include <err_status.h>
#include <cln_fw.h>
#include "internal.h"
#include "platform_data.h"
#include "capsule.h"
#include "buffer_stream.h"

err_status_t
cln_fw_parser_create(void *fw, unsigned long fw_len,
		     cln_fw_parser_t **out)
{
	cln_fw_parser_t *parser;

	if (!out)
		return CLN_FW_ERR_INVALID_PARAMETER;

	parser = eee_malloc(sizeof(*parser));
	if (!parser)
		return CLN_FW_ERR_OUT_OF_MEM;

	eee_memset(parser, 0, sizeof(*parser));
	bs_init(&parser->firmware, fw, fw_len);
	bs_init(&parser->mfh, NULL, 0);
	bs_init(&parser->pdata, NULL, 0);
	bs_init(&parser->pdata_header, NULL, 0);
	bcll_init(&parser->pdata_entry_list);

	*out = parser;

	return CLN_FW_ERR_NONE;
}

static void
free_all_cln_fw_pdata_entry(cln_fw_parser_t *parser)
{
	cln_fw_pdata_entry_t *entry, *tmp;

	bcll_for_each_link_safe(entry, tmp, &parser->pdata_entry_list, link) {
		bcll_del(&entry->link);
		eee_mfree(entry);
		--parser->nr_pdata_entry;
	}
}

void
cln_fw_parser_destroy(cln_fw_parser_t *parser)
{
	if (!parser)
		return;

	free_all_cln_fw_pdata_entry(parser);

	if (parser->pdata_entry)
		eee_mfree(parser->pdata_entry);

	if (bs_head(&parser->pdata_header))
		eee_mfree(bs_head(&parser->pdata_header));

	eee_mfree(parser);
}

static err_status_t
add_cln_fw_pdata_entry(cln_fw_parser_t *parser, void *pdata_entry_buf)
{
	cln_fw_pdata_entry_t *entry;

	entry = eee_malloc(sizeof(*entry));
	if (!entry)
		return CLN_FW_ERR_OUT_OF_MEM;

	bs_init(&entry->bs, pdata_entry_buf,
		platform_data_entry_size(pdata_entry_buf));
	bcll_add_tail(&parser->pdata_entry_list, &entry->link);
	++parser->nr_pdata_entry;

	return CLN_FW_ERR_NONE;
}

err_status_t
cln_fw_parser_parse(cln_fw_parser_t *parser)
{
	buffer_stream_t *fw = &parser->firmware;
	void *mfh, *pdata;
	unsigned long mfh_len, pdata_len;
	err_status_t err;

	err = bs_get_at(fw, &mfh, mfh_header_size(), mfh_offset());
	if (is_err_status(err)) {
		err(T("The length of firmware is not expected for ")
		    T("searching MFH\n"));
		return err;
	}

	mfh_len = bs_remain(fw);
	err = mfh_probe(mfh, &mfh_len);
	if (!is_err_status(err) && bs_empty(&parser->mfh))
		bs_init(&parser->mfh, mfh, mfh_len);

	pdata_len = platform_data_max_size();
	err = bs_get_at(fw, &pdata, pdata_len, platform_data_offset());
	if (is_err_status(err)) {
		err(T("The length of firmware is not expected for ")
		    T("searching platform data\n"));
		return err;
	}

	err = platform_data_probe(pdata, &pdata_len);
	if (!is_err_status(err) && bs_empty(&parser->pdata)) {
		void *pdata_header_buf, *pdata_entry_buf, *p;
		unsigned long i, nr_pdata_entry;

		err = platform_data_parse(pdata, pdata_len,
					  &pdata_header_buf,
					  &pdata_entry_buf,
					  &nr_pdata_entry);
		if (is_err_status(err))
			return err;

		p = pdata_entry_buf;
		for (i = 0; i < nr_pdata_entry; ++i) {
			err = add_cln_fw_pdata_entry(parser, p);
			if (is_err_status(err)) {
				free_all_cln_fw_pdata_entry(parser);
				return err;
			}
			p += platform_data_entry_size(p);
		}

		parser->pdata_entry = pdata_entry_buf;
		parser->nr_pdata_entry = nr_pdata_entry;
		bs_init(&parser->pdata_header, pdata_header_buf,
			platform_data_header_size());

		bs_init(&parser->pdata, pdata, pdata_len);
	}

	return CLN_FW_ERR_NONE;
}

err_status_t
cln_fw_parser_embed_key(cln_fw_parser_t *parser, cln_fw_sb_key_t key,
			void *in, unsigned long in_len)
{
	buffer_stream_t *pdata;
	uint16_t id;
	void *pdata_entry;
	unsigned long pdata_entry_len;
	err_status_t err;

	if (key == CLN_FW_SB_KEY_PK)
		id = PDATA_ID_PK;
	else
		id = PDATA_ID_CERT;

	pdata = &parser->pdata;
	err = platform_data_search_entry(bs_head(pdata), bs_size(pdata), id);
	if (!is_err_status(err)) {
		cln_fw_pdata_entry_t *entry, *tmp;

		dbg(T("Updating platform entry ID %d ...\n"), id);
		bcll_for_each_link_safe(entry, tmp,
				&parser->pdata_entry_list, link) {
			uint16_t pdata_entry_id;
			uint32_t header;
			void *pdata_entry = bs_head(&entry->bs);

			pdata_entry_id = platform_data_entry_id(pdata_entry);
			if (pdata_entry_id != id)
				continue;

			if (key == CLN_FW_SB_KEY_PK)
				break;

			header = platform_data_cert_header(pdata_entry);
			if (header == PDATA_KEK_CERT_HEADER
					&& key == CLN_FW_SB_KEY_KEK)
				break;
			else if (header == PDATA_DB_CERT_HEADER
					&& key == CLN_FW_SB_KEY_DB)
				break;
		}
		if (&entry->link == &parser->pdata_entry_list)
			return CLN_FW_ERR_PDATA_ENTRY_NOT_FOUND;

		err = platform_data_update_entry(bs_head(&entry->bs),
						 bs_size(&entry->bs), id,
						 NULL, NULL, in, in_len,
						 &pdata_entry,
						 &pdata_entry_len);
		if (is_err_status(err))
			return err;

		bs_init(&entry->bs, pdata_entry, pdata_entry_len);
	} else if (err == CLN_FW_ERR_PDATA_ENTRY_NOT_FOUND) {
		const char desc[][10] = {
			"pk",
			"kek cert",
			"db cert",
			"dbx cert",
		};

		err = platform_data_create_entry(id, 0, desc[key], in, in_len,
						 &pdata_entry,
						 &pdata_entry_len);
		if (is_err_status(err))
			return err;

		err = add_cln_fw_pdata_entry(parser, pdata_entry);
		if (is_err_status(err)) {
			eee_mfree(pdata_entry);
			return err;
		}
	}

	return CLN_FW_ERR_NONE;
}

err_status_t
cln_fw_parser_flush(cln_fw_parser_t *parser, void *fw_buf,
		    unsigned long fw_buf_len)
{
	buffer_stream_t fw;
	cln_fw_pdata_entry_t *entry;
	void *pdata, *pdata_entry;
	unsigned long total_entry_len;
	err_status_t err;

	bs_init(&fw, fw_buf, fw_buf_len);

	err = bs_get_at(&fw, (void **)&pdata, platform_data_max_size(),
			platform_data_offset());
	if (is_err_status(err)) {
		err(T("The length of firmware is not expected for ")
		    T("searching platform data\n"));
		return err;
	}

	bs_seek(&fw, platform_data_header_size());
	bs_get(&fw, (void **)&pdata_entry, 0);

	total_entry_len = 0;
	bcll_for_each_link(entry, &parser->pdata_entry_list, link) {
		bs_post_put(&fw, bs_head(&entry->bs), bs_size(&entry->bs));
		total_entry_len += bs_size(&entry->bs);
	}

	platform_data_update_header(pdata, pdata_entry, total_entry_len);

	bs_put_at(&fw, pdata, platform_data_header_size(),
		  platform_data_offset());

	if (cln_fw_verbose()) {
		dbg(T("Showing platform data after embedding the key ...\n"));
		platform_data_show(pdata, platform_data_size(pdata));
	}

	return CLN_FW_ERR_NONE;
}

err_status_t
cln_fw_parser_generate_capsule(cln_fw_parser_t *parser, int bios_only,
			       void **out, unsigned long *out_len)
{
	buffer_stream_t *fw = &parser->firmware;
	buffer_stream_t cap;
	void *cap_header, *update_entry;
	unsigned long cap_header_len, update_entry_len, payload_len, cap_len;
	uint32_t addr;
	err_status_t err;

	if (!bios_only) {
		addr = 0xFF800000;
		payload_len = bs_size(fw);
		if (payload_len != FIRMWARE_SIZE) {
			err(T("The firmware size is expected length\n"));
			return CLN_FW_ERR_INVALID_PARAMETER;
		}
	} else {
		addr = 0xFFD00000;
		if (bs_size(fw) < BIOS_REGION_SIZE) {
			err(T("The BIOS part in firmware is not big enough\n"));
			return CLN_FW_ERR_INVALID_PARAMETER;
		}
		payload_len = BIOS_REGION_SIZE;
	}

	payload_len = (payload_len + (BLOCK_SIZE - 1)) & ~(BLOCK_SIZE - 1);
	err = capsule_create_header(payload_len, &cap_header,
				    &cap_header_len);
	if (is_err_status(err))
		return err;

	err = capsule_create_update_entry(addr, payload_len,
					  &update_entry, &update_entry_len);
	if (is_err_status(err))
		goto err_create_update_entry;

	bs_init(&cap, NULL, 0);

	cap_len = cap_header_len + update_entry_len + payload_len;
	err = bs_reserve(&cap, cap_len);
	if (is_err_status(err)) {
		err(T("Failed to reserve the memory for capsule\n"));
		goto err_reserve;
	}

	bs_post_put(&cap, cap_header, cap_header_len);
	bs_post_put(&cap, update_entry, update_entry_len);
	bs_put(&cap, bs_head(fw) + bs_size(fw) - payload_len, payload_len);

	if (out)
		*out = bs_head(&cap);

	if (out_len)
		*out_len = bs_size(&cap);

err_reserve:
	eee_mfree(update_entry);

err_create_update_entry:
	eee_mfree(cap_header);

	return err;
}