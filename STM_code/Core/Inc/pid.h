/*
 * pid.h
 *
 *  Created on: Jan 20, 2023
 *      Author: molsz
 */

#ifndef PID_H_
#define PID_H_

//#include "platform_specific.h"
#include <math.h>
#include <stdint.h>

void pid_init(float kp, float ki, float kd);

uint16_t pid_calculate(float set_val, float read_val);

#endif /* PID_H_ */
