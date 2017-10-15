#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include <user/syscall.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/init.h"

static void syscall_handler (struct intr_frame *);
void syscall_halt (void);
void syscall_exit (int status);
pid_t syscall_exec (const char *cmd_line);
int syscall_wait (pid_t pid);
bool syscall_create (const char *file, unsigned initial_size);
bool syscall_remove (const char *file);
int syscall_open (const char *file);
int syscall_filesize (int fd);
int syscall_read (int fd, const void *buffer, unsigned size);
void syscall_seek (int fd, unsigned position);
unsigned syscall_tell (int fd);
void syscall_close (int fd);
int syscall_fib (int n);
int syscall_sumFour (int a, int b, int c, int d);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  printf ("system call!\n");
  thread_exit ();
  /* Do syscall here. */
}

void
syscall_halt (void)
{
  shutdown_power_off();
}

void
syscall_exit (int status)
{
}

pid_t syscall_exec (const char *cmd_line);

int syscall_wait (pid_t pid);

bool syscall_create (const char *file, unsigned initial_size);

bool syscall_remove (const char *file);

int syscall_open (const char *file);

int syscall_filesize (int fd);

int syscall_read (int fd, const void *buffer, unsigned size);

void syscall_seek (int fd, unsigned position);

unsigned syscall_tell (int fd);

void syscall_close (int fd);

int
syscall_fib (int n)
{
  int a, b;
  for(b = 1 + (a = 0); n--; b = a - b) a += b;
  return a;
}

int
syscall_sumFour (int a, int b, int c, int d)
{
  return a + b + c + d;
}
