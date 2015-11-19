/*
 * Firmware show command
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

static char *opt_output_file = DEF_OUTPUT_NAME;

static void
show_usage(void)
{
	info_cont(T("\t--output-file, -o: (optional) The output file name ")
		  T("to override the default name \"%s\"\n"),
		  DEF_OUTPUT_NAME);
}

static int
parse_arg(int opt, char *optarg)
{
	switch (opt) {
	case 'o':
		opt_output_file = optarg;
		break;
	default:
		return -1;
	}

	return 0;
}

static int
run_capsule(const char *file_path)
{
	void *fw, *out;
	unsigned long fw_len, out_len;
	err_status_t err;
	int ret;

	ret = load_file(file_path, (uint8_t **)&fw, &fw_len);
	if (ret)
		return ret;

	err = cln_fw_util_generate_capsule(fw, fw_len, &out, &out_len);
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
	{ T("help"), no_argument, NULL, T('h') },
	{ T("output"), required_argument, NULL, T('o') },
	{ 0 },	/* NULL terminated */
};

cln_fwtool_command_t command_capsule = {
	.name = T("capsule"),
	.optstring = T("ho:"),
	.no_required_arg = 1,
	.long_opts = long_opts,
	.parse_arg = parse_arg,
	.show_usage = show_usage,
	.run = run_capsule,
};