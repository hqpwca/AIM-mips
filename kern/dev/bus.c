#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <raim/bus.h>
#include <raim/debug.h>
#include <raim/io.h>
#include <raim/vmm.h>
#include <util.h>


// mmiobus

uint8_t  mmiobus_r8 (struct bus *bself, unsigned long offset) { DECLSELF(struct mmiobus); assert(ADDLEQ(offset,8 ,self->limit)); return read8 (self->base+offset); }
uint16_t mmiobus_r16(struct bus *bself, unsigned long offset) { DECLSELF(struct mmiobus); assert(ADDLEQ(offset,16,self->limit)); return read16(self->base+offset); }
uint32_t mmiobus_r32(struct bus *bself, unsigned long offset) { DECLSELF(struct mmiobus); assert(ADDLEQ(offset,32,self->limit)); return read32(self->base+offset); }
uint64_t mmiobus_r64(struct bus *bself, unsigned long offset) { DECLSELF(struct mmiobus); assert(ADDLEQ(offset,64,self->limit)); return read64(self->base+offset); }
void mmiobus_w8 (struct bus *bself, unsigned long offset, uint8_t  value) { DECLSELF(struct mmiobus); assert(ADDLEQ(offset,8 ,self->limit)); write8 (self->base+offset,value); }
void mmiobus_w16(struct bus *bself, unsigned long offset, uint16_t value) { DECLSELF(struct mmiobus); assert(ADDLEQ(offset,16,self->limit)); write16(self->base+offset,value); }
void mmiobus_w32(struct bus *bself, unsigned long offset, uint32_t value) { DECLSELF(struct mmiobus); assert(ADDLEQ(offset,32,self->limit)); write32(self->base+offset,value); }
void mmiobus_w64(struct bus *bself, unsigned long offset, uint64_t value) { DECLSELF(struct mmiobus); assert(ADDLEQ(offset,64,self->limit)); write64(self->base+offset,value); }

struct mmiobus *mmiobus_create(void *base, size_t limit)
{
    struct mmiobus *self = NEW(struct mmiobus);
    *self = (struct mmiobus) {
        .r8=mmiobus_r8,.r16=mmiobus_r16,.r32=mmiobus_r32,.r64=mmiobus_r64,
        .w8=mmiobus_w8,.w16=mmiobus_w16,.w32=mmiobus_w32,.w64=mmiobus_w64,
        .base=base,.limit=limit
    };
    return self;
}

