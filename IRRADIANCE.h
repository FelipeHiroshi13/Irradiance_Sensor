#ifndef __IRRADIANCE_H__
#define __IRRADIANCE__

#include <SPI.h>
#include <SD.h>
#include "RTClib.h"
#include "Arduino.h"
#include <Wire.h>
#include <Adafruit_INA219.h>



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
        void getTimeTemperature();
        int getRTCseconds();
        bool isTimeRead();
        
        //Calculate Irradaiance
        void writeIrradiance(placas pvcell);
        void writeCurrentVoltage();
        void writeCurrent();
        void readSD();

        //Serial
        void runTimeSensor();
        void showCommands();
        String getValue(String data, char separator, int index);
        void setTimeMeasure(sensors sensor);
        void compareCommands();
        void defineTime(sensors sensor);
        void checkTimeRead(sensors sensor);
        void showTime();


    private:
        //_sensor porta analogica
        uint8_t _sensor;
        uint8_t _caseTime;
        uint32_t _timeReadIrradiance;
        uint32_t _timeReadIna;

        uint8_t _hourSensor;
        uint8_t _minuteSensor;
        uint8_t _secondSensor;

        const int _G_STC = 1000;
        const float _TEMPERATURE_STC = 25;

        const float _I_SC_STC_MONO = 0.034977;
        const float _U_STC = 0.0000189;

        const float _I_SC_STC_POLI = 0.03;
        char _filename[20];

        bool _definedCommands;
        bool _definedTime;
      
        File _irradianceFile;
        File _currentFile;
        RTC_DS3231 _rtc;
        DateTime _now;
        DateTime _future;
        Adafruit_INA219 _ina219;
        
        void _setupSD();
        void _setupRTC();
        void _setupINA219();
      
        void _formatTime(sensors sensor, File file);
        void _reloadTimeRead();
        void _textIrradiance(File file, placas pvcell);
};


#endif