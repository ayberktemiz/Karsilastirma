#ifndef __USR_NVS_H
#define __USR_NVS_H

#include "usr_general.h"

#define _USR_NVS_START_ADD (uint32_t)0x08080000
#define _USR_NVS_CHECKDATA (uint32_t)0xaabbccdd

typedef struct S_DEVICE_NVS_INFO_TAG
{
    uint32_t eepromCheckValue;
    uint16_t sendingDataInterval; // sending data interval in minute
    uint16_t fullAlarmLimit;      // destination alarm limit, in cm(under this value meaning is alarm condition)
    uint16_t depthAlarmLimit;     // in cm
    uint8_t toleranceValue;       // percentage
    uint16_t fullnessAlarmLimit;  // in cm
    uint8_t enterFireAlarmValue;  // in santigrat
    uint8_t exitFireAlarmValue;   // in santigrat

    uint16_t sensorWakeUpTime;    // in second
    uint16_t accReadingTime;
    uint8_t crc;
    uint32_t deviceStatusCheckTime; // in minute
    uint8_t deviceStatus;
    uint8_t accParamsBuf[11];
} S_DEVICE_NVS_INFO;

typedef struct S_DEVICE_NVS_BOOT_DATA_TAG
{
    uint32_t checkRegionValue; // _USR_EEPROM_CHECK_VALUE
    uint8_t versionBuf[12];    // vx.x
    /* 0-> do nothing    1-> get firmware from m66 */
    uint8_t process;
    uint8_t dummyBuf[2];
    uint8_t crc;
} S_DEVICE_NVS_BOOT_DATA;

void UsrNvsInitial(void);
void UsrNvsUpdate(void);
// void UsrNvsBootUpdate(void);

extern S_DEVICE_NVS_INFO g_sNvsDeviceInfo;

#endif