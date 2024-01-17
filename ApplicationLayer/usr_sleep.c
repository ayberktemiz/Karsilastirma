#include "usr_general.h"

#define _fire_alarm_log_count 6

_io void BaseTimerControlProc(ETimerControl f_eControl);
_io bool FireAlarmProc(void);
_io bool CoverAlarmProc(void);
_io void SleepProcedureProc(void);

bool g_sleepFlag = false;
_io uint8_t m_lastHallAlarm;

#ifdef _accModuleCompile
    bool g_accelometerWakeUpFlag            = false;            // harbiden uyandı flag'i
    bool g_accelometerInterruptDetectedFlag = false;
    uint8_t _controlPeriodMin = 10;
    uint8_t _controlPeriodMax = 15;
    uint32_t _controlPeriodMinTimestamp;
    uint32_t _accInterruptTime = 0;
    uint32_t _lastAccInterruptTime = 0;
    
#endif
    
void UsrSleepGeneral(void)
{
    if (g_sleepFlag)
    {
        bool alarmCheckFlg = false;
        bool _wakeUpRequest = false;
        g_sleepFlag = false;

        m_lastHallAlarm = g_sAllSensorValues.halleffectAlarmStatus;
        int lastTs = UL_RtcGetTs();

    main_sleep:;
        MX_DMA_Deinit();
        BaseTimerControlProc(disableTimer);
    sleep_label:;
        #ifdef _accModuleCompile
            g_accelometerWakeUpFlag = false;
            g_accelometerInterruptDetectedFlag = false;
        #endif
            
        SleepProcedureProc();

        UsrSystemWatchdogRefresh();

        g_dailyResetTimer += 10000;
        int ts = UL_RtcGetTs();

        #ifdef __usr_sleep_log
            char CaFC[10]; int CaFC_int = (30-(ts - lastTs)); if(CaFC_int <=0) sprintf(CaFC, "%s", "Ready"); else sprintf(CaFC, "%dsec", CaFC_int);
            char DS[10]; int DS_int = (g_sNvsDeviceInfo.sendingDataInterval - (ts - g_dataSendTs)); if(DS_int <=0) sprintf(DS, "%s", "Ready"); else sprintf(DS, "%dsec", DS_int);
            char SR[10]; int SR_int = (g_sNvsDeviceInfo.sensorWakeUpTime - (ts - g_sensorReadTs)); if(SR_int <=0) sprintf(SR, "%s", "Ready"); else sprintf(SR, "%dsec", SR_int);
            char DR[10]; int DR_int = ((_USR_SYSTEM_DAILY_RESET_TIME-g_dailyResetTimer)/1000); if(DR_int <=0) sprintf(DR, "%s", "Ready"); else sprintf(DR, "%dsec", DR_int);
            __logsi("Intervals Checked. Time Until to Next; CoverAndFireCheck: %s, DataSend: %s, SensorRead: %s, DailyReset: %s",CaFC , DS , SR , DR);
        #endif

        #ifdef _accModuleCompile

            if(g_accelometerInterruptDetectedFlag)  /// herhangi bir sarsilma
            {
                g_accelometerInterruptDetectedFlag = false;
                _accInterruptTime = UL_RtcGetTs();
                
                if(!g_sNvsDeviceInfo.deviceStatus)
                { 
                    #ifdef __usr_sleep_log
                    __logsw("Device wake up from acc on deviceStatus: disable status, back to sleep again");
                    #endif
                    goto sleep_label;
                }
                else
                {
                    if( ( _accInterruptTime - _lastAccInterruptTime ) > _controlPeriodMax )
                    {
                        _controlPeriodMinTimestamp = (_accInterruptTime + _controlPeriodMin) ;
                    }
                    
                    _lastAccInterruptTime = _accInterruptTime;
                    
                    if( _accInterruptTime > _controlPeriodMinTimestamp )  // Bu Bir 2. tetiklenmeydi ! devam edip uyansin
                    {
                        g_accelometerWakeUpFlag = true; // Harbiden uyandı,     // Process'te, alarm bitini isledikten sonra 0'a cekilir.
                        #ifdef __usr_sleep_log
                        __logsi("ts: %d After a shake, shook again within the next %u-%u seconds! Therefore, instead of back to sleep, Wake up to Send Alarm!\n", _accInterruptTime,_controlPeriodMin,_controlPeriodMax);
                        #endif
                    }   
                    else   // 2. tetikleme durumu yok, geri uyutacaz arkadasi
                    {
                        #ifdef __usr_sleep_log
                        __logsi("ts: %d Device Shaked ! Back to Sleep", _accInterruptTime);
                        #endif
                        goto sleep_label;
                        
                    }
                }         
            }
        #endif

        if(g_dailyResetTimer > _USR_SYSTEM_DAILY_RESET_TIME)
        {
            #ifdef __usr_sleep_log
                __logsi("ts: %d, Daily Reset !.  \n",UL_RtcGetTs());
            #endif
            HAL_NVIC_SystemReset();
        }

        if (g_sNvsDeviceInfo.deviceStatus)
        {
            if ((ts - lastTs) > 30)
            {
                #ifdef __usr_sleep_log
                    __logsi("ts: %d, Wake up Reason: Cover and Fire alarms check per 30 seconds.\n", UL_RtcGetTs(), 30);
                #endif
                lastTs = ts;
                alarmCheckFlg = true;
            }

            if ((ts - g_dataSendTs) >= g_sNvsDeviceInfo.sendingDataInterval)
            {
                #ifdef __usr_sleep_log
                    __logsi("ts: %d, Wake up Reason: Dummy Data Send ! There is no data sent since (%dsec). So sensor read started and Data will be sent.\n", UL_RtcGetTs(), g_sNvsDeviceInfo.sendingDataInterval);
                #endif
                if(alarmCheckFlg)
                    _wakeUpRequest = 1;
                goto wakeup_label;
            }    
            else if ((ts - g_sensorReadTs) >= g_sNvsDeviceInfo.sensorWakeUpTime)
            {
                #ifdef __usr_sleep_log
                    __logsi("ts: %d, Wake up Reason: Sensor Read ! %dsec passed from last sensor read. If alarm case detected, Data will be send.\n", UL_RtcGetTs(), g_sNvsDeviceInfo.sensorWakeUpTime);
                #endif
                if(alarmCheckFlg)
                    _wakeUpRequest = 1;
                goto wakeup_label;
            }
            else if(g_accelometerWakeUpFlag)
            {
                #ifdef __usr_sleep_log
                    __logsi("ts: %d, Wake up Reason: Accelometer detected Shake. Data will be sent.\n", UL_RtcGetTs(), g_sNvsDeviceInfo.sensorWakeUpTime);
                #endif
                if(alarmCheckFlg)
                    _wakeUpRequest = 1;
                goto wakeup_label;
            }
            else
                if(!alarmCheckFlg)
                    goto sleep_label;
        }
        else
        {
            if ((ts - lastTs) <= g_sNvsDeviceInfo.deviceStatusCheckTime)   // <= olarak düzeltildi
            {
                #ifdef __usr_sleep_log
                    __logsi("ts: %d, deviceStatus is 0, check period hasn't come yet. Back to sleep again. Time until next check: %d \n", UL_RtcGetTs(), (g_sNvsDeviceInfo.deviceStatusCheckTime - (ts - lastTs)));
                #endif
                goto sleep_label;
            }
            else
            {
                #ifdef __usr_sleep_log
                    __logsi("ts: %d, deviceStatus is 0, wake up for deviceStatus Check Mission !.  \n",UL_RtcGetTs());
                #endif

                lastTs = UL_RtcGetTs();
                goto wakeup_label;
            }
        }

    wakeup_label:;
        HAL_ResumeTick();
        SystemClock_Config();
        //MX_GPIO_Init();       // Kapatan ayberk, accel interrupt'ını bozuyor !
        MX_DMA_Init();
        BaseTimerControlProc(enableTimer);
    
        if (alarmCheckFlg)
        {
            alarmCheckFlg = false;
            UsrSensorGetAdcAndHalleffectValues();
            #ifdef __usr_sleep_log
                __logsi("Covers and Temp Sensor Read OK! --> ts:%d, Cover:%02x, Temp: %.3f",UL_RtcGetTs(), g_sAllSensorValues.halleffectAlarmStatus, g_sAllSensorValues.tempValue);
            #endif
            if (!FireAlarmProc() && !CoverAlarmProc())
            {
                if(!_wakeUpRequest)
                {
                   #ifdef __usr_sleep_log
                    __logsi("ts: %d, Fire and Cover alarms Checked. No Alarm Condition. Back to sleep again.\n", UL_RtcGetTs());
                   #endif
                   goto main_sleep;
                }
                #ifdef __usr_sleep_log
                else 
                {
                   __logsi("ts: %d, Fire and Cover alarms Checked. No Alarm Condition. But still woken up for SensorRead / DataSend Intervals or Acc Shake !.\n", UL_RtcGetTs());
                }
                #endif
            }
            #ifdef __usr_sleep_log
            else
            {
                __logsi("ts: %d, Fire and Cover alarms Checked. ALARM DETECTED. Going to SensorGetValues for Sensor Readings.\n", UL_RtcGetTs());
            }
            #endif
        }
        g_sensorsReadingFlag = true;
    }
}

void UsrSleepEnterSubSleep(uint32_t f_time)
{
    UsrSystemWatchdogRefresh();
    MX_DMA_Deinit();
    BaseTimerControlProc(disableTimer);
    int waitCount = 0;
    while (waitCount <= f_time)
    {
        SleepProcedureProc();
        UsrSystemWatchdogRefresh();
        waitCount++;
    }
    HAL_ResumeTick();
    SystemClock_Config();
    //MX_GPIO_Init();       // Kapatan ayberk, accel interrupt'ını bozuyor !
    MX_DMA_Init();
    BaseTimerControlProc(enableTimer);
}

_io void SleepProcedureProc(void)
{
    #ifdef __usr_sleep_log
        SystemClock_Config();       /// LOG GONDEREBILMEK ICIN !!!
        __logse("ts: %d, !!! Device Just Enter the Sleep !\n",UL_RtcGetTs());
        HAL_UART_DeInit(&hlpuart1);  /// LOG GONDEREBILMEK ICIN !!!            
    #endif

    HAL_SuspendTick();
    __HAL_PWR_CLEAR_FLAG(PWR_FLAG_WU);
    PWR->CR |= 0x00000600;
    HAL_PWR_EnterSTOPMode(PWR_LOWPOWERREGULATOR_ON, PWR_STOPENTRY_WFI);

    #ifdef __usr_sleep_log
        HAL_UART_Init(&hlpuart1);   /// LOG GONDEREBILMEK ICIN !!! 
        SystemClock_Config();       /// LOG GONDEREBILMEK ICIN !!! 
        __logse("ts: %d, Device Just Wake Up !!!",UL_RtcGetTs());
    #endif
}

_io bool FireAlarmProc(void)
{
    _io uint8_t _temperatureArrayCnt = 0;
    _io float _temperatureBuf[_fire_alarm_log_count] = {0};

    if (_temperatureArrayCnt < _fire_alarm_log_count)
        _temperatureBuf[_temperatureArrayCnt++] = g_sAllSensorValues.tempValue;
    else
    {
        for (int i = 0; i < (_fire_alarm_log_count - 1); i++)
            _temperatureBuf[i] = _temperatureBuf[i + 1];
        _temperatureBuf[_fire_alarm_log_count - 1] = g_sAllSensorValues.tempValue;
        float diff = _temperatureBuf[_fire_alarm_log_count - 1] - _temperatureBuf[0];
        if (diff >= g_sNvsDeviceInfo.enterFireAlarmValue)
        {
            if (!g_fireAlarmFlag)
            {
                #ifdef __usr_sleep_log
                    __logse("Fire alarm detected");
                #endif
                g_fireAlarmFlag = true;
                return true;
            }
        }
        else if (diff < g_sNvsDeviceInfo.enterFireAlarmValue)
        {
            if (g_fireAlarmFlag)
            {
                #ifdef __usr_sleep_log
                    __logse("Fire alarm exit");
                #endif
                g_fireAlarmFlag = false;
                return true;
            }
        }
    }
    return false;
}


_io bool CoverAlarmProc(void)
{
    if (m_lastHallAlarm != g_sAllSensorValues.halleffectAlarmStatus)
    {
        m_lastHallAlarm = g_sAllSensorValues.halleffectAlarmStatus;
        return true;
    }
    return false;
}


_io void BaseTimerControlProc(ETimerControl f_eControl)
{
    if (f_eControl == enableTimer)
    {
        MX_TIM6_Init();
        HAL_TIM_Base_Start_IT(&_USR_SYSTEM_BASE_TIMER_CHANNEL);
    }
    else
    {
        HAL_TIM_Base_DeInit(&_USR_SYSTEM_BASE_TIMER_CHANNEL);
    }
}



void UsrSleepAdcPins(GPIO_TypeDef *f_pGpio, uint16_t f_pinGroup, GPIO_PinState f_ePinstate)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    HAL_GPIO_WritePin(f_pGpio, f_pinGroup, f_ePinstate);
    GPIO_InitStruct.Pin = f_pinGroup;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(f_pGpio, &GPIO_InitStruct);
}


void UsrSleepGpioOutPins(GPIO_TypeDef *f_pGpio, uint16_t f_pinGroup, GPIO_PinState f_ePinstate)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    HAL_GPIO_WritePin(f_pGpio, f_pinGroup, f_ePinstate);
    GPIO_InitStruct.Pin = f_pinGroup;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(f_pGpio, &GPIO_InitStruct);
}


void UsrSleepGpioInputPins(GPIO_TypeDef *f_pGpio, uint16_t f_pinGroup)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    GPIO_InitStruct.Pin = f_pinGroup;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(f_pGpio, &GPIO_InitStruct);
}
