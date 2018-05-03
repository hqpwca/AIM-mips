//by ZBY
#ifndef _BUS_H
#define _BUS_H

// the base class
struct bus {
    // read virtual functions
    uint8_t (*r8)(struct bus *self, unsigned long offset);
    uint16_t (*r16)(struct bus *self, unsigned long offset);
    uint32_t (*r32)(struct bus *self, unsigned long offset);
    uint64_t (*r64)(struct bus *self, unsigned long offset);
    
    // write virtual functions
    void (*w8)(struct bus *self, unsigned long offset, uint8_t value);
    void (*w16)(struct bus *self, unsigned long offset, uint16_t value);
    void (*w32)(struct bus *self, unsigned long offset, uint32_t value);
    void (*w64)(struct bus *self, unsigned long offset, uint64_t value);  
};
#define  bus_r8(bus,offset) ((bus)->r8 )(bus,offset)
#define bus_r16(bus,offset) ((bus)->r16)(bus,offset)
#define bus_r32(bus,offset) ((bus)->r32)(bus,offset)
#define bus_r64(bus,offset) ((bus)->r64)(bus,offset)
#define  bus_w8(bus,offset,value) ((bus)->w8 )(bus,offset,value)
#define bus_w16(bus,offset,value) ((bus)->w16)(bus,offset,value)
#define bus_w32(bus,offset,value) ((bus)->w32)(bus,offset,value)
#define bus_w64(bus,offset,value) ((bus)->w64)(bus,offset,value)



// memory maped I/O bus
struct mmiobus {
    struct bus;
    void *base;
    size_t limit;
};

struct mmiobus *mmiobus_create(void *base, size_t limit);



#endif /* _BUS_H */
