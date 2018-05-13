// by ZBY
// Virtual File System for RAIM

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <raim/fs.h>
#include <libc/string.h>
#include <raim/console.h>
#include <raim/vmm.h>
#include <raim/file.h>


struct inode *inode_ctor(struct inode *ino, struct superblock *sb)
{
    ino->ref = 0;
    ino->sb = sb;
    return inode_addref(ino);
}

struct inode *inode_addref(struct inode *ino)
{
    ino->ref++; // FIXME: use atomic increase
    return ino;
}
atomic_t inode_decref(struct inode *ino) // should not called directly! call ino->ops->decref(ino)
{
    return --ino->ref; //FIXME: use atomic decrease
}


/////////////////////// vfs


struct inode *fsroot;




static struct inode *vfs_lookup(struct inode *curdir, const char *path, int flags) // lookup a inode, will increase ref
{
    char buf[MAXPATH];
    char *ptr;
    size_t len;
    
    int endflag = 0;
    
    struct inode *nextino;
    
    if (*path == '/') { curdir = fsroot; path++; }
    
    inode_addref(curdir);

    while (1) {
    
        // split path string    
        ptr = strchr(path, '/');
        if (ptr) {
            len = ptr - path;
        } else {
            len = strlen(path);
            endflag = 1;
        }
        if (len >= MAXPATH) return NULL;
        memcpy(buf, path, len);
        buf[len] = 0;
        if (ptr) while (*ptr == '/') ptr++;
        path = ptr;
        
        kprintf("buf=%s\n", buf);
        
        nextino = curdir->ops->lookup(curdir, buf, endflag ? flags : 0);
        if (nextino == NULL) return NULL;
        if (endflag) return nextino;
    
        curdir->ops->decref(curdir);
        curdir = nextino;
    }
}

static void vfs_freefile(struct file *filp)
{
    if (filp->inode) filp->inode->ops->decref(filp->inode);
    kfree(filp);
}


struct file *vfs_open(struct inode *curdir, const char *path, int flags)
{
    struct inode *ino = vfs_lookup(curdir, path, flags);
    if (!ino) return NULL;
    struct file *filp = kmalloc(sizeof(struct file), 0);
    *filp = (struct file) {
        .ops = ino->sb->fops,
        .inode = ino,
        .pos = 0,
    };
    int r = filp->ops->at_open(filp);
    if (r < 0) {
        vfs_freefile(filp);
        return NULL;
    }
    
    return filp;
}


void vfs_close(struct file *filp)
{
    filp->ops->at_close(filp);
    vfs_freefile(filp);
}

ssize_t vfs_read(struct file *filp, userptr dest, size_t len)
{
    ssize_t sz = filp->ops->read(filp, dest, len, filp->pos);
    if (sz > 0) filp->pos += sz;
    return sz;
}

