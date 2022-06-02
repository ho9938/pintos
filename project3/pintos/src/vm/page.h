#ifndef VM_PAGE_H
#define VM_PAGE_H

#include "filesys/filesys.h"
#include <list.h>
#include <stdio.h>
#include <hash.h>

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

	int mapping;
	struct list_elem mmlelem;
};

struct page *vm_get_page (void *address);
void vm_free_page (struct page *);

void vm_spt_init (struct hash *spt);
void vm_spt_destroy (struct hash *spt);
bool vm_spt_insert (struct hash *spt, struct page *page);
struct page *vm_spt_find (struct hash *spt, void *address);
bool vm_spt_delete (struct hash *spt, struct page *page);

void vm_mml_init (struct list *mml);
void vm_mml_destroy (struct list *mml);
struct page *vm_get_mmap (void *address);
struct page *vm_find_mmap (int mapping);
void vm_free_mmap (struct page *page);

#endif /* vm/page.h */
