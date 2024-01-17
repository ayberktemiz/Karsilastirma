#include "usr_lib_ntc.h"

#define _io static
#define _iov static volatile

_io S_NTC_PARAMETERS  m_sNtcParameters;
bool m_ntcPowerOkFlag = false;

_io void SleepNtcSensorGpioPinsProc(GPIO_TypeDef *f_pGpio, uint16_t f_pinGroup, GPIO_PinState f_ePinstate);
_io void WakeUpNtcSensorGpioPinsProc(GPIO_TypeDef *f_pGpio, uint16_t f_pinGroup, GPIO_PinState f_ePinstate);

#define _NTC_POWER(x)     HAL_GPIO_WritePin(m_sNtcParameters.pNtcPort, m_sNtcParameters.pNtcPin, (GPIO_PinState)x)

const float m_ntcTable1Buf[40][2] = {
    {-40, 6120926.223}, //// @ -40
    {-35, 4118517.102}, //// @ -35
    {-30, 2816706.28},  //// @ -30
    {-25, 1956101.683}, //// @ -25
    {-20, 1378149.975}, //// @ -20
    {-15, 984222.4427}, //// @ -15
    {-10, 711944.165},  //// @ -10
    {-5, 521247.1641},  //// @ -5
    {0, 386009.9863},   //// @ 0
    {5, 288963.5853},   //// @ 5
    {10, 218539.0216},  //// @ 10
    {15, 166887.948},   //// @	15
    {20, 128622.1114},  //// @ 20
    {25, 100000},       //// @ 25
    {30, 78395.35969},  //// @ 30
    {35, 61945.7014},   //// @ 35
    {40, 49317.16834},  //// @ 40
    {45, 39545.51541},  //// @ 45
    {50, 31927.4356},   //// @ 50
    {55, 25945.54926},  //// @ 55
    {60, 21216.1327},   //// @ 60
    {65, 17452.35867},  //// @ 65
    {70, 14438.21854},  //// @ 70
    {75, 12009.86651},  //// @ 75
    {80, 10042.16562},  //// @ 80
    {85, 8438.910153},  //// @ 85
    {90, 7125.666732},  //// @ 90
    {95, 6044.495457},  //// @ 95
    {100, 5150.030465}, //// @ 100
    {105, 4406.550634}, //// @ 105
    {110, 3785.776329}, //// @ 110
    {115, 3265.202078}, //// @ 115
    {120, 2826.827341}, //// @ 120
    {125, 2456.184829}, //// @ 125
    {130, 2141.592509}, //// @ 130
    {135, 1873.574746}, //// @ 135
    {140, 1644.412049}, //// @ 140
    {145, 1447.789097}, //// @	145
    {150, 1278.518288}  //// @ 150
};

const float m_ntcTable2Buf[40][2] = {
    {-40, 4397119}, //// @ -40
    {-35, 3088599}, //// @ -35
    {-30, 2197225}, //// @ -30
    {-25, 1581881}, //// @ -25
    {-20, 1151037}, //// @ -20
    {-15, 846579},  //// @ -15
    {-10, 628988},  //// @ -10
    {-5, 471632},   //// @ -5
    {0, 357012},    //// @ 0
    {5, 272500},    //// @ 5
    {10, 209710},   //// @ 10
    {15, 162651},   //// @	15
    {20, 127080},   //// @ 20
    {25, 100000},   //// @ 25
    {30, 79222},    //// @ 30
    {35, 63167},    //// @ 35
    {40, 50677},    //// @ 40
    {45, 40904},    //// @ 45
    {50, 33195},    //// @ 50
    {55, 27091},    //// @ 55
    {60, 22224},    //// @ 60
    {65, 18323},    //// @ 65
    {70, 15184},    //// @ 70
    {75, 12635},    //// @ 75
    {80, 10566},    //// @ 80
    {85, 8873},     //// @ 85
    {90, 7481},     //// @ 90
    {95, 6337},     //// @ 95
    {100, 5384},    //// @ 100
    {105, 4594},    //// @ 105
    {110, 3934},    //// @ 110
    {115, 3380},    //// @ 115
    {120, 2916},    //// @ 120
    {125, 2522},    //// @ 125
    {130, 2522},    //// @ 130
    {135, 2522},    //// @ 135
    {140, 2522},    //// @ 140
    {145, 2522},    //// @ 145
    {150, 2522}     //// @ 150
};


bool UL_NtcInitial(S_NTC_PARAMETERS *f_pParameter)
{
    m_sNtcParameters = *f_pParameter;  
    _NTC_POWER((GPIO_PinState)m_sNtcParameters.ntcStatus);
    m_ntcPowerOkFlag = true;
    return true;
}


void UL_NtcPeripheral(EControl f_eControl)
{
    if(f_eControl == disableNtcPeripheral)
    {
      m_ntcPowerOkFlag = false;
      SleepNtcSensorGpioPinsProc(NTC_ACTIVE_GPIO_Port,      NTC_ACTIVE_Pin,      GPIO_PIN_RESET);
    }
    else
    {
      m_ntcPowerOkFlag = true;
      WakeUpNtcSensorGpioPinsProc(NTC_ACTIVE_GPIO_Port,      NTC_ACTIVE_Pin,      GPIO_PIN_SET);
    }
}


float UL_NtcGetValue(uint32_t f_rawValue)
{
  uint8_t count = 0;
  float resistor, resmin, resmax;
  float retval, step, stepcount;

  resistor = (_NTC_SERIAL_RES * f_rawValue) / (_USR_ADC_RESOLUTION - f_rawValue); 

  // Temperature between -40 +150
  // Check the limits
  if(resistor < _NTC_TOP_RES_VALUE)
  {
    resistor = _NTC_TOP_RES_VALUE;
  }  
  else if(resistor > _NTC_BOTTOM_RES_VALUE)
  {
    resistor  =_NTC_BOTTOM_RES_VALUE;
  }

  for(count = 0; count < _MEMBER_SIZE_OF_NTC; count++)
  {
    if(resistor >= m_ntcTable1Buf[count][1])
    {
      if(resistor == m_ntcTable1Buf[count][1])
      {
        retval = m_ntcTable1Buf[count][0];
      }
      else
      {
        resmax = m_ntcTable1Buf[count - 1][1];
        resmin = m_ntcTable1Buf[count][1];
      }
      break;
    }
  }

  step = (float)(resmax - resmin) / 50;
  stepcount = (float)(resistor - resmin) / step;
  retval = m_ntcTable1Buf[count][0] - (stepcount * 0.1);
  return (float)(retval);   // *10 vardı
}


_io void SleepNtcSensorGpioPinsProc(GPIO_TypeDef *f_pGpio, uint16_t f_pinGroup, GPIO_PinState f_ePinstate)
{
  GPIO_InitTypeDef GPIO_InitStruct;
 
  if(m_ntcPowerOkFlag)
  {
    HAL_GPIO_WritePin(f_pGpio, f_pinGroup, f_ePinstate);
  }
  
  GPIO_InitStruct.Pin = f_pinGroup;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  
  HAL_GPIO_Init(f_pGpio, &GPIO_InitStruct);
}


_io void WakeUpNtcSensorGpioPinsProc(GPIO_TypeDef *f_pGpio, uint16_t f_pinGroup, GPIO_PinState f_ePinstate)
{
  GPIO_InitTypeDef GPIO_InitStruct;
  
  if(m_ntcPowerOkFlag)
  {
    HAL_GPIO_WritePin(f_pGpio, f_pinGroup, f_ePinstate);
  }
  
  GPIO_InitStruct.Pin = f_pinGroup;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  
  HAL_GPIO_Init(f_pGpio, &GPIO_InitStruct);
}



