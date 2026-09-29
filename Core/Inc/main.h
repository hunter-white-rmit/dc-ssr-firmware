/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "stm32f0xx_hal.h"

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

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define CCPGOOD_Pin GPIO_PIN_0
#define CCPGOOD_GPIO_Port GPIOA
#define CCPGOOD_EXTI_IRQn EXTI0_1_IRQn
#define CCFLT_Pin GPIO_PIN_1
#define CCFLT_GPIO_Port GPIOA
#define CCFLT_EXTI_IRQn EXTI0_1_IRQn
#define PREPGOOD_Pin GPIO_PIN_2
#define PREPGOOD_GPIO_Port GPIOA
#define PREPGOOD_EXTI_IRQn EXTI2_3_IRQn
#define CCALM_Pin GPIO_PIN_3
#define CCALM_GPIO_Port GPIOA
#define CCALM_EXTI_IRQn EXTI2_3_IRQn
#define CCEN_Pin GPIO_PIN_4
#define CCEN_GPIO_Port GPIOA
#define PREEN_Pin GPIO_PIN_7
#define PREEN_GPIO_Port GPIOA
#define SCOPE_Pin GPIO_PIN_0
#define SCOPE_GPIO_Port GPIOB
#define OUTCMP2_Pin GPIO_PIN_1
#define OUTCMP2_GPIO_Port GPIOB
#define INCMP1_Pin GPIO_PIN_12
#define INCMP1_GPIO_Port GPIOA
#define INCMP1_EXTI_IRQn EXTI4_15_IRQn
#define HEARTBEAT_Pin GPIO_PIN_3
#define HEARTBEAT_GPIO_Port GPIOB
#define OUTCMP1_Pin GPIO_PIN_6
#define OUTCMP1_GPIO_Port GPIOB
#define OUTCMP1_EXTI_IRQn EXTI4_15_IRQn
#define OUTCMP2B7_Pin GPIO_PIN_7
#define OUTCMP2B7_GPIO_Port GPIOB
#define OUTCMP2B7_EXTI_IRQn EXTI4_15_IRQn

/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
