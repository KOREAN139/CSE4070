#include "userprog/process.h"
#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include <user/syscall.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/init.h"
#include "threads/malloc.h"
#include "threads/synch.h"
#include "threads/vaddr.h"
#include "filesys/filesys.h"
#include "filesys/file.h"

static void syscall_handler (struct intr_frame *);

/* Project 1(+ read, write for stdbuff). */
void syscall_halt (void);
void syscall_exit (int status);
pid_t syscall_exec (const char *cmd_line);
int syscall_wait (pid_t pid);
int syscall_fib (int n);
int syscall_sumFour (int a, int b, int c, int d);
bool isVargs (struct intr_frame *f, int n);
uint32_t readWord (const void *ptr);

/* Project 2. */
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
  int sysnum = (int)readWord((const void *)f->esp);

  /* Save esp into thread on initial transition
	 from user to kernel. (pintos document 4.3.3) */
  thread_current()->esp = f->esp;

  /* Do syscall here. */
  switch(sysnum) {
	/* Project 1. */
	case SYS_HALT:
	  syscall_halt();
	  break;

	case SYS_EXIT:
	  syscall_exit((int)readWord((const void *)(f->esp + 4)));
	  break;

	case SYS_EXEC:
	  f->eax = syscall_exec((char *)readWord((const void *)(f->esp + 4)));
	  break;

	case SYS_WAIT:
	  f->eax = syscall_wait((pid_t)readWord((const void *)(f->esp + 4)));
	  break;

	case SYS_FIB:
	  f->eax = syscall_fib((int)readWord((const void *)(f->esp + 4)));
	  break;

	case SYS_SUMFOUR:
	  f->eax = syscall_sumFour(
		  (int)readWord((const void *)(f->esp + 4)),
		  (int)readWord((const void *)(f->esp + 8)),
		  (int)readWord((const void *)(f->esp + 12)), 
		  (int)readWord((const void *)(f->esp + 16))
		  );
	  break;

	case SYS_READ:
      f->eax = syscall_read(
		  (int)readWord((const void *)(f->esp + 4)),
		  (void *)readWord((const void *)(f->esp + 8)),
		  (unsigned)readWord((const void *)(f->esp + 12))
		  );
	  break;
	
	case SYS_WRITE:
      f->eax = syscall_write(
		  (int)readWord((const void *)(f->esp + 4)),
		  (const void *)readWord((const void *)(f->esp + 8)),
		  (unsigned)readWord((const void *)(f->esp + 12))
		  );
	  break;

	/* Project 2. */
	case SYS_CREATE:
	  f->eax = syscall_create(
		  (char *)readWord((const void *)(f->esp + 4)),
		  (unsigned)readWord((const void *)(f->esp + 8))
		  );
	  break;

	case SYS_REMOVE:
	  f->eax = syscall_remove((char *)readWord((const void*)(f->esp + 4)));
	  break;

	case SYS_OPEN:
	  f->eax = syscall_open(
		  (char *)readWord((const void*)(f->esp + 4))
		  );
	  break;

	case SYS_FILESIZE:
	  f->eax = syscall_filesize(
		  (int)readWord((const void *)(f->esp + 4))
		  );
	  break;

	case SYS_SEEK:
	  syscall_seek(
		  (int)readWord((const void *)(f->esp + 4)),
		  (unsigned)readWord((const void *)(f->esp + 8))
		  );
	  break;

	case SYS_TELL:
	  f->eax = syscall_tell(
		  (int)readWord((const void *)(f->esp + 4))
		  );
	  break;

	case SYS_CLOSE:
	  syscall_close((int)readWord((const void *)(f->esp + 4)));
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
  struct thread *cur = thread_current();
  struct thread *parent = cur->parent;
  struct list_elem *e;

  parent->exit_status = status;

  /* Print the process's name and exit code. */
  printf("%s: exit(%d)\n", cur->name, status);

  /* Find current thread in parent's child list,
	 and remove itself. */
  for(e = list_begin(&parent->childList); e != list_end(&parent->childList) 
	  && (list_entry(e, struct thread, childElem)->tid) != cur->tid;
	  e = list_next(e));
  list_remove(e);

  /* Clean up file descriptor table, and deallocates it. */
  while(--cur->fd > 1) file_close(cur->fdTable[cur->fd]);
  free(cur->fdTable);

  file_close (cur->curFile);

  /* Ready for being terminated. */
  sema_up(&parent->wait);

  thread_exit();
}

pid_t
syscall_exec (const char *cmd_line)
{
  readWord((const void *)cmd_line);
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

/* Read one word (4byte) from given ptr.
   Checks that ptr is below PHYS_BASE,
   and read a word using get_user.
   If given ptr is not on below PHYS_BASE or 
   segfalut occured while reading word, call syscall_exit(-1). */
uint32_t
readWord (const void *ptr)
{
  int i, t, ret = 0;
  if(!is_user_vaddr(ptr)) syscall_exit(-1);
  for(i = 0; i < 4; i++){
	if((t = get_user((const uint8_t *)(ptr + i))) == -1) syscall_exit(-1);
	ret |= (t << (8 * i));
  }
  return ret;
}

/* Project 2. */
bool
syscall_create (const char *file, unsigned initial_size)
{
  /* Check for bad-ptr. */
  readWord((const void *)file);
  return filesys_create(file, initial_size);
}

bool
syscall_remove (const char *file)
{
  /* Check for bad-ptr. */
  readWord((const void *)file);
  // if(!*file) syscall_exit(-1);
  return filesys_remove(file);
}

int
syscall_open (const char *file)
{
  /* Check for bad-ptr. */
  readWord((const void *)file);

  struct thread *cur;

  /* Open file with given name. */
  lock_acquire(&fLock);
  struct file *f = filesys_open(file);
  lock_release(&fLock);

  /* When such file doesn't exist. */
  if( !f ) return -1;

  /* Write on file descriptor table and returns that fd.
	 (postfix for fd to store file descriptor for next file) */
  cur = thread_current();
  cur->fdTable[cur->fd] = f;
  return cur->fd++;
}

int
syscall_filesize (int fd)
{
  struct thread *cur = thread_current();
  /* Check for bad file descriptor. */
  if(fd < 2 || cur->fd <= fd) syscall_exit(-1);

  return file_length(cur->fdTable[fd]);
}

int
syscall_read (int fd, void *buffer, unsigned size)
{
  /* Check for bad-ptr. */
  readWord(buffer);
  
  /* For project 1, stdin. */
  if(fd == 0){
	int cnt = 0;
	uint8_t c;
	while(cnt < size && (c = input_getc()) != '\n'){
	  /* If segfault occurs, call syscall_exit(). */
	  if(!put_user((const uint8_t *)(buffer + cnt++), c)) syscall_exit(-1);
	}
	return cnt;
  } 
  /* For project 2, read from files. */
  else{
	int cnt;
	struct thread *cur = thread_current();
	/* Check for bad file descriptor. */
	if(fd < 2 || cur->fd <= fd || !(cur->fdTable[fd])) return -1;

	lock_acquire(&fLock);
	cnt = (int)file_read(cur->fdTable[fd], buffer, size);
	lock_release(&fLock);
	return cnt;
  }
}

int
syscall_write (int fd, const void *buffer, unsigned size)
{
  /* Check for bad-ptr. */
  readWord(buffer);
  
  /* For project 1, stdout. */
  if(fd == 1){
    putbuf((const char *)buffer, (size_t)size);
	return size;
  }
  /* For project 2, write to files. */
  else{
	int cnt;
	struct thread *cur = thread_current();
	/* Check for bad file descriptor. */
	if(fd < 2 || cur->fd <= fd || !(cur->fdTable[fd])) return -1;

	lock_acquire(&fLock);
	cnt = (int)file_write(cur->fdTable[fd], buffer, size);
	lock_release(&fLock);
	return cnt;
  }
}

void
syscall_seek (int fd, unsigned position)
{
  struct thread *cur = thread_current();
  /* Check for bad file descriptor. */
  if(fd < 2 || cur->fd <= fd) return;

  file_seek(cur->fdTable[fd], position);
}

unsigned
syscall_tell (int fd)
{
  struct thread *cur = thread_current();
  /* Check for bad file descriptor. */
  if(fd < 2 || cur->fd <= fd) return -1;

  return (unsigned)file_tell(cur->fdTable[fd]);
}

void
syscall_close (int fd)
{
  struct thread *cur = thread_current();
  /* Check for bad file descriptor. */
  if(fd < 2 || cur->fd <= fd) syscall_exit(-1);

  file_close(cur->fdTable[fd]);
  cur->fdTable[fd] = NULL;
}
