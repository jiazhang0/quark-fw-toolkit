/*
 * Firmware Tool for Intel Clanton SoC
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

#define CLN_FWTOOL_MAX_COMMANDS			16

static int opt_quiet;
static char *opt_input_file;
static cln_fwtool_command_t *curr_command;
static unsigned int cln_fwtool_nr_command;
static cln_fwtool_command_t *cln_fwtool_commands[CLN_FWTOOL_MAX_COMMANDS];

static void
show_banner(void)
{
	info_cont(T("\nFirmware Tool for Intel Clanton SoC ")
		  T("Intel Quark X1000 series)\n"));
	info_cont(T("Copyright (c) 2015, ")
		  T("Lans Zhang <jia.zhang@windriver.com>\n"));
	info_cont(T("Version: %s\n\n"), T(VERSION));
}

static void
show_version(void)
{
	info_cont(T("%s\n"), T(VERSION));
}

static void
show_usage(const tchar_t *prog)
{
	info_cont(T("\nusage: %s <options> <command> <file> [<args>]\n"),
		  prog);
	info_cont(T("\noptions:\n"));
	info_cont(T("  --help, -h: Print this help information\n"));
	info_cont(T("  --version, -V: Show version number\n"));
	info_cont(T("  --verbose, -v: Show verbose messages\n"));
	info_cont(T("  --quite, -q: Don't show banner information\n"));
	info_cont(T("\ncommand:\n"));
	info_cont(T("  help: Display the help information for the ")
		  T("specified command\n"));
	info_cont(T("  show: Display the details of firmware\n"));
	info_cont(T("  sbembed: Embed the keys for UEFI Secure Boot ")
		  T("enablement\n"));
	info_cont(T("  capsule: Generate capsule image\n"));
	info_cont(T("\nfile:\n"));
	info_cont(T("  Input file to be parsed\n"));
	info_cont(T("\nargs:\n"));
	info_cont(T("  Run `%s help <command>` for the details\n"), prog);
}

int
cln_fwtool_add_command(cln_fwtool_command_t *cmd)
{
	if (!cmd->name || !cmd->optstring || !cmd->long_opts
			|| !cmd->parse_arg)
		return -1;

	if (cln_fwtool_nr_command >= CLN_FWTOOL_MAX_COMMANDS)
		return -1;

	cln_fwtool_commands[cln_fwtool_nr_command++] = cmd;

	return 0;
}

static int
parse_command(char *command, int argc, tchar_t *argv[])
{
	cln_fwtool_command_t *cmd;
	int cmd_required_arg_parsed;
	unsigned int i;

	for (i = 0; i < cln_fwtool_nr_command; ++i) {
		if (!eee_strcmp(command, cln_fwtool_commands[i]->name))
			break;
	}
	if (i == cln_fwtool_nr_command)
		return -1;

	cmd = cln_fwtool_commands[i];
	cmd_required_arg_parsed = 0;

	while (1) {
		int opt;

		opt = getopt_long(argc, argv, cmd->optstring,
				  cmd->long_opts, NULL);
		if (opt == -1)
			break;

		switch (opt) {
		case T('?'):
		case T('h'):
			info_cont(T("\narguments:\n"));
			cmd->show_usage();
			exit(EXIT_SUCCESS);
		case 1:
			if (opt_input_file) {
				err(T("Duplicated input file specified.\n"));
				return -1;
			}
			if (access(optarg, R_OK)) {
				err(T("Invalid input file specified.\n"));
				return -1;
			}
			opt_input_file = optarg;
			break;
		default:
			cmd_required_arg_parsed = 1;
			if (cmd->parse_arg(opt, optarg))
				return -1;
		}
	}

	if (!opt_input_file)
		die("No input file specified\n");

	if (!cmd_required_arg_parsed && !cmd->no_required_arg) {
		cmd->show_usage();
		exit(EXIT_SUCCESS);
	}

	curr_command = cmd;

	return 0;
}

static int
parse_options(int argc, tchar_t *argv[])
{
	tchar_t opts[] = T("-hVvq");
	struct option long_opts[] = {
		{ T("help"), no_argument, NULL, T('h') },
		{ T("version"), no_argument, NULL, T('V') },
		{ T("verbose"), no_argument, NULL, T('v') },
		{ T("quiet"), no_argument, NULL, T('q') },
		{ 0 },	/* NULL terminated */
	};

	while (1) {
		int opt, index;

		opt = getopt_long(argc, argv, opts, long_opts, NULL);
		if (opt == -1)
			break;

		switch (opt) {
		case T('?'):
		case T('h'):
			show_usage(argv[0]);
			exit(EXIT_SUCCESS);
		case T('V'):
			show_version();
			exit(EXIT_SUCCESS);
		case T('v'):
			cln_fw_set_verbosity(1);
			break;
		case T('q'):
			opt_quiet = 1;
			break;
		case 1:
			index = optind;
			optind = 1;
			if (parse_command(optarg, argc - index + 1,
					  argv + index - 1)) {
				err(T("Unrecognized command %s.\n"), optarg);
				show_usage(argv[0]);
				exit(EXIT_FAILURE);
			}
			return 0;
		default:
			return -1;
		}
	}

	if (!curr_command) {
		show_usage(argv[0]);
		exit(EXIT_SUCCESS);
	}

	return 0;
}

int
main(int argc, tchar_t *argv[])
{
	int ret;

	cln_fwtool_add_command(&command_sbembed);
	cln_fwtool_add_command(&command_show);
	cln_fwtool_add_command(&command_capsule);

	ret = parse_options(argc, argv);
	if (ret)
		return ret;

	if (!opt_quiet)
		show_banner();

	return curr_command->run(opt_input_file);
}