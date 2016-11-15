#ifdef HAVE_CONFIG_H
#include <config.h>
#endif /* HAVE_CONFIG_H */

#include <sys/types.h>
#include <aim/mmu.h>
#include <aim/smp.h>
#include <aim/console.h>
#include <aim/panic.h>
#include <aim/trap.h>
#include <arch-mp.h>
#include <arch-lapic.h>
#include <asm.h>

int ismp;
int ncpu;
uint8_t ioapicid;

static uint8_t
sum(uint8_t *addr, int len)
{
	int i, sum;

	sum = 0;
	for(i=0; i<len; i++)
		sum += addr[i];
	return sum;
}

// Look for an MP structure in the len bytes at addr.
static struct mp*
mpsearch1(uint32_t a, int len)
{
	uint8_t *e, *p, *addr;

	addr = postmap_addr(a);
	e = addr+len;
	for(p = addr; p < e; p += sizeof(struct mp))
		if(memcmp(p, "_MP_", 4) == 0 && sum(p, sizeof(struct mp)) == 0)
			return (struct mp*)p;
	return 0;
}

// Search for the MP Floating Pointer Structure, which according to the
// spec is in one of the following three locations:
// 1) in the first KB of the EBDA;
// 2) in the last KB of system base memory;
// 3) in the BIOS ROM between 0xE0000 and 0xFFFFF.
static struct mp*
mpsearch(void)
{
	uint8_t *bda;
	uint32_t p;
	struct mp *mp;

	bda = (uint8_t *) postmap_addr(0x400);
	if((p = ((bda[0x0F]<<8)| bda[0x0E]) << 4)){
		if((mp = mpsearch1(p, 1024)))
			return mp;
	} else {
		p = ((bda[0x14]<<8)|bda[0x13])*1024;
		if((mp = mpsearch1(p-1024, 1024)))
			return mp;
	}
	return mpsearch1(0xF0000, 0x10000);
}

// Search for an MP configuration table.  For now,
// don't accept the default configurations (physaddr == 0).
// Check for correct signature, calculate the checksum and,
// if correct, check the version.
// To do: check extended table checksum.
static struct mpconf*
mpconfig(struct mp **pmp)
{
	struct mpconf *conf;
	struct mp *mp;

	if((mp = mpsearch()) == 0 || mp->physaddr == 0)
		return 0;
	conf = (struct mpconf*) postmap_addr((uint32_t) mp->physaddr);
	if(memcmp(conf, "PCMP", 4) != 0)
		return 0;
	if(conf->version != 1 && conf->version != 4)
		return 0;
	if(sum((uint8_t*)conf, conf->length) != 0)
		return 0;
	*pmp = mp;
	return conf;
}

void
mpinit(void)
{
	uint8_t *p, *e;
	struct mp *mp;
	struct mpconf *conf;
	struct mpproc *proc;
	struct mpioapic *ioapic;

	if((conf = mpconfig(&mp)) == 0)
		return;
	ismp = 1;
	lapic = (uint32_t*)conf->lapicaddr;
	kprintf("Local APIC addr : 0x%x\n", lapic);
	for(p=(uint8_t*)(conf+1), e=(uint8_t*)conf+conf->length; p<e; ){
		switch(*p){
		case MPPROC:
			proc = (struct mpproc*)p;
			if(ncpu < CPUNUM) {
				cpus[ncpu].apicid = proc->apicid;  // apicid may differ from ncpu
				ncpu++;
			}
			p += sizeof(struct mpproc);
			kprintf("CPU NO.%d Local APIC ID : 0x%x\n", ncpu - 1, proc->apicid);
			continue;
		case MPIOAPIC:
			ioapic = (struct mpioapic*)p;
			ioapicid = ioapic->apicno;
			p += sizeof(struct mpioapic);
			kprintf("IO APIC addr: 0x%x, ID : 0x%x\n", ioapic, ioapicid);
			continue;
		case MPBUS:
		case MPIOINTR:
		case MPLINTR:
			p += 8;
			continue;
		default:
			ismp = 0;
			break;
		}
	}
	if(!ismp){
		// Didn't like what we found; fall back to no MP.
		ncpu = 1;
		lapic = 0;
		ioapicid = 0;

		panic("mpinit failed!\n");
		return;
	}

	if(mp->imcrp){
		// Bochs doesn't support IMCR, so this doesn't run on Bochs.
		// But it would on real hardware.
		outb(0x22, 0x70);   // Select IMCR
		outb(0x23, inb(0x23) | 1);  // Mask external interrupts.
	}
}