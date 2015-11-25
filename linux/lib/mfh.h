/*
 * Master Flash Header (MFH) Data Structure
 *
 * Copyright (c) 2015 Wind River Systems, Inc.
 *
 * See "LICENSE" for license terms.
 *
 * Author: Lans Zhang <jia.zhang@windriver.com>
 */

#ifndef __MFH_H__
#define __MFH_H__

#include <eee.h>

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

typedef enum {
	host_fw_stage1 = 0x00000000,
	host_fw_stage1_signed,
	reserved_0x00000002,
	host_fw_stage2,
	host_fw_stage2_signed,
	mfh_host_fw_stage2_conf,
	mfh_host_fw_stage2_conf_sign,
	mfh_host_fw_parameters,
	mfh_host_recovery_fw,
	mfh_host_recovery_fw_signed,
	reserved_0x0000000a,
	mfh_bootloader,
	mfh_bootloader_signed,
	mfh_bootloader_conf,
	mfh_bootloader_conf_signed,
	reserved_0x0000000f,
	mfh_kernel,
	mfh_kernel_signed,
	mfh_ramdisk,
	mfh_ramdisk_signed,
	reserved_0x00000014,
	mfh_loadable_program,
	mfh_loadable_program_signed,
	reserved_0x00000017,
	mfh_build_information,
	mfh_version,
} mfh_flash_item_type_t;

typedef struct {
	mfh_flash_item_type_t Type;
	uint32_t FlashItemAddress;
	uint32_t FlashItemLength;
	uint32_t Reserved;
} mfh_flash_item_t;

#pragma pack()

#endif	/* __MFH_H__ */