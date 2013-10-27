/*
 * Copyright (c) 2005-2010 Brocade Communications Systems, Inc.
 * All rights reserved
 * www.brocade.com
 *
 * Linux driver for Brocade Fibre Channel Host Bus Adapter.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License (GPL) Version 2 as
 * published by the Free Software Foundation
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 */

#ifndef __BFA_PORT_H__
#define __BFA_PORT_H__

#include "bfa_defs_svc.h"
#include "bfa_ioc.h"
#include "bfa_cs.h"

typedef void (*bfa_port_stats_cbfn_t) (void *dev, bfa_status_t status);
typedef void (*bfa_port_endis_cbfn_t) (void *dev, bfa_status_t status);

struct bfa_port_s {
	void				*dev;
	struct bfa_ioc_s		*ioc;
	struct bfa_trc_mod_s		*trcmod;
	u32			msgtag;
	bfa_boolean_t			stats_busy;
	struct bfa_mbox_cmd_s		stats_mb;
	bfa_port_stats_cbfn_t		stats_cbfn;
	void				*stats_cbarg;
	bfa_status_t			stats_status;
	u32			stats_reset_time;
	union bfa_port_stats_u		*stats;
	struct bfa_dma_s		stats_dma;
	bfa_boolean_t			endis_pending;
	struct bfa_mbox_cmd_s		endis_mb;
	bfa_port_endis_cbfn_t		endis_cbfn;
	void				*endis_cbarg;
	bfa_status_t			endis_status;
	struct bfa_ioc_notify_s		ioc_notify;
	bfa_boolean_t			pbc_disabled;
	struct bfa_mem_dma_s		port_dma;
};

#define BFA_MEM_PORT_DMA(__bfa)		(&((__bfa)->modules.port.port_dma))

void	     bfa_port_attach(struct bfa_port_s *port, struct bfa_ioc_s *ioc,
				void *dev, struct bfa_trc_mod_s *trcmod);
void	bfa_port_notify(void *arg, enum bfa_ioc_event_e event);

bfa_status_t bfa_port_get_stats(struct bfa_port_s *port,
				 union bfa_port_stats_u *stats,
				 bfa_port_stats_cbfn_t cbfn, void *cbarg);
bfa_status_t bfa_port_clear_stats(struct bfa_port_s *port,
				   bfa_port_stats_cbfn_t cbfn, void *cbarg);
bfa_status_t bfa_port_enable(struct bfa_port_s *port,
			      bfa_port_endis_cbfn_t cbfn, void *cbarg);
bfa_status_t bfa_port_disable(struct bfa_port_s *port,
			       bfa_port_endis_cbfn_t cbfn, void *cbarg);
u32     bfa_port_meminfo(void);
void	     bfa_port_mem_claim(struct bfa_port_s *port,
				 u8 *dma_kva, u64 dma_pa);

/*
 * CEE declaration
 */
typedef void (*bfa_cee_get_attr_cbfn_t) (void *dev, bfa_status_t status);
typedef void (*bfa_cee_get_stats_cbfn_t) (void *dev, bfa_status_t status);
typedef void (*bfa_cee_reset_stats_cbfn_t) (void *dev, bfa_status_t status);

struct bfa_cee_cbfn_s {
	bfa_cee_get_attr_cbfn_t		get_attr_cbfn;
	void				*get_attr_cbarg;
	bfa_cee_get_stats_cbfn_t	get_stats_cbfn;
	void				*get_stats_cbarg;
	bfa_cee_reset_stats_cbfn_t	reset_stats_cbfn;
	void				*reset_stats_cbarg;
};

struct bfa_cee_s {
	void *dev;
	bfa_boolean_t		get_attr_pending;
	bfa_boolean_t		get_stats_pending;
	bfa_boolean_t		reset_stats_pending;
	bfa_status_t		get_attr_status;
	bfa_status_t		get_stats_status;
	bfa_status_t		reset_stats_status;
	struct bfa_cee_cbfn_s	cbfn;
	struct bfa_ioc_notify_s	ioc_notify;
	struct bfa_trc_mod_s	*trcmod;
	struct bfa_cee_attr_s	*attr;
	struct bfa_cee_stats_s	*stats;
	struct bfa_dma_s	attr_dma;
	struct bfa_dma_s	stats_dma;
	struct bfa_ioc_s	*ioc;
	struct bfa_mbox_cmd_s	get_cfg_mb;
	struct bfa_mbox_cmd_s	get_stats_mb;
	struct bfa_mbox_cmd_s	reset_stats_mb;
	struct bfa_mem_dma_s	cee_dma;
};

#define BFA_MEM_CEE_DMA(__bfa)	(&((__bfa)->modules.cee.cee_dma))

u32	bfa_cee_meminfo(void);
void	bfa_cee_mem_claim(struct bfa_cee_s *cee, u8 *dma_kva, u64 dma_pa);
void	bfa_cee_attach(struct bfa_cee_s *cee,
			struct bfa_ioc_s *ioc, void *dev);
bfa_status_t	bfa_cee_get_attr(struct bfa_cee_s *cee,
				struct bfa_cee_attr_s *attr,
				bfa_cee_get_attr_cbfn_t cbfn, void *cbarg);
bfa_status_t	bfa_cee_get_stats(struct bfa_cee_s *cee,
				struct bfa_cee_stats_s *stats,
				bfa_cee_get_stats_cbfn_t cbfn, void *cbarg);
bfa_status_t	bfa_cee_reset_stats(struct bfa_cee_s *cee,
				bfa_cee_reset_stats_cbfn_t cbfn, void *cbarg);

#endif	/* __BFA_PORT_H__ */
