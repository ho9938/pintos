#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"
#include "vm/page.h"
#include "filesys/filesys.h"
#include <stdio.h>

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);
void push_argument (int, char **argv, void **esp_ptr);
bool process_page_fault (struct page *page);

int process_mmap (struct file *file, void *address);
void process_munmap (int mapping);

void process_swap_out (struct frame *frame);
bool process_swap_in (struct frame *frame);

#endif /* userprog/process.h */
