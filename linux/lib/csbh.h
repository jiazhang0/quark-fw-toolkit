/*
 * Clanton Secure Boot Header data structure API
 *
 * Copyright (c) 2015 Wind River Systems, Inc.
 *
 * See "LICENSE" for license terms.
 *
 * Author: Lans Zhang <jia.zhang@windriver.com>
 */

#ifndef __CSBH_H__
#define __CSBH_H__

#include <eee.h>

typedef enum {
	CSBH_KEY_TYPE_NONE,
	CSBH_KEY_TYPE_X102xD,
	CSBH_KEY_TYPE_X102x
} csbh_key_type_t;

typedef struct __csbh_context		csbh_context_t;

struct __csbh_context {
	err_status_t (*probe)(csbh_context_t *ctx, void *buf,
			      unsigned long buf_len);
	void (*destroy)(csbh_context_t *ctx);
	void (*show)(csbh_context_t *ctx);
	csbh_key_type_t (*pubkey_type)(csbh_context_t *ctx);
	unsigned long csbh_size;
	unsigned long body_size;
	void *priv;
};

unsigned long
csbh_header_size(void);

err_status_t
csbh_context_class_init(void);
err_status_t
csbh_context_new(csbh_context_t **ctx);

#endif	/* __CSBH_H__ */