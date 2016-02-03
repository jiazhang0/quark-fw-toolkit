/*
 * cln_fwtool command definitions
 *
 * Copyright (c) 2015-2016 Wind River Systems, Inc.
 *
 * See "LICENSE" for license terms.
 *
 * Author: Lans Zhang <jia.zhang@windriver.com>
 */

#ifndef __CLN_FWTOOL_H__
#define __CLN_FWTOOL_H__

#include <eee.h>

typedef struct {
	const tchar_t *name;
	const char *optstring;
	const struct option *long_opts;
	int (*parse_arg)(int opt, char *optarg);
	void (*show_usage)(tchar_t *prog);
	int (*run)(tchar_t *prog);
} cln_fwtool_command_t;

extern cln_fwtool_command_t command_help;
extern cln_fwtool_command_t command_sbembed;
extern cln_fwtool_command_t command_show;
extern cln_fwtool_command_t command_capsule;
extern cln_fwtool_command_t command_diagnosis;

int
cln_fwtool_add_command(cln_fwtool_command_t *cmd);

cln_fwtool_command_t *
cln_fwtool_find_command(char *command);

#endif	/* __CLN_FWTOOL_H__ */