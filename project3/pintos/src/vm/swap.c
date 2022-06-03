#include <bitmap.h>
#include "vm/swap.h"
#include "devices/block.h"
#include "vm/frame.h"
#include "vm/page.h"
#include "threads/vaddr.h"
#include "userprog/process.h"

static struct block *swap_block;
static block_sector_t ss_size; 
static block_sector_t spp;			/* sector per page */
static struct bitmap *swap_slot;

void 
vm_ss_init (void)
{
	swap_block = block_get_role (BLOCK_SWAP);
	ss_size = block_size (swap_block);
	spp = PGSIZE / BLOCK_SECTOR_SIZE;
	swap_slot = bitmap_create (ss_size);
	bitmap_set_all (swap_slot, false);
}

int
vm_swap_out (struct frame *frame)
{
	block_sector_t bs = bitmap_scan (swap_slot, 0, spp, false); 	
	ASSERT (bs != BITMAP_ERROR);
	frame->ss_idx = bs;

	block_sector_t idx;
	for (idx = 0; idx < spp; idx ++)
		block_write (swap_block, bs + idx, frame->page->address + BLOCK_SECTOR_SIZE * idx);

	bitmap_set_multiple (swap_slot, bs, spp, true);

	process_swap_out (frame);

	vm_ft_remove (frame);

	return bs;
}

bool
vm_swap_in (struct frame *frame)
{
	if (!process_swap_in (frame))
		return false;
	
	block_sector_t bs = frame->ss_idx;

	block_sector_t idx;
	for (idx = 0; idx < spp; idx ++)
		block_read (swap_block, bs + idx, frame->page->address + BLOCK_SECTOR_SIZE * idx);

	bitmap_set_multiple (swap_slot, bs, spp, false);

	vm_ft_insert (frame);

	return true;
}
