#define MAIN_IMPL inline static
#include <main.h>

static uint8_t BUFFER[SIZE_DATA] = {'|'};
static DataDyno dyno = {0};

USBD_HandleTypeDef DYNO_USBDeviceFS;
USBD_CDC_LineCodingTypeDef DYNO_USBDeviceLC = {
    512000, /* baud rate*/
    0x00,   /* stop bits-1*/
    0x00,   /* parity - none*/
    0x08    /* nb. of bits 8*/
};
TIM_HandleTypeDef htim2;
volatile int DYNO_UsbCmd = 0;
volatile uint32_t pulse_enc_ext = 0;
float temp_ext;

#if !SIM_USB_SEND
#define MAX6675_IMPLEMENTATION
#include "max6675.h"
#endif

/**
 * @brief  The application entry point.
 * @retval int
 */
int main(void) {
    HAL_Init();
    SystemClock_Config();

#if !SIM_USB_SEND
    DYNO_GPIO_Init();
    DYNO_TIM2_Init();
    DYNO_TIM3_Init();
#endif
    DYNO_USB_DEVICE_Init();

    while (1) {
#if SIM_USB_SEND
        {
            uint32_t tick = HAL_GetTick();
            dyno.pulse_enc = tick;
            dyno.pulse_rpm = tick;
            dyno.temperature = (float)tick;

            memcpy(BUFFER, &dyno, SIZE_DATA);
            BUFFER[SIZE_DATA - 1] = '\n';
            CDC_Transmit_FS(BUFFER, SIZE_DATA);
            HAL_Delay(50);
        }
#endif
        switch (DYNO_UsbCmd) {
        case Start: {
            TODO(" Implementing Start Command from Serial "
                 "Communication")
        } break;
        case Stop: {
            TODO(" Implementing Stop Command from Serial "
                 "Communication")
        } break;
        case Restart: {
            TODO(" Implementing Restart Command from Serial "
                 "Communication")
        } break;
        case ShutDown: {
            TODO(" Implementing Shutdown Command from Serial "
                 "Communication")
        } break;
        default:
            break;
        }
    }
}

/**
 * @brief System Clock Configuration
 * @retval None
 */
MAIN_IMPL void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
    RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

    /** Initializes the RCC Oscillators according to the specified
     * parameters in the RCC_OscInitTypeDef structure.
     */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    /** Initializes the CPU, AHB and APB buses clocks
     */
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK |
                                  RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USB;
    PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
    if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK) {
        Error_Handler();
    }
}

/**
 * @brief NVIC Configuration.
 * @retval None
 */

MAIN_IMPL void DYNO_USB_DEVICE_Init(void) {
    if (USBD_Init(&DYNO_USBDeviceFS, &FS_Desc, DEVICE_FS) != USBD_OK) {
        Error_Handler();
    }
    if (USBD_RegisterClass(&DYNO_USBDeviceFS, &USBD_CDC) != USBD_OK) {
        Error_Handler();
    }
    if (USBD_CDC_RegisterInterface(&DYNO_USBDeviceFS,
                                   &USBD_Interface_fops_FS) != USBD_OK) {
        Error_Handler();
    }
    if (USBD_Start(&DYNO_USBDeviceFS) != USBD_OK) {
        Error_Handler();
    }

    HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(USB_LP_CAN1_RX0_IRQn);
}

MAIN_IMPL void DYNO_GPIO_Init(void) {
    __HAL_RCC_GPIOD_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
}

MAIN_IMPL void DYNO_TIM2_Init(void) {
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 4499; // (F_CPU / 1000 /*desired freq*/) - 1;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 3999; // (1000 / 5 /*interrupt freq (Hz)*/) - 1;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    if (HAL_TIM_Base_Init(&htim2) != HAL_OK) {
        Error_Handler();
    }

    HAL_NVIC_SetPriority(TIM2_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);
    if (HAL_TIM_Base_Start_IT(&htim2) != HAL_OK) {
        Error_Handler();
    }
}

/**
 *  this function sets up TIM3 to count up from 0 to its maximum value
 *  and generate an interrupt when it overflows. The timer is configured
 *  to start counting when a trigger event occurs on its encoder input channels.
 */
MAIN_IMPL void DYNO_TIM3_Init(void) {
    __HAL_RCC_TIM3_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    // `0xFFFF` == MAX of uint16_t (2^16 or 65535)
    TIM3->ARR = 0xFFFF;

#if ONE_CHANNEL
    /// ==================================================================== ///
    /// USING JUST 1 CHANNEL

    // Configure channel 1 for input capture
    TIM3->CCMR1 |= TIM_CCMR1_CC1S_0;
    // Configure channel 1 to capture on rising edge
    TIM3->CCER &= ~(TIM_CCER_CC1P);
    // Enable update interrupt, triggered when the timer overflows.
    TIM3->DIER |= TIM_DIER_UIE;
    // Configure timer in input capture mode
    TIM3->SMCR |= TIM_SMCR_SMS_0;
    // Start the timer
    TIM3->CR1 |= TIM_CR1_CEN;

#else
    /// ==================================================================== ///
    /// USING BOTH CHANNEL (1 and 2)

    // Configure channel 1 and 2 for input capture
    TIM3->CCMR1 |= (TIM_CCMR1_CC1S_0 | TIM_CCMR1_CC2S_0); // step 1 and 2
    // disables the polarity inversion for capture/compare channel 1 and and 2.
    TIM3->CCER &= ~(TIM_CCER_CC1P | TIM_CCER_CC2P); // step 3 and 4
    // Enable update interrupt, triggered when the timer overflows.
    TIM3->DIER |= TIM_DIER_UIE;
    // Configure timer in input capture mode
    TIM3->SMCR |= TIM_SMCR_SMS_0 | TIM_SMCR_SMS_1; // step 5
    // Start the timer
    TIM3->CR1 |= TIM_CR1_CEN; // step 6
#endif

    /// This enables the interrupt for TIM3 in the Nested Vector Interrupt
    /// Controller (NVIC), which allows the interrupt to be serviced when it
    /// occurs.
    NVIC_SetPriority(TIM3_IRQn, 0);
    NVIC_EnableIRQ(TIM3_IRQn);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM2) {
        #if !SIM_USB_SEND
        dyno.temperature = MAX6675_Temp();
        dyno.pulse_enc = (uint32_t)(pulse_enc_ext + TIM3->CNT);
        dyno.pulse_enc *= 4;

        // TODO! IMPLEMENTING RPM COUNTER VAR
        dyno.pulse_rpm = (uint32_t)(pulse_enc_ext);

        memcpy(BUFFER, &dyno, SIZE_DATA);
        BUFFER[SIZE_DATA - 1] = '|';
        CDC_Transmit_FS(BUFFER, SIZE_DATA);

        // reset
        pulse_enc_ext = 0;
        TIM3->CNT = 0;
        #endif
    }
}
void TIM3_IRQHandler(void) {
    pulse_enc_ext += 0xFFFF;
    HAL_TIM_IRQHandler(&htim2);
}
void TIM2_IRQHandler(void) { HAL_TIM_IRQHandler(&htim2); }

void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *tim_baseHandle) {
    if (tim_baseHandle->Instance == TIM2) {
        __HAL_RCC_TIM2_CLK_ENABLE();
    }
}

void HAL_TIM_Base_MspDeInit(TIM_HandleTypeDef *tim_baseHandle) {
    if (tim_baseHandle->Instance == TIM2) {
        __HAL_RCC_TIM2_CLK_DISABLE();
        HAL_NVIC_DisableIRQ(TIM2_IRQn);
    }
}

void HAL_MspInit(void) {
    __HAL_RCC_SYSCFG_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
}

void NMI_Handler(void) {
    while (1) {
    }
}

void HardFault_Handler(void) {
    while (1) {
    }
}

void MemManage_Handler(void) {
    while (1) {
    }
}

void BusFault_Handler(void) {
    while (1) {
    }
}

void UsageFault_Handler(void) {
    while (1) {
    }
}
void SVC_Handler(void) {}
void DebugMon_Handler(void) {}
void PendSV_Handler(void) {}
void SysTick_Handler(void) { HAL_IncTick(); }
void USB_LP_CAN1_RX0_IRQHandler(void) { HAL_PCD_IRQHandler(&hpcd_USB_FS); }

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) {}
#endif /* USE_FULL_ASSERT */
