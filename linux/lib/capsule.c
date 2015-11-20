/*
 * Capsule generation
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