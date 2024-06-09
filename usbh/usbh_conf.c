#include "stm32f7xx_hal.h"
#include "usbh_core.h"
// #include "stm32f769i_eval_io.h"

HCD_HandleTypeDef hhcd;

void HAL_HCD_MspInit(HCD_HandleTypeDef* hhcd){
    __HAL_RCC_GPIOB_CLK_ENABLE();

    GPIO_InitTypeDef g;
    // power enable pin
    g.Pin = GPIO_PIN_12;
    g.Mode  = GPIO_MODE_OUTPUT_PP;
    g.Pull  = GPIO_NOPULL;
    g.Speed = GPIO_SPEED_HIGH;
    HAL_GPIO_Init(GPIOB, &g);

    // d+/d- signal pins
    g.Pin = GPIO_PIN_14 | GPIO_PIN_15;
    g.Mode = GPIO_MODE_AF_PP;
    g.Alternate = GPIO_AF12_OTG_HS_FS;
    HAL_GPIO_Init(GPIOB, &g);

    // TODO B13 is over-current-sense for usb switch

    __HAL_RCC_USB_OTG_HS_CLK_ENABLE();
    HAL_NVIC_SetPriority(OTG_HS_IRQn, 6, 0);
    HAL_NVIC_EnableIRQ(OTG_HS_IRQn);
}

void HAL_HCD_MspDeInit(HCD_HandleTypeDef* hhcd){
    __HAL_RCC_USB_OTG_HS_CLK_DISABLE();
    __HAL_RCC_USB_OTG_HS_ULPI_CLK_DISABLE();
}

///////////////////////////////////////////////////////////
// LL Driver Callbacks (HCD -> USB Host Library)

void HAL_HCD_SOF_Callback(HCD_HandleTypeDef *hhcd){
    USBH_LL_IncTimer (hhcd->pData);
}

void HAL_HCD_Connect_Callback(HCD_HandleTypeDef *hhcd){
    USBH_LL_Connect(hhcd->pData);
}

void HAL_HCD_Disconnect_Callback(HCD_HandleTypeDef *hhcd){
    USBH_LL_Disconnect(hhcd->pData);
} 

void HAL_HCD_PortEnabled_Callback(HCD_HandleTypeDef *hhcd){
    USBH_LL_PortEnabled(hhcd->pData);
}

void HAL_HCD_PortDisabled_Callback(HCD_HandleTypeDef *hhcd){
    USBH_LL_PortDisabled(hhcd->pData);
}

void HAL_HCD_HC_NotifyURBChange_Callback(HCD_HandleTypeDef *hhcd, uint8_t chnum, HCD_URBStateTypeDef urb_state){
    // USBH_LL_NotifyURBChange(hhcd->pData);
}


///////////////////////////////////////////////////////////
// LL Driver Interface (USB Host Library --> HCD)
USBH_StatusTypeDef USBH_LL_Init(USBH_HandleTypeDef* phost){
    hhcd.Instance = USB_OTG_HS;
    hhcd.Init.Host_channels = 11; 
    hhcd.Init.dma_enable = 0;
    hhcd.Init.low_power_enable = 0;
    hhcd.Init.phy_itface = HCD_PHY_EMBEDDED; 
    hhcd.Init.Sof_enable = 0;
    hhcd.Init.speed = HCD_SPEED_HIGH;
    hhcd.Init.vbus_sensing_enable = 0;
    hhcd.Init.use_external_vbus = 1;
    hhcd.Init.lpm_enable = 0;

    // Link the driver to the stack
    hhcd.pData = phost;
    phost->pData = &hhcd;

    // Initialize the LL driver
    HAL_HCD_Init(&hhcd);

    USBH_LL_SetTimer(phost, HAL_HCD_GetCurrentFrame(&hhcd));

    return USBH_OK;
}

USBH_StatusTypeDef USBH_LL_DeInit(USBH_HandleTypeDef *phost){
    HAL_HCD_DeInit(phost->pData);
    return USBH_OK; 
}

USBH_StatusTypeDef USBH_LL_Start(USBH_HandleTypeDef *phost){
    HAL_HCD_Start(phost->pData);
    return USBH_OK; 
}

USBH_StatusTypeDef USBH_LL_Stop(USBH_HandleTypeDef *phost){
    HAL_HCD_Stop(phost->pData);
    return USBH_OK; 
}

USBH_SpeedTypeDef USBH_LL_GetSpeed(USBH_HandleTypeDef *phost){
    USBH_SpeedTypeDef speed = USBH_SPEED_FULL;

    switch (HAL_HCD_GetCurrentSpeed(phost->pData)){
    case 0: speed = USBH_SPEED_HIGH; break;
    case 1: speed = USBH_SPEED_FULL; break;
    case 2: speed = USBH_SPEED_LOW; break;
    default: speed = USBH_SPEED_FULL; break;
    }
    return speed;
}

USBH_StatusTypeDef USBH_LL_ResetPort (USBH_HandleTypeDef *phost){
    HAL_HCD_ResetPort(phost->pData);
    return USBH_OK; 
}

uint32_t USBH_LL_GetLastXferSize(USBH_HandleTypeDef *phost, uint8_t pipe){
    return HAL_HCD_HC_GetXferCount(phost->pData, pipe);
}

/**
  * @brief  Opens a pipe of the Low Level Driver.
  * @param  phost: Host handle
  * @param  pipe: Pipe index
  * @param  epnum: Endpoint Number
  * @param  dev_address: Device USB address
  * @param  speed: Device Speed 
  * @param  ep_type: Endpoint Type
  * @param  mps: Endpoint Max Packet Size
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_LL_OpenPipe(USBH_HandleTypeDef *phost, 
                                    uint8_t pipe,
                                    uint8_t epnum,
                                    uint8_t dev_address,
                                    uint8_t speed,
                                    uint8_t ep_type,
                                    uint16_t mps){
    HAL_HCD_HC_Init(phost->pData,
                    pipe,
                    epnum,
                    dev_address,
                    speed,
                    ep_type,
                    mps);
    return USBH_OK; 
}

/**
  * @brief  Closes a pipe of the Low Level Driver.
  * @param  phost: Host handle
  * @param  pipe: Pipe index
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_LL_ClosePipe(USBH_HandleTypeDef *phost, uint8_t pipe)
{
    HAL_HCD_HC_Halt(phost->pData, pipe);
    return USBH_OK;
}

/**
  * @brief  Submits a new URB to the low level driver.
  * @param  phost: Host handle
  * @param  pipe: Pipe index    
  *          This parameter can be a value from 1 to 15
  * @param  direction: Channel number
  *          This parameter can be one of these values:
  *           0: Output 
  *           1: Input
  * @param  ep_type: Endpoint Type
  *          This parameter can be one of these values:
  *            @arg EP_TYPE_CTRL: Control type
  *            @arg EP_TYPE_ISOC: Isochronous type
  *            @arg EP_TYPE_BULK: Bulk type
  *            @arg EP_TYPE_INTR: Interrupt type
  * @param  token: Endpoint Type
  *          This parameter can be one of these values:
  *            @arg 0: PID_SETUP
  *            @arg 1: PID_DATA
  * @param  pbuff: pointer to URB data
  * @param  length: length of URB data
  * @param  do_ping: activate do ping protocol (for high speed only)
  *          This parameter can be one of these values:
  *           0: do ping inactive 
  *           1: do ping active 
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_LL_SubmitURB(USBH_HandleTypeDef *phost, 
                                     uint8_t pipe, 
                                     uint8_t direction,
                                     uint8_t ep_type,  
                                     uint8_t token, 
                                     uint8_t* pbuff, 
                                     uint16_t length,
                                     uint8_t do_ping){
    HAL_HCD_HC_SubmitRequest(phost->pData,
                             pipe, 
                             direction,
                             ep_type,  
                             token, 
                             pbuff, 
                             length,
                             do_ping);
    return USBH_OK;   
}

/**
  * @brief  Gets a URB state from the low level driver.
  * @param  phost: Host handle
  * @param  pipe: Pipe index
  *          This parameter can be a value from 1 to 15
  * @retval URB state
  *          This parameter can be one of these values:
  *            @arg URB_IDLE
  *            @arg URB_DONE
  *            @arg URB_NOTREADY
  *            @arg URB_NYET 
  *            @arg URB_ERROR  
  *            @arg URB_STALL      
  */
USBH_URBStateTypeDef USBH_LL_GetURBState(USBH_HandleTypeDef *phost, uint8_t pipe){
    return (USBH_URBStateTypeDef)HAL_HCD_HC_GetURBState (phost->pData, pipe);
}

/**
  * @brief  Drives VBUS.
  * @param  phost: Host handle
  * @param  state: VBUS state
  *          This parameter can be one of these values:
  *           0: VBUS Active 
  *           1: VBUS Inactive
  * @retval USBH Status
  */
USBH_StatusTypeDef USBH_LL_DriverVBUS(USBH_HandleTypeDef *phost, uint8_t state){
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, !!state);
    HAL_Delay(200);
    return USBH_OK;  
}

USBH_StatusTypeDef USBH_LL_SetToggle(USBH_HandleTypeDef *phost, uint8_t pipe, uint8_t toggle){
    if(hhcd.hc[pipe].ep_is_in){
        hhcd.hc[pipe].toggle_in = toggle;
    }else{
        hhcd.hc[pipe].toggle_out = toggle;
    }
    return USBH_OK; 
}

uint8_t USBH_LL_GetToggle(USBH_HandleTypeDef *phost, uint8_t pipe){
    uint8_t toggle = 0;

    if(hhcd.hc[pipe].ep_is_in){
        toggle = hhcd.hc[pipe].toggle_in;
    }else{
        toggle = hhcd.hc[pipe].toggle_out;
    }
return toggle; 
}

void USBH_Delay(uint32_t Delay){
    HAL_Delay(Delay);  
}
