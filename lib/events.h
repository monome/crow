#pragma once

#include <stm32f7xx.h>

typedef enum {
  E_none,
  E_metro,
  E_eventcount
} event_type_t;

typedef struct {
  event_type_t type;
  int32_t data;
} event_t;

extern void events_init(void);
extern uint8_t event_post(event_t *e);
extern uint8_t event_next(event_t *e);
