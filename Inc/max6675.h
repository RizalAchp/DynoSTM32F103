#pragma once

#ifndef INC_MAX6675_H_
#define INC_MAX6675_H_

#include <stm32f1xx_hal.h>
#include <stm32f1xx_hal_conf.h>
#include <stm32f1xx_hal_spi.h>

// ------------------------- Defines -------------------------
#ifndef SSPORT
#define SSPORT GPIOB // GPIO Port of Chip Select(Slave Select)
#endif
#ifndef SSPIN
#define SSPIN GPIO_PIN_1 // GPIO PIN of Chip Select(Slave Select)
#endif
#ifndef MAX6675_DELC_FUNC
#define MAX6675_DELC_FUNC static
#endif
// ------------------------- Functions  ----------------------
/// get temp in celcius
MAX6675_DELC_FUNC float MAX6675_Temp(void);
MAX6675_DELC_FUNC void MAX6675_Init(void);

void SPI1_IRQHandler(void);

#ifdef MAX6675_IMPLEMENTATION
// ------------------- Variables ----------------
SPI_HandleTypeDef hspi1;

MAX6675_DELC_FUNC _Bool TCF = 0;
MAX6675_DELC_FUNC uint8_t DATARX[2] = {0};
// ------------------- Functions ----------------
MAX6675_DELC_FUNC float MAX6675_Temp(void) {
    float temp;
    // Waits for Chip Ready(according to Datasheet, the max time for
    // conversion is 220ms)
    HAL_GPIO_WritePin(SSPORT, SSPIN, GPIO_PIN_RESET);
    HAL_SPI_Receive(&hspi1, DATARX, 1, 50);
    HAL_GPIO_WritePin(SSPORT, SSPIN, GPIO_PIN_SET);
    // State of Connecting
    TCF = (((DATARX[0] | (DATARX[1] << 8)) >> 2) & 0x0001);
    // Temperature Data Extraction
    temp = ((((DATARX[0] | DATARX[1] << 8))) >> 3);
    temp *= 0.25; // Data to Centigrade Conversation

    return temp;
}

MAX6675_DELC_FUNC void MAX6675_Init(void) {

    GPIO_InitTypeDef GPIO_InitStruct = {0};

    __HAL_RCC_GPIOC_CLK_ENABLE();
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_1, GPIO_PIN_RESET);

    /*Configure GPIO pin : PB1 */
    GPIO_InitStruct.Pin = GPIO_PIN_1;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES_RXONLY;
    hspi1.Init.DataSize = SPI_DATASIZE_16BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_256;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 7;
    // hspi1.Init.CRCLength = SPI_CRC_LENGTH_DATASIZE;
    // hspi1.Init.NSSPMode = SPI_NSS_PULSE_ENABLE;
    if (HAL_SPI_Init(&hspi1) != HAL_OK) {
        Error_Handler();
    }
}

void HAL_SPI_MspInit(SPI_HandleTypeDef *hspi) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    if (hspi->Instance == SPI1) {
        __HAL_RCC_SPI1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();
        GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_6;
        GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
        GPIO_InitStruct.Pull = GPIO_NOPULL;
        GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
        HAL_NVIC_SetPriority(SPI1_IRQn, 0, 0);
        HAL_NVIC_EnableIRQ(SPI1_IRQn);
    }
}

void HAL_SPI_MspDeInit(SPI_HandleTypeDef *hspi) {
    if (hspi->Instance == SPI1) {
        __HAL_RCC_SPI1_CLK_DISABLE();
        HAL_GPIO_DeInit(GPIOA, GPIO_PIN_5 | GPIO_PIN_6);
        HAL_NVIC_DisableIRQ(SPI1_IRQn);
    }
}
void SPI1_IRQHandler(void) { HAL_SPI_IRQHandler(&hspi1); }
#endif

#endif // define INC_MAX6675_H_
