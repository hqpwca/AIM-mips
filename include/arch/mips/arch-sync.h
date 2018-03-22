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

#include <raim/debug.h>
#include <raim/smp.h>

typedef union {
	uint32_t	lock;
	struct {
		/* Assumes little-endian */
		uint16_t	head;
		uint16_t	tail;
	};
} lock_t;

#define EMPTY_LOCK(lk)	{ .lock = 0 }

#define smp_mb() \
	asm volatile ("sync" : : : "memory")

static inline void spinlock_init(lock_t *lock)
{
	lock->lock = 0;
	/* Currently as Loongson 3A automatically handles hazards and
	 * consistency, and MSIM is deterministic, we don't care about
	 * barriers here. */
}

static inline void spin_lock(lock_t *lock)
{
	uint32_t inc = 0x10000;
	uint32_t lock_val, new_lock, my_tail, head;

	/*
	 * Locking a ticket lock:
	 * 1. Atomically increment the tail by one.
	 * 2. Wait until the head becomes equal to initial value of the tail.
	 */
	asm volatile (
		"1:	.set	push;"
		"	.set	noreorder;"
		"	ll	%[lock], %[lock_ptr];"
		"	addu	%[new_lock], %[lock], %[inc];"
		"	sc	%[new_lock], %[lock_ptr];"
		"	beqz	%[new_lock], 1b;"
		"	 srl	%[my_tail], %[lock], 16;"
		"	andi	%[head], %[lock], 0xffff;"
		"	beq	%[head], %[my_tail], 9f;"
		"	 nop;"
		"2:	lhu	%[head], %[lock_head_ptr];"
		"	bne	%[head], %[my_tail], 2b;"
		"	 nop;"
		"9:	.set	pop;"
		: [lock_ptr] "+m" (lock->lock),
		  [lock_head_ptr] "+m" (lock->head),
		  [lock] "=&r" (lock_val),
		  [new_lock] "=&r" (new_lock),
		  [my_tail] "=&r" (my_tail),
		  [head] "=&r" (head)
		: [inc] "r" (inc)
	);
	smp_mb();
}

static inline void spin_unlock(lock_t *lock)
{
	unsigned int head = lock->head + 1U;
	smp_mb();
	lock->head = (uint16_t)head;
	smp_mb();
}

static inline
bool spin_is_locked(lock_t *lock)
{
	return lock->lock;
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
