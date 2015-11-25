/*
 * Capsule Data Structure
 *
 * Copyright (c) 2015 Wind River Systems, Inc.
 *
 * See "LICENSE" for license terms.
 *
 * Author: Lans Zhang <jia.zhang@windriver.com>
 */

#ifndef __CAPSULE_H__
#define __CAPSULE_H__

#include <eee.h>

#define CAPSULE_GUID	\
	"\xe4\xd1\x00\xd4\x14\xa3\x2b\x44\x89\xed\xa9\x2e\x4c\x81\x97\xcb"
#define CAPSULE_GUID_SIZE			16

#define BIOS_REGION_SIZE			0x300000
#define FIRMWARE_SIZE				0x800000

#define BLOCK_SIZE				4096

#define CAPSULE_FL_UPDATE_MAC			0x00001
#define CAPSULE_FL_PERSIST_ACROSS_RESET		0x10000
#define CAPSULE_FL_INITIATE_RESET		0x40000

typedef struct {
	uint8_t guid[CAPSULE_GUID_SIZE];
	/* Not include update entry table */
	uint32_t header_size;
	uint32_t flags;
	uint32_t image_size;
	uint8_t reserved[52];
} capsule_header_t;

#define CAPSULE_MAX_UPDATE_ENTRIES		20

typedef struct {
	uint32_t addr;
	uint32_t size;
	uint32_t offset;
	uint32_t reserved;
} capsule_update_entry_t;

#pragma pack()

#endif	/* __CAPSULE_H__ */