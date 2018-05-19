//by ZBY
#ifndef _FILE_H
#define _FILE_H

#include <raim/fs.h>
#include <raim/uvm.h>

struct file { // represent a file descriptor of a process
    struct file_ops *ops;
    
    struct inode *inode;
    uint64_t pos; // file pointer
};

struct file_ops {
    int (*at_open)(struct file *filp);
    void (*at_close)(struct file *filp);
    ssize_t (*io)(struct file *filp, userptr uptr, size_t len, uint64_t pos, bool iswrite);
};

#endif
