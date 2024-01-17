#ifndef __USR_LIB_BATTERY_H
#define __USR_LIB_BATTERY_H

#include "usr_arch.h"

#ifdef STM32_L051R8
    #include "main.h"
    #include "gpio.h"
#endif

typedef enum
{
    enableBatteryPeripheral,
    disableBatteryPeripheral,
}EBatteryControl;

typedef struct S_BATTERY_DATA_TAG
{
    GPIO_TypeDef   *pBatteryPort;
    int             pBatteryPin;
    float           batteryRawValue;
    uint8_t         batteryPowerStatus;
}S_BATTERY_DATA;

void UL_BatteryPeripheral(EBatteryControl f_eControl);
bool UL_BatteryInitial(S_BATTERY_DATA *f_pParameter);

#endif

