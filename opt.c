#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"
#include "sim.h"

extern int debug;

// A linked list implemented by double linklist

int count;
int* dist;
extern struct frame *coremap;
int trace_pos;


/* Page to evict is chosen using the optimal (aka MIN) algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */
int opt_evict() {
	// Check which distance is the furthest and evict that page
	int max_dist = -1;
	int i;
	int frame = -1, counter=-1;
	for (i = 0 ; i< memsize; i++){
	    if (coremap[i].dist == -1){
            return i;
	    }
	    if (coremap[i].dist > max_dist){
	        max_dist = coremap[i].dist;
	        frame = i;
	        counter = coremap[i].counter;
	    }else if (coremap[i].dist == max_dist && coremap[i].counter > counter){
	        max_dist = coremap[i].dist;
	        frame = i;
	        counter = coremap[i].counter;
	    }

	}
	return frame;
}



/* This function is called on each access to a page to update any information
 * needed by the opt algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void opt_ref(pgtbl_entry_t *p) {
    coremap[p->frame >> PAGE_SHIFT].dist = dist[trace_pos];
    coremap[p->frame >> PAGE_SHIFT].counter = trace_pos;
    trace_pos++;
	return;
}

/* Initializes any data structures needed for this
 * replacement algorithm.
 */
void opt_init() {
	//Set initial addr to first index
	trace_pos = 0;
	count = 0;
	FILE *address_file;
	address_file = fopen(tracefile, "r");
	// Make sure file is read correctly
	assert(address_file != NULL);
	char buffer[MAXLINE];
	while (fgets(buffer, MAXLINE, address_file)){
	    count++;
	 }
	rewind(address_file);
	int j,i=0;
	addr_t* current_addr =  malloc(sizeof(addr_t) * count);
	while (fgets(buffer, MAXLINE, address_file)){
	    char nouse;
	    sscanf(buffer, "%c %lx", &nouse, &current_addr[i]);
	    i++;
	}
	//Get distance of next call
    dist = malloc(sizeof(int) * count);
    for(i = 0; i< count; i++){

        int temp_dist = 0;
        for (j = i + 1; j< count; j++){
            temp_dist++;
            if (current_addr[i] == current_addr[j]){
                break;
            }
        }
        if (j == count){
            dist[i] = -1;
        }else {
            dist[i] = temp_dist;
        }
    }
    free(current_addr);
    return;


}
