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

        Adafruit_INA219 INA219_1  = 0x40;
        Adafruit_INA219 INA219_2  = 0x44;
        Adafruit_INA219 INA219_3 = 0x45;

        void setup();
        void compareCommands(int input);
        float getISC_AD627();
        void fileConfigure();

        //EEPRON
        void EEPROMWriteInt(int address, int value);
        int EEPROMReadInt(int address);
        void clearEEPROM();

        char readCommand();

        //Serial
        void runTimeSensor();
        void checkTimeRead();
        void speedMode();

        void writeINA219_1(File file);
        void writeINA219_2(File file);
        void writeINA219_3(File file);

        float getIrradiance();

    private:
        //_sensor porta analogica
        uint8_t _sensor;

        char _typeTime;
        uint8_t _numberTime;

        const int _G_STC = 1000;
        const int _TEMPERATURE_STC = 25;

        const float _I_SC_STC_MONO = 0.034977;
        const float _U_STC = 0.0000189;

        const float _I_SC_STC_POLI = 0.03;

        uint8_t _numberChanels;
        bool _isATtinny;
        bool _isConfigured;

        void _configureSensor();
        void _setFlagConfigure();

        String _filename; 

        File _file;
        File _configFile;
        RTC_DS3231 _rtc;
        DateTime _future;
        
        void _setupSD();
        void _setupRTC();
        void _setupINA219();
      
        void _formatTime(File file);
        void _reloadTimeRead();

        void _setTime(int input);
        void _deleteFile(int input);
        void _setNumberChannels(int input);

        void _writeFile();
        char _readFile();
        void _headerFile();
};


#endif