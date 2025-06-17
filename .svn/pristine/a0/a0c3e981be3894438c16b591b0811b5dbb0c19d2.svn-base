/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "string.h"
#include "canfdspi_api.h"
#include "canfdspi_defines.h"
#include "canfdspi_register.h"
#include "app.h"
#include "time.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
RTC_HandleTypeDef hrtc;

SPI_HandleTypeDef hspi1;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
char logTime[9];
char txBuffer[MAX_BUFFER];
char tempBuffer[MAX_BUFFER];
uint8_t canRxInterrupt = 0;
uint8_t canTxInterrupt = 0;
extern APP_DATA appData;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_SPI1_Init(void);
static void MX_RTC_Init(void);
static void MX_USART2_UART_Init(void);
/* USER CODE BEGIN PFP */



void writeRTC(time_t now);
time_t readRTC(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
/***************************************************************************************************************
*     S E R I A L   O U T                                                              P A C K   E M U L A T O R
***************************************************************************************************************/
void serialOut(char* message){
  getTime();
  sprintf(txBuffer,"%s %s\r\n",logTime, message);

  if (HAL_UART_Transmit(&huart2, (uint8_t*)txBuffer, strlen(txBuffer), 2000) != HAL_OK){// Sending in normal mode
    Error_Handler();
  }
}

/***************************************************************************************************************
*     T I M E S T A M P                                                                P A C K   E M U L A T O R
***************************************************************************************************************/
void getTime(void){

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  HAL_RTC_GetTime(&hrtc,&sTime, RTC_FORMAT_BCD);
  HAL_RTC_GetDate(&hrtc,&sDate, RTC_FORMAT_BCD);
  uint8_t seconds = sTime.Seconds;
  uint8_t minutes = sTime.Minutes;
  uint8_t hours = sTime.Hours;
  sprintf(logTime,"%02x:%02x:%02x",hours,minutes,seconds);
}

/***************************************************************************************************************
 *     G P I O     I N T E R R U P T    H A N D L E R   &   C A L L B A C              P A C K   E M U L A T O R
***************************************************************************************************************/
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {

  // Clear Wake Up Flag (when woken from STOP
  //__HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);

  if(GPIO_Pin == CAN_INT_Pin){
    // Interrupt
  }
  else if (GPIO_Pin == CAN_INT0_Pin){
    // TX Interrupt
    canTxInterrupt = 1;
  }
  else if (GPIO_Pin == CAN_INT1_Pin){
    // RX Interrupt
    canRxInterrupt = 1;
    appData.state = APP_STATE_RECEIVE;
  }
  else if(GPIO_Pin == USER_BUTTON_Pin){
    // Spawn a new module?
  }
}


/***************************************************************************************************************
*     w r i t e R T C                                                                  P A C K   E M U L A T O R
***************************************************************************************************************/
void writeRTC(time_t now)
{

 RTC_TimeTypeDef sTime;
 RTC_DateTypeDef sDate;

 struct tm time_tm;
 time_tm = *(localtime(&now));

 sTime.Hours = (uint8_t)time_tm.tm_hour;
 sTime.Minutes = (uint8_t)time_tm.tm_min;
 sTime.Seconds = (uint8_t)time_tm.tm_sec;
 // Set the time in the RTC
 if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
 {
   sprintf(tempBuffer,"ERROR - HAL_RTC_SetTime returned error"); serialOut(tempBuffer);
 }

 if (time_tm.tm_wday == 0) { time_tm.tm_wday = 7; }     // STM32 day order is Monday to Sunday
 sDate.WeekDay = (uint8_t)time_tm.tm_wday;
 sDate.Month = (uint8_t)time_tm.tm_mon+1;               // STM32 month starts at 1
 sDate.Date = (uint8_t)time_tm.tm_mday;
 sDate.Year = (uint16_t)(time_tm.tm_year + 1900-2000);  // time.h is years since 1900, STM32 is years since 2000

//Set the Date in the RTC
 if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
 {
  sprintf(tempBuffer,"ERROR - HAL_RTC_SetDate returned error"); serialOut(tempBuffer);
 }

 HAL_RTCEx_BKUPWrite(&hrtc,RTC_BKP_DR0,0x32F2);       // lock it in with the backup registers

}

/***************************************************************************************************************
*     r e a d R T C                                                                    P A C K   E M U L A T O R
***************************************************************************************************************/
time_t readRTC(void)
{

  RTC_DateTypeDef rtcDate;
  RTC_TimeTypeDef rtcTime;
  time_t currentTime = {0};
  struct tm tim = {0};

  HAL_RTC_GetTime(&hrtc, &rtcTime, RTC_FORMAT_BIN);
  HAL_RTC_GetDate(&hrtc, &rtcDate, RTC_FORMAT_BIN);
  uint8_t hh  = rtcTime.Hours;
  uint8_t mm  = rtcTime.Minutes;
  uint8_t ss  = rtcTime.Seconds;
  uint8_t d   = rtcDate.Date;
  uint8_t m   = rtcDate.Month;
  uint16_t y  = rtcDate.Year;
  uint16_t yr = (uint16_t)(y + 2000-1900);  // time.h is years since 1900, STM32 is years since 2000

  tim.tm_year = yr;
  tim.tm_mon  = m - 1;       // adjust the month numbering (STM32 starts at one, time.h starts at 0)
  tim.tm_mday = d;
  tim.tm_hour = hh;
  tim.tm_min  = mm;
  tim.tm_sec  = ss;
  currentTime = mktime(&tim);

  return currentTime;

  /*
  struct tm printTm = {0};
  printTm = *(localtime(&currentTime));
  char buffer[80];
  strftime(buffer,80,'RTC %m/%d/%y %H:%M:%S', &printTm);
  sprintf('%s  ', buffer);
*/
}

/***************************************************************************************************************
*     t e s t T i m e                                                                  P A C K   E M U L A T O R
***************************************************************************************************************/
time_t testTime(void)
{
  // test function to check that the 2038 bug does not exist in the time library (years > 2038 results in overflow if bug exists)

  struct timeval time;
  struct tm t_decomp;
  time_t seconds = {0};
  uint8_t sizeBytes = 0;

  time.tv_sec = -1;

  t_decomp.tm_year  = 139; //2039
  t_decomp.tm_mon   = 0;
  t_decomp.tm_mday  = 1;
  t_decomp.tm_hour  = 8;
  t_decomp.tm_min   = 37;
  t_decomp.tm_sec   = 53;
  t_decomp.tm_isdst = -1;

  time.tv_sec = mktime(&t_decomp);
  time.tv_usec = 0;

  sizeBytes = sizeof(time.tv_sec); // check sizeBytes = 8 (64 bit)
  sizeBytes = sizeBytes;           // added to get rid of warning for variables set but not used!
  seconds   = time.tv_sec;         // Check that seconds is positive for year 2039 (2177483873)
  return seconds;

  // If seconds returned by mktime() is negative then the 2038 bug is an issue - the time is decoded into 1902-11-26 02:09:37,000 (instead of 2039-01-01)
  // TESTED and all good!
}


/***************************************************************************************************************
*     M A I N                                                                          P A C K   E M U L A T O R
***************************************************************************************************************/
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_SPI1_Init();
  MX_RTC_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */


  DRV_CANFDSPI_Reset(DRV_CANFDSPI_INDEX_0);
  APP_Initialize();
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    APP_Tasks();
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSI|RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 64;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV4;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief SPI1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_SPI1_Init(void)
{

  /* USER CODE BEGIN SPI1_Init 0 */

  /* USER CODE END SPI1_Init 0 */

  /* USER CODE BEGIN SPI1_Init 1 */

  /* USER CODE END SPI1_Init 1 */
  /* SPI1 parameter configuration*/
  hspi1.Instance = SPI1;
  hspi1.Init.Mode = SPI_MODE_MASTER;
  hspi1.Init.Direction = SPI_DIRECTION_2LINES;
  hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
  hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
  hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
  hspi1.Init.NSS = SPI_NSS_SOFT;
  hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;
  hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
  hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
  hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
  hspi1.Init.CRCPolynomial = 10;
  if (HAL_SPI_Init(&hspi1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN SPI1_Init 2 */

  /* USER CODE END SPI1_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */


  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */


  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */


  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOD, CAN_CS_Pin|LED4_GREEN_Pin|LED3_ORANGE_Pin|LED5_RED_Pin
                          |LED6_BLUE_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : USER_BUTTON_Pin */
  GPIO_InitStruct.Pin = USER_BUTTON_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_BUTTON_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : CAN_CS_Pin LED4_GREEN_Pin LED3_ORANGE_Pin LED5_RED_Pin
                           LED6_BLUE_Pin */
  GPIO_InitStruct.Pin = CAN_CS_Pin|LED4_GREEN_Pin|LED3_ORANGE_Pin|LED5_RED_Pin
                          |LED6_BLUE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /*Configure GPIO pins : CAN_INT_Pin CAN_INT1_Pin CAN_INT0_Pin */
  GPIO_InitStruct.Pin = CAN_INT_Pin|CAN_INT1_Pin|CAN_INT0_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
