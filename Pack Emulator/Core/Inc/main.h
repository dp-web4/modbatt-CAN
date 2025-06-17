/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f4xx_hal.h"
#include "time.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */
void getTime(void);
void serialOut(char* message);
/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define USER_BUTTON_Pin GPIO_PIN_0
#define USER_BUTTON_GPIO_Port GPIOA
#define USER_BUTTON_EXTI_IRQn EXTI0_IRQn
#define CAN_CS_Pin GPIO_PIN_8
#define CAN_CS_GPIO_Port GPIOD
#define CAN_INT_Pin GPIO_PIN_9
#define CAN_INT_GPIO_Port GPIOD
#define CAN_INT_EXTI_IRQn EXTI9_5_IRQn
#define CAN_INT1_Pin GPIO_PIN_10
#define CAN_INT1_GPIO_Port GPIOD
#define CAN_INT1_EXTI_IRQn EXTI15_10_IRQn
#define CAN_INT0_Pin GPIO_PIN_11
#define CAN_INT0_GPIO_Port GPIOD
#define CAN_INT0_EXTI_IRQn EXTI15_10_IRQn
#define LED4_GREEN_Pin GPIO_PIN_12
#define LED4_GREEN_GPIO_Port GPIOD
#define LED3_ORANGE_Pin GPIO_PIN_13
#define LED3_ORANGE_GPIO_Port GPIOD
#define LED5_RED_Pin GPIO_PIN_14
#define LED5_RED_GPIO_Port GPIOD
#define LED6_BLUE_Pin GPIO_PIN_15
#define LED6_BLUE_GPIO_Port GPIOD

/* USER CODE BEGIN Private defines */
#define MAX_BUFFER      100
#define UART_TIMEOUT    1000
#define DEBUG           1
#define SPI_TIMEOUT     100


// Code anchor for break points
#define Nop() asm("nop")

extern void writeRTC(time_t now);
extern time_t readRTC(void);

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
