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

        void setup(uint8_t sensor, uint8_t time_read);
        float getIrradiance();
        void writeIrradiance();
        void writeCurrentVoltage();
        void getTimeTemperature();
        void readSD();

    private:
        uint8_t _sensor;
        uint8_t _time_read;
        const float _I_SC_STC = 0.0419;
        const int _G_STC = 1000;
        const float _U_STC = 0.0000189;
        const float _TEMPERATURE_STC = 25;
      
        File _irradianceFile;
        RTC_DS3231 _rtc;
        DateTime _now;
        Adafruit_INA219 _ina219;
        
        void _setupSD();
        void _setupRTC();
        void _setupINA219();
};


#endif