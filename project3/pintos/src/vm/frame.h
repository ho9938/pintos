#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <list.h>

struct frame
{
	void *address;
	struct page *page;
	struct list_elem ftelem;
};

struct frame *vm_get_frame (void);
void vm_free_frame (struct frame *);
void vm_init_ft (void);

#endif /* vm/frame.h */
