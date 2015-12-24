/*
 * Generic firmware parser APIs
 *
 * Copyright (c) 2015 Wind River Systems, Inc.
 *
 * See "LICENSE" for license terms.
 *
 * Author: Lans Zhang <jia.zhang@windriver.com>
 */

#ifndef CLN_FW_H
#define CLN_FW_H

#include <err_status.h>

typedef unsigned long *				cln_fw_handle_t;

typedef enum {
	CLN_FW_SB_KEY_PK,
	CLN_FW_SB_KEY_KEK,
	CLN_FW_SB_KEY_DB,
	CLN_FW_SB_KEY_DBX,
	CLN_FW_SB_KEY_MAX,
} cln_fw_sb_key_t;

#ifndef CLN_FW_ERROR_BASE
#define CLN_FW_ERROR_BASE			0
#endif

#define CLN_FW_ERR(err)				(CLN_FW_ERROR_BASE + (err))
#define CLN_FW_ERR_NONE				CLN_FW_ERR(0)
#define CLN_FW_ERR_INVALID_PARAMETER		CLN_FW_ERR(1)
#define CLN_FW_ERR_INVALID_MFH			CLN_FW_ERR(2)
#define CLN_FW_ERR_INVALID_PDATA		CLN_FW_ERR(3)
#define CLN_FW_ERR_OUT_OF_MEM			CLN_FW_ERR(4)
#define CLN_FW_ERR_NO_PDATA			CLN_FW_ERR(5)
#define CLN_FW_ERR_PDATA_ITEM_NOT_FOUND		CLN_FW_ERR(6)
#define CLN_FW_ERR_MFH_FLASH_ITEM_NOT_FOUND	CLN_FW_ERR(7)

/* Handle routines */

err_status_t
cln_fw_handle_open(cln_fw_handle_t *handle, void *fw, unsigned long fw_len);

void
cln_fw_handle_close(cln_fw_handle_t handle);

void
cln_fw_handle_show_all(cln_fw_handle_t handle);

err_status_t
cln_fw_handle_embed_key(cln_fw_handle_t handle, cln_fw_sb_key_t key,
			void *in, unsigned long in_len);

err_status_t
cln_fw_handle_flush(cln_fw_handle_t handle, void **out,
		    unsigned long *out_len);

err_status_t
cln_fw_handle_generate_capsule(cln_fw_handle_t handle, int bios_only,
			       void **out, unsigned long *out_len);

err_status_t
cln_fw_handle_diagnose_firmware(cln_fw_handle_t handle, void *in,
				unsigned long in_len);

/* Utility routines */

err_status_t
cln_fw_util_show_firmware(void *fw, unsigned long fw_len);

err_status_t
cln_fw_util_embed_sb_keys(void *fw, unsigned long fw_len,
			  void *pk, unsigned long pk_len,
			  void *kek, unsigned long kek_len,
			  void *db, unsigned long db_len,
			  void *dbx, unsigned long dbx_len,
			  void **out, unsigned long *out_len);

err_status_t
cln_fw_util_generate_capsule(void *fw, unsigned long fw_len,
			     int bios_only, void **out,
			     unsigned long *out_len);

int
cln_fw_verbose(void);

void
cln_fw_set_verbosity(int verbose);

#endif	/* CLN_FW_H */