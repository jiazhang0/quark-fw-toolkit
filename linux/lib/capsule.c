/*
 * Capsule generation
 *
 * Copyright (c) 2015-2016 Wind River Systems, Inc.
 *
 * See "LICENSE" for license terms.
 *
 * Author: Lans Zhang <jia.zhang@windriver.com>
 */

#include <err_status.h>
#include <eee.h>
#include <cln_fw.h>
#include "capsule.h"

err_status_t
capsule_create_header(unsigned long payload_len, void **out,
		      unsigned long *out_len)
{
	capsule_header_t *cap_header;
	unsigned long update_entry_len;

	if (!payload_len)
		return CLN_FW_ERR_INVALID_PARAMETER;

	cap_header = eee_malloc(sizeof(*cap_header));
	if (!cap_header)
		return CLN_FW_ERR_OUT_OF_MEM;

	eee_memcpy((void *)cap_header->guid, CAPSULE_GUID,
		   sizeof(cap_header->guid));
	cap_header->header_size = sizeof(*cap_header);
	cap_header->flags = CAPSULE_FL_PERSIST_ACROSS_RESET;
	update_entry_len = sizeof(capsule_update_entry_t)
			   * CAPSULE_MAX_UPDATE_ENTRIES;
	payload_len = (payload_len + (BLOCK_SIZE - 1)) & ~(BLOCK_SIZE - 1);
	cap_header->image_size = sizeof(*cap_header) + update_entry_len
				 + payload_len;
	eee_memset(cap_header->reserved, 0xff, sizeof(cap_header->reserved));

	if (out)
		*out = cap_header;

	if (out_len)
		*out_len = sizeof(*cap_header);

	return CLN_FW_ERR_NONE;
}

err_status_t
capsule_create_update_entry(uint32_t addr, uint32_t payload_len,
			    void **out, unsigned long *out_len)
{
	capsule_update_entry_t *update_entry;
	unsigned long update_entry_len;

	update_entry_len = sizeof(*update_entry) * CAPSULE_MAX_UPDATE_ENTRIES;
	update_entry = eee_malloc(update_entry_len);
	if (!update_entry)
		return CLN_FW_ERR_OUT_OF_MEM;

	update_entry->addr = addr;
	update_entry->size = (payload_len + (BLOCK_SIZE - 1))
			     & ~(BLOCK_SIZE - 1);
	update_entry->offset = sizeof(capsule_header_t) + update_entry_len;
	update_entry->reserved = 0xffffffff;

	/* NULL update entry terminated */
	eee_memset(update_entry + 1, 0, sizeof(*update_entry));
	eee_memset(update_entry + 2, 0xff,
		   update_entry_len - sizeof(*update_entry) * 2);

	if (out)
		*out = update_entry;

	if (out_len)
		*out_len = update_entry_len;

	return CLN_FW_ERR_NONE;
}