#ifndef __USR_SENSOR_H
#define __USR_SENSOR_H

#include "usr_general.h"

#define _USR_SENSOR_DISTANCE_ERROR_VALUE             (int)-100                               

typedef struct S_SENSOR_ALL_VALUES_TAG
{
    int rtc;

    float tempValue;

    float batteryVoltage;
    uint8_t batteryVoltagePercentage;

    int32_t distanceValue;

    uint8_t halleffectAlarmStatus; // hall effect alarm status, battery cover and top cover
    //uint8_t alarmEventGroup;       // ilk bit fullness alarm group, ikinci bit full alarm limit   // BUNU silmedim kapattim sadece simdilik, eren abi sen sil.

    bool sendDataFlag; //// to show send gsm data ok flag
} S_SENSOR_ALL_VALUES;

void UsrSensorGetValues(void);
void UsrSensorGetAdcAndHalleffectValues(void);
void UsrSensorGetDistance(void);
void UsrSensorGetHalleffectStatusDirectly(void);
void UsrSensorHallEffectPinStatus(void);
void UsrSensorHallEffectGivePower(void);

extern bool g_sensorsReadingFlag;
extern uint32_t g_sensorReadTs;

extern S_ADC_PARAMETERS               g_sAdcParameters;
extern S_ADC_RAW_PARAMETERS           g_sAdcRawParameters;
extern S_BATTERY_DATA                 g_sBatteryParameters;
extern S_NTC_PARAMETERS               g_sNtcParameters;
extern S_HALLEFFECT_PARAMETERS        g_sHalleffectParameters;
extern S_ULTRASONIC_SENSOR_PARAMETERS g_sUltrasonicParameters;
// extern S_TIMEDATE                     g_sTimeParameters;

extern S_SENSOR_ALL_VALUES            g_sAllSensorValues;

#endif
