/*
 * cln_fwtool command definitions
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

int
cln_fwtool_add_command(cln_fwtool_command_t *cmd);

cln_fwtool_command_t *
cln_fwtool_find_command(char *command);

#endif	/* __CLN_FWTOOL_H__ */