#ifndef __USR_LIB_GSM_H
#define __USR_LIB_GSM_H

#include "usr_arch.h"

#ifdef STM32_L051R8
    #include "main.h"
    #include "gpio.h"
    #include "usart.h"
    #include "iwdg.h"
#endif


#ifdef GSM2G
    #define _gsm_watchdog()                   HAL_IWDG_Refresh(&hiwdg)
    #define _m65_m66
    
    #ifdef _freertos
        #include "cmsis_os.h"
        #define _gsm_malloc pvPortMalloc
        #define _gsm_free vPortFree
        #define _gsm_delay osDelay
    #else
        #define _gsm_malloc                      malloc
        #define _gsm_free                        free
        #define _gsm_delay(x)                    HAL_Delay(x)
    #endif
#endif // DEBUG


#ifdef GSM4G
    #define _gsm_watchdog()                   HAL_IWDG_Refresh(&hiwdg)
    #ifdef _freertos
        #include "cmsis_os.h"
        #define _gsm_malloc pvPortMalloc
        #define _gsm_free vPortFree
        #define _gsm_delay osDelay
    #else
        #define _gsm_malloc                      malloc
        #define _gsm_free                        free
        #define _gsm_delay(x)                    HAL_Delay(x)
    #endif
#endif
 
#include "MQTTPacket.h" 

#define __gsm_lib_imp_log               // important log
//#define __gsm_lib_detailed_log        // detailed  log

#define _gsm_receive_buffer_size         512  
#define _gsm_buffer_size                 1150

#define _gsm_mqtt_max_topic_size         128  
#define _gsm_mqtt_max_payload_size       1024


// new
/* Bu makro      UL_GsmModuleMqttPublishTopic   ve    UL_GsmModuleMqttSubcribeTopic        de var */
#define _gsm_mqtt_subcribe_buffer        256   // 
#define _gsm_mqtt_publish_buffer         1024  // 

#define _USR_GSM_UART_CHANNEL              huart2
#define _USR_GSM_UART_RAW_INIT_FUNC()      MX_USART2_UART_Init()
#define _USR_GSM_UART_RAW_DEINIT_FUNC()    HAL_UART_DeInit(&_USR_GSM_UART_CHANNEL)

#define _USR_GSM_MAIN_POWER_CONTROL(x)     (x ? (DC_DC_POWER_ON_OFF_GPIO_Port->BSRR = DC_DC_POWER_ON_OFF_Pin) : (DC_DC_POWER_ON_OFF_GPIO_Port->BRR = DC_DC_POWER_ON_OFF_Pin))
#define _USR_GSM_POWER_CONTROL(x)          (x ? (GPRS_POWER_ON_OFF_GPIO_Port->BSRR  = GPRS_POWER_ON_OFF_Pin)  : (GPRS_POWER_ON_OFF_GPIO_Port->BRR  = GPRS_POWER_ON_OFF_Pin))
#define _USR_GSM_POWERKEY_CONTROL(x)       (x ? (PWRKEY_CONTROL_GPIO_Port->BSRR     = PWRKEY_CONTROL_Pin)     : (PWRKEY_CONTROL_GPIO_Port->BRR     = PWRKEY_CONTROL_Pin))

typedef enum
{
    quectelM65GsmModule,
    quectelM66GsmModule,
    cavliGsmModules,
    quectelEC200Module,
}EGsmModules;

typedef enum
{
    disconnectGsmMqttConnectionStatus,
    connectGsmMqttConnectionStatus
}EGsmMqttConnectionStatus;

typedef enum
{
    ipPppMqttUrlType,
    domainPppMqttUrlType,
}EPppMqttUrlType;

typedef enum
{
    disableGsmPeripheral,
    enableGsmPeripheral,
}EGsmPeripheral;

typedef struct S_GSM_PARAMETERS_TAG
{
    UART_HandleTypeDef *pUart;

    GPIO_TypeDef *pPowerPort;
    GPIO_TypeDef *pPowerkeyPort;
    GPIO_TypeDef *pResetPort;

    int powerPin;
    int powerKeyPin;
    int resetPin;
    int gsmProcessMcuPin;
    
    uint8_t powerPinEnableStatus;
    uint8_t powerKeyPinEnableStatus;
    uint8_t resetPinEnableStatus;
    uint8_t gsmProcessMcuStatus;

    EGsmModules eModuleType;
}S_GSM_PARAMETERS;

typedef struct S_GSM_APN_PARAMETERS_TAG
{
    char name[16];
    char userName[16];
    char userPassword[16];
}S_GSM_APN_PARAMETERS;

typedef struct S_GSM_MQTT_PARAMETERS_TAG
{
    char urlBuf[64];       // 256
    uint32_t port;
    char randomIdBuf[16];   // 64
    char usernameBuf[16];   // 64
    char passwordBuf[16];   // 64
    uint8_t keepAlive;
    char userConnectionName[16];   // 64
    int connectionTimeout;
    EPppMqttUrlType urlType;
}S_GSM_MQTT_PARAMETERS;

typedef struct S_GSM_MQTT_CONNECTION_PARAMETERS_TAG
{
    S_GSM_APN_PARAMETERS     sGsmApn;
    S_GSM_MQTT_PARAMETERS    sMqtt;
}S_GSM_MQTT_CONNECTION_PARAMETERS;

typedef struct S_GSM_MODULE_INFO_TAG
{
    uint8_t signal;
    char moduleInfoBuf[64];
    char iccidBuf[32];
    char imeiBuf[32];
}S_GSM_MODULE_INFO;

typedef struct S_GSM_FTP_TAG
{
    S_GSM_APN_PARAMETERS sGsmApn;   
    char urlBuf[128];
    int port;                  // uint32_t
    char userNameBuf[128];
    char userPasswordBuf[128];
    char filePathBuf[64];
    char fileNameBuf[64];
}S_GSM_FTP;

void UL_GsmModuleInitial(S_GSM_PARAMETERS *f_pParam);
void UL_GsmModuleDeInitial(void);
bool UL_GsmModuleCheck(void);
bool UL_GsmModuleGetInfo(S_GSM_MODULE_INFO *f_pInfo);
bool UL_GsmModuleMqttInitial(const S_GSM_MQTT_CONNECTION_PARAMETERS *f_pcParameter);
bool UL_GsmModuleMqttSubcribeTopic(const char *f_cpTopic, int f_qos);
bool UL_GsmModuleMqttPublishTopic(const char *f_cpTopic, const char *f_cpData, int f_qos, int f_retain);
void UL_GsmModuleHardwareReset(void);
uint8_t UL_GsmModuleMqttGeneral(void);
void UL_GsmModuleMqttClosed(void);

bool UL_GsmModuleMqttStart(const S_GSM_MQTT_CONNECTION_PARAMETERS *f_pcParameter);

void UL_GsmModuleUartInterruptCallback(void);
void UL_GsmModuleMqttConnectionStatusCallback(EGsmMqttConnectionStatus f_eStatus);
void UL_GsmModuleMqttSubcribeDataCallback(const char *f_pTopic, uint16_t f_topicLen, const char *f_pPayload, uint16_t f_payloadLen);

bool UL_GsmModuleFtpFileDownload(const S_GSM_FTP *f_cpFtp);
int UL_GsmModuleGetFileTotalLen(const char *f_cpFileName);
int UL_GsmModuleReadFile(const char *f_cpFileName, uint32_t f_startIndex, uint32_t f_size,uint8_t *f_pData);
bool UL_GsmModuleDeleteFile(const char *f_cpFileName);

void UL_GsmModulePeripheral(EGsmPeripheral f_eControl);

uint8_t* UL_GetHelperBufAddress(void);

#endif