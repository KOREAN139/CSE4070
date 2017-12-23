#ifndef VM_FRAME_H
#define VM_FRAME_H

#include <list.h>
#include "threads/thread.h"
#include "vm/page.h"

/* Lock for frame table. */
struct lock frameLock;

/* Frame table. */
struct list frameTable;

struct frameEntry
{
  /* Physical addres.s */
  void *kpa;

  /* Thread using this frame. */
  struct thread *owner;

  /* For pointing supplemental page table entry
	 for current frame. */
  struct supPTE *spte;

  /* For frame table. */
  struct list_elem elem;
};

#endif // vm/frame.h
