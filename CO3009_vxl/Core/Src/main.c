/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "scheduler.h"
#include "software_timer.h"
#include "global.h"
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
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim2;

/* USER CODE BEGIN PV */
/* Test counters: each TIM2 tick is 10 ms. */
volatile uint32_t test_tick_10ms = 0U;
volatile uint32_t test_count_500 = 0U;
volatile uint32_t test_count_1000 = 0U;
volatile uint32_t test_count_700 = 0U;
volatile uint32_t test_oneshot_count = 0U;
volatile uint32_t test_delete_count = 0U;
volatile uint32_t test_delete_ok = 0U;
volatile uint32_t test_last_500_tick = 0U;
volatile uint32_t test_last_1000_tick = 0U;
volatile uint32_t test_last_700_tick = 0U;
volatile uint32_t test_oneshot_tick = 0U;
volatile uint32_t test_delete_tick = 0U;
static uint32_t test_id_700 = NO_TASK_ID;

/* ===== Software Timer Monitoring ===== */

volatile uint32_t sw_first_expiry_tick[4] = {0};

volatile uint32_t sw_blink_count = 0;
volatile uint32_t sw_last_blink_tick = 0;

volatile uint32_t sw_cancel_tick = 0;
volatile uint32_t sw_msg_flag_after_cancel = 0;

volatile uint32_t sw_observer_count = 0;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_TIM2_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
static void Test_500ms(void)
{
  ++test_count_500;
  test_last_500_tick = test_tick_10ms;
}

static void Test_1000ms(void)
{
  ++test_count_1000;
  test_last_1000_tick = test_tick_10ms;
}

static void Test_700ms(void)
{
  ++test_count_700;
  test_last_700_tick = test_tick_10ms;
}

static void Test_OneShot(void)
{
  ++test_oneshot_count;
  test_oneshot_tick = test_tick_10ms;
}

static void Test_Delete700(void)
{
  ++test_delete_count;
  test_delete_tick = test_tick_10ms;
  test_delete_ok = SCH_Delete_Task(test_id_700);
}

/* Observe software timer expiration */
static void Test_SW_TimerObserver(void)
{
    sw_observer_count++;

    if (timer_flag[TIMER_PHASE] &&
        sw_first_expiry_tick[TIMER_PHASE] == 0U)
    {
        sw_first_expiry_tick[TIMER_PHASE] =
            test_tick_10ms;
    }

    if (timer_flag[TIMER_1SEC] &&
        sw_first_expiry_tick[TIMER_1SEC] == 0U)
    {
        sw_first_expiry_tick[TIMER_1SEC] =
            test_tick_10ms;
    }

    /* Test repeated 250 ms timer */
    if (timer_flag[TIMER_BLINK])
    {
        if (sw_first_expiry_tick[TIMER_BLINK] == 0U)
        {
            sw_first_expiry_tick[TIMER_BLINK] =
                test_tick_10ms;
        }

        sw_blink_count++;
        sw_last_blink_tick = test_tick_10ms;

        if (sw_blink_count < 4U)
        {
            setTimer(TIMER_BLINK, 250);
        }
        else
        {
            clearTimer(TIMER_BLINK);
        }
    }

    if (timer_flag[TIMER_MSG] &&
        sw_first_expiry_tick[TIMER_MSG] == 0U)
    {
        sw_first_expiry_tick[TIMER_MSG] =
            test_tick_10ms;
    }
}


/* Cancel message timer at 1500 ms */
static void Test_SW_CancelMsg(void)
{
    sw_cancel_tick = test_tick_10ms;

    clearTimer(TIMER_MSG);

    sw_msg_flag_after_cancel =
        timer_flag[TIMER_MSG];
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

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  /* Register tasks before enabling the TIM2 interrupt. */
  SCH_Init();

  const uint32_t id500 = SCH_Add_Task(Test_500ms, 50U, 50U);
  const uint32_t id1000 = SCH_Add_Task(Test_1000ms, 100U, 100U);
  const uint32_t idOneShot = SCH_Add_Task(Test_OneShot, 120U, 0U);
  test_id_700 = SCH_Add_Task(Test_700ms, 70U, 70U);
  const uint32_t idDelete = SCH_Add_Task(Test_Delete700, 230U, 0U);

  if (id500 == NO_TASK_ID || id1000 == NO_TASK_ID ||
      idOneShot == NO_TASK_ID || test_id_700 == NO_TASK_ID ||
      idDelete == NO_TASK_ID)
  {
    Error_Handler();
  }

  /* ===== Register Software Timer Tasks ===== */

  /* Run software timers every 10 ms */
  uint32_t idSwTimer = SCH_Add_Task(
      timerRun,
      0U,
      1U
  );

  /* Observe expiration flags every 10 ms */
  uint32_t idSwObserver = SCH_Add_Task(
      Test_SW_TimerObserver,
      1U,
      1U
  );

  /* Cancel message timer at 1500 ms */
  uint32_t idSwCancel = SCH_Add_Task(
      Test_SW_CancelMsg,
      150U,
      0U
  );

  /* Verify task registration */
  if (idSwTimer == NO_TASK_ID ||
      idSwObserver == NO_TASK_ID ||
      idSwCancel == NO_TASK_ID)
  {
      Error_Handler();
  }

  /*
   * Execute the initial delay-0 timerRun callback
   * before starting any software timer.
   */
  SCH_Dispatch_Tasks();

  /* Initialize four software timers */
  setTimer(TIMER_PHASE, 1200);
  setTimer(TIMER_1SEC, 1000);
  setTimer(TIMER_BLINK, 250);
  setTimer(TIMER_MSG, 3000);

  if (HAL_TIM_Base_Start_IT(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
/* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    SCH_Dispatch_Tasks();
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 7999;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 9;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, TL2_B_Pin|TL1_A_Pin|TL2_A_Pin|TL1_B_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : BTN_USER_Pin */
  GPIO_InitStruct.Pin = BTN_USER_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(BTN_USER_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : TL2_B_Pin TL1_A_Pin TL2_A_Pin TL1_B_Pin */
  GPIO_InitStruct.Pin = TL2_B_Pin|TL1_A_Pin|TL2_A_Pin|TL1_B_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : BTN_3_Pin */
  GPIO_InitStruct.Pin = BTN_3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(BTN_3_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : BTN_1_Pin BTN_2_Pin */
  GPIO_InitStruct.Pin = BTN_1_Pin|BTN_2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : BTN_4_Pin */
  GPIO_InitStruct.Pin = BTN_4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(BTN_4_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM2)
  {
    ++test_tick_10ms;
    SCH_Update();
  }
}
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

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
