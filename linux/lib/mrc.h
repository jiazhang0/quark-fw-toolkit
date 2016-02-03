/*
 * MRC Data Structure
 *
 * Copyright (c) 2015-2016 Wind River Systems, Inc.
 *
 * See "LICENSE" for license terms.
 *
 * Author: Lans Zhang <jia.zhang@windriver.com>
 */

#ifndef __MRC_H__
#define __MRC_H__

#include <eee.h>

#pragma pack(1)

#define MRC_FLAGS_ECC_EN			(1 << 0)
#define MRC_FLAGS_SCRAMBLE_EN			(1 << 1)
#define MRC_FLAGS_MEMTEST_EN			(1 << 2)
#define MRC_FLAGS_TOP_TREE_EN			(1 << 3)	/* 0b DDR "fly-by" topology else 1b DDR "tree" topology. */
#define MRC_FLAGS_WR_ODT_EN			(1 << 4)	/* If set ODR signal is asserted to DRAM devices on writes. */

#define RANK0_EN				(1 << 0)
#define RANK1_EN				(1 << 1)

#define CHAN0_EN				(1 << 0)

struct mrc_parameter {
	/* should match value of platform type/id record as sanity check. */
	uint16_t platform_id;
	/* bit[0] ECC_EN, bit[1] SCRAMBLE_EN, others=RESERVED */
	uint32_t flags;
	/* 0=x8, 1=x16, others=RESERVED. */
	uint8_t dram_width;
	/* 0=DDRFREQ_800, 1=DDRFREQ_1066, others=RESERVED. Only 533MHz SKU support 1066 memory. */
	uint8_t dram_speed;
	/* 0=DDR3, 1=DDR3L, others=RESERVED. */
	uint8_t dram_type;
	/* bit[0] RANK0_EN, bit[1] RANK1_EN, others=RESERVED. */
	uint8_t rank_mask;
	/* bit[0] CHAN0_EN, others=RESERVED. */
	uint8_t chan_mask;
	/* 1=x16, others=RESERVED. */
	uint8_t chan_width;
	/* 0, 1, 2 (mode 2 forced if ecc enabled), others=RESERVED */
	uint8_t addr_mode;
	/* 1=1.95us, 2=3.9us, 3=7.8us, others=RESERVED. REFRESH_RATE. */
	uint8_t sr_int;
	/* 0=normal, 1=extended, others=RESERVED. SR_TEMP_RANGE. */
	uint8_t sr_temp;
	/* 0=34ohm, 1=40ohm, others=RESERVED. RON_VALUE Select MRS1.DIC driver impedance control. */
	uint8_t dram_ron_val;
	/* 0=40ohm, 1=60ohm, 2=120ohm, others=RESERVED. RTT_NOM_VALUE. */
	uint8_t dram_rtt_nom_val;
	/* 0=off, others=RESERVED */
	uint8_t dram_rtt_wr_val;
	/* 0=off, 1=60ohm, 2=120ohm, 3=180ohm, others=RESERVED. RD_ODT_VALUE. */
	uint8_t soc_rd_odt_val;
	/* 0=27ohm, 1=32ohm, 2=40ohm, others=RESERVED. (WR_RON_VALUE select Vref code for DQ DRV PU evaluation using external resistor). */
	uint8_t soc_wr_ron_val;
	/* 0=2.5V/ns, 1=4V/ns, others=RESERVED. (WR_SLEW_RATE). */
	uint8_t soc_wr_slew;
	/* 0=512Mb, 1=Gb, 2=2Gb, 3=4Gb, others=RESERVED. */
	uint8_t dram_density;
	/* ACT to PRE command period in picoseconds. */
	uint32_t tRAS;
	/* Delay from start of internal write transaction to internal read command in picoseconds. */
	uint32_t tWTR;
	/* ACT to ACT command period (JESD79 specific to page size 1K/2K) in picoseconds. */
	uint32_t tRRD;
	/* Four activate window (JESD79 specific to page size 1K/2K) in picoseconds. */
	uint32_t tFAW;
	/* DRAM CAS Latency in clocks. */
	uint8_t tCL;
};

#pragma pack()

#endif	/* __MRC_H__ */