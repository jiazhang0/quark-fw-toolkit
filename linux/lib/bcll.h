/*
 * Generic Bidirectional Circular Linked List (BCLL)
 *
 * Copyright (c) 2015 Wind River Systems, Inc.
 *
 * See "LICENSE" for license terms.
 *
 * Author: Lans Zhang <jia.zhang@windriver.com>
 */

#ifndef __BCLL_H__
#define __BCLL_H__

#include "internal.h"

struct __bcll;
typedef struct __bcll {
	struct __bcll *next;
	struct __bcll *prev;
} bcll_t;

#define BCLL_INIT(head)		{ .prev = head, .next = head }

static inline void
bcll_init(bcll_t *head)
{
	head->next = head->prev = head;
}

static inline void
__bcll_add(bcll_t *head, bcll_t *entry)
{
	bcll_t *next = head->next;

	next->prev = entry;
	entry->next = next;
	head->next = entry;
	entry->prev = head;
}

static inline void
bcll_add(bcll_t *head, bcll_t *entry)
{
	__bcll_add(head, entry);
}

static inline void
bcll_add_tail(bcll_t *head, bcll_t *entry)
{
	__bcll_add(head->prev, entry);
}

static inline void
bcll_del(bcll_t *entry)
{
	entry->prev->next = entry->next;
	entry->next->prev = entry->prev;
}

static inline void
bcll_del_init(bcll_t *entry)
{
	bcll_del(entry);
	bcll_init(entry);
}

#define bcll_for_each_link(p, head, member)	\
	for (p = container_of((head)->next, typeof(*p), member);	\
		&p->member != (head);	\
		p = container_of(p->member.next, typeof(*p), member))

#define bcll_for_each_link_safe(p, tmp, head, member)	\
	for (p = container_of((head)->next, typeof(*p), member),	\
		tmp = container_of(p->member.next, typeof(*p), member);	\
		&p->member != (head);	\
		p = tmp, tmp = container_of(tmp->member.next, typeof(*tmp), \
					    member))

#endif	/* __BCLL_H__ */