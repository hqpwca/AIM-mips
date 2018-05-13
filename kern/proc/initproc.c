/* Copyright (C) 2016 Gan Quan <coin2028@hotmail.com>
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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <sys/types.h>
#include <raim/console.h>
#include <raim/proc.h>
#include <raim/smp.h>
#include <raim/sched.h>
//#include <fs/vfs.h>	/* fsinit() */
//#include <fs/namei.h>
//#include <fs/vnode.h>
//#include <fs/specdev.h>
//#include <fs/uio.h>



//FIXME:test
#include <raim/bus.h>
#include <raim/device.h>
#include <platform.h>
#include <libc/string.h>
#include <libc/stdio.h>



static struct proc *initproc;

char *initargv[] = {
	"/sbin/init",
	NULL
};

char *initenvp[] = {
	NULL
};

void initproc_entry(void)
{
	/*
	 * initproc mounts the root filesystem first in kernel space,
	 * then finds the /sbin/init executable, loads it, and performs
	 * an execve(2) system call to jump into user space.
	 *
	 * The reason we mount a file system in a (kernel) process is that
	 * interacting with disks involves sleep() and wakeup(), which
	 * requires a working scheduler and interrupts enabled.
	 */
	kpdebug("initproc running.\n");
	kpdebug("running on cpu %d.\n", cpuid());
	//fsinit();
	//kpdebug("FS initialized.\n");
	//ttyinit();
	//kpdebug("TTY initialized.\n");

	//current_proc->cwd = rootvnode;
	//vref(rootvnode);
	//current_proc->rootd = rootvnode;
	//vref(rootvnode);

	//execve("/sbin/init", initargv, initenvp);


////////////////////////////////////////////////////////////

struct bdev *virtblock_create(struct mmiobus *bus);
struct bdev *bd = virtblock_create(mmiobus_create((void*)VIRTBLOCK_BASE,0x1000));

/*for(int i=0;i<1000;i++){
    kprintf("i=%d\n",i);
    char buf[512];
    memset(buf,0xdd,512);
    struct bio req = {
    .blkid=i,
    .data=buf,
    .size=1,
    .write=false,
    .offset=0,
    };
    bd->io(bd, &req);
    dump(buf, sizeof(buf));
}*/

struct superblock *v6fs_superblock_create(struct bdev *bdev);
struct superblock *sb = v6fs_superblock_create(bd);
fsroot=sb->root;

//struct inode *ino = vfs_lookup(fsroot, "/usr/lib/quiz/bard", 0);
//kprintf("ino=%p\n", ino);
struct file *f = vfs_open(fsroot, "/usr/games/wump", 0);

kprintf("f = %p\n",f);

char buf[16];
vfs_read(f, (userptr)buf, sizeof(buf));
dump(buf,sizeof(buf));
vfs_read(f, (userptr)buf, sizeof(buf));
dump(buf,sizeof(buf));
vfs_read(f, (userptr)buf, sizeof(buf));
dump(buf,sizeof(buf));
vfs_read(f, (userptr)buf, sizeof(buf));
dump(buf,sizeof(buf));

vfs_close(f);




/////////////////////////////////////////////////////////////

panic("haha");

    while(1) {
        kprintf("initproc_entry() yield\n");
        schedule();
    }
    
	for (;;)
		/* nothing */;
}

void spawn_initproc(void)
{
	initproc = proc_new(NULL);
	proc_ksetup(initproc, initproc_entry, NULL);
	initproc->state = PS_RUNNABLE;
	initproc->groupleader = initproc;
	initproc->sessionleader = initproc;
	initproc->mainthread = initproc;
	proc_add(initproc);
	kputs("spawn_initproc here\n");
}

