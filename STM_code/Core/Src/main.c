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
#include "adc.h"
#include "i2c.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdbool.h"
#include "ssd1306.h"
#include "ssd1306_tests.h"
#include "mcp9808.h"
#include "pid.h"
#include <stdio.h>
#include <stdlib.h>
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

/* USER CODE BEGIN PV */
char received_data[4];  // do tego będą sie odbierać dane z portu szeregowego
float akutalna_temperatura = 0.00;  // temperatura aktualna tu bedzie
float value = 0.00; // to z zadajnika analogowego 0.0f-1.0f
uint16_t current_duty_cycle = 0; // 0-1000 (multiplied x10 to get higher resolution)
uint16_t sterowanie = 0;
float zadana_temperatura = 0;
float kp = 55;
float ki = 0.08;
float kd = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

bool grzanie_on_off() {
	if(HAL_GPIO_ReadPin(przycisk0_GPIO_Port, przycisk0_Pin) == GPIO_PIN_SET){
		return true;
	}
	else{
		return false;
	}
}

bool auto_manual_on_off() {
	if(HAL_GPIO_ReadPin(przycisk1_GPIO_Port, przycisk1_Pin) == GPIO_PIN_SET){
		return true;
	}
	else{
		return false;
	}
}

// Z tym gównem też chyba jest coś nie tak
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	//HAL_UART_Receive_IT(&huart3, received_data, 3); // Tu włącza sie to gowno znowu :)
	if(auto_manual_on_off()){
		if(atof(received_data)>50){
			zadana_temperatura = 50;
		}
		else if(atof(received_data)<20){
			zadana_temperatura = 20;
		}
		else{
			zadana_temperatura = atof(received_data);
		}
	}
}

float zadajnik() {
	HAL_ADC_Start(&hadc1);
	HAL_ADC_PollForConversion(&hadc1, HAL_MAX_DELAY);
	uint32_t value = HAL_ADC_GetValue(&hadc1);
	float ser = (1.0f * value /4095.0f)*30+20;
	return ser;
}

void wentyl(){
	float blad = zadana_temperatura - akutalna_temperatura;
	float hist = -0.5;
	if (blad<=hist){
		HAL_GPIO_WritePin(wentylator_GPIO_Port, wentylator_Pin, GPIO_PIN_SET);
	}
	if (blad>-0.125){
		HAL_GPIO_WritePin(wentylator_GPIO_Port, wentylator_Pin, GPIO_PIN_RESET);
	}
}

// Jakbym miał powiedzieć gdzie sie wypierdoli ten kod to wlasnie tutaj
void change_current_duty_cycle(TIM_HandleTypeDef* htim, uint32_t channel, uint16_t current_duty_cycle)
{
    // calculate the new pulse value
    uint32_t pulse = ((htim->Init.Period+1) * current_duty_cycle) / 1000;
    // update the capture/compare register
    __HAL_TIM_SET_COMPARE(htim, channel, pulse);
}

void wyswietlacz(){
	ssd1306_Fill(Black);
	uint8_t y = 0; // ktora linia wyswietlacza

	char temp_buf[6];
	gcvt(akutalna_temperatura, 6, temp_buf);
	ssd1306_SetCursor(2, y);
	char buf0[20] = "Temperatura:";
	ssd1306_WriteString(strcat(buf0, temp_buf), Font_7x10, White);
	y += 10;

	char set_buf[6];
	gcvt(zadana_temperatura, 4, set_buf);
	ssd1306_SetCursor(2, y);
	char buf1[20] = "Nastawa:";
	ssd1306_WriteString(strcat(buf1, set_buf), Font_7x10, White);
	y += 10;

	float duty_cl = sterowanie/10.0f;
	char duty_buf[6];
	gcvt(duty_cl, 3, duty_buf);
	ssd1306_SetCursor(2, y);
	char buf2[20] = "PWM: ";
	strcat(buf2, duty_buf);
	ssd1306_WriteString(strcat(buf2, "%"), Font_7x10, White);
	y += 10;

	ssd1306_SetCursor(2, y);
	char buf3[20] = "Dziala: ";
	if(grzanie_on_off()){
		ssd1306_WriteString(strcat(buf3, "TAK"), Font_7x10, White);
	}
	else{
		ssd1306_WriteString(strcat(buf3, "NIE"), Font_7x10, White);
	}
	y += 10;

	ssd1306_SetCursor(2, y);
		char buf6[20] = "Tryb: ";
		if(auto_manual_on_off()){
			ssd1306_WriteString(strcat(buf6, "AUTOMAT"), Font_7x10, White);
		}
		else{
			ssd1306_WriteString(strcat(buf6, "MANUAL"), Font_7x10, White);
		}
		y += 10;


	ssd1306_UpdateScreen();
}

void transmit_data(float current_temp, float set_temp){
    char data_buf[100];
    gcvt(current_temp, 6, data_buf); // convertuje float na string
    strcat(data_buf, ";"); // dodaje srednik
    //gcvt(set_temp, 6, data_buf+strlen(data_buf)); // dodaje set_temp do stringa
    gcvt(sterowanie/1.0f, 6, data_buf+strlen(data_buf));
    strcat(data_buf, "\r\n");
    HAL_UART_Transmit(&huart3, data_buf, strlen(data_buf), 100);
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim){
	if(htim->Instance == TIM2){ // If the interrupt is from timer 2 - 10Hz
		transmit_data(akutalna_temperatura, value);
		current_duty_cycle = sterowanie;
	}
	if(htim->Instance == TIM3){ // If the interrupt is from timer 3 - 2Hz
		//ssd1306_TestAll();
		HAL_UART_Receive_IT(&huart3, received_data, 4);
	}
	if(htim->Instance == TIM4){ // If the interrupt is from timer 4 - 8Hz
		MCP9808_MeasureTemperature(&akutalna_temperatura);
		if(auto_manual_on_off()==false){
			zadana_temperatura = round(zadajnik());
		}
		wyswietlacz();
	}
	if(htim->Instance == TIM12){ // If the interrupt is from timer 12 - ~83.3kHz
			if(grzanie_on_off()){
				sterowanie = pid_calculate(zadana_temperatura, akutalna_temperatura);
				wentyl();
			}
			else{
				sterowanie = 0;
				wentyl();
			}
			change_current_duty_cycle(&htim1, TIM_CHANNEL_1, sterowanie);
		}

}

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_USART3_UART_Init();
  MX_I2C2_Init();
  MX_TIM2_Init();
  MX_TIM3_Init();
  MX_TIM4_Init();
  MX_I2C4_Init();
  MX_ADC1_Init();
  MX_TIM1_Init();
  MX_TIM8_Init();
  MX_TIM12_Init();
  /* USER CODE BEGIN 2 */
  //ssd1306_TestAll();
  ssd1306_Init(); // Inicjalizacja wyświetlacza
  MCP9808_Init(&hi2c4, 0x18); // inicjalizacja sensora temperatury
  // Tutej nastawiają się dokładności czujnika temperatury :))))
  MCP9808_SetResolution(MCP9808_High_Res);  ///> High 0.125 (130 ms)
  pid_init(kp, ki, kd);  // tutaj inicjalizuje i nastawia się wartości PID

  HAL_UART_Receive_IT(&huart3, received_data, 4);
  HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);

// te niżej najlepiej jak beda na koncu // tak powiedzial szef
  HAL_TIM_Base_Start_IT(&htim2);
  HAL_TIM_Base_Start_IT(&htim3);
  HAL_TIM_Base_Start_IT(&htim4);
  HAL_TIM_Base_Start_IT(&htim12);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
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

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 216;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
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
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV4;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK)
  {
    Error_Handler();
  }
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
