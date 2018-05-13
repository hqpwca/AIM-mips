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
    ssize_t (*read)(struct file *filp, userptr dest, size_t len, uint64_t pos);
    ssize_t (*write)(struct file *filp, userptr src, size_t len, uint64_t pos);
};

#endif
