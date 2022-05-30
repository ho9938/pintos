#include "vm/frame.h"
#include "threads/palloc.h"
#include "threads/malloc.h"
#include <list.h>

static struct list frame_table;

struct frame *
vm_get_frame (void)
{
	// printf("vm_get_frame~~~~~~~~~~~~~~~~~~~~\n");
	struct frame *frame = (struct frame *) malloc (sizeof (struct frame));

	void *address = palloc_get_page (PAL_USER | PAL_ZERO);
	// ASSERT (address);
	if (!address) {
		// take new
	}	
	frame->address = address;

	// page?
	
	list_push_back (&frame_table, &frame->ftelem);
	// printf("~~~~~~~~~~~~~~~~~~~~~vm_get_frame\n");

	return frame;
}

void
vm_free_frame (struct frame *frame)
{
	// printf("vm_free_frame~~~~~~~~~~~~~~~~~~~~\n");
	// ASSERT (frame);

	palloc_free_page (frame->address);
	list_remove (&frame->ftelem);
	free (frame);
	// printf("~~~~~~~~~~~~~~~~~~~~~vm_free_frame\n");
}

void
vm_init_ft (void)
{
	list_init (&frame_table);
}
