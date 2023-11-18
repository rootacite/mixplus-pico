
#ifndef __H_MIXPLUS

#define __H_MIXPLUS
#define MX_BASIC_HARDWARE

#ifdef MX_BASIC_HARDWARE

#include "timer.hpp"
#include "dma.hpp"
#include "spi.hpp"
#include "gpio.hpp"
#include "pio.hpp"
#include "adc.hpp"
#include "iic.hpp"
#include "pwm.hpp"
#include "task.hpp"

#endif

#ifdef MX_ENABLE_ST7735
#include "st7735.hpp"
#endif

#ifdef MX_ENABLE_MPU6050
#include "mpu6050.hpp"
#endif

#ifdef MX_ENABLE_SD
#include "sd.hpp"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void sys_test();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif