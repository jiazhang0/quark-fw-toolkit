/*
 * Implement the infrastructures to support object-oriented
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

typedef struct __class_meta			class_meta_t;
typedef struct __object_meta			object_meta_t;

struct __class_meta {
	class_ctor_t obj_ctor;
	class_dtor_t obj_dtor;
	unsigned int obj_size;
	class_meta_t *parent;
	class_meta_t *children;
	class_meta_t *sibling;
	bcll_t class_hierarchy;
	/* Class name must be always at the end */
	char *name;
};

struct __object_meta {
	class_meta_t *class_meta;
	unsigned long ref_count;
};

static struct {
	class_meta_t class_meta;
	unsigned long priv[0];
} root_obj = {
	.class_meta = {
		.name = "base_class_t",
		.class_hierarchy = BCLL_INIT(&root_obj.class_meta.class_hierarchy),
	},
};

static class_meta_t *base_class = &root_obj.class_meta;

unsigned long
obj_ref(void *ptr)
{
	object_meta_t *o;

	o = (object_meta_t *)(ptr - sizeof(*o));
	return ++o->ref_count;
}

unsigned long
obj_unref(void *ptr)
{
	object_meta_t *o;
	unsigned long ref_count;

	o = (object_meta_t *)(ptr - sizeof(*o));
	ref_count = --o->ref_count;
	if (!ref_count) {
		if (o->class_meta->obj_dtor)
			o->class_meta->obj_dtor(ptr);
		eee_mfree(o);
	}

	return ref_count;
}

static class_meta_t *
search_class_hierarchy(const char *name)
{
	class_meta_t *p;

	bcll_for_each_link(p, &base_class->class_hierarchy, class_hierarchy) {
		if (!eee_strcmp(p->name, name))
			return p;
	}

	return NULL;
}

err_status_t
class_instantiate(const char *name, void **obj)
{
	class_meta_t *c;
	object_meta_t *o;
	err_status_t err;

	c = search_class_hierarchy(name);
	if (!c)
		return CLASS_ERR_NOT_FOUND;

	o = eee_malloc(sizeof(*o) + c->obj_size);
	if (!o)
		return CLN_FW_ERR_OUT_OF_MEM;

	o->class_meta = c;
	o->ref_count = 1;
	*obj = o + 1;

	if (c->obj_ctor)
		err = c->obj_ctor(*obj);
	else
		err = CLN_FW_ERR_NONE;

	if (is_err_status(err)) {
		eee_mfree(o);
		*obj = NULL;
	}

	return err;
}

static void
set_class_hierarchy(class_meta_t *c, class_meta_t *parent)
{
	c->children = NULL;
	c->sibling = NULL;
	c->parent = parent;
	parent->children = c;

	bcll_add_tail(&base_class->class_hierarchy, &c->class_hierarchy);
}

err_status_t
class_register(const char *name, const char *parent,
	       const class_ctor_t obj_ctor, const class_dtor_t obj_dtor,
	       unsigned int obj_size)
{
	class_meta_t *c, *p;
	int nlen;

	if (!name)
		return CLN_FW_ERR_INVALID_PARAMETER;

	nlen = eee_strlen(name);
	if (!nlen)
		return CLN_FW_ERR_INVALID_PARAMETER;

	c = search_class_hierarchy(name);
	if (c)
		return CLASS_ERR_REGISTERED;

	if (parent) {
		if (!eee_strlen(parent))
			return CLN_FW_ERR_INVALID_PARAMETER;

		p = search_class_hierarchy(parent);
		if (!p)
			return CLASS_ERR_NOT_FOUND;
	} else
		p = base_class;

	c = eee_malloc(sizeof(*c) + nlen + 1);
	if (!c)
		return CLN_FW_ERR_OUT_OF_MEM;

	c->name = (char *)(c + 1);
	eee_strcpy(c->name, name);
	c->obj_ctor = obj_ctor;
	c->obj_dtor = obj_dtor;
	c->obj_size = obj_size;
	set_class_hierarchy(c, p);

	return CLN_FW_ERR_NONE;
}

err_status_t
class_unregister(const char *name)
{
	return CLN_FW_ERR_NONE;
}