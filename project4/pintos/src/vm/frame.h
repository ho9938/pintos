#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <list.h>

struct frame
{
	void *address;
	struct page *page;
	struct list_elem ftelem;
	size_t ss_idx;
};

struct frame *vm_get_frame (struct page *page);
void vm_free_frame (struct frame *);

void vm_ft_init (void);
void vm_ft_insert (struct frame *frame);
void vm_ft_remove (struct frame *frame);

struct frame *vm_ft_victim (void);

#endif /* vm/frame.h */
