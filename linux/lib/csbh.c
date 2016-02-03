/*
 * Clanton Secure Boot Header data structure implementation
 *
 * Copyright (c) 2015-2016 Wind River Systems, Inc.
 *
 * See "LICENSE" for license terms.
 *
 * Author: Lans Zhang <jia.zhang@windriver.com>
 */

#include <eee.h>
#include <err_status.h>
#include <cln_fw.h>
#include "internal.h"
#include "bcll.h"
#include "class.h"
#include "csbh.h"

#pragma pack(1)

#define CSBH_IDENTIFIER			0x5f435348	/* "_CSH" */
#define CSBH_VERSION			0x00000001
#define CSBH_MODULE_VENDOR		0x00008086
#define CSBH_HASH_ALGO_SHA256		0x00000001
#define CSBH_CRYPTO_ALGO_RSA2048	0x00000001

typedef struct {
	uint32_t Identifier;
	uint32_t Version;
	uint32_t ModuleSize;
	uint32_t SecurityVersionNumberIndex;
	uint32_t SecurityVersionNumber;
	uint32_t ReservedModuleID;
	uint32_t ReservedModuleVendor;
	uint32_t ReservedDate;
	uint32_t ModuleHeaderSize;
	uint32_t HashAlgorithm;
	uint32_t CryptoAlgorithm;
	uint32_t KeySize;
	uint32_t SignatureSize;
	uint32_t ReservedNextHeaderPointer;
	uint8_t Reserved[8];
} csbh_header_t;

typedef struct {
	uint32_t ModulusSize;
	uint32_t ExponentSize;
	uint8_t Modulus[256];
	uint32_t Exponent;
} csbh_rsa_pubkey_t;

typedef struct {
	uint8_t Signature[256];
} csbh_rsa_signature_t;

#pragma pack()

typedef struct {
	csbh_header_t *header;
	csbh_rsa_pubkey_t *pubkey;
	csbh_rsa_signature_t* signature;
	void *body;
} csbh_internal_t;

/* Dump with xxd -p -i -s 0x7d8048 -l 256 -c 8 <flash.bin> */
static csbh_rsa_pubkey_t clanton_x102xD_pubkey = {
	.ModulusSize = 256,
	.ExponentSize = 4,
	.Modulus = {
		0xa1, 0x8c, 0x95, 0x58, 0x9d, 0xb4, 0x6f, 0xcc,
		0x1b, 0xfc, 0x57, 0xaa, 0xfa, 0xc4, 0xb7, 0xc8,
		0xd7, 0xb4, 0x3a, 0xc1, 0x97, 0x6f, 0xf4, 0xe1,
		0x0d, 0x4b, 0x0e, 0xee, 0xcc, 0x64, 0x71, 0x2b,
		0x29, 0x58, 0xce, 0x00, 0x80, 0x83, 0x4b, 0x60,
		0x9f, 0x30, 0xf9, 0xa6, 0x9f, 0x32, 0x25, 0xe5,
		0x2c, 0x89, 0x73, 0x16, 0x82, 0xfb, 0xb5, 0x34,
		0x00, 0xc6, 0x5f, 0x4c, 0xdd, 0x26, 0x5f, 0x60,
		0x6a, 0x88, 0xbe, 0x5a, 0xcf, 0x77, 0x45, 0x27,
		0xec, 0xe7, 0x28, 0xd5, 0x02, 0x48, 0x14, 0x36,
		0x38, 0xb7, 0xe9, 0xf1, 0x3a, 0xb7, 0x4e, 0x7d,
		0xe5, 0xdf, 0x45, 0x71, 0x4f, 0xa1, 0x7f, 0x75,
		0x7e, 0x3d, 0xc9, 0x3f, 0xcf, 0x83, 0x6b, 0xa2,
		0x86, 0x42, 0xf1, 0xfa, 0x55, 0x9d, 0x94, 0x89,
		0x0b, 0x65, 0xb5, 0x24, 0x8e, 0xc2, 0x06, 0x16,
		0xd7, 0x62, 0x71, 0x75, 0x71, 0xe8, 0x28, 0xa0,
		0x53, 0xc0, 0x22, 0x3d, 0xbf, 0x76, 0x9e, 0xde,
		0x32, 0x9c, 0x32, 0xe6, 0x8f, 0xed, 0xe0, 0x9e,
		0xd1, 0xcf, 0x3f, 0xc8, 0x1e, 0x3b, 0xe4, 0x26,
		0xe0, 0x6a, 0x71, 0xbd, 0x77, 0xd8, 0xbc, 0x24,
		0x2b, 0x5c, 0x3d, 0x38, 0xda, 0xe0, 0x9f, 0x5e,
		0xcf, 0x51, 0x43, 0x9b, 0x7c, 0xd9, 0xd1, 0x67,
		0x16, 0xf2, 0xe8, 0x67, 0x92, 0x24, 0xcd, 0x50,
		0x27, 0xa3, 0x75, 0x2d, 0x03, 0x3c, 0xf5, 0x30,
		0xc3, 0xd1, 0xe4, 0x5a, 0x38, 0xf8, 0x2b, 0x96,
		0x43, 0x4d, 0x8d, 0x12, 0xd3, 0x9d, 0xa8, 0xcb,
		0x01, 0x35, 0xcd, 0x74, 0xca, 0x59, 0xce, 0x47,
		0x22, 0xe9, 0xdc, 0x77, 0xe1, 0xd1, 0x4f, 0x59,
		0x06, 0x83, 0xf9, 0x21, 0x98, 0x4c, 0xfc, 0x18,
		0x6e, 0x0b, 0x4a, 0x48, 0x52, 0xcd, 0x8d, 0x17,
		0xf5, 0x1a, 0x5b, 0x4d, 0x4d, 0x98, 0x95, 0xc2,
		0xa6, 0x05, 0x31, 0x01, 0xf8, 0x44, 0xb5, 0x7d
	},
	.Exponent = 16777472
};

/* FIXME: don't include pad */
unsigned long
csbh_header_size(void)
{
	return sizeof(csbh_header_t) + sizeof(csbh_rsa_pubkey_t)
	       + sizeof(csbh_rsa_signature_t);
}

static csbh_key_type_t
get_pubkey_type(csbh_context_t *csbh)
{
	csbh_internal_t *priv = csbh->priv;

	if (!priv)
		return CLN_FW_ERR_INVALID_PARAMETER;

	if (!priv->pubkey)
		return CSBH_KEY_TYPE_NONE;

	if (!eee_memcmp(priv->pubkey, &clanton_x102xD_pubkey,
			sizeof(clanton_x102xD_pubkey)))
		return CSBH_KEY_TYPE_X102xD;

	return CSBH_KEY_TYPE_X102x;
}

static void
show_csbh(csbh_context_t *ctx)
{
	csbh_internal_t *priv = ((csbh_context_t *)ctx)->priv;
	csbh_header_t *header = priv->header;

	info_cont(T("CSBH Header:\n"));
	info_cont(T("  Identifier: 0x%x\n"), header->Identifier);
	info_cont(T("  Version: 0x%x\n"), header->Version);
	info_cont(T("  Module Size: 0x%x\n"), header->ModuleSize);
	info_cont(T("  Security Version Number Index: 0x%x\n"),
		  header->SecurityVersionNumberIndex);
	info_cont(T("  Security Version Number: 0x%x\n"),
		  header->SecurityVersionNumber);
	info_cont(T("  Reserved Module ID: 0x%x\n"), header->ReservedModuleID);
	info_cont(T("  Reserved Module Vendor: 0x%x\n"),
		  header->ReservedModuleVendor);
	info_cont(T("  Reserved Date: 0x%x\n"), header->ReservedDate);
	info_cont(T("  Module Header Size: 0x%x\n"), header->ModuleHeaderSize);
	info_cont(T("  Hash Algorithm: 0x%x\n"), header->HashAlgorithm);
	info_cont(T("  Crypto Algorithm: 0x%x\n"), header->CryptoAlgorithm);
	info_cont(T("  Key Size: 0x%x\n"), header->KeySize);
	info_cont(T("  Signature Size: 0x%x\n"), header->SignatureSize);
	info_cont(T("  Reserved Next Header Pointer: 0x%x\n"),
		  header->ReservedNextHeaderPointer);
}

static err_status_t
probe_csbh(csbh_context_t *ctx, void *buf, unsigned long buf_len)
{
	buffer_stream_t bs;
	csbh_header_t *header;
	csbh_rsa_pubkey_t *pubkey;
	csbh_rsa_signature_t *sig;
	void *body;
	csbh_internal_t *priv;
	unsigned long body_size, pad_size;
	err_status_t err;

	if (!ctx || !buf || !buf_len)
		return CLN_FW_ERR_INVALID_PARAMETER;

	bs_init(&bs, buf, buf_len);

	err = bs_post_get(&bs, (void **)&header, sizeof(*header));
	if (is_err_status(err)) {
		err(T("The length is not expected for searching CSBH header\n"));
		return err;
	}

	if (header->Identifier != CSBH_IDENTIFIER) {
		err(T("Failed to locate CSBH\n"));
		return CLN_FW_ERR_INVALID_CSBH;
	}

	if (header->Version != CSBH_VERSION) {
		err(T("Invalid CSBH version: 0x%08x\n"), header->Version);
		return CLN_FW_ERR_INVALID_CSBH;
	}

	if (header->ModuleHeaderSize < sizeof(*header) + sizeof(csbh_rsa_pubkey_t)
			+ sizeof(csbh_rsa_signature_t)) {
		err(T("Invalid module header size: 0x%08x\n"), header->ModuleHeaderSize);
		return CLN_FW_ERR_INVALID_CSBH;
	}

	pad_size = header->ModuleHeaderSize -
		   (sizeof(*header) + sizeof(csbh_rsa_pubkey_t) +
		   sizeof(csbh_rsa_signature_t));

	if (header->ModuleSize < header->ModuleHeaderSize) {
		err(T("Invalid Module size: 0x%08x\n"), header->ModuleSize);
		return CLN_FW_ERR_INVALID_CSBH;
	}

	body_size = header->ModuleSize - header->ModuleHeaderSize;
	if (!aligned(body_size, 64)) {
		err(T("Body size not 64-bit aligned: 0x%lx\n"), body_size);
		return CLN_FW_ERR_INVALID_CSBH;
	}

	if ((header->ReservedModuleVendor != CSBH_MODULE_VENDOR) && cln_fw_verbose())
		warn(T("Module Vendor should be reserved: 0x%08x\n"),
		     header->ReservedModuleVendor);

	if (header->HashAlgorithm != CSBH_HASH_ALGO_SHA256) {
		err(T("Invalid hash algorithm: 0x%08x\n"), header->HashAlgorithm);
		return CLN_FW_ERR_INVALID_CSBH;
	}

	if (header->CryptoAlgorithm != CSBH_CRYPTO_ALGO_RSA2048) {
		err(T("Invalid crypto algorithm: 0x%08x\n"), header->CryptoAlgorithm);
		return CLN_FW_ERR_INVALID_CSBH;
	}

	if (header->KeySize != sizeof(csbh_rsa_pubkey_t)) {
		err(T("Invalid key size: 0x%08x\n"),
		    header->KeySize);
		return CLN_FW_ERR_INVALID_CSBH;
	}

	if (header->SignatureSize != sizeof(csbh_rsa_signature_t)) {
		err(T("Invalid signature size: 0x%08x\n"),
		    header->SignatureSize);
		return CLN_FW_ERR_INVALID_CSBH;
	}

	err = bs_post_get(&bs, (void **)&pubkey, sizeof(*pubkey));
	if (is_err_status(err)) {
		err(T("Cannot locate CSBH public key\n"));
		return err;
	}

	err = bs_post_get(&bs, (void **)&sig, sizeof(*sig));
	if (is_err_status(err)) {
		err(T("Cannot locate CSBH signature\n"));
		return err;
	}

	err = bs_seek(&bs, pad_size);
	if (is_err_status(err)) {
		err(T("Cannot locate CSBH header pad\n"));
		return err;
	}

	err = bs_get(&bs, (void **)&body, body_size);
	if (is_err_status(err)) {
		err(T("Cannot locate CSBH body\n"));
		return err;
	}

	priv = eee_malloc(sizeof(*priv));
	if (!priv)
		return CLN_FW_ERR_OUT_OF_MEM;

	ctx->csbh_size = header->ModuleSize;
	ctx->body_size = body_size;

	priv->header = header;
	priv->pubkey = pubkey;
	priv->signature = sig;
	priv->body = body;
	ctx->priv = priv;

	return CLN_FW_ERR_NONE;
}

static void
destroy_csbh(csbh_context_t *ctx)
{
	obj_unref(ctx);
}

static void
csbh_context_dtor(void *ctx)
{
	csbh_internal_t *priv = ((csbh_context_t *)ctx)->priv;

	if (priv)
		eee_mfree(priv);
}

static err_status_t
csbh_context_ctor(void *ctx)
{
	csbh_context_t *csbh_ctx = ctx;

	eee_memset(csbh_ctx, 0, sizeof(*csbh_ctx));
	csbh_ctx->probe = probe_csbh;
	csbh_ctx->destroy = destroy_csbh;
	csbh_ctx->show = show_csbh;
	csbh_ctx->pubkey_type = get_pubkey_type;

	return CLN_FW_ERR_NONE;
}

err_status_t
csbh_context_new(csbh_context_t **ctx)
{
	if (!ctx)
		return CLN_FW_ERR_INVALID_PARAMETER;

	return obj_new("csbh_context_t", ctx);
}

err_status_t
csbh_context_class_init(void)
{
	return class_register("csbh_context_t", NULL, csbh_context_ctor,
			      csbh_context_dtor, sizeof(csbh_context_t));
}