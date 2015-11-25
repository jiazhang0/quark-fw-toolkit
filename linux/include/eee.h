/*
 * Linux-specific helpers
 *
 * Copyright (c) 2015 Wind River Systems, Inc.
 *
 * See "LICENSE" for license terms.
 *
 * Author: Lans Zhang <jia.zhang@windriver.com>
 */

#ifndef EEE_H
#define EEE_H

#define _GNU_SOURCE

#include <ctype.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>

#define T(str)			str

typedef unsigned char		uint8_t;
typedef unsigned short		uint16_t;
typedef unsigned int		uint32_t;
typedef unsigned long long	uint64_t;
typedef char			tchar_t;
typedef unsigned int		bool;

#define die(fmt, ...)	\
	do {	\
		fprintf(stderr, T("FAULT: ") fmt, ##__VA_ARGS__);	\
		exit(EXIT_FAILURE);	\
	} while (0)

#define dbg(fmt, ...) \
	do {	\
		if (cln_fw_verbose())	\
			fprintf(stdout, T("DEBUG: ") fmt, ##__VA_ARGS__);	\
	} while (0)

#define dbg_cont(fmt, ...) \
	do {	\
		if (cln_fw_verbose())	\
			fprintf(stdout, fmt, ##__VA_ARGS__);	\
	} while (0)

#define info(fmt, ...) \
	fprintf(stdout, T("INFO: ") fmt, ##__VA_ARGS__)

#define info_cont(fmt, ...) \
	fprintf(stdout, fmt, ##__VA_ARGS__)

#define warn(fmt, ...) \
	fprintf(stderr, T("WARNING: ") fmt, ##__VA_ARGS__)

#define err(fmt, ...) \
	fprintf(stderr, T("ERROR: ") fmt, ##__VA_ARGS__)

#define err_cont(fmt, ...) \
	fprintf(stdout, fmt, ##__VA_ARGS__)

int
load_file(const char *file_path, uint8_t **out, unsigned long *out_len);

int
save_output_file(const char *file_path, uint8_t *buf, unsigned long size);

int
eee_strcmp(const char *s1, const char *s2);

int
eee_memcmp(const void *s1, const void *s2, unsigned long n);

void *
eee_memcpy(void *dst, const void *src, unsigned long size);

void *
eee_memset(void *s, int c, unsigned long n);

void *
eee_malloc(unsigned long size);

void *
eee_mrealloc(void *buf, unsigned long size, unsigned long new_size);

void *
eee_mrealloc_aligned(void *buf, unsigned long size, unsigned long *new_size);

void
eee_mfree(void *buf);

#endif	/* EEE_H */