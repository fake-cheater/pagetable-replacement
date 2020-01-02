#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

int current_frame;
/* Page to evict is chosen using the clock algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */

int clock_evict() {
	// Check which page was last referenced in clock and evict page
	while (coremap[current_frame].ref_bit == 1){
		coremap[current_frame].ref_bit = 0;
		current_frame = (current_frame + 1 == memsize) ? 0 : current_frame + 1;
	}
	int evict = current_frame;
	current_frame = (current_frame + 1 == memsize) ? 0 : current_frame + 1;
	return evict;
}

/* This function is called on each access to a page to update any information
 * needed by the clock algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void clock_ref(pgtbl_entry_t *p) {
	// Set ref bit to 1
	coremap[p->frame >> PAGE_SHIFT].ref_bit=1;
	return;
}

/* Initialize any data structures needed for this replacement
 * algorithm.
 */
void clock_init() {
	current_frame = 0; // Initialize to 0 (first address location)
}
