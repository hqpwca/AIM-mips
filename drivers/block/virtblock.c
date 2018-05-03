// virtio-block driver for RAIM
// 2018.4.24 by ZBY

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <raim/device.h>
#include <raim/bus.h>
#include <raim/vmm.h>
#include <raim/console.h>
#include <libc/string.h>

#define VIRTIO_ACKNOWLEDGE 1
#define VIRTIO_DRIVER 2
#define VIRTIO_FAILED 128
#define VIRTIO_FEATURES_OK 8
#define VIRTIO_DRIVER_OK 4
#define VIRTIO_DEVICE_NEEDS_RESET 64

#define VIRTIO_BLK_F_SIZE_MAX 1
#define VIRTIO_BLK_F_SEG_MAX 2
#define VIRTIO_BLK_F_GEOMETRY 4
#define VIRTIO_BLK_F_RO 5
#define VIRTIO_BLK_F_BLK_SIZE 6
#define VIRTIO_BLK_F_FLUSH 9
#define VIRTIO_BLK_F_TOPOLOGY 10
#define VIRTIO_BLK_F_CONFIG_WCE 11

#define VIRTIO_BLK_T_IN           0 
#define VIRTIO_BLK_T_OUT          1 
#define VIRTIO_BLK_T_FLUSH        4 

#define VIRTIO_BLK_S_OK        0 
#define VIRTIO_BLK_S_IOERR     1 
#define VIRTIO_BLK_S_UNSUPP    2 

typedef uint16_t le16; // FIXME:check endianess
typedef uint32_t le32;
typedef uint64_t le64;
typedef uint8_t u8;



struct virtq_desc { 
        /* Address (guest-physical). */ 
        le64 addr; 
        /* Length. */ 
        le32 len; 
 
/* This marks a buffer as continuing via the next field. */ 
#define VIRTQ_DESC_F_NEXT   1 
/* This marks a buffer as device write-only (otherwise device read-only). */ 
#define VIRTQ_DESC_F_WRITE     2 
/* This means the buffer contains a list of buffer descriptors. */ 
#define VIRTQ_DESC_F_INDIRECT   4 
        /* The flags as indicated above. */ 
        le16 flags; 
        /* Next field if flags & NEXT */ 
        le16 next; 
}; 

struct virtq_avail { 
#define VIRTQ_AVAIL_F_NO_INTERRUPT      1 
        le16 flags; 
        le16 idx; 
        le16 ring[ /* Queue Size */ ]; 
        //le16 used_event; /* Only if VIRTIO_F_EVENT_IDX */ 
}; 

/* le32 is used here for ids for padding reasons. */ 
struct virtq_used_elem { 
        /* Index of start of used descriptor chain. */ 
        le32 id; 
        /* Total length of the descriptor chain which was used (written to) */ 
        le32 len; 
}; 


struct virtq_used { 
#define VIRTQ_USED_F_NO_NOTIFY  1 
        le16 flags; 
        le16 idx; 
        struct virtq_used_elem ring[ /* Queue Size */]; 
        //le16 avail_event; /* Only if VIRTIO_F_EVENT_IDX */ 
}; 
 

struct virtio_blk_req_header { 
#define VIRTIO_BLK_T_IN           0 
#define VIRTIO_BLK_T_OUT          1 
#define VIRTIO_BLK_T_FLUSH        4 
    le32 type; 
    le32 reserved; 
    le64 sector; 
};
//    u8 data[512]; 
#define VIRTIO_BLK_S_OK        0 
#define VIRTIO_BLK_S_IOERR     1 
#define VIRTIO_BLK_S_UNSUPP    2 
//    u8 status;
 





struct virtblock {
    struct bdev;
    struct mmiobus *bus;
    
    uint32_t DeviceFeatures;
    uint64_t Capacity; // in 512bytes sector
    uint32_t QueueSize;
    size_t OffsetOfAvail;
    size_t OffsetOfUsed;
    struct pages QueuePages;
    
    struct virtq_desc *QueueDesc;
    struct virtq_avail *QueueAvail;
    volatile struct virtq_used *QueueUsed;
    int NextBuffer;
};








static void virtblock_io(struct bdev *bself, struct bio *request)
{
    DECLSELF(struct virtblock);
    
    for (;request->offset < request->size; request->offset++) {
        int oldusedidx = self->QueueUsed->idx;
        
        // make request header and tail
        u8 rqstatus = 0;
        struct virtio_blk_req_header rqh = {
            .type = request->write ? VIRTIO_BLK_T_OUT : VIRTIO_BLK_T_IN,
            .reserved = 0,
            .sector = request->blkid + request->offset
        };
        
        // alloc buffer id
        assert(self->QueueSize >= 3);
        int bufid0 = self->NextBuffer % self->QueueSize;
        int bufid1 = (self->NextBuffer+1) % self->QueueSize;
        int bufid2 = (self->NextBuffer+2) % self->QueueSize;
        self->NextBuffer += 3;
        
        // make buffer
        self->QueueDesc[bufid0] = (struct virtq_desc) {
            .addr = (uintptr_t)&rqh - KOFFSET,
            .len = sizeof(rqh),
            .flags = VIRTQ_DESC_F_NEXT,
            .next = bufid1,
        };
        self->QueueDesc[bufid1] = (struct virtq_desc) {
            .addr = (uintptr_t)request->data + 512*request->offset - KOFFSET,
            .len = 512,
            .flags = (request->write ? 0 : VIRTQ_DESC_F_WRITE)|VIRTQ_DESC_F_NEXT,
            .next = bufid2,
        };
        self->QueueDesc[bufid2] = (struct virtq_desc) {
            .addr = (uintptr_t)&rqstatus - KOFFSET,
            .len = 1,
            .flags = VIRTQ_DESC_F_WRITE,
            .next = 0,
        };
        
        
        // submit to avail
        int index = self->QueueAvail->idx % self->QueueSize;
        self->QueueAvail->ring[index] = bufid0;
        self->QueueAvail->idx++;
        
        
        bus_w32(self->bus,0x050,0);//Notify Queue 0
        
        while (self->QueueUsed->idx == oldusedidx);
    }
}


struct bdev *virtblock_create(struct mmiobus *bus)
{
    struct virtblock *self = kmalloc(sizeof(struct virtblock), 0);
    *self = (struct virtblock) {
        .blksz = 512,
        .io = virtblock_io,
        .bus = bus,
    };
    
    kprintf("virtio-block driver for RAIM initializing ...\n");
    kprintf("MagicValue                 : %08X\n", bus_r32(bus, 0x000));
    kprintf("Version                    : %08X\n", bus_r32(bus, 0x004));
    kprintf("DeviceID                   : %08X\n", bus_r32(bus, 0x008));
    kprintf("VendorID                   : %08X\n", bus_r32(bus, 0x00C));
    kprintf("DeviceFeatures             : %08X\n", bus_r32(bus, 0x010));
    assert(bus_r32(bus, 0x000)==0x74726976);
    assert(bus_r32(bus, 0x004)==0x00000001);
    assert(bus_r32(bus, 0x008)==0x00000002);
    
    
    //3.1.1 Driver Requirements: Device Initialization
    //3.1.2 Legacy Interface: Device Initialization
    bus_w32(bus,0x070,0);                                     //1. Reset the device.
    bus_w32(bus,0x070,bus_r32(bus,0x070)|VIRTIO_ACKNOWLEDGE); //2. Set the ACKNOWLEDGE status bit
    bus_w32(bus,0x070,bus_r32(bus,0x070)|VIRTIO_DRIVER);      //3. Set the DRIVER status bit
    bus_w32(bus,0x024,(1<<VIRTIO_BLK_F_BLK_SIZE)|(1<<VIRTIO_BLK_F_CONFIG_WCE));//DeviceFeaturesSel
    self->DeviceFeatures = bus_r32(bus,0x010);                //4. Read device feature bits
    
    
    kprintf("Capacity                   : %llX sectors\n", bus_r64(bus, 0x100));
    self->Capacity = bus_r64(bus,0x100);
    
    if ((self->DeviceFeatures&(1<<VIRTIO_BLK_F_BLK_SIZE)))
        kprintf("BlockSize                  : %08X\n", bus_r32(bus, 0x114));
    if ((self->DeviceFeatures&(1<<VIRTIO_BLK_F_CONFIG_WCE))) {
        bus_w8(bus,0x11C,0);
        kprintf("WriteBack                  : %02X\n", bus_r8(bus, 0x11C));
    }
    
    
    // init queue
    bus_w32(bus,0x028,PAGE_SIZE);//GuestPageSize
    bus_w32(bus,0x030,0);//1.Select the queue writing its index (first queue is 0) to QueueSel. 
    assert(bus_r32(bus,0x040)==0);//2.Check if the queue is not already in use: read QueuePFN, expecting a returned value of zero (0x0).
    self->QueueSize = bus_r32(bus,0x034);//3.Read maximum queue size (number of elements) from QueueNumMax. If the returned value is zero (0x0) the queue is not available. 
    //4.Allocate and zero the queue pages in contiguous virtual memory, aligning the Used Ring to an optimal boundary (usually page size). The driver should choose a queue size smaller than or equal to QueueNumMax.
    kprintf("QueueSize                  : %08X\n", self->QueueSize);
    self->OffsetOfAvail = sizeof(struct virtq_desc)*self->QueueSize;
    self->OffsetOfUsed  = ROUNDUP(self->OffsetOfAvail + sizeof(uint16_t)*(3 + self->QueueSize), PAGE_SIZE);
    size_t qsize = self->OffsetOfUsed + ROUNDUP(sizeof(uint16_t)*3 + sizeof(struct virtq_used_elem)*self->QueueSize, PAGE_SIZE);
    kprintf("need alloc %016llX bytes of queue memory\n", (uint64_t)qsize);
    self->QueuePages = (struct pages) {
        .paddr=0, .size=qsize, .flags=0
    };
    int r = alloc_pages(&self->QueuePages);
    assert(r==0);
    self->QueueDesc = (void*)(self->QueuePages.paddr + KOFFSET);
    self->QueueAvail = ((void*)self->QueueDesc) + self->OffsetOfAvail;
    self->QueueUsed = ((void*)self->QueueDesc) + self->OffsetOfUsed;
    memset(self->QueueDesc,0,qsize);
    bus_w32(bus,0x038,self->QueueSize);//5.Notify the device about the queue size by writing the size to QueueNum. 
    bus_w32(bus,0x03C,PAGE_SIZE);//6.Notify the device about the used alignment by writing its value in bytes to QueueAlign
    bus_w32(bus,0x040,self->QueuePages.paddr / PAGE_SIZE);//7.Write the physical number of the first page of the queue to the QueuePFN register.


    bus_w32(bus,0x070,bus_r32(bus,0x070)|VIRTIO_DRIVER_OK);   //8. Set the DRIVER_OK status bit
    
    self->NextBuffer = 0;
    
    kprintf("status=%08X\n", bus_r32(bus,0x070));

    
    
    return self;
}


