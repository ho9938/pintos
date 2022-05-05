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

static void syscall_handler (struct intr_frame *);
static void check_address (void *addr);
static int get_arg (struct intr_frame *f, int index);

void
syscall_init (void) 
{
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
  printf("%s: exit(%d)\n", thread_current()->name, status);
  thread_exit ();
}

/* Start another process. */
pid_t
exec (const char *file)
{
  check_address ((void *) file);
  return -1;
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
  return 0;
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
filesize (int fd UNUSED)
{
  return -1;
}

/* Read from a file. */
int 
read (int fd UNUSED, void *buffer UNUSED, unsigned length UNUSED)
{
  return -1;
}

/* Write to a file. */
int 
write (int fd, const void *buffer, unsigned length)
{
  check_address ((void *) buffer);

  /* STDOUT_FILENO */
  if (fd == 1)
	  putbuf (buffer, length);

  return length;
}

/* Change position in a file. */
void 
seek (int fd UNUSED, unsigned position UNUSED)
{
  return;
}

/* Report current position in a file. */
unsigned 
tell (int fd UNUSED)
{
  return -1;
}

/* Close a file. */
void 
close (int fd)
{
  /* Invalid fd */
  if (fd < 0 || fd >= FDT_SIZE)
	  return;

  struct file *file_to_close;
  file_to_close = thread_current()->fdt[fd];

  /* Already closed file */
  if (file_to_close == NULL)
	  return;

  file_close (file_to_close);
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
