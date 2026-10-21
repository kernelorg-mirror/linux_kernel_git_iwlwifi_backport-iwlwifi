/* SPDX-License-Identifier: GPL-2.0 */
/*
 * Copyright (C) 2026 Intel Corporation
 */
#ifndef __BACKPORT_MM_H
#define __BACKPORT_MM_H
#include_next <linux/mm.h>
#include <linux/overflow.h>

#if LINUX_VERSION_IS_LESS(4,18,0)
#define kvcalloc LINUX_BACKPORT(kvcalloc)
static inline void *kvcalloc(size_t n, size_t size, gfp_t flags)
{
	return kvmalloc_array(n, size, flags | __GFP_ZERO);
}
#endif /* < 4.18 */
#if LINUX_VERSION_IS_LESS(6,3,0)

#define kvmemdup LINUX_BACKPORT(kvmemdup)
static inline void *kvmemdup(const void *src, size_t len, gfp_t gfp)
{
	void *p;

	p = kvmalloc(len, gfp);
	if (p)
		memcpy(p, src, len);
	return p;
}
#endif

/* also backported to 6.12.34 and 6.15.3 */
#if LINUX_VERSION_IS_LESS(6,12,34) || LINUX_VERSION_IN_RANGE(6,13,0, 6,15,3)
#include <linux/poison.h>

#define page_pool_page_is_pp LINUX_BACKPORT(page_pool_page_is_pp)
static inline bool page_pool_page_is_pp(const struct page *page)
{
/* struct page::pp_magic and PP_SIGNATURE only exist since 5.14 */
#if LINUX_VERSION_IS_LESS(5,14,0)
	WARN_ONCE(1, "page_pool_page_is_pp() is not supported before 5.14\n");
	return false;
#elif IS_ENABLED(CONFIG_PAGE_POOL)
	return (page->pp_magic & ~0x3UL) == PP_SIGNATURE;
#else
	return false;
#endif
}
#endif /* < 6.12.34 || 6.13.0 <= x < 6.15.3 */

#endif /* __BACKPORT_MM_H */
