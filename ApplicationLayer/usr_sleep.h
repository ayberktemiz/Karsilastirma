#ifndef __USR_SLEEP_H
#define __USR_SLEEP_H

#include "usr_general.h"

#define _USR_SLEEP_SYSTEM_TIMEOUT_TIME             (uint32_t)300000

extern S_GSM_PARAMETERS                 g_sGsmParameters;
extern S_GSM_MODULE_INFO                g_sGsmModuleInfo;
extern S_GSM_MQTT_CONNECTION_PARAMETERS g_sGsmMqttInitialParameters;

extern S_ADC_PARAMETERS                 g_sAdcParameters;
extern S_ADC_RAW_PARAMETERS             g_sAdcRawParameters;
extern S_HALLEFFECT_PARAMETERS          g_sHalleffectParameters;
extern S_ULTRASONIC_SENSOR_PARAMETERS   g_sUltrasonicParameters;

typedef enum 
{
    disableTimer,
    enableTimer,
}ETimerControl;

void UsrSleepGeneral(void);
void UsrSleepEnterSubSleep(uint32_t f_time);

void UsrSleepAdcPins(GPIO_TypeDef *f_pGpio, uint16_t f_pinGroup, GPIO_PinState f_ePinstate);
void UsrSleepGpioOutPins(GPIO_TypeDef *f_pGpio, uint16_t f_pinGroup, GPIO_PinState f_ePinstate);
void UsrSleepGpioInputPins(GPIO_TypeDef *f_pGpio, uint16_t f_pinGroup);

extern bool g_sleepFlag;  // biraz daha dursun

#endif

