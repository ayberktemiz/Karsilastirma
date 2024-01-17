#ifndef __USR_PROCESS_H
#define __USR_PROCESS_H

#include "usr_general.h"

#define _USR_RESPONSE_TIMEOUT         (uint32_t)(5 * 60 * 1000)  // 30 sanyeye ve ardindan 5dk 
#define _SIM_POWER_PIN(x)             ((x) ? (SIM_DETECT_POWER_GPIO_Port->BSRR = SIM_DETECT_POWER_Pin) : (SIM_DETECT_POWER_GPIO_Port->BRR = SIM_DETECT_POWER_Pin)) 
#define _SIM_DETECT_READ_PIN()        (SIM_DETECT_GPIO_Port->IDR & SIM_DETECT_Pin)
#define _SIM_POWER_PIN_2()            SIM_DETECT_POWER_GPIO_Port->ODR |= SIM_DETECT_POWER_Pin

void UsrProcess(void); 
void UsrProcessDecideFirstState(void);
void UsrProcessLedOpenAnimation(void);

extern S_SENSOR_ALL_VALUES              g_sAllSensorValues;
extern S_HALLEFFECT_PARAMETERS          g_sHalleffectParameters;
extern S_LED_PARAMETERS                 g_sLedParameters;
extern S_GSM_PARAMETERS                 g_sGsmParameters;
extern S_GSM_MODULE_INFO                g_sGsmModuleInfo;
extern S_GSM_MQTT_CONNECTION_PARAMETERS g_sGsmMqttInitialParameters;
// extern S_GSM_FTP                        g_sGsmFtpParameters;
extern S_DEVICE_NVS_INFO                   g_sNvsDeviceInfo;


typedef struct S_GET_DATA_TAG
{
    uint32_t ts;
    uint32_t interval;
    uint16_t fullAlarmLimit;
    char version[8];    // 16 idi 8 yapildi
    uint8_t deviceStatus;  
    uint32_t deviceStatusCheckTime;
    char link[100];
    uint16_t depthAlarmLimit;
    uint16_t fullnessAlarmLimit;
    uint16_t toleranceValue;
    uint16_t sensorWakeUp;
     
}S_GET_DATA;

extern S_GET_DATA g_sGetData;

//extern char response[];               // 400
extern char parsedStringValueBuf[];   // 50
extern char parsedIntValueBuf[];      // 12
extern char parsedFloatValueBuf[];    // 12
extern int integerResponseValue;
extern float floatResponseValue;
extern bool responseSubcribeDataCallbackFlag;
extern uint16_t g_subcribeDataCallbackCounter;
extern uint32_t g_waitResponseCount;



#endif
