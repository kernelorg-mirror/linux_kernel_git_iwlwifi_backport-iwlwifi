/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2026 Intel Corporation
 */
#ifndef __BACKPORT_NET_PAGE_POOL_TYPES_H
#define __BACKPORT_NET_PAGE_POOL_TYPES_H
#include <linux/version.h>

#if LINUX_VERSION_IS_LESS(6,6,0)
/* before 6.6, the page pool types and helpers were all in net/page_pool.h */
#include <net/page_pool.h>
#else
#include_next <net/page_pool/types.h>
#endif

#endif /* __BACKPORT_NET_PAGE_POOL_TYPES_H */
