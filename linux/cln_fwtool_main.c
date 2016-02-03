/*
 * Firmware Tool for Intel Clanton SoC
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

#define CLN_FWTOOL_MAX_COMMANDS			16

static int opt_quiet;
static cln_fwtool_command_t *curr_command;
static unsigned int cln_fwtool_nr_command;
static cln_fwtool_command_t *cln_fwtool_commands[CLN_FWTOOL_MAX_COMMANDS];

static void
show_banner(void)
{
	info_cont(T("\nFirmware Tool for Intel Clanton SoC ")
		  T("(Intel Quark SoC X1000 series)\n"));
	info_cont(T("Copyright (c) 2015-2016 ")
		  T("Wind River Systems, Inc.\n"));
	info_cont(T("Author: Lans Zhang <jia.zhang@windriver.com>\n"));
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
	info_cont(T("usage: %s <options> <command> <file> [<args>]\n"),
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
	info_cont(T("  diagnosis: Give the diagosis information\n"));
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

cln_fwtool_command_t *
cln_fwtool_find_command(char *command)
{
	unsigned int i;

	for (i = 0; i < cln_fwtool_nr_command; ++i) {
		if (!eee_strcmp(command, cln_fwtool_commands[i]->name))
			break;
	}
	if (i == cln_fwtool_nr_command)
		return NULL;

	return cln_fwtool_commands[i];
}

static int
parse_command(char *prog, char *command, int argc, tchar_t *argv[])
{
	cln_fwtool_command_t *cmd;
	int cmd_arg_parsed;

	dbg(T("Input command: %s\n"), command);

	cmd = cln_fwtool_find_command(command);
	if (!cmd) {
		err(T("Unrecognized command: %s\n"), command);
		return -1;
	}

	cmd_arg_parsed = 0;

	while (1) {
		int opt;

		opt = getopt_long(argc, argv, cmd->optstring,
				  cmd->long_opts, NULL);
		if (opt == -1)
			break;

		switch (opt) {
		case T('?'):
			err(T("Unrecongnized argument\n"));
			return -1;
		default:	/* Command arguments */
			cmd_arg_parsed = 1;
			if (cmd->parse_arg(opt, optarg)) {
				if (eee_strcmp(command, T("help")))
					cmd->show_usage(prog);
				return -1;
			}
		}
	}

	if (!cmd_arg_parsed && !cln_fw_util_cpu_is_clanton()) {
		err(T("Nothing specified\n"));
		if (eee_strcmp(cmd->name, T("help")))
			err(T(". Run \"%s help %s \" for the help info\n"),
			    prog, command);
		else
			err_cont(T("\n"));
		return -1;
	}

	curr_command = cmd;
	if (!curr_command) {
		show_usage(argv[0]);
		exit(EXIT_SUCCESS);
	}

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
			err(T("Unrecognized option\n"));
			return -1;
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
			if (parse_command(argv[0], optarg, argc - index + 1,
					  argv + index - 1)) 
				exit(EXIT_FAILURE);
			return 0;
		default:
			return -1;
		}
	}

	return 0;
}

int
main(int argc, tchar_t *argv[])
{
	int ret;

	/*
	 * For static link which doesn't link constructor/destructor if
	 * without explicitly call them.
	 */
	libclnfw_init();

	cln_fwtool_add_command(&command_help);
	cln_fwtool_add_command(&command_sbembed);
	cln_fwtool_add_command(&command_show);
	cln_fwtool_add_command(&command_capsule);
	cln_fwtool_add_command(&command_diagnosis);

	ret = parse_options(argc, argv);
	if (ret)
		return ret;

	if (!opt_quiet)
		show_banner();

	if (!curr_command) {
		show_usage(argv[0]);
		exit(EXIT_SUCCESS);
	}

	return curr_command->run(argv[0]);
}
