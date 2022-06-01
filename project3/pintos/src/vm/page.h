#ifndef VM_PAGE_H
#define VM_PAGE_H

#include "filesys/filesys.h"

struct page
{
	void *address;
	struct frame *frame;

	struct file *file;
	off_t ofs;
	uint32_t read_bytes;
	uint32_t zero_bytes;
	bool writable;

	struct hash_elem sptelem;
};

struct page *vm_get_page (void *address);
void vm_free_page (struct page *);

void vm_spt_init (struct hash *spt);
void vm_spt_destroy (struct hash *spt);
bool vm_spt_insert (struct hash *spt, struct page *page);
struct page *vm_spt_find (struct hash *spt, void *address);
bool vm_spt_delete (struct hash *spt, struct page *page);

#endif /* vm/page.h */
