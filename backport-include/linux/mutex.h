#ifndef __BACKPORT_LINUX_MUTEX_H
#define __BACKPORT_LINUX_MUTEX_H
#include_next <linux/mutex.h>
#include <linux/cleanup.h>

/*
 * The mutex lock guard was added together with the spinlock one by commit
 * 54da6a092431 ("locking: Introduce __cleanup() based infrastructure") in
 * 6.5, and backported to the 5.15.195 and 6.1.79 stable kernels.
 */
#if LINUX_VERSION_IS_LESS(6,5,0) &&			\
     !LINUX_VERSION_IN_RANGE(5,15,195, 5,16,0) &&	\
     !LINUX_VERSION_IN_RANGE(6,1,79, 6,2,0)
DEFINE_LOCK_GUARD_1(mutex, struct mutex,
		    mutex_lock(_T->lock),
		    mutex_unlock(_T->lock))
#endif /* LINUX_VERSION_IS_LESS(6,5,0) */

#endif /* __BACKPORT_LINUX_MUTEX_H */
