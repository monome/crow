#pragma once

#include "stm32f7xx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../lib/caw.h"

#define USBH_MAX_NUM_ENDPOINTS                2
#define USBH_MAX_NUM_INTERFACES               2
#define USBH_MAX_NUM_CONFIGURATION            1
#define USBH_MAX_NUM_SUPPORTED_CLASS          1
#define USBH_KEEP_CFG_DESCRIPTOR              0
#define USBH_MAX_SIZE_CONFIGURATION           0x200
#define USBH_MAX_DATA_BUFFER                  0x200
#define USBH_DEBUG_LEVEL                      3
#define USBH_USE_OS                           0

/* Memory management macros */
#define USBH_malloc               malloc
#define USBH_free                 free
#define USBH_memset               memset
#define USBH_memcpy               memcpy
    
/* DEBUG macros */   
#if (USBH_DEBUG_LEVEL > 0)
#define USBH_UsrLog(...)   Caw_printf(__VA_ARGS__);\
                           Caw_printf("\n");
#else
#define USBH_UsrLog(...)   
#endif 
                            
                            
#if (USBH_DEBUG_LEVEL > 1)
#define USBH_ErrLog(...)   Caw_printf("ERROR: ") ;\
                           Caw_printf(__VA_ARGS__);\
                           Caw_printf("\n");
#else
#define USBH_ErrLog(...)   
#endif 
                                                      
#if (USBH_DEBUG_LEVEL > 2)                         
#define USBH_DbgLog(...)   Caw_printf("DEBUG : ") ;\
                           Caw_printf(__VA_ARGS__);\
                           Caw_printf("\n");
#else
#define USBH_DbgLog(...)                         
#endif
