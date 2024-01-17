#include "usr_lib_led.h"

#define _io static
#define _iov static volatile

_io S_LED_PARAMETERS m_sLedParameters;

#define _red(x)   HAL_GPIO_WritePin(m_sLedParameters.pRedPort,   m_sLedParameters.redPin,   x)
#define _green(x) HAL_GPIO_WritePin(m_sLedParameters.pGreenPort, m_sLedParameters.greenPin, x)
#define _blue(x)  HAL_GPIO_WritePin(m_sLedParameters.pBluePort,  m_sLedParameters.bluePin,  x)

bool UL_LedInitial(S_LED_PARAMETERS *f_pLedParam)
{
    m_sLedParameters = *f_pLedParam;
    _red(GPIO_PIN_RESET);
    _green(GPIO_PIN_RESET);
    _blue(GPIO_PIN_RESET);
    return true;
}

void UL_Led(ELedColor f_eColor)
{
    GPIO_PinState red = GPIO_PIN_RESET;
    GPIO_PinState green = GPIO_PIN_RESET;
    GPIO_PinState blue = GPIO_PIN_RESET;

    if (f_eColor == redColor)
        red = GPIO_PIN_SET;
    else if (f_eColor == greenColor)
        green = GPIO_PIN_SET;
    else if (f_eColor == blueColor)
        blue = GPIO_PIN_SET;
    else if (f_eColor == yellowColor)
    {
        red = GPIO_PIN_SET;
        green = GPIO_PIN_SET;
    }
    else if (f_eColor == purpleColor)
    {
        red = GPIO_PIN_SET;
        blue = GPIO_PIN_SET;
    }

    _red(red);
    _green(green);
    _blue(blue);
}

void UL_LedTime(ELedColor f_eColor, uint32_t f_closeTime)
{
    UL_Led(f_eColor);
    _led_delay(f_closeTime);
    UL_Led(noColor);
}

void UL_LedPeripheral(ELedControl f_eControl)
{
    if (f_eControl == enableLedPeripheral)
    {
        _red(GPIO_PIN_SET);
        _green(GPIO_PIN_SET);
        _blue(GPIO_PIN_SET);
    }
    else
    {
        _red(GPIO_PIN_RESET);
        _green(GPIO_PIN_RESET);
        _blue(GPIO_PIN_RESET);
    }
}

void UL_LedAllDisable(void)
{
    _RGB_LED_RED_CONTROL(disableLedPeripheral);
    _RGB_LED_GREEN_CONTROL(disableLedPeripheral);
    _RGB_LED_BLUE_CONTROL(disableLedPeripheral);
}

void UL_LedOpenAnimation(uint32_t f_closeTime)
{
    UL_LedAllDisable();

    _RGB_LED_RED_CONTROL(1);
    _led_delay(f_closeTime);
    _RGB_LED_RED_CONTROL(0);
    _led_delay(400);

    _RGB_LED_GREEN_CONTROL(1);
    _led_delay(f_closeTime);
    _RGB_LED_GREEN_CONTROL(0);
    _led_delay(400);

    _RGB_LED_BLUE_CONTROL(1);
    _led_delay(f_closeTime);
    _RGB_LED_BLUE_CONTROL(0);
    _led_delay(400);
}

void UL_LedPassiveAnimation(uint32_t f_closeTime)
{
    UL_LedAllDisable();

    _RGB_LED_RED_CONTROL(1);
    _led_delay(f_closeTime);
    _RGB_LED_RED_CONTROL(0);
    _led_delay(f_closeTime);
}

void UL_LedPilVoltageError(void)
{
    UL_LedAllDisable();
    _RGB_LED_RED_CONTROL(1);
    _led_delay(50);
    _RGB_LED_RED_CONTROL(0);
    _led_delay(400);
}


void UL_LedLevelSensorError(void)
{
    UL_LedAllDisable();
    _RGB_LED_RED_CONTROL(1);
    _RGB_LED_BLUE_CONTROL(1);
    _led_delay(50);
    _RGB_LED_RED_CONTROL(0);
    _RGB_LED_BLUE_CONTROL(0);
    _led_delay(400);
}


void UL_LedAccelError(void)
{
    UL_LedAllDisable();
    _RGB_LED_RED_CONTROL(1);
    _RGB_LED_GREEN_CONTROL(1);
    _led_delay(50);
    _RGB_LED_RED_CONTROL(0);
    _RGB_LED_GREEN_CONTROL(0);
    _led_delay(400);
}


/*
void UL_LedGsmNotifications(uint8_t f_value)
{
    UL_LedAllDisable();

    if(f_value == 0)                                  // Yeşil 5 kez Yanar Söner (GSM Mission Basarili)
    {
        for(uint8_t i = 0; i < 5; i++)
        {
            _RGB_LED_GREEN_CONTROL(1);
            _led_delay(100);
            _RGB_LED_GREEN_CONTROL(0);
            _led_delay(100);
        }
    }

    else if(f_value == 1)                              // Kirmizi 5 kez Yanar Söner (GSM Mission Fail)         
    {
        for(uint8_t i = 0; i < 5; i++)
        {
            _RGB_LED_RED_CONTROL(1);
            _led_delay(100);
            _RGB_LED_RED_CONTROL(0);
            _led_delay(100);
        }
    }

    else if(f_value == 2)                               // Lacivert 5 kez Yanar Söner (GSM Mission Started)
    {
        for(uint8_t i = 0; i < 5; i++)
        {
            _RGB_LED_BLUE_CONTROL(1);
            _led_delay(100);
            _RGB_LED_BLUE_CONTROL(0);
            _led_delay(100);
        }
    }

    if(f_value == 3)                                     // Turkuaz 5 kez Yanar Söner (Data Send Basarili)
    {
        for(uint8_t i = 0; i < 5; i++)
        {
            _RGB_LED_BLUE_CONTROL(1);
            _RGB_LED_GREEN_CONTROL(1);
            _led_delay(100);
            _RGB_LED_BLUE_CONTROL(0);
            _RGB_LED_GREEN_CONTROL(0);
            _led_delay(100);
        }
    }
    else if(f_value == 4)                                 // Turkuaz-Kirmizi 5 kez sirayla Yanar  (Response Gelmemis)
    {
        for(uint8_t i = 0; i < 5; i++)
        {
            _RGB_LED_BLUE_CONTROL(1);
            _RGB_LED_GREEN_CONTROL(1);
            _led_delay(100);
            _RGB_LED_BLUE_CONTROL(0);
            _RGB_LED_GREEN_CONTROL(0);
            _RGB_LED_RED_CONTROL(1);
            _led_delay(100);
            _RGB_LED_RED_CONTROL(0);
        }
    }
    else if(f_value == 5)                                  // Turkuaz-Yesil 5 kez sirayla Yanar  (Response Alindi)
    {
        for(uint8_t i = 0; i < 5; i++)
        {
            _RGB_LED_BLUE_CONTROL(1);
            _RGB_LED_GREEN_CONTROL(1);
            _led_delay(100);
            _RGB_LED_BLUE_CONTROL(0);
            _led_delay(100);
        }
    }

    UL_LedAllDisable();
}
*/

void UL_LedGsmNotifications(uint8_t f_value)
{
    UL_LedAllDisable();

    if(f_value == 0)                                  // Yeşil 5 kez Yanar Söner (GSM Mission Basarili)
    {
        for(uint8_t i = 0; i < 4; i++)
        {
            _RGB_LED_GREEN_CONTROL(1);
            _led_delay(150);
            _RGB_LED_GREEN_CONTROL(0);
            _led_delay(100);
        }
    }

    else if(f_value == 1)                              // Lacivert 5 kez Yanar Söner (GSM Mission Fail)         
    {
        for(uint8_t i = 0; i < 4; i++)
        {
            _RGB_LED_BLUE_CONTROL(1);
            _led_delay(250);
            _RGB_LED_BLUE_CONTROL(0);
            _led_delay(100);
        }
    }

    else if(f_value == 2)                               // Kirmizi 5 kez Yanar Söner (No Sim Card)
    {
        for(uint8_t i = 0; i < 4; i++)
        {
            _RGB_LED_RED_CONTROL(1);
            _led_delay(250);
            _RGB_LED_RED_CONTROL(0);
            _led_delay(100);
        }
    }
    
    UL_LedAllDisable();
}

void UL_LedGsmWaitForResponse(uint16_t f_value, uint16_t f_period)          // Beklerken Turkuaz Blink Animation
{
    _io uint32_t _tsForBlinkAnim = 0;

    if( HAL_GetTick() > (_tsForBlinkAnim + f_period) )                        
    {
        UL_LedAllDisable();
        _RGB_LED_BLUE_CONTROL(1);
        _RGB_LED_GREEN_CONTROL(1);
        _led_delay(f_value);
        UL_LedAllDisable();

        _tsForBlinkAnim = HAL_GetTick();
    }
}