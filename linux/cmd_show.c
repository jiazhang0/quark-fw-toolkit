/*
 * Firmware show command
 *
 * Copyright (c) 2015 Wind River Systems, Inc.
 *
 * See "LICENSE" for license terms.
 *
 * Author: Lans Zhang <jia.zhang@windriver.com>
 */

#include <eee.h>
#include <cln_fw.h>
#include <err_status.h>
#include "cln_fwtool.h"

static char *opt_input_file;

static void
show_usage(tchar_t *prog)
{
	info_cont(T("\nusage: %s show <file>\n"), prog);
	info_cont(T("Display the details of a firmware image\n"));
	info_cont(T("\nfile:\n"));
	info_cont(T("  Input firmware to be parsed\n"));
}

static int
parse_arg(int opt, char *optarg)
{
	switch (opt) {
	case 1:
		if (access(optarg, R_OK)) {
			err(T("Invalid input file specified\n"));
			return -1;
		}
		opt_input_file = optarg;
		break;
	default:
		return -1;
	}

	return 0;
}

static int
run_show(tchar_t *prog)
{
	void *fw;
	unsigned long fw_len;
	err_status_t err;
	int ret;

	if (!opt_input_file)
		die("No input file specified\n");

	ret = load_file(opt_input_file, (uint8_t **)&fw, &fw_len);
	if (ret)
		return ret;

	err = cln_fw_util_show_firmware(fw, fw_len);
	if (is_err_status(err))
		ret = -1;

	free(fw);

	return ret;
}

static struct option long_opts[] = {
	{ 0 },	/* NULL terminated */
};

cln_fwtool_command_t command_show = {
	.name = T("show"),
	.optstring = T("-"),
	.long_opts = long_opts,
	.parse_arg = parse_arg,
	.show_usage = show_usage,
	.run = run_show,
};