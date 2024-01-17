#ifndef __USR_LED_LIB_H
#define __USR_LED_LIB_H

#include "usr_arch.h"

#ifdef STM32_L051R8
#include "main.h"
#include "gpio.h"
#endif

#define _led_delay(x)              HAL_Delay(x) 

#define _RGB_LED_RED_CONTROL(x)    (x ? (RGB_LED_RED_PWM_GPIO_Port->BSRR = RGB_LED_RED_PWM_Pin) : (RGB_LED_RED_PWM_GPIO_Port->BRR = RGB_LED_RED_PWM_Pin))
#define _RGB_LED_GREEN_CONTROL(x)  (x ? (RGB_LED_GREEN_PWM_GPIO_Port->BSRR = RGB_LED_GREEN_PWM_Pin) : (RGB_LED_GREEN_PWM_GPIO_Port->BRR = RGB_LED_GREEN_PWM_Pin))
#define _RGB_LED_BLUE_CONTROL(x)   (x ? (RGB_LED_BLUE_PWM_GPIO_Port->BSRR = RGB_LED_BLUE_PWM_Pin) : (RGB_LED_BLUE_PWM_GPIO_Port->BRR = RGB_LED_BLUE_PWM_Pin))

typedef struct S_LED_PARAMETERS_TAG
{
    GPIO_TypeDef *pRedPort;
    GPIO_TypeDef *pGreenPort;
    GPIO_TypeDef *pBluePort;
    int redPin;
    int greenPin;
    int bluePin;
}S_LED_PARAMETERS;

typedef enum
{
    noColor,
    redColor,
    greenColor,
    blueColor,
    yellowColor,
    purpleColor,
    allColor,
}ELedColor;

typedef enum
{
    disableLedPeripheral,
    enableLedPeripheral,
}ELedControl;

bool UL_LedInitial(S_LED_PARAMETERS *f_pParam);
void UL_Led(ELedColor f_eColor);
void UL_LedPeripheral(ELedControl f_eControl);
void UL_LedTime(ELedColor f_eColor, uint32_t f_closeTime);
void UL_LedAllDisable(void);
void UL_LedOpenAnimation(uint32_t f_closeTime);
void UL_LedPassiveAnimation(uint32_t f_closeTime);
void UL_LedPilVoltageError(void);
void UL_LedLevelSensorError(void);
void UL_LedAccelError(void);
void UL_LedGsmNotifications(uint8_t f_value);

#endif //__USR_LIB_LED_H
