/*
 * Master Flash Header (MFH) infrastructure and data structure parser
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
#include "mfh.h"
#include "buffer_stream.h"
#include "class.h"

/* Refer to 330234-002US for the details */

#define MFH_OFFSET			(-0xF8000)

#pragma pack(1)

#define MFH_IDENTIFIER			0x5F4D4648U
#define MFH_VERSION			1
#define MFH_MAX_FLASH_ITEMS		240
#define MFH_MAX_BOOT_ITEMS		24

typedef struct {
	uint32_t Identifier;
	uint32_t Version;
	uint32_t Flags;
	uint32_t NextHeaderBlock;
	uint32_t FlashItemCount;
	uint32_t BootPriorityListCount;
} mfh_header_t;

typedef struct {
	mfh_flash_item_type_t Type;
	uint32_t FlashItemAddress;
	uint32_t FlashItemLength;
	uint32_t Reserved;
} mfh_flash_item_t;

#pragma pack()

typedef struct {
	mfh_header_t *header;
	mfh_flash_item_t *flash_item;
	unsigned long nr_flash_item;
	uint32_t *boot_list;
	unsigned long nr_boot_list;
	unsigned long total_len;
	buffer_stream_t bs;
} mfh_internal_t;

typedef struct {
	mfh_flash_item_t *start;
	mfh_flash_item_t *end;
	mfh_flash_item_t *current;
} mfh_find_context_t;

unsigned long
mfh_header_size(void)
{
	return sizeof(mfh_header_t);
}

long
mfh_offset(void)
{
	return MFH_OFFSET;
}

err_status_t
mfh_probe(void *mfh_buf, unsigned long *mfh_buf_len)
{
	buffer_stream_t bs;
	mfh_header_t *mfh;
	mfh_flash_item_t *flash_item;
	err_status_t err;

	bs_init(&bs, mfh_buf, *mfh_buf_len);

	err = bs_post_get(&bs, (void **)&mfh, sizeof(*mfh));
	if (is_err_status(err)) {
		err(T("The length is not expected for searching MFH\n"));
		return err;
	}

	if (mfh->Identifier != MFH_IDENTIFIER) {
		err(T("Failed to locate MFH\n"));
		return CLN_FW_ERR_INVALID_MFH;
	}

	if (mfh->Version != MFH_VERSION) {
		err(T("Invalid MFH Version: 0x%08x\n"), mfh->Version);
		return CLN_FW_ERR_INVALID_MFH;
	}

	if (mfh->Flags && cln_fw_verbose())
		warn(T("MFH Flags should be 0: 0x%08x\n"), mfh->Flags);

	if (mfh->NextHeaderBlock && cln_fw_verbose())
		warn(T("MFH NextHeaderBlock should be 0: 0x%08x\n"),
		     mfh->NextHeaderBlock);

	if (!mfh->FlashItemCount
	    || mfh->FlashItemCount > MFH_MAX_FLASH_ITEMS) {
		err(T("Invalid MFH FlashItemCount: 0x%08x\n"),
		    mfh->FlashItemCount);
		return CLN_FW_ERR_INVALID_MFH;
	}

	if (mfh->BootPriorityListCount > MFH_MAX_BOOT_ITEMS) {
		err(T("Invalid MFH BootPriorityListCount: 0x%08x\n"),
		    mfh->BootPriorityListCount);
		return CLN_FW_ERR_INVALID_MFH;
	}

	/* Skip Boot Priority List */
	err = bs_seek(&bs, mfh->BootPriorityListCount * sizeof(uint32_t));
	if (is_err_status(err)) {
		err(T("Invalid MFH BootPriorityListCount: 0x%08x\n"),
		    mfh->BootPriorityListCount);
		return err;
	}

	err = bs_post_get(&bs, (void **)&flash_item, mfh->FlashItemCount
			  * sizeof(mfh_flash_item_t));
	if (is_err_status(err)) {
		err(T("Invalid MFH FlashItemCount: 0x%08x\n"),
		    mfh->FlashItemCount);
		return err;
	}

	*mfh_buf_len = bs_tell(&bs);

	return CLN_FW_ERR_NONE;
}

err_status_t
mfh_show(void *mfh_buf, unsigned long mfh_buf_len)
{
	mfh_header_t *mfh;
	uint32_t *boot_list;
	mfh_flash_item_t *flash_item;
	uint32_t i;
	err_status_t err;

	err = mfh_probe(mfh_buf, &mfh_buf_len);
	if (is_err_status(err))
		return err;

	mfh = (mfh_header_t *)mfh_buf;

	info_cont(T("MFH Header:\n"));
	info_cont(T("  Identifier: 0x%x\n"), mfh->Identifier);
	info_cont(T("  Version: 0x%x\n"), mfh->Version);
	info_cont(T("  Flags: 0x%x\n"), mfh->Flags);
	info_cont(T("  Next Header Block: 0x%x\n"), mfh->NextHeaderBlock);
	info_cont(T("  Flash Item Count: 0x%x\n"), mfh->FlashItemCount);
	info_cont(T("  Boot Priority List Count: 0x%x\n"),
		  mfh->BootPriorityListCount);

	boot_list = (uint32_t *)(mfh + 1);
	for (i = 0; i < mfh->BootPriorityListCount; i++)
		info_cont(T("    [%d] Flash Item: %d\n"), i, boot_list[i]);

	flash_item = (mfh_flash_item_t *)(boot_list
		     + mfh->BootPriorityListCount);
	for (i = 0; i < mfh->FlashItemCount; i++) {
		info_cont(T("  Flash Item %d:\n"), i);
		info_cont(T("    Type: 0x%x\n"), flash_item[i].Type);
		info_cont(T("    Address: 0x%x\n"),
			  flash_item[i].FlashItemAddress);
		info_cont(T("    Length: 0x%x\n"),
			  flash_item[i].FlashItemLength);
	}

	return CLN_FW_ERR_NONE;
}

err_status_t
mfh_show_fw_version(void *mfh_buf, unsigned long mfh_buf_len)
{
	mfh_context_t *mfh_ctx;
	uint32_t fw_version;
	err_status_t err;

	err = mfh_context_new(&mfh_ctx);
	if (is_err_status(err))
		return err;

	err = mfh_ctx->probe(mfh_ctx, mfh_buf, mfh_buf_len);
	if (is_err_status(err))
		goto probe_err;

	err = mfh_ctx->firmware_version(mfh_ctx, &fw_version);
	if (is_err_status(err))
		goto get_fw_version_err;

	info_cont(T("Firmware Version: %d.%d.%d (%s Edition %d)\n"),
		  fw_version >> 24, (fw_version) >> 16 & 0xff,
		  (fw_version >> 8) & 0xff,
		  ((fw_version >> 4) & 0xf) == 0xf ? 
		  "Wind River" : "Intel", fw_version & 0xf);

get_fw_version_err:
probe_err:
	mfh_ctx->destroy(mfh_ctx);

	return err;
}

static err_status_t
__find_flash_item(mfh_find_context_t *find_ctx, mfh_flash_item_type_t type)
{
	while (find_ctx->current != find_ctx->end) {
		if (type == find_ctx->current->Type)
			break;

		++find_ctx->current;
	}

	return find_ctx->current == find_ctx->end
			? CLN_FW_ERR_MFH_FLASH_ITEM_NOT_FOUND
			: CLN_FW_ERR_NONE;
}

#if 0
static err_status_t
find_next_flash_item(mfh_flash_item_type_t type, mfh_find_context_t *find_ctx)
{
	if (!find_ctx || type >= mfh_flash_item_type_max)
		return CLN_FW_ERR_INVALID_PARAMETER;

	return __find_flash_item(find_ctx, type);
}
#endif

static err_status_t
find_first_flash_item(mfh_internal_t *mfh, mfh_flash_item_type_t type,
		      mfh_find_context_t *find_ctx)
{
	if (!mfh || !find_ctx || type >= mfh_flash_item_type_max)
		return CLN_FW_ERR_INVALID_PARAMETER;

	find_ctx->current = find_ctx->start = mfh->flash_item;
	find_ctx->end = find_ctx->start
			+ mfh->nr_flash_item * sizeof(mfh_flash_item_t);

	return __find_flash_item(find_ctx, type);
}

static err_status_t
find_flash_item(mfh_internal_t *mfh, mfh_flash_item_type_t type,
		mfh_flash_item_t **item)
{
	mfh_find_context_t find_ctx;
	err_status_t err;

	err = find_first_flash_item(mfh, type, &find_ctx);
	if (is_err_status(err))
		return err;

	if (item)
		*item = find_ctx.current;

	return CLN_FW_ERR_NONE;
}

static err_status_t
search_flash_item(mfh_context_t *ctx, mfh_flash_item_type_t type,
		  void **out, unsigned long *out_len)
{
	mfh_internal_t *mfh = ctx->priv;
	mfh_flash_item_t *item;
	void *p;
	unsigned long len;
	err_status_t err;

	err = find_flash_item(mfh, type, &item);
	if (is_err_status(err))
		return err;

	switch (type) {
	/* Yes the firmware version is stored in the Reserved field */
	case mfh_version:
		p = &item->Reserved;
		len = sizeof(item->Reserved);
		break;
	default:
		p = (void *)(unsigned long)item->FlashItemAddress;
		len = item->FlashItemLength;
	}

	if (out)
		*out = p;
	if (out_len)
		*out_len = len;

	return CLN_FW_ERR_NONE;
}

static err_status_t
get_firmware_version(mfh_context_t *ctx, uint32_t *version)
{
	uint32_t *fw_version;
	err_status_t err;

	err = search_flash_item(ctx, mfh_version, (void **)&fw_version, NULL);
	if (is_err_status(err))
		return err;

	*version = *fw_version;

	return CLN_FW_ERR_NONE;
}

static err_status_t
probe_mfh(mfh_context_t *ctx, void *buf, unsigned long buf_len)
{
	buffer_stream_t bs;
	mfh_header_t *mfh;
	uint32_t *boot_list;
	mfh_flash_item_t *flash_item;
	mfh_internal_t *priv;
	err_status_t err;

	if (!ctx || !buf || !buf_len)
		return CLN_FW_ERR_INVALID_PARAMETER;

	bs_init(&bs, buf, buf_len);

	err = bs_post_get(&bs, (void **)&mfh, sizeof(*mfh));
	if (is_err_status(err)) {
		err(T("The length is not expected for searching MFH\n"));
		return err;
	}

	if (mfh->Identifier != MFH_IDENTIFIER) {
		err(T("Failed to locate MFH\n"));
		return CLN_FW_ERR_INVALID_MFH;
	}

	if (mfh->Version != MFH_VERSION) {
		err(T("Invalid MFH Version: 0x%08x\n"), mfh->Version);
		return CLN_FW_ERR_INVALID_MFH;
	}

	if (mfh->Flags && cln_fw_verbose())
		warn(T("MFH Flags should be 0: 0x%08x\n"), mfh->Flags);

	if (mfh->NextHeaderBlock && cln_fw_verbose())
		warn(T("MFH NextHeaderBlock should be 0: 0x%08x\n"),
		     mfh->NextHeaderBlock);

	if (!mfh->FlashItemCount
	    || mfh->FlashItemCount > MFH_MAX_FLASH_ITEMS) {
		err(T("Invalid MFH FlashItemCount: 0x%08x\n"),
		    mfh->FlashItemCount);
		return CLN_FW_ERR_INVALID_MFH;
	}

	if (mfh->BootPriorityListCount > MFH_MAX_BOOT_ITEMS) {
		err(T("Invalid MFH BootPriorityListCount: 0x%08x\n"),
		    mfh->BootPriorityListCount);
		return CLN_FW_ERR_INVALID_MFH;
	}

	err = bs_post_get(&bs, (void **)&boot_list,
			  mfh->BootPriorityListCount * sizeof(uint32_t));
	if (is_err_status(err)) {
		err(T("Invalid MFH BootPriorityListCount: 0x%08x\n"),
		    mfh->BootPriorityListCount);
		return err;
	}

	err = bs_post_get(&bs, (void **)&flash_item, mfh->FlashItemCount
			  * sizeof(mfh_flash_item_t));
	if (is_err_status(err)) {
		err(T("Invalid MFH FlashItemCount: 0x%08x\n"),
		    mfh->FlashItemCount);
		return err;
	}

	priv = eee_malloc(sizeof(*priv));
	if (!priv)
		return CLN_FW_ERR_OUT_OF_MEM;

	priv->header = mfh;
	priv->flash_item = flash_item;
	priv->nr_flash_item = mfh->BootPriorityListCount;
	priv->boot_list = boot_list;
	priv->nr_boot_list = mfh->BootPriorityListCount;
	priv->total_len = bs_tell(&bs);
	ctx->priv = priv;

	return CLN_FW_ERR_NONE;
}

static void
destroy_mfh(mfh_context_t *ctx)
{
	obj_unref(ctx);
}

static void
mfh_context_dtor(void *ctx)
{
	mfh_internal_t *priv = ((mfh_context_t *)ctx)->priv;

	if (priv)
		eee_mfree(priv);
}

static err_status_t
mfh_context_ctor(void *ctx)
{
	mfh_context_t *mfh_ctx = ctx;

	eee_memset(mfh_ctx, 0, sizeof(*mfh_ctx));
	mfh_ctx->probe = probe_mfh;
	mfh_ctx->destroy = destroy_mfh;
	mfh_ctx->firmware_version = get_firmware_version;
	mfh_ctx->find_item = search_flash_item;

	return CLN_FW_ERR_NONE;
}

err_status_t
mfh_context_new(mfh_context_t **ctx)
{
	if (!ctx)
		return CLN_FW_ERR_INVALID_PARAMETER;

	return obj_new("mfh_context_t", ctx);
}

#if 0
err_status_t
mfh_add_flash_item(mfh_context_t *mfh_ctx, void *data, unsigned long data_len,
		   mfh_flash_item_type_t type, uint32_t addr, int bootable)
{
	return CLN_FW_ERR_NONE;
}

err_status_t
mfh_set_flash_item_bootable(mfh_context_t *mfh_ctx, mfh_flash_item_type_t type,
			    uint32_t addr, int bootable)
{
	return CLN_FW_ERR_NONE;
}
#endif

err_status_t
mfh_context_class_init(void)
{
	return class_register("mfh_context_t", NULL, mfh_context_ctor,
			      mfh_context_dtor, sizeof(mfh_context_t));
}