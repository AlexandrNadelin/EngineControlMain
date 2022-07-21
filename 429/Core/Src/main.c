/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "lwip.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "TimeSpan.h"
#include "Memory.h"
#include "CoreDebug.h"
#include "HTTPServer.h"
#include "ModbusCommon.h"
#include "ModbusTCPServer.h"
#include "DeviceData.h"
#include "string.h"
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
TIM_HandleTypeDef htim5;

/* USER CODE BEGIN PV */
/*#if   (defined ( __CC_ARM ))
__IO CommonData commonData  __attribute__((at(0x20000000)));
#elif (defined (__ICCARM__))
#pragma location = 0x20000000
__no_init __IO CommonData commonDta;
#elif defined   (  __GNUC__  )
__IO CommonData commonDta __attribute__((section(".RAMVectorTable")));
#endif*/

//---DEBUG---//
#include <stdio.h>
#define ITM_Port8(n)    (*((volatile unsigned char *)(0xE0000000+4*n)))
#define ITM_Port16(n)   (*((volatile unsigned short*)(0xE0000000+4*n)))
#define ITM_Port32(n)   (*((volatile unsigned long *)(0xE0000000+4*n)))
#define DEMCR           (*((volatile unsigned long *)(0xE000EDFC)))
#define TRCENA          0x01000000

struct __FILE { int handle; /* Add whatever you need here */ };
FILE __stdout;
FILE __stdin;

int fputc(int ch, FILE *f) {
   if (DEMCR & TRCENA) {

while (ITM_Port32(0) == 0)
	;
    ITM_Port8(0) = ch;
  }
  return(ch);
}
//---End Debug---//

Memory* memory = (void*)FIRST_PAGE_PROPERTY_ADDR;

HTTPServer httpServer;
ModbusTCPServer modbusTCPServer;

DeviceData deviceData;
float crankshaftTimeSpans1[CRANKSHAFT_POS_NUM];
float crankshaftTimeSpans2[CRANKSHAFT_POS_NUM];

const char infoStringFirstPart[]="Engine Control Device ID_";
char infoString[sizeof(infoStringFirstPart)+24];
InfoRegisters infoRegisters={(uint8_t*)infoString,sizeof(infoString)+24};

InputRegisters inputRegisters={
  .registers = (uint16_t*)&deviceData,
  .count = sizeof(DeviceData)/2,
};

HoldingRegisters holdingRegisters={
  .registers = (uint16_t*)FIRST_PAGE_PROPERTY_ADDR,
  .count = sizeof(Memory)/2,
};

ContactRegisters contactRegisters = {
  .bytes = NULL,
  .count = 0,
};

CoilRegisters coilRegisters = {
  .bytes = &deviceData.clearCounters,
  .count = 2,
};

/*float speedRPM = 1500.0F;
float speedRPS = 1500.0F/60.0F;
float crankshaftAllMarkCount = 60.0F;
float timerClock = 84000000.0F;*/
uint32_t oneMarkRefTick = (uint32_t)(84000000.0F/((1500.0F/60.0F)*60.0F));/*время одной метки в тиках*/

uint32_t camshaftOneTurnTickRef = (uint32_t)(84000000.0F/(750.0F/60.0F));/*время одного оборота в тиках*/


uint32_t lastFireErrorCylinderTicks[6]={0,0,0,0,0,0};
uint32_t lastMisfireCylinderTicks[6]={0,0,0,0,0,0};
//---DEL---//
/*uint8_t cylinder_1_crankshaft_pos =0;
uint8_t cylinder_2_crankshaft_pos =0;
uint8_t cylinder_3_crankshaft_pos =0;
uint8_t cylinder_4_crankshaft_pos =0;
uint8_t cylinder_5_crankshaft_pos =0;
uint8_t cylinder_6_crankshaft_pos =0;*/
//----------//
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM5_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void modbusRequestProcessedCallback(uint8_t modbusRequestType,uint16_t regAddress,uint16_t regCount)
{
	if(modbusRequestType==WRITE_COIL_REGISTER && regAddress==0)
	{
		deviceData.misfireCounter=0;
		
	}
	else if(modbusRequestType==WRITE_COIL_REGISTER && regAddress==1)deviceData.fireErrorCounter=0;
	else if(modbusRequestType==WRITE_COIL_REGISTERS)
	{
		if(regAddress==0)deviceData.misfireCounter=0;
		if(regAddress+regCount>1)deviceData.fireErrorCounter=0;
	}
}

void dataChangedCallback()
{
	if(deviceData.crankshaftFiringAllowed&0x01)HAL_GPIO_WritePin(IGNITION_EN_1_GPIO_Port,IGNITION_EN_1_Pin,GPIO_PIN_SET);
	else HAL_GPIO_WritePin(IGNITION_EN_1_GPIO_Port,IGNITION_EN_1_Pin,GPIO_PIN_RESET);
			
	if(deviceData.crankshaftFiringAllowed&0x02)HAL_GPIO_WritePin(IGNITION_EN_2_GPIO_Port,IGNITION_EN_2_Pin,GPIO_PIN_SET);
	else HAL_GPIO_WritePin(IGNITION_EN_2_GPIO_Port,IGNITION_EN_2_Pin,GPIO_PIN_RESET);
			
	if(deviceData.crankshaftFiringAllowed&0x04)HAL_GPIO_WritePin(IGNITION_EN_3_GPIO_Port,IGNITION_EN_3_Pin,GPIO_PIN_SET);
	else HAL_GPIO_WritePin(IGNITION_EN_3_GPIO_Port,IGNITION_EN_3_Pin,GPIO_PIN_RESET);
			
	if(deviceData.crankshaftFiringAllowed&0x08)HAL_GPIO_WritePin(IGNITION_EN_4_GPIO_Port,IGNITION_EN_4_Pin,GPIO_PIN_SET);
	else HAL_GPIO_WritePin(IGNITION_EN_4_GPIO_Port,IGNITION_EN_4_Pin,GPIO_PIN_RESET);
			
	if(deviceData.crankshaftFiringAllowed&0x10)HAL_GPIO_WritePin(IGNITION_EN_5_GPIO_Port,IGNITION_EN_5_Pin,GPIO_PIN_SET);
	else HAL_GPIO_WritePin(IGNITION_EN_5_GPIO_Port,IGNITION_EN_5_Pin,GPIO_PIN_RESET);
			
	if(deviceData.crankshaftFiringAllowed&0x20)HAL_GPIO_WritePin(IGNITION_EN_6_GPIO_Port,IGNITION_EN_6_Pin,GPIO_PIN_SET);
	else HAL_GPIO_WritePin(IGNITION_EN_6_GPIO_Port,IGNITION_EN_6_Pin,GPIO_PIN_RESET);
}
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
  __disable_irq();
  SCB->VTOR = MAIN_APP_START_ADDR;
	__enable_irq();
  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */
  uint32_t* id = (uint32_t*)UID_BASE;
  infoRegisters.len = sprintf((char*)infoRegisters.data,"%s%.8X%.8X%.8X",infoStringFirstPart,id[0],id[1],id[2]);
	
	#ifdef OLD_CODE
  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_LWIP_Init();
  MX_TIM5_Init();
  /* USER CODE BEGIN 2 */
	#endif	
	Memory_Init();
	
	MX_GPIO_Init();
  MX_LWIP_Init();
  MX_TIM5_Init();
	
	printf("Main App started.\r\n");
	
	ModbusTCPServer_Init(&modbusTCPServer);
	HTTPServer_Init(&httpServer);
  
	deviceData.isDataProcessAllowed=0;
	deviceData.isCylinderFiredMask=0x00;//deviceData.currentCylinderBitMaskCrankshaft=0x00;//битовая маска каленвала(в зоне какого цилиндра находится), здесь можно определить только группу цилиндров (1,6 или 2,5 или 3,4), какой именно сработал - нужно смотреть по маске распредвала
  //deviceData.currentCylinderBitMaskCAMshaft=0x00;  //битовая маска распредвала(в зоне какого цилиндра находится)
	deviceData.crankshaftFiringAllowed=0x3F;         //битовая маска каленвала разрешение на поджигание смеси в цилиндрах ( по умолчанию разрешено - запуск двигателя)
	//deviceData.camshaftFiringAllowed=0x3F;           //битовая маска распредвала разрешение на поджигание смеси в цилиндрах( по умолчанию разрешено - запуск двигателя)
	deviceData.currentCylinder=0;
	//deviceData.currentCylinderForAccelerationCalc=0;
	
	//Запуск таймера в режиме захвата
	//HAL_TIM_IC_Start_IT(&htim5, TIM_CHANNEL_1);
	//__HAL_TIM_ENABLE_IT(&htim5, TIM_CHANNEL_4);//Чтобы срабатывали прерывания от двух каналов
	/*HAL_TIM_IC_Start(&htim5, TIM_CHANNEL_1);
	__HAL_TIM_ENABLE_IT(&htim5, TIM_IT_CC1);
	__HAL_TIM_ENABLE_IT(&htim5, TIM_IT_CC4);*/
	HAL_TIM_IC_Start_IT(&htim5, TIM_CHANNEL_1);
	HAL_TIM_IC_Start_IT(&htim5, TIM_CHANNEL_4);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
		static uint32_t lastBlinkTime =0;
		if(GetTimeSpan(lastBlinkTime,HAL_GetTick())>500)
		{
			lastBlinkTime=HAL_GetTick();
		  HAL_GPIO_TogglePin(LD1_GPIO_Port,LD1_Pin);//GREEN
		  //HAL_GPIO_TogglePin(LD2_GPIO_Port,LD2_Pin);//BLUE
		  //HAL_GPIO_TogglePin(LD3_GPIO_Port,LD3_Pin);//RED
		}
		
		MX_LWIP_Process();
		
		static uint32_t buttonResetIPReleaseTime = 0;
		
		if(HAL_GPIO_ReadPin(USER_Btn_GPIO_Port,USER_Btn_Pin)==GPIO_PIN_RESET)buttonResetIPReleaseTime = HAL_GetTick();
		else if(GetTimeSpan(buttonResetIPReleaseTime,HAL_GetTick())>5000)
		{
			HAL_GPIO_WritePin(LD1_GPIO_Port,LD1_Pin,GPIO_PIN_RESET);//GREEN
		  HAL_GPIO_WritePin(LD2_GPIO_Port,LD2_Pin,GPIO_PIN_RESET);//BLUE
		  HAL_GPIO_WritePin(LD3_GPIO_Port,LD3_Pin,GPIO_PIN_RESET);//RED			
			HAL_Delay(500);
			HAL_GPIO_WritePin(LD1_GPIO_Port,LD1_Pin,GPIO_PIN_SET);//GREEN
			HAL_Delay(500);
			HAL_GPIO_WritePin(LD2_GPIO_Port,LD2_Pin,GPIO_PIN_SET);//BLUE
			HAL_Delay(500);
			HAL_GPIO_WritePin(LD3_GPIO_Port,LD3_Pin,GPIO_PIN_SET);//RED			
			HAL_Delay(500);
			HAL_GPIO_WritePin(LD1_GPIO_Port,LD1_Pin,GPIO_PIN_RESET);//GREEN
		  HAL_GPIO_WritePin(LD2_GPIO_Port,LD2_Pin,GPIO_PIN_RESET);//BLUE
		  HAL_GPIO_WritePin(LD3_GPIO_Port,LD3_Pin,GPIO_PIN_RESET);//RED
			
			NetworkParameters networkParameters ={
			  .ipAddr={192,168,1,251},
			  .netMask={255,255,255,0},
	      .gateWay={192,168,1,001},
			  .modbusPort=502,
		  };
			memoryWriteNetworkParameters(&networkParameters);
			
			HAL_Delay(500);
			
			buttonResetIPReleaseTime = HAL_GetTick();
	  }
		
		for(int i =0; i<6;i++)
		{
			if(deviceData.fireErrorCylinder[i] && GetTimeSpan(lastFireErrorCylinderTicks[i],HAL_GetTick())>4000)deviceData.fireErrorCylinder[i]=0x00;
			if(deviceData.misfireCylinder[i] && GetTimeSpan(lastMisfireCylinderTicks[i],HAL_GetTick())>4000)deviceData.misfireCylinder[i]=0;
		}
		
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
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
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 7;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief TIM5 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM5_Init(void)
{

  /* USER CODE BEGIN TIM5_Init 0 */

  /* USER CODE END TIM5_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_IC_InitTypeDef sConfigIC = {0};

  /* USER CODE BEGIN TIM5_Init 1 */

  /* USER CODE END TIM5_Init 1 */
  htim5.Instance = TIM5;
  htim5.Init.Prescaler = 0;
  htim5.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim5.Init.Period = 4294967295;
  htim5.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim5.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim5) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim5, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_IC_Init(&htim5) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim5, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_FALLING;
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1;
  sConfigIC.ICFilter = 4;
  if (HAL_TIM_IC_ConfigChannel(&htim5, &sConfigIC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING;
  if (HAL_TIM_IC_ConfigChannel(&htim5, &sConfigIC, TIM_CHANNEL_4) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM5_Init 2 */

  /* USER CODE END TIM5_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOG_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOE, IGNITION_EN_3_Pin|IGNITION_EN_4_Pin|IGNITION_EN_5_Pin|IGNITION_EN_6_Pin
                          |IGNITION_EN_1_Pin|IGNITION_EN_2_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LD1_Pin|LD3_Pin|LD2_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USB_PowerSwitchOn_GPIO_Port, USB_PowerSwitchOn_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : IGNITION_EN_3_Pin IGNITION_EN_4_Pin IGNITION_EN_5_Pin IGNITION_EN_6_Pin
                           IGNITION_EN_1_Pin IGNITION_EN_2_Pin */
  GPIO_InitStruct.Pin = IGNITION_EN_3_Pin|IGNITION_EN_4_Pin|IGNITION_EN_5_Pin|IGNITION_EN_6_Pin
                          |IGNITION_EN_1_Pin|IGNITION_EN_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOE, &GPIO_InitStruct);

  /*Configure GPIO pin : USER_Btn_Pin */
  GPIO_InitStruct.Pin = USER_Btn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USER_Btn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : LD1_Pin LD3_Pin LD2_Pin */
  GPIO_InitStruct.Pin = LD1_Pin|LD3_Pin|LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_PowerSwitchOn_Pin */
  GPIO_InitStruct.Pin = USB_PowerSwitchOn_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USB_PowerSwitchOn_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_OverCurrent_Pin */
  GPIO_InitStruct.Pin = USB_OverCurrent_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USB_OverCurrent_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : USB_SOF_Pin USB_ID_Pin USB_DM_Pin USB_DP_Pin */
  GPIO_InitStruct.Pin = USB_SOF_Pin|USB_ID_Pin|USB_DM_Pin|USB_DP_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
  GPIO_InitStruct.Alternate = GPIO_AF10_OTG_FS;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : USB_VBUS_Pin */
  GPIO_InitStruct.Pin = USB_VBUS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(USB_VBUS_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : GPIO_EXTI0_IGNITION_1_Pin GPIO_EXTI1_IGNITION_2_Pin GPIO_EXTI2_IGNITION_3_Pin GPIO_EXTI3_IGNITION_4_Pin
                           GPIO_EXTI4_IGNITION_5_Pin GPIO_EXTI5_IGNITION_6_Pin */
  GPIO_InitStruct.Pin = GPIO_EXTI0_IGNITION_1_Pin|GPIO_EXTI1_IGNITION_2_Pin|GPIO_EXTI2_IGNITION_3_Pin|GPIO_EXTI3_IGNITION_4_Pin
                          |GPIO_EXTI4_IGNITION_5_Pin|GPIO_EXTI5_IGNITION_6_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI0_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI0_IRQn);

  HAL_NVIC_SetPriority(EXTI1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI1_IRQn);

  HAL_NVIC_SetPriority(EXTI2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

  HAL_NVIC_SetPriority(EXTI3_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  HAL_NVIC_SetPriority(EXTI4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

}

/* USER CODE BEGIN 4 */
//Проверяем была ли ошибка раньше
void checkOtherFireErrorVars(uint8_t currentVarNumber)
{
	for(uint8_t i =0;i<6;i++)
	{
		if(i!=currentVarNumber && deviceData.fireErrorCylinder[i]!=0x00)return;
	}
	deviceData.fireErrorCylinder[currentVarNumber]|=0x80;//Это значит что ошибка началась с этого цилиндра
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(GPIO_Pin== GPIO_PIN_0)//Зажигание в первом цилиндре
	{
		if(deviceData.isDataProcessAllowed && deviceData.currentCylinder!=1)//искра там где не надо
		{
			lastFireErrorCylinderTicks[0]=HAL_GetTick();
			if(deviceData.fireErrorCylinder[0]&0x80)deviceData.fireErrorCylinder[0]=0x81;
			else 
			{
				deviceData.fireErrorCylinder[0]=0x01;
			  checkOtherFireErrorVars(0);//Проверяем была ли ошибка раньше, если не было - помечаем как первый цилиндр с которого началась ошибка
			}
			deviceData.fireErrorCounter++;
		}
		else
		{
			deviceData.isCylinderFiredMask|=0x01;
		}
	}
	else if(GPIO_Pin== GPIO_PIN_1)//Зажигание во втором цилиндре
	{
		if(deviceData.isDataProcessAllowed && deviceData.currentCylinder!=2)//искра там где не надо
		{
			lastFireErrorCylinderTicks[1]=HAL_GetTick();
			if(deviceData.fireErrorCylinder[1]&0x80)deviceData.fireErrorCylinder[1]=0x81;
			else 
			{
				deviceData.fireErrorCylinder[1]=0x01;
			  checkOtherFireErrorVars(1);//Проверяем была ли ошибка раньше, если не было - помечаем как первый цилиндр с которого началась ошибка
			}
			deviceData.fireErrorCounter++;
		}
		else
		{
			deviceData.isCylinderFiredMask|=0x02;
		}
	}
	else if(GPIO_Pin== GPIO_PIN_2)//Зажигание в третьем цилиндре
	{
		if(deviceData.isDataProcessAllowed && deviceData.currentCylinder!=3)//искра там где не надо
		{
			lastFireErrorCylinderTicks[2]=HAL_GetTick();
			if(deviceData.fireErrorCylinder[2]&0x80)deviceData.fireErrorCylinder[2]=0x81;
			else 
			{
				deviceData.fireErrorCylinder[2]=0x01;
			  checkOtherFireErrorVars(2);//Проверяем была ли ошибка раньше, если не было - помечаем как первый цилиндр с которого началась ошибка
			}
			deviceData.fireErrorCounter++;
		}
		else
		{
			deviceData.isCylinderFiredMask|=0x04;
		}
	}
	else if(GPIO_Pin== GPIO_PIN_3)//Зажигание в четвертом цилиндре
	{
		if(deviceData.isDataProcessAllowed && deviceData.currentCylinder!=4)//искра там где не надо
		{
			lastFireErrorCylinderTicks[3]=HAL_GetTick();
			if(deviceData.fireErrorCylinder[3]&0x80)deviceData.fireErrorCylinder[3]=0x81;
			else 
			{
				deviceData.fireErrorCylinder[3]=0x01;
			  checkOtherFireErrorVars(3);//Проверяем была ли ошибка раньше, если не было - помечаем как первый цилиндр с которого началась ошибка
			}
			deviceData.fireErrorCounter++;
		}
		else
		{
			deviceData.isCylinderFiredMask|=0x08;
		}
	}
	else if(GPIO_Pin== GPIO_PIN_4)//Зажигание в пятом цилиндре
	{
		if(deviceData.isDataProcessAllowed && deviceData.currentCylinder!=5)//искра там где не надо
		{
			lastFireErrorCylinderTicks[4]=HAL_GetTick();
			if(deviceData.fireErrorCylinder[4]&0x80)deviceData.fireErrorCylinder[4]=0x81;
			else 
			{
				deviceData.fireErrorCylinder[4]=0x01;
			  checkOtherFireErrorVars(4);//Проверяем была ли ошибка раньше, если не было - помечаем как первый цилиндр с которого началась ошибка
			}
			deviceData.fireErrorCounter++;
		}
		else
		{
			deviceData.isCylinderFiredMask|=0x10;
		}
	}
	else if(GPIO_Pin== GPIO_PIN_5)//Зажигание в шестом цилиндре
	{
		if(deviceData.isDataProcessAllowed && deviceData.currentCylinder!=6)//искра там где не надо
		{
			lastFireErrorCylinderTicks[5]=HAL_GetTick();
			if(deviceData.fireErrorCylinder[5]&0x80)deviceData.fireErrorCylinder[5]=0x81;
			else 
			{
				deviceData.fireErrorCylinder[5]=0x01;
			  checkOtherFireErrorVars(5);//Проверяем была ли ошибка раньше, если не было - помечаем как первый цилиндр с которого началась ошибка
			}
			deviceData.fireErrorCounter++;
		}
		else
		{
			deviceData.isCylinderFiredMask|=0x20;
		}
	}
}

void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{	
  if(htim->Instance == htim5.Instance)
	{	
		if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1)
		{
			//Сработалл захват каленвала
			deviceData.crankshaftTicks[deviceData.crankshaftCurPos] = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_1);
			uint32_t lastTick = deviceData.crankshaftCurPos==0?deviceData.crankshaftTicks[CRANKSHAFT_POS_NUM/**2*/-1]:deviceData.crankshaftTicks[deviceData.crankshaftCurPos-1];
			deviceData.crankshaftTickSpans[deviceData.crankshaftCurPos]=GetTimeSpan(lastTick,deviceData.crankshaftTicks[deviceData.crankshaftCurPos]);
			deviceData.crankshaftTimeSpans[deviceData.crankshaftCurPos]=deviceData.crankshaftTickSpans[deviceData.crankshaftCurPos]*1000.0F/84000000.0F;//В милисекундах
						
			static uint32_t oneMarkTick = 0;
			static uint32_t markTicksSum=0;
			static uint32_t currentPos = 0;
			oneMarkTick = 0;
			markTicksSum = 0;
			currentPos = deviceData.crankshaftCurPos;
			
			for(int i =0;i < CRANKSHAFT_POS_NUM; i++)
			{
				markTicksSum+=deviceData.crankshaftTickSpans[currentPos];
				if(currentPos==0)currentPos=CRANKSHAFT_POS_NUM-1;
				else currentPos--;
			}
			
			oneMarkTick = markTicksSum/CRANKSHAFT_ALL_POS_NUM;
			
			//если отклонение не больше чем на 15 % - можно вычислять 
			if(oneMarkTick>oneMarkRefTick*0.85F && oneMarkTick<oneMarkRefTick*1.15F)
			{
				// �?щем метку
				int i =0;
				for(;i<CRANKSHAFT_POS_NUM;i++)
				{
					if(oneMarkTick*1.6F<deviceData.crankshaftTickSpans[i] && oneMarkTick*2.4F>deviceData.crankshaftTickSpans[i])
					{
						//Метка найдена
						if(i>0)//Если метка не на первом месте в массиве - для дальнейшего удобства копируем массив временных промежутков так чтобы была на первом (экономит время поиска)
						{
							currentPos = i;
							uint32_t tmpArr[CRANKSHAFT_POS_NUM];
							memcpy(tmpArr,deviceData.crankshaftTickSpans,CRANKSHAFT_POS_NUM);
							for(int j =0;j < CRANKSHAFT_POS_NUM; j++)
			        {
								deviceData.crankshaftTickSpans[j]=tmpArr[currentPos++];
				        if(currentPos==CRANKSHAFT_POS_NUM)currentPos=0;
			        }
							
							if(i>deviceData.crankshaftCurPos)deviceData.crankshaftCurPos = deviceData.crankshaftCurPos + CRANKSHAFT_POS_NUM - i;
				      else deviceData.crankshaftCurPos-=i;//чтобы следующий элемент копировался куда положено
							//теперь каждый элемент на своем месте, первый элемент - временной промежуток между предпоследней меткой (последняя отсутствует) и первой меткой
						}
						
						break;
					}
				}	
        if(i ==	CRANKSHAFT_POS_NUM)//Не найдено начальная метка коленвала ( вообще такого не должно быть, но ХЗ)
				{
          if(++deviceData.crankshaftCurPos==CRANKSHAFT_POS_NUM/**2*/)deviceData.crankshaftCurPos=0;
					deviceData.crankshaftFiringAllowed=0x3F;         //битовая маска каленвала разрешение на поджигание смеси в цилиндрах ( по умолчанию разрешено - запуск двигателя)
	        //deviceData.camshaftFiringAllowed=0x3F;           //битовая маска распредвала разрешение на поджигание смеси в цилиндрах( по умолчанию разрешено - запуск двигателя)
					
					dataChangedCallback();//Включаем зажигание
					return;
				}

				if(!deviceData.isDataProcessAllowed)//Скорость вращения распредвала больше или меньше чем скорость, соответствующая 1500 оборотов в минуту, зажигание разрешаем, продолжаем работать
				{
					if(++deviceData.crankshaftCurPos==CRANKSHAFT_POS_NUM/**2*/)deviceData.crankshaftCurPos=0;
					deviceData.crankshaftFiringAllowed=0x3F;         //битовая маска каленвала разрешение на поджигание смеси в цилиндрах ( по умолчанию разрешено - запуск двигателя)
					
					dataChangedCallback();//Включаем зажигание
					return;
				}
				
        //Производим необходимые вычисления
					
				//запрещаем зажигание
				deviceData.crankshaftFiringAllowed=0x00;
				
				if(deviceData.crankshaftCurPos==50/*45*/)//(deviceData.crankshaftCurPos>44 || (deviceData.crankshaftCurPos>=0 && deviceData.crankshaftCurPos<5))
				{
					//проверяем происходило ли зажигание в 3 и 4 цилиндрах
					if(deviceData.currentCylinder==3)//прошлый цилиндр - 3
					{
						//сбрасываем флаги зажигание произошло в 6 цилиндре
					  deviceData.isCylinderFiredMask&=~0x20;
						
						if(!(deviceData.isCylinderFiredMask&0x04))//поджигание смеси в третьем цилиндре не происходило, авария
						{//еще проверить отрицательное ускорени результат - операция или
							deviceData.misfireCylinder[2] =0x01;
							lastMisfireCylinderTicks[2]=HAL_GetTick();
							deviceData.misfireCounter++;
						}
						//else deviceData.misfireCylinder[2] =0x00;
					}
					else if(deviceData.currentCylinder==4)//прошлый цилиндр - 4
					{
						//сбрасываем флаги зажигание произошло в 1 цилиндре
					  deviceData.isCylinderFiredMask&=~0x01;
						
						if(!(deviceData.isCylinderFiredMask&0x08))//поджигание смеси в четвертом цилиндре не происходило, авария
						{
							deviceData.misfireCylinder[3] =0x01;
							lastMisfireCylinderTicks[3]=HAL_GetTick();
							deviceData.misfireCounter++;
						}
						//else deviceData.misfireCylinder[3] =0x00;
					}
					
					//deviceData.currentCylinderForAccelerationCalc=deviceData.currentCylinder;
				}	
        else if(deviceData.crankshaftCurPos==5)//(deviceData.crankshaftCurPos>4 && deviceData.crankshaftCurPos<10)
				{
					//разрешаем зажигание в 1 или 6 цилиндрах
					if(deviceData.currentCylinder==1)
					{
						deviceData.crankshaftFiringAllowed=0x01;
						//Предыдущтий цилиндр был четвертый
					  uint32_t sumTicks =0;
					  for(int i = 50;i<59;i++)
					  {
						  sumTicks+=deviceData.crankshaftTickSpans[i];
					  }
					  for(int i = 0;i<5;i++)
					  {
						  sumTicks+=deviceData.crankshaftTickSpans[i];
					  }
					  deviceData.cylinder_4_Time=sumTicks*1000.0F/84000000.0F;
					}
					else if(deviceData.currentCylinder==6)
					{
						deviceData.crankshaftFiringAllowed=0x20;
						//Предыдущтий цилиндр был третий
					  uint32_t sumTicks =0;
					  for(int i = 50;i<59;i++)
					  {
						  sumTicks+=deviceData.crankshaftTickSpans[i];
					  }
					  for(int i = 0;i<5;i++)
					  {
						  sumTicks+=deviceData.crankshaftTickSpans[i];
					  }
					  deviceData.cylinder_3_Time=sumTicks*1000.0F/84000000.0F;
					}
				}	
        else if(deviceData.crankshaftCurPos==10)//(deviceData.crankshaftCurPos>9  && deviceData.crankshaftCurPos<25)
				{
					//проверяем происходило ли зажигание в 1 и 6 цилиндрах
					if(deviceData.currentCylinder==1)//прошлый цилиндр - 1
					{
						//сбрасываем флаги зажигание произошло в 5 цилиндре
					  deviceData.isCylinderFiredMask&=~0x10;
						
						if(!(deviceData.isCylinderFiredMask&0x01))//поджигание смеси в первом цилиндре не происходило, авария
						{//еще проверить отрицательное ускорени результат - операция или
							deviceData.misfireCylinder[0] =0x01;
							lastMisfireCylinderTicks[0]=HAL_GetTick();
							deviceData.misfireCounter++;
						}
						//else deviceData.misfireCylinder[0] =0x00;
					}
					else if(deviceData.currentCylinder==6)//прошлый цилиндр - 6
					{
						//сбрасываем флаги зажигание произошло в 2 цилиндре
					  deviceData.isCylinderFiredMask&=~0x02;
						
						if(!(deviceData.isCylinderFiredMask&0x20))//поджигание смеси в шестом цилиндре не происходило, авария
						{
							deviceData.misfireCylinder[5] =0x01;
							lastMisfireCylinderTicks[5]=HAL_GetTick();
							deviceData.misfireCounter++;
						}
						//else deviceData.misfireCylinder[5] =0x00;
					}
					
					//deviceData.currentCylinderForAccelerationCalc=deviceData.currentCylinder;
				}
        else if(deviceData.crankshaftCurPos==25)//(deviceData.crankshaftCurPos>24 && deviceData.crankshaftCurPos<30)
				{
					//разрешаем зажигание в 2 или 5 цилиндрах
					if(deviceData.currentCylinder==2)
					{
						deviceData.crankshaftFiringAllowed=0x02;
						//Предыдущтий цилиндр был шестой
					  uint32_t sumTicks =0;
					  for(int i = 10;i<25;i++)
					  {
						  sumTicks+=deviceData.crankshaftTickSpans[i];
					  }
					  deviceData.cylinder_6_Time=sumTicks*1000.0F/84000000.0F;
					}
					else if(deviceData.currentCylinder==5)
					{
						deviceData.crankshaftFiringAllowed=0x10;	
						//Предыдущтий цилиндр был первый
					  uint32_t sumTicks =0;
					  for(int i = 10;i<25;i++)
					  {
						  sumTicks+=deviceData.crankshaftTickSpans[i];
					  }
					  deviceData.cylinder_1_Time=sumTicks*1000.0F/84000000.0F;
				  }					
				}		
        else if(deviceData.crankshaftCurPos==30)//(deviceData.crankshaftCurPos>29  && deviceData.crankshaftCurPos<45)//зажигание в 3 и 4 цилиндрах еще не разрешено
				{
					//проверяем происходило ли зажигание в 2 и 5 цилиндрах
					if(deviceData.currentCylinder==2)//прошлый цилиндр - 2
					{
						//сбрасываем флаги зажигание произошло в 4 цилиндрах
					  deviceData.isCylinderFiredMask&=~0x08;
						
						if(!(deviceData.isCylinderFiredMask&0x02))//поджигание смеси во втором цилиндре не происходило, авария
						{//еще проверить отрицательное ускорени результат - операция или
							deviceData.misfireCylinder[1] =0x01;
							lastMisfireCylinderTicks[1]=HAL_GetTick();
							deviceData.misfireCounter++;
						}
						//else deviceData.misfireCylinder[1] =0x00;
					}
					else if(deviceData.currentCylinder==5)//прошлый цилиндр - 5
					{
						//сбрасываем флаги зажигание произошло в 3 цилиндрах
					  deviceData.isCylinderFiredMask&=~0x04;
						
						if(!(deviceData.isCylinderFiredMask&0x10))//поджигание смеси в пятом цилиндре не происходило, авария
						{
							deviceData.misfireCylinder[4] =0x01;
							lastMisfireCylinderTicks[4]=HAL_GetTick();
							deviceData.misfireCounter++;
						}
						//else deviceData.misfireCylinder[4] =0x00;
					}
					
					//deviceData.currentCylinderForAccelerationCalc=deviceData.currentCylinder;
				}
        else if(deviceData.crankshaftCurPos==45)//(deviceData.crankshaftCurPos>44 && deviceData.crankshaftCurPos<60)//разрешеено зажигание в 2 и 5 цилиндрах
				{
					//разрешаем зажигание в 3 или 4 цилиндрах
					if(deviceData.currentCylinder==3)
					{
						deviceData.crankshaftFiringAllowed=0x04;
						//Предыдущтий цилиндр был пятый 
					  uint32_t sumTicks =0;
					  for(int i = 30;i<45;i++)
					  {
						  sumTicks+=deviceData.crankshaftTickSpans[i];
					  }
					  deviceData.cylinder_5_Time=sumTicks*1000.0F/84000000.0F;
					}
					else if(deviceData.currentCylinder==4)
					{
						deviceData.crankshaftFiringAllowed=0x08;
						//Предыдущтий цилиндр был второй
					  uint32_t sumTicks =0;
					  for(int i = 30;i<45;i++)
					  {
						  sumTicks+=deviceData.crankshaftTickSpans[i];
					  }
					  deviceData.cylinder_2_Time=sumTicks*1000.0F/84000000.0F;
					}
				}
				else if(deviceData.crankshaftCurPos==CRANKSHAFT_POS_NUM-1)
				{
					if(deviceData.currentCylinder==3)memcpy(crankshaftTimeSpans1,deviceData.crankshaftTimeSpans,CRANKSHAFT_POS_NUM*4);
					else if(deviceData.currentCylinder==4)memcpy(crankshaftTimeSpans2,deviceData.crankshaftTimeSpans,CRANKSHAFT_POS_NUM*4);
				}
			}
			else//Разрешаем поджигать смесь - возможно происходит перезапуск двигателя
			{
				deviceData.crankshaftFiringAllowed=0x3F;         //битовая маска каленвала разрешение на поджигание смеси в цилиндрах ( по умолчанию разрешено - запуск двигателя)
			}
			
			if(++deviceData.crankshaftCurPos==CRANKSHAFT_POS_NUM/**2*/)deviceData.crankshaftCurPos=0;
			
			dataChangedCallback();//Данные обновились - можно принимать решение об управлении					
		}
		else if(htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4)
		{
			//Спработал захват распредвала
			deviceData.camshaftTicks[deviceData.camshaftCurPos] = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_4);
			uint32_t lastTick = deviceData.camshaftCurPos==0?deviceData.camshaftTicks[CAMSHAFT_POS_NUM+1-1]:deviceData.camshaftTicks[deviceData.camshaftCurPos-1];
			deviceData.camshaftTickSpans[deviceData.camshaftCurPos]=GetTimeSpan(lastTick,deviceData.camshaftTicks[deviceData.camshaftCurPos]);
			deviceData.camshaftTimeSpans[deviceData.camshaftCurPos]=deviceData.camshaftTickSpans[deviceData.camshaftCurPos]*1000.0F/84000000.0F;//В милисекундах
			
			static uint32_t oneTurnTick = 0;
			static uint32_t currentPos = 0;
			oneTurnTick = 0;
			currentPos = deviceData.camshaftCurPos;
			
			for(int i =0;i < CAMSHAFT_POS_NUM+1; i++)
			{
				oneTurnTick+=deviceData.camshaftTickSpans[currentPos];
				if(currentPos==0)currentPos=CAMSHAFT_POS_NUM+1-1;
				else currentPos--;
			}
			
			//если отклонение не больше чем на 15 % - можно вычислять 
			if(oneTurnTick>camshaftOneTurnTickRef*0.85F && oneTurnTick<camshaftOneTurnTickRef*1.15F)
			{
				deviceData.isDataProcessAllowed=1;
				// �?щем метку
				int i =0;
				for(;i<CAMSHAFT_POS_NUM+1;i++)
				{
					//Важно учесть что первый элемент массива распредвала - это как раз расстояние между первым и вторым зубом, а каленвала между предпоследним (последний выкинут) и первым
					if(deviceData.camshaftTickSpans[i]>(oneTurnTick*(20.0F/360.0F))*0.95F && deviceData.camshaftTickSpans[i]<(oneTurnTick*(20.0F/360.0F))*1.05F)//Метка - начало оборота, 1 цилиндр
					{
						//Метка найдена
						if(i>0)//Если метка не на первом месте в массиве - для дальнейшего удобства копируем массив временных промежутков так чтобы была на первом (экономит время поиска)
						{
							currentPos = i;
							uint32_t tmpArr[CAMSHAFT_POS_NUM+1];
							memcpy(tmpArr,deviceData.camshaftTickSpans,CAMSHAFT_POS_NUM+1);
							for(int j =0;j < CAMSHAFT_POS_NUM+1; j++)
			        {
								deviceData.camshaftTickSpans[j]=tmpArr[currentPos++];
				        if(currentPos==CAMSHAFT_POS_NUM+1)currentPos=0;
			        }
							
							if(i>deviceData.camshaftCurPos)deviceData.camshaftCurPos = deviceData.camshaftCurPos + CAMSHAFT_POS_NUM+1 - i;
				      else deviceData.camshaftCurPos-=i;//чтобы следующий элемент копировался куда положено

							//теперь каждый элемент на своем месте, первый элемент - временной промежуток между предпоследней меткой (последняя отсутствует) и первой меткой
						}
						
						break;
					}
				}	
        if(i ==	CAMSHAFT_POS_NUM+1)//Не найдено начальная метка распредвала ( вообще такого не должно быть, но ХЗ)
				{
          if(++deviceData.camshaftCurPos==CAMSHAFT_POS_NUM+1)deviceData.camshaftCurPos=0;
					deviceData.isDataProcessAllowed=0;
					
					deviceData.crankshaftFiringAllowed=0x3F;         //битовая маска каленвала разрешение на поджигание смеси в цилиндрах ( по умолчанию разрешено - запуск двигателя)					
					dataChangedCallback();//Включаем зажигание
					return;
				}	
				if(deviceData.camshaftCurPos==1)
				{
					deviceData.currentCylinder = 5;//Номер цилиндра в котором в данный момент происходит поджигание смеси
					//cylinder_5_crankshaft_pos = deviceData.crankshaftCurPos;
				}
        else if(deviceData.camshaftCurPos==2)
				{
					deviceData.currentCylinder = 3;
					//cylinder_3_crankshaft_pos = deviceData.crankshaftCurPos;
				}
        else if(deviceData.camshaftCurPos==3)
				{
					deviceData.currentCylinder = 6;
					//cylinder_6_crankshaft_pos = deviceData.crankshaftCurPos;
				}
        else if(deviceData.camshaftCurPos==4)
				{
					deviceData.currentCylinder = 2;
					//cylinder_2_crankshaft_pos = deviceData.crankshaftCurPos;
				}
        else if(deviceData.camshaftCurPos==5)
				{
					deviceData.currentCylinder = 4;
					//cylinder_4_crankshaft_pos = deviceData.crankshaftCurPos;
				}
        else if(deviceData.camshaftCurPos==6)
				{
					deviceData.currentCylinder = 1;
					//cylinder_1_crankshaft_pos = deviceData.crankshaftCurPos;
				}				
			}
			else deviceData.isDataProcessAllowed=0;
			
			if(++deviceData.camshaftCurPos==CAMSHAFT_POS_NUM+1)deviceData.camshaftCurPos=0;
			
			//dataChangedCallback();//Данные обновились - можно принимать решение об управлении
		}
	}
}
/* USER CODE END 4 */

 /**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM10 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM10) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */
  if(htim->Instance==htim5.Instance)
	{		
		__NOP();
    //Период таймера около 58 секунд//irTransmitter.transmitCmpltCallback(&irTransmitter);		     			
	}
  /* USER CODE END Callback 1 */
}

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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
