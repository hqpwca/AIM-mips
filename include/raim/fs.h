//by ZBY
#ifndef _FS_H
#define _FS_H


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
};

struct superblock_ops {
    struct inode *(*get_inode)(struct superblock *self, uint64_t id); // read inode from disk, will increase refcnt
};



#endif
