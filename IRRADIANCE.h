#ifndef __IRRADIANCE_H__
#define __IRRADIANCE__

#include <SPI.h>
#include <SD.h>
#include "RTClib.h"
#include "Arduino.h"
#include <Wire.h>
#include <Adafruit_INA219.h>
#include <EEPROM.h>


class IRRADIANCE{
    public:
        float V_OC;
        float I_SC;
        float G_SENSOR;
        float temperature_RTC;

        //SENSORS AND SETUP
        enum placas{monocristalina, policristalina};  
        enum sensors{pvcell, ina219};
        void setup(uint8_t sensor);
        float getISC_AD627();
        float getIrradiance(placas pvcell);
        float getINA219current();
        bool isTimeRead();
        
        //Calculate Irradaiance
        void writeIrradiance(placas pvcell);
        void writeCurrentVoltage();
        void writeCurrent();
        void readSD();

        //EEPRON
        void EEPROMWriteInt(int address, int value);
        int EEPROMReadInt(int address);
        void clearEEPROM();
        void setMonitor(bool monitor);

        int getNumberSerial();

        //Serial
        void runTimeSensor();
        void showCommands();
        void showFileConfigured();
        void compareCommands();
        void defineTime();
        void checkTimeRead();
        void showTime();
        void showTimeConfigured();
        void changeConfig();

        void attinyMode();

    private:
        //_sensor porta analogica
        uint8_t _sensor;

        //Time Sensor to Measrured irradiance/Current/Voltage
        uint8_t _hourSensor;
        uint8_t _minuteSensor;
        uint8_t _secondSensor;

        const int _G_STC = 1000;
        const int _TEMPERATURE_STC = 25;

        const float _I_SC_STC_MONO = 0.034977;
        const float _U_STC = 0.0000189;

        const float _I_SC_STC_POLI = 0.03;

        uint32_t _timeMeasured;
        bool _isMonitor;
        bool _isSpeedMode;
        uint8_t _numberChanels;
        bool _isATtinny;
        bool _isConfigured;

        void _writeTimeEEPROM(int hour, int minute, int second);
        void _configureSensor();
        void _setFlagConfigure();

        void _setNumberChannels();
        void setTimeSensors();
        void _changeMode(int byteEEPROM, bool varTochange);

        char _yesOrNo();
        void _finishConfigure();
        bool _timeConfigure();

        String filename; 

        File _file;
        RTC_DS3231 _rtc;
        DateTime _future;
        Adafruit_INA219 _ina219;
        void _headerFile(File file);
        
        void _setupSD();
        void _setupRTC();
        void _setupINA219();
      
        void _formatTime(File file);
        void _reloadTimeRead();
};


#endif