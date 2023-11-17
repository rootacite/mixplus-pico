
#ifndef __H_MIXPLUS

#define __H_MIXPLUS

#include "timer.hpp"
#include "dma.hpp"
#include "spi.hpp"
#include "gpio.hpp"
#include "pio.hpp"
#include "adc.hpp"
#include "iic.hpp"

#include "st7735.hpp"
#include "mpu6050.hpp"

#include "sd.hpp"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void sys_test();

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif