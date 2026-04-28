#ifndef __BACKPORT_LINUX_STRING_H
#define __BACKPORT_LINUX_STRING_H
#include_next <linux/string.h>
#include <linux/version.h>

#ifndef memset_after
#define memset_after(obj, v, member)					\
({									\
	u8 *__ptr = (u8 *)(obj);					\
	typeof(v) __val = (v);						\
	memset(__ptr + offsetofend(typeof(*(obj)), member), __val,	\
	       sizeof(*(obj)) - offsetofend(typeof(*(obj)), member));	\
})
#endif

#ifndef memset_startat
#define memset_startat(obj, v, member)					\
({									\
	u8 *__ptr = (u8 *)(obj);					\
	typeof(v) __val = (v);						\
	memset(__ptr + offsetof(typeof(*(obj)), member), __val,		\
	       sizeof(*(obj)) - offsetof(typeof(*(obj)), member));	\
})
#endif

#if LINUX_VERSION_IS_LESS(5,2,0)
ssize_t backport_strscpy_pad(char *dest, const char *src, size_t count);
#define __bp_strscpy_pad3(dst, src, size)	backport_strscpy_pad(dst, src, size)
#else
#define __bp_strscpy_pad3(dst, src, size)	strscpy_pad(dst, src, size)
#endif

#if LINUX_VERSION_IS_LESS(6,9,0)
#include <linux/kernel.h>
/* Allow 2-argument strscpy_pad() where size is inferred from dest array */
#undef strscpy_pad
#define __bp_strscpy_pad0(dst, src, ...)	__bp_strscpy_pad3(dst, src, sizeof(dst) + __must_be_array(dst))
#define __bp_strscpy_pad1(dst, src, size)	__bp_strscpy_pad3(dst, src, size)
#define strscpy_pad(dst, src, ...)	\
	CONCATENATE(__bp_strscpy_pad, COUNT_ARGS(__VA_ARGS__))(dst, src, __VA_ARGS__)
#endif

#if LINUX_VERSION_IS_LESS(6,10,0)
#include <linux/overflow.h>
#define kmemdup_array LINUX_BACKPORT(kmemdup_array)
static inline void *
kmemdup_array(const void *src, size_t count, size_t element_size, gfp_t gfp)
{
	return kmemdup(src, size_mul(element_size, count), gfp);
}
#endif

#endif /* __BACKPORT_LINUX_STRING_H */
