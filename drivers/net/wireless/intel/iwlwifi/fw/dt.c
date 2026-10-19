// SPDX-License-Identifier: GPL-2.0 OR BSD-3-Clause
/*
 * Copyright (C) 2026 Intel Corporation
 */
#include <linux/of.h>
#include "iwl-drv.h"
#include "iwl-debug.h"
#include "fw/runtime.h"
#include "fw/regulatory.h"
#include "dt.h"

/* PCI compatible strings for supported hardware */
static const char * const iwl_dt_compatible[] = {
	"pci8086,272b",	/* Intel Wi-Fi 7 BE200 */
	NULL
};

void iwl_dt_init(struct iwl_fw_runtime *fwrt)
{
	struct device_node *node = dev_of_node(fwrt->dev);
	const char * const *compat;

	if (!node)
		return;

	for (compat = iwl_dt_compatible; *compat; compat++) {
		if (of_device_is_compatible(node, *compat)) {
			fwrt->dt_compatible = true;
			return;
		}
	}

	IWL_ERR(fwrt,
		"DT node compatible does not match any supported hardware - ignoring DT\n");
}
IWL_EXPORT_SYMBOL(iwl_dt_init);

/* intel,wcpe - channel puncturing enable bits */
#define IWL_DT_WCPE_USA_EN		BIT(0)	/* USA */
#define IWL_DT_WCPE_CANADA_EN		BIT(1)	/* Canada */
#define IWL_DT_WCPE_MASK \
	(IWL_DT_WCPE_USA_EN | IWL_DT_WCPE_CANADA_EN)

/* SAR layout for "intel,wrds": 4 chains x 12 subbands */
#define IWL_DT_WRDS_NUM_CHAINS		4
#define IWL_DT_WRDS_NUM_SUBBANDS	12
/* Total cell count: 1-cell mode header + SAR data */
#define IWL_DT_WRDS_MAX_CELLS \
	(1 + IWL_DT_WRDS_NUM_CHAINS * IWL_DT_WRDS_NUM_SUBBANDS)

int iwl_dt_get_wrds_table(struct iwl_fw_runtime *fwrt)
{
	struct device_node *node = dev_of_node(fwrt->dev);
	u32 buf[IWL_DT_WRDS_MAX_CELLS];
	int ret;

	if (!node)
		return -ENOENT;

	ret = of_property_read_u32_array(node, "intel,wrds", buf,
					 IWL_DT_WRDS_MAX_CELLS);
	if (ret) {
		IWL_DEBUG_RADIO(fwrt, "DT: intel,wrds read failed (%d)\n", ret);
		return ret;
	}

	IWL_DEBUG_RADIO(fwrt,
			"DT: reading intel,wrds (%u chains, %u subbands)\n",
			IWL_DT_WRDS_NUM_CHAINS, IWL_DT_WRDS_NUM_SUBBANDS);

	/* buf[0]: mode; bit 0 (IWL_SAR_ENABLE_MSK) enables SAR */

	BUILD_BUG_ON(IWL_DT_WRDS_NUM_CHAINS > BIOS_SAR_MAX_CHAINS_PER_PROFILE);
	BUILD_BUG_ON(IWL_DT_WRDS_NUM_SUBBANDS > BIOS_SAR_MAX_SUB_BANDS_NUM);

	/* buf[1..]: chains[0..N-1] x subbands[0..M-1] in row-major order */
	for (int i = 0; i < IWL_DT_WRDS_NUM_CHAINS; i++) {
		for (int j = 0; j < IWL_DT_WRDS_NUM_SUBBANDS; j++) {
			u32 val = buf[1 + i * IWL_DT_WRDS_NUM_SUBBANDS + j];

			if (val > U8_MAX) {
				IWL_DEBUG_RADIO(fwrt,
						"DT: intel,wrds OOB [%u][%u]=%u\n",
						i, j, val);
				return -EINVAL;
			}
			fwrt->sar_profiles[0].chains[i].subbands[j] = (u8)val;
		}
	}

	if (buf[0] & IWL_SAR_ENABLE_MSK)
		fwrt->sar_profiles[0].enabled = true;

	return 0;
}

int iwl_dt_get_uats_table(struct iwl_fw_runtime *fwrt)
{
	size_t map_size = sizeof(fwrt->ap_type_cmd.mcc_to_ap_type_map);
	struct device_node *node = dev_of_node(fwrt->dev);
	const u8 *prop_data;
	int len;

	if (!node)
		return -ENOENT;

	prop_data = of_get_property(node, "intel,uats", &len);
	if (!prop_data)
		return -ENOENT;

	if (len != (int)map_size) {
		IWL_DEBUG_RADIO(fwrt,
				"DT: intel,uats bad length %d (expected %zu)\n",
				len, map_size);
		return -EINVAL;
	}

	IWL_DEBUG_RADIO(fwrt, "DT: reading intel,uats\n");
	memcpy(fwrt->ap_type_cmd.mcc_to_ap_type_map, prop_data, map_size);
	fwrt->ap_type_cmd_valid = true;
	return 0;
}
IWL_EXPORT_SYMBOL(iwl_dt_get_uats_table);

int iwl_dt_get_dsm(struct iwl_fw_runtime *fwrt, enum iwl_dsm_funcs func,
		   u32 *value)
{
	struct device_node *node = dev_of_node(fwrt->dev);
	const char *str;
	int ret;

	if (!node)
		return -ENOENT;

	switch (func) {
	case DSM_FUNC_DISABLE_SRD:
		ret = of_property_read_string(node, "intel,srd", &str);
		if (!ret) {
			if (!strcmp(str, "active-scan"))
				*value = 0;
			else if (!strcmp(str, "passive-scan"))
				*value = 1;
			else if (!strcmp(str, "disabled"))
				*value = 2;
			else
				ret = -EINVAL;
		}
		IWL_DEBUG_RADIO(fwrt, "DT: intel,srd = %u (%d)\n", *value, ret);
		break;
	case DSM_FUNC_ENABLE_6E:
		ret = of_property_read_u32(node, "intel,6e-uhb", value);
		IWL_DEBUG_RADIO(fwrt, "DT: intel,6e-uhb = 0x%x (%d)\n",
				*value, ret);
		break;
	case DSM_FUNC_REGULATORY_CONFIG:
		ret = of_property_read_u32(node,
					   "intel,regulatory-special",
					   value);
		IWL_DEBUG_RADIO(fwrt,
				"DT: intel,regulatory-special = 0x%x (%d)\n",
				*value, ret);
		break;
	case DSM_FUNC_ACTIVATE_CHANNEL:
		ret = of_property_read_u32(node, "intel,activate-channel",
					   value);
		IWL_DEBUG_RADIO(fwrt,
				"DT: intel,activate-channel = 0x%x (%d)\n",
				*value, ret);
		break;
	case DSM_FUNC_FORCE_DISABLE_CHANNELS:
		ret = of_property_read_u32(node,
					   "intel,force-disable-channels",
					   value);
		IWL_DEBUG_RADIO(fwrt,
				"DT: intel,force-disable-channels = 0x%x (%d)\n",
				*value, ret);
		break;
	case DSM_FUNC_ENABLE_11BE:
		ret = of_property_read_u32(node, "intel,11be", value);
		IWL_DEBUG_RADIO(fwrt, "DT: intel,11be = 0x%x (%d)\n",
				*value, ret);
		break;
	default:
		return -EOPNOTSUPP;
	}

	if (ret)
		return ret;

	fwrt->dsm_source = BIOS_SOURCE_DT;
	fwrt->dsm_revision = 0;

	return 0;
}

int iwl_dt_get_pwr_limit(struct iwl_fw_runtime *fwrt, u64 *data)
{
	struct device_node *node = dev_of_node(fwrt->dev);
	u32 val;
	int ret;

	if (!node)
		return -ENOENT;

	ret = of_property_read_u32(node, "intel,splc", &val);
	if (ret)
		return ret;

	*data = val;
	IWL_DEBUG_RADIO(fwrt, "DT: intel,splc = %u mW\n", val);
	return 0;
}

int iwl_dt_get_wbem(struct iwl_fw_runtime *fwrt, u32 *data)
{
	struct device_node *node = dev_of_node(fwrt->dev);
	int ret;

	if (!node)
		return -ENOENT;

	ret = of_property_read_u32(node, "intel,wbem", data);
	if (ret)
		return ret;

	IWL_DEBUG_RADIO(fwrt, "DT: intel,wbem = 0x%x\n", *data);
	return 0;
}

int iwl_dt_get_puncturing(struct iwl_fw_runtime *fwrt)
{
	struct device_node *node = dev_of_node(fwrt->dev);
	u32 val;
	int ret;

	if (!node)
		return -ENOENT;

	ret = of_property_read_u32(node, "intel,wcpe", &val);
	if (ret)
		return -ENOENT;

	fwrt->bios_puncturing = val & IWL_DT_WCPE_MASK;
	fwrt->puncturing_source = BIOS_SOURCE_DT;
	fwrt->puncturing_revision = 0;
	IWL_DEBUG_RADIO(fwrt, "DT: Loaded puncturing bits: 0x%x\n",
			fwrt->bios_puncturing);
	return 0;
}
IWL_EXPORT_SYMBOL(iwl_dt_get_puncturing);
