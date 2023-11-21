
#ifndef __H_MIXPLUS

#define __H_MIXPLUS
#define MX_BASIC_HARDWARE

#ifdef MX_BASIC_HARDWARE

#include "hardware/timer.hpp"
#include "hardware/dma.hpp"
#include "hardware/spi.hpp"
#include "hardware/gpio.hpp"
#include "hardware/pio.hpp"
#include "hardware/adc.hpp"
#include "hardware/iic.hpp"
#include "hardware/pwm.hpp"
#include "task.hpp"

#endif

#ifdef MX_ENABLE_ST7735
#include "devices/st7735.hpp"
#endif

#ifdef MX_ENABLE_MPU6050
#include "devices/mpu6050.hpp"
#include "devices/gy273.hpp"
#endif

#ifdef MX_ENABLE_SD
#include "devices/sd.hpp"
#endif

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void sys_test();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif