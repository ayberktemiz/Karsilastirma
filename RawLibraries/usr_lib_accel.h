#ifndef __USR_ACCEL_LIB_H
#define __USR_ACCEL_LIB_H

#include "usr_arch.h"

#ifdef STM32_L051R8
    #include "main.h"
    #include "gpio.h"
    #include "i2c.h"  
#endif

#define LISDE12_ADDRESS             0x18 // 0001 1001  SDO power supply'a bagli ise LSB 1 olur.
#define LIS2DE12_WHO_AM_I           0x0f
#define LIS2DE12_TEMP_CFG_REG       0x1F 
#define LIS2DE12_CTRL_REG0          0x1E
#define LIS2DE12_CTRL_REG1          0x20
#define LIS2DE12_CTRL_REG2          0x21
#define LIS2DE12_CTRL_REG3          0x22
#define LIS2DE12_CTRL_REG4          0x23
#define LIS2DE12_CTRL_REG5          0x24
#define LIS2DE12_CTRL_REG6          0x25
#define LIS2DE12_CTRL_REG7          0x26
#define LIS2DE12_STATUS_REG         0x27
#define LIS2DE12_FIFO_READ_START    0x28
#define LIS2DE12_OUT_X              0x29
#define LIS2DE12_OUT_Y		        0x2B
#define LIS2DE12_OUT_Z	            0x2D
#define LISD2E12_INT1_CFG           0x30
#define LISD2E12_INT1_SRC           0x31
#define LISD2E12_INT1_THS           0x32
#define LISD2E12_INT1_DURATION      0x33
#define LISD2E12_INT2_CFG           0x34
#define LISD2E12_INT2_SRC           0x35
#define LISD2E12_INT2_THS           0x36
#define LISD2E12_INT2_DURATION      0x37
#define LISD2E12_CLICK_CFG          0x38

#define ACC_I2C_CHANNEL             hi2c2
#define ACC_INT1_IRQ                EXTI->PR & EXTI_PR_PIF2
#define ACC_INT_CLEAR()             EXTI->PR |= EXTI_PR_PIF2   
#define ACC_INT1_PIN()              (GPIOB->IDR & GPIO_PIN_2) 

typedef struct S_ACC_PARAMETERS_TAG
{
    GPIO_TypeDef *accelPowerPort;
    int           accelPin;
    uint8_t       accelPowerStatus;
    GPIO_TypeDef *interruptPort;
    int           interruptPin;
}S_ACC_PARAMETERS;


bool UL_AccelCheckChip(void);
bool UL_AccelInitial(S_ACC_PARAMETERS *f_pParameter);
bool UL_AccelChipOk(void);
bool UL_AccelFabrication(void);
bool UL_AccelCheckParam(void);
void UL_AccelometerClearFlag(void);

#endif