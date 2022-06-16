#include "vm/frame.h"
#include "threads/palloc.h"
#include "threads/malloc.h"
#include <list.h>
#include <debug.h>
#include "vm/page.h"
#include "vm/swap.h"
#include "threads/thread.h"
#include "userprog/pagedir.h"

static struct list frame_table;

struct frame *
vm_get_frame (struct page *page)
{
	struct frame *frame = (struct frame *) malloc (sizeof (struct frame));

	void *address = palloc_get_page (PAL_USER | PAL_ZERO);
	if (!address) {
		vm_swap_out (vm_ft_victim ());
		address = palloc_get_page (PAL_USER | PAL_ZERO);
	}	
	frame->address = address;

	frame->page = page;
	frame->ss_idx = 0;
	
	vm_ft_insert (frame);

	return frame;
}

void
vm_free_frame (struct frame *frame)
{
	ASSERT (frame);

	palloc_free_page (frame->address);
	vm_ft_remove (frame);
	frame->page->frame = NULL;
	free (frame);
}

void
vm_ft_init (void)
{
	list_init (&frame_table);
}

void
vm_ft_insert (struct frame *frame)
{
	list_push_back (&frame_table, &frame->ftelem);
}

void
vm_ft_remove (struct frame *frame)
{
	list_remove (&frame->ftelem);
}

struct frame *
vm_ft_victim (void)
{
	uint32_t *pd = thread_current ()->pagedir;

	struct list_elem *e;
	struct frame *f;

	while (true)
	{
		e = list_begin (&frame_table);
		f = list_entry (e, struct frame, ftelem);

		if (!pagedir_is_accessed (pd, f->page->address))
			return f;
		else {
			pagedir_set_accessed (pd, f->page->address, false);
			vm_ft_remove (f);
			vm_ft_insert (f);
		}
	}

	NOT_REACHED ();
}
