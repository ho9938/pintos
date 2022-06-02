#include "vm/frame.h"
#include "threads/palloc.h"
#include "threads/malloc.h"
#include <list.h>
#include <debug.h>
#include "vm/page.h"

static struct list frame_table;

struct frame *
vm_get_frame (void)
{
	struct frame *frame = (struct frame *) malloc (sizeof (struct frame));

	void *address = palloc_get_page (PAL_USER | PAL_ZERO);
	// printf ("-------------------vm_get_frame (): %x\n", address);
	if (!address) {
		// printf ("------------------------palloc failed\n");
		ASSERT (false);
	}	
	frame->address = address;

	// page?
	
	list_push_back (&frame_table, &frame->ftelem);

	return frame;
}

void
vm_free_frame (struct frame *frame)
{
	ASSERT (frame);

	// printf ("----------------before vm_free_frame\n");
	palloc_free_page (frame->address);
	// printf ("----------------after vm_free_frame\n");
	list_remove (&frame->ftelem);
	frame->page->frame = NULL;
	free (frame);
}

void
vm_ft_init (void)
{
	list_init (&frame_table);
}
