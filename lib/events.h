#pragma once

#include <stm32f7xx.h>

typedef enum {
    E_none,
    E_metro,
    E_stream,
    E_change,
    E_toward,
    E_midi,
    E_COUNT
} event_type_t;

union Data{
    int i;
    float f;
    uint8_t u8s[4];
};

typedef struct {
    event_type_t type;
    int8_t       index;
    union Data   data;
} event_t;


// global array of pointers to handlers
extern void (*app_event_handlers[])(event_t *e);


extern void events_init(void);
extern void events_clear(void);
extern uint8_t event_post(event_t *e);
extern uint8_t event_next(event_t *e);

static void handler_none(event_t *e);
static void handler_metro(event_t *e);
static void handler_stream(event_t *e);
static void handler_change(event_t *e);
static void handler_toward(event_t *e);
static void handler_midi(event_t *e);
