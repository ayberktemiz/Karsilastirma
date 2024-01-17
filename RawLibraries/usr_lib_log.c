#include "usr_lib_log.h"
#include <stdint.h>
#include <string.h>
#define _io static
#define _iov static volatile
_io void SendDataProc(const uint8_t *f_pData, uint16_t f_size);
#ifdef _freertos
osMutexId_t g_sDebugMutex;
const osMutexAttr_t m_csDebugMutexAttribute = {
    "myDebugMutex",                        // human readable mutex name
    osMutexRecursive | osMutexPrioInherit, // attr_bits
    NULL,                                  // memory for control block
    0U                                     // size for control block
};
#endif
uint8_t g_logStringBuf[_log_buffer_size];
void UL_LogInitial(void)
{
#ifdef _freertos
    g_sDebugMutex = osMutexNew(&m_csDebugMutexAttribute);
#endif
}
void UL_LogSendString(ELogType f_eType, const char *f_ptr)
{
    uint32_t len = strlen(f_ptr) + 2;
    if((len + 1) >= _log_buffer_size)
        return;
    g_logStringBuf[0] = 0x1a;
    g_logStringBuf[1] = (0xff) & (len >> 8);
    g_logStringBuf[2] = 0xff & len;
    g_logStringBuf[3] = f_eType;
    g_logStringBuf[4] = 0x01;
    for (uint16_t i = 0; i < strlen(f_ptr); i++)
        g_logStringBuf[5 + i] = f_ptr[i];
    SendDataProc((uint8_t *)g_logStringBuf, strlen(f_ptr) + 5);
}
_io void SendDataProc(const uint8_t *f_pData, uint16_t f_size)
{
#if defined _log_send_over_usb
    CDC_Transmit_FS(f_pData, f_size);
#elif defined _log_send_over_i2c
    //coming soon :)
#else
   HAL_UART_Transmit(&_UL_LOG_UART_CHANNEL, f_pData, f_size, 5 * f_size);
#endif
}