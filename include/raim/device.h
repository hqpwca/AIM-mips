//by ZBY
#ifndef _DEVICE_H
#define _DEVICE_H

// the base class

#include <raim/file.h>

enum {
    DEVTYPE_NONE,
    DEVTYPE_CHAR,
    DEVTYPE_BLOCK,
};
struct device {
    int devtype;
};

// char device
struct cdev {
    struct device;
    struct file_ops *ops; // use file I/O operations directly
};

// block device
struct bio { // I/O buffer object for block device
    uint64_t blkid; // block id for this I/O request
    void *data; // data pointer for this I/O request
    size_t size; // data size for this I/O request, unit is bdev->blksz
    bool write; // read or write
    
    size_t offset; // current pointer
};
struct bdev {
    struct device;
    uint64_t blksz; // size of a block
    void (*io)(struct bdev *self, struct bio *request); // make an I/O request
};


void bdev_oneblkio(struct bdev *bdev, void *ptr, uint64_t blkid, bool iswrite); // convenient wrapper 

#define DECLBDEV(ptr) struct bdev *bdev = (void*) ptr

#endif /* _DEVICE_H */

