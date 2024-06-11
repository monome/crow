#pragma once

#include "usbd_midi.h"

extern USBD_MIDI_ItfTypeDef USBD_MIDI_fops;

uint8_t USBD_MIDI_GetDeviceState(USBD_HandleTypeDef  *pdev);
uint8_t USBD_MIDI_GetState(USBD_HandleTypeDef  *pdev);
uint8_t USBD_MIDI_SendReport (USBD_HandleTypeDef *pdev, uint8_t *report, uint16_t len);
