/*
 * Platform Data Structure parser
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
#include "platform_data.h"
#include "internal.h"
#include "buffer_stream.h"
#include "bcll.h"

unsigned long
platform_data_header_size(void)
{
	return sizeof(platform_data_header_t);
}

long
platform_data_offset(void)
{
	return PLATFORM_DATA_OFFSET;
}

unsigned long
platform_data_max_size(void)
{
	return PLATFORM_DATA_MAX_SIZE;
}

unsigned long
platform_data_all_entry_size(void *pdata_buf)
{
	platform_data_header_t *pdata_header = pdata_buf;
	return pdata_header->length;
}

unsigned long
platform_data_size(void *pdata_buf)
{
	return platform_data_all_entry_size(pdata_buf)
	       + platform_data_header_size();
}

unsigned long
platform_data_entry_size(void *pdata_entry_buf)
{
	platform_data_entry_t *pdata_entry = pdata_entry_buf;
	return pdata_entry->length + sizeof(*pdata_entry);
}

uint16_t
platform_data_entry_id(void *pdata_entry_buf)
{
	platform_data_entry_t *pdata_entry = pdata_entry_buf;
	return pdata_entry->id;
}

void
platform_data_update_header(void *pdata, void *pdata_entry,
			    unsigned long pdata_entry_len)
{
	platform_data_header_t *pdata_header = pdata;
	pdata_header->length = pdata_entry_len;
	pdata_header->crc32 = crc32(pdata_entry, pdata_entry_len);
}

uint32_t
platform_data_cert_header(void *pdata_entry_buf)
{
	platform_data_entry_t *pdata_entry = pdata_entry_buf;
	return *(uint32_t *)(pdata_entry + 1);
}

err_status_t
platform_data_probe(void *pdata_buf, unsigned long *pdata_buf_len)
{
	buffer_stream_t bs;
	platform_data_header_t *pdata;
	unsigned long nr_pdata_entry;
	uint16_t total_entry_len;
	err_status_t err;

	bs_init(&bs, pdata_buf, *pdata_buf_len);

	err = bs_post_get(&bs, (void **)&pdata, sizeof(*pdata));
	if (is_err_status(err)) {
		err(T("The length of flash image is not expected for ")
		    T("searching platform data\n"));
		return err;
	}

	if (pdata->magic != PLATFORM_DATA_MAGIC) {
		err(T("Invalid platform data magic: 0x%x\n"), pdata->magic);
		return CLN_FW_ERR_INVALID_PDATA;
	}

	err = CLN_FW_ERR_NONE;
	for (nr_pdata_entry = 0, total_entry_len = 0;
			total_entry_len < pdata->length; ++nr_pdata_entry) {
		platform_data_entry_t *pdata_entry;
		uint8_t *pdata_entry_data;

		err = bs_post_get(&bs, (void **)&pdata_entry,
				  sizeof(*pdata_entry));
		if (is_err_status(err)) {
			err(T("Invalid length of entry %ld\n"),
			    nr_pdata_entry);
			return CLN_FW_ERR_INVALID_PDATA;
		}

		if (pdata_entry->id == PDATA_ID_UNDEF
				|| pdata_entry->id >= PDATA_ID_MAX) {
			err(T("Invalid id of entry %ld: 0x%x\n"),
			    nr_pdata_entry, pdata_entry->id);
			err = CLN_FW_ERR_INVALID_PDATA;
		}

		err = bs_post_get(&bs, (void **)&pdata_entry_data,
				  pdata_entry->length);
		if (is_err_status(err)) {
			err(T("Invalid length of entry %ld: 0x%x\n"),
			    nr_pdata_entry, pdata_entry->length);
			return CLN_FW_ERR_INVALID_PDATA;
		}

		total_entry_len += sizeof(*pdata_entry) + pdata_entry->length;
	}
	if (is_err_status(err))
		return CLN_FW_ERR_INVALID_PDATA;

	if (total_entry_len != pdata->length) {
		err(T("Invalid platform data length: 0x%x\n"), pdata->length);
		return CLN_FW_ERR_INVALID_PDATA;
	}

	if (crc32((uint8_t *)(pdata + 1), pdata->length) != pdata->crc32) {
		err(T("Invalid platform data CRC32: 0x%x\n"), pdata->crc32);
		return CLN_FW_ERR_INVALID_PDATA;
	}

	*pdata_buf_len = bs_tell(&bs);

	return CLN_FW_ERR_NONE;
}

void
__platform_data_show(void *pdata_buf, unsigned long pdata_buf_len)
{
	platform_data_header_t *pdata;
	platform_data_entry_t *pdata_entry;
	unsigned long nr_pdata_entry;
	uint16_t total_entry_len;

	pdata = (platform_data_header_t *)pdata_buf;

	info_cont(T("Platform Data Header:\n"));
	info_cont(T("  Magic: 0x%x\n"), pdata->magic);
	info_cont(T("  Length: 0x%x\n"), pdata->length);
	info_cont(T("  CRC32: 0x%x\n"), pdata->crc32);

	pdata_entry = (platform_data_entry_t *)(pdata + 1);
	for (nr_pdata_entry = 0, total_entry_len = 0;
			total_entry_len < pdata->length; ++nr_pdata_entry) {
		char desc[sizeof(pdata_entry->desc) + 1];
		uint8_t *pdata_entry_data = (uint8_t *)(pdata_entry + 1);
		uint16_t data_len_shown = 8;
		uint16_t i;

		memcpy(desc, pdata_entry->desc, sizeof(pdata_entry->desc));
		desc[sizeof(pdata_entry->desc)] = 0;

		info_cont(T("  Entry %ld:\n"), nr_pdata_entry);
		info_cont(T("    ID: 0x%x\n"), pdata_entry->id);
		info_cont(T("    Length: 0x%x\n"), pdata_entry->length);
		info_cont(T("    Description: %s\n"), desc);
		info_cont(T("    Version: 0x%x\n"), pdata_entry->version);

		if (data_len_shown > pdata_entry->length) {
			data_len_shown = pdata_entry->length;
			info_cont(T("    Data: "));
		} else
			info_cont(T("    Data (first 8 bytes): "));

		for (i = 0; i < data_len_shown; i++)
			info_cont(T("0x%02x "), pdata_entry_data[i]);
		info_cont(T("\n"));

		total_entry_len += sizeof(*pdata_entry) + pdata_entry->length;
		pdata_entry = (platform_data_entry_t *)(pdata_entry_data
			      + pdata_entry->length);
	}
}

err_status_t
platform_data_show(void *pdata_buf, unsigned long pdata_buf_len)
{
	err_status_t err;

	err = platform_data_probe(pdata_buf, &pdata_buf_len);
	if (is_err_status(err))
		return err;

	__platform_data_show(pdata_buf, pdata_buf_len);

	return CLN_FW_ERR_NONE;
}

static unsigned long
count_pdata_entry(platform_data_header_t *pdata)
{
	platform_data_entry_t *pdata_entry;
	unsigned long nr_pdata_entry;
	uint16_t total_entry_len;

	pdata_entry = (platform_data_entry_t *)(pdata + 1);
	for (nr_pdata_entry = 0, total_entry_len = 0;
			total_entry_len < pdata->length; ++nr_pdata_entry) {
		uint8_t *pdata_entry_data = (uint8_t *)(pdata_entry + 1);

		total_entry_len += sizeof(*pdata_entry) + pdata_entry->length;
		pdata_entry = (platform_data_entry_t *)(pdata_entry_data
			      + pdata_entry->length);
	}

	return nr_pdata_entry;
}

err_status_t
platform_data_parse(void *pdata_buf, unsigned long pdata_buf_len,
		    void **out_pdata_header_buf, void **out_pdata_entry_buf,
		    unsigned long *out_nr_pdata_entry)
{
	platform_data_header_t *pdata;
	err_status_t err;

	err = platform_data_probe(pdata_buf, &pdata_buf_len);
	if (is_err_status(err))
		return err;

	pdata = (platform_data_header_t *)pdata_buf;

	if (out_pdata_header_buf) {
		platform_data_header_t *out_pdata;

		out_pdata = eee_malloc(sizeof(*out_pdata));
		if (!out_pdata)
			return CLN_FW_ERR_OUT_OF_MEM;

		eee_memcpy(out_pdata, pdata, sizeof(*out_pdata));
		*out_pdata_header_buf = out_pdata;
	}

	if (out_pdata_entry_buf) {
		platform_data_entry_t *pdata_entry, *out_pdata_entry;

		pdata_entry = (platform_data_entry_t *)(pdata + 1);
		out_pdata_entry = eee_malloc(pdata->length);
		if (!out_pdata_entry) {
			if (out_pdata_header_buf)
				eee_mfree(*out_pdata_header_buf);
			return CLN_FW_ERR_OUT_OF_MEM;
		}

		eee_memcpy(out_pdata_entry, pdata_entry, pdata->length);
		*out_pdata_entry_buf = out_pdata_entry;
	}

	if (out_nr_pdata_entry)
		*out_nr_pdata_entry = count_pdata_entry(pdata);

	return CLN_FW_ERR_NONE;
}

static int
search_pdata_entry(platform_data_header_t *pdata, uint16_t id,
		   platform_data_entry_t **out)
{
	platform_data_entry_t *pdata_entry;
	unsigned long total_entry_len;

	pdata_entry = (platform_data_entry_t *)(pdata + 1);
	for (total_entry_len = 0; total_entry_len < pdata->length;) {
		uint8_t *entry_data;

		if (id == pdata_entry->id) {
			if (out)
				*out = pdata_entry;
			return 1;
		}
		total_entry_len += sizeof(*pdata_entry) + pdata_entry->length;
		entry_data = (uint8_t *)(pdata_entry + 1);
		pdata_entry = (platform_data_entry_t *)(entry_data
			      + pdata_entry->length);
	}

	return 0;
}

err_status_t
platform_data_search_entry(void *pdata_buf, unsigned long pdata_buf_len,
			   uint16_t id)
{
	platform_data_header_t *pdata;
	platform_data_entry_t *pdata_entry;
	err_status_t err;

	err = platform_data_probe(pdata_buf, &pdata_buf_len);
	if (is_err_status(err))
		return err;

	pdata = (platform_data_header_t *)pdata_buf;
	if (!search_pdata_entry(pdata, id, &pdata_entry))
		return CLN_FW_ERR_PDATA_ENTRY_NOT_FOUND;

	return CLN_FW_ERR_NONE;
}

static void
init_pdata_entry(platform_data_entry_t *entry, uint16_t id, uint16_t version,
		 const char desc[10], uint8_t *data, uint16_t data_len)
{
	entry->id = id;
	entry->version = version;
	eee_memcpy(entry->desc, desc, sizeof(entry->desc));
	entry->length = data_len;
	eee_memcpy(entry->data, data, data_len);
}

err_status_t
platform_data_create_entry(uint16_t id, uint16_t version, const char desc[10],
			   uint8_t *data, uint16_t data_len, void **out,
			   unsigned long *out_len)
{
	platform_data_entry_t *pdata_entry;

	if (!out && !out_len)
		return CLN_FW_ERR_INVALID_PARAMETER;

	pdata_entry = eee_malloc(sizeof(*pdata_entry) + data_len);
	if (!pdata_entry)
		return CLN_FW_ERR_OUT_OF_MEM;

	init_pdata_entry(pdata_entry, id, version, desc, data, data_len);

	if (out)
		*out = pdata_entry;

	if (out_len)
		*out_len = sizeof(*pdata_entry) + data_len;

	return CLN_FW_ERR_NONE;
}

err_status_t
platform_data_update_entry(void *pdata_entry_buf,
			   unsigned long pdata_entry_buf_len,
			   uint16_t id, uint16_t *version,
			   const char desc[10], uint8_t *data,
			   uint16_t data_len, void **out,
			   unsigned long *out_len)
{
	platform_data_entry_t *pdata_entry;
	buffer_stream_t bs;
	unsigned long pdata_entry_len;
	err_status_t err;

	if (!out && !out_len)
		return CLN_FW_ERR_INVALID_PARAMETER;

	if (pdata_entry_buf_len !=
			platform_data_entry_size((void *)pdata_entry_buf))
		return CLN_FW_ERR_INVALID_PARAMETER;

	bs_init(&bs, pdata_entry_buf, pdata_entry_buf_len);

	err = bs_reserve_at(&bs, data_len, sizeof(*pdata_entry));
	if (is_err_status(err))
		return err;

	pdata_entry_len = sizeof(*pdata_entry) + data_len;
	bs_get_at(&bs, (void **)&pdata_entry, pdata_entry_len, 0);

	if (version)
		pdata_entry->version = *version;
	if (desc)
		eee_memcpy(pdata_entry->desc, desc,
			   sizeof(pdata_entry->desc));
	if (data) {
		eee_memcpy(pdata_entry->data, data, data_len);
		pdata_entry->length = data_len;
	}

	if (out)
		*out = pdata_entry;

	if (out_len)
		*out_len = pdata_entry_len;

	return CLN_FW_ERR_NONE;
}