#include <stdio.h>
#include <assert.h>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include "pagetable.h"


extern int memsize;

extern int debug;

extern struct frame *coremap;

// A node inside queue
typedef struct Node{
	struct Node *prev, *next;
	int page_frame;
} Node;

// A queue implemented by double linklist
typedef struct Queue{
	int count;
	struct Node *front, *end;
}Queue;

Queue* queue; // double linklist queue
Node** nodeArray; // Array of node pointers


/* Page to evict is chosen using the accurate LRU algorithm.
 * Returns the page frame number (which is also the index in the coremap)
 * for the page that is to be evicted.
 */

int lru_evict() {

	//Init all neccessary info
	Node* newFront = queue->front->next;
	Node* oldFront = queue->front;
	int oldFrontFrame;
	if (oldFront != NULL){
	    oldFrontFrame = oldFront->page_frame;
    }

	// If queue is empty, nothing we can do about it
	if (queue->count == 0){

		return -1;

	}

	// Queue have only 1 nodes
	if (queue->count == 1){

		queue->front = queue->end = NULL;

	//otherwise
	}else {

		newFront->prev = NULL;
		queue->front = newFront;
	}

	// Update queue and nodeArray and free that evicted node
	queue->count--;
	nodeArray[oldFront->page_frame] = NULL;
	free(oldFront);

	return oldFrontFrame;
}

/* This function is called on each access to a page to update any information
 * needed by the lru algorithm.
 * Input: The page table entry for the page that is being accessed.
 */
void lru_ref(pgtbl_entry_t *p) {

	Node* newNode = malloc(sizeof(Node));
	newNode->page_frame = p->frame>>PAGE_SHIFT;

	// get page frame from p

	// If queue is empty, just map it in array and enqueue it.
	if (queue->count == 0){

		nodeArray[newNode->page_frame] = newNode;
		queue->front=queue->end= newNode;
		queue->count++;

	// It is a new element in the page, enqueue it and map it in array
	} else if (nodeArray[newNode->page_frame] == NULL){

		nodeArray[newNode->page_frame] = newNode;
		queue->end->next = newNode;
		newNode->prev = queue->end;
		queue->end = newNode;
		queue->count++;

	//  If page is hit and already present in the array
	} else {

		Node* refNode = nodeArray[newNode->page_frame]; // This is the actual node
		free(newNode); // Free it because we already had a node.

		// Queue only has 1 node or at end queue, do nothing
		if (refNode == queue->end ||  queue->count == 1){

			;

		// Hit node is in the front queue, move it to end of queue
		} else if (refNode == queue->front){

			Node* newfront = refNode->next;
			refNode->next = NULL;
			queue->front = newfront;
			queue->end->next = refNode;
			refNode->prev = queue->end;
			queue->end = refNode;


		// hit element, move it the end of queue and update its neighbors information
		} else{

			Node* prevRefNode = refNode->prev;
			Node* nextRefNode = refNode->next;
			prevRefNode->next = nextRefNode;
			nextRefNode->prev = prevRefNode;
			refNode->next = NULL;
			refNode->prev = queue->end;
			queue->end->next = refNode;
			queue->end = refNode;

		}

	}

	return;
}


/* Initialize any data structures needed for this
 * replacement algorithm
 */
void lru_init() {

	nodeArray = malloc(sizeof(Node *) * memsize);
	queue = malloc (sizeof (Queue *));
	queue->front = NULL;
	queue->end = NULL;
	queue->count = 0;
	int i;
	for (i = 0; i< memsize;i++){
		nodeArray[i] = NULL;
	}
}
