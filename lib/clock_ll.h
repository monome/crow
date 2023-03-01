#pragma once

#include <stdbool.h>

typedef struct clock_node{
    double   wakeup;
    int      coro_id;
    bool     running; // for clock.sleep
    bool     syncing; // for clock.sync (usees beat_wakeup)
    struct clock_node* next;
} clock_node_t;

extern clock_node_t* sleep_head;
extern clock_node_t* sync_head;
extern int sleep_count;
extern int sync_count;

void ll_init(int max_clocks);
void ll_cleanup(void);
clock_node_t* ll_pop(clock_node_t** head);
void ll_insert_idle(clock_node_t* node);
bool ll_insert_event(clock_node_t** head, int coro_id, double seconds_or_beats);
void ll_remove_by_id(int coro_id);




