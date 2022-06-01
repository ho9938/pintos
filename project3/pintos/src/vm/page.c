#include <stdio.h>
#include <debug.h>
#include <hash.h>
#include "vm/page.h"
#include "threads/vaddr.h"
#include "threads/thread.h"

static unsigned spt_hash_func (const struct hash_elem *element, void *aux);
static bool spt_less_func (const struct hash_elem *a, const struct hash_elem *b, void *aux);

struct page *
vm_get_page (void *address)
{
	struct page *page = (struct page *) malloc (sizeof (struct page));
	if (page == NULL)
		return page;

	page->address = address;
	page->frame = NULL;
	page->file = NULL;
	page->ofs = 0;
	page->read_bytes = 0;
	page->zero_bytes = 0;
	page->writable = true;

	struct hash *spt = &thread_current ()->spt;
	ASSERT (spt != NULL);

	if (!vm_spt_insert (spt, page)) {
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
	// printf("-------------------------vm_spt_insert(): %d\n", result);
	return result;
}

struct page *
vm_spt_find (struct hash *spt, void *address)
{
	struct page p;
	struct hash_elem *e;

	// printf ("-----------------vm_spt_find(): address = %x\n", address);
	p.address = pg_round_down (address);
	// printf ("-----------------vm_spt_find(): p.address = %x\n", p.address);
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
