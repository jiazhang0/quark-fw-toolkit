/*
 * Master Flash Header (MFH) infrastructure API
 *
 * Copyright (c) 2015-2016 Wind River Systems, Inc.
 *
 * See "LICENSE" for license terms.
 *
 * Author: Lans Zhang <jia.zhang@windriver.com>
 */

#ifndef __MFH_H__
#define __MFH_H__

#include <eee.h>

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
	mfh_flash_item_type_max
} mfh_flash_item_type_t;

typedef struct __mfh_context		mfh_context_t;

struct __mfh_context {
	err_status_t (*probe)(mfh_context_t *ctx, void *buf,
			      unsigned long buf_len);
	void (*destroy)(mfh_context_t *ctx);
	err_status_t (*find_item)(mfh_context_t *ctx,
				  mfh_flash_item_type_t type,
				  void **out, unsigned long *out_len);
	err_status_t (*firmware_version)(mfh_context_t *ctx,
					 uint32_t *version);
	void *priv;
};

err_status_t
mfh_context_class_init(void);
err_status_t
mfh_context_new(mfh_context_t **ctx);

#endif	/* __MFH_H__ */