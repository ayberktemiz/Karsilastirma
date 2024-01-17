#ifndef __USR_SENSOR_LIB_H
#define __USR_SENSOR_LIB_H

#include "usr_arch.h"

#ifdef STM32_L051R8
    #include "main.h"
    #include "gpio.h"
    #include "usart.h"
#endif

#define _USR_UART_TOTAL_BYTES                   512
#define _USR_ULTRASONIC_SENSOR_RAW_INIT_FUNC    MX_USART1_UART_Init
#define _USR_ULTRASONIC_DELAY                   HAL_Delay

#define _USR_ULTRASONIC_SENSOR_TX_GPIO_PORT     TX_SENSOR_GPIO_Port
#define _USR_ULTRASONIC_TX_GPIO_PIN             TX_SENSOR_Pin
#define _USR_ULTRASONIC_RX_GPIO_PIN             RX_SENSOR_Pin

#define _USR_DISTANCE_SENSOR_ERROR_VALUE        (int)-100

typedef enum
{
    model1Sensor,
    model2Sensor,
}EUltasonicSensor;

typedef enum
{
    disableUltrasonicSensor,
    enableUltrasonicSensor,
}EUltrasonicControl;

typedef struct S_ULTRASONIC_SENSOR_PARAMETERS_TAG
{
    UART_HandleTypeDef     *pUart;
    GPIO_TypeDef           *pDistanceSensorOnOffPort;
    int                    sensorOnOffPin; 
    uint8_t                sensorPowerStatus;   
    EUltasonicSensor       eSensorType;
}S_ULTRASONIC_SENSOR_PARAMETERS;


void UL_UltrasonicSensorCallback(void);
int UL_UltrasonicSensorGetValue(uint32_t f_timeoutMilisecond);
bool UL_UltrasonicSensorInitial(S_ULTRASONIC_SENSOR_PARAMETERS *f_pSensor);
void UL_UltrasonicSensorPeripheral(EUltrasonicControl f_eControl);

#endif