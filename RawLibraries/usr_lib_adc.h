#ifndef __USR_LIB_ADC_H
#define __USR_LIB_ADC_H

#include "usr_arch.h"

#ifdef STM32_L051R8
    #include "main.h"
    #include "adc.h"
    #include "gpio.h"   
#endif

#define USR_ADC_RESOLUTION          (float)4095.0
#define ADC_CHANNEL_COUNT           (uint8_t)4
#define SAMPLE_COUNT                (uint8_t)16
#define VREF_ADD                    ((uint16_t *)((uint32_t)0x1FF80078))      // VREFINT_CAL_ADDR  stm32l051 datasheet 6.3.3.
#define TEMP_ADC_CHANNEL            0                           // adc channel 11
#define VBAT_ADC_HIGH_CHANNEL       1                           // adc channel 14
#define VBAT_ADC_LOW_CHANNEL        2                           // adc channel 15
#define VREF_ADC_CHANNEL            3                           // adc channel 17
#define VBAT_ADC_CALIBRATION_VALUE  (float)3.209000

#define _USR_ADC_INIT_FUNC()          MX_ADC_Init()
#define _USR_ADC_CHANNEL              hadc 

typedef struct S_ADC_RAW_PARAMETERS_TAG
{
    uint16_t rawTempValue;
    uint32_t rawBatteryHighValue;
    uint32_t rawBatteryLowValue;
    uint32_t rawVrefTempValue;
}S_ADC_RAW_PARAMETERS;

typedef struct S_ADC_PARAMETERS_TAG
{
    ADC_HandleTypeDef       *pAdcforDma;
}S_ADC_PARAMETERS;

typedef enum
{
    disableAdcPeripheral,
    enableAdcPeripheral,
}EAdcControl;

void UL_AdcPeripheral(EAdcControl f_eControl);  // S_ADC_PARAMETERS *f_pParameter, 
void UL_AdcCallback(void);
bool UL_AdcGetValues(S_ADC_PARAMETERS *f_pParameter ,S_ADC_RAW_PARAMETERS *f_pData);
bool UL_AdcInitial(S_ADC_PARAMETERS *f_pParameter);

#endif


