#include "clock_ll.h"

#include <stdio.h>
#include <stdlib.h>

// clock_thread as a linked list node

static clock_node_t* clock_pool; // storage for the nodes

// clock_pool elements are moved between these 3 lists
clock_node_t* idle_head = NULL;
clock_node_t* sleep_head = NULL;
clock_node_t* sync_head = NULL;

void ll_init(int max_clocks){
    clock_pool = malloc( sizeof(clock_node_t) * max_clocks );
    if(!clock_pool){
        printf("clock_init failed!\n");
        return;
    }
    for(int i=0; i<max_clocks; i++){ // put all allocated nodes into the idle list
        ll_insert_idle(&clock_pool[i]);
    }
}

void ll_cleanup(void){
    // return all active nodes to the idle list
    while(sleep_head)
        ll_insert_idle(ll_pop(&sleep_head));
    while(sync_head)
        ll_insert_idle(ll_pop(&sync_head));
}

// pop the head of the provided list
clock_node_t* ll_pop(clock_node_t** head){
    if(!*head) return NULL;
    clock_node_t* old_head = *head; // the orphaned head
    *head = old_head->next; // update the head of the provided list
    return old_head; // return the orphaned node
}

void ll_insert_idle(clock_node_t* node){
    // push the provided node onto the idle_head ll
    clock_node_t* old_idle_head = idle_head;
    idle_head = node;
    node->next = old_idle_head;
}

// schedule an event & place it in the provided LL (sync / sleep) in order
bool ll_insert_event(clock_node_t** head, int coro_id, double seconds_or_beats){
    // take a node from the idle list
    clock_node_t* new_node = ll_pop(&idle_head);
    if(!new_node) return false;

    clock_node_t* previous = NULL;
    clock_node_t* compare = *head; // will be NULL for an empty list
    while(compare && seconds_or_beats >= compare->wakeup){
        // node exists & our new_node should come *later*
        previous = compare;
        compare = compare->next;
    }

    // fill in the node
    new_node->next    = compare; // can be = NULL if new_node at end of the list
    new_node->coro_id = coro_id;
    new_node->wakeup  = seconds_or_beats;

    // link from previous to new_node
    if(previous){
        previous->next = new_node;
    } else {
        *head = new_node; // special case for new head of the list
    }
    return true; // success
}

// remove the node referenced by coro_id
void ll_remove_by_id(int coro_id){
    for(int i=0; i<2; i++){
        clock_node_t* previous = NULL;
        clock_node_t** head = i==0 ? &sleep_head : &sync_head; // save for removal
        clock_node_t* compare = *head; // will be NULL for an empty list

        while(compare){
            if(coro_id == compare->coro_id){ // found!
                // remove it!
                if(previous){ // ie, not the head of the list
                    previous->next = compare->next;
                } else { // no prev, therefore this was the head
                    *head = compare->next;
                }
                // push it to the idle list
                ll_insert_idle(compare);
                break; // assume only 1 match can exist
            }
            // node exists but not a match
            previous = compare;
            compare = compare->next;
        }
    }
}
