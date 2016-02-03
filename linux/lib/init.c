/*
 * Signed key module implementation
 *
 * Copyright (c) 2015 Wind River Systems, Inc.
 *
 * See "LICENSE" for license terms.
 *
 * Author: Lans Zhang <jia.zhang@windriver.com>
 */

#include <err_status.h>
#include <cln_fw.h>
#include "internal.h"
#include "class.h"
#include "csbh.h"
#include "mfh.h"
#include "skm.h"

static int initialized;

void __attribute__ ((constructor))
libclnfw_init(void)
{
	err_status_t err;

	if (initialized)
		return;

	err = mfh_context_class_init();
	if (is_err_status(err)) {
		err(T("Failed to register mfh_context_t\n"));
		return;
	}

	err = csbh_context_class_init();
	if (is_err_status(err)) {
		err(T("Failed to register csbh_context_t\n"));
		return;
	}

	err = skm_context_class_init();
	if (is_err_status(err)) {
		err(T("Failed to register skm_context_t\n"));
		return;
	}

	initialized = 1;
}

void __attribute__((destructor))
libclnfw_fini(void)
{
}