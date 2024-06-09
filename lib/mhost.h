#pragma once

void MHost_Init(void);
void MHost_Power(int status); // enable +5v to connected device

/*
// for the low-level driver
// B12: PWR_ENABLE: set high to engage power to the connected device
// B13: PWR_nERROR: set pullup. PSU pulls low if error occurs (need IRQ)
    // set PWR_ENABLE low if this occurs (TODO retry?)
// B14: USB_D-
// B15: USB_D+

#define MIDI_IN_PORTS_NUM              0x01
#define MIDI_OUT_PORTS_NUM             0x03

void USB_LP_CAN1_RX0_IRQHandler(void);

extern PCD_HandleTypeDef hpcd_USB_FS;

void MX_USB_DEVICE_Init(void);
*/
