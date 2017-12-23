#include "vm/page.h"

/* Hash function for supplemental page table. */
unsigned 
sup_hash (const struct hash_elem *e, void *aux)
{
  return hash_int((int)hash_entry(e, struct supPTE, elem)->uva);
}

/* Less function for supplemental page table. */
bool
sup_less (const struct hash_elem *a, const struct hash_elem *b,
		  void *aux)
{
  int A = (int)hash_entry(a, struct supPTE, elem)->uva;
  int B = (int)hash_entry(b, struct supPTE, elem)->uva;
  return A < B;
}

/* Initialize supplemental page table. */
void
init_supPT(struct hash *supPT)
{
  hash_init(supPT, sup_hash, sup_less, NULL);
}

void
destroy_supPT(struct hash *supPT)
{
  /* Not implemented yet. */
}

bool
load_page(struct supPTE *entry)
{
}

/* Returns pointer of entry of supplemental page table 
   with page where given virtual address lies if exists,
   NULL otherwise. */
struct
supPTE* get_supPTE(void *uva)
{
  /* Set current page's address as uva. */
  struct supPTE t;
  t.uva = pg_round_down(uva);

  struct hash_elem *e;
  /* If there is entry with uva calculated above,
	 return that entry. */
  if((e = hash_find(&thread_current()->supPT, &t.elem))) 
	  return hash_entry(e, struct supPTE, elem);

  /* Else, return NULL. */
  return e;
}
