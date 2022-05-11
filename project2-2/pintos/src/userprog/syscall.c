#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/vaddr.h"
#include "userprog/pagedir.h"
#include "threads/init.h"
#include "devices/shutdown.h"
#include "filesys/filesys.h"
#include "filesys/file.h"
#include "devices/input.h"
#include "threads/synch.h"
#include "userprog/process.h"

static void syscall_handler (struct intr_frame *);
static void check_address (void *addr);
static int get_arg (struct intr_frame *f, int index);
static struct file *get_file (int fd);
struct lock filesys_lock;

void
syscall_init (void) 
{
  lock_init (&filesys_lock);
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{
	switch (get_arg (f, 0))
	{
		case SYS_HALT:
			halt ();
			break;
		case SYS_EXIT:
			exit (get_arg (f, 1));
			break;
		case SYS_EXEC:
			f->eax = exec ((const char *) get_arg (f, 1));
			break;
		case SYS_WAIT:
			f->eax = wait ((pid_t) get_arg (f, 1));
			break;
		case SYS_CREATE:
			f->eax = create ((const char *) get_arg (f,4), (unsigned) get_arg (f,5));
			break;
		case SYS_REMOVE:
			f->eax = remove ((const char *) get_arg (f, 1));
			break;
		case SYS_OPEN:
			f->eax = open ((const char *) get_arg (f, 1));
			break;
		case SYS_FILESIZE:
			f->eax = filesize (get_arg (f, 1));
			break;
		case SYS_READ:
			f->eax = read (get_arg (f, 5), (void *) get_arg (f, 6), (unsigned) get_arg (f, 7));
			break;
		case SYS_WRITE:
			f->eax = write (get_arg (f, 5), (const void *) get_arg (f, 6), (unsigned) get_arg (f, 7));
			break;
		case SYS_SEEK:
			seek (get_arg (f,4), (unsigned) get_arg (f,5));
			break;
		case SYS_TELL:
			f->eax = tell (get_arg (f, 1));
			break;
		case SYS_CLOSE:
			close (get_arg (f, 1));
			break;
		default:
			exit(-1);
			break;
	}
}

/* Halt the operating system. */
void
halt (void)
{
  shutdown_power_off();	
}

/* Terminate this process. */
void
exit (int status)
{
  /* Process termination message */
  thread_current ()->exit_status = status;
  printf("%s: exit(%d)\n", thread_current()->name, status);
  thread_exit ();
}

/* Start another process. */
pid_t
exec (const char *file)
{
  check_address ((void *) file);
  return process_execute (file);
}

/* Wait for a child process to die. */
int
wait (pid_t pid UNUSED)
{
  return -1;
}

/* Create a file. */
int 
create (const char *file, unsigned initial_size)
{
  check_address ((void *) file);
  return filesys_create (file, initial_size);
}

/* Delete a file. */
int 
remove (const char *file)
{
  check_address ((void *) file);
  return (int) filesys_remove (file);
}

/* Open a file. */
int 
open (const char *file)
{
  check_address ((void *) file);
  
  /* Search for empty fdt index */
  int index = 2;
  while (index < FDT_SIZE && thread_current()->fdt[index] != NULL)
	index++;

  /* No empty index in fdt */
  if (index >= FDT_SIZE)
	  return -1;

  struct file *opened_file;
  opened_file = filesys_open (file);

  /* Open failed */
  if (!opened_file) 
	  return -1;

  thread_current()->fdt[index] = opened_file;
  return index;
}

/* Obtain a file's size. */
int 
filesize (int fd)
{
  struct file *target_file = get_file (fd);
  if (!target_file)
	  return -1;

  return (int) file_length (target_file);
}

/* Read from a file. */
int 
read (int fd, void *buffer, unsigned length)
{
  check_address ((void *) buffer);

  int read_size = 0;
  char key;

  if (fd == 0) { /* STDIN_FILENO */
	  do {
		  key = input_getc ();
	  } while (key != '\0' && ++read_size < (int) length);
  }
  else if (fd == 1) /* STDOUT_FILENO */
	  return -1;
  else {
	  struct file *target_file = get_file (fd);
	  if (!target_file)
		  return -1;

	  lock_acquire (&filesys_lock);
	  read_size = (int) file_read (target_file, buffer, (off_t) length);
	  lock_release (&filesys_lock);
  }

  return read_size;
}

/* Write to a file. */
int 
write (int fd, const void *buffer, unsigned length)
{
  check_address ((void *) buffer);

  int write_size = 0;

  /* STDOUT_FILENO */
  if (fd == 1) {
	  putbuf (buffer, length);
	  write_size = length;
  }
  else if (fd == 0)
	  return -1;
  else {
	  struct file *target_file = get_file (fd);
	  if (!target_file)
		  return -1;

	  lock_acquire (&filesys_lock);
	  write_size = (int) file_write (target_file, buffer, (off_t) length);
	  lock_release (&filesys_lock);
  }

  return write_size;
}

/* Change position in a file. */
void 
seek (int fd, unsigned position)
{
  struct file *target_file = get_file (fd);
  if (target_file == NULL)
	  return;

  file_seek (target_file, (off_t) position);
}

/* Report current position in a file. */
unsigned 
tell (int fd)
{
  struct file *target_file = get_file (fd);
  if (target_file == NULL)
	  return -1;

  return (unsigned) file_tell (target_file);
}

/* Close a file. */
void 
close (int fd)
{
  struct file *target_file = get_file (fd);
  if (target_file == NULL)
	  return;

  file_close (target_file);
  thread_current()->fdt[fd] = NULL;
}

/* Check the validity of address */
static void
check_address (void *addr)
{
  /* Null, not in user space, not allocated */
  if (addr == NULL || !is_user_vaddr (addr) ||
		  pagedir_get_page (thread_current()->pagedir, addr) == NULL)
  {
	  exit(-1);
  }
}

/* Get INDEXth argument from intr_frame */
static int
get_arg (struct intr_frame *f, int index)
{
	int *arg_ptr = (int *)(f->esp) + index;
	check_address ((void *) arg_ptr);
	return *arg_ptr;
}

/* get file from fd, return NULL if failed */
static struct file *
get_file (int fd)
{
  if (fd < 0 || fd >= FDT_SIZE)
	  return NULL;

  return thread_current()->fdt[fd];
}
