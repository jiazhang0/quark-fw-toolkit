/*
 * Class implementation
 *
 * Copyright (c) 2015-2016 Wind River Systems, Inc.
 *
 * See "LICENSE" for license terms.
 *
 * Author: Lans Zhang <jia.zhang@windriver.com>
 */

#ifndef __CLASS_H__
#define __CLASS_H__

#include <eee.h>
#include <err_status.h>
#include <cln_fw.h>
#include "class.h"
#include "internal.h"

#ifndef CLASS_ERROR_BASE
#define CLASS_ERROR_BASE			0
#endif
#define CLASS_ERR(err)				(CLASS_ERROR_BASE + (err))
#define CLASS_ERR_NONE				CLASS_ERR(0)
#define CLASS_ERR_NOT_FOUND			CLASS_ERR(1)
#define CLASS_ERR_REGISTERED			CLASS_ERR(2)

typedef err_status_t (*class_ctor_t)(void *);
typedef void (*class_dtor_t)(void *);

err_status_t
class_register(const char *name, const char *parent,
	       const class_ctor_t obj_ctor, const class_dtor_t obj_dtor,
	       unsigned int obj_size);
err_status_t
class_instantiate(const char *name, void **obj);
unsigned long
obj_unref(void *ptr);
unsigned long
obj_ref(void *ptr);

#define obj_new(type, pptr)	({	\
	err_status_t __err;	\
	__err = class_instantiate(type, (void **)pptr); \
	__err; })

#define obj_destroy(pptr)	\
	do {	\
		typeof(*(ptr)) __ptr = *(pptr);	\
		unsigned long __ref_count;	\
		__ref_count = obj_unref(__ptr);	\
		if (!__ref_count)	\
			*(pptr) = NULL;	\
	} while (0)

#endif	/* __CLASS_H__ */