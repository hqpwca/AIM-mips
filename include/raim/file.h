//by ZBY
#ifndef _FILE_H
#define _FILE_H

#include <raim/fs.h>
#include <raim/uvm.h>

struct file { // represent a file descriptor of a process
    struct file_ops *ops;
    
    uint64_t pos; // file pointer
    struct inode *inode;
    
};

struct file_ops {
    int (*at_open)(struct file *filp, struct inode *inode);
    void (*at_close)(struct file *filp);
    size_t (*read)(struct file *filp, userptr dest, size_t len, uint64_t pos);
    size_t (*write)(struct file *filp, userptr src, size_t len, uint64_t pos);
    uint64_t (*lseek)(struct file *filp, uint64_t offset, int whence);
};

#endif
