/******************************************************************************
 *
 * This file is provided under a dual BSD/GPLv2 license.  When using or
 * redistributing this file, you may do so under either license.
 *
 * GPL LICENSE SUMMARY
 *
 * Copyright(c) 2007 - 2012 Intel Corporation. All rights reserved.
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
 *
 *****************************************************************************/
#ifndef __iwl_trans_h__
#define __iwl_trans_h__

#include <linux/debugfs.h>
#include <linux/skbuff.h>

#include "iwl-shared.h"
#include "iwl-commands.h"

 /*This file includes the declaration that are exported from the transport
 * layer */

struct iwl_priv;
struct iwl_shared;

#define SEQ_TO_SN(seq) (((seq) & IEEE80211_SCTL_SEQ) >> 4)
#define SN_TO_SEQ(ssn) (((ssn) << 4) & IEEE80211_SCTL_SEQ)
#define MAX_SN ((IEEE80211_SCTL_SEQ) >> 4)

enum {
	CMD_SYNC = 0,
	CMD_ASYNC = BIT(0),
	CMD_WANT_SKB = BIT(1),
	CMD_ON_DEMAND = BIT(2),
};

#define DEF_CMD_PAYLOAD_SIZE 320

/**
 * struct iwl_device_cmd
 *
 * For allocation of the command and tx queues, this establishes the overall
 * size of the largest command we send to uCode, except for commands that
 * aren't fully copied and use other TFD space.
 */
struct iwl_device_cmd {
	struct iwl_cmd_header hdr;	/* uCode API */
	u8 payload[DEF_CMD_PAYLOAD_SIZE];
} __packed;

#define TFD_MAX_PAYLOAD_SIZE (sizeof(struct iwl_device_cmd))

#define IWL_MAX_CMD_TFDS	2

enum iwl_hcmd_dataflag {
	IWL_HCMD_DFL_NOCOPY	= BIT(0),
};

/**
 * struct iwl_host_cmd - Host command to the uCode
 * @data: array of chunks that composes the data of the host command
 * @reply_page: pointer to the page that holds the response to the host command
 * @handler_status: return value of the handler of the command
 *	(put in setup_rx_handlers) - valid for SYNC mode only
 * @callback:
 * @flags: can be CMD_* note CMD_WANT_SKB is incompatible withe CMD_ASYNC
 * @len: array of the lenths of the chunks in data
 * @dataflags:
 * @id: id of the host command
 */
struct iwl_host_cmd {
	const void *data[IWL_MAX_CMD_TFDS];
	unsigned long reply_page;
	int handler_status;

	u32 flags;
	u16 len[IWL_MAX_CMD_TFDS];
	u8 dataflags[IWL_MAX_CMD_TFDS];
	u8 id;
};

/* one for each uCode image (inst/data, boot/init/runtime) */
struct fw_desc {
	dma_addr_t p_addr;	/* hardware address */
	void *v_addr;		/* software address */
	u32 len;		/* size in bytes */
};

struct fw_img {
	struct fw_desc code;	/* firmware code image */
	struct fw_desc data;	/* firmware data image */
};

/**
 * struct iwl_trans_ops - transport specific operations
 * @start_hw: starts the HW- from that point on, the HW can send interrupts
 * @stop_hw: stops the HW- from that point on, the HW will be in low power but
 *	will still issue interrupt if the HW RF kill is triggered.
 * @start_fw: allocates and inits all the resources for the transport
 *	layer. Also kick a fw image. This handler may sleep.
 * @fw_alive: called when the fw sends alive notification
 * @wake_any_queue: wake all the queues of a specfic context IWL_RXON_CTX_*
 * @stop_device:stops the whole device (embedded CPU put to reset)
 * @send_cmd:send a host command
 * @tx: send an skb
 * @reclaim: free packet until ssn. Returns a list of freed packets.
 * @tx_agg_alloc: allocate resources for a TX BA session
 * @tx_agg_setup: setup a tx queue for AMPDU - will be called once the HW is
 *                 ready and a successful ADDBA response has been received.
 * @tx_agg_disable: de-configure a Tx queue to send AMPDUs
 * @free: release all the ressource for the transport layer itself such as
 *        irq, tasklet etc...
 * @stop_queue: stop a specific queue
 * @check_stuck_queue: check if a specific queue is stuck
 * @wait_tx_queue_empty: wait until all tx queues are empty
 * @dbgfs_register: add the dbgfs files under this directory. Files will be
 *	automatically deleted.
 * @suspend: stop the device unless WoWLAN is configured
 * @resume: resume activity of the device
 * @write8: write a u8 to a register at offset ofs from the BAR
 * @write32: write a u32 to a register at offset ofs from the BAR
 * @read32: read a u32 register at offset ofs from the BAR
 */
struct iwl_trans_ops {

	int (*start_hw)(struct iwl_trans *iwl_trans);
	void (*stop_hw)(struct iwl_trans *iwl_trans);
	int (*start_fw)(struct iwl_trans *trans, struct fw_img *fw);
	void (*fw_alive)(struct iwl_trans *trans);
	void (*stop_device)(struct iwl_trans *trans);

	void (*wake_any_queue)(struct iwl_trans *trans,
			       enum iwl_rxon_context_id ctx,
			       const char *msg);

	int (*send_cmd)(struct iwl_trans *trans, struct iwl_host_cmd *cmd);

	int (*tx)(struct iwl_trans *trans, struct sk_buff *skb,
		struct iwl_device_cmd *dev_cmd, enum iwl_rxon_context_id ctx,
		u8 sta_id, u8 tid);
	int (*reclaim)(struct iwl_trans *trans, int sta_id, int tid,
			int txq_id, int ssn, u32 status,
			struct sk_buff_head *skbs);

	int (*tx_agg_disable)(struct iwl_trans *trans,
			      int sta_id, int tid);
	int (*tx_agg_alloc)(struct iwl_trans *trans,
			    int sta_id, int tid);
	void (*tx_agg_setup)(struct iwl_trans *trans,
			     enum iwl_rxon_context_id ctx, int sta_id, int tid,
			     int frame_limit, u16 ssn);

	void (*free)(struct iwl_trans *trans);

	void (*stop_queue)(struct iwl_trans *trans, int q, const char *msg);

	int (*dbgfs_register)(struct iwl_trans *trans, struct dentry* dir);
	int (*check_stuck_queue)(struct iwl_trans *trans, int q);
	int (*wait_tx_queue_empty)(struct iwl_trans *trans);
#ifdef CONFIG_PM_SLEEP
	int (*suspend)(struct iwl_trans *trans);
	int (*resume)(struct iwl_trans *trans);
#endif
	void (*write8)(struct iwl_trans *trans, u32 ofs, u8 val);
	void (*write32)(struct iwl_trans *trans, u32 ofs, u32 val);
	u32 (*read32)(struct iwl_trans *trans, u32 ofs);
};

/* Opaque calibration results */
struct iwl_calib_result {
	struct list_head list;
	size_t cmd_len;
	struct iwl_calib_hdr hdr;
	/* data follows */
};

/**
 * struct iwl_trans - transport common data
 * @ops - pointer to iwl_trans_ops
 * @shrd - pointer to iwl_shared which holds shared data from the upper layer
 * @hcmd_lock: protects HCMD
 * @reg_lock - protect hw register access
 * @dev - pointer to struct device * that represents the device
 * @irq - the irq number for the device
 * @hw_id: a u32 with the ID of the device / subdevice.
  *	Set during transport alloaction.
 * @hw_id_str: a string with info about HW ID. Set during transport allocation.
 * @ucode_write_complete: indicates that the ucode has been copied.
 * @ucode_rt: run time ucode image
 * @ucode_init: init ucode image
 * @ucode_wowlan: wake on wireless ucode image (optional)
 * @nvm_device_type: indicates OTP or eeprom
 * @pm_support: set to true in start_hw if link pm is supported
 * @calib_results: list head for init calibration results
 */
struct iwl_trans {
	const struct iwl_trans_ops *ops;
	struct iwl_shared *shrd;
	spinlock_t hcmd_lock;
	spinlock_t reg_lock;

	struct device *dev;
	unsigned int irq;
	u32 hw_rev;
	u32 hw_id;
	char hw_id_str[52];

	u8 ucode_write_complete;	/* the image write is complete */
	struct fw_img ucode_rt;
	struct fw_img ucode_init;
	struct fw_img ucode_wowlan;

	/* eeprom related variables */
	int    nvm_device_type;
	bool pm_support;

	/* init calibration results */
	struct list_head calib_results;

	/* pointer to trans specific struct */
	/*Ensure that this pointer will always be aligned to sizeof pointer */
	char trans_specific[0] __attribute__((__aligned__(sizeof(void *))));
};

static inline int iwl_trans_start_hw(struct iwl_trans *trans)
{
	return trans->ops->start_hw(trans);
}

static inline void iwl_trans_stop_hw(struct iwl_trans *trans)
{
	trans->ops->stop_hw(trans);
}

static inline void iwl_trans_fw_alive(struct iwl_trans *trans)
{
	trans->ops->fw_alive(trans);
}

static inline int iwl_trans_start_fw(struct iwl_trans *trans, struct fw_img *fw)
{
	might_sleep();

	return trans->ops->start_fw(trans, fw);
}

static inline void iwl_trans_stop_device(struct iwl_trans *trans)
{
	trans->ops->stop_device(trans);
}

static inline void iwl_trans_wake_any_queue(struct iwl_trans *trans,
					    enum iwl_rxon_context_id ctx,
					    const char *msg)
{
	trans->ops->wake_any_queue(trans, ctx, msg);
}


static inline int iwl_trans_send_cmd(struct iwl_trans *trans,
				struct iwl_host_cmd *cmd)
{
	return trans->ops->send_cmd(trans, cmd);
}

int iwl_trans_send_cmd_pdu(struct iwl_trans *trans, u8 id,
			   u32 flags, u16 len, const void *data);

static inline int iwl_trans_tx(struct iwl_trans *trans, struct sk_buff *skb,
		struct iwl_device_cmd *dev_cmd, enum iwl_rxon_context_id ctx,
		u8 sta_id, u8 tid)
{
	return trans->ops->tx(trans, skb, dev_cmd, ctx, sta_id, tid);
}

static inline int iwl_trans_reclaim(struct iwl_trans *trans, int sta_id,
				 int tid, int txq_id, int ssn, u32 status,
				 struct sk_buff_head *skbs)
{
	return trans->ops->reclaim(trans, sta_id, tid, txq_id, ssn,
				   status, skbs);
}

static inline int iwl_trans_tx_agg_disable(struct iwl_trans *trans,
					    int sta_id, int tid)
{
	return trans->ops->tx_agg_disable(trans, sta_id, tid);
}

static inline int iwl_trans_tx_agg_alloc(struct iwl_trans *trans,
					 int sta_id, int tid)
{
	return trans->ops->tx_agg_alloc(trans, sta_id, tid);
}


static inline void iwl_trans_tx_agg_setup(struct iwl_trans *trans,
					   enum iwl_rxon_context_id ctx,
					   int sta_id, int tid,
					   int frame_limit, u16 ssn)
{
	trans->ops->tx_agg_setup(trans, ctx, sta_id, tid, frame_limit, ssn);
}

static inline void iwl_trans_free(struct iwl_trans *trans)
{
	trans->ops->free(trans);
}

static inline void iwl_trans_stop_queue(struct iwl_trans *trans, int q,
					const char *msg)
{
	trans->ops->stop_queue(trans, q, msg);
}

static inline int iwl_trans_wait_tx_queue_empty(struct iwl_trans *trans)
{
	return trans->ops->wait_tx_queue_empty(trans);
}

static inline int iwl_trans_check_stuck_queue(struct iwl_trans *trans, int q)
{
	return trans->ops->check_stuck_queue(trans, q);
}
static inline int iwl_trans_dbgfs_register(struct iwl_trans *trans,
					    struct dentry *dir)
{
	return trans->ops->dbgfs_register(trans, dir);
}

#ifdef CONFIG_PM_SLEEP
static inline int iwl_trans_suspend(struct iwl_trans *trans)
{
	return trans->ops->suspend(trans);
}

static inline int iwl_trans_resume(struct iwl_trans *trans)
{
	return trans->ops->resume(trans);
}
#endif

static inline void iwl_trans_write8(struct iwl_trans *trans, u32 ofs, u8 val)
{
	trans->ops->write8(trans, ofs, val);
}

static inline void iwl_trans_write32(struct iwl_trans *trans, u32 ofs, u32 val)
{
	trans->ops->write32(trans, ofs, val);
}

static inline u32 iwl_trans_read32(struct iwl_trans *trans, u32 ofs)
{
	return trans->ops->read32(trans, ofs);
}

/*****************************************************
* Utils functions
******************************************************/
void iwl_dealloc_ucode(struct iwl_trans *trans);

int iwl_send_calib_results(struct iwl_trans *trans);
int iwl_calib_set(struct iwl_trans *trans,
		  const struct iwl_calib_hdr *cmd, int len);
void iwl_calib_free_results(struct iwl_trans *trans);

/*****************************************************
* Transport layers implementations + their allocation function
******************************************************/
struct pci_dev;
struct pci_device_id;
extern const struct iwl_trans_ops trans_ops_pcie;
struct iwl_trans *iwl_trans_pcie_alloc(struct iwl_shared *shrd,
				       struct pci_dev *pdev,
				       const struct pci_device_id *ent);

extern const struct iwl_trans_ops trans_ops_idi;
struct iwl_trans *iwl_trans_idi_alloc(struct iwl_shared *shrd,
				      void *pdev_void,
				      const void *ent_void);
#endif /* __iwl_trans_h__ */
