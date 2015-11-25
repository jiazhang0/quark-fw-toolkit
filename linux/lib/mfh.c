/*
 * Master Flash Header (MFH) Data Structure parser
 *
 * Copyright (c) 2015 Wind River Systems, Inc.
 *
 * See "LICENSE" for license terms.
 *
 * Author: Lans Zhang <jia.zhang@windriver.com>
 */

#include <eee.h>
#include <err_status.h>
#include <cln_fw.h>
#include "mfh.h"
#include "buffer_stream.h"

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

#if 0
err_status_t
mfh_parse_flash_items(cln_fw_parser_t *ctx,
		      mfh_flash_item_t *flash_item,
		      uint32_t items)
{
	uint32_t i;

	for (i = 0; i < items; i++) {
		if (flash_item[i].Type == mfh_bootloader_signed) {
			warn(T("Signed bootloader detected. ")
			     T("You may be unable to replace it ")
			     T("with yours.\n"));
		}
	}

	return CLN_FW_ERR_NONE;
}
#endif