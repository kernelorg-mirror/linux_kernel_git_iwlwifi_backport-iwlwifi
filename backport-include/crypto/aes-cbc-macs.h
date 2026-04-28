/* SPDX-License-Identifier: GPL-2.0 */
#ifndef _BACKPORT_CRYPTO_AES_CBC_MACS_H
#define _BACKPORT_CRYPTO_AES_CBC_MACS_H

#include <linux/version.h>

#if LINUX_VERSION_IS_GEQ(7,1,0)
#include_next <crypto/aes-cbc-macs.h>
#else

#include <crypto/aes.h>
#include <linux/string.h>
#include <crypto/utils.h>

/**
 * struct aes_cmac_key - Prepared key for AES-CMAC
 */
struct aes_cmac_key {
	struct aes_enckey aes;
	union {
		u8 b[AES_BLOCK_SIZE];
		__be64 w[2];
	} k_final[2];
};

/**
 * struct aes_cmac_ctx - Context for computing an AES-CMAC value
 */
struct aes_cmac_ctx {
	const struct aes_cmac_key *key;
	size_t partial_len;
	u8 h[AES_BLOCK_SIZE];
};

static inline void _bp_aes_enc(const struct aes_enckey *key,
			       u8 out[AES_BLOCK_SIZE],
			       const u8 in[AES_BLOCK_SIZE])
{
#if LINUX_VERSION_IS_GEQ(7,0,0)
	aes_encrypt(key, out, in);
#else
	aes_encrypt(&key->ctx, out, in);
#endif
}

static inline int aes_cmac_preparekey(struct aes_cmac_key *key,
				      const u8 *in_key, size_t key_len)
{
	u64 hi, lo, mask;
	int err;
	int i;

	err = aes_prepareenckey(&key->aes, in_key, key_len);
	if (err)
		return err;

	memset(key->k_final[0].b, 0, AES_BLOCK_SIZE);
	_bp_aes_enc(&key->aes, key->k_final[0].b, key->k_final[0].b);
	hi = be64_to_cpu(key->k_final[0].w[0]);
	lo = be64_to_cpu(key->k_final[0].w[1]);
	for (i = 0; i < 2; i++) {
		mask = ((s64)hi >> 63) & 0x87;
		hi = (hi << 1) ^ (lo >> 63);
		lo = (lo << 1) ^ mask;
		key->k_final[i].w[0] = cpu_to_be64(hi);
		key->k_final[i].w[1] = cpu_to_be64(lo);
	}
	return 0;
}

static inline void aes_cmac_init(struct aes_cmac_ctx *ctx,
				 const struct aes_cmac_key *key)
{
	memset(ctx, 0, sizeof(*ctx));
	ctx->key = key;
}

static inline void aes_cmac_update(struct aes_cmac_ctx *ctx,
				   const u8 *data, size_t data_len)
{
	size_t nblocks;

	if (ctx->partial_len) {
		size_t l = min(data_len, AES_BLOCK_SIZE - ctx->partial_len);

		crypto_xor(&ctx->h[ctx->partial_len], data, l);
		data += l;
		data_len -= l;
		ctx->partial_len += l;
		if (data_len == 0)
			return;
		_bp_aes_enc(&ctx->key->aes, ctx->h, ctx->h);
	}

	nblocks = data_len / AES_BLOCK_SIZE;
	data_len %= AES_BLOCK_SIZE;

	if (nblocks == 0) {
		crypto_xor(ctx->h, data, data_len);
		ctx->partial_len = data_len;
	} else if (data_len != 0) {
		while (nblocks-- > 0) {
			crypto_xor(ctx->h, data, AES_BLOCK_SIZE);
			data += AES_BLOCK_SIZE;
			_bp_aes_enc(&ctx->key->aes, ctx->h, ctx->h);
		}
		crypto_xor(ctx->h, data, data_len);
		ctx->partial_len = data_len;
	} else {
		while (nblocks-- > 1) {
			crypto_xor(ctx->h, data, AES_BLOCK_SIZE);
			data += AES_BLOCK_SIZE;
			_bp_aes_enc(&ctx->key->aes, ctx->h, ctx->h);
		}
		crypto_xor(ctx->h, data, AES_BLOCK_SIZE);
		ctx->partial_len = AES_BLOCK_SIZE;
	}
}

static inline void aes_cmac_final(struct aes_cmac_ctx *ctx,
				  u8 out[AES_BLOCK_SIZE])
{
	if (ctx->partial_len == AES_BLOCK_SIZE) {
		crypto_xor(ctx->h, ctx->key->k_final[0].b, AES_BLOCK_SIZE);
	} else {
		ctx->h[ctx->partial_len] ^= 0x80;
		crypto_xor(ctx->h, ctx->key->k_final[1].b, AES_BLOCK_SIZE);
	}
	_bp_aes_enc(&ctx->key->aes, out, ctx->h);
	memzero_explicit(ctx, sizeof(*ctx));
}

static inline void aes_cmac(const struct aes_cmac_key *key, const u8 *data,
			    size_t data_len, u8 out[AES_BLOCK_SIZE])
{
	struct aes_cmac_ctx ctx;

	aes_cmac_init(&ctx, key);
	aes_cmac_update(&ctx, data, data_len);
	aes_cmac_final(&ctx, out);
}

#endif /* < 7.1 */

#endif /* _BACKPORT_CRYPTO_AES_CBC_MACS_H */
