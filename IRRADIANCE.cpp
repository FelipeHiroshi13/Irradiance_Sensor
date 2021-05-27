#include "IRRADIANCE.h"


void IRRADIANCE::setup(uint8_t sensor, uint8_t time_read){
    _sensor = sensor;
    _time_read = time_read;

    Serial.begin(9600);

    _setupRTC();
    _setupSD();
}

void IRRADIANCE::_setupRTC(){
    _now = _rtc.now();
    if (! _rtc.begin()) {
        Serial.println("Couldn't find RTC");
        Serial.flush();
        abort();
    }


    if (_rtc.lostPower()) {
        Serial.println("RTC lost power, let's set the time!");
        // When time needs to be set on a new device, or after a power loss, the
        // following line sets the RTC to the date & time this sketch was compiled
        _rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        // This line sets the RTC with an explicit date & time, for example to set
        // January 21, 2014 at 3am you would call:
        // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    }
}


void IRRADIANCE::_setupSD(){
    while (!Serial) {
        ; // wait for serial port to connect. Needed for native USB port only
    }

    Serial.print("Initializing SD card...");

    if (!SD.begin(4)) {
        Serial.println("initialization failed!");
        while (1);
    }
    Serial.println("initialization done.");

    _irradianceFile = SD.open("irradiance.txt", FILE_WRITE);

    // if the file opened okay, write to it:
    if (_irradianceFile) {
        Serial.print("Writing to test.txt...");
        _irradianceFile.println("testing 1, 2, 3.");
        // close the file:
        _irradianceFile.close();
        Serial.println("done.");
    } else {
        // if the file didn't open, print an error:
        Serial.println("error opening irradiance.txt");
    }       
}

float IRRADIANCE::getIrradiance(){
    float V_OC;
    float I_SC;

    temperature_RTC = _rtc.getTemperature();

    V_OC = (float(analogRead(_sensor))/ float(1023))*3.3;;
    I_SC = (V_OC/float(470))/float(0.1);

    return(I_SC * _G_STC) / (_I_SC_STC + _U_STC*(temperature_RTC - _TEMPERATURE_STC)); 
}

void IRRADIANCE::writeIrradiance(){
    Serial.print(_now.second(), DEC);
    //Serial.println(" " + _time_read);      
    if(_now.second()%_time_read == 0){
        if (_irradianceFile) {
            Serial.print("Writing to Irradiance.txt...");
            _irradianceFile.print(_now.day(), DEC);
            _irradianceFile.print('/');
            _irradianceFile.print(_now.month(), DEC);
            _irradianceFile.print('/');
            _irradianceFile.print(_now.year(), DEC);
            _irradianceFile.print('(');
            _irradianceFile.print(_now.hour(), DEC);
            _irradianceFile.print(':');
            _irradianceFile.print(_now.minute(), DEC);
            _irradianceFile.print(':');
            _irradianceFile.print(_now.second(), DEC);
            _irradianceFile.print(')');
            _irradianceFile.print(' IRRADIANCE: ');
            _irradianceFile.print(getIrradiance());
            _irradianceFile.print('W/m^2');
            // close the file:
            _irradianceFile.close();
            Serial.println("done.");
        } else {
            // if the file didn't open, print an error:
            Serial.println("error opening test.txt");
        }
    }

}

