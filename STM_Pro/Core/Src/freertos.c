/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * File Name          : freertos.c
 * Description        : Code for freertos applications
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */     
#include "can.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "sbus_rc.h"
#include "hw_can.h"
#include "agv_ctr.h"
#include "agv_def.h"
#include "hw_ublox.h"
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
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for AGV_Ctr_Task */
osThreadId_t AGV_Ctr_TaskHandle;
const osThreadAttr_t AGV_Ctr_Task_attributes = {
  .name = "AGV_Ctr_Task",
  .priority = (osPriority_t) osPriorityNormal,
  .stack_size = 512 * 4
};
/* Definitions for Gps_Task */
osThreadId_t Gps_TaskHandle;
const osThreadAttr_t Gps_Task_attributes = {
  .name = "Gps_Task",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 512 * 4
};
/* Definitions for Sbus_Task */
osThreadId_t Sbus_TaskHandle;
const osThreadAttr_t Sbus_Task_attributes = {
  .name = "Sbus_Task",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 512 * 4
};
/* Definitions for RTK_Task */
osThreadId_t RTK_TaskHandle;
const osThreadAttr_t RTK_Task_attributes = {
  .name = "RTK_Task",
  .priority = (osPriority_t) osPriorityLow,
  .stack_size = 1024 * 4
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void tsk_agv_ctr(void *argument);
void tsk_gps(void *argument);
void tsk_sbus(void *argument);
void tsk_rtk(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */
	sbus_init();
	hw_canInit();
	agv_ctr_init();
	hw_ubloxInit();
	HAL_GPIO_WritePin(CPU_Power_GPIO_Port, CPU_Power_Pin, GPIO_PIN_RESET);
	HAL_Delay(1000);
	HAL_GPIO_WritePin(CPU_Power_GPIO_Port, CPU_Power_Pin, GPIO_PIN_SET);
  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
	/* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
	/* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
	/* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
	/* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of AGV_Ctr_Task */
  AGV_Ctr_TaskHandle = osThreadNew(tsk_agv_ctr, NULL, &AGV_Ctr_Task_attributes);

  /* creation of Gps_Task */
  Gps_TaskHandle = osThreadNew(tsk_gps, NULL, &Gps_Task_attributes);

  /* creation of Sbus_Task */
  Sbus_TaskHandle = osThreadNew(tsk_sbus, NULL, &Sbus_Task_attributes);

  /* creation of RTK_Task */
  RTK_TaskHandle = osThreadNew(tsk_rtk, NULL, &RTK_Task_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
	/* add threads, ... */
  /* USER CODE END RTOS_THREADS */

}

/* USER CODE BEGIN Header_tsk_agv_ctr */
/**
 * @brief  Function implementing the AGV_Ctr_Task thread.
 * @param  argument: Not used
 * @retval None
 */
/* USER CODE END Header_tsk_agv_ctr */
void tsk_agv_ctr(void *argument)
{
  /* USER CODE BEGIN tsk_agv_ctr */



	/* Infinite loop */
	for(;;)
	{
		agv_sysProc();
		osDelay(10);
	}
  /* USER CODE END tsk_agv_ctr */
}

/* USER CODE BEGIN Header_tsk_gps */
/**
 * @brief Function implementing the Gps_Task thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_tsk_gps */
void tsk_gps(void *argument)
{
  /* USER CODE BEGIN tsk_gps */
	/* Infinite loop */
	for(;;)
	{
		gps_uploadData();
		osDelay(10);
	}
  /* USER CODE END tsk_gps */
}

/* USER CODE BEGIN Header_tsk_sbus */
/**
 * @brief Function implementing the Sbus_Task thread.
 * @param argument: Not used
 * @retval None
 */
/* USER CODE END Header_tsk_sbus */
void tsk_sbus(void *argument)
{
  /* USER CODE BEGIN tsk_sbus */
	/* Infinite loop */
	for(;;)
	{
		osDelay(10);
	}
  /* USER CODE END tsk_sbus */
}

/* USER CODE BEGIN Header_tsk_rtk */
/**
* @brief Function implementing the RTK_Task thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_tsk_rtk */
void tsk_rtk(void *argument)
{
  /* USER CODE BEGIN tsk_rtk */
  /* Infinite loop */
  for(;;)
  {
	  agv_procRTKMsg();
	  osDelay(10);
  }
  /* USER CODE END tsk_rtk */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
