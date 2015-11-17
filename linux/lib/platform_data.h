/*
 * Platform Data Structure
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