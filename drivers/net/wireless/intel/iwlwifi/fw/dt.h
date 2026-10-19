/* SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause */
/*
 * Copyright (C) 2026 Intel Corporation
 */
#ifndef __iwl_fw_dt_h__
#define __iwl_fw_dt_h__

#include "fw/runtime.h"

/*
 * Device Tree property parsing for Intel Wi-Fi 7 BE200 PCIe adapters.
 *
 * On platforms without UEFI variables or ACPI methods, regulatory and
 * power configuration can be supplied via Device Tree properties in the
 * Wi-Fi PCIe device node.
 *
 * DT is the lowest-priority source: UEFI variables take precedence, then
 * ACPI methods, and finally these DT properties.
 */

void iwl_dt_init(struct iwl_fw_runtime *fwrt);

static inline bool iwl_dt_is_compatible(struct iwl_fw_runtime *fwrt)
{
	return fwrt->dt_compatible;
}

int iwl_dt_get_wrds_table(struct iwl_fw_runtime *fwrt);
int iwl_dt_get_uats_table(struct iwl_fw_runtime *fwrt);
int iwl_dt_get_dsm(struct iwl_fw_runtime *fwrt, enum iwl_dsm_funcs func,
		   u32 *value);
int iwl_dt_get_pwr_limit(struct iwl_fw_runtime *fwrt, u64 *data);
int iwl_dt_get_wbem(struct iwl_fw_runtime *fwrt, u32 *data);
int iwl_dt_get_puncturing(struct iwl_fw_runtime *fwrt);

/* Not yet implemented in DT - return -ENOENT as placeholder */
static inline int iwl_dt_get_ppag_table(struct iwl_fw_runtime *fwrt)
{ return -ENOENT; }
static inline int iwl_dt_get_tas_table(struct iwl_fw_runtime *fwrt,
				       struct iwl_tas_data *data)
{ return -ENOENT; }
static inline int iwl_dt_get_ewrd_table(struct iwl_fw_runtime *fwrt)
{ return -ENOENT; }
static inline int iwl_dt_get_wgds_table(struct iwl_fw_runtime *fwrt)
{ return -ENOENT; }
static inline int iwl_dt_get_phy_filters(struct iwl_fw_runtime *fwrt)
{ return -ENOENT; }
static inline int iwl_dt_get_mcc(struct iwl_fw_runtime *fwrt, char *data)
{ return -ENOENT; }
static inline int iwl_dt_get_eckv(struct iwl_fw_runtime *fwrt, u32 *data)
{ return -ENOENT; }
static inline int iwl_dt_get_dsbr(struct iwl_fw_runtime *fwrt, u32 *data)
{ return -ENOENT; }
static inline int iwl_dt_get_wsss_table(struct iwl_fw_runtime *fwrt)
{ return -ENOENT; }
static inline int iwl_dt_get_ewss_table(struct iwl_fw_runtime *fwrt)
{ return -ENOENT; }

#endif /* __iwl_fw_dt_h__ */
