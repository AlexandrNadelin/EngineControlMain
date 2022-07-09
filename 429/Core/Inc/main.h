/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
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
#define IGNITION_EN_3_Pin GPIO_PIN_2
#define IGNITION_EN_3_GPIO_Port GPIOE
#define IGNITION_EN_4_Pin GPIO_PIN_3
#define IGNITION_EN_4_GPIO_Port GPIOE
#define IGNITION_EN_5_Pin GPIO_PIN_4
#define IGNITION_EN_5_GPIO_Port GPIOE
#define IGNITION_EN_6_Pin GPIO_PIN_5
#define IGNITION_EN_6_GPIO_Port GPIOE
#define USER_Btn_Pin GPIO_PIN_13
#define USER_Btn_GPIO_Port GPIOC
#define MCO_Pin GPIO_PIN_0
#define MCO_GPIO_Port GPIOH
#define RMII_MDC_Pin GPIO_PIN_1
#define RMII_MDC_GPIO_Port GPIOC
#define TIM5_CH1_CrankShaft_Pin GPIO_PIN_0
#define TIM5_CH1_CrankShaft_GPIO_Port GPIOA
#define RMII_REF_CLK_Pin GPIO_PIN_1
#define RMII_REF_CLK_GPIO_Port GPIOA
#define RMII_MDIO_Pin GPIO_PIN_2
#define RMII_MDIO_GPIO_Port GPIOA
#define TIM5_CH4_CamShaft_Pin GPIO_PIN_3
#define TIM5_CH4_CamShaft_GPIO_Port GPIOA
#define RMII_CRS_DV_Pin GPIO_PIN_7
#define RMII_CRS_DV_GPIO_Port GPIOA
#define RMII_RXD0_Pin GPIO_PIN_4
#define RMII_RXD0_GPIO_Port GPIOC
#define RMII_RXD1_Pin GPIO_PIN_5
#define RMII_RXD1_GPIO_Port GPIOC
#define LD1_Pin GPIO_PIN_0
#define LD1_GPIO_Port GPIOB
#define RMII_TXD1_Pin GPIO_PIN_13
#define RMII_TXD1_GPIO_Port GPIOB
#define LD3_Pin GPIO_PIN_14
#define LD3_GPIO_Port GPIOB
#define USB_PowerSwitchOn_Pin GPIO_PIN_6
#define USB_PowerSwitchOn_GPIO_Port GPIOG
#define USB_OverCurrent_Pin GPIO_PIN_7
#define USB_OverCurrent_GPIO_Port GPIOG
#define USB_SOF_Pin GPIO_PIN_8
#define USB_SOF_GPIO_Port GPIOA
#define USB_VBUS_Pin GPIO_PIN_9
#define USB_VBUS_GPIO_Port GPIOA
#define USB_ID_Pin GPIO_PIN_10
#define USB_ID_GPIO_Port GPIOA
#define USB_DM_Pin GPIO_PIN_11
#define USB_DM_GPIO_Port GPIOA
#define USB_DP_Pin GPIO_PIN_12
#define USB_DP_GPIO_Port GPIOA
#define TMS_Pin GPIO_PIN_13
#define TMS_GPIO_Port GPIOA
#define TCK_Pin GPIO_PIN_14
#define TCK_GPIO_Port GPIOA
#define GPIO_EXTI0_IGNITION_1_Pin GPIO_PIN_0
#define GPIO_EXTI0_IGNITION_1_GPIO_Port GPIOD
#define GPIO_EXTI0_IGNITION_1_EXTI_IRQn EXTI0_IRQn
#define GPIO_EXTI1_IGNITION_2_Pin GPIO_PIN_1
#define GPIO_EXTI1_IGNITION_2_GPIO_Port GPIOD
#define GPIO_EXTI1_IGNITION_2_EXTI_IRQn EXTI1_IRQn
#define GPIO_EXTI2_IGNITION_3_Pin GPIO_PIN_2
#define GPIO_EXTI2_IGNITION_3_GPIO_Port GPIOD
#define GPIO_EXTI2_IGNITION_3_EXTI_IRQn EXTI2_IRQn
#define GPIO_EXTI3_IGNITION_4_Pin GPIO_PIN_3
#define GPIO_EXTI3_IGNITION_4_GPIO_Port GPIOD
#define GPIO_EXTI3_IGNITION_4_EXTI_IRQn EXTI3_IRQn
#define GPIO_EXTI4_IGNITION_5_Pin GPIO_PIN_4
#define GPIO_EXTI4_IGNITION_5_GPIO_Port GPIOD
#define GPIO_EXTI4_IGNITION_5_EXTI_IRQn EXTI4_IRQn
#define GPIO_EXTI5_IGNITION_6_Pin GPIO_PIN_5
#define GPIO_EXTI5_IGNITION_6_GPIO_Port GPIOD
#define GPIO_EXTI5_IGNITION_6_EXTI_IRQn EXTI9_5_IRQn
#define RMII_TX_EN_Pin GPIO_PIN_11
#define RMII_TX_EN_GPIO_Port GPIOG
#define RMII_TXD0_Pin GPIO_PIN_13
#define RMII_TXD0_GPIO_Port GPIOG
#define LD2_Pin GPIO_PIN_7
#define LD2_GPIO_Port GPIOB
#define IGNITION_EN_1_Pin GPIO_PIN_0
#define IGNITION_EN_1_GPIO_Port GPIOE
#define IGNITION_EN_2_Pin GPIO_PIN_1
#define IGNITION_EN_2_GPIO_Port GPIOE
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
