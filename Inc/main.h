#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx.h"
#include "stm32f1xx_hal.h"
#include "stm32f1xx_hal_conf.h"
#include "stm32f1xx_hal_tim.h"
#include "stm32f1xx_it.h"
#include "usbd_cdc_if.h"
#include "usbd_def.h"
#include "usbd_desc.h"
#include <stdio.h>
#include <string.h>

#define ENCODER_CH1_Pin        GPIO_PIN_6
#define ENCODER_CH1_GPIO_Port  GPIOA
#define ENCCODER_CH2_Pin       GPIO_PIN_7
#define ENCCODER_CH2_GPIO_Port GPIOA

#ifndef SIM_USB_SEND
#define SIM_USB_SEND 1
#endif

#ifndef ONE_CHANNEL
#define ONE_CHANNEL 0
#endif

#define TODO(...)

typedef enum { Noop = 0, Start, Stop, Restart, ShutDown } DYNO_USBDeviceFlag;
typedef struct {
    uint16_t time;
    uint32_t index;
    uint32_t pulse_enc;
    uint32_t pulse_enc_ext;
    uint32_t pulse_enc_interrupt;
    uint32_t pulse_rpm;
    float temperature;
} DataDyno;

#define SIZE_DATA sizeof(DataDyno) + 1

inline static void Error_Handler(void) {
    __disable_irq();
    while (1) {
    }
}

#ifdef MAIN_IMPL
MAIN_IMPL void DYNO_USB_DEVICE_Init(void);
MAIN_IMPL void SystemClock_Config(void);
MAIN_IMPL void DYNO_GPIO_Init(void);
MAIN_IMPL void DYNO_TIM2_Init(void);
MAIN_IMPL void DYNO_TIM3_Init(void);
#endif

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
