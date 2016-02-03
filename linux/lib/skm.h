/*
 * Signed key module API
 *
 * Copyright (c) 2015-2016 Wind River Systems, Inc.
 *
 * See "LICENSE" for license terms.
 *
 * Author: Lans Zhang <jia.zhang@windriver.com>
 */

#ifndef __SKM_H__
#define __SKM_H__

#include <eee.h>
#include "csbh.h"

typedef struct __skm_context		skm_context_t;

struct __skm_context {
	err_status_t (*probe)(skm_context_t *ctx, void *buf,
			      unsigned long buf_len);
	void (*destroy)(skm_context_t *ctx);
	void (*show)(skm_context_t *ctx);
	csbh_key_type_t key_type;
	void *priv;
};

unsigned long
skm_size(void);
long
skm_offset(void);

err_status_t
skm_context_class_init(void);
err_status_t
skm_context_new(skm_context_t **ctx);

#endif	/* __SKM_H__ */