/*
 * Capsule Data Structure
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

#ifndef __CAPSULE_H__
#define __CAPSULE_H__

#include <eee.h>

#define CAPSULE_GUID	\
	"\xe4\xd1\x00\xd4\x14\xa3\x2b\x44\x89\xed\xa9\x2e\x4c\x81\x97\xcb"
#define CAPSULE_GUID_SIZE			16

#define CAPSULE_FL_UPDATE_MAC			0x00001
#define CAPSULE_FL_PERSIST_ACROSS_RESET		0x10000
#define CAPSULE_FL_INITIATE_RESET		0x40000

struct capsule_header {
	const uint8_t guid[CAPSULE_GUID_SIZE];
	/* Not include update entry table */
	uint32_t header_size;
	uint32_t flags;
	uint32_t image_size;
	uint8_t reserved[52];
};

#define CAPSULE_MAX_UPDATE_ENTRIES		20

struct capsule_update_entry {
	uint32_t addr;
	uint32_t size;
	uint32_t offset;
	uint32_t reserved;
};

#pragma pack()

#endif	/* __CAPSULE_H__ */