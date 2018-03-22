/* Copyright (C) 2016 David Gao <davidgao1001@gmail.com>
 *
 * This file is part of RAIM.
 *
 * RAIM is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * RAIM is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _ARCH_SYNC_H
#define _ARCH_SYNC_H

#ifndef __ASSEMBLER__

#include <asm.h>

typedef unsigned int lock_t;
#define EMPTY_LOCK(lock)	(UNLOCKED)

static inline
void spinlock_init(lock_t *lock)
{
	*lock = UNLOCKED;
}

static inline
void spin_lock(lock_t *lock)
{
	while (xchg(lock, LOCKED) != 0);
}

static inline
void spin_unlock(lock_t *lock)
{
	xchg(lock, UNLOCKED);
}

static inline
bool spin_is_locked(lock_t *lock)
{
	if(*lock == LOCKED)
		return true;
	else
		return false;
}

/* Semaphore */
typedef struct {
	int val;
	int limit;
	lock_t spinlock;

} semaphore_t;

static inline
void semaphore_init(semaphore_t *sem, int val)
{
	spinlock_init(&sem->spinlock);
	sem->val = val;
	sem->limit = val;
}

static inline
void semaphore_dec(semaphore_t *sem)
{
	while(1)
	{
		while(sem->val <= 0);
		spin_lock(&sem->spinlock);
		if(sem->val > 0)
		{
			sem->val --;
			spin_unlock(&sem->spinlock);
			return;
		}
		spin_unlock(&sem->spinlock);
	}
}

static inline
void semaphore_inc(semaphore_t *sem)
{
	while(1)
	{
		while(sem->val >= sem->limit);
		spin_lock(&sem->spinlock);
		if(sem->val < sem->limit)
		{
			sem->val ++;
			spin_unlock(&sem->spinlock);
			return;
		}
		spin_unlock(&sem->spinlock);
	}
}

#endif /* __ASSEMBLER__ */

#endif /* _ARCH_SYNC_H */

