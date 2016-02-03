/*
 * UEFI definitions
 *
 * Copyright (c) 2015-2016 Wind River Systems, Inc.
 *
 * See "LICENSE" for license terms.
 *
 * Author: Lans Zhang <jia.zhang@windriver.com>
 */

#ifndef __UEFI_H__
#define __UEFI_H__

#include <eee.h>

typedef uint8_t			UINT8;
typedef uint16_t		UINT16;
typedef uint32_t		UINT32;

typedef struct {
	UINT32 Data1;
	UINT16 Data2;
	UINT16 Data3;
	UINT8 Data4[8];
} EFI_GUID;

typedef struct {
	/* Type of the signature */
	EFI_GUID SignatureType;
	/* Total size of the signature list, including this header */
	UINT32 SignatureListSize;
	/* Size of the signature header which precedes the array of signatures */
	UINT32 SignatureHeaderSize;
	/* Size of each signature */
	UINT32 SignatureSize;
	/* Header before the array of signatures */
	UINT8 SignatureHeader[0];
} EFI_SIGNATURE_LIST;

typedef struct {
	/* An identifier which identifies the agent which added the signature to the list */
	EFI_GUID SignatureOwner;
	/* The format of the signature is defined by the SignatureType */
	UINT8 SignatureData[0];
} EFI_SIGNATURE_DATA;

#endif	/* __UEFI_H__ */