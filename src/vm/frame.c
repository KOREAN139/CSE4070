#include "vm/frame.h"

/* Initialize frame table and
   lock for operations on frame table. */
void
init_frame ()
{
  list_init(&frameTable);
  lock_init(&frameLock);
}
