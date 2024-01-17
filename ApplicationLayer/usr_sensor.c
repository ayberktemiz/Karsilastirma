#include "usr_general.h"

#define _USR_SENSOR_DISTANCE_SAMPLE 16
//#define _USR_SENSOR_NTC_SAMPLE      16
//#define _USR_SENSOR_BATTERY_SAMPLE  16

S_SENSOR_ALL_VALUES  g_sAllSensorValues;

#define _USR_SENSOR_DISTANCE_SENSOR_ON_OFF(x) HAL_GPIO_WritePin(g_sUltrasonicParameters.pDistanceSensorOnOffPort, g_sUltrasonicParameters.sensorOnOffPin, (GPIO_PinState)x)
#define _USR_SENSOR_NTC_SENSOR_ON_OFF(x)      HAL_GPIO_WritePin(g_sNtcParameters.pNtcPort,                        g_sNtcParameters.pNtcPin,               (GPIO_PinState)x)
#define _USR_SENSOR_BATTERY_SENSOR_ON_OFF(x)  HAL_GPIO_WritePin(g_sBatteryParameters.pBatteryPort,                g_sBatteryParameters.pBatteryPin,       (GPIO_PinState)x)

_io float CalculateBatteryVoltageProc(void);
_io uint8_t CalculateBatteryVoltagePercentageProc(float f_BatteryValue);
_io int CalculateDistanceSensorValueProc(void);
//_io float CalculateNtcTempValueProc(void);
_io void AdcSensorsInitialProc(void);
_io void AdcSensorsDisableProc(void);

bool g_sensorsReadingFlag;
uint32_t g_sensorReadTs;


void UsrSensorGetValues(void)
{
    if (g_sensorsReadingFlag)
    {
        UsrSensorGetAdcAndHalleffectValues();     // Adc values and Hall Effect
        UsrSensorGetDistance();                   // Distance open and close process

        g_sensorReadTs = UL_RtcGetTs();
        g_sensorsReadingFlag = false;           

        #ifdef __usr_sensor_log
            __logsi("Battery and Distance Sensor Reads OK! --> ts: %d, Charge: %.3f, Distance: %d\n", g_sensorReadTs, g_sAllSensorValues.batteryVoltage, g_sAllSensorValues.distanceValue);
        #endif
   }
}


void UsrSensorGetAdcAndHalleffectValues(void)
{
    UsrSystemWatchdogRefresh();
    UsrSensorGetHalleffectStatusDirectly();   // burada halleffect disable oluyor
    AdcSensorsInitialProc();                  // adc sensorlerin enable olma durumu
    HAL_Delay(250);                           // sorun çözen bir delay 

    for (uint8_t i = 0; i < 5; i++)
    {
        if (UL_AdcGetValues(&g_sAdcParameters, &g_sAdcRawParameters))
            break;
        else
            HAL_Delay(100); 
    }
    
    g_sAllSensorValues.tempValue = UL_NtcGetValue(g_sAdcRawParameters.rawTempValue);         // aradaki fonksiyonu yok ettim, direkt koydum bunu buraya
    g_sAllSensorValues.batteryVoltage = CalculateBatteryVoltageProc();
    g_sAllSensorValues.batteryVoltagePercentage = CalculateBatteryVoltagePercentageProc(g_sAllSensorValues.batteryVoltage);

    AdcSensorsDisableProc();
}


void UsrSensorGetDistance(void)
{
    UsrSystemWatchdogRefresh();
    UL_UltrasonicSensorPeripheral(enableUltrasonicSensor);

    for (uint8_t i = 0; i < 5; i++)
    {
        g_sAllSensorValues.distanceValue = CalculateDistanceSensorValueProc();
        if (g_sAllSensorValues.distanceValue != _USR_SENSOR_DISTANCE_ERROR_VALUE)                    
            break;
        else
            HAL_Delay(100);
    }
    UL_UltrasonicSensorPeripheral(disableUltrasonicSensor);
}


void UsrSensorGetHalleffectStatusDirectly(void)
{
    UsrSensorHallEffectGivePower();
    HAL_Delay(50);  
    UsrSensorHallEffectPinStatus();
}


void UsrSensorHallEffectGivePower(void)
{
    UsrSleepGpioInputPins(BATTERY_COVER_HALL_SWITCH_OUT_INT_GPIO_Port, BATTERY_COVER_HALL_SWITCH_OUT_INT_Pin | TOP_COVER_HALL_SWITCH_OUT_INT_Pin);
    UL_HalleffectPeripheral(enableHalleffectPeripheral);                                                                         
}


void UsrSensorHallEffectPinStatus(void)
{
    g_sAllSensorValues.halleffectAlarmStatus = 0;
    if (_TOP_COVER_HALL_READ_PIN())
    {
        g_sAllSensorValues.halleffectAlarmStatus |= 0x01;
    }
    else
    {
        g_sAllSensorValues.halleffectAlarmStatus &= (~0x01);
    }

    if (_BATTERY_COVER_HALL_READ_PIN())
    {
        g_sAllSensorValues.halleffectAlarmStatus |= 0x02;
    }
    else
    {
        g_sAllSensorValues.halleffectAlarmStatus &= (~0x02);
    }

    UL_HalleffectPeripheral(disableHalleffectPeripheral);
    UsrSleepGpioOutPins(BATTERY_COVER_HALL_SWITCH_OUT_INT_GPIO_Port, BATTERY_COVER_HALL_SWITCH_OUT_INT_Pin | TOP_COVER_HALL_SWITCH_OUT_INT_Pin, GPIO_PIN_RESET);

    // Her okuma sonrasi kapatildi ayaklar
    // kapak acikken (miknatis yokken) 1 cikisi veriyor, kapak kapaliyken (miknatsi goruyorsa) 0 cikisi veriyor.
    // halleffectAlarmStatus'te sagdaki bit U8 (TOP_COVER), Soldaki bit U9 (BATTERY_COVER)'u temsil ediyor.
}


_io void AdcSensorsDisableProc(void)
{
    UL_AdcPeripheral(disableAdcPeripheral);
    UL_BatteryPeripheral(disableBatteryPeripheral);                 //0.6mA
    UL_NtcPeripheral(disableNtcPeripheral);                         //0.17mA
}


_io void AdcSensorsInitialProc(void)
{
    UL_AdcPeripheral(enableAdcPeripheral);
    UL_BatteryPeripheral(enableBatteryPeripheral);
    UL_NtcPeripheral(enableNtcPeripheral);
}

/*
_io float CalculateBatteryVoltageProc(void)
{
    float m_BatterySum = 0;

    for (uint8_t i = 0; i < _USR_SENSOR_BATTERY_SAMPLE; i++)
    {
        float factor = (3 * (*(VREF_ADD))) / ((float)g_sAdcRawParameters.rawVrefTempValue);
        uint32_t rawBatteryValue = g_sAdcRawParameters.rawBatteryHighValue - g_sAdcRawParameters.rawBatteryLowValue;
        float m_batteryVoltage = ((factor * (float)rawBatteryValue) / USR_ADC_RESOLUTION) * VBAT_ADC_CALIBRATION_VALUE;

        m_BatterySum += m_batteryVoltage;
    }
    return (m_BatterySum / _USR_SENSOR_BATTERY_SAMPLE);
}
*/

_io float CalculateBatteryVoltageProc(void)
{
    float factor = (3 * (*(VREF_ADD))) / ((float)g_sAdcRawParameters.rawVrefTempValue);
    uint32_t rawBatteryValue = g_sAdcRawParameters.rawBatteryHighValue - g_sAdcRawParameters.rawBatteryLowValue;
    float m_batteryVoltage = ((factor * (float)rawBatteryValue) / USR_ADC_RESOLUTION) * VBAT_ADC_CALIBRATION_VALUE;

    return m_batteryVoltage;
}

_io int CalculateDistanceSensorValueProc(void)
{
    uint8_t sampleCount = 0;
    int m_UltrasonicRawDistance = 0;
    int m_DistanceSum = 0;

    for (uint8_t i = 0; i < _USR_SENSOR_DISTANCE_SAMPLE; i++) 
    {
        m_UltrasonicRawDistance = UL_UltrasonicSensorGetValue(100);
        HAL_Delay(1);                                            
        if (m_UltrasonicRawDistance != _USR_SENSOR_DISTANCE_ERROR_VALUE)
        {
            sampleCount++;
            m_DistanceSum += m_UltrasonicRawDistance;
        }
    }

    if (sampleCount <= (_USR_SENSOR_DISTANCE_SAMPLE / 2))
        return _USR_SENSOR_DISTANCE_ERROR_VALUE;      
    else
        return (m_DistanceSum / sampleCount);
}

/*
_io float CalculateNtcTempValueProc(void)
{
    float m_NtcSum = 0;
    float m_NtcRawTemp = 0;

    for (uint8_t i = 0; i < _USR_SENSOR_NTC_SAMPLE; i++)
    {
        m_NtcRawTemp = UL_NtcGetValue(g_sAdcRawParameters.rawTempValue);
        m_NtcSum += m_NtcRawTemp;
    }
    return (m_NtcSum / _USR_SENSOR_NTC_SAMPLE);
}
*/

_io uint8_t CalculateBatteryVoltagePercentageProc(float f_BatteryValue)
{
    uint8_t batteryPercentage = 0;

    if (f_BatteryValue >= 6.95)
    {
        batteryPercentage = 100;
    }
    else if (f_BatteryValue >= 6.85)
    {
        batteryPercentage = 60;
    }
    else if (f_BatteryValue >= 6.70)
    {
        batteryPercentage = 50;
    }
    else if (f_BatteryValue >= 6.50)
    {
        batteryPercentage = 30;
    }
    else if (f_BatteryValue >= 6.30)
    {
        batteryPercentage = 10;
    }
    else
    {
        batteryPercentage = 0;
    }
    return batteryPercentage;
}
