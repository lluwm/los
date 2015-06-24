#ifndef _X86_H_
#define _X86_H_
#include <include/types.h>

// 
static __inline uint8_t inb(int port) __attribute__((always_inline));
static __inline void insl(int port, void *addr, int cnt) __attribute__((always_inline));

static __inline void outb(int port, uint8_t data) __attribute__((always_inline));
static __inline void outw(int port, uint16_t data) __attribute__((always_inline));

static __inline void invlpg(void *addr) __attribute__((always_inline));


static __inline uint8_t
inb(int port)
{
	uint8_t data;
	__asm __volatile("inb %w1, %0" : "=a" (data) : "d" (port));
	return data;
}

static __inline void
insl(int port, void *addr, int cnt)
{
	__asm __volatile("cld\n\trepne\n\tinsl"				:
					 "=D" (addr), "=c" (cnt)			:
					 "d" (port), "0" (addr), "1" (cnt)	:
					 "memory", "cc");
}

static __inline void
outb(int port, uint8_t data)
{
	__asm __volatile("outb %0, %w1" : : "a" (data), "d" (port));
}

static __inline void
outw(int port, uint16_t data)
{
	__asm __volatile("outw %0, %w1" : : "a" (data), "d" (port));
}

static __inline uint32_t
read_ebp(void)
{
	uint32_t ebp;
	__asm __volatile("movl %%ebp, %0" : "=r" (ebp));
	return ebp;
}

static __inline void
invlpg(void *addr)
{
	__asm __volatile("invlpg (%0)" : : "r" (addr): "memory");
}

#endif
