/*
 * mcp9808.h
 *
 *  Created on: Jan 14, 2023
 *      Author: 0lszowm
 *

 ******************************
 * 	Supports:
 * 	- Read temperature
 * 	- Set resolution
 * 	- Read resolution
 ******************************
 */

#ifndef MCP9808_H_
#define MCP9808_H_

#include "main.h"

/**
 * Various other registers.
 * Mainly 0x05 for reading temperature and
 * 0x08 for setting read resolution.
 */
typedef enum {
	MCP9808_CONFIG_REG = 0x01,
	MCP9808_T_AMBIENT_REG = 0x05,
	MCP9808_MANUFACTURER_REG = 0x06,
	MCP9808_DEVICE_ID_REG = 0x07,
	MCP9808_RESOLUTION_REG = 0x08
} MCP9808_REG_TypeDef;

/**
 * Temperature read resolution values.
 */
typedef enum {
	MCP9808_Low_Res = 0x00, ///> Lowest +0.5 (Fastest 30 ms)
	MCP9808_Medium_Res = 0x01, ///> Medium +0.25 (65 ms)
	MCP9808_High_Res = 0x02, ///> High 0.125 (130 ms)
	MCP9808_VeryHigh_Res = 0x03 ///> Highest 0.0625 (Slowest 250 ms)
} MCP9808_Resolution_TypeDef;

/**
 * Struct to hold an instance of MCP9808.
 */
typedef struct {
		I2C_HandleTypeDef *hi2c;
		uint8_t address; ///> Default I2C address is 0x18
		MCP9808_Resolution_TypeDef resolution;
} MCP9808_TypeDef;

/**
 * Method defintiions.
 */
void MCP9808_Init(I2C_HandleTypeDef *hi2c, uint8_t addr);
HAL_StatusTypeDef MCP9808_MeasureTemperature(float *temperature);
HAL_StatusTypeDef MCP9808_SetResolution(MCP9808_Resolution_TypeDef resolution);
HAL_StatusTypeDef MCP9808_GetResolution();




#endif /* MCP9808_H_ */
