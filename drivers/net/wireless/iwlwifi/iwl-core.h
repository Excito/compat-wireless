/******************************************************************************
 *
 * This file is provided under a dual BSD/GPLv2 license.  When using or
 * redistributing this file, you may do so under either license.
 *
 * GPL LICENSE SUMMARY
 *
 * Copyright(c) 2008 - 2012 Intel Corporation. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110,
 * USA
 *
 * The full GNU General Public License is included in this distribution
 * in the file called LICENSE.GPL.
 *
 * Contact Information:
 *  Intel Linux Wireless <ilw@linux.intel.com>
 * Intel Corporation, 5200 N.E. Elam Young Parkway, Hillsboro, OR 97124-6497
 *
 * BSD LICENSE
 *
 * Copyright(c) 2005 - 2012 Intel Corporation. All rights reserved.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *  * Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *  * Neither the name Intel Corporation nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *****************************************************************************/

#ifndef __iwl_core_h__
#define __iwl_core_h__

#include "iwl-dev.h"
#include "iwl-io.h"

/************************
 * forward declarations *
 ************************/
struct iwl_host_cmd;
struct iwl_cmd;

#define TIME_UNIT		1024

struct iwl_lib_ops {
	/* set hw dependent parameters */
	int (*set_hw_params)(struct iwl_priv *priv);
	/* setup BT Rx handler */
	void (*bt_rx_handler_setup)(struct iwl_priv *priv);
	/* setup BT related deferred work */
	void (*bt_setup_deferred_work)(struct iwl_priv *priv);
	/* cancel deferred work */
	void (*cancel_deferred_work)(struct iwl_priv *priv);
	int (*set_channel_switch)(struct iwl_priv *priv,
				  struct ieee80211_channel_switch *ch_switch);
	/* device specific configuration */
	void (*nic_config)(struct iwl_priv *priv);

	/* eeprom operations (as defined in iwl-eeprom.h) */
	struct iwl_eeprom_ops eeprom_ops;

	/* temperature */
	void (*temperature)(struct iwl_priv *priv);
};

/*
 * @max_ll_items: max number of OTP blocks
 * @shadow_ram_support: shadow support for OTP memory
 * @led_compensation: compensate on the led on/off time per HW according
 *	to the deviation to achieve the desired led frequency.
 *	The detail algorithm is described in iwl-led.c
 * @chain_noise_num_beacons: number of beacons used to compute chain noise
 * @adv_thermal_throttle: support advance thermal throttle
 * @support_ct_kill_exit: support ct kill exit condition
 * @support_wimax_coexist: support wimax/wifi co-exist
 * @plcp_delta_threshold: plcp error rate threshold used to trigger
 *	radio tuning when there is a high receiving plcp error rate
 * @chain_noise_scale: default chain noise scale used for gain computation
 * @wd_timeout: TX queues watchdog timeout
 * @max_event_log_size: size of event log buffer size for ucode event logging
 * @shadow_reg_enable: HW shadhow register bit
 * @no_idle_support: do not support idle mode
 * @hd_v2: v2 of enhanced sensitivity value, used for 2000 series and up
 * wd_disable: disable watchdog timer
 */
struct iwl_base_params {
	int eeprom_size;
	int num_of_queues;	/* def: HW dependent */
	int num_of_ampdu_queues;/* def: HW dependent */
	/* for iwl_apm_init() */
	u32 pll_cfg_val;

	const u16 max_ll_items;
	const bool shadow_ram_support;
	u16 led_compensation;
	bool adv_thermal_throttle;
	bool support_ct_kill_exit;
	const bool support_wimax_coexist;
	u8 plcp_delta_threshold;
	s32 chain_noise_scale;
	unsigned int wd_timeout;
	u32 max_event_log_size;
	const bool shadow_reg_enable;
	const bool no_idle_support;
	const bool hd_v2;
	const bool wd_disable;
};
/*
 * @advanced_bt_coexist: support advanced bt coexist
 * @bt_init_traffic_load: specify initial bt traffic load
 * @bt_prio_boost: default bt priority boost value
 * @agg_time_limit: maximum number of uSec in aggregation
 * @bt_sco_disable: uCode should not response to BT in SCO/ESCO mode
 */
struct iwl_bt_params {
	bool advanced_bt_coexist;
	u8 bt_init_traffic_load;
	u8 bt_prio_boost;
	u16 agg_time_limit;
	bool bt_sco_disable;
	bool bt_session_2;
};
/*
 * @use_rts_for_aggregation: use rts/cts protection for HT traffic
 */
struct iwl_ht_params {
	const bool ht_greenfield_support; /* if used set to true */
	bool use_rts_for_aggregation;
	enum ieee80211_smps_mode smps_mode;
};

/***************************
 *   L i b                 *
 ***************************/

void iwl_set_rxon_hwcrypto(struct iwl_priv *priv, struct iwl_rxon_context *ctx,
			   int hw_decrypt);
int iwl_check_rxon_cmd(struct iwl_priv *priv, struct iwl_rxon_context *ctx);
int iwl_full_rxon_required(struct iwl_priv *priv, struct iwl_rxon_context *ctx);
int iwl_set_rxon_channel(struct iwl_priv *priv, struct ieee80211_channel *ch,
			 struct iwl_rxon_context *ctx);
void iwl_set_flags_for_band(struct iwl_priv *priv,
			    struct iwl_rxon_context *ctx,
			    enum ieee80211_band band,
			    struct ieee80211_vif *vif);
u8 iwl_get_single_channel_number(struct iwl_priv *priv,
				  enum ieee80211_band band);
void iwl_set_rxon_ht(struct iwl_priv *priv, struct iwl_ht_config *ht_conf);
bool iwl_is_ht40_tx_allowed(struct iwl_priv *priv,
			    struct iwl_rxon_context *ctx,
			    struct ieee80211_sta_ht_cap *ht_cap);
void iwl_connection_init_rx_config(struct iwl_priv *priv,
				   struct iwl_rxon_context *ctx);
void iwl_set_rate(struct iwl_priv *priv);
int iwl_cmd_echo_test(struct iwl_priv *priv);
#ifdef CONFIG_IWLWIFI_DEBUGFS
int iwl_alloc_traffic_mem(struct iwl_priv *priv);
void iwl_free_traffic_mem(struct iwl_priv *priv);
void iwl_dbg_log_tx_data_frame(struct iwl_priv *priv,
				u16 length, struct ieee80211_hdr *header);
void iwl_dbg_log_rx_data_frame(struct iwl_priv *priv,
				u16 length, struct ieee80211_hdr *header);
const char *get_mgmt_string(int cmd);
const char *get_ctrl_string(int cmd);
void iwl_clear_traffic_stats(struct iwl_priv *priv);
void iwl_update_stats(struct iwl_priv *priv, bool is_tx, __le16 fc,
		      u16 len);
#else
static inline int iwl_alloc_traffic_mem(struct iwl_priv *priv)
{
	return 0;
}
static inline void iwl_free_traffic_mem(struct iwl_priv *priv)
{
}
static inline void iwl_reset_traffic_log(struct iwl_priv *priv)
{
}
static inline void iwl_dbg_log_tx_data_frame(struct iwl_priv *priv,
		      u16 length, struct ieee80211_hdr *header)
{
}
static inline void iwl_dbg_log_rx_data_frame(struct iwl_priv *priv,
		      u16 length, struct ieee80211_hdr *header)
{
}
static inline void iwl_update_stats(struct iwl_priv *priv, bool is_tx,
				    __le16 fc, u16 len)
{
}
#endif

/*****************************************************
* RX
******************************************************/
void iwl_chswitch_done(struct iwl_priv *priv, bool is_success);

void iwl_setup_watchdog(struct iwl_priv *priv);
/*****************************************************
 * TX power
 ****************************************************/
int iwl_set_tx_power(struct iwl_priv *priv, s8 tx_power, bool force);

/*******************************************************************************
 * Scanning
 ******************************************************************************/
void iwl_init_scan_params(struct iwl_priv *priv);
int iwl_scan_cancel(struct iwl_priv *priv);
void iwl_scan_cancel_timeout(struct iwl_priv *priv, unsigned long ms);
void iwl_force_scan_end(struct iwl_priv *priv);
void iwl_internal_short_hw_scan(struct iwl_priv *priv);
int iwl_force_reset(struct iwl_priv *priv, int mode, bool external);
u16 iwl_fill_probe_req(struct iwl_priv *priv, struct ieee80211_mgmt *frame,
		       const u8 *ta, const u8 *ie, int ie_len, int left);
void iwl_setup_rx_scan_handlers(struct iwl_priv *priv);
void iwl_setup_scan_deferred_work(struct iwl_priv *priv);
void iwl_cancel_scan_deferred_work(struct iwl_priv *priv);
int __must_check iwl_scan_initiate(struct iwl_priv *priv,
				   struct ieee80211_vif *vif,
				   enum iwl_scan_type scan_type,
				   enum ieee80211_band band);

/* For faster active scanning, scan will move to the next channel if fewer than
 * PLCP_QUIET_THRESH packets are heard on this channel within
 * ACTIVE_QUIET_TIME after sending probe request.  This shortens the dwell
 * time if it's a quiet channel (nothing responded to our probe, and there's
 * no other traffic).
 * Disable "quiet" feature by setting PLCP_QUIET_THRESH to 0. */
#define IWL_ACTIVE_QUIET_TIME       cpu_to_le16(10)  /* msec */
#define IWL_PLCP_QUIET_THRESH       cpu_to_le16(1)  /* packets */

#define IWL_SCAN_CHECK_WATCHDOG		(HZ * 7)

/*****************************************************
 *   S e n d i n g     H o s t     C o m m a n d s   *
 *****************************************************/

void iwl_bg_watchdog(unsigned long data);
u32 iwl_usecs_to_beacons(struct iwl_priv *priv, u32 usec, u32 beacon_interval);
__le32 iwl_add_beacon_time(struct iwl_priv *priv, u32 base,
			   u32 addon, u32 beacon_interval);


/*****************************************************
*  GEOS
******************************************************/
int iwl_init_geos(struct iwl_priv *priv);
void iwl_free_geos(struct iwl_priv *priv);

extern void iwl_send_bt_config(struct iwl_priv *priv);
extern int iwl_send_statistics_request(struct iwl_priv *priv,
				       u8 flags, bool clear);

int iwl_send_rxon_timing(struct iwl_priv *priv, struct iwl_rxon_context *ctx);

static inline const struct ieee80211_supported_band *iwl_get_hw_mode(
			struct iwl_priv *priv, enum ieee80211_band band)
{
	return priv->hw->wiphy->bands[band];
}

static inline bool iwl_advanced_bt_coexist(struct iwl_priv *priv)
{
	return cfg(priv)->bt_params &&
	       cfg(priv)->bt_params->advanced_bt_coexist;
}

extern bool bt_siso_mode;

#endif /* __iwl_core_h__ */
