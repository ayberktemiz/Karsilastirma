#ifndef __USR_SYSTEM_H
#define __USR_SYSTEM_H

#include "usr_general.h"

#define _USR_SYSTEM_BASE_TIMER_CHANNEL   htim6
#define _USR_SYSTEM_LOG_UART_CHANNEL     hlpuart1
#define _USR_SYSTEM_ADC_CHANNEL          hadc
#define _USR_SYSTEM_UART_1_CHANNEL       huart1
#define _USR_SYSTEM_UART_2_CHANNEL       huart2
#define _USR_SYSTEM_DAILY_RESET_TIME     (24 * 60 * 60 * 1000)

#define _USR_SYSTEM_EVENT_BITS_DEVICE_RESET        (1 << 0)
#define _USR_SYSTEM_EVENT_BITS_COVERS_ALARM        (1 << 1)
#define _USR_SYSTEM_EVENT_BITS_FIRE_ALARM          (1 << 2)
#define _USR_SYSTEM_EVENT_BITS_FULL_ALARM          (1 << 3)
#define _USR_SYSTEM_EVENT_BITS_EMPTIED_ALARM       (1 << 4)
#define _USR_SYSTEM_EVENT_BITS_PERIODIC_DATA_SEND  (1 << 5)
#ifdef _accModuleCompile
#define _USR_SYSTEM_EVENT_BITS_ACC_SHAKE_ALARM     (1 << 6)
#define _USR_SYSTEM_EVENT_BITS_ACC_COMMUNICATION_ERROR  (1 << 7)
#endif

void UsrSystemInitial(void); 
void UsrSystemGeneral(void);

void UsrSystemHardFault(void);
void UsrSystemUpdateTsValues(void);
void UsrSystemWatchdogRefresh(void);

extern uint32_t g_dataSendTs;
extern bool g_fireAlarmFlag;
extern uint32_t g_dailyResetTimer;
extern uint32_t g_packageEventBits;
extern uint32_t g_waitResponseCount;

extern uint16_t g_subcribeDataCallbackCounter;

#endif //__USR_SYSTEM_H

/*  NOTLAR:
// HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);    // Dc Dc power on off pin
// HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12, GPIO_PIN_SET);   // PowerKey control Pin
// HAL_GPIO_WritePin(GPIOB, GPIO_PIN_14, GPIO_PIN_SET);   // Gsm Process Status mcu
// HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0,  GPIO_PIN_SET);   // Ntc Active Pin 
// HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_SET);    // Distance Sensor On off pin
// HAL_GPIO_WritePin(GPIOB, GPIO_PIN_13, GPIO_PIN_SET);   // Gprs power on off 
// HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6,  GPIO_PIN_SET);   // Sim Detect Power
// HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);    // Rgb led red pwm pin 
// HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);    // Rgb led green pwm 
// HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);    // Rgb led blue pwm pin
// HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);   // Vbat adc on off pin
*/


