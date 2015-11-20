/*
 * Capsule generation command
 *
 * Copyright (c) 2015, Lans Zhang <jia.zhang@windriver.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <eee.h>
#include <cln_fw.h>
#include <err_status.h>
#include "cln_fwtool.h"

#define DEF_OUTPUT_NAME			T("output_unsigned.cap")

static char *opt_input_file;
static char *opt_output_file = DEF_OUTPUT_NAME;
static int opt_bios_only;

static void
show_usage(tchar_t *prog)
{
	info_cont(T("\nusage: %s capsule <file> <args>\n"), prog);
	info_cont(T("Generate the capsule image\n"));
	info_cont(T("\nfile:\n"));
	info_cont(T("  Input firmware to be parsed\n"));
	info_cont(T("\nargs:\n"));
	info_cont(T("\n  --output-file, -o\n")
		  T("    (optional) The output file name to override the ")
		  T("default name \"%s\"\n"), DEF_OUTPUT_NAME);
	info_cont(T("\n  --bios-only, -b\n")
		  T("    (optional) enables the generated capsule contains ")
		  T("the BIOS part in firmware image only.\n")
		  T("    By default, the entire input firmware image is ")
		  T("wrapped with capsule header\n"));
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
	case 'o':
		opt_output_file = optarg;
		break;
	case 'b':
		opt_bios_only = 1;
		break;
	default:
		return -1;
	}

	return 0;
}

static int
run_capsule(tchar_t *prog)
{
	void *fw, *out;
	unsigned long fw_len, out_len;
	err_status_t err;
	int ret;

	if (!opt_input_file)
		die("No input file specified\n");

	ret = load_file(opt_input_file, (uint8_t **)&fw, &fw_len);
	if (ret)
		return ret;

	err = cln_fw_util_generate_capsule(fw, fw_len, opt_bios_only,
					   &out, &out_len);
	if (is_err_status(err))
		ret = -1;

	free(fw);

	if (!ret) {
		ret = save_output_file(opt_output_file, out, out_len);
		if (!ret)
			free(out);
	}

	if (!ret)
		info(T("Saved the unsigned capsule\n"));
	else
		err(T("Failed to save the unsigned capsule\n"));

	return ret;
}

static struct option long_opts[] = {
	{ T("output"), required_argument, NULL, T('o') },
	{ T("bios-only"), no_argument, NULL, T('b') },
	{ 0 },	/* NULL terminated */
};

cln_fwtool_command_t command_capsule = {
	.name = T("capsule"),
	.optstring = T("-o:b"),
	.long_opts = long_opts,
	.parse_arg = parse_arg,
	.show_usage = show_usage,
	.run = run_capsule,
};