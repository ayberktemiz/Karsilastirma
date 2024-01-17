#include "usr_general.h"

S_DEVICE_NVS_INFO g_sNvsDeviceInfo;

_io void CheckFactorySettingProc(void);
_io void WriteDataToEepromProc(uint32_t f_add, uint8_t *f_p, uint16_t f_len);
_io void ReadDataToEepromProc(uint32_t f_add, uint8_t *f_pBuf, uint16_t f_len);
_io uint8_t CalculateCrcProc(uint8_t *f_p, uint16_t f_len);

uint8_t crcRes = 0;

void UsrNvsInitial(void)
{
    CheckFactorySettingProc();
}

void UsrNvsUpdate(void)
{
    S_DEVICE_NVS_INFO sDeviceInfo;
    uint8_t i = 0;

    g_sNvsDeviceInfo.crc = CalculateCrcProc((uint8_t *)&g_sNvsDeviceInfo, sizeof(g_sNvsDeviceInfo) - 1);
    for (i = 0; i < 5; i++)
    {
        WriteDataToEepromProc(_USR_NVS_START_ADD, (uint8_t *)&g_sNvsDeviceInfo, sizeof(g_sNvsDeviceInfo));
        ReadDataToEepromProc(_USR_NVS_START_ADD, (uint8_t *)&sDeviceInfo, sizeof(sDeviceInfo));

        if (g_sNvsDeviceInfo.crc == sDeviceInfo.crc)
            break;
    }

    if (i == 5)
        UsrSystemHardFault();
}


_io void CheckFactorySettingProc(void)
{
    ReadDataToEepromProc(_USR_NVS_START_ADD, (uint8_t *)&g_sNvsDeviceInfo, sizeof(g_sNvsDeviceInfo));
    
    crcRes = CalculateCrcProc((uint8_t *)&g_sNvsDeviceInfo, sizeof(g_sNvsDeviceInfo) - 1);

    if ((g_sNvsDeviceInfo.eepromCheckValue != _USR_NVS_CHECKDATA) || (CalculateCrcProc((uint8_t *)&g_sNvsDeviceInfo, sizeof(g_sNvsDeviceInfo) - 1) != g_sNvsDeviceInfo.crc))
    {
        g_sNvsDeviceInfo.eepromCheckValue      = _USR_NVS_CHECKDATA;
        g_sNvsDeviceInfo.sendingDataInterval   = (uint32_t) 60*2;    // seconds
        g_sNvsDeviceInfo.fullAlarmLimit        = 70;                 // veya 50 cm
        g_sNvsDeviceInfo.depthAlarmLimit       = 500;                // cm
        g_sNvsDeviceInfo.toleranceValue        = 10;                 // percentage
        g_sNvsDeviceInfo.fullnessAlarmLimit    = 100;                // cm
        g_sNvsDeviceInfo.enterFireAlarmValue   = (float)5;           // santigrat  deltaTemperature
        g_sNvsDeviceInfo.exitFireAlarmValue    = (float)3;           // santigrat
        g_sNvsDeviceInfo.sensorWakeUpTime      = (uint32_t) 60;      // seconds
        g_sNvsDeviceInfo.accReadingTime        = (uint32_t)60;       // seconds
        g_sNvsDeviceInfo.crc                   = 0;
        g_sNvsDeviceInfo.deviceStatusCheckTime = (uint32_t)60 * 1;   // seconds
        g_sNvsDeviceInfo.deviceStatus          = 1;                  // 0                 

        g_sNvsDeviceInfo.accParamsBuf[0] = 0x90;
        g_sNvsDeviceInfo.accParamsBuf[1] = 0x5F;
        g_sNvsDeviceInfo.accParamsBuf[2] = 0x00;
        g_sNvsDeviceInfo.accParamsBuf[3] = 0x40;
        g_sNvsDeviceInfo.accParamsBuf[4] = 0x30;
        g_sNvsDeviceInfo.accParamsBuf[5] = 0x00;
        g_sNvsDeviceInfo.accParamsBuf[6] = 0x00;
        g_sNvsDeviceInfo.accParamsBuf[7] = 0x7F;
        g_sNvsDeviceInfo.accParamsBuf[8] = 0x00;
        g_sNvsDeviceInfo.accParamsBuf[9] = 0x14;
        g_sNvsDeviceInfo.accParamsBuf[10] = 0x02;

        /*
            .....
        */
        UsrNvsUpdate();
    }

    if (g_sNvsDeviceInfo.sensorWakeUpTime > (60 * 15))
        g_sNvsDeviceInfo.sensorWakeUpTime = 60 * 15; // 15 dakikadan fazla olmasın
    /* ........  */
}

_io void WriteDataToEepromProc(uint32_t f_add, uint8_t *f_p, uint16_t f_len)
{
    HAL_FLASHEx_DATAEEPROM_Unlock();
    for (uint16_t i = 0; i < f_len; i++)
    {
        for (uint8_t m = 0; m < 5; m++)
        {
            HAL_FLASHEx_DATAEEPROM_Program(TYPEPROGRAMDATA_BYTE, f_add + i, f_p[i]);
            if (*((uint8_t *)(f_add + i)) == f_p[i])
            {
                break;
            }
        }
    }
    HAL_FLASHEx_DATAEEPROM_Lock();
}

_io void ReadDataToEepromProc(uint32_t f_add, uint8_t *f_pBuf, uint16_t f_len)
{
    memcpy((void *)f_pBuf, (const void *)f_add, f_len);
}

_io uint8_t CalculateCrcProc(uint8_t *f_p, uint16_t f_len)
{
    uint8_t crc = 0;
    uint16_t len = 0;
    while (len < f_len)
    {
        uint8_t extract = f_p[len++];
        for (uint8_t i = 8; i; i--)
        {
            uint8_t sum = (crc ^ extract) & 0x01;
            crc >>= 1;
            if (sum)
            {
                crc ^= 0x8C;
            }
            extract >>= 1;
        }
    }
    return crc;
}
