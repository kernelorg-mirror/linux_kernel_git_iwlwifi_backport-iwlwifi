// SPDX-License-Identifier: GPL-2.0-only
/*
 * KUnit tests for regulatory
 *
 * Copyright (C) 2026 Intel Corporation
 */
#include <linux/ieee80211.h>
#include <net/cfg80211.h>
#include <kunit/test.h>
#include "../core.h"
#include "util.h"

#if LINUX_VERSION_IS_LESS(6,13,0)
MODULE_IMPORT_NS(EXPORTED_FOR_KUNIT_TESTING);
#else
MODULE_IMPORT_NS("EXPORTED_FOR_KUNIT_TESTING");
#endif

#define CHAN5G(_freq) { \
	.band = NL80211_BAND_5GHZ, \
	.center_freq = (_freq), \
	.hw_value = (_freq), \
}

static const struct ieee80211_channel channels_5ghz[] = {
	/* UNII-1 */
	CHAN5G(5180), /* 36 */
	CHAN5G(5200), /* 40 */
	CHAN5G(5220), /* 44 */
	CHAN5G(5240), /* 48 */
	/* UNII-2A */
	CHAN5G(5260), /* 52 */
	CHAN5G(5280), /* 56 */
	CHAN5G(5300), /* 60 */
	CHAN5G(5320), /* 64 */
};

#define NO_IR_CONC (IEEE80211_CHAN_NO_IR | IEEE80211_CHAN_IR_CONCURRENT)

static const struct beacon_relax_case {
	const char *desc;
	u32 unii1_flags, unii2a_flags;
	u32 sta_freq;
	enum nl80211_iftype iftype;
	u32 control_freq, center_freq1;
	enum nl80211_chan_width width;
	bool relax;
	bool expected;
} beacon_relax_cases[] = {
	{
		.desc = "no restrictions",
		.iftype = NL80211_IFTYPE_P2P_GO,
		.control_freq = 5180,
		.center_freq1 = 5250,
		.width = NL80211_CHAN_WIDTH_160,
		.relax = true,
		.expected = true,
	},
	{
		.desc = "NO_IR without relaxation",
		.unii1_flags = NO_IR_CONC,
		.sta_freq = 5180,
		.iftype = NL80211_IFTYPE_P2P_GO,
		.control_freq = 5180,
		.center_freq1 = 5210,
		.width = NL80211_CHAN_WIDTH_80,
		.expected = false,
	},
#if IS_ENABLED(CPTCFG_CFG80211_REG_RELAX_NO_IR)
	{
		.desc = "80 MHz GO, STA on primary",
		.unii1_flags = NO_IR_CONC,
		.sta_freq = 5180,
		.iftype = NL80211_IFTYPE_P2P_GO,
		.control_freq = 5180,
		.center_freq1 = 5210,
		.width = NL80211_CHAN_WIDTH_80,
		.relax = true,
		.expected = true,
	},
	{
		.desc = "80 MHz GO, STA in same UNII band",
		.unii1_flags = NO_IR_CONC,
		.sta_freq = 5240,
		.iftype = NL80211_IFTYPE_P2P_GO,
		.control_freq = 5180,
		.center_freq1 = 5210,
		.width = NL80211_CHAN_WIDTH_80,
		.relax = true,
		.expected = true,
	},
	{
		.desc = "80 MHz GO, no STA",
		.unii1_flags = NO_IR_CONC,
		.iftype = NL80211_IFTYPE_P2P_GO,
		.control_freq = 5180,
		.center_freq1 = 5210,
		.width = NL80211_CHAN_WIDTH_80,
		.relax = true,
		.expected = false,
	},
	{
		.desc = "80 MHz AP, STA on primary",
		.unii1_flags = NO_IR_CONC,
		.sta_freq = 5180,
		.iftype = NL80211_IFTYPE_AP,
		.control_freq = 5180,
		.center_freq1 = 5210,
		.width = NL80211_CHAN_WIDTH_80,
		.relax = true,
		.expected = false,
	},
	{
		.desc = "160 MHz GO, only primary UNII band is NO_IR",
		.unii1_flags = NO_IR_CONC,
		.sta_freq = 5180,
		.iftype = NL80211_IFTYPE_P2P_GO,
		.control_freq = 5180,
		.center_freq1 = 5250,
		.width = NL80211_CHAN_WIDTH_160,
		.relax = true,
		.expected = true,
	},
	{
		.desc = "160 MHz GO, NO_IR secondary 80 MHz",
		.unii1_flags = NO_IR_CONC,
		.unii2a_flags = IEEE80211_CHAN_NO_IR,
		.sta_freq = 5180,
		.iftype = NL80211_IFTYPE_P2P_GO,
		.control_freq = 5180,
		.center_freq1 = 5250,
		.width = NL80211_CHAN_WIDTH_160,
		.relax = true,
		.expected = false,
	},
	{
		.desc = "160 MHz GO, IR_CONCURRENT secondary 80 MHz in other UNII band",
		.unii1_flags = NO_IR_CONC,
		.unii2a_flags = NO_IR_CONC,
		.sta_freq = 5180,
		.iftype = NL80211_IFTYPE_P2P_GO,
		.control_freq = 5180,
		.center_freq1 = 5250,
		.width = NL80211_CHAN_WIDTH_160,
		.relax = true,
		.expected = false,
	},
	{
		.desc = "160 MHz GO, NO_IR primary 80 MHz, permissive secondary",
		.unii1_flags = IEEE80211_CHAN_NO_IR,
		.unii2a_flags = NO_IR_CONC,
		.sta_freq = 5260,
		.iftype = NL80211_IFTYPE_P2P_GO,
		.control_freq = 5260,
		.center_freq1 = 5250,
		.width = NL80211_CHAN_WIDTH_160,
		.relax = true,
		.expected = false,
	},
	{
		.desc = "160 MHz GO, only secondary 80 MHz is NO_IR but permissive",
		.unii2a_flags = NO_IR_CONC,
		.sta_freq = 5300,
		.iftype = NL80211_IFTYPE_P2P_GO,
		.control_freq = 5180,
		.center_freq1 = 5250,
		.width = NL80211_CHAN_WIDTH_160,
		.relax = true,
		.expected = true,
	},
#endif /* CPTCFG_CFG80211_REG_RELAX_NO_IR */
};

KUNIT_ARRAY_PARAM_DESC(beacon_relax, beacon_relax_cases, desc)

struct beacon_relax_test_ctx {
	struct ieee80211_supported_band band_5ghz;
	struct ieee80211_channel channels[ARRAY_SIZE(channels_5ghz)];
	struct cfg80211_internal_bss bss;
	struct wireless_dev sta_wdev;
};

static void beacon_relax_test_remove_wdev(void *data)
{
	struct wireless_dev *wdev = data;

	list_del(&wdev->list);
}

static struct wiphy *beacon_relax_test_setup(struct kunit *test,
					  struct beacon_relax_test_ctx *ctx,
					  const struct beacon_relax_case *params)
{
	struct ieee80211_supported_band *sband = &ctx->band_5ghz;
	struct wiphy *wiphy = T_WIPHY(test, ctx);

	memcpy(ctx->channels, channels_5ghz, sizeof(channels_5ghz));
	for (int i = 0; i < ARRAY_SIZE(ctx->channels); i++) {
		struct ieee80211_channel *chan = &ctx->channels[i];

		if (chan->center_freq <= 5250)
			chan->flags = params->unii1_flags;
		else
			chan->flags = params->unii2a_flags;
	}

	sband->band = NL80211_BAND_5GHZ;
	sband->channels = ctx->channels;
	sband->n_channels = ARRAY_SIZE(ctx->channels);
	sband->ht_cap.ht_supported = true;
	sband->ht_cap.cap = IEEE80211_HT_CAP_SUP_WIDTH_20_40;
	sband->vht_cap.vht_supported = true;
	sband->vht_cap.cap = IEEE80211_VHT_CAP_SUPP_CHAN_WIDTH_160MHZ;
	wiphy->bands[NL80211_BAND_5GHZ] = sband;

	wiphy->regulatory_flags |= REGULATORY_ENABLE_RELAX_NO_IR;

	if (params->sta_freq) {
		struct wireless_dev *wdev = &ctx->sta_wdev;

		ctx->bss.pub.channel = ieee80211_get_channel(wiphy,
							     params->sta_freq);
		KUNIT_ASSERT_NOT_NULL(test, ctx->bss.pub.channel);

		wdev->wiphy = wiphy;
		wdev->iftype = NL80211_IFTYPE_STATION;
		wdev->links[0].client.current_bss = &ctx->bss;
		list_add(&wdev->list, &wiphy->wdev_list);
		KUNIT_ASSERT_EQ(test, 0,
				kunit_add_action_or_reset(test,
							  beacon_relax_test_remove_wdev,
							  wdev));
	}

	return wiphy;
}

static void test_beacon_relax(struct kunit *test)
{
	const struct beacon_relax_case *params = test->param_value;
	struct cfg80211_beaconing_check_config cfg = {
		.iftype = params->iftype,
		.relax = params->relax,
	};
	struct beacon_relax_test_ctx *ctx;
	struct cfg80211_chan_def chandef;
	struct wiphy *wiphy;
	bool ret;

	ctx = kunit_kzalloc(test, sizeof(*ctx), GFP_KERNEL);
	KUNIT_ASSERT_NOT_NULL(test, ctx);

	wiphy = beacon_relax_test_setup(test, ctx, params);

	chandef = (struct cfg80211_chan_def){
		.chan = ieee80211_get_channel(wiphy, params->control_freq),
		.center_freq1 = params->center_freq1,
		.width = params->width,
	};
	KUNIT_ASSERT_NOT_NULL(test, chandef.chan);
	KUNIT_ASSERT_TRUE(test, cfg80211_chandef_valid(&chandef));

	wiphy_lock(wiphy);
	ret = cfg80211_reg_check_beaconing(wiphy, &chandef, &cfg);
	wiphy_unlock(wiphy);

	KUNIT_EXPECT_EQ(test, ret, params->expected);
}

static struct kunit_case beacon_relax_test_cases[] = {
	KUNIT_CASE_PARAM(test_beacon_relax, beacon_relax_gen_params),
	{}
};

static struct kunit_suite beacon_relax = {
	.name = "cfg80211-beacon-relax",
	.test_cases = beacon_relax_test_cases,
};

kunit_test_suite(beacon_relax);
