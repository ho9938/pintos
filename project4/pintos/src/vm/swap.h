#ifndef VM_SWAP_H
#define VM_SWAP_H

#include <stdio.h>
#include "vm/frame.h"

void vm_ss_init (void);

int vm_swap_out (struct frame *frame);
bool vm_swap_in (struct frame *frame);

/*
void vm_swt_init (void);
void vm_swt_insert (struct frame *frame);
struct frame *vm_swt_remove (struct frame *frame);
*/

#endif /* vm/swap.h */
