/*
 * Platform Data Structure
 *
 * Copyright (c) 2015 Wind River Systems, Inc.
 *
 * See "LICENSE" for license terms.
 *
 * Author: Lans Zhang <jia.zhang@windriver.com>
 */

#ifndef __PLATFORM_DATA_H__
#define __PLATFORM_DATA_H__

#include <eee.h>

/* Refer to 330234-002US for the details */

#define PLATFORM_DATA_OFFSET			(-0xF0000)
#define PLATFORM_DATA_MAX_SIZE			0x20000

#pragma pack(1)

#define PLATFORM_DATA_MAGIC			0x54414450U

typedef struct {
	/* Constant 0x54414450 */
	uint32_t magic;
	/* Total length of payload after header (not include 12-byte header) */
	uint32_t length;
	/* CRC32 of the payload after header */
	uint32_t crc32;
} platform_data_header_t;

#define MAX_PLATFORM_ID				0xFFFFU

#define PDATA_ID_UNDEF				0
#define PDATA_ID_PLATFORM_ID			1
#define PDATA_ID_SERIAL_NUMBER			2
#define PDATA_ID_1ST_MAC			3
#define PDATA_ID_2ND_MAC			4
#define PDATA_ID_MRC				6
#define PDATA_ID_PK				7
#define PDATA_ID_CERT				8	/* KEK, DB */
#define PDATA_ID_MAX				9
/* Custom IDs */
#define CAP_DATA_ID_PAYLOAD			0xFFF0

typedef struct {
	uint16_t id;
	/* Length of the data immediately following the VERSION field */
	uint16_t length;
	/* Freeform string data to name this data */
	char desc[10];
	/* Version of data; default 0 */
	uint16_t version;
	/* Variable data determined by id */
	uint8_t data[0];
} platform_data_entry_t;

#define PDATA_KEK_CERT_HEADER			0x00000001U
#define PDATA_DB_CERT_HEADER			0x00010002U

#pragma pack()

#endif	/* __PLATFORM_DATA_H__ */