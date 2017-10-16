#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include <user/syscall.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/init.h"
#include "filesys/filesys.h"

static void syscall_handler (struct intr_frame *);

/* Project 1. */
void syscall_halt (void);
void syscall_exit (int status);
pid_t syscall_exec (const char *cmd_line);
int syscall_wait (pid_t pid);
int syscall_fib (int n);
int syscall_sumFour (int a, int b, int c, int d);
int isVargs (struct intr_frame *f, int n);

/* Project 2 (maybe). */
bool syscall_create (const char *file, unsigned initial_size);
bool syscall_remove (const char *file);
int syscall_open (const char *file);
int syscall_filesize (int fd);
int syscall_read (int fd, const void *buffer, unsigned size);
void syscall_seek (int fd, unsigned position);
unsigned syscall_tell (int fd);
void syscall_tell (int fd);

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

  printf ("system call!\n");
  printf ("%d - esp\n", sysnum);

  thread_exit ();
  /* Do syscall here. */
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
  thread_exit();
  return;
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
  bool valid = true;
  void *ptr = NULL;
  while (n--){
	ptr = *((void *)f->esp + 4*n);
	if(!is_user_vaddr(ptr)) valid = false;
  }
  return valid;
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
syscall_read (int fd, const void *buffer, unsigned size)
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
