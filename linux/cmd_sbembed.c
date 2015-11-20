/*
 * UEFI Secure Boot embed command
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

#define DEF_OUTPUT_NAME			T("output.bin")

static char *opt_input_file;
static char *opt_pk_file, *opt_kek_file, *opt_db_file, *opt_dbx_file;
static char *opt_output_file = DEF_OUTPUT_NAME;

static void
show_usage(tchar_t *prog)
{
	info_cont(T("\nusage: %s sbembed <file> <args>\n"), prog);
	info_cont(T("Embed the keys for the enablement of UEFI Secure Boot\n"));
	info_cont(T("\nfile:\n"));
	info_cont(T("  Input firmware to be parsed\n"));
	info_cont(T("\nargs:\n"));
	info_cont(T("  --output-file, -o\n")
		  T("    (optional) The output file name to override the ")
		  T("default name \"%s\"\n"), DEF_OUTPUT_NAME);
	info_cont(T("\n  --pk, -p\n")
		  T("    (optional) Specify DER formatted PK file\n"));
	info_cont(T("\n  --kek, -k\n")
		  T("    (optional) Specify DER formatted KEK file\n"));
	info_cont(T("\n  --db, -d\n")
		  T("    (optional) Specify DER formatted DB file\n"));
	info_cont(T("\n  --dbx, -x\n")
		  T("    (optional) Specify DER formatted DBX file\n"));
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
	case 'p':
		if (access(optarg, R_OK)) {
			err(T("Invalid PK file specified\n"));
			return -1;
		}
		opt_pk_file = optarg;
		break;
	case 'k':
		if (access(optarg, R_OK)) {
			err(T("Invalid KEK file specified\n"));
			return -1;
		}
		opt_kek_file = optarg;
		break;
	case 'd':
		if (access(optarg, R_OK)) {
			err(T("Invalid DB file specified\n"));
			return -1;
		}
		opt_db_file = optarg;
		break;
	case 'x':
		if (access(optarg, R_OK)) {
			err(T("Invalid DBX file specified\n"));
			return -1;
		}
		opt_dbx_file = optarg;
		break;
	default:
		return -1;
	}

	return 0;
}

static int
run_sbembed(tchar_t *prog)
{
	uint8_t *fw, *pk, *kek, *db, *dbx, *out;
	unsigned long fw_len, pk_len, kek_len, db_len, dbx_len, out_len;
	err_status_t err;
	int ret;

	if (!opt_input_file)
		die("No input file specified\n");

	if (!opt_pk_file && !opt_kek_file && !opt_db_file && !opt_dbx_file) {
		err(T("Neither PK, KEK, DB and DBX specified\n"));
		return -1;
	}

	ret = load_file(opt_input_file, &fw, &fw_len);
	if (ret)
		return ret;

	if (opt_pk_file) {
		ret = load_file(opt_pk_file, &pk, &pk_len);
		if (ret)
			goto err_load_pk;
	} else {
		pk = NULL;
		pk_len = 0;
	}

	if (opt_kek_file) {
		ret = load_file(opt_kek_file, &kek, &kek_len);
		if (ret)
			goto err_load_kek;
	} else {
		kek = NULL;
		kek_len = 0;
	}

	if (opt_db_file) {
		ret = load_file(opt_db_file, &db, &db_len);
		if (ret)
			goto err_load_db;
	} else {
		db = NULL;
		db_len = 0;
	}

	if (opt_dbx_file) {
		ret = load_file(opt_dbx_file, &dbx, &dbx_len);
		if (ret)
			goto err_load_dbx;
	} else {
		dbx = NULL;
		dbx_len = 0;
	}

	err = cln_fw_util_embed_sb_keys(fw, fw_len, pk, pk_len, kek, kek_len,
					db, db_len, dbx, dbx_len,
					(void **)&out, &out_len);
	if (is_err_status(err))
		goto err_embde_key;

	ret = save_output_file(opt_output_file, out, out_len);
	free(out);

	if (!ret)
		info(T("Saved the ouput firmware\n"));
	else
		err(T("Failed to save the ouput firmware\n"));

err_embde_key:
	free(dbx);

err_load_dbx:
	free(db);

err_load_db:
	free(kek);

err_load_kek:
	free(pk);

err_load_pk:
	free(fw);

	return ret;
}

static struct option long_opts[] = {
	{ T("output-file"), required_argument, NULL, T('o') },
	{ T("pk"), required_argument, NULL, T('p') },
	{ T("kek"), required_argument, NULL, T('k') },
	{ T("db"), required_argument, NULL, T('d') },
	{ T("dbx"), required_argument, NULL, T('x') },
	{ 0 },	/* NULL terminated */
};

cln_fwtool_command_t command_sbembed = {
	.name = T("sbembed"),
	.optstring = T("-o:p:k:d:x:"),
	.long_opts = long_opts,
	.parse_arg = parse_arg,
	.show_usage = show_usage,
	.run = run_sbembed,
};