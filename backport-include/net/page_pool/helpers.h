/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2026 Intel Corporation
 */
#ifndef __BACKPORT_NET_PAGE_POOL_HELPERS_H
#define __BACKPORT_NET_PAGE_POOL_HELPERS_H
#include <linux/version.h>

#if LINUX_VERSION_IS_LESS(6,6,0)
/* before 6.6, the page pool types and helpers were all in net/page_pool.h */
#include <net/page_pool.h>
#else
#include_next <net/page_pool/helpers.h>
#endif

#if LINUX_VERSION_IS_LESS(5,18,0)
#define page_pool_fragment_page LINUX_BACKPORT(page_pool_fragment_page)
static inline void page_pool_fragment_page(struct page *page, long nr)
{
	page_pool_set_frag_count(page, nr);
}
#endif /* < 5.18 */

#if LINUX_VERSION_IS_LESS(6,7,0)
/*
 * Since 6.7 the page pool sets the first reference when the page is
 * allocated. Before that, the user has to do it, and the pool has to be
 * created with PP_FLAG_PAGE_FRAG for the references to be honoured.
 */
static inline struct page *
backport_page_pool_dev_alloc_pages(struct page_pool *pool)
{
	struct page *page = page_pool_dev_alloc_pages(pool);

	if (page)
		page_pool_fragment_page(page, 1);

	return page;
}
#define page_pool_dev_alloc_pages LINUX_BACKPORT(page_pool_dev_alloc_pages)
#endif /* < 6.7 */

#if LINUX_VERSION_IS_LESS(6,8,0)
#define page_pool_ref_page LINUX_BACKPORT(page_pool_ref_page)
static inline void page_pool_ref_page(struct page *page)
{
	/* pp_ref_count was called pp_frag_count before 6.8 */
	atomic_long_inc(&page->pp_frag_count);
}
#endif /* < 6.8 */

#endif /* __BACKPORT_NET_PAGE_POOL_HELPERS_H */
