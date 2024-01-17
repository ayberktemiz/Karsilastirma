#include "usr_lib_battery.h"

#define _io static
#define _iov static volatile

_io S_BATTERY_DATA m_sBatteryParameters;
bool m_batteryPowerOkFlag = false;

#define _BATTERY_CONTROL_POWER(x)      HAL_GPIO_WritePin(m_sBatteryParameters.pBatteryPort, m_sBatteryParameters.pBatteryPin, (GPIO_PinState)x)

_io void SleepBatteryGpioPinsProc(GPIO_TypeDef *f_pGpio, uint16_t f_pinGroup, GPIO_PinState f_ePinstate);
_io void WakeUpBatteryGpioPinsProc(GPIO_TypeDef *f_pGpio, uint16_t f_pinGroup, GPIO_PinState f_ePinstate);


bool UL_BatteryInitial(S_BATTERY_DATA *f_pParameter)
{
    m_sBatteryParameters = *f_pParameter;
    _BATTERY_CONTROL_POWER((GPIO_PinState)m_sBatteryParameters.batteryPowerStatus);
    m_batteryPowerOkFlag = true;
    return true;
}


void UL_BatteryPeripheral(EBatteryControl f_eControl)
{ 
    if(f_eControl == enableBatteryPeripheral)
    {
        m_batteryPowerOkFlag = true;
        WakeUpBatteryGpioPinsProc(VBAT_ADC_ON_OFF_GPIO_Port, VBAT_ADC_ON_OFF_Pin, GPIO_PIN_SET);
    }
    else
    {
        m_batteryPowerOkFlag = false; 
        SleepBatteryGpioPinsProc(VBAT_ADC_ON_OFF_GPIO_Port, VBAT_ADC_ON_OFF_Pin, GPIO_PIN_RESET);
    }
}


_io void SleepBatteryGpioPinsProc(GPIO_TypeDef *f_pGpio, uint16_t f_pinGroup, GPIO_PinState f_ePinstate)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    
    if(m_batteryPowerOkFlag)
    {
        HAL_GPIO_WritePin(f_pGpio, f_pinGroup, f_ePinstate);
    }
    GPIO_InitStruct.Pin = f_pinGroup;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    
    HAL_GPIO_Init(f_pGpio, &GPIO_InitStruct);
}


_io void WakeUpBatteryGpioPinsProc(GPIO_TypeDef *f_pGpio, uint16_t f_pinGroup, GPIO_PinState f_ePinstate)
{ 
    GPIO_InitTypeDef GPIO_InitStruct;
    
    if(m_batteryPowerOkFlag)
    {
        HAL_GPIO_WritePin(f_pGpio, f_pinGroup, f_ePinstate);
    }
    GPIO_InitStruct.Pin = f_pinGroup;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    
    HAL_GPIO_Init(f_pGpio, &GPIO_InitStruct);  
}


