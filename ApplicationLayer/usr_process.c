#include "usr_general.h"

#define _USR_PERCENTAGE_LIMIT 10

S_GET_DATA g_sGetData;

_io void AddDataToBufProc(void);

_io void PreparePublishJsonDataProc(void);
_io bool PrepareResponseDataProc(void);

_io bool SimDetectedProc(void);
_io bool GsmGeneralProc(void);
_io void GsmCloseProc(void);
_io bool GsmModuleInitialProc(void);

_io bool UsrProcessStringResponseParser(const char* input, const char* parserStringValue);
_io bool UsrProcessIntResponseParser(const char* input, const char* parserIntValue);
_io bool UsrProcessFloatResponseParser(const char* input, const char* parserFloatValue);

_io void UpdateParametersProc(void);

_io int16_t m_logRawDataBuf[144];
_io uint8_t m_logRawDataBufCnt;
bool g_PublishOkFlag;
bool updateParamsFlag;
char *json;
char parsedStringValueBuf[150];   // 50
char parsedIntValueBuf[12];      // 12
char parsedFloatValueBuf[8];    // 12
int integerResponseValue;
float floatResponseValue;

#ifdef _accModuleCompile
    extern bool g_accelometerWakeUpFlag;
#endif

// json payload
char *mcu = "STM32L051";
char *version = "8.4.1";
uint8_t lat, lon = 0;


// uint8_t m_receiveGsmBuf[_gsm_receive_buffer_size];
extern uint8_t m_GsmBuf[_gsm_receive_buffer_size];
extern uint8_t m_HelperBuf[_gsm_receive_buffer_size];



void UsrProcess(void)
{
    _io bool _sendDataFlg = true;
    _io bool _isTankFullState = false;
    _io bool _fullnessLimitExdeed = false;
    _io bool _isCleaned = false;
    _io bool _fireAlarmFlag = false;
    _io uint8_t _coverAlarmFlag = 0;

    if (_sendDataFlg)                                                  // UsrSystemInitial --> UsrProcessDecideFirstState
    {
        #ifdef __usr_process_log
          __logsw("DEVICE COME FROM RESET");
        #endif
        g_packageEventBits |= _USR_SYSTEM_EVENT_BITS_DEVICE_RESET;
        g_dataSendTs = UL_RtcGetTs();
    }

    if (!g_sensorsReadingFlag)                                           // usr_sensor den false olarak geliyor.
    {
        AddDataToBufProc();                                              // 144 byte lik diziyi doldurmak

        if (_coverAlarmFlag != g_sAllSensorValues.halleffectAlarmStatus)
        {
            _coverAlarmFlag = g_sAllSensorValues.halleffectAlarmStatus;
            #ifdef __usr_process_log
                __logse("Cover Status Changed, last status %02x,", _coverAlarmFlag);
                if( _coverAlarmFlag == 1)                                // sadece top cover acik
                {
                    __logse("Top Cover Open, Battery Cover Closed");
                }
                else if(_coverAlarmFlag == 2)                            // sadece bat cover acik
                {
                    __logse("Top Cover Closed, Battery Cover Open");
                }
                else if(_coverAlarmFlag == 3)                            // ikisi de acik
                {
                    __logse("Top Cover and Battery Cover Open");
                }
                else                                                     // hepsi kapali
                {
                    __logse("Top Cover and Battery Cover Closed");
                }
            #endif
            g_packageEventBits |= _USR_SYSTEM_EVENT_BITS_COVERS_ALARM;

            _sendDataFlg = true;
        }

        if (_fireAlarmFlag != g_fireAlarmFlag)
        {
            _fireAlarmFlag = g_fireAlarmFlag;
            g_packageEventBits |= _USR_SYSTEM_EVENT_BITS_FIRE_ALARM;
            #ifdef __usr_process_log
                __logse("Fire alarm status was changed", (int)_fireAlarmFlag);
            #endif
            _sendDataFlg = true;
        }

        if (g_sAllSensorValues.distanceValue != _USR_SENSOR_DISTANCE_ERROR_VALUE)
        {
            if (g_sAllSensorValues.distanceValue <= g_sNvsDeviceInfo.fullAlarmLimit)
            {
                if (!_isTankFullState)
                {
                    #ifdef __usr_process_log
                        __logse("Enter full alarm");
                    #endif
                    g_packageEventBits |= _USR_SYSTEM_EVENT_BITS_FULL_ALARM;
                    _sendDataFlg = true;
                }
                _isTankFullState = true;
            }
            else
            {
                if (_isTankFullState)
                {
                    #ifdef __usr_process_log
                        __logse("Exit full alarm");
                    #endif
                    g_packageEventBits &= ~ _USR_SYSTEM_EVENT_BITS_FULL_ALARM;
                    _sendDataFlg = true;
                }
                _isTankFullState = false;
            }

            if (g_sAllSensorValues.distanceValue <= g_sNvsDeviceInfo.fullnessAlarmLimit)
            {
                if (!_fullnessLimitExdeed)
                {
                    #ifdef __usr_process_log
                        __logse("Enter fullness state. This is not an alarm");
                    #endif
                }
                _fullnessLimitExdeed = true;
            }
            else if (_fullnessLimitExdeed && (g_sAllSensorValues.distanceValue >= (g_sNvsDeviceInfo.depthAlarmLimit * ((100 - g_sNvsDeviceInfo.toleranceValue) / 100))))
            {
                #ifdef __usr_process_log
                    __logse("Tank has been emptied. Enter Emptied alarm !");
                #endif
                g_packageEventBits  |= _USR_SYSTEM_EVENT_BITS_EMPTIED_ALARM;
                _sendDataFlg         = true;
                _isCleaned           = true;
                _fullnessLimitExdeed = false;
            }
        }

        if(g_accelometerWakeUpFlag)
        {
            g_accelometerWakeUpFlag = 0;

            g_packageEventBits |= _USR_SYSTEM_EVENT_BITS_ACC_SHAKE_ALARM;
            _sendDataFlg = true;
        }

        #ifdef _accModuleCompile
        if(UL_AccelCheckChip())
        {
            g_packageEventBits &= ~ _USR_SYSTEM_EVENT_BITS_ACC_COMMUNICATION_ERROR;
        }
        else
        {
            g_packageEventBits |= _USR_SYSTEM_EVENT_BITS_ACC_COMMUNICATION_ERROR;
        }
        // UL_AccelometerClearFlag(); //..
        #endif
        

        #ifdef __usr_process_log
            __logsi("Data Process Completed. AlarmCase: %d", g_packageEventBits);
        if(_sendDataFlg)
            __logsw("ts: %d, Data Send mission STARTED. Reason for Sending Data: Alarm case detection\n", UL_RtcGetTs());
        #endif

        if ((UL_RtcGetTs() - g_dataSendTs) >= g_sNvsDeviceInfo.sendingDataInterval)     // periyodik data gönderme zamanı
        {
            g_packageEventBits |= _USR_SYSTEM_EVENT_BITS_PERIODIC_DATA_SEND;
            _sendDataFlg = true;
            #ifdef __usr_process_log
            __logsw("ts: %d, Data Send mission STARTED. Reason for Sending Data: Dummy Data Send Interval\n", UL_RtcGetTs());
            #endif
        }
    }

    #ifdef __usr_process_log
        if (!_sendDataFlg)
        {
            __logsw("ts: %d, NOT any Alarm Case or Dummy Data Interval\n", UL_RtcGetTs());  
        }
    #endif
    
    if (_sendDataFlg)
    {
        g_dataSendTs = UL_RtcGetTs();
        if(GsmGeneralProc()) 
        // if(1)
        {
            m_logRawDataBufCnt = 0;
            g_packageEventBits = 0;
            _isCleaned = false;

            #ifdef __usr_process_log
                __logsw("ts: %d, Data Send mission SUCCESFULLY Completed. Going to Sleep Now !\n", UL_RtcGetTs());  // Eren yoruma aldi
            #endif
        }
        else
        {
            #ifdef __usr_process_log
                __logsw("ts: %d, Data Send mission FAILED ! Going to Sleep Now !\n", UL_RtcGetTs()); 
            #endif
        }
        _sendDataFlg = false;
    }
    g_sleepFlag = true;   // En başa gitsin başa sarsın 
}

void UsrProcessLedOpenAnimation(void)
{
    if (g_sNvsDeviceInfo.deviceStatus)
        UL_LedOpenAnimation(250); // RGB blink with timeout
    else
        UL_LedPassiveAnimation(250);
}

void UsrProcessDecideFirstState(void)
{
    if (!g_sAllSensorValues.halleffectAlarmStatus)
        g_sleepFlag = false;                                    // usr_sleep.c  dosyası

    g_sensorsReadingFlag = true;                                // usr_sensor.c
}

_io void AddDataToBufProc(void)
{
    _io int32_t _lastdistanceValue = 0;

    if (g_sAllSensorValues.distanceValue != _USR_SENSOR_DISTANCE_ERROR_VALUE && _lastdistanceValue != 0)
    {
        uint8_t tryCount = 0;
        uint8_t tryErrorCount = 0;
        int percentage = 0;
    start_step:;
        if (++tryCount > 3)
        {
            _lastdistanceValue = g_sAllSensorValues.distanceValue;
            #ifdef __usr_process_log
                __logsw("That was 3rd attempt. Data assumed true.");
            #endif
            goto reading_log_step;
        }
        
        float value = g_sAllSensorValues.distanceValue - _lastdistanceValue;
        if (value < 0)
            value *= -1;
        percentage = (int)((value * 100) / _lastdistanceValue);
        if (percentage >= _USR_PERCENTAGE_LIMIT)
        {
            #ifdef __usr_process_log
                __logsw("Distance Value suddenly changed too much, Going to 1min sleep than try again. (%u/3) ",tryCount);
            #endif
        sleep_step:;
            UsrSleepEnterSubSleep(6); // every wakeup time is ten second so waiting is 1 minute
            UsrSensorGetDistance();
            if (g_sAllSensorValues.distanceValue == _USR_SENSOR_DISTANCE_ERROR_VALUE)
            {
                if (++tryErrorCount > 3)
                {
                    #ifdef __usr_process_log
                        __logsw("That was 3rd attempt resulted with Error. Data assumed true.");
                    #endif
                    goto reading_log_step;
                }    
                else
                {
                    #ifdef __usr_process_log
                        __logsw("Distance Sensor Error. Going to 1min sleep than try again. (%u/3) ",tryErrorCount);
                    #endif
                    goto sleep_step;
                }
            }
            goto start_step;
        }
        else
            _lastdistanceValue = g_sAllSensorValues.distanceValue;
    }
    else if(g_sAllSensorValues.distanceValue != _USR_SENSOR_DISTANCE_ERROR_VALUE && !_lastdistanceValue)
            _lastdistanceValue = g_sAllSensorValues.distanceValue;   

reading_log_step:;
    // (m_logRawDataBufCnt == 0)        // alttakini silince bunu da silmem gerekti cunku asagidakiler bu if'in komutuna donustu ve akisi bozdu!
        // m_startTs = UL_RtcGetTs();   // KULLANILMAMIS, YINE DE SILMEDEN ONCE TEVFIK BEYE HABER VERMELI MIYIZ ??

    if (m_logRawDataBufCnt >= 144)
    {
        for (uint8_t i = 0; i < (m_logRawDataBufCnt - 1); i++)
            m_logRawDataBuf[i] = m_logRawDataBuf[i + 1];

        m_logRawDataBuf[143] = g_sAllSensorValues.distanceValue;
    }
    else
        m_logRawDataBuf[m_logRawDataBufCnt++] = g_sAllSensorValues.distanceValue;
}

// Bu fonksiyon _sendDataFlg flagine bagli olarak calismaktadir. 
_io bool GsmGeneralProc(void)
{
    UsrSystemWatchdogRefresh();
    
    uint8_t tryCount = 0;
    uint8_t step = 0;
    bool _gsmMissionSuccess = false;                                            // false baslatilir, is biterse true olur ve bu, fonksiyon cikisinda true dondurur !
    bool _gsmLoopEnable = true;                                                 // dongu'ye izin var mi ?
    
    UL_GsmModulePeripheral(enableGsmPeripheral);                                // burada olsun, önce Gsm modülü açılsın
    _gsm_delay(250);

    if(!SimDetectedProc())                                                      // SIM algılanmazsa, SimDetectedProc return'u false olacak
    {
        _gsmLoopEnable = false;                                                 // bu da loop'u iptal eder.
        goto end_step;
    }

    uint32_t gsmControlTimeout = HAL_GetTick();
    while(_gsmLoopEnable)
    {
        UsrSystemWatchdogRefresh();
        if(step == 0)                                                           // step 0'da, 60sn kadar bulunulabilir
        {
            if((HAL_GetTick() - gsmControlTimeout) > 60000)                     
            {
                #ifdef __usr_process_log
                    __logse("Step 0: GSM module couldn't be activated within 60sec ! Mission Failed. "); 
                #endif          
                break;
            }
            if(GsmModuleInitialProc())
            {
                #ifdef __usr_process_log
                    __logsw("Step 0: Activate MQTT and GSM -> DONE\n");
                #endif
                step = 1;
            }
        }
        else if(step == 1)                                                      // step 1'de, herhangi bir adım'da hata oluşması halinde 5 kez başa dönülür
        {
            if(UL_GsmModuleMqttInitial((const S_GSM_MQTT_CONNECTION_PARAMETERS *)&g_sGsmMqttInitialParameters))
            {
                if(UL_GsmModuleMqttSubcribeTopic(TOPIC_BRK2MCU, 0))
                {
                    #ifdef __usr_process_log
                        __logsi("Step 1: Subscribed for Listening to This Topic: %s", TOPIC_BRK2MCU);
                    #endif
                        
                    PreparePublishJsonDataProc();
                start_label:;

                    tryCount++;
                    #ifdef __usr_process_log
                        __logsi("Step 1: This is %uth attempt to Publish Data.",tryCount);
                    #endif


                    if(UL_GsmModuleMqttPublishTopic(TOPIC_MCU2BRK, m_HelperBuf, 0, 0))
                    {
                        #ifdef __usr_process_log
                            __logsi("Step 1: Data Published to This Topic: %s",TOPIC_MCU2BRK);
                            __logsw("Step 1: Subscribe Topic and Publish Data -> DONE\n");
                        #endif
                        g_PublishOkFlag = true;
                        step = 2;
                    }
                    else
                    {
                        HAL_Delay(250);
                        goto start_label;
                    }
                    
                }
                else
                    tryCount++;
            }
            else
                tryCount++;
        
            g_waitResponseCount = 0;
            memset(m_GsmBuf,0,512);
            memset(m_HelperBuf,0,512);
            #ifdef __usr_process_log
                if(step == 2)
                    __logsi("Step 2: Wait %usec for response",(_USR_RESPONSE_TIMEOUT/1000));
            #endif
        }
        else if(step == 2)                                                      // step 2'de, _USR_RESPONSE_TIMEOUT suresi kadar bulunulabilir
        {
            if(g_waitResponseCount >= _USR_RESPONSE_TIMEOUT)
            {
                #ifdef __usr_process_log
                    __logse(" Step 2: Data Published but No Data Recieved for %usec. Data Send Mission Failed !", (_USR_RESPONSE_TIMEOUT/1000));
                #endif

                _gsmMissionSuccess = false;
                break;
            }
            else
            {
                UL_GsmModuleMqttGeneral();
                if(responseSubcribeDataCallbackFlag)
                {
                    #ifdef __usr_process_log
                        __logsi("Step 2: Data Successfully Recieved !")
                    #endif  
                    if(PrepareResponseDataProc())
                    {
                        #ifdef __usr_process_log
                            __logsw("Step 2: Recieve Data from Broker, Parsing Data and Updating NVS -> DONE\n");
                        #endif

                        responseSubcribeDataCallbackFlag = false;
                        _gsmMissionSuccess  = true;
                        break;                        
                    }
                    else
                    {
                        #ifdef __usr_process_log
                            __logsw("Step 2: Recieve Data from Broker but Response Data is Failed !");
                        #endif

                        responseSubcribeDataCallbackFlag = false;
                        _gsmMissionSuccess    = false;
                        break;
                    }
                }   
            }
        }
        if(tryCount > 5)
        {
            #ifdef __usr_process_log
                __logse("Step 1: After 5 attempts on Step 1, Data Publish Failed\n");
            #endif
            break;
        }
    }
    
end_step:;

    #ifdef __usr_process_log
      if( (step == 1) && !g_PublishOkFlag )
          __logse(" Data Publish Failed 5 times. Data Send Mission Failed !");   
      __logsi(" Terminating Data Sending Task. GSM is shutting down !");   
    #endif

    g_PublishOkFlag = false;
    
    UsrSystemWatchdogRefresh();
    GsmCloseProc();                                                             // Modul kapanip uyku asamasina gidilecek !
    
    if(_gsmMissionSuccess)
        return true;

    return false;
}

_io void GsmCloseProc(void)
{
    UL_GsmModuleMqttClosed();
    UL_GsmModuleDeInitial();
    UL_GsmModulePeripheral(disableGsmPeripheral);
    UL_GsmModuleHardwareReset();
    #ifdef __usr_process_log
        __logsw("GSM Module Closed");
    #endif
}

_io bool GsmModuleInitialProc(void)
{
    if(UL_GsmModuleCheck())
    {
        #ifdef __usr_process_log
            __logsi("Step 0: GSM Module OK");
        #endif
        if(UL_GsmModuleGetInfo(&g_sGsmModuleInfo))
        {
            #ifdef __usr_process_log
                __logsi("Step 0: GSM Info OK");
            #endif
            return true;
        }
        else 
        {
            #ifdef __usr_process_log
                __logse("Step 0: Gsm Info NOT OK ! Check SIM Card !!!");
            #endif
        }   
    }
    return false;
}

// ResponseDataParserProc olsun yeni ismi ? 
_io bool PrepareResponseDataProc(void)
{
    if(UsrProcessStringResponseParser(m_HelperBuf, "success"))
    {
        #ifdef __usr_process_log
           // __logsi("success: %s", parsedStringValueBuf);
        #endif   

        if(strstr(parsedStringValueBuf, "true"))
        {
            #ifdef __usr_process_log
                __logsi("\"success\" is True, Parsing Starting\n");
            #endif

            /* timestamp zaman guncellemesi */
            if(UsrProcessIntResponseParser(m_HelperBuf, "ts"))
            {
                #ifdef __usr_process_log
                    __logsi("ts: %d", integerResponseValue);
                #endif
                if(integerResponseValue)
                {
                    g_sGetData.ts = integerResponseValue;
                }
            }

            /* sendingDataInterval (periyodik data gonderme)*/
            if(UsrProcessIntResponseParser(m_HelperBuf, "interval"))
            {
                #ifdef __usr_process_log
                    __logsi("interval: %d", integerResponseValue);
                #endif
                if(integerResponseValue)
                {
                    g_sGetData.interval = integerResponseValue;
                }
            }
            
            /* fullAlarmLimit */
            
            /* version guncellemesi varsa */
            memset((void*)parsedStringValueBuf, 0, sizeof(parsedStringValueBuf));
            if(UsrProcessStringResponseParser(m_HelperBuf, "version"))
            {
                #ifdef __usr_process_log
                    __logsi("version: %s", parsedStringValueBuf);
                #endif
                if(strcmp((const char*)parsedStringValueBuf, _device_version) != 0)
                {
                    #ifdef __usr_process_log
                        __logsi("firmware was different");
                    #endif
                }
            }

            /* DeviceStatus guncellemesi */
            memset((void*)parsedStringValueBuf, 0, sizeof(parsedStringValueBuf));
            if(UsrProcessStringResponseParser(m_HelperBuf, "status"))
            {
                #ifdef __usr_process_log
                    __logsi("status: %s", parsedStringValueBuf);
                #endif
                if(strstr((const char*)parsedStringValueBuf, "enable") != NULL)
                {
                    g_sGetData.deviceStatus = 1;
                }
                else
                {
                    g_sGetData.deviceStatus = 0;
                }
            }

            /* deltaTemp e bakiyorum ama parse etmiyorum */
            if(UsrProcessFloatResponseParser(m_HelperBuf, "deltaTemp"))
            {
                #ifdef __usr_process_log
                    __logsi("deltaTemp: %.3f", floatResponseValue);
                #endif
            }

            /* deviceStatusCheckTime guncellemesi */
            if(UsrProcessIntResponseParser(m_HelperBuf, "deviceStatusCheckTime"))
            {
                #ifdef __usr_process_log
                    __logsi("deviceStatusCheckTime: %d", integerResponseValue);
                #endif
                if(integerResponseValue)
                {
                    g_sGetData.deviceStatusCheckTime = integerResponseValue;
                }
            }

            /* link guncellemesi */
            memset((void*)parsedStringValueBuf, 0, sizeof(parsedStringValueBuf));
            if(UsrProcessStringResponseParser(m_HelperBuf, "link"))
            {
                #ifdef __usr_process_log
                    __logsi("link: %s", parsedStringValueBuf);
                #endif
                if(parsedStringValueBuf)
                {
                    #ifdef __usr_process_log
                        __logsi("yazilim linki mevcut");
                    #endif
                }
            }

            /* depthAlarmLimit guncellemesi */
            if(UsrProcessIntResponseParser(m_HelperBuf, "depth"))
            {
                #ifdef __usr_process_log
                    __logsi("depthAlarmLimit: %d", integerResponseValue);
                #endif
                if(integerResponseValue)
                {
                    g_sGetData.depthAlarmLimit = integerResponseValue;

                }
            }

            /* fullnessAlarmLimit guncellemesi */
            if(UsrProcessIntResponseParser(m_HelperBuf, "fullness"))
            {
                #ifdef __usr_process_log
                    __logsi("fullnessAlarmLimit: %d", integerResponseValue);
                #endif
                if(integerResponseValue)
                {
                    g_sGetData.fullnessAlarmLimit = integerResponseValue;
                }
            }

            /* toleranceValue guncellemesi */
            if(UsrProcessIntResponseParser(m_HelperBuf, "tolerance"))
            {
                #ifdef __usr_process_log
                    __logsi("toleranceValue: %d", integerResponseValue);
                #endif
                if(integerResponseValue)
                {
                    g_sGetData.toleranceValue = integerResponseValue;
                }
            }

            /* sensorWakeUp guncellemesi */
            if(UsrProcessIntResponseParser(m_HelperBuf, "sensorWakeUp"))
            {
                #ifdef __usr_process_log
                    __logsi("sensorWakeUp: %d", integerResponseValue);
                #endif
                if(integerResponseValue)
                {
                    g_sGetData.sensorWakeUp = integerResponseValue;
                }
            }
            
            UpdateParametersProc();
            #ifdef __usr_process_log
                __logsi("Step 2: Data parsed for %d times. Parsing Done !", g_subcribeDataCallbackCounter);
            #endif

            return true;
        }
        else
        {
            #ifdef __usr_process_log
                __logse("Step 2: success is false, it must be true and please write -> success <- for parsing process");
            #endif
            return false;
        }
    }
    else
    {
        #ifdef __usr_process_log
            __logse("Step 2: Invalid response");
        #endif
        return false;
    }
}

_io void PreparePublishJsonDataProc(void)
{
    memset(m_HelperBuf,0,512);
    if(1)
    {
        sprintf(m_HelperBuf,"{\"deviceReset\":%d,\"ts\":%d,\"sendType\":%d,\"mcu\":%s,\"module\":%s,\"sigq\":%d,\"ccid\":%s,\"comeFromReset\":%s,\"periodicDataSend\":%s,\"accShakeSend\":%s,\"accCommmErr\":%s,\"temp\":%.3f,\"charge\":%.3f,\"fireAlarm\":%s,\"fullAlarm\":%s,\"emptiedAlarm\":%s,\"coverAlarm\":%s,\"batteryCoverAlarm\":%s,\"version\":%s,\"lat\":%d,\"lon\":%d,\"distanceDatas\":",
            g_sNvsDeviceInfo.deviceStatus,
            g_sAllSensorValues.rtc,
            g_packageEventBits,
            mcu,
            g_sGsmModuleInfo.moduleInfoBuf,
            g_sGsmModuleInfo.signal,
            g_sGsmModuleInfo.iccidBuf,
            ( (g_packageEventBits & _USR_SYSTEM_EVENT_BITS_DEVICE_RESET) ? "true" : "false" ),
            ( (g_packageEventBits & _USR_SYSTEM_EVENT_BITS_PERIODIC_DATA_SEND) ? "true" : "false" ),
            ( (g_packageEventBits & _USR_SYSTEM_EVENT_BITS_ACC_SHAKE_ALARM) ? "true" : "false" ),
            ( (g_packageEventBits & _USR_SYSTEM_EVENT_BITS_ACC_COMMUNICATION_ERROR) ? "true" : "false" ),
            g_sAllSensorValues.tempValue,
            g_sAllSensorValues.batteryVoltage,
            ( (g_packageEventBits & _USR_SYSTEM_EVENT_BITS_FIRE_ALARM) ? "true" : "false" ),
            ( (g_packageEventBits & _USR_SYSTEM_EVENT_BITS_FULL_ALARM) ? "true" : "false" ),
            ( (g_packageEventBits & _USR_SYSTEM_EVENT_BITS_EMPTIED_ALARM) ? "true" : "false" ),
            ((g_sAllSensorValues.halleffectAlarmStatus & 0x01) ? "true" : "false"),
            ((g_sAllSensorValues.halleffectAlarmStatus & 0x02) ? "true" : "false"),
            (_device_version),
            lat,
            lon);

        // eğer tek data varsa direkt [data] seklinde yazalim
        if(m_logRawDataBufCnt == 1)
            sprintf(m_HelperBuf, "%s[%u]}",m_HelperBuf, m_logRawDataBuf[0]);
        else
        {
            // yukarida hazirlanan datayla birlesim durumunu yonetebilmek icin ilk datayi ayri yazdim, benimki bosluksuz ve [ ile basliyor simdilik
            sprintf(m_HelperBuf, "%s[%u",m_HelperBuf, m_logRawDataBuf[0]);

            for(int i=1 ; i<(m_logRawDataBufCnt-1) ; i++)
            {
                  // dizi ortasinda yer alan datalarin bitisiklik durumunu yonetiyor, ben virgül ve bosluk biraktim simdilik
                  sprintf(m_HelperBuf, "%s, %u",m_HelperBuf, m_logRawDataBuf[i]);
            }

            // dizisinin sonlanisi icin son data ayri yazildi. ben diziyi son data ile ] bosluksuz, sonrada bosluksuz } seklinde bitirdim.
            sprintf(m_HelperBuf, "%s, %u]}",m_HelperBuf, m_logRawDataBuf[m_logRawDataBufCnt-1]);
        }
        
    #ifdef __usr_process_log
        //__logsi("Preloaded Json: %s",m_HelperBuf);
        __logsi("Preloaded Json: OLMASI GEREKIYO AMA OLMUYO !! CTRL SHIFT F ILE BUL BURAYI !!!");
    #endif
    }
    else
    {
        #ifdef __usr_process_log 
            __logse("m_JsonDataBuf NULL !!!");
        #endif
    }  
}

_io void UpdateParametersProc(void)
{
    if(g_sGetData.deviceStatus != g_sNvsDeviceInfo.deviceStatus)
    {
        g_sNvsDeviceInfo.deviceStatus = g_sGetData.deviceStatus;
        updateParamsFlag = true;
    }
    if(g_sGetData.ts != g_sNvsDeviceInfo.deviceStatusCheckTime)
    {
        g_sNvsDeviceInfo.deviceStatusCheckTime = g_sGetData.ts;
        updateParamsFlag = true;
    }
    if(g_sGetData.interval != g_sNvsDeviceInfo.sendingDataInterval)
    {
        if(g_sGetData.interval > (24*60*60*1000))    // 1440 dakika yapmaya calistim, daily reset olmasın 1 gün 1440 dakikadir
            g_sGetData.interval = (24*60*60*1000);
        else if(g_sGetData.interval < (11*60*1000))
            g_sGetData.interval = (11*60*1000);   // 11 dakikadan kucuk olmasin
        else
        {
            if(g_sGetData.sensorWakeUp<g_sGetData.interval)
            {
                g_sNvsDeviceInfo.sendingDataInterval = g_sGetData.interval;
                updateParamsFlag = true;
            }
        }
    }
    if(g_sGetData.deviceStatusCheckTime != g_sNvsDeviceInfo.deviceStatusCheckTime)
    {
        g_sNvsDeviceInfo.deviceStatusCheckTime = g_sGetData.deviceStatusCheckTime;
        updateParamsFlag = true;
    }
    if(g_sGetData.depthAlarmLimit != g_sNvsDeviceInfo.depthAlarmLimit)
    {
        g_sNvsDeviceInfo.depthAlarmLimit = g_sGetData.depthAlarmLimit;
        updateParamsFlag = true;
    }
    if(g_sGetData.fullnessAlarmLimit != g_sNvsDeviceInfo.fullnessAlarmLimit)
    {
        g_sNvsDeviceInfo.fullnessAlarmLimit = g_sGetData.fullnessAlarmLimit;
        updateParamsFlag = true;
    }
    if(g_sGetData.toleranceValue != g_sNvsDeviceInfo.toleranceValue)
    {
        g_sNvsDeviceInfo.toleranceValue = g_sGetData.toleranceValue;
        updateParamsFlag = true;
    }
    if(g_sGetData.sensorWakeUp != g_sNvsDeviceInfo.sensorWakeUpTime)
    {
        if(g_sGetData.sensorWakeUp<g_sGetData.interval)
        {
            g_sNvsDeviceInfo.sensorWakeUpTime = g_sGetData.sensorWakeUp;
            updateParamsFlag = true;
        }
    }
    if(updateParamsFlag)
        UsrNvsUpdate();
}

_io bool UsrProcessStringResponseParser(const char* input, const char* parserStringValue)
{
    const char* startResponseArray = strchr(input, '#');
    const char* endResponseArray = strchr(startResponseArray + 1, '#');

    if(startResponseArray != NULL && endResponseArray != NULL)
    {
        uint16_t length = endResponseArray - (startResponseArray + 1);
        char* copyResponse = (char*)_gsm_malloc(length + 1);

        if(copyResponse != NULL)
        {
            strncpy(copyResponse, startResponseArray + 1, length);
            copyResponse[length] = '\0';

            char *ptr = copyResponse;
            ptr = strstr(ptr, parserStringValue);
            if(ptr != NULL)
            {
                ptr = strchr(ptr, ':');
                if(ptr != NULL)
                {
                    ptr++;
                    while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r') 
                        ptr++;
                }

                if (*ptr == '\"') 
                {
                    ptr++;
                    uint8_t i = 0;
                    while (*ptr != '\"' && i < sizeof(parsedStringValueBuf) - 1)
                        parsedStringValueBuf[i++] = *ptr++;
                    
                    parsedStringValueBuf[i] = '\0';
                }
                ptr++;
                
                free(copyResponse);
                return true;
            }
            else
            {
                #ifdef __usr_process_log
                    //__logse("StringResponseParser]: parserStringValue error");
                #endif
                free(copyResponse);
                return false;
            }
        }
        else
        {
            #ifdef __usr_process_log
                //__logse("StringResponseParser]: copyResponse error");
            #endif
            return false;
        }
    }
    else
    {
        #ifdef __usr_process_log
            //__logse("StringResponseParser]: error");
        #endif
        return false;
    }
}

_io bool UsrProcessIntResponseParser(const char* input, const char* parserIntValue)
{
    const char* startResponseArray = strchr(input, '#');
    const char* endResponseArray = strchr(startResponseArray + 1, '#');

    if(startResponseArray != NULL && endResponseArray != NULL)
    {
        uint16_t length = endResponseArray - (startResponseArray + 1);
        char* copyResponse = (char*)_gsm_malloc(length + 1);

        if(copyResponse != NULL)
        {
            strncpy(copyResponse, startResponseArray + 1, length);
            copyResponse[length] = '\0';
            
            char *ptr = strstr(copyResponse, parserIntValue);
            if (ptr != NULL)
            {
                ptr = strchr(ptr, ':');
                if (ptr != NULL)
                {
                    ptr++;
                    while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r')
                        ptr++;

                    uint8_t i = 0;
                    while (*ptr != '\"' && *ptr != ',' && i < sizeof(parsedIntValueBuf) - 1)
                        parsedIntValueBuf[i++] = *ptr++;

                    parsedIntValueBuf[i] = '\0';
                    integerResponseValue = atoi(parsedIntValueBuf);
                    free(copyResponse);
                    return true;
                }        
            }
            else
            {
                #ifdef __usr_process_log
                    //__logse("IntResponseParser]: parserIntValue error");
                #endif
                free(copyResponse);
                return false;
            }
        } 
        else
        {
            #ifdef __usr_process_log
                //__logse("IntResponseParser]: copyResponse Error");
            #endif
            return false;
        }
    }
    else
    {
        #ifdef __usr_process_log
            //__logse("IntResponseParser]: error");
        #endif
        return false; 
    }
}

_io bool UsrProcessFloatResponseParser(const char* input, const char* parserFloatValue)
{
    const char* startResponseArray = strchr(input, '#');
    const char* endResponseArray = strchr(startResponseArray + 1, '#');

    if(startResponseArray != NULL && endResponseArray != NULL)
    {
        uint16_t length = endResponseArray - (startResponseArray + 1);
        char* copyResponse = (char*)_gsm_malloc(length + 1);

        if(copyResponse != NULL)
        {
            strncpy(copyResponse, startResponseArray + 1, length);
            copyResponse[length] = '\0';
            
            char *ptr = strstr(copyResponse, parserFloatValue);
            if (ptr != NULL)
            {
                ptr = strchr(ptr, ':');
                if (ptr != NULL)
                {
                    ptr++;
                    while (*ptr == ' ' || *ptr == '\t' || *ptr == '\n' || *ptr == '\r')
                        ptr++;

                    uint8_t i = 0;
                    while (*ptr != '\"' && *ptr != ',' && i < sizeof(parsedFloatValueBuf) - 1)
                        parsedFloatValueBuf[i++] = *ptr++;

                    parsedFloatValueBuf[i] = '\0';
                    floatResponseValue = atof(parsedFloatValueBuf);
                    free(copyResponse);
                    return true;
                }        
            }
            else
            {
                #ifdef __usr_process_log
                    //__logse("FloatResponseParser]: parserFloatValue error");
                #endif
                free(copyResponse);
                return false;
            }
        } 
        else
        {
            #ifdef __usr_process_log
                //__logse("FloatResponseParser]: copyResponse Error");
            #endif
            return false;
        }
    }
    else
    {
        #ifdef __usr_process_log
            //__logse("FloatResponseParser]: error");
        #endif
        return false;
    }
}

_io bool SimDetectedProc(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = SIM_DETECT_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;

    HAL_GPIO_Init(SIM_DETECT_GPIO_Port, &GPIO_InitStruct);
    HAL_GPIO_WritePin(SIM_DETECT_POWER_GPIO_Port, SIM_DETECT_POWER_Pin, GPIO_PIN_SET);
    HAL_Delay(100);

    bool simInsertedFlag = false;    
    uint32_t simDetectedTimeout = HAL_GetTick();
    while(!simInsertedFlag)
    {
        if(HAL_GetTick() - simDetectedTimeout > 3000)
        {
            #ifdef __usr_process_log
                __logse("Step 0: SIM NOT INSERTED");
            #endif
            simInsertedFlag = false;
            break;
        }

        if(HAL_GPIO_ReadPin(SIM_DETECT_GPIO_Port, SIM_DETECT_Pin) == GPIO_PIN_SET)
        {
            #ifdef __usr_process_log
                __logsi("Step 0: SIM DETECTED");
            #endif
            simInsertedFlag = true;
        }
    }

    HAL_GPIO_WritePin(SIM_DETECT_POWER_GPIO_Port, SIM_DETECT_POWER_Pin, GPIO_PIN_RESET);
    UsrSleepGpioOutPins(SIM_DETECT_GPIO_Port,     SIM_DETECT_Pin,       GPIO_PIN_RESET);

    return simInsertedFlag;
}