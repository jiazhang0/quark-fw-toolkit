/*
 * Error status definitions
 *
 * Copyright (c) 2015 Wind River Systems, Inc.
 *
 * See "LICENSE" for license terms.
 *
 * Author: Lans Zhang <jia.zhang@windriver.com>
 */

#ifndef ERROR_STATUS_H
#define ERROR_STATUS_H

typedef unsigned long		err_status_t;

#define ERROR_MASK		0x0000FFFFU

static int inline
is_err_status(err_status_t err)
{
	return !!(err & ERROR_MASK);
}

#endif	/* ERROR_STATUS_H */