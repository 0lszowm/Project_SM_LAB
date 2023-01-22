/*
 * pid.c
 *
 *  Created on: Jan 20, 2023
 *      Author: molsz
 */


#include <pid.h>

#define ERR_SUM_MAX		2500

struct pid_params
{
	float kp;
	float ki;
	float kd;
	float err;
	float err_sum;
	float err_last;
};

static struct pid_params pid_params;

void pid_init(float kp, float ki, float kd)
{
	pid_params.kp = kp;
	pid_params.ki = ki;
	pid_params.kd = kd;
	pid_params.err = 0;
	pid_params.err_sum = 0;
	pid_params.err_last = 0;
}

uint16_t pid_calculate(float set_val, float read_val)
{
	float err_d, u;
	uint16_t u_ext;

	pid_params.err = set_val - read_val;
	pid_params.err_sum += pid_params.err;

	if (pid_params.err_sum > ERR_SUM_MAX) {
		pid_params.err_sum = ERR_SUM_MAX;
	} else if (pid_params.err_sum < -ERR_SUM_MAX) {
		pid_params.err_sum = -ERR_SUM_MAX;
	}

	err_d = pid_params.err_last - pid_params.err;
	pid_params.err_last = pid_params.err;
	u = pid_params.kp * pid_params.err + pid_params.ki * pid_params.err_sum
			+ pid_params.kd * err_d;
	u_ext = round(u);
	if(u_ext>500){
		u_ext = 500;
	}
	else if(u_ext<0){
		u_ext = 0;
	}
	return u_ext;
}
