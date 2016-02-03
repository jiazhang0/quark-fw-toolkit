/*
 * Help command
 *
 * Copyright (c) 2015-2016 Wind River Systems, Inc.
 *
 * See "LICENSE" for license terms.
 *
 * Author: Lans Zhang <jia.zhang@windriver.com>
 */

#include <eee.h>
#include <cln_fw.h>
#include <err_status.h>
#include "cln_fwtool.h"

static char *opt_command;

static void
show_usage(tchar_t *prog)
{
	info_cont(T("\nNobody can help you this time :(\n"));
}

static int
parse_arg(int opt, char *optarg)
{
	switch (opt) {
	case 1:
		{
			cln_fwtool_command_t *cmd;

			cmd = cln_fwtool_find_command(optarg);
			if (!cmd) {
				err(T("Unrecognized command argument ")
				    T("\"%s\" specified\n"), optarg);
				return -1;
			}
			opt_command = optarg;
		}
		break;
	default:
		return -1;
	}

	return 0;
}

static int
run_help(tchar_t *prog)
{
	cln_fwtool_find_command(opt_command)->show_usage(prog);

	return 0;
}

static struct option long_opts[] = {
	{ 0 },	/* NULL terminated */
};

cln_fwtool_command_t command_help = {
	.name = T("help"),
	.optstring = T("-"),
	.long_opts = long_opts,
	.parse_arg = parse_arg,
	.show_usage = show_usage,
	.run = run_help,
};