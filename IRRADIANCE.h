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
        void movePVcell();

        //Serial
        void runTimeSensor();
        void showCommands();
        String getValue(String data, char separator, int index);
        void setTimeMeasure();
        void compareCommands();

       



    private:
        uint8_t _sensor;
        uint32_t _timeRead;


        const int _G_STC = 1000;
        const float _TEMPERATURE_STC = 25;

        const float _I_SC_STC_MONO = 0.034977;
        const float _U_STC = 0.0000189;

        const float _I_SC_STC_POLI = 0.03;

        int _topleft;
        int _topright;
        int _downleft;
        int _downright;
        int _waittime = 2;
        
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
        void _setupPVCELL();
      
        void _textIrradiance(File file, placas pvcell);
};


#endif