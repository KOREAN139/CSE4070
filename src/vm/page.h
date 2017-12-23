#ifndef VM_PAGE_H
#define VM_PAGE_H

#include <hash.h>
#include "threads/thread.h"
#include "threads/vaddr.h"

struct supPTE
{
  /* User virtual address. */
  void *uva;

  /* True if physical page is writable.
	 False otherwise. */
  bool writable;

  /* For supplemental page table. */
  struct hash_elem elem;
};

unsigned sup_hash (const struct hash_elem *e, void *aux);
bool sup_less (const struct hash_elem *a, 
		 	   const struct hash_elem *b,
			   void *aux);

void init_supPT(struct hash *supPT);
void destroy_supPT(struct hash *supPT);

bool load_page(struct supPTE *entry);

struct supPTE* get_supPTE(void *uva);

#endif /* vm/page.h */
