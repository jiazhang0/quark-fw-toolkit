/*
 * Diagnosis command
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

static char *opt_input_file;

static void
show_usage(tchar_t *prog)
{
	info_cont(T("\nusage: %s diagnosis <file> <args>\n"), prog);
	info_cont(T("Give the diagnosis information\n"));
	info_cont(T("\nfile:\n"));
	info_cont(T("  Input firmware to be parsed\n"));
	info_cont(T("\nargs: N/A\n"));
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
run_diagnosis(tchar_t *prog)
{
	void *fw;
	unsigned long fw_len;
	cln_fw_handle_t handle;
	err_status_t err;
	int ret;

	if (!opt_input_file) {
		if (!cln_fw_util_cpu_is_clanton())
			die("No input file specified\n");

		fw_len = 0x800000;
		ret = read_phys_mem("/dev/mem", (uint8_t **)&fw, fw_len,
				    0xFF800000);
	} else
		ret = load_file(opt_input_file, (uint8_t **)&fw, &fw_len);

	if (ret)
		return ret;

	handle = NULL;
	err = cln_fw_handle_open(&handle, fw, fw_len);
	if (is_err_status(err))
		goto err_open_handle;

	err = cln_fw_handle_diagnose_firmware(handle, fw, fw_len);
	if (is_err_status(err))
		goto err_diagnose_firmware;

	cln_fw_handle_close(handle);

	return ret;

err_diagnose_firmware:
err_open_handle:
	eee_mfree(fw);
	return -1;
}

static struct option long_opts[] = {
	{ 0 },	/* NULL terminated */
};

cln_fwtool_command_t command_diagnosis = {
	.name = T("diagnosis"),
	.optstring = T("-"),
	.long_opts = long_opts,
	.parse_arg = parse_arg,
	.show_usage = show_usage,
	.run = run_diagnosis,
};