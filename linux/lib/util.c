/*
 * Utility routines
 *
 * Copyright (c) 2015-2016 Wind River Systems, Inc.
 *
 * See "LICENSE" for license terms.
 *
 * Author: Lans Zhang <jia.zhang@windriver.com>
 */

#include <cln_fw.h>
#include <err_status.h>
#include <eee.h>
#include "uefi.h"
#include "buffer_stream.h"
#include "platform_data.h"

static int show_verbose;

int
cln_fw_verbose(void)
{
	return show_verbose;
}

void
cln_fw_set_verbosity(int verbose)
{
	show_verbose = verbose;
}

/*
 * We need to manually preserve ebx since we are compiling PIC code.
 * This means we cannot use "=b" for the 2nd output register.
 */
#ifdef __x86_64__
static void
x86_cpuid(uint32_t index, uint32_t *ax)
{
	__asm__ __volatile__(
		"pushq %%rbx\n\t"
		"cpuid\n\t"
		"popq %%rbx\n\t"
		:"=a"(*ax)
		:"0"(index)
		:"%ecx", "%edx"
	);
}
#else
static void
x86_cpuid(uint32_t index, uint32_t *ax)
{
	__asm__ __volatile__(
		"pushl %%ebx\n\t"
		"cpuid\n\t"
		"popl %%ebx\n\t"
		:"=a"(*ax)
		:"0"(index)
		:"%ecx", "%edx"
	);
}
#endif

static int
cpu_is_lakemont_gen1(void)
{
	struct {	/* Version information returned by CPUID.1:EAX */
		uint32_t Stepping:4;
		uint32_t Model:4;
		uint32_t Family:4;
		uint32_t ProcessorType:2;
		uint32_t :2;
		uint32_t ExtendedModel:4;
		uint32_t ExtendedFamily:8;
		uint32_t :4;
	} ax;

	x86_cpuid(1U, (uint32_t *)&ax);

	return ax.Family == 0x5 && ax.Model == 0x9 && ax.Stepping == 0;
}

#define PCI_CONFIG_ADDRESS	0xcf8
#define PCI_CONFIG_DATA		0xcfc

#define PCI_ADDRESS(bus, dev, fn, reg) \
	(0x80000000 | ((reg & 0xf00) << 16) | (bus << 16) \
	| (dev << 11) | (fn << 8) | (reg & 0xfc))

static inline uint8_t
io_read8(uint16_t port)
{
	uint8_t val;

	__asm__ __volatile__("inb %1, %0"
			     :"=a"(val)
			     :"Nd"(port));

	return val;
}

static inline uint16_t
io_read16(uint16_t port)
{
	uint16_t val;

	__asm__ __volatile__("inw %1, %0"
			     :"=a"(val)
			     :"Nd"(port));

	return val;
}

static inline void
io_write32(uint32_t val, uint16_t port)
{
	__asm__ __volatile__("outl %0, %1"
			     :
			     :"a"(val), "Nd"(port));
}

static inline
uint8_t pci_config_read8(unsigned int bus, unsigned int dev,
			 unsigned int fn, int reg)
{
	io_write32(PCI_ADDRESS(bus, dev, fn, reg), PCI_CONFIG_ADDRESS);
	return io_read8(PCI_CONFIG_DATA + (reg & 3));
}

static inline
uint16_t pci_config_read16(unsigned int bus, unsigned int dev,
			   unsigned int fn, int reg)
{
	io_write32(PCI_ADDRESS(bus, dev, fn, reg), PCI_CONFIG_ADDRESS);
	return io_read16(PCI_CONFIG_DATA + (reg & 2));
}

static int
grsec_enabled(void)
{
	return !access("/dev/grsec", R_OK);
}

static int
clanton_is_A0_stepping(void)
{
	uint16_t vendor, device;
	uint8_t rev;

	/*
	 * Note that grsec denies the access to I/O space
	 */
	if (grsec_enabled())
		return 1;

	/* Vendor ID Register */
	vendor = pci_config_read16(0, 0, 0, 0);
	/* Device ID Register */
	device = pci_config_read16(0, 0, 0, 2);
	/* Revision ID Register */
	rev = pci_config_read8(0, 0, 0, 8);

	return vendor == 0x8086 && device == 0x0958 && rev == 0x00;
}

int
cln_fw_util_cpu_is_clanton(void)
{
	return cpu_is_lakemont_gen1() && clanton_is_A0_stepping();
}

err_status_t
cln_fw_util_show_firmware(void *fw, unsigned long fw_len)
{
	cln_fw_handle_t handle;
	err_status_t err;

	if (!fw || !fw_len)
		return CLN_FW_ERR_INVALID_PARAMETER;

	handle = NULL;
	err = cln_fw_handle_open(&handle, fw, fw_len);
	if (is_err_status(err))
		return err;

	cln_fw_handle_show_all(handle);

	cln_fw_handle_close(handle);

	return err;
}

static err_status_t
der2db(void **out, unsigned long *out_len,
	void *der, unsigned long der_len)
{
	void *buf;
	unsigned long buf_len;
	uint32_t *header;
	EFI_SIGNATURE_DATA *data;
	EFI_GUID owner = { 0xf134da79, 0xb948, 0x499a,
				{0xb1, 0x22, 0x26, 0xa9, 0xf2,
				0x8e, 0xd7, 0xa4} };
	buffer_stream_t bs;

	buf_len = sizeof(*header) + sizeof(*data) + der_len;
	buf = eee_malloc(buf_len);
	if (!buf)
		return CLN_FW_ERR_OUT_OF_MEM;

	bs_init(&bs, buf, buf_len);
	bs_post_get(&bs, (void **)&header, sizeof(*header));
	bs_post_get_at(&bs, (void **)&data, sizeof(*data), sizeof(*header));
	bs_put(&bs, der, der_len);

	*header = PDATA_DB_CERT_HEADER;
	data->SignatureOwner = owner;

	*out = buf;
	*out_len = buf_len;

	return CLN_FW_ERR_NONE;
}

static err_status_t
der2kek(void **out, unsigned long *out_len,
	void *der, unsigned long der_len)
{
	void *buf;
	unsigned long buf_len;
	uint32_t *header;
	buffer_stream_t bs;

	buf_len = sizeof(*header) + der_len;
	buf = eee_malloc(buf_len);
	if (!buf)
		return CLN_FW_ERR_OUT_OF_MEM;

	bs_init(&bs, buf, buf_len);
	bs_post_get(&bs, (void **)&header, sizeof(*header));
	bs_put(&bs, der, der_len);

	*header = PDATA_KEK_CERT_HEADER;

	*out = buf;
	*out_len = buf_len;

	return CLN_FW_ERR_NONE;
}

err_status_t
cln_fw_util_embed_sb_keys(void *fw, unsigned long fw_len,
			  void *pk, unsigned long pk_len,
			  void *kek, unsigned long kek_len,
			  void *db, unsigned long db_len,
			  void *dbx, unsigned long dbx_len,
			  void **out, unsigned long *out_len)
{
	cln_fw_handle_t handle;
	void *extra_buf;
	unsigned long extra_buf_len;
	err_status_t err;

	if (!fw || !fw_len)
		return CLN_FW_ERR_INVALID_PARAMETER;

	if (!pk && !kek && !db && !dbx)
		return CLN_FW_ERR_INVALID_PARAMETER;

	if (pk && !pk_len)
		return CLN_FW_ERR_INVALID_PARAMETER;

	if (kek && !kek_len)
		return CLN_FW_ERR_INVALID_PARAMETER;

	if (db && !db_len)
		return CLN_FW_ERR_INVALID_PARAMETER;

	if (dbx && !dbx_len)
		return CLN_FW_ERR_INVALID_PARAMETER;

	handle = NULL;
	err = cln_fw_handle_open(&handle, fw, fw_len);
	if (is_err_status(err))
		return err;

	if (pk) {
		err = cln_fw_handle_embed_key(handle, CLN_FW_SB_KEY_PK, pk,
					      pk_len);
		if (is_err_status(err))
			goto err_embed_key;
	}

	if (kek) {
		extra_buf = NULL;
		extra_buf_len = 0;
		err = der2kek(&extra_buf, &extra_buf_len, kek, kek_len);
		if (is_err_status(err))
			goto err_der2kek;

		err = cln_fw_handle_embed_key(handle, CLN_FW_SB_KEY_KEK,
					      extra_buf, extra_buf_len);
		eee_mfree(extra_buf);
		if (is_err_status(err))
			goto err_embed_key;
	}

	if (db) {
		extra_buf = NULL;
		extra_buf_len = 0;
		err = der2db(&extra_buf, &extra_buf_len, db, db_len);
		if (is_err_status(err))
			goto err_der2db;

		err = cln_fw_handle_embed_key(handle, CLN_FW_SB_KEY_DB,
					      extra_buf, extra_buf_len);
		eee_mfree(extra_buf);
		if (is_err_status(err))
			goto err_embed_key;
	}

	if (dbx) {
		extra_buf = NULL;
		extra_buf_len = 0;
		err = der2db(&extra_buf, &extra_buf_len, dbx, dbx_len);
		if (is_err_status(err))
			goto err_der2db;

		err = cln_fw_handle_embed_key(handle, CLN_FW_SB_KEY_DBX,
					      extra_buf, extra_buf_len);
		eee_mfree(extra_buf);
		if (is_err_status(err))
			goto err_embed_key;
	}

	err = cln_fw_handle_flush(handle, out, out_len);
	if (is_err_status(err))
		goto err_embed_key;

err_embed_key:
err_der2db:
err_der2kek:
	cln_fw_handle_close(handle);

	return err;
}

err_status_t
cln_fw_util_generate_capsule(void *fw, unsigned long fw_len,
			     int bios_only, void **out,
			     unsigned long *out_len)
{
	cln_fw_handle_t handle;
	err_status_t err;

	if (!fw || !fw_len)
		return CLN_FW_ERR_INVALID_PARAMETER;

	handle = NULL;
	err = cln_fw_handle_open(&handle, fw, fw_len);
	if (is_err_status(err))
		return err;

	err = cln_fw_handle_generate_capsule(handle, bios_only, out, out_len);
	cln_fw_handle_close(handle);

	if (is_err_status(err))
		return err;

	return CLN_FW_ERR_NONE;
}