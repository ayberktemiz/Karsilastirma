#include "usr_lib_gsm.h"

#ifdef _gsm_debug
    #include "usr_lib_log.h"
#endif

#define _c(x)         ((const char *)x)
#define _size(x)      strlen((const char *)x)
#define LOCALPOSITION "UFS"

_io bool ModuleSendCommandAndGetResponseProc(const char *f_pcCommand, const char *f_pcResponse, uint8_t f_tryCount, uint32_t f_timeout);
_io bool ModuleSendCommandWithLenAndGetResponseProc(const uint8_t *f_pcCommand, uint16_t f_len, const char *f_pcResponse, uint8_t f_tryCount, uint32_t f_timeout);
//_io 
bool MqttConnectionProc(const S_GSM_MQTT_CONNECTION_PARAMETERS *f_pGsmMqttConnectionParameters);
_io void UartParserProc(void);
_io void ClearUartProc(void);
_io bool TcpSendDataProc(const uint8_t *f_pData, uint16_t f_len);
_io int TcpGetRawDataProc(void);
_io int TcpGetDataProc(unsigned char *f_pData, int f_len);
_io void TcpGetStoredDataTriggerReadProc(uint32_t *f_pTimerCnt);
_io void TcpGetStoredDataProc(void);
_io void TcpConnectionStatus(void);
_io void MqttKeepaliveProc(void);
//_io 
int ModuleListenResultProc(const char *f_pRes, uint32_t f_timeout);
_io int ModuleListenResultsProc(const char *f_pList, uint8_t f_totalNumber, uint32_t f_timeout);
_io void ModuleResetProc(void);
_io void SleepGsmGpioOutPinsProc(GPIO_TypeDef *f_pGpio, uint16_t f_pinGroup, GPIO_PinState f_ePinstate);

_io S_GSM_PARAMETERS                  m_sGsmParameters;
// _io S_GSM_FTP                         m_sGsmFtpParameters;
_io S_GSM_MQTT_PARAMETERS             m_sGsmMqttParameters;
_io S_GSM_MQTT_CONNECTION_PARAMETERS  m_sGsmMqttConnectionParameters;
_io S_GSM_APN_PARAMETERS              m_sGsmApnParameters;
_io S_GSM_MODULE_INFO                 m_sGsmModuleInfoParameters;

#define _USR_GSM_UART_RAW_CHANNEL     m_sGsmParameters.pUart->Instance

/*
_io uint8_t m_receiveGsmBuf[_gsm_receive_buffer_size];
_io uint8_t m_receiveGsmEndBuf[_gsm_receive_buffer_size];  
_io uint8_t m_transmitGsmBuf[256];                          // GSM'ye yolla, esas yazan: _gsm_trasbuffer_buffer_size yaziyordu  // 300 Eren yazdi
_io uint8_t m_tcpGsmBuf[_gsm_receive_buffer_size];

// UL_GsmModuleMqttGeneral fonksiyonu icin kullanilan degiskenler
_io uint8_t m_mqttDataBuf[512];         // 128 ise yaramadi dikkat et,  256 yaziyordu
char response[512];                     // UL_GsmModuleMqttGeneral()    400'e geri inebilir
*/


bool responseSubcribeDataCallbackFlag;  // UL_GsmModuleMqttGeneral();

_io uint16_t m_receiveGsmUartCnt;
_io uint16_t m_tcpGsmUartCnt;
_io int m_tcpCurrentLen = 0;

_io bool m_eReceiveGsmDataCameOkFlg;
_io bool m_eReceiveGsmDataCameFlg;

_io bool m_eMqttConnectionOkFlg;
_io uint32_t m_mqttKeepAliveTime = 0;


uint8_t m_receiveGsmBuf[_gsm_receive_buffer_size];
uint8_t m_GsmBuf[_gsm_receive_buffer_size];
uint8_t m_HelperBuf[_gsm_receive_buffer_size];

_io void ClearUartProc_ayberk(void);
bool ModuleSendCommandAndGetResponseProc_ayberk(const char *f_pcCommand, const char *f_pcResponse, uint8_t f_tryCount, uint32_t f_timeout);

void UL_GsmModuleInitial(S_GSM_PARAMETERS *f_pParam)
{
    m_sGsmParameters = *f_pParam;

    m_sGsmParameters.pUart->Instance->CR1 |= ((uint32_t)0x00000010); ////parity error, idle error
    m_sGsmParameters.pUart->Instance->CR3 |= ((uint32_t)0x00000001); ////FE , ORE , NF error

    HAL_UART_Abort_IT(m_sGsmParameters.pUart);
    HAL_UART_Receive_IT(m_sGsmParameters.pUart, m_receiveGsmBuf, _gsm_receive_buffer_size);

    HAL_GPIO_WritePin(m_sGsmParameters.pPowerkeyPort, m_sGsmParameters.powerKeyPin,      (GPIO_PinState)m_sGsmParameters.powerKeyPinEnableStatus);
    HAL_GPIO_WritePin(m_sGsmParameters.pPowerkeyPort, m_sGsmParameters.resetPin,         (GPIO_PinState)m_sGsmParameters.powerPinEnableStatus);
    HAL_GPIO_WritePin(m_sGsmParameters.pPowerkeyPort, m_sGsmParameters.gsmProcessMcuPin, (GPIO_PinState)m_sGsmParameters.gsmProcessMcuStatus);

    _gsm_delay(500);
}


void UL_GsmModuleDeInitial(void)
{
    if (m_sGsmParameters.pUart != NULL)
    {
        HAL_UART_Abort_IT(m_sGsmParameters.pUart);
        HAL_UART_DeInit(m_sGsmParameters.pUart);
    }
    HAL_GPIO_WritePin(m_sGsmParameters.pPowerPort,    m_sGsmParameters.powerPin,    (GPIO_PinState)!m_sGsmParameters.powerPinEnableStatus);
    HAL_GPIO_WritePin(m_sGsmParameters.pPowerkeyPort, m_sGsmParameters.powerKeyPin, (GPIO_PinState)!m_sGsmParameters.powerKeyPinEnableStatus);
    HAL_GPIO_WritePin(m_sGsmParameters.pResetPort,    m_sGsmParameters.resetPin,    (GPIO_PinState)!m_sGsmParameters.resetPinEnableStatus);
}


void UL_GsmModulePeripheral(EGsmPeripheral f_eControl)
{
    if (f_eControl == enableGsmPeripheral)
    {
        _USR_GSM_UART_RAW_INIT_FUNC();
        _USR_GSM_UART_RAW_CHANNEL->ICR = 0xFFFFFFFF;
        _USR_GSM_UART_RAW_CHANNEL->CR1 |= 0x00000010;
        _USR_GSM_UART_RAW_CHANNEL->CR3 |= 0x00000001;
        HAL_UART_AbortReceive_IT(&_USR_GSM_UART_CHANNEL);
        HAL_UART_Receive_IT(&_USR_GSM_UART_CHANNEL, m_receiveGsmBuf, _gsm_receive_buffer_size);
        _USR_GSM_MAIN_POWER_CONTROL(f_eControl);
        _gsm_delay(1000);
        _USR_GSM_POWER_CONTROL(f_eControl);
        _gsm_delay(250);
        _USR_GSM_POWERKEY_CONTROL(f_eControl);
    }
    else
    {
        _USR_GSM_UART_RAW_DEINIT_FUNC();
        _USR_GSM_MAIN_POWER_CONTROL(f_eControl);
        _gsm_delay(1000);
        _USR_GSM_POWER_CONTROL(f_eControl);
        _gsm_delay(250);
        _USR_GSM_POWERKEY_CONTROL(f_eControl);
        SleepGsmGpioOutPinsProc(GSM_RX_GPIO_Port,           GSM_RX_Pin,           GPIO_PIN_RESET);
        SleepGsmGpioOutPinsProc(GSM_TX_GPIO_Port,           GSM_TX_Pin,           GPIO_PIN_RESET);
        SleepGsmGpioOutPinsProc(SIM_DETECT_POWER_GPIO_Port, SIM_DETECT_POWER_Pin, GPIO_PIN_RESET);
        SleepGsmGpioOutPinsProc(SIM_DETECT_GPIO_Port,       SIM_DETECT_Pin,       GPIO_PIN_RESET);
    }
    SleepGsmGpioOutPinsProc(GSM_PROCESS_STATUS_MCU_GPIO_Port, GSM_PROCESS_STATUS_MCU_Pin, GPIO_PIN_RESET);
}


bool UL_GsmModuleCheck(void)
{
    if (!ModuleSendCommandAndGetResponseProc("AT\r", "\r\nOK\r\n", 20, 1000)) // 10, 1000
        return false;
    if (!ModuleSendCommandAndGetResponseProc("ATE0\r", "\r\nOK\r\n", 3, 1000))
        return false;

    return true;
}


bool UL_GsmModuleGetInfo(S_GSM_MODULE_INFO *f_pInfo)
{
    m_sGsmModuleInfoParameters = *f_pInfo;

    int tryCount = 0;
start_step:;
    if (++tryCount > 20)
    {
        #ifdef _gsm_debug
            __logsw("UL_GsmModuleGetInfo:Try limit error\n");
        #endif
        return false;
    }

    bool checkFlg = false;
    if (ModuleSendCommandAndGetResponseProc_ayberk("AT+CSQ\r", "\r\nOK\r\n", 5, 300))
    {
        int val1, val2;
        if (sscanf(_c(m_GsmBuf), "%*[^:]: %d,%d%*[^\r\n]", &val1, &val2) == 2)
        {
            if (val1 >= 0 && val1 <= 31)
            {
                f_pInfo->signal = val1;
                checkFlg = true;
            }
        }
    }
    else
        checkFlg = false;
    ClearUartProc_ayberk();
    if (!checkFlg)
    {
        _gsm_delay(1000);
        goto start_step;
    }

    checkFlg = false;
    if (ModuleSendCommandAndGetResponseProc_ayberk("ATI\r", "\r\nOK\r\n", 5, 300))
    {
        const char *ptr = strstr(_c(m_GsmBuf), "Revision");
        if (ptr != NULL)
        {
            memset((void *)f_pInfo->moduleInfoBuf, 0, 64);
            if (sscanf(_c(ptr), "Revision: %[^\r\n]", f_pInfo->moduleInfoBuf) == 1)
                checkFlg = true;
        }
    }
    else
        checkFlg = false;
    ClearUartProc_ayberk();
    if (!checkFlg)
        goto start_step;

    checkFlg = false;
    if (ModuleSendCommandAndGetResponseProc_ayberk("AT+CGSN\r", "\r\nOK\r\n", 5, 300))
    {
        memset((void *)f_pInfo->imeiBuf, 0, 32);
        if (sscanf(_c(m_GsmBuf), "\r\n%[^\r\n]", f_pInfo->imeiBuf) == 1)
            checkFlg = true;
    }
    else
        checkFlg = false;
    ClearUartProc_ayberk();
    if (!checkFlg)
        goto start_step;

    checkFlg = false;
    if (ModuleSendCommandAndGetResponseProc_ayberk("AT+QCCID\r", "\r\nOK\r\n", 5, 300))
    {
        memset((void *)f_pInfo->iccidBuf, 0, 32);
        if (sscanf(_c(m_GsmBuf), "\r\n%[^\r\n]", f_pInfo->iccidBuf) == 1)
            checkFlg = true;
    }
    else
        checkFlg = false;
    ClearUartProc_ayberk();
    if (!checkFlg)
        goto start_step;

    return true;
}


bool UL_GsmModuleMqttInitial(const S_GSM_MQTT_CONNECTION_PARAMETERS *f_pcParameter)
{
    m_sGsmMqttConnectionParameters = *f_pcParameter;

    int tryCount = 0;
start_step:;
    if (++tryCount > 20)
    {
        #ifdef _gsm_debug
            __logsw("UL_GsmModuleMqttInitial:Try limit error\n");
        #endif
        return false;
    }

    bool checkFlg = false;
    if (ModuleSendCommandAndGetResponseProc_ayberk("AT+CREG?\r", "\r\nOK\r\n", 5, 1000))
    {
        int val1, val2;
        if (sscanf(_c(m_GsmBuf), "%*[^:]: %d,%d%*[^\r\n]", &val1, &val2) == 2)
        {
            if (val2 == 1)
                checkFlg = true;
        }
    }
    ClearUartProc_ayberk();
    if (!checkFlg)
    {
        _gsm_delay(1000);
        goto start_step;
    }

    if (m_sGsmParameters.eModuleType == cavliGsmModules)
    {
        sprintf((char *)m_GsmBuf, "AT+CGDCONT=2,\"IP\",\"%s\"\r", f_pcParameter->sGsmApn.name);
        if (!ModuleSendCommandAndGetResponseProc((const char *)m_GsmBuf, "\r\nOK\r\n", 5, 1000))
        {
            _gsm_delay(1000);
            goto start_step;
        }

        if (!ModuleSendCommandAndGetResponseProc("AT+CGACT=1,2\r", "\r\nOK\r\n", 5, 1000))
        {
            _gsm_delay(1000);
            goto start_step;
        }

        tryCount = 0;
    second_step:;
        if (++tryCount > 20)
        {
            #ifdef _gsm_debug
                __logsw("UL_GsmModuleMqttInitial:Second step try limit error\n");
            #endif
            return false;
        }

        checkFlg = false;
        if (ModuleSendCommandAndGetResponseProc("AT+CGACT?\r", "\r\nOK\r\n", 5, 1000))
        {
            int val1, val2;
            if (sscanf((const char *)m_GsmBuf, "%*[^: ]: %d,%d[^\r\n]", &val1, &val2) == 2)
            {
                if (val2 == 1)
                    checkFlg = true;
            }
        }

        if (!checkFlg)
        {
            _gsm_delay(1000);
            goto second_step;
        }

        #ifdef _gsm_debug
            __logsw("UL_GsmModuleMqttInitial:Raw ip initial ok\n");
        #endif
    }

    tryCount = 0;
    bool tcpConnectFlg = false;
third_step:;
    if (++tryCount > 20)
    {
#ifdef _gsm_debug
        __logsw("UL_GsmModuleMqttInitial:Third step try limit error\n");
#endif
        return false;
    }

    if (m_sGsmParameters.eModuleType == cavliGsmModules)
    {
        if (!ModuleSendCommandAndGetResponseProc("AT+CIPMUX=0\r", "\r\nOK\r\n", 5, 1000))
        {
            _gsm_delay(1000);
            goto third_step;
        }

        if (!ModuleSendCommandAndGetResponseProc("AT+CIPHEAD=1\r", "\r\nOK\r\n", 5, 1000))
        {
            _gsm_delay(1000);
            goto third_step;
        }

        checkFlg = false;
        sprintf((char *)m_GsmBuf, "AT+CIPSTART=\"TCP\",\"%s\",%d\r", f_pcParameter->sMqtt.urlBuf, f_pcParameter->sMqtt.port);
        if (ModuleSendCommandAndGetResponseProc((const char *)m_GsmBuf, "\r\nOK\r\n", 5, 5000))
        {
            if (strstr((const char *)m_GsmBuf, "CONNECT OK") != NULL)
                checkFlg = true;
            else if (strstr((const char *)m_GsmBuf, "ALREADY CONNECT") != NULL)
                checkFlg = true;
        }

        if (!checkFlg)
        {
            _gsm_delay(1000);
            goto third_step;
        }

        if (!ModuleSendCommandAndGetResponseProc("AT+CIPRXGET=1\r", "\r\nOK\r\n", 5, 1000))
        {
            _gsm_delay(1000);
            goto third_step;
        }
    }
    else 
    {
        sprintf((char *)m_GsmBuf, "AT+CGDCONT=3,\"IP\",\"%s\"\r", f_pcParameter->sGsmApn.name);
        _gsm_watchdog(); // Eren yazdı

        if (!ModuleSendCommandAndGetResponseProc((const char *)m_GsmBuf, "\r\nOK\r\n", 5, 1000))
        {
            _gsm_delay(1000);
            goto third_step;
        }

        if (!ModuleSendCommandAndGetResponseProc("AT+CGACT=1,1\r", "\r\nOK\r\n", 5, 1000))
        {
            _gsm_delay(1000);
            goto third_step;
        }
        /*
        if (!ModuleSendCommandAndGetResponseProc("AT+CGACT=?\r", "\r\nOK\r\n", 5, 1000))
        {
            _gsm_delay(1000);
            goto third_step;
        }
        */
        if (!ModuleSendCommandAndGetResponseProc("AT+CGPADDR=1\r", "\r\nOK\r\n", 5, 1000))
        {
            _gsm_delay(1000);
            goto third_step;
        }
        
        if (!ModuleSendCommandAndGetResponseProc("AT+CGATT=1\r", "\r\nOK\r\n", 5, 1000))
        {
            _gsm_delay(1000);
            goto third_step;
        }

        if (!ModuleSendCommandAndGetResponseProc("AT+CGREG=1\r", "\r\nOK\r\n", 5, 3000))
        {
            _gsm_delay(1000);
            goto third_step;
        }

        checkFlg = false;
        if (ModuleSendCommandAndGetResponseProc_ayberk("AT+CGACT?\r", "\r\nOK\r\n", 5, 3000))
        {
            int val1, val2;
            if (sscanf((const char *)m_GsmBuf, "%*[^: ]: %d,%d[^\r\n]", &val1, &val2) == 2)
            {
                if (val2 == 1)
                    checkFlg = true;
            }
        }
        ClearUartProc_ayberk();
        
        if (!checkFlg)
        {
            _gsm_delay(1000);
            goto third_step;
        }
/*
        if (tcpConnectFlg)
        {
            if (!ModuleSendCommandAndGetResponseProc("AT+QICLOSE\r", "\r\nOK\r\n", 5, 3000))
            {
                _gsm_delay(1000);
                goto third_step;
            }
            tcpConnectFlg = false;
        }
*/
        if (!ModuleSendCommandAndGetResponseProc("AT+QINDI=1\r", "\r\nOK\r\n", 5, 3000))
        {
            _gsm_delay(1000);
            goto third_step;
        }

        _gsm_watchdog(); // Eren yazdi
        HAL_Delay(5);
        
        sprintf((char *)m_GsmBuf, "AT+QIOPEN=\"TCP\",\"%s\",%d\r", f_pcParameter->sMqtt.urlBuf, f_pcParameter->sMqtt.port);
        if (!ModuleSendCommandAndGetResponseProc((const char *)m_GsmBuf, "\r\nOK\r\n", 1, 120000))
        {
            #ifdef _gsm_debug
                __logse("UL_GsmModuleMqttInitial:Tcp connection error step 1\n");
            #endif
            return false;
        }

        // if (!ModuleListenResultProc("\r\n+QIOPEN: 0,0\r\n", 30000))
        if (!ModuleListenResultProc("CONNECT OK", 30000))
        {
            #ifdef _gsm_debug
                __logse("UL_GsmModuleMqttInitial:Tcp connection error step 2\n");
            #endif
            return false;
        }
    }

    #ifdef _gsm_debug
        __logsi("UL_GsmModuleMqttInitial:Tcp connection ok\n");
    #endif

    ClearUartProc_ayberk();
    if (!MqttConnectionProc(f_pcParameter))
    {
        #ifdef _gsm_debug
            __logse("UL_GsmModuleMqttInitial:Mqtt connection error\n");
        #endif
        return false;
    }

    m_eMqttConnectionOkFlg = true;
    UL_GsmModuleMqttConnectionStatusCallback(connectGsmMqttConnectionStatus);

    return true;
}


bool UL_GsmModuleMqttSubcribeTopic(const char *f_cpTopic, int f_qos)
{
    if (!m_eMqttConnectionOkFlg)
    {
        #ifdef _gsm_debug
            __logsw("UL_GsmModuleMqttSubcribeTopic:Mqtt connection not ok\n");
        #endif
        return false;
    }

    bool res = false;
    
    uint8_t *ptr = (uint8_t *)_gsm_malloc(_gsm_mqtt_subcribe_buffer);
    if (ptr != NULL)
    {
        MQTTString topicString = MQTTString_initializer;
        topicString.cstring = (char *)f_cpTopic;

        int len = MQTTSerialize_subscribe(ptr, _gsm_mqtt_subcribe_buffer, 0, 1, 1, &topicString, &f_qos);
        if (TcpSendDataProc((const uint8_t *)ptr, len))
        {
            int timeout = HAL_GetTick();
            while ((HAL_GetTick() - timeout) < 5000)
            {
                if (TcpGetRawDataProc() == -1)
                {
                    _gsm_free(ptr);
                    return false;
                }
                if (MQTTPacket_read(ptr, _gsm_mqtt_subcribe_buffer, TcpGetDataProc) == SUBACK)
                {
                    unsigned short submsgid;
                    int subcount;
                    int grantedQos;

                    MQTTDeserialize_suback(&submsgid, 1, &subcount, &grantedQos, ptr, _gsm_mqtt_subcribe_buffer);

                    if (grantedQos != 0)
                    {
                        #ifdef _gsm_debug
                            __logsw("UL_GsmModuleMqttSubcribeTopic:Subcribe ack error\n");
                        #endif
                        _gsm_free(ptr);
                        return false;
                    }
                    else
                    {
                        res = true;
                        break;
                    }
                }
            }
        }
        _gsm_free(ptr);
    }
    return res;
}


bool UL_GsmModuleMqttPublishTopic(const char *f_cpTopic, const char *f_cpData, int f_qos, int f_retain)
{
    if (!m_eMqttConnectionOkFlg)
    {
        #ifdef _gsm_debug
            __logsw("UL_GsmModuleMqttPublishTopic:Mqtt connection not ok\n");
        #endif
        return false;
    }
    
    // Eren yazdi
    memset((void *)m_GsmBuf, 0, _gsm_receive_buffer_size); 
    bool res = false;
    MQTTString topicString = MQTTString_initializer;
    topicString.cstring = (char *)f_cpTopic;
    int len = MQTTSerialize_publish(m_GsmBuf, 512, 0, f_qos, 0, 0, topicString, (unsigned char *)f_cpData, _size(f_cpData)); // _gsm_mqtt_publish_buffer, 256 dene
    memcpy(m_HelperBuf, m_GsmBuf, 512  );
    if (TcpSendDataProc((const uint8_t *)m_HelperBuf, len))
            res = true;
    /*
    bool res = false;
    uint8_t *ptr = (uint8_t *)_gsm_malloc(256);   // _gsm_mqtt_publish_buffer, 256 dene
    if (ptr != NULL)
    {
        MQTTString topicString = MQTTString_initializer;
        topicString.cstring = (char *)f_cpTopic;
        int len = MQTTSerialize_publish(ptr, 256, 0, f_qos, 0, 0, topicString, (unsigned char *)f_cpData, _size(f_cpData)); // _gsm_mqtt_publish_buffer, 256 dene
        if (TcpSendDataProc((const uint8_t *)ptr, len))
            res = true;
        _gsm_free(ptr);
    }
    */
    return res;
}


void UL_GsmModuleMqttGeneral(void)
{
    if (!m_eMqttConnectionOkFlg)
        return;

    _io uint32_t _mqttCheckDataTimeout = 0;
    if (_mqttCheckDataTimeout == 0)
        _mqttCheckDataTimeout = HAL_GetTick();

    TcpGetRawDataProc();
    //TcpGetStoredDataTriggerReadProc(&_mqttCheckDataTimeout);
    MqttKeepaliveProc();

    if (m_tcpGsmUartCnt != 0)
    {
        /*
        uint8_t *ptr = (uint8_t *)_gsm_malloc(_gsm_receive_buffer_size);                       
        if (ptr != NULL)
        {                                                                                      
            memset((void *)ptr, 0, _gsm_receive_buffer_size);                                  
            if (MQTTPacket_read(ptr, _gsm_receive_buffer_size, TcpGetDataProc) == PUBLISH)
            {
                MQTTString received;
                unsigned char dup;
                int qos;
                unsigned char retained;
                unsigned short msgid;
                int payloadLenIn;
                unsigned char *payloadIn;

                int res = MQTTDeserialize_publish(&dup, &qos, &retained, &msgid, &received, &payloadIn, &payloadLenIn, ptr, _gsm_receive_buffer_size);
                if (res == 1)
                {
                    if (received.lenstring.len >= _gsm_mqtt_max_topic_size || payloadLenIn >= _gsm_mqtt_max_payoad_size)
                    {
                        m_tcpCurrentLen = 0;
                        m_tcpGsmUartCnt = 0;
                        TcpGetStoredDataProc();
                        _gsm_free(ptr);
                        return;
                    }
                    _mqttCheckDataTimeout = HAL_GetTick();
                    UL_GsmModuleMqttSubcribeDataCallback(received.lenstring.data, received.lenstring.len, (const char*)payloadIn, payloadLenIn);
                }
            }
            _gsm_free(ptr);
        }
        */


        memset((void *)m_GsmBuf, 0, _gsm_receive_buffer_size);                                 // _gsm_receive_buffer_size yaziyordu, sizeof(m_mqttDataBuf) Eren yazdi                         
        if (MQTTPacket_read(m_GsmBuf, _gsm_receive_buffer_size, TcpGetDataProc) == PUBLISH)    // _gsm_receive_buffer_size yaziyordu, sizeof(m_mqttDataBuf) Eren yazdi
        {
            MQTTString received;
            unsigned char dup;
            int qos;
            unsigned char retained;
            unsigned short msgid;
            int payloadLenIn;
            unsigned char *payloadIn;

            int res = MQTTDeserialize_publish(&dup, &qos, &retained, &msgid, &received, &payloadIn, &payloadLenIn, m_GsmBuf, _gsm_receive_buffer_size);
            if (res == 1)
            {
                if (received.lenstring.len >= _gsm_mqtt_max_topic_size || payloadLenIn >= _gsm_mqtt_max_payload_size)
                {
                    m_tcpCurrentLen = 0;
                    m_tcpGsmUartCnt = 0;
                    TcpGetStoredDataProc();
                    return;
                }
                _mqttCheckDataTimeout = HAL_GetTick();
                UL_GsmModuleMqttSubcribeDataCallback(received.lenstring.data, received.lenstring.len, (const char *)payloadIn, payloadLenIn);
                
                // Bu asagi tarafi Eren yazdi 
                
                size_t responseLength = strlen(received.lenstring.data);
                size_t destinationSize = sizeof(m_HelperBuf);

                if(responseLength < destinationSize)
                {
                    memcpy(m_HelperBuf, received.lenstring.data, destinationSize);
                    m_HelperBuf[responseLength] = '\0';
                    responseSubcribeDataCallbackFlag = true;
                }
                
            }
          }
    }
}


void UL_GsmModuleMqttClosed(void)
{
    m_eMqttConnectionOkFlg = false;

    if (m_sGsmParameters.eModuleType == cavliGsmModules)
        ModuleSendCommandAndGetResponseProc("AT+CIPCLOSE\r", "OK", 3, 1000);
    else
        ModuleSendCommandAndGetResponseProc("AT+QICLOSE\r", "\r\nOK\r\n", 3, 1000);
}


void UL_GsmModuleUartInterruptCallback(void)
{
    if (m_sGsmParameters.pUart == NULL)
        return;
    if (m_sGsmParameters.pUart->Instance->ISR & ((uint32_t)0x1f))
    {
        if ((m_sGsmParameters.pUart->Instance->ISR & 0x10) == 0)
            HAL_UART_Receive_IT(m_sGsmParameters.pUart, m_receiveGsmBuf, _gsm_receive_buffer_size);   // esas yazan: _gsm_receive_buffer_size
        m_sGsmParameters.pUart->Instance->ICR |= 0x1f;
        m_eReceiveGsmDataCameFlg = true;                 // Bu flag UartParserProc fonksiyonuna gidecek
    }
}


void UL_GsmModuleHardwareReset(void)
{
    ModuleResetProc();
    _gsm_delay(5000);
}


int UL_GsmModuleReadFile(const char *f_cpFileName, uint32_t f_startIndex, uint32_t f_size, uint8_t *f_pData)
{
#ifdef _4GGSM
    int res = -1;

    sprintf(m_transmitGsmBuf, "AT+FSRDBLOCK=\"%s\",%d,%d\r", f_cpFileName, f_startIndex, f_size);
    ClearUartProc();
    HAL_UART_Transmit(m_sGsmParameters.pUart, (const uint8_t *)m_transmitGsmBuf, strlen((const char *)m_transmitGsmBuf), 5 * strlen((const char *)m_transmitGsmBuf));

    int timeout = HAL_GetTick();
    sprintf(m_transmitGsmBuf, "\r\n+FSRDBLOCK: %s,%%d,", f_cpFileName);
    while ((HAL_GetTick() - timeout) < 2000)
    {
        UartParserProc();
        int len;
        if (sscanf((const char *)m_receiveGsmEndBuf, (const char *)m_transmitGsmBuf, &len) == 1)
        {
            char tempBuf[64];
            sprintf(tempBuf, "\r\n+FSRDBLOCK: %s,%d,", f_cpFileName, len);
            int totalLenght = _size(tempBuf) + len + 8; //// \r\n\r\nOK\r\n
            if (m_receiveGsmUartCnt >= totalLenght)
            {
                char *ptr;
                if ((ptr = strstr(_c(m_receiveGsmEndBuf), _c(tempBuf))) != NULL)
                {
                    int tempBuflen = _size(tempBuf);
                    for (uint16_t i = 0; i < len; i++)
                        f_pData[i] = ptr[tempBuflen + i];
                    res = len;
                    break;
                }
            }
        }
    }

    return res;
#endif
    int val1, val2, val3, new_val1, new_val2;

    #ifdef _gsm_debug
        __logsi("UL_GsmModuleReadFile:Read File  connection ok\n");
    #endif

    sprintf((char *)m_GsmBuf, "AT+QFOPEN=\"%s\"\r", f_cpFileName);
    if (ModuleSendCommandAndGetResponseProc(_c(m_GsmBuf), "\r\nOK\r\n", 2, 3000))
    {
        if (sscanf(_c(m_GsmBuf), "%*[^:]: %d%*[^\r\n]", &val1) == 1)
            new_val1 = val1;
    }

    
    memset((void *)m_GsmBuf, 0, sizeof(m_GsmBuf));
    sprintf((char *)m_GsmBuf, "AT+QFSEEK=%d,%d,0\r", new_val1, f_startIndex);
    if (!ModuleSendCommandAndGetResponseProc(_c(m_GsmBuf), "\r\nOK\r\n", 5, 3000))
        return false;

    sprintf((char *)m_GsmBuf, "AT+QFREAD=%d\r", new_val1);
    if (!ModuleSendCommandAndGetResponseProc(_c(m_GsmBuf), "\r\nOK\r\n", 5, 3000))
        return false;

    sprintf((char *)m_GsmBuf, "AT+QFCLOSE=%d\r", new_val1);
    if (!ModuleSendCommandAndGetResponseProc(_c(m_GsmBuf), "\r\nOK\r\n", 5, 3000))
        return false;

    return true;
}

int UL_GsmModuleGetFileTotalLen(const char *f_cpFileName)
{
#ifdef _4GGSM
    int res = -1;
    sprintf(m_transmitGsmBuf, "AT+FSLSTFILE=2,\"%s\"\r", f_cpFileName);
    ClearUartProc();
    HAL_UART_Transmit(m_sGsmParameters.pUart, (const uint8_t *)m_transmitGsmBuf, strlen((const char *)m_transmitGsmBuf), 5 * strlen((const char *)m_transmitGsmBuf));

    int timeout = HAL_GetTick();
    while ((HAL_GetTick() - timeout) < 2000)
    {
        int len;
        UartParserProc();
        if (sscanf((const char *)m_receiveGsmEndBuf, "\r\n+FSLSTFILE: %d\r\n\r\nOK\r\n", &len) == 1)
            return len;
    }
    return res;
#endif

    sprintf((char *)m_GsmBuf, "AT+QFLDS=\"%s\"\r", LOCALPOSITION);
    if (!ModuleSendCommandAndGetResponseProc((const char *)m_GsmBuf, "\r\nOK\r\n", 1, 3000))
        ;

    memset((void *)m_GsmBuf, 0, sizeof(m_GsmBuf));
    sprintf((char *)m_GsmBuf, "AT+QFLST\r");
    if (!ModuleSendCommandAndGetResponseProc((const char *)m_GsmBuf, "\r\nOK\r\n", 1, 3000))
        ;

    return 1;
}

bool UL_GsmModuleFtpFileDownload(const S_GSM_FTP *f_cpFtp)
{
  return 0;
}


bool UL_GsmModuleDeleteFile(const char *f_cpFileName)
{
    if (m_sGsmParameters.eModuleType == cavliGsmModules)
        sprintf((char *)m_GsmBuf, "AT+FSDELFILE=\"%s\"\r", f_cpFileName);
    else
        sprintf((char *)m_GsmBuf, "AT+QFDEL=\"%s\"\r", f_cpFileName);
    if (!ModuleSendCommandAndGetResponseProc(_c(m_GsmBuf), "\r\nOK\r\n", 5, 1000))
        return false;

    return ModuleSendCommandAndGetResponseProc((const char *)m_GsmBuf, "\r\nOK\r\n", 5, 1000);
}


_io bool ModuleSendCommandAndGetResponseProc(const char *f_pcCommand, const char *f_pcResponse, uint8_t f_tryCount, uint32_t f_timeout)
{
    bool res = false;
    int tryCount = 0;

start_step:;
    if (++tryCount > f_tryCount)
    {
        #ifdef _gsm_debug
            __logsw("ModuleSendCommandAndGetResponseProc:At command error : %s\n", f_pcCommand);
        #endif
        return false;
    }
    // m_receiveGsmUartCnt = 0;    // Eren acti
    ClearUartProc();
    #ifdef _gsm_debug
        __logsw("ModuleSendCommandAndGetResponseProc:Transmit command [%d]: %s\n", strlen(f_pcCommand), f_pcCommand);
    #endif
    m_eReceiveGsmDataCameOkFlg = false;
    #ifdef _gsm_debug
        __logse("Command : %s", f_pcCommand);
    #endif
    HAL_UART_Transmit(m_sGsmParameters.pUart, (const uint8_t *)f_pcCommand, strlen(f_pcCommand), 5 * strlen(f_pcCommand));
    
    int timeout = HAL_GetTick();
    while ((HAL_GetTick() - timeout) < f_timeout)
    {
        UartParserProc();
        if (m_eReceiveGsmDataCameOkFlg)
        {
            if (strstr((const char *)m_GsmBuf, f_pcResponse))
            {
                res = true;
                break;
            }
            else if (strstr((const char *)m_GsmBuf, "ERROR"))
            {
                res = false;
                // responseSubcribeDataCallbackFlag = false;    // Eren yazdi      
                break;
            }
            m_eReceiveGsmDataCameOkFlg = false;
        }
    }
    if (!res)
        goto start_step;
    
    ClearUartProc_ayberk();
    return res;
}

bool ModuleSendCommandAndGetResponseProc_ayberk(const char *f_pcCommand, const char *f_pcResponse, uint8_t f_tryCount, uint32_t f_timeout)
{
    bool res = false;
    int tryCount = 0;

start_step:;
    if (++tryCount > f_tryCount)
    {
        #ifdef _gsm_debug
            __logsw("ModuleSendCommandAndGetResponseProc:At command error : %s\n", f_pcCommand);
        #endif
        return false;
    }
    // m_receiveGsmUartCnt = 0;    // Eren acti
    ClearUartProc();
    #ifdef _gsm_debug
        __logsw("ModuleSendCommandAndGetResponseProc:Transmit command [%d]: %s\n", strlen(f_pcCommand), f_pcCommand);
    #endif
    m_eReceiveGsmDataCameOkFlg = false;
    #ifdef _gsm_debug
        __logse("Command : %s", f_pcCommand);
    #endif
    HAL_UART_Transmit(m_sGsmParameters.pUart, (const uint8_t *)f_pcCommand, strlen(f_pcCommand), 5 * strlen(f_pcCommand));
    
    int timeout = HAL_GetTick();
    while ((HAL_GetTick() - timeout) < f_timeout)
    {
        UartParserProc();
        if (m_eReceiveGsmDataCameOkFlg)
        {
            if (strstr((const char *)m_GsmBuf, f_pcResponse))
            {
                res = true;
                break;
            }
            else if (strstr((const char *)m_GsmBuf, "ERROR"))
            {
                res = false;
                // responseSubcribeDataCallbackFlag = false;    // Eren yazdi      
                break;
            }
            m_eReceiveGsmDataCameOkFlg = false;
        }
    }
    if (!res)
        goto start_step;
    return res;
}


_io bool ModuleSendCommandWithLenAndGetResponseProc(const uint8_t *f_pcCommand, uint16_t f_len, const char *f_pcResponse, uint8_t f_tryCount, uint32_t f_timeout)
{
    bool res = false;
    int tryCount = 0;

start_step:;
    if (++tryCount > f_tryCount)
    {
        #ifdef _gsm_debug
            __logsw("ModuleSendCommandWithLenAndGetResponseProc:At command error \n");
        #endif
        return false;
    }
    // m_receiveGsmUartCnt = 0;
    ClearUartProc();
    // HAL_UART_Abort_IT(m_sGsmParameters.pUart);
    // HAL_UART_Receive_IT(m_sGsmParameters.pUart, m_receiveGsmBuf, _gsm_receive_buffer_size);

    m_eReceiveGsmDataCameOkFlg = false;
    HAL_UART_Transmit(m_sGsmParameters.pUart, (const uint8_t *)f_pcCommand, f_len, f_len);

    int timeout = HAL_GetTick();

    while ((HAL_GetTick() - timeout) < f_timeout)
    {
        UartParserProc();
        if (m_eReceiveGsmDataCameOkFlg)
        {
            #ifdef _gsm_debug
                __logsw("Recevive : %s", m_receiveGsmEndBuf);
            #endif
            if (strstr((const char *)m_GsmBuf, f_pcResponse))
            {
                res = true;
                break;
            }
            m_eReceiveGsmDataCameOkFlg = false;
        }
    }
    if (!res)
        goto start_step;
    return res;
}


//_io 
bool MqttConnectionProc(const S_GSM_MQTT_CONNECTION_PARAMETERS *f_pcParameter)
{
    m_sGsmMqttConnectionParameters = *f_pcParameter;
    unsigned char buf[128] = {0};
    MQTTPacket_connectData mqtt_packet = MQTTPacket_connectData_initializer;
    mqtt_packet.MQTTVersion = 4;
    mqtt_packet.username.cstring = (char *)f_pcParameter->sMqtt.usernameBuf;
    mqtt_packet.password.cstring = (char *)f_pcParameter->sMqtt.passwordBuf;
    mqtt_packet.clientID.cstring = (char *)f_pcParameter->sMqtt.randomIdBuf;
    mqtt_packet.keepAliveInterval = f_pcParameter->sMqtt.keepAlive;
    mqtt_packet.cleansession = 1;
    m_mqttKeepAliveTime = mqtt_packet.keepAliveInterval;

    int len = MQTTSerialize_connect(buf, sizeof(buf), &mqtt_packet);

    if (TcpSendDataProc((const uint8_t *)buf, len))
    {
        int timeout = HAL_GetTick();
        while ((HAL_GetTick() - timeout) < 30000) 
        {
            if (TcpGetRawDataProc() == -1)
                return false;
    
            if (MQTTPacket_read(buf, 128, TcpGetDataProc) == CONNACK)
            {
                unsigned char sessionResent, connack_rc;
                int res = MQTTDeserialize_connack(&sessionResent, &connack_rc, buf, 128);

                if (res != 1 || connack_rc != 0)
                {
                    #ifdef _gsm_debug
                        __logse("MqttConnectionProc:Mqtt connection ack error\n");
                    #endif
                    return false;
                }
                else
                    return true;
            }
        }
    }
    return false;
}


_io void ClearUartProc(void)
{
    m_eReceiveGsmDataCameFlg = false;
    m_eReceiveGsmDataCameOkFlg = false;
    m_receiveGsmUartCnt = 0;
    //m_GsmBuf[0] = '\0';
    //memset(m_GsmBuf,0,512);
    HAL_UART_Abort_IT(m_sGsmParameters.pUart);
    HAL_UART_Receive_IT(m_sGsmParameters.pUart, m_receiveGsmBuf, _gsm_receive_buffer_size);   // _gsm_receive_buffer_size
}


_io void ClearUartProc_ayberk(void)
{
    m_eReceiveGsmDataCameFlg = false;
    m_eReceiveGsmDataCameOkFlg = false;
    m_receiveGsmUartCnt = 0;
    //m_GsmBuf[0] = '\0';
    memset(m_GsmBuf,0,512);
    HAL_UART_Abort_IT(m_sGsmParameters.pUart);
    HAL_UART_Receive_IT(m_sGsmParameters.pUart, m_receiveGsmBuf, _gsm_receive_buffer_size);   // _gsm_receive_buffer_size
}


_io void UartParserProc(void)
{
    if (m_eReceiveGsmDataCameFlg)
    {
        _gsm_watchdog(); // Eren yazdi
        int currentUartCount = m_sGsmParameters.pUart->RxXferSize - m_sGsmParameters.pUart->RxXferCount;

        if ((m_receiveGsmUartCnt + currentUartCount) >= _gsm_receive_buffer_size)
        {
            m_receiveGsmUartCnt = 0;
            #ifdef _gsm_debug
                __logsw("UartParserProc: Clear counter : %d-%d\n", m_receiveGsmUartCnt, currentUartCount);
            #endif
            goto end_step;
        }
        memcpy((void *)&m_GsmBuf[m_receiveGsmUartCnt], (const void *)m_receiveGsmBuf, currentUartCount);
        m_receiveGsmUartCnt += currentUartCount;
        m_GsmBuf[m_receiveGsmUartCnt] = 0;               // Null ile sonlandır.
        #ifdef _gsm_debug
            __logsw("Coming data : %s", m_GsmBuf);
        #endif
        m_eReceiveGsmDataCameOkFlg = true;
    end_step:;
        HAL_UART_Abort_IT(m_sGsmParameters.pUart);
        HAL_UART_Receive_IT(m_sGsmParameters.pUart, m_receiveGsmBuf, _gsm_receive_buffer_size);  // esas yazan: _gsm_receive_buffer_size //
        m_eReceiveGsmDataCameFlg = false;
    }
}


_io bool TcpSendDataProc(const uint8_t *f_pData, uint16_t f_len)
{
    if (m_sGsmParameters.eModuleType == cavliGsmModules)
        sprintf((char *)m_GsmBuf, "AT+CIPSEND=%d\r", f_len);
    else
        sprintf((char *)m_GsmBuf, "AT+QISEND=%d\r", f_len);
    if (!ModuleSendCommandAndGetResponseProc((const char *)m_GsmBuf, ">", 1, 5000))
        return false;
    ClearUartProc_ayberk();
    memcpy((void *)m_GsmBuf, (const void *)f_pData, f_len);
    m_GsmBuf[f_len] = 0x1a;
    if (!ModuleSendCommandWithLenAndGetResponseProc((const uint8_t *)m_GsmBuf, f_len + 1, "SEND OK", 1, 5000))
    {
        #ifdef _gsm_debug
            __logse("usr_lib_gsm: TcpSendDataProc: send data error \n");
        #endif
        return false;
    }
    return true;
}


_io int TcpGetRawDataProc(void)
{
    UartParserProc();
    if (m_receiveGsmUartCnt != 0)
    {
        if (m_sGsmParameters.eModuleType == cavliGsmModules)
        {
            if (strstr((const char *)&m_GsmBuf, "\r\nCLOSED\r\n") != NULL)
            {
                m_eMqttConnectionOkFlg = false;
                UL_GsmModuleMqttConnectionStatusCallback(disconnectGsmMqttConnectionStatus);
                m_receiveGsmUartCnt = 0;
                return -1;
            }
            else if (strstr((const char *)&m_GsmBuf, "\r\n+CIPRXGET\r\n") != NULL)
            {
                TcpGetStoredDataProc();
                m_receiveGsmUartCnt = 0;
                return 0;
            }
        }
        else
        {
            if (strstr((const char *)&m_GsmBuf, "\r\nCLOSED\r\n") != NULL)
            {
                m_eMqttConnectionOkFlg = false;
                UL_GsmModuleMqttConnectionStatusCallback(disconnectGsmMqttConnectionStatus);
                m_receiveGsmUartCnt = 0;
                return -1;
            }
            else if (strstr((const char *)&m_GsmBuf, "+QIRDI") != NULL)
            {
                TcpGetStoredDataProc();
                m_receiveGsmUartCnt = 0;
                return 0;
            }
        }
        m_receiveGsmUartCnt = 0;
        ClearUartProc_ayberk();
        
    }
    return 0;
}


_io int TcpGetDataProc(unsigned char *f_pData, int f_len)
{
    uint8_t *ptr = (uint8_t *)f_pData;
    int r = 0;

    if (m_tcpGsmUartCnt != 0)
    {
        if (f_len >= m_tcpGsmUartCnt)
            f_len = m_tcpGsmUartCnt;

        r = f_len;
        for (int i = 0; i < r; i++)
            ptr[i] = m_HelperBuf[m_tcpCurrentLen + i];

        m_tcpCurrentLen += r;

        if (m_tcpCurrentLen >= m_tcpGsmUartCnt)
        {
            m_tcpCurrentLen = 0;
            m_tcpGsmUartCnt = 0;
        }
    }
    return r;
}


_io void TcpGetStoredDataTriggerReadProc(uint32_t *f_pTimerCnt)
{
    if ((HAL_GetTick() - *f_pTimerCnt) > (m_sGsmMqttConnectionParameters.sMqtt.keepAlive / 2))    // (m_sGsmMqttParameters.keepAlive / 2)   // eskiden: 30000 yaziyordu
    {
        *f_pTimerCnt = HAL_GetTick();
        TcpConnectionStatus();
        TcpGetStoredDataProc();
        m_receiveGsmUartCnt = 0;
    }
}


_io void TcpGetStoredDataProc(void)
{
    //memcpy(m_HelperBuf,m_GsmBuf,512);
    char *ptr;
    int val = 0;
    if (m_sGsmParameters.eModuleType == cavliGsmModules)
    {
        if (!ModuleSendCommandAndGetResponseProc((const char *)"AT+CIPRXGET=2,1024\r", "\r\n+IPD", 3, 1000))
            return;
        ptr = strstr((const char *)&m_GsmBuf, "\r\n+IPD");
    }
    else
    {
        /* 4 satiri Eren yazdi */ 

        sprintf((char *)m_GsmBuf, "AT+QIRD=0,1,0,1500\r");
        if (!ModuleSendCommandAndGetResponseProc_ayberk(_c(m_GsmBuf), "+QIRD:", 5, 3000))
            return;
        ptr = strstr((const char *)&m_GsmBuf, "+QIRD:");
    }

    if (ptr != NULL)
    {
        if (m_sGsmParameters.eModuleType == cavliGsmModules)
        {
            if (sscanf((const char *)ptr, "%*[^,],%d:%*[]", &val) == 1)
            {
                if (val < _gsm_receive_buffer_size && val <= m_receiveGsmUartCnt && val != 0)
                {
                    char buf[32];
                    sprintf(buf, "\r\n+IPD,%d:", val);
                    ptr += strlen((const char *)buf);

                    if ((m_tcpGsmUartCnt + val) >= _gsm_receive_buffer_size)
                    {
                        m_tcpCurrentLen = 0;
                        m_tcpGsmUartCnt = 0;
                    }

                    for (int i = 0; i < val; i++)
                        m_HelperBuf[m_tcpGsmUartCnt + i] = ptr[i];
                    m_tcpGsmUartCnt += val;
                }
                else
                {
                    m_tcpCurrentLen = 0;
                    m_tcpGsmUartCnt = 0;
                }
            }
            else
            {
                m_tcpCurrentLen = 0;
                m_tcpGsmUartCnt = 0;
            }
        }
        else
        {
            if (sscanf((const char *)ptr, "%*[^,],TCP,%d\r\n", &val) == 1)
            {
                if (val < _gsm_receive_buffer_size && val <= m_receiveGsmUartCnt && val != 0)
                {
                    uint16_t m = 0;
                    for (; m <= m_receiveGsmUartCnt; m++)
                    {
                        if (*ptr == '\n')
                        {
                            ptr++;
                            break;
                        }
                        ptr++;
                    }
                    if (m == (m_receiveGsmUartCnt + 1))
                    {
                        m_tcpCurrentLen = 0;
                        m_tcpGsmUartCnt = 0;
                        return;
                    }
                    if ((m_tcpGsmUartCnt + val) >= _gsm_receive_buffer_size)
                    {
                        m_tcpCurrentLen = 0;
                        m_tcpGsmUartCnt = 0;
                    }

                    for (int i = 0; i < val; i++)
                      m_HelperBuf[m_tcpGsmUartCnt + i] = ptr[i];
                    m_tcpGsmUartCnt += val;
                }
                else
                {
                    m_tcpCurrentLen = 0;
                    m_tcpGsmUartCnt = 0;
                }
            }
            else
            {
                m_tcpCurrentLen = 0;
                m_tcpGsmUartCnt = 0;
            }
        }
    }
}


_io void TcpConnectionStatus(void)
{
    _io uint8_t _connectionCheckCnt = 0;

    if (!m_eMqttConnectionOkFlg)
        return;

    if (_connectionCheckCnt > 10)     // esas deger: 10 // ara ara degistirdim
    {
        _connectionCheckCnt = 0;      // Mqtt yi bitiren kisim
        m_eMqttConnectionOkFlg = false;
        UL_GsmModuleMqttConnectionStatusCallback(disconnectGsmMqttConnectionStatus);
        return;
    }

    if (m_sGsmParameters.eModuleType == cavliGsmModules)
    {
        if (!ModuleSendCommandAndGetResponseProc((const char *)"AT+CIPSTATUS\r", "\r\nOK\r\n", 3, 1000))
        {
            ++_connectionCheckCnt;
            return;
        }

        if (strstr((const char *)m_GsmBuf, "CONNECTED") != NULL)
        {
            _connectionCheckCnt = 0;
            return;
        }
        else if (strstr((const char *)m_GsmBuf, "CLOSING") != NULL ||
                 strstr((const char *)m_GsmBuf, "CLOSED") != NULL ||
                 strstr((const char *)m_GsmBuf, "PDP DEACT") != NULL)
        {
            m_eMqttConnectionOkFlg = false;
            UL_GsmModuleMqttConnectionStatusCallback(disconnectGsmMqttConnectionStatus);
            m_receiveGsmUartCnt = 0;
        }
        else
            ++_connectionCheckCnt;
    }
    else
    {
        ClearUartProc_ayberk();
        if (!ModuleSendCommandAndGetResponseProc((const char *)"AT+QISTAT\r", "\r\nOK\r\n", 3, 1000))
        {
            ++_connectionCheckCnt;
            #ifdef _gsm_debug
                __logsi("STATE ERROR !");
            #endif
            return;
        }

        /*
        // Burada response olarak STATE OK vb. bir sey geliyor, TCP ile alakali bi sey yok
        char *ptr = strstr((const char *)m_receiveGsmEndBuf, "TCP");
        if (ptr != NULL)
        {
            ptr += 6; // to show "TCP","
            int remotePort = 0, localePort = 0, conStat = 0;
            if (sscanf((const char *)ptr, "%*[\"]\",%d,%d,%d,%*[]", &remotePort, &localePort, &conStat) == 3)
            {
                if (conStat == 2)
                {
                    _connectionCheckCnt = 0;
                    return;
                }
                else if (conStat == 4)
                {
                    m_eMqttConnectionOkFlg = false;
                    UL_GsmModuleMqttConnectionStatusCallback(disconnectGsmMqttConnectionStatus);
                    m_receiveGsmUartCnt = 0;
                    return;
                }
            }
        }
        
        // Bunu Eren yoruma aldı
        // ++_connectionCheckCnt;
        */
    }
}


_io int ModuleListenResultsProc(const char *f_pList, uint8_t f_totalNumber, uint32_t f_timeout)
{
    // m_receiveGsmUartCnt = 0;
    ClearUartProc();
    // HAL_UART_Abort_IT(m_sGsmParameters.pUart);
    // HAL_UART_Receive_IT(m_sGsmParameters.pUart, m_receiveGsmBuf, _gsm_receive_buffer_size);
    int timeout = HAL_GetTick();
    while ((HAL_GetTick() - timeout) < f_timeout)
    {
        UartParserProc();
        if (m_eReceiveGsmDataCameOkFlg)
        {
            for (uint8_t i = 0; i < f_totalNumber; i++)
            {
                if (strstr((const char *)m_GsmBuf, (char *)f_pList[i]))
                    return i;
            }
            m_eReceiveGsmDataCameOkFlg = false;
        }
    }
    return -1;
}


//_io 
int ModuleListenResultProc(const char *f_pRes, uint32_t f_timeout)
{
    _gsm_watchdog();
    m_receiveGsmUartCnt = 0;
    HAL_UART_Abort_IT(m_sGsmParameters.pUart);
    HAL_UART_Receive_IT(m_sGsmParameters.pUart, m_receiveGsmBuf, _gsm_receive_buffer_size); // _gsm_receive_buffer_size
    int timeout = HAL_GetTick();
    while ((HAL_GetTick() - timeout) < f_timeout)
    {
        UartParserProc();
        if (m_eReceiveGsmDataCameOkFlg)
        {
            if (strstr((const char *)m_GsmBuf, (const char *)f_pRes))
                return true;
            m_eReceiveGsmDataCameOkFlg = false;
        }
    }
    return false;
}


_io void MqttKeepaliveProc(void)
{
    _io uint32_t _mqttKeepAliveTimer = 0;

    if (((HAL_GetTick() - _mqttKeepAliveTimer) / 1000) < (m_mqttKeepAliveTime / 2))
        return;

    _mqttKeepAliveTimer = HAL_GetTick();
    UL_GsmModuleMqttPublishTopic("keepalive", "a", 0, 0);
}


_io void ModuleResetProc(void)
{
    if (m_sGsmParameters.eModuleType != quectelM65GsmModule && m_sGsmParameters.eModuleType != quectelM66GsmModule)
    {
        HAL_GPIO_WritePin(m_sGsmParameters.pResetPort, m_sGsmParameters.resetPin, (GPIO_PinState)m_sGsmParameters.resetPinEnableStatus);
        _gsm_delay(500);
        HAL_GPIO_WritePin(m_sGsmParameters.pResetPort, m_sGsmParameters.resetPin, (GPIO_PinState)!m_sGsmParameters.resetPinEnableStatus);
        _gsm_delay(500);
    }
    else
    {
        HAL_GPIO_WritePin(m_sGsmParameters.pPowerkeyPort, m_sGsmParameters.gsmProcessMcuPin, (GPIO_PinState)!m_sGsmParameters.gsmProcessMcuStatus);
        _gsm_delay(500);
        HAL_GPIO_WritePin(m_sGsmParameters.pPowerkeyPort, m_sGsmParameters.powerKeyPin, (GPIO_PinState)!m_sGsmParameters.powerPinEnableStatus);
        _gsm_delay(500);
        HAL_GPIO_WritePin(m_sGsmParameters.pPowerkeyPort, m_sGsmParameters.powerKeyPin, (GPIO_PinState)!m_sGsmParameters.powerKeyPinEnableStatus);
        _gsm_delay(500);
    }
}


__attribute__((weak)) void UL_GsmModuleMqttConnectionStatusCallback(EGsmMqttConnectionStatus f_eStatus)
{
#ifdef _gsm_debug
    __logsw("UL_GsmModulePPPMqttConnectionStatusCallback:Mqtt connection status : %d\n", f_eStatus);
#endif
}


__attribute__((weak)) void UL_GsmModuleMqttSubcribeDataCallback(const char *f_pTopic, uint16_t f_topicLen, const char *f_pPayload, uint16_t f_payloadLen)
{
#ifdef _gsm_debug
    __logsw("UL_GsmModuleMqttSubcribeDataCallback: Toppic len : %d Data len : %d\n", f_topicLen, f_payloadLen);
    __logsw("UL_GsmModuleMqttSubcribeDataCallback:  Topic: %.*s  Payload: %.*s\n", f_topicLen, f_pTopic, f_payloadLen, f_pPayload);
#endif
}


_io void SleepGsmGpioOutPinsProc(GPIO_TypeDef *f_pGpio, uint16_t f_pinGroup, GPIO_PinState f_ePinstate)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    HAL_GPIO_WritePin(f_pGpio, f_pinGroup, f_ePinstate);
    GPIO_InitStruct.Pin = f_pinGroup;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(f_pGpio, &GPIO_InitStruct);
}