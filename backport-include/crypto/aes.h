/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _BACKPORT_CRYPTO_AES_H
#define _BACKPORT_CRYPTO_AES_H

#include <linux/version.h>
#include_next <crypto/aes.h>

#if LINUX_VERSION_IS_LESS(7,0,0)

/*
 * Compat: provide struct aes_enckey wrapping the old struct crypto_aes_ctx.
 * The new kernel (>= 7.0) introduced struct aes_enckey with arch-specific
 * layouts, but for our backport purposes we just need something that holds
 * the key schedule and can be passed to aes_encrypt().
 */
struct aes_enckey {
	struct crypto_aes_ctx ctx;
};

static inline int aes_prepareenckey(struct aes_enckey *key,
				    const u8 *in_key, size_t key_len)
{
	return aes_expandkey(&key->ctx, in_key, key_len);
}

#endif /* < 7.0 */

#endif /* _BACKPORT_CRYPTO_AES_H */
