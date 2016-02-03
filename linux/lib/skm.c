/*
 * Signed key module implementation
 *
 * Copyright (c) 2015 Wind River Systems, Inc.
 *
 * See "LICENSE" for license terms.
 *
 * Author: Lans Zhang <jia.zhang@windriver.com>
 */

#include <eee.h>
#include <err_status.h>
#include <cln_fw.h>
#include "internal.h"
#include "class.h"
#include "csbh.h"
#include "skm.h"

#define SKM_SIZE		(32 * 1024)
#define SKM_OFFSET		(-0x28000)

#pragma pack(1)

typedef struct {
	uint32_t ModulusSize;
	uint32_t ExponentSize;
	uint8_t Modulus[256];
	uint32_t Exponent;
} stage1_rsa_pubkey_t;

#pragma pack()

typedef struct {
	csbh_context_t *csbh;
	stage1_rsa_pubkey_t *stage1_key;
} skm_internal_t;

unsigned long
skm_size(void)
{
	return SKM_SIZE;
}

long
skm_offset(void)
{
	return SKM_OFFSET;
}

static void
show_skm(skm_context_t *ctx)
{
	skm_internal_t *priv = ctx->priv;
	csbh_context_t *csbh = priv->csbh;

	info_cont(T("Signed key module:\n"));
	switch (ctx->key_type) {
	case CSBH_KEY_TYPE_NONE:
		info_cont(T("  No signed key module detected\n"));
		break;
	case CSBH_KEY_TYPE_X102xD:
		info_cont(T("  Signed key module for X1020D/X1021D detected\n"));
		break;
	case CSBH_KEY_TYPE_X102x:
		info_cont(T("  Signed key module for X1020 or X1021 detected\n"));
		break;
	}

	csbh->show(csbh);
}

static err_status_t
probe_skm(skm_context_t *ctx, void *buf, unsigned long buf_len)
{
	buffer_stream_t bs;
	csbh_context_t *csbh;
	stage1_rsa_pubkey_t *stage1_key;
	skm_internal_t *priv;
	err_status_t err;

	if (!ctx || !buf || !buf_len)
		return CLN_FW_ERR_INVALID_PARAMETER;

	csbh = NULL;
	err = csbh_context_new(&csbh);
	if (is_err_status(err))
		return err;

	err = csbh->probe(csbh, buf, buf_len);
	if (is_err_status(err)) {
		csbh->destroy(csbh);
		return err;
	}

	bs_init(&bs, buf, buf_len);

	err = bs_get_at(&bs, (void **)&stage1_key, csbh->body_size, csbh->csbh_size);
	if (is_err_status(err)) {
		csbh->destroy(csbh);
		err(T("The length is not expected for searching signed key module\n"));
		return err;
	}

	priv = eee_malloc(sizeof(*priv));
	if (!priv) {
		csbh->destroy(csbh);
		return CLN_FW_ERR_OUT_OF_MEM;
	}

	ctx->key_type = csbh->pubkey_type(csbh);
	priv->csbh = csbh;
	priv->stage1_key = stage1_key;
	ctx->priv = priv;

	return CLN_FW_ERR_NONE;
}

static void
destroy_skm(skm_context_t *ctx)
{
	obj_unref(ctx);
}

static void
skm_context_dtor(void *ctx)
{
	skm_internal_t *priv = ((skm_context_t *)ctx)->priv;

	if (priv) {
		if (priv->csbh)
			priv->csbh->destroy(priv->csbh);
		eee_mfree(priv);
	}
}

static err_status_t
skm_context_ctor(void *ctx)
{
	skm_context_t *skm_ctx = ctx;

	eee_memset(skm_ctx, 0, sizeof(*skm_ctx));
	skm_ctx->probe = probe_skm;
	skm_ctx->destroy = destroy_skm;
	skm_ctx->show = show_skm;
	skm_ctx->key_type = CSBH_KEY_TYPE_NONE;

	return CLN_FW_ERR_NONE;
}

err_status_t
skm_context_new(skm_context_t **ctx)
{
	if (!ctx)
		return CLN_FW_ERR_INVALID_PARAMETER;

	return obj_new("skm_context_t", ctx);
}

err_status_t
skm_context_class_init(void)
{
	return class_register("skm_context_t", NULL,
			      skm_context_ctor, skm_context_dtor,
			      sizeof(skm_context_t));
}