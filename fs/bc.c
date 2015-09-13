#include <fs/fs.h>

/* Return the virtual address of this disk block */
void *
diskaddr(uint32_t blockno)
{
	if (blockno == 0 || (super && blockno >= super->s_nblocks))
		panic("bad block number %08x in diskaddr", blockno);

	return (char *) (DISKMAP + blockno * BLKSIZE);
}

/* Is this virtual address mapped in page table */
bool
va_is_mapped(void *va)
{
	return (uvpd[PDX(va)] & PTE_P) && (uvpt[PGNUM(va)] & PTE_P);
}

/* Is this virtual address dirty? */
bool
va_is_dirty(void *va)
{
	return (uvpt[PGNUM(va)] & PTE_D) != 0;
}

/* Fault any disk block that is read in to memory by 
 * loading it from disk.
 */
static void
bc_pgfault(struct UTrapframe *utf)
{
	void *addr = (void *) utf->utf_fault_va;
	uint32_t blockno = ((uint32_t) addr - DISKMAP) / BLKSIZE;
	int r;

	// Check that the fault was within the block cache region
	if (addr < (void *)DISKMAP || addr >= (void *)(DISKMAP + DISKSIZE))
		panic("page fault in FS: eip %08x, va %08x, err %04x",
			   utf->utf_eip, addr, utf->utf_err);

	// Sanity check the block number.
	if (super && blockno >= super->s_nblocks)
		panic("reading non-existent block %08x\n", blockno);

	// Allocate a page in the disk map region, read the contents
	// of the block from the disk into that page.
	// Hint: first round addr to page boundary. fs/ide.c has code
	// to read the disk.
	addr = ROUNDDOWN(addr, PGSIZE);

	if ((r = sys_page_alloc(0, addr, PTE_U | PTE_W | PTE_P)) < 0)
		panic("bc_pgfault: no phys mem %e", r);

	if ((r = ide_read(blockno * BLKSECTS, addr, BLKSECTS)) < 0)
		panic("bc_pgfault: ide read error %e", r);

	// Clear the dirty bit for the disk block page since we
	// just read the block from disk
	if ((r = sys_page_map(0, addr, 0, addr, uvpt[PGNUM(addr)] & PTE_SYSCALL)) < 0)
		panic("bc_pgfault: sys_page_map: %e", r);

	// Check that the block we read was allocated. Why do 
	// we do this after reading the block in?
	if (bitmap && block_is_free(blockno))
		panic("reading free block %08x\n", blockno);
}

/* Flush the contents to the block containing VA out to disk if necessary, 
 * then clear the PTE_D bit using sys_page_map. If the block is not in the
 * block cache or is not dirty, does nothing.
 *
 * Hint: Use va_is_mapped, va_is_dirty, and ide_write.
 * Hint: Use the PTE_SYSCALL constant when calling sys_page_map.
 * Hint: Don't forget to round addr down.
 */
void
flush_block(void *addr)
{
	int ret;

	uint32_t blockno = ((uint32_t) addr - DISKMAP) / BLKSIZE;

	if (addr < (void *) DISKMAP || addr >= (void *) (DISKMAP + DISKSIZE))
		panic("flush_block of bad va %08x\n", addr);

	// addr is rounded down to PGSIZE
	addr = ROUNDDOWN(addr, PGSIZE);

	pte_t pte = uvpt[PGNUM(addr)];

	// does nothing if not in the block cache
	if (!va_is_mapped(addr))
		return;

	// does nothing if not dirty
	if (!va_is_dirty(addr))
		return;

	if ((ret = ide_write(blockno * BLKSECTS, addr, BLKSECTS)) < 0)
		panic("flush_block: ide write error: %e", ret);

	if ((ret = sys_page_map(0, addr, 0, addr, pte & PTE_SYSCALL)) < 0)
		panic("flush_block: map error: %e", ret);
}


/* Test that the block cache works, by smashing the superblock and 
 * reading it back.
 */
static void
check_bc(void)
{
	Super backup;

	// back up super block
	memmove(&backup, diskaddr(1), sizeof(backup));

	// smash it
	strcpy(diskaddr(1), "OOPS!\n");
	flush_block(diskaddr(1));
	assert(va_is_mapped(diskaddr(1)));
	assert(!va_is_dirty(diskaddr(1)));

	// clear it out
	sys_page_unmap(0, diskaddr(1));
	assert(!va_is_mapped(diskaddr(1)));

	// read it back in
	assert(strcmp(diskaddr(1), "OOPS!\n") == 0);

	// fix it
	memmove(diskaddr(1), &backup, sizeof (backup));
	flush_block(diskaddr(1));

	cprintf("block cache is good\n");
	
}


void
bc_init(void)
{
	Super super;
	set_pgfault_handler(bc_pgfault);
	check_bc();

	// cache the super block by reading it once
	memmove(&super, diskaddr(1), sizeof(super));
}
