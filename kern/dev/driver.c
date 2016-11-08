#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <aim/mmu.h>
#include <aim/pmm.h>
#include <aim/vmm.h>
#include <aim/device.h>
#include <aim/console.h>
#include <aim/trap.h>
#include <aim/panic.h>
#include <libc/string.h>
#include <errno.h>

#define MAX_DRIVER_NUM 32
struct driver *devsw[MAX_DRIVER_NUM];
static int dev_pos = 0;

void register_driver(unsigned int major, struct driver *drv)
{
	devsw[dev_pos] = drv;
	dev_pos ++;
}

void initdev(struct device *dev, int class, const char *devname, dev_t devno, struct driver *drv)
{
	int i;

	dev->class = class;
	for(i = 0; i < (int)sizeof(devname); ++i)
		dev->name[i] = devname[i];
	dev->devno = devno;
	dev->driver = *drv;
}

/*
void probe_devices(void)
{

}

void discover_device(struct devtree_entry *entry)
{

}*/