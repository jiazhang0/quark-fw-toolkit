/*
 * Platform Data Structure parser
 *
 * Copyright (c) 2015-2016 Wind River Systems, Inc.
 *
 * See "LICENSE" for license terms.
 *
 * Author: Lans Zhang <jia.zhang@windriver.com>
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
platform_data_all_item_size(void *pdata_buf)
{
	platform_data_header_t *pdata_header = pdata_buf;
	return pdata_header->length;
}

unsigned long
platform_data_size(void *pdata_buf)
{
	return platform_data_all_item_size(pdata_buf)
	       + platform_data_header_size();
}

unsigned long
platform_data_item_size(void *pdata_item_buf)
{
	platform_data_item_t *pdata_item = pdata_item_buf;
	return pdata_item->length + sizeof(*pdata_item);
}

uint16_t
platform_data_item_id(void *pdata_item_buf)
{
	platform_data_item_t *pdata_item = pdata_item_buf;
	return pdata_item->id;
}

void
platform_data_update_header(void *pdata, void *pdata_item,
			    unsigned long pdata_item_len)
{
	platform_data_header_t *pdata_header = pdata;
	pdata_header->length = pdata_item_len;
	pdata_header->crc32 = crc32(pdata_item, pdata_item_len);
}

uint32_t
platform_data_cert_header(void *pdata_item_buf)
{
	platform_data_item_t *pdata_item = pdata_item_buf;
	return *(uint32_t *)(pdata_item + 1);
}

err_status_t
platform_data_probe(void *pdata_buf, unsigned long *pdata_buf_len)
{
	buffer_stream_t bs;
	platform_data_header_t *pdata;
	unsigned long nr_pdata_item;
	uint16_t total_item_len;
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
	for (nr_pdata_item = 0, total_item_len = 0;
			total_item_len < pdata->length; ++nr_pdata_item) {
		platform_data_item_t *pdata_item;
		uint8_t *pdata_item_data;

		err = bs_post_get(&bs, (void **)&pdata_item,
				  sizeof(*pdata_item));
		if (is_err_status(err)) {
			err(T("Invalid length of item %ld\n"),
			    nr_pdata_item);
			return CLN_FW_ERR_INVALID_PDATA;
		}

		if (pdata_item->id == PDATA_ID_INVALID
				|| pdata_item->id >= PDATA_ID_MAX) {
			err(T("Invalid id of item %ld: 0x%x\n"),
			    nr_pdata_item, pdata_item->id);
			err = CLN_FW_ERR_INVALID_PDATA;
		}

		err = bs_post_get(&bs, (void **)&pdata_item_data,
				  pdata_item->length);
		if (is_err_status(err)) {
			err(T("Invalid length of item %ld: 0x%x\n"),
			    nr_pdata_item, pdata_item->length);
			return CLN_FW_ERR_INVALID_PDATA;
		}

		total_item_len += sizeof(*pdata_item) + pdata_item->length;
	}
	if (is_err_status(err))
		return CLN_FW_ERR_INVALID_PDATA;

	if (total_item_len != pdata->length) {
		err(T("Invalid platform data length: 0x%x\n"), pdata->length);
		return CLN_FW_ERR_INVALID_PDATA;
	}

	if (total_item_len + sizeof(*pdata) > PLATFORM_DATA_MAX_SIZE) {
		err(T("Platform data length is too long: 0x%x\n"),
		    total_item_len);
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
	platform_data_item_t *pdata_item;
	unsigned long nr_pdata_item;
	uint16_t total_item_len;

	pdata = (platform_data_header_t *)pdata_buf;

	info_cont(T("Platform Data Header:\n"));
	info_cont(T("  Magic: 0x%x\n"), pdata->magic);
	info_cont(T("  Length: 0x%x\n"), pdata->length);
	info_cont(T("  CRC32: 0x%x\n"), pdata->crc32);

	pdata_item = (platform_data_item_t *)(pdata + 1);
	for (nr_pdata_item = 0, total_item_len = 0;
			total_item_len < pdata->length; ++nr_pdata_item) {
		char desc[sizeof(pdata_item->desc) + 1];
		uint8_t *pdata_item_data = (uint8_t *)(pdata_item + 1);
		uint16_t data_len_shown = 8;
		uint16_t i;

		memcpy(desc, pdata_item->desc, sizeof(pdata_item->desc));
		desc[sizeof(pdata_item->desc)] = 0;

		info_cont(T("  Entry %ld:\n"), nr_pdata_item);
		info_cont(T("    ID: 0x%x\n"), pdata_item->id);
		info_cont(T("    Length: 0x%x\n"), pdata_item->length);
		info_cont(T("    Description: %s\n"), desc);
		info_cont(T("    Version: 0x%x\n"), pdata_item->version);

		if (data_len_shown > pdata_item->length) {
			data_len_shown = pdata_item->length;
			info_cont(T("    Data: "));
		} else
			info_cont(T("    Data (first 8 bytes): "));

		for (i = 0; i < data_len_shown; i++)
			info_cont(T("0x%02x "), pdata_item_data[i]);
		info_cont(T("\n"));

		total_item_len += sizeof(*pdata_item) + pdata_item->length;
		pdata_item = (platform_data_item_t *)(pdata_item_data
			      + pdata_item->length);
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
count_pdata_item(platform_data_header_t *pdata)
{
	platform_data_item_t *pdata_item;
	unsigned long nr_pdata_item;
	uint16_t total_item_len;

	pdata_item = (platform_data_item_t *)(pdata + 1);
	for (nr_pdata_item = 0, total_item_len = 0;
			total_item_len < pdata->length; ++nr_pdata_item) {
		uint8_t *pdata_item_data = (uint8_t *)(pdata_item + 1);

		total_item_len += sizeof(*pdata_item) + pdata_item->length;
		pdata_item = (platform_data_item_t *)(pdata_item_data
			      + pdata_item->length);
	}

	return nr_pdata_item;
}

err_status_t
platform_data_parse(void *pdata_buf, unsigned long pdata_buf_len,
		    void **out_pdata_header_buf, void **out_pdata_item_buf,
		    unsigned long *out_nr_pdata_item)
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

	if (out_pdata_item_buf) {
		platform_data_item_t *pdata_item, *out_pdata_item;

		pdata_item = (platform_data_item_t *)(pdata + 1);
		out_pdata_item = eee_malloc(pdata->length);
		if (!out_pdata_item) {
			if (out_pdata_header_buf)
				eee_mfree(*out_pdata_header_buf);
			return CLN_FW_ERR_OUT_OF_MEM;
		}

		eee_memcpy(out_pdata_item, pdata_item, pdata->length);
		*out_pdata_item_buf = out_pdata_item;
	}

	if (out_nr_pdata_item)
		*out_nr_pdata_item = count_pdata_item(pdata);

	return CLN_FW_ERR_NONE;
}

static int
search_pdata_item(platform_data_header_t *pdata, uint16_t id,
		   platform_data_item_t **out)
{
	platform_data_item_t *pdata_item;
	unsigned long total_item_len;

	pdata_item = (platform_data_item_t *)(pdata + 1);
	for (total_item_len = 0; total_item_len < pdata->length;) {
		uint8_t *item_data;

		if (id == pdata_item->id) {
			if (out)
				*out = pdata_item;
			return 1;
		}
		total_item_len += sizeof(*pdata_item) + pdata_item->length;
		item_data = (uint8_t *)(pdata_item + 1);
		pdata_item = (platform_data_item_t *)(item_data
			      + pdata_item->length);
	}

	return 0;
}

err_status_t
platform_data_search_item(void *pdata_buf, unsigned long pdata_buf_len,
			   uint16_t id)
{
	platform_data_header_t *pdata;
	platform_data_item_t *pdata_item;
	err_status_t err;

	err = platform_data_probe(pdata_buf, &pdata_buf_len);
	if (is_err_status(err))
		return err;

	pdata = (platform_data_header_t *)pdata_buf;
	if (!search_pdata_item(pdata, id, &pdata_item))
		return CLN_FW_ERR_PDATA_ITEM_NOT_FOUND;

	return CLN_FW_ERR_NONE;
}

static void
init_pdata_item(platform_data_item_t *item, uint16_t id, uint16_t version,
		 const char desc[10], uint8_t *data, uint16_t data_len)
{
	item->id = id;
	item->version = version;
	eee_memcpy(item->desc, desc, sizeof(item->desc));
	item->length = data_len;
	eee_memcpy(item->data, data, data_len);
}

err_status_t
platform_data_create_item(uint16_t id, uint16_t version, const char desc[10],
			  uint8_t *data, uint16_t data_len, void **out,
			  unsigned long *out_len)
{
	platform_data_item_t *pdata_item;

	if (!out && !out_len)
		return CLN_FW_ERR_INVALID_PARAMETER;

	pdata_item = eee_malloc(sizeof(*pdata_item) + data_len);
	if (!pdata_item)
		return CLN_FW_ERR_OUT_OF_MEM;

	init_pdata_item(pdata_item, id, version, desc, data, data_len);

	if (out)
		*out = pdata_item;

	if (out_len)
		*out_len = sizeof(*pdata_item) + data_len;

	return CLN_FW_ERR_NONE;
}

err_status_t
platform_data_update_item(void *pdata_item_buf,
			  unsigned long pdata_item_buf_len,
			  uint16_t id, uint16_t *version,
			  const char desc[10], uint8_t *data,
			  uint16_t data_len, void **out,
			  unsigned long *out_len)
{
	platform_data_item_t *pdata_item;
	buffer_stream_t bs;
	unsigned long pdata_item_len;
	err_status_t err;

	if (!out && !out_len)
		return CLN_FW_ERR_INVALID_PARAMETER;

	if (pdata_item_buf_len !=
			platform_data_item_size((void *)pdata_item_buf))
		return CLN_FW_ERR_INVALID_PARAMETER;

	bs_init(&bs, pdata_item_buf, pdata_item_buf_len);

	err = bs_reserve_at(&bs, data_len, sizeof(*pdata_item));
	if (is_err_status(err))
		return err;

	pdata_item_len = sizeof(*pdata_item) + data_len;
	bs_get_at(&bs, (void **)&pdata_item, pdata_item_len, 0);

	if (version)
		pdata_item->version = *version;
	if (desc)
		eee_memcpy(pdata_item->desc, desc,
			   sizeof(pdata_item->desc));
	if (data) {
		eee_memcpy(pdata_item->data, data, data_len);
		pdata_item->length = data_len;
	}

	if (out)
		*out = pdata_item;

	if (out_len)
		*out_len = pdata_item_len;

	return CLN_FW_ERR_NONE;
}