################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Components/Src/ssd1306.c \
../Components/Src/ssd1306_fonts.c \
../Components/Src/ssd1306_tests.c 

OBJS += \
./Components/Src/ssd1306.o \
./Components/Src/ssd1306_fonts.o \
./Components/Src/ssd1306_tests.o 

C_DEPS += \
./Components/Src/ssd1306.d \
./Components/Src/ssd1306_fonts.d \
./Components/Src/ssd1306_tests.d 


# Each subdirectory must supply rules for building sources it contributes
Components/Src/%.o Components/Src/%.su: ../Components/Src/%.c Components/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m7 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F746xx -c -I../Core/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc -I../Drivers/STM32F7xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F7xx/Include -I../Drivers/CMSIS/Include -I"D:/Project_SM_LAB/Project_SM_LAB/STM_code/Components/Inc" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv5-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Components-2f-Src

clean-Components-2f-Src:
	-$(RM) ./Components/Src/ssd1306.d ./Components/Src/ssd1306.o ./Components/Src/ssd1306.su ./Components/Src/ssd1306_fonts.d ./Components/Src/ssd1306_fonts.o ./Components/Src/ssd1306_fonts.su ./Components/Src/ssd1306_tests.d ./Components/Src/ssd1306_tests.o ./Components/Src/ssd1306_tests.su

.PHONY: clean-Components-2f-Src

