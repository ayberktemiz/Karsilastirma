#ifndef __USR_LIB_LOG_H
#define __USR_LIB_LOG_H

#include "usr_arch.h"

#include <stdint.h>
// #define _freertos
#define _log_send_over_uart
// #define _log_send_over_i2c
// #define _log_send_over_usb
#ifdef _freertos
#include "cmsis_os.h"
#endif
#ifdef _log_send_over_usb
#include "usb_device.h"
#endif
#ifdef _log_send_over_uart
#include "usart.h"
#define _UL_LOG_UART_CHANNEL hlpuart1
#endif
#ifdef _log_send_over_i2c
#include "i2c.h"
#define _UL_LOG_I2C_CHANNEL hi2c1
#define _UL_LOG_I2C_SLAVE_ADD 5
#endif
#define _log_buffer_size 256
typedef enum
{
    infoLogType,
    warningLogType,
    errorLogType
} ELogType;
void UL_LogInitial(void);
void UL_LogSendString(ELogType f_eType, const char *f_ptr);
extern uint8_t g_logStringBuf[_log_buffer_size];
#ifdef _freertos
extern osMutexId_t g_sDebugMutex;
#endif
#ifdef _freertos
#define __logsi(x, ...)                                                    \
    {                                                                      \
        osMutexAcquire(g_sDebugMutex, osWaitForever);                      \
        sprintf((char*)&g_logStringBuf[5], "[|%d-%s|:info]", __LINE__,__func__); \
        char *ptr = &g_logStringBuf[5];                                    \
        sprintf((char*)&g_logStringBuf[5 + strlen((const char *)ptr)], x, ##__VA_ARGS__);        \
        UL_LogSendString(infoLogType, (const char *)(&g_logStringBuf[5]));             \
        osMutexRelease(g_sDebugMutex);                                     \
    }
#define __logsw(x, ...)                                                    \
    {                                                                      \
        osMutexAcquire(g_sDebugMutex, osWaitForever);                      \
        sprintf((char*)&g_logStringBuf[5], "[|%d-%s|:info]", __LINE__,__func__); \
        char *ptr = &g_logStringBuf[5];                                    \
        sprintf((char*)&g_logStringBuf[5 + strlen((const char *)ptr)], x, ##__VA_ARGS__);        \
        UL_LogSendString(warningLogType, (const char *)(&g_logStringBuf[5]));          \
        osMutexRelease(g_sDebugMutex);                                     \
    }
#define __logse(x, ...)                                                    \
    {                                                                      \
        osMutexAcquire(g_sDebugMutex, osWaitForever);                      \
        sprintf((char*)&g_logStringBuf[5], "[|%d-%s|:info]", __LINE__,__func__); \
        char *ptr = &g_logStringBuf[5];                                    \
        sprintf((char*)&g_logStringBuf[5 + strlen((const char *)ptr)], x, ##__VA_ARGS__);        \
        UL_LogSendString(errorLogType, (const char *)(&g_logStringBuf[5]));            \
        osMutexRelease(g_sDebugMutex);                                     \
    }
#else
#define __logsi(x, ...)                                                    \
    {                                                                      \
        sprintf((char*)&g_logStringBuf[5], "[|%d-%s|:info]", __LINE__,__func__); \
        char *ptr = &g_logStringBuf[5];                                    \
        sprintf((char*)&g_logStringBuf[5 + strlen((const char *)ptr)], x, ##__VA_ARGS__);        \
        UL_LogSendString(infoLogType, (const char *)(&g_logStringBuf[5]));             \
    }
#define __logsw(x, ...)                                                    \
    {                                                                      \
        sprintf((char*)&g_logStringBuf[5], "[|%d-%s|:warning]", __LINE__,__func__); \
        char *ptr = &g_logStringBuf[5];                                    \
        sprintf((char*)&g_logStringBuf[5 + strlen((const char *)ptr)], x, ##__VA_ARGS__);        \
        UL_LogSendString(warningLogType, (const char *)(&g_logStringBuf[5]));          \
    }
#define __logse(x, ...)                                                    \
    {                                                                      \
        sprintf((char*)&g_logStringBuf[5], "[|%d-%s|:error]", __LINE__,__func__); \
        char *ptr = &g_logStringBuf[5];                                    \
        sprintf((char*)&g_logStringBuf[5 + strlen((const char *)ptr)], x, ##__VA_ARGS__);        \
        UL_LogSendString(errorLogType, (const char *)(&g_logStringBuf[5]));            \
    }
#endif
#endif //__USR_LIB_LOG_H