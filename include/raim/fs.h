//by ZBY
#ifndef _FS_H
#define _FS_H

#include <raim/uvm.h>

#define MAXPATH 300


struct inode { // vfs-inode object
    struct inode_ops *ops;
    
    struct superblock *sb;
    
    uint64_t id; // inode id
    
    uint64_t length; // file length (in bytes)
    
    atomic_t ref; // reference count
    
    struct device *dev; // if not regular file, this points to a struct device
    
};

struct inode_ops {
    void (*decref)(struct inode *self); // decrease reference count, if zero, free it
    struct inode *(*lookup)(struct inode *self, const char *path, int flags); // find a existing file/dir in current directory
};

#define DECLINO(type,ptr) type *ino = (void*)ptr
extern struct inode *inode_ctor(struct inode *ino, struct superblock *sb);
extern struct inode *inode_addref(struct inode *ino);
extern atomic_t inode_decref(struct inode *ino);



struct superblock {
    struct superblock_ops *ops;
    struct device *dev; // underlying device
    struct inode *root;
    struct file_ops *fops;
};

struct superblock_ops {
    struct inode *(*get_inode)(struct superblock *self, uint64_t id); // read inode from disk, will increase refcnt
};




extern struct inode *fsroot;


extern struct file *vfs_open(struct inode *curdir, const char *path, int flags);
extern void vfs_close(struct file *filp);
extern ssize_t vfs_read(struct file *filp, userptr dest, size_t len);

#endif
