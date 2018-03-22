#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <raim/mmu.h>
#include <raim/pmm.h>
#include <raim/vmm.h>
#include <raim/device.h>
#include <raim/console.h>
#include <raim/trap.h>
#include <raim/panic.h>
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
	for(i = 0; devname[i] != '\0' ; ++i)
		dev->name[i] = devname[i];
	dev->devno = devno;

	switch(class)
	{
	case DEVCLASS_NON:
		dev->driver = *drv;
		break;
	case DEVCLASS_CHR:
		dev->chr_driver = *(struct chr_driver *)drv;
		break;
	case DEVCLASS_BLK:
		dev->blk_driver = *(struct blk_driver *)drv;
		break;
	case DEVCLASS_NET:
		dev->net_driver = *(struct net_driver *)drv;
		break;
	case DEVCLASS_BUS:
		dev->bus_driver = *(struct bus_driver *)drv;
		break;
	default:
		dev->driver = *drv;
	}

	//kprintf("Initdev: %s\n", dev->name);
}

/*
void probe_devices(void)
{

}

void discover_device(struct devtree_entry *entry)
{

}*/