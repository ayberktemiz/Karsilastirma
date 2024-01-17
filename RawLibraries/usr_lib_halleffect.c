#include "usr_lib_halleffect.h"

#define HALL_EFFECT_DELAY_MS    50

_io S_HALLEFFECT_PARAMETERS m_sHalleffectParameter; 

bool UL_HalleffectInitial(S_HALLEFFECT_PARAMETERS *f_pParameter)
{
    m_sHalleffectParameter = *f_pParameter;
    HAL_Delay(HALL_EFFECT_DELAY_MS);
    HAL_GPIO_WritePin(m_sHalleffectParameter.batteryHalleffectPowerPort, m_sHalleffectParameter.batteryHalleffectPowerPin, (GPIO_PinState)m_sHalleffectParameter.batteryHalleffectStatus);
    HAL_GPIO_WritePin(m_sHalleffectParameter.topHalleffectPowerPort    , m_sHalleffectParameter.topHalleffectPowerPin    , (GPIO_PinState)m_sHalleffectParameter.topHalleffectStatus);

    // _BATTERY_COVER_HALL_POWER(1);
    // _TOP_COVER_HALL_POWER(1);
    return true;
}


void UL_HalleffectPeripheral(EHalleffectControl f_eControl)
{
    if(f_eControl == enableHalleffectPeripheral)
    {
        HAL_Delay(HALL_EFFECT_DELAY_MS);
        _BATTERY_COVER_HALL_POWER(1);
        _TOP_COVER_HALL_POWER(1);
    }
    else
    {
        HAL_Delay(HALL_EFFECT_DELAY_MS);
        _BATTERY_COVER_HALL_POWER(0);
        _TOP_COVER_HALL_POWER(0);
    }
}

