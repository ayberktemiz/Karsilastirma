#include "usr_lib_sensor.h"

_io bool m_dataOkFlag;
uint8_t m_globalRxBuffer[_USR_UART_TOTAL_BYTES];
_io bool m_distanceOkFlag = false;

_io S_ULTRASONIC_SENSOR_PARAMETERS m_sUltrasonicParameter;

#define _USR_ULTRASONIC_SENSOR_RAW_CHANNEL      m_sUltrasonicParameter.pUart->Instance
#define _USR_ULTRASONIC_SENSOR_POWER(x)         HAL_GPIO_WritePin(m_sUltrasonicParameter.pDistanceSensorOnOffPort, m_sUltrasonicParameter.sensorOnOffPin, (GPIO_PinState)x)

_io void SleepSensorGpioPinsProc(GPIO_TypeDef *f_pGpio, uint16_t f_pinGroup, GPIO_PinState f_ePinstate);


void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    HAL_UART_Receive_IT(m_sUltrasonicParameter.pUart, m_globalRxBuffer, _USR_UART_TOTAL_BYTES);
}


bool UL_UltrasonicSensorInitial(S_ULTRASONIC_SENSOR_PARAMETERS *f_pSensor)
{
    m_sUltrasonicParameter = *f_pSensor;
    _USR_ULTRASONIC_SENSOR_POWER(1);
    return true;
}


void UL_UltrasonicSensorPeripheral(EUltrasonicControl f_eControl)
{
    if (f_eControl == enableUltrasonicSensor)
    {
        HAL_GPIO_WritePin(m_sUltrasonicParameter.pDistanceSensorOnOffPort, m_sUltrasonicParameter.sensorOnOffPin, (GPIO_PinState)m_sUltrasonicParameter.sensorPowerStatus);
        HAL_UART_DeInit(m_sUltrasonicParameter.pUart);
        _USR_ULTRASONIC_SENSOR_RAW_INIT_FUNC();
        _USR_ULTRASONIC_DELAY(100);
        _USR_ULTRASONIC_SENSOR_RAW_CHANNEL->ICR  = 0xFFFFFFFF;
        _USR_ULTRASONIC_SENSOR_RAW_CHANNEL->CR3 |= ((uint32_t)0x00000001);
        _USR_ULTRASONIC_SENSOR_RAW_CHANNEL->CR1 |= ((uint32_t)0x00000010);
        HAL_UART_Receive_IT(m_sUltrasonicParameter.pUart, (uint8_t *)m_globalRxBuffer, _USR_UART_TOTAL_BYTES);
    }
    else
    {
        HAL_GPIO_WritePin(m_sUltrasonicParameter.pDistanceSensorOnOffPort, m_sUltrasonicParameter.sensorOnOffPin, (GPIO_PinState)!m_sUltrasonicParameter.sensorPowerStatus);
        HAL_UART_DeInit(m_sUltrasonicParameter.pUart);
        SleepSensorGpioPinsProc(_USR_ULTRASONIC_SENSOR_TX_GPIO_PORT, _USR_ULTRASONIC_TX_GPIO_PIN | _USR_ULTRASONIC_RX_GPIO_PIN, GPIO_PIN_RESET);
    }
}

// Timeout girilmesi gerekiyor  // Gerçekten 32 bitlik bir zaman değeri girecek mi?
int UL_UltrasonicSensorGetValue(uint32_t f_timeoutMilisecond)
{
    int m_distance = 0;
    uint32_t startTime = HAL_GetTick();
    while(HAL_GetTick() - startTime < f_timeoutMilisecond)
    {
        if(m_dataOkFlag)
        {
            uint16_t counter = m_sUltrasonicParameter.pUart->RxXferSize - m_sUltrasonicParameter.pUart->RxXferCount;

            if(counter >= 4)
            {
                int receiverCheckSum = (m_globalRxBuffer[0] + m_globalRxBuffer[1] + m_globalRxBuffer[2]) & 0x00FF;
                if (receiverCheckSum == m_globalRxBuffer[3])
                {
                    m_distance = ((uint16_t)(m_globalRxBuffer[2] << 0) | (uint16_t)(m_globalRxBuffer[1] << 8));
                    if (m_distance > 0)
                    {
                        m_distanceOkFlag = true;
                    }
                    else
                    {
                        m_distanceOkFlag = false;
                    }
                }
                counter = 0;
            }
            HAL_UART_Abort_IT(m_sUltrasonicParameter.pUart);
            HAL_UART_Receive_IT(m_sUltrasonicParameter.pUart, m_globalRxBuffer, _USR_UART_TOTAL_BYTES);
            if(m_distanceOkFlag)
            {
                m_dataOkFlag = false;
                return m_distance;
            } 
            m_dataOkFlag = false;
        }
    }
    return _USR_DISTANCE_SENSOR_ERROR_VALUE;
}


// uart datasinin interrupt altinda toplanmasini saglayan fonksiyon
void UL_UltrasonicSensorCallback(void) // stm32l0xx_it.c dosyası
{
    if (m_sUltrasonicParameter.pUart->Instance->ISR & ((uint32_t)0x1f))
    {
        if ((m_sUltrasonicParameter.pUart->Instance->ISR & 0x10) == 0)
        {
            HAL_UART_Receive_IT(m_sUltrasonicParameter.pUart, (uint8_t *)m_globalRxBuffer, _USR_UART_TOTAL_BYTES);
        }
        m_sUltrasonicParameter.pUart->Instance->ICR |= 0x1f;
        m_dataOkFlag = true;
    }
}


_io void SleepSensorGpioPinsProc(GPIO_TypeDef *f_pGpio, uint16_t f_pinGroup, GPIO_PinState f_ePinstate)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    HAL_GPIO_WritePin(f_pGpio, f_pinGroup, f_ePinstate);
    GPIO_InitStruct.Pin = f_pinGroup;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    
    HAL_GPIO_Init(f_pGpio, &GPIO_InitStruct);
}
