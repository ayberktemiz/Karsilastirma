#ifndef __USR_GENERAL_H
#define __USR_GENERAL_H

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "iwdg.h"
#include "usart.h"
#include "rtc.h"
#include "tim.h"
#include "gpio.h"
#include "stm32l0xx_hal.h"

#define ACCELOMETER

#define _device_version "v5.3"

#ifdef ACCELOMETER
  #define _subversion "1.4"
  #define _accModuleCompile
#endif

#define __it_log
#define __usr_process_log
#define __usr_sleep_log
#define __usr_system_log 
#define __usr_sensor_log


#define TOPIC_BRK2MCU "ayb_C2M"
#define TOPIC_MCU2BRK "ayb_M2C"

#include "usr_arch.h"
#include "usr_lib_accel.h"
#include "usr_lib_adc.h"
#include "usr_lib_led.h"
#include "usr_lib_rtc.h"
#include "usr_lib_sensor.h"
#include "usr_lib_ntc.h"
#include "usr_lib_gsm.h"
#include "usr_lib_battery.h"
#include "usr_lib_halleffect.h"
#include "usr_lib_log.h"

#include "usr_system.h"
#include "usr_nvs.h"
#include "usr_sleep.h"
#include "usr_sensor.h"
#include "usr_process.h"

#endif