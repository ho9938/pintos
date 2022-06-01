#include "vm/frame.h"
#include "threads/palloc.h"
#include "threads/malloc.h"
#include <list.h>
#include <debug.h>

static struct list frame_table;

struct frame *
vm_get_frame (void)
{
	struct frame *frame = (struct frame *) malloc (sizeof (struct frame));

	void *address = palloc_get_page (PAL_USER | PAL_ZERO);
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

	palloc_free_page (frame->address);
	list_remove (&frame->ftelem);
	free (frame);
}

void
vm_ft_init (void)
{
	list_init (&frame_table);
}
