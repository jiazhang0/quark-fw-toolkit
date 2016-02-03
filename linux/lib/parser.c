/*
 * Generic firmware parser APIs
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
#include "internal.h"
#include "platform_data.h"
#include "capsule.h"
#include "buffer_stream.h"
#include "mfh.h"
#include "skm.h"
#include "csbh.h"

#define FLASH_SKM_SIZE			0x8000
#define FLASH_SKM_OFFSET		(-0x28000)

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
	bs_init(&parser->skm, NULL, 0);
	bs_init(&parser->pdata, NULL, 0);
	bs_init(&parser->pdata_header, NULL, 0);
	bcll_init(&parser->pdata_item_list);

	*out = parser;

	return CLN_FW_ERR_NONE;
}

static void
free_all_cln_fw_pdata_item(cln_fw_parser_t *parser)
{
	cln_fw_pdata_item_t *item, *tmp;

	bcll_for_each_link_safe(item, tmp, &parser->pdata_item_list, link) {
		bcll_del(&item->link);
		eee_mfree(item);
		--parser->nr_pdata_item;
	}
}

void
cln_fw_parser_destroy(cln_fw_parser_t *parser)
{
	if (!parser)
		return;

	free_all_cln_fw_pdata_item(parser);

	if (parser->pdata_item)
		eee_mfree(parser->pdata_item);

	if (bs_head(&parser->pdata_header))
		eee_mfree(bs_head(&parser->pdata_header));

	eee_mfree(parser);
}

static err_status_t
add_cln_fw_pdata_item(cln_fw_parser_t *parser, void *pdata_item_buf)
{
	cln_fw_pdata_item_t *item;

	item = eee_malloc(sizeof(*item));
	if (!item)
		return CLN_FW_ERR_OUT_OF_MEM;

	bs_init(&item->bs, pdata_item_buf,
		platform_data_item_size(pdata_item_buf));
	bcll_add_tail(&parser->pdata_item_list, &item->link);
	++parser->nr_pdata_item;

	return CLN_FW_ERR_NONE;
}

err_status_t
cln_fw_parser_parse(cln_fw_parser_t *parser)
{
	buffer_stream_t *fw = &parser->firmware;
	void *mfh, *pdata, *skm;
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
		void *pdata_header_buf, *pdata_item_buf, *p;
		unsigned long i, nr_pdata_item;

		err = platform_data_parse(pdata, pdata_len,
					  &pdata_header_buf,
					  &pdata_item_buf,
					  &nr_pdata_item);
		if (is_err_status(err))
			return err;

		p = pdata_item_buf;
		for (i = 0; i < nr_pdata_item; ++i) {
			err = add_cln_fw_pdata_item(parser, p);
			if (is_err_status(err)) {
				free_all_cln_fw_pdata_item(parser);
				return err;
			}
			p += platform_data_item_size(p);
		}

		parser->pdata_item = pdata_item_buf;
		parser->nr_pdata_item = nr_pdata_item;
		bs_init(&parser->pdata_header, pdata_header_buf,
			platform_data_header_size());

		bs_init(&parser->pdata, pdata, pdata_len);
	}

	err = bs_get_at(fw, &skm, FLASH_SKM_SIZE, FLASH_SKM_OFFSET);
	if (!is_err_status(err) && bs_empty(&parser->skm))
		bs_init(&parser->skm, skm, FLASH_SKM_SIZE);

	return CLN_FW_ERR_NONE;
}

err_status_t
cln_fw_parser_embed_key(cln_fw_parser_t *parser, cln_fw_sb_key_t key,
			void *in, unsigned long in_len)
{
	buffer_stream_t *pdata;
	uint16_t id;
	void *pdata_item;
	unsigned long pdata_item_len;
	err_status_t err;

	if (key == CLN_FW_SB_KEY_PK)
		id = PDATA_ID_PK;
	else
		id = PDATA_ID_SB_RECORD;

	pdata = &parser->pdata;
	err = platform_data_search_item(bs_head(pdata), bs_size(pdata), id);
	if (!is_err_status(err)) {
		cln_fw_pdata_item_t *item, *tmp;

		dbg(T("Updating platform item ID %d ...\n"), id);
		bcll_for_each_link_safe(item, tmp,
				&parser->pdata_item_list, link) {
			uint16_t pdata_item_id;
			uint32_t header;
			void *pdata_item = bs_head(&item->bs);

			pdata_item_id = platform_data_item_id(pdata_item);
			if (pdata_item_id != id)
				continue;

			if (key == CLN_FW_SB_KEY_PK)
				break;

			header = platform_data_cert_header(pdata_item);
			if (header == PDATA_KEK_CERT_HEADER
					&& key == CLN_FW_SB_KEY_KEK)
				break;
			else if (header == PDATA_DB_CERT_HEADER
					&& key == CLN_FW_SB_KEY_DB)
				break;
		}
		if (&item->link == &parser->pdata_item_list)
			return CLN_FW_ERR_PDATA_ITEM_NOT_FOUND;

		err = platform_data_update_item(bs_head(&item->bs),
						 bs_size(&item->bs), id,
						 NULL, NULL, in, in_len,
						 &pdata_item,
						 &pdata_item_len);
		if (is_err_status(err))
			return err;

		bs_init(&item->bs, pdata_item, pdata_item_len);
	} else if (err == CLN_FW_ERR_PDATA_ITEM_NOT_FOUND) {
		const char desc[][10] = {
			"pk",
			"kek cert",
			"db cert",
			"dbx cert",
		};

		err = platform_data_create_item(id, 0, desc[key], in, in_len,
						 &pdata_item,
						 &pdata_item_len);
		if (is_err_status(err))
			return err;

		err = add_cln_fw_pdata_item(parser, pdata_item);
		if (is_err_status(err)) {
			eee_mfree(pdata_item);
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
	cln_fw_pdata_item_t *item;
	void *pdata, *pdata_item;
	unsigned long total_item_len;
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
	bs_get(&fw, (void **)&pdata_item, 0);

	total_item_len = 0;
	bcll_for_each_link(item, &parser->pdata_item_list, link) {
		bs_post_put(&fw, bs_head(&item->bs), bs_size(&item->bs));
		total_item_len += bs_size(&item->bs);
	}

	platform_data_update_header(pdata, pdata_item, total_item_len);

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
	void *cap_header, *update_item;
	unsigned long cap_header_len, update_item_len, payload_len, cap_len;
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
					  &update_item, &update_item_len);
	if (is_err_status(err))
		goto err_create_update_item;

	bs_init(&cap, NULL, 0);

	cap_len = cap_header_len + update_item_len + payload_len;
	err = bs_reserve(&cap, cap_len);
	if (is_err_status(err)) {
		err(T("Failed to reserve the memory for capsule\n"));
		goto err_reserve;
	}

	bs_post_put(&cap, cap_header, cap_header_len);
	bs_post_put(&cap, update_item, update_item_len);
	bs_put(&cap, bs_head(fw) + bs_size(fw) - payload_len, payload_len);

	if (out)
		*out = bs_head(&cap);

	if (out_len)
		*out_len = bs_size(&cap);

err_reserve:
	eee_mfree(update_item);

err_create_update_item:
	eee_mfree(cap_header);

	return err;
}

#define MFH_FLASH_ITEM_SIGNED_BOOTLOADER		(1 << 0)
#define MFH_FLASH_ITEM_SIGNED_BOOTLOADER_CONF		(1 << 1)
#define MFH_FLASH_ITEM_SIGNED_KERNEL			(1 << 2)
#define MFH_FLASH_ITEM_SIGNED_RAMDISK			(1 << 3)
#define MFH_FLASH_ITEM_FW_VERSION			(1 << 4)

#define PDATA_ITEM_SERIAL_NUMBER			(1 << 0)
#define PDATA_ITEM_1ST_MAC				(1 << 1)
#define PDATA_ITEM_2ND_MAC				(1 << 2)
#define PDATA_ITEM_PK					(1 << 3)
#define PDATA_ITEM_SB_RECORD				(1 << 4)

err_status_t
cln_fw_parser_diagnose_firmware(cln_fw_parser_t *parser)
{
	mfh_context_t *mfh_ctx;
	skm_context_t *skm_ctx;
	buffer_stream_t *fw = &parser->firmware;
	void *skm, *mfh, *pdata;
	unsigned long skm_max_len, mfh_len, pdata_len;
	err_status_t err;
	int skm_status, mfh_status, pdata_status;
	uint32_t fw_version;

	skm_max_len = skm_size();
	err = bs_get_at(fw, &skm, skm_max_len, skm_offset());
	if (is_err_status(err)) {
		err(T("The length of firmware is not expected for ")
		    T("searching signed key module\n"));
		return err;
	}

	err = skm_context_new(&skm_ctx);
	if (is_err_status(err))
		return err;

	skm_status = 0;
	err = skm_ctx->probe(skm_ctx, skm, skm_max_len);
	if (is_err_status(err)) {
		skm_status = -1;
		goto show_skm_status;
	}

show_skm_status:
	info_cont(T("\nSigned key module symptoms:\n"));
	if (skm_status == -1)
		info_cont(T("- Not detected\n"));
	else {
		switch (skm_ctx->key_type) {
		case CSBH_KEY_TYPE_NONE:
			info_cont(T("- No signed key module detected\n"));
			break;
		case CSBH_KEY_TYPE_X102xD:
			info_cont(T("- Signed key module for X1020D/X1021D detected\n"));
			break;
		case CSBH_KEY_TYPE_X102x:
			info_cont(T("- Signed key module for X1020 or X1021 detected\n"));
			break;
		}

		info_cont(T("Diagnosis result:\n"));
		if (skm_ctx->key_type == CSBH_KEY_TYPE_X102x)
			info_cont(T("- You may need to contact hardware vendor ")
				  T("for help to generate the signed capsule ")
				  T("images and firmware image\n"));
		else
			info_cont(T("- N/A\n"));
	}

	err = bs_get_at(fw, &mfh, mfh_header_size(), mfh_offset());
	if (is_err_status(err)) {
		err(T("The length of firmware is not expected for ")
		    T("searching MFH\n"));
		return err;
	}

	err = mfh_context_new(&mfh_ctx);
	if (is_err_status(err))
		return err;

	mfh_status = 0;
	mfh_len = bs_remain(fw);
	err = mfh_ctx->probe(mfh_ctx, mfh, mfh_len);
	if (is_err_status(err)) {
		mfh_status = -1;
		goto show_mfh_status;
	}

	err = mfh_ctx->find_item(mfh_ctx, mfh_bootloader_signed, NULL, NULL);
	if (!is_err_status(err))
		mfh_status |= MFH_FLASH_ITEM_SIGNED_BOOTLOADER;

	err = mfh_ctx->find_item(mfh_ctx, mfh_bootloader_conf_signed, NULL, NULL);
	if (!is_err_status(err))
		mfh_status |= MFH_FLASH_ITEM_SIGNED_BOOTLOADER_CONF;

	err = mfh_ctx->find_item(mfh_ctx, mfh_kernel_signed, NULL, NULL);
	if (!is_err_status(err))
		mfh_status |= MFH_FLASH_ITEM_SIGNED_KERNEL;

	err = mfh_ctx->find_item(mfh_ctx, mfh_ramdisk_signed, NULL, NULL);
	if (!is_err_status(err))
		mfh_status |= MFH_FLASH_ITEM_SIGNED_RAMDISK;

	err = mfh_ctx->firmware_version(mfh_ctx, &fw_version);
	if (!is_err_status(err))
		mfh_status |= MFH_FLASH_ITEM_FW_VERSION;

show_mfh_status:
	info_cont(T("\nMFH Symptoms:\n"));
	if (mfh_status == -1)
		info_cont(T("- Not detected\n"));
	else {
		if (mfh_status & MFH_FLASH_ITEM_SIGNED_BOOTLOADER)
			info_cont(T("- Signed bootloader detected\n"));
		if (mfh_status & MFH_FLASH_ITEM_SIGNED_BOOTLOADER_CONF)
			info_cont(T("- Signed configuration of bootloader ")
				  T("detected\n"));
		if (mfh_status & MFH_FLASH_ITEM_SIGNED_KERNEL)
			info_cont(T("- Signed kernel detected\n"));
		if (mfh_status & MFH_FLASH_ITEM_SIGNED_RAMDISK)
			info_cont(T("- Signed ramdisk detected\n"));
		if (mfh_status & MFH_FLASH_ITEM_SIGNED_RAMDISK)
			info_cont(T("- Signed ramdisk detected\n"));

		info_cont(T("Diagnosis result:\n"));
		if (mfh_status & (MFH_FLASH_ITEM_SIGNED_BOOTLOADER
				| MFH_FLASH_ITEM_SIGNED_BOOTLOADER_CONF
				| MFH_FLASH_ITEM_SIGNED_KERNEL
				| MFH_FLASH_ITEM_SIGNED_RAMDISK))
			info_cont(T("- You may be unable to replace above ")
				  T("components with yours if you don't own ")
				  T("the stage 1 private key\n"));
		else
			info_cont(T("- N/A\n"));
	}

	pdata_len = platform_data_max_size();
	err = bs_get_at(fw, &pdata, pdata_len, platform_data_offset());
	if (is_err_status(err)) {
		err(T("The length of firmware is not expected for ")
		    T("searching platform data\n"));
		return err;
	}

	pdata_status = 0;
	err = platform_data_probe(pdata, &pdata_len);
	if (is_err_status(err)) {
		pdata_status = -1;
		goto show_pdata_status;
	}

	err = platform_data_search_item(pdata, pdata_len,
					PDATA_ID_SERIAL_NUMBER);
	if (!is_err_status(err))
		pdata_status |= PDATA_ITEM_SERIAL_NUMBER;
	err = platform_data_search_item(pdata, pdata_len, PDATA_ID_1ST_MAC);
	if (!is_err_status(err))
		pdata_status |= PDATA_ITEM_1ST_MAC;
	err = platform_data_search_item(pdata, pdata_len, PDATA_ID_2ND_MAC);
	if (!is_err_status(err))
		pdata_status |= PDATA_ITEM_2ND_MAC;
	err = platform_data_search_item(pdata, pdata_len, PDATA_ID_PK);
	if (!is_err_status(err))
		pdata_status |= PDATA_ITEM_PK;
	err = platform_data_search_item(pdata, pdata_len, PDATA_ID_SB_RECORD);
	if (!is_err_status(err))
		pdata_status |= PDATA_ITEM_SB_RECORD;

show_pdata_status:
	info_cont(T("\nPlatform Data Symptoms:\n"));
	if (pdata_status == -1) {
		info_cont(T("- Not detected\n"));
		return CLN_FW_ERR_NONE;
	}

	if (pdata_status & PDATA_ITEM_SERIAL_NUMBER)
		info_cont(T("- Board serial number detected\n"));
	if (pdata_status & PDATA_ITEM_1ST_MAC)
		info_cont(T("- MAC 0 detected\n"));
	if (pdata_status & PDATA_ITEM_2ND_MAC)
		info_cont(T("- MAC 1 detected\n"));
	if (pdata_status & PDATA_ITEM_PK)
		info_cont(T("- PK detected\n"));
	if (pdata_status & PDATA_ITEM_SB_RECORD)
		info_cont(T("- KEK/DB detected\n"));

	info_cont(T("Diagnosis result:\n"));
	if (fw_version <= 0x010100ff &&
			(pdata_status & (PDATA_ITEM_SERIAL_NUMBER |
				PDATA_ITEM_1ST_MAC | PDATA_ITEM_2ND_MAC))) {
		info_cont(T("- You need to run %d-step process of ")
			  T("r1.2 firmware upgrade to preserve the ")
			  T("contents of above asset\n"),
			  pdata_status & PDATA_ITEM_SERIAL_NUMBER ? 2 : 1);
	} else
		info_cont(T("N/A\n"));

	return CLN_FW_ERR_NONE;
}