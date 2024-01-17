#ifndef _USR_LIB_NTC_H
#define _USR_LIB_NTC_H

#include "usr_arch.h"

#ifdef STM32_L051R8
    #include "main.h"
    #include "adc.h"
    #include "gpio.h"
#endif

#define _NTC_SERIAL_RES       (uint32_t)100000
#define _NTC_BOTTOM_RES_VALUE (uint32_t)6120926
#define _NTC_TOP_RES_VALUE    (uint32_t)1278
#define _MEMBER_SIZE_OF_NTC   (uint8_t)39
#define _USR_ADC_RESOLUTION   (float)4095.0

typedef struct S_NTC_PARAMETRS_TAG
{
    GPIO_TypeDef *pNtcPort;
    int           pNtcPin;
    uint8_t       ntcStatus;
}S_NTC_PARAMETERS;

typedef enum
{
    disableNtcPeripheral,
    enableNtcPeripheral,
}EControl;

void UL_NtcPeripheral(EControl f_eControl);
bool UL_NtcInitial(S_NTC_PARAMETERS *f_pParameter);

float UL_NtcGetValue(uint32_t f_rawValue);

#endif