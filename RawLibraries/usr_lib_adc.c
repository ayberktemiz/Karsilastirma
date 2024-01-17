#include "usr_lib_adc.h"

#define _io static
#define _iov static volatile
#define _adc_debug

_io S_ADC_PARAMETERS m_sAdcParameters;
_io S_ADC_RAW_PARAMETERS m_sAdcRawParameters;

_io void SleepAdcPinsProc(GPIO_TypeDef *f_pGpio, uint16_t f_pinGroup, GPIO_PinState f_ePinstate);
_io void SleepGpioOutPinsProc(GPIO_TypeDef *f_pGpio, uint16_t f_pinGroup, GPIO_PinState f_ePinstate);
_io void WakeUpAdcGpioPinsProc(S_ADC_PARAMETERS *f_pAdc);
_io void SleepAdcGpioPinsProc(S_ADC_PARAMETERS *f_pAdc);

bool m_adcFinishedFlag = false;
bool m_adcInitialFlag = false;
bool m_adcStartGetValueFlag = false;

_io uint8_t m_adcSampleCounter = 0;

uint16_t adc_values_dma[ADC_CHANNEL_COUNT];
_iov uint16_t m_adcRawValueBuf[ADC_CHANNEL_COUNT][SAMPLE_COUNT];


bool UL_AdcInitial(S_ADC_PARAMETERS *f_pParameter)
{
    m_sAdcParameters = *f_pParameter;
    m_adcInitialFlag = true;                   // Bu flag'ı UL_Adc_Peripheral'da kullandık
    m_adcStartGetValueFlag = true;
    
    HAL_ADC_DeInit(m_sAdcParameters.pAdcforDma); 

    return true; 
}

void UL_AdcPeripheral(EAdcControl f_eControl)
{
    if(f_eControl == disableAdcPeripheral)      
    {
        HAL_ADC_DeInit(&_USR_ADC_CHANNEL);
        
        SleepGpioOutPinsProc(VBAT_ADC_HIGH_GPIO_Port, VBAT_ADC_HIGH_Pin, GPIO_PIN_SET);
        SleepAdcPinsProc(VBAT_ADC_LOW_GPIO_Port, VBAT_ADC_LOW_Pin, GPIO_PIN_SET);
        SleepAdcPinsProc(TEMP_ADC_GPIO_Port, TEMP_ADC_Pin, GPIO_PIN_RESET);
        /*
        SleepGpioOutPinsProc(VBAT_ADC_HIGH_GPIO_Port, VBAT_ADC_HIGH_Pin, GPIO_PIN_SET);
        SleepAdcPinsProc(VBAT_ADC_LOW_GPIO_Port,      VBAT_ADC_LOW_Pin,  GPIO_PIN_RESET);
        SleepAdcPinsProc(NTC_ACTIVE_GPIO_Port,          NTC_ACTIVE_Pin,      GPIO_PIN_RESET);
        */
    }
    else
    {
        _USR_ADC_INIT_FUNC();
        // HAL_ADC_DeInit(&_USR_ADC_CHANNEL); 

        ADC1->CR &= (uint32_t)0xfffffffe;          // Önce adc disable
        ADC1->CR |= (uint32_t)0x80000000;          // adc kalibrasyon enable
        while(ADC1->CR & ((uint32_t)0x80000000))   // calibrasyon bekleme
        ;
        ADC1->CR |= (uint32_t)0x00000001;          // sonra adc enable  
    }
}

// blokeli olarak adc değerleri burada toplanancak
bool UL_AdcGetValues(S_ADC_PARAMETERS *f_pParameter, S_ADC_RAW_PARAMETERS *f_pData)
{
    if (f_pData != NULL && f_pParameter != NULL) 
    {
        *f_pParameter = m_sAdcParameters;
        *f_pData      = m_sAdcRawParameters;
    }

    m_adcFinishedFlag = false;
    if((HAL_OK == HAL_ADC_Start_DMA(m_sAdcParameters.pAdcforDma, (uint32_t*)adc_values_dma, ADC_CHANNEL_COUNT)))
    {
        while(!m_adcFinishedFlag)
        ;

        uint16_t temp = 0;
        uint32_t batteryHigh = 0;
        uint32_t batteryLow = 0;
        uint32_t vrefTemp = 0; 
        
        if(!m_adcStartGetValueFlag)
        {
          return false;
        }
        else
        {
            for(uint16_t m = 0; m < ADC_CHANNEL_COUNT; m++)
            {
                for(uint8_t i = 0; i < SAMPLE_COUNT; i++)
                {
                    temp        += m_adcRawValueBuf[TEMP_ADC_CHANNEL][i];
                    batteryHigh += m_adcRawValueBuf[VBAT_ADC_HIGH_CHANNEL][i];
                    batteryLow  += m_adcRawValueBuf[VBAT_ADC_LOW_CHANNEL][i];
                    vrefTemp    += m_adcRawValueBuf[VREF_ADC_CHANNEL][i];
                }
                
                temp         /= SAMPLE_COUNT;
                batteryHigh  /= SAMPLE_COUNT;
                batteryLow   /= SAMPLE_COUNT;
                vrefTemp     /= SAMPLE_COUNT;
            }

            f_pData->rawTempValue        = temp;
            f_pData->rawBatteryHighValue = batteryHigh;
            f_pData->rawBatteryLowValue  = batteryLow;
            f_pData->rawVrefTempValue    = vrefTemp;
            return true;
        }
    }
    else
    {
        HAL_ADC_Stop_DMA(m_sAdcParameters.pAdcforDma);
        return false;        
    }
}

void UL_AdcCallback(void)
{
    //// Take the values from dma
    for (int i = 0; i < ADC_CHANNEL_COUNT; i++)
    {
        m_adcRawValueBuf[i][m_adcSampleCounter] = adc_values_dma[i];
    }
    //// Increase the sample count
    m_adcSampleCounter++;
    //// Check adc sampling that is enough
    if (m_adcSampleCounter >= SAMPLE_COUNT)
    {
        //// Close the dma interrupt reading
        HAL_ADC_Stop_DMA(&_USR_ADC_CHANNEL);
        //// Clear sampling count
        m_adcSampleCounter = 0;
        //// Show the system that the reading finished
        m_adcFinishedFlag = true;
    }
}

_io void SleepAdcPinsProc(GPIO_TypeDef *f_pGpio, uint16_t f_pinGroup, GPIO_PinState f_ePinstate)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    HAL_GPIO_WritePin(f_pGpio, f_pinGroup, f_ePinstate);
    GPIO_InitStruct.Pin = f_pinGroup;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(f_pGpio, &GPIO_InitStruct);
}


_io void SleepGpioOutPinsProc(GPIO_TypeDef *f_pGpio, uint16_t f_pinGroup, GPIO_PinState f_ePinstate)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    HAL_GPIO_WritePin(f_pGpio, f_pinGroup, f_ePinstate);
    GPIO_InitStruct.Pin = f_pinGroup;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(f_pGpio, &GPIO_InitStruct);
}


_io void SleepAdcGpioPinsProc(S_ADC_PARAMETERS *f_pAdc)
{
    m_sAdcParameters = *f_pAdc;
    if (!m_adcInitialFlag)
    {
        HAL_ADC_Stop(m_sAdcParameters.pAdcforDma);
    }
    m_adcStartGetValueFlag = false;
}


_io void WakeUpAdcGpioPinsProc(S_ADC_PARAMETERS *f_pAdc)
{
    m_sAdcParameters = *f_pAdc;
    if (m_adcInitialFlag)
    {
        HAL_ADC_Start_DMA(m_sAdcParameters.pAdcforDma, (uint32_t *)adc_values_dma, ADC_CHANNEL_COUNT);
    }
    m_adcStartGetValueFlag = true;
}
