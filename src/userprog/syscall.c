#include "userprog/process.h"
#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include <user/syscall.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/init.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#include "filesys/filesys.h"

/* Get pointer that stores IDX-th argument's addr in INTFRM. */
#define GET_ARGPTR(INTFRM, IDX) ((void *)((uintptr_t)(INTFRM)->esp + 4 * (IDX)))

static void syscall_handler (struct intr_frame *);

/* Project 1. */
void syscall_halt (void);
void syscall_exit (int status);
pid_t syscall_exec (const char *cmd_line);
int syscall_wait (pid_t pid);
int syscall_fib (int n);
int syscall_sumFour (int a, int b, int c, int d);
bool isVargs (struct intr_frame *f, int n);

/* Project 2 (maybe). */
bool syscall_create (const char *file, unsigned initial_size);
bool syscall_remove (const char *file);
int syscall_open (const char *file);
int syscall_filesize (int fd);
int syscall_read (int fd, void *buffer, unsigned size);
int syscall_write (int fd, const void *buffer, unsigned size);
void syscall_seek (int fd, unsigned position);
unsigned syscall_tell (int fd);
void syscall_close (int fd);

/* Reads a byte at user virtual address UADDR.
   UADDR must be below PHYS_BASE.
   Returns the byte value if successful, -1 if a segfault
   occurred. */
static int
get_user (const uint8_t *uaddr)
{
  int result;
  asm ("movl $1f, %0; movzbl %1, %0; 1:"
       : "=&a" (result) : "m" (*uaddr));
  return result;
}
 
/* Writes BYTE to user address UDST.
   UDST must be below PHYS_BASE.
   Returns true if successful, false if a segfault occurred. */
static bool
put_user (uint8_t *udst, uint8_t byte)
{
  int error_code;
  asm ("movl $1f, %0; movb %b2, %1; 1:"
       : "=&a" (error_code), "=m" (*udst) : "q" (byte));
  return error_code != -1;
}

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  /* Get system call number. */ 
  int sysnum = *(int *)f->esp;

//  printf ("system call!\n");
//  printf ("%d - esp\n", sysnum);

  /* Do syscall here. */
  switch(sysnum) {
	case SYS_HALT:
	  syscall_halt();
	  break;

	case SYS_EXIT:
	  if(!isVargs(f, 1)) syscall_exit(-1);
	  syscall_exit(*(int *)GET_ARGPTR(f, 1));
	  break;

	case SYS_EXEC:
	  if(!isVargs(f, 1)) syscall_exit(-1);
	  f->eax = syscall_exec(*(char **)GET_ARGPTR(f, 1));
	  break;

	case SYS_WAIT:
	  if(!isVargs(f, 1)) syscall_exit(-1);
	  f->eax = syscall_wait(*(pid_t *)GET_ARGPTR(f, 1));
	  break;

	case SYS_FIB:
	  if(!isVargs(f, 1)) syscall_exit(-1);
	  f->eax = syscall_fib(*(int *)GET_ARGPTR(f, 1));
	  break;

	case SYS_SUMFOUR:
	  if(!isVargs(f, 4)) syscall_exit(-1);
	  f->eax = syscall_sumFour(
		  *(int *)GET_ARGPTR(f, 1), *(int *)GET_ARGPTR(f, 2),
		  *(int *)GET_ARGPTR(f, 3), *(int *)GET_ARGPTR(f, 4));
	  break;

	/* When given sysnum is not valid. */
	default: 
	  syscall_exit(-1);
  }
}

/* Proejct 1. */
void
syscall_halt (void)
{
  shutdown_power_off ();
}

void
syscall_exit (int status)
{
  /* Need to implement lock or semaphore. */
  struct thread *cur = thread_current();
  struct thread *parent = cur->parent;
  struct list_elem *e;

  /* Print the process's name and exit code. */
  printf("%s: exit(%d)\n", cur->name, status);

  /* Find current thread in parent's child list,
	 and remove itself. */
  for(e = list_begin(&parent->childList); e != list_end(&parent->childList) 
	  && (list_entry(e, struct thread, childElem)->tid) != cur->tid;
	  e = list_next(e));
  list_remove(e);

  /* Ready for being terminated. */
  sema_up(&parent->wait);

  thread_exit();
}

pid_t
syscall_exec (const char *cmd_line)
{
  return (pid_t) process_execute (cmd_line);
}

int
syscall_wait (pid_t pid)
{
  return process_wait ((tid_t) pid);
}

int
syscall_fib (int n)
{
  int a, b;
  for (b = 1 + (a = 0); n--; b = a - b) a += b;
  return a;
}

int
syscall_sumFour (int a, int b, int c, int d)
{
  return a + b + c + d;
}

/* Checks whether given arguments are valid or not.
   Returns true if given n-arguments are valid,
   false if any of given n-arguments is invalid. */
bool
isVargs (struct intr_frame *f, int n)
{
  void *ptr = NULL;
  while (n--){
	/* 4 for sizeof(uintptr_t). */
	ptr = ((void *)f->esp + 4 * (n + 1));
	if(!is_user_vaddr(ptr)) return false;
  }
  return true;
}

/* Project 2 (maybe). */
bool
syscall_create (const char *file, unsigned initial_size)
{
}

bool
syscall_remove (const char *file)
{
}

int
syscall_open (const char *file)
{
}

int
syscall_filesize (int fd)
{
}

int
syscall_read (int fd, void *buffer, unsigned size)
{
}

int
syscall_write (int fd, const void *buffer, unsigned size)
{
}

void
syscall_seek (int fd, unsigned position)
{
}

unsigned
syscall_tell (int fd)
{
}

void
syscall_close (int fd)
{
}
