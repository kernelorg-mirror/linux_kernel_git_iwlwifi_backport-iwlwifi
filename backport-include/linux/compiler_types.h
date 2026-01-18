#ifndef _BACKPORTS_LINUX_COMPILER_TYPES_H
#define _BACKPORTS_LINUX_COMPILER_TYPES_H 1
#include_next <linux/compiler_types.h>

#ifndef __has_builtin
#define __has_builtin(x) (0)
#endif

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 202311L
# define auto __auto_type
#endif

/*
 * When the size of an allocated object is needed, use the best available
 * mechanism to find it. (For cases where sizeof() cannot be used.)
 */
#if __has_builtin(__builtin_dynamic_object_size)
#define __struct_size(p)	__builtin_dynamic_object_size(p, 0)
#define __member_size(p)	__builtin_dynamic_object_size(p, 1)
#else
#define __struct_size(p)	__builtin_object_size(p, 0)
#define __member_size(p)	__builtin_object_size(p, 1)
#endif

#endif /* _BACKPORTS_LINUX_COMPILER_TYPES_H */
