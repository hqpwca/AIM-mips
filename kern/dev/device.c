#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <raim/device.h>


void bdev_oneblkio(struct bdev *bdev, void *ptr, uint64_t blkid, bool iswrite)
{
    struct bio req = {
        .blkid = blkid,
        .data = ptr,
        .size = 1,
        .write = iswrite,
        .offset = 0,
    };
    
    bdev->io(bdev, &req);
}
