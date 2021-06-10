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

        enum placas{monocristalina, policristalina};  
        void setup(uint8_t sensor, uint8_t time_read);
        float getISC_AD627();
        float getIrradiance(placas pvcell);
        float getINA219current();
        void getTimeTemperature();
        int getRTCseconds();

        void writeIrradiance(placas pvcell);
        void writeCurrentVoltage();
        void writeCurrent();
        void readSD();
        void movePVcell();
       

    private:
        uint8_t _sensor;
        uint8_t _time_read;
        const int _G_STC = 1000;
        const float _TEMPERATURE_STC = 25;

        const float _I_SC_STC_MONO = 0.04;
        const float _U_STC = 0.0000189;

        const float _I_SC_STC_POLI = 0.03;

        int _topleft;
        int _topright;
        int _downleft;
        int _downright;
        int _waittime = 1;
         
      
        File _irradianceFile;
        File _currentFile;
        RTC_DS3231 _rtc;
        DateTime _now;
        Adafruit_INA219 _ina219;
        
        void _setupSD();
        void _setupRTC();
        void _setupINA219();
        void _setupPVCELL();
      
        void _textIrradiance(File file, placas pvcell);
};


#endif