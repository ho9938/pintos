#include <stdio.h>
#include <debug.h>
#include <hash.h>
#include <stdint.h>
#include "vm/page.h"
#include "threads/vaddr.h"
#include "threads/thread.h"
#include "threads/malloc.h"
#include "vm/frame.h"

static unsigned spt_hash_func (const struct hash_elem *element, void *aux);
static bool spt_less_func (const struct hash_elem *a, const struct hash_elem *b, void *aux);

struct page *
vm_get_page (void *address)
{
	struct page *page = (struct page *) malloc (sizeof (struct page));
	if (page == NULL)
		return page;

	page->address = pg_round_down (address);
	page->frame = NULL;
	page->file = NULL;
	page->ofs = 0;
	page->read_bytes = 0;
	page->zero_bytes = 0;
	page->writable = true;

	struct hash *spt = &thread_current ()->spt;
	ASSERT (spt != NULL);

	if (!page->address || !vm_spt_insert (spt, page)) {
		free (page);
		page = NULL;
	}

	return page;
}

void 
vm_free_page (struct page *page)
{
	ASSERT (page != NULL);

	struct hash *spt = &thread_current ()->spt;
	ASSERT (spt != NULL);

	ASSERT (vm_spt_delete (spt, page) == true);

	free (page);
}

void
vm_spt_init (struct hash *spt)
{
	hash_init (spt, spt_hash_func, spt_less_func, NULL);
}

void
vm_spt_destroy (struct hash *spt)
{
	hash_destroy (spt, NULL);
}

bool
vm_spt_insert (struct hash *spt, struct page *page)
{
	bool result = hash_insert (spt, &page->sptelem) == NULL ? true : false;
	return result;
}

struct page *
vm_spt_find (struct hash *spt, void *address)
{
	struct page p;
	struct hash_elem *e;

	p.address = pg_round_down (address);
	e = hash_find (spt, &p.sptelem);

	return e != NULL ? hash_entry (e, struct page, sptelem) : NULL;
}

bool
vm_spt_delete (struct hash *spt, struct page *page)
{
	return hash_delete (spt, &page->sptelem) != NULL ? true : false;
}

static unsigned
spt_hash_func (const struct hash_elem *element, void *aux UNUSED)
{
	struct page *p;
	p = hash_entry (element, struct page, sptelem);

	return (unsigned) p->address;
}

static bool
spt_less_func (const struct hash_elem *a, const struct hash_elem *b, void *aux UNUSED)
{
	struct page *pa, *pb;
	pa = hash_entry (a, struct page, sptelem);
	pb = hash_entry (b, struct page, sptelem);
	
	return pa->address < pb->address;
}

void
vm_mml_init (struct list *mml)
{
	list_init (mml);
}

void
vm_mml_destroy (struct list *mml)
{
	struct list_elem *e;
	struct page *p;

	e = list_begin (mml);
	while (e != list_end (mml)) {
		p = list_entry (e, struct page, mmlelem);
		process_munmap (p->mapping);
		e = list_begin (mml);
	}
}

struct page *
vm_get_mmap (void *address)
{
	/* page is already exist in spt */
	if (vm_spt_find (&thread_current ()->spt, address))
		return NULL;
		
	struct page *page = (struct page *) malloc (sizeof (struct page));
	if (page == NULL)
		return page;

	page->address = pg_round_down (address);
	page->frame = NULL;
	page->file = NULL;
	page->ofs = 0;
	page->read_bytes = 0;
	page->zero_bytes = 0;
	page->writable = true;

	if (!page->address) {
		free (page);
		return NULL;
	}

	struct list *mml = &thread_current ()->mml;
	ASSERT (mml != NULL);

	list_push_back (mml, &page->mmlelem);

	return page;
}

struct page *
vm_find_mmap (int mapping)
{
	struct list *mml = &thread_current ()->mml;
	struct list_elem *e;

	for (e = list_begin (mml); e != list_end (mml);
			e = list_next (e))
	{
		struct page *p = list_entry (e, struct page, mmlelem);
		if (p->mapping == mapping)
			return p;
	}

	/* no such mapping */
	return NULL;
}

void
vm_free_mmap (struct page *page)
{
	ASSERT (page != NULL);

	vm_free_frame (page->frame);
	list_remove (&page->mmlelem);
	free (page);
}
