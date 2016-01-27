/*
 * Parser internal APIs
 *
 * Copyright (c) 2015 Wind River Systems, Inc.
 *
 * See "LICENSE" for license terms.
 *
 * Author: Lans Zhang <jia.zhang@windriver.com>
 */

#ifndef __INTERNAL_H__
#define __INTERNAL_H__

#include <eee.h>
#include <err_status.h>
#include "buffer_stream.h"
#include "bcll.h"
#include "mfh.h"

#define stringify(x)		#x

#define offsetof(type, member)	((unsigned long)&((type *)0)->member)

#define container_of(ptr, type, member)	({	\
	const __typeof__(((type *)0)->member) *__ptr = (ptr);	\
	(type *)((char *)__ptr - offsetof(type, member));})

#define align_up(x, n)	(((x) + ((n) - 1)) & ~((n) - 1))
#define aligned(x, n)	(!!((x) & ((n) - 1)))

typedef struct {
	bcll_t link;
	buffer_stream_t bs;
} cln_fw_pdata_item_t;

typedef struct {
	buffer_stream_t firmware;
	buffer_stream_t mfh;
	buffer_stream_t pdata;
	buffer_stream_t skm;
	/* For output */
	buffer_stream_t pdata_header;
	void *pdata_item;
	bcll_t pdata_item_list;
	unsigned long nr_pdata_item;
} cln_fw_parser_t;

err_status_t
cln_fw_parser_create(void *fw, unsigned long fw_len,
		     cln_fw_parser_t **out);

void
cln_fw_parser_destroy(cln_fw_parser_t *parser);

err_status_t
cln_fw_parser_parse(cln_fw_parser_t *parser);

err_status_t
cln_fw_parser_embed_key(cln_fw_parser_t *ctx, cln_fw_sb_key_t key, void *in,
			unsigned long in_len);

err_status_t
cln_fw_parser_flush(cln_fw_parser_t *parser, void *fw_buf,
		    unsigned long fw_buf_len);

err_status_t
cln_fw_parser_generate_capsule(cln_fw_parser_t *parser, int bios_only,
			       void **out, unsigned long *out_len);

err_status_t
cln_fw_parser_diagnose_firmware(cln_fw_parser_t *parser);

/* MFH functions */

unsigned long
mfh_header_size(void);
long
mfh_offset(void);
err_status_t
mfh_probe(void *mfh_buf, unsigned long *mfh_buf_len);
err_status_t
mfh_show(void *mfh_buf, unsigned long mfh_buf_len);
err_status_t
mfh_show_fw_version(void *mfh_buf, unsigned long mfh_buf_len);

/* Signed key module functions */

/* Platform Data functions */

unsigned long
platform_data_header_size(void);

long
platform_data_offset(void);

unsigned long
platform_data_max_size(void);

unsigned long
platform_data_size(void *pdata_buf);

unsigned long
platform_data_all_item_size(void *pdata_buf);

unsigned long
platform_data_item_size(void *pdata_item_buf);

uint16_t
platform_data_item_id(void *pdata_item_buf);

void
platform_data_update_header(void *pdata, void *pdata_item,
			    unsigned long pdata_item_len);

uint32_t
platform_data_cert_header(void *pdata_item_buf);

err_status_t
platform_data_probe(void *pdata_buf, unsigned long *pdata_buf_len);

void
__platform_data_show(void *pdata_buf, unsigned long pdata_buf_len);

err_status_t
platform_data_show(void *pdata_buf, unsigned long pdata_buf_len);

err_status_t
platform_data_parse(void *pdata_buf, unsigned long pdata_buf_len,
		    void **out_pdata_header_buf, void **out_pdata_item_buf,
		    unsigned long *out_nr_pdata_item);

err_status_t
platform_data_search_item(void *pdata_buf, unsigned long pdata_buf_len,
			  uint16_t id);

err_status_t
platform_data_create_item(uint16_t id, uint16_t version, const char desc[10],
			  uint8_t *data, uint16_t data_len, void **out,
			  unsigned long *out_len);

err_status_t
platform_data_update_item(void *pdata_item_buf,
			  unsigned long pdata_item_buf_len,
			  uint16_t id, uint16_t *version,
			  const char desc[10], uint8_t *data,
			  uint16_t data_len, void **out,
			  unsigned long *out_len);

uint32_t
crc32(uint8_t *buf, uint32_t size);

/* Capsule functions */

err_status_t
capsule_create_header(unsigned long payload_len, void **out,
		      unsigned long *out_len);

err_status_t
capsule_create_update_entry(uint32_t addr, uint32_t size,
			    void **out, unsigned long *out_len);

#endif	/* __INTERNAL_H__ */