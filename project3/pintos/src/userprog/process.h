#ifndef USERPROG_PROCESS_H
#define USERPROG_PROCESS_H

#include "threads/thread.h"
#include "vm/page.h"
#include <stdio.h>

tid_t process_execute (const char *file_name);
int process_wait (tid_t);
void process_exit (void);
void process_activate (void);
void push_argument (int, char **argv, void **esp_ptr);
bool process_page_fault (struct page *page);

#endif /* userprog/process.h */
