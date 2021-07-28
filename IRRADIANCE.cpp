#include "IRRADIANCE.h"

void IRRADIANCE::setup(uint8_t sensor){
    _sensor = sensor;

    _definedCommands = false;
    _definedTime = false;

    Serial.begin(9600);

    while (!Serial) ;

    showCommands();

    //_setupPVCELL();
    //_setupINA219();
    //_setupSD();
    //_setupRTC();
}

void IRRADIANCE::_setupPVCELL(){
    pinMode(9, OUTPUT);
    pinMode(10, OUTPUT);
    pinMode(3, INPUT);
    TCCR1A = 0;
    TCCR1A = (1 << COM1A1) | (1 << COM1B1) | (1 << WGM11);
    TCCR1B = 0;
    TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11);
    ICR1 = 40000;
    OCR1A = 3000;
    OCR1B = 4000;


    for(int i = 0; i < 600; i++){
        movePVcell();
    }
}

void IRRADIANCE::_setupRTC(){
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
    Serial.print("Initializing SD card...");

    if (!SD.begin(4)) {
        Serial.println("initialization failed!");
        while (1);
    }
    Serial.println("initialization done.");

    _irradianceFile = SD.open("001.txt", FILE_WRITE);

    // if the file opened okay, write to it:
    if (!_irradianceFile) {
        Serial.println("error opening irradiance.txt");
    }       
}

void IRRADIANCE::_setupINA219(){
    // Initialize the INA219.
    // By default the initialization will use the largest range (32V, 2A).  However
    // you can call a setCalibration function to change this range (see comments).
    if (! _ina219.begin()) {
        Serial.println("Failed to find INA219 chip");
        while (1) { delay(10); }
    }
    // To use a slightly lower 32V, 1A range (higher precision on amps):
    //ina219.setCalibration_32V_1A();
    // Or to use a lower 16V, 400mA range (higher precision on volts and amps):
    _ina219.setCalibration_16V_400mA();
    Serial.println("INA219");
}

float IRRADIANCE::getISC_AD627(){
    float V_OC;

    temperature_RTC = _rtc.getTemperature();

    V_OC = (float(analogRead(_sensor))/ float(1023))*3.3;;
    return(V_OC/float(470))/float(0.1);
}

float IRRADIANCE::getIrradiance(placas pvcell){
    if(pvcell == monocristalina){
        I_SC = getISC_AD627();
        temperature_RTC = 25;
        return(I_SC * _G_STC) / (_I_SC_STC_MONO + _U_STC*(temperature_RTC - _TEMPERATURE_STC)); 
    }else{
        I_SC = _ina219.getCurrent_mA()/1000;
        temperature_RTC = _rtc.getTemperature();
        return(I_SC * _G_STC) / (_I_SC_STC_POLI + _U_STC*(temperature_RTC - _TEMPERATURE_STC)); 
    }

}



float IRRADIANCE::getINA219current(){
    return (_ina219.getCurrent_mA()/100);
}

void IRRADIANCE::writeCurrent(){
    _currentFile = SD.open("current.txt", FILE_WRITE);
    if(_currentFile){
        Serial.print("Writing current...");
        _currentFile.print(_now.day(), DEC);
        _currentFile.print('/');
        _currentFile.print(_now.month(), DEC);
        _currentFile.print('/');
        _currentFile.print(_now.year(), DEC);
        _currentFile.print('(');
        _currentFile.print(_now.hour(), DEC);
        _currentFile.print(':');
        _currentFile.print(_now.minute(), DEC);
        _currentFile.print(':');
        _currentFile.print(_now.second(), DEC);
        _currentFile.print(')');
        _currentFile.print(_ina219.getCurrent_mA()/1000);
        _currentFile.println("mA");
        _currentFile.print(" IRRADIANCE: ");
    }else{
        // if the file didn't open, print an error:
        Serial.println("error opening test.txt");
    }
    _currentFile.close();
}

void IRRADIANCE::writeIrradiance(placas pvcell){
    if(pvcell == monocristalina){
        _irradianceFile = SD.open("mono.txt", FILE_WRITE);
    }else{
        _irradianceFile = SD.open("poli.txt", FILE_WRITE);
    }

    if(_now.second()%_timeRead == 0){
       _textIrradiance(_irradianceFile, pvcell);
    }
    _irradianceFile.close();

}

bool IRRADIANCE::isTimeRead(){
    _now = _rtc.now();
    if(_now.unixtime() == _timeRead){
        return true;
    }
    return false;
}

int IRRADIANCE::getRTCseconds(){
    _now = _rtc.now();
    return _now.second();
}

void IRRADIANCE::_textIrradiance(File file, placas pvcell){
    _now = _rtc.now();
     if (file) {
            Serial.print("Writing to Irradiance.txt...");
            file.print(_now.day(), DEC);
            file.print('/');
            file.print(_now.month(), DEC);
            file.print('/');
            file.print(_now.year(), DEC);
            file.print('(');
            file.print(_now.hour(), DEC);
            file.print(':');
            file.print(_now.minute(), DEC);
            file.print(':');
            file.print(_now.second(), DEC);
            file.print(')');
            file.print(" IRRADIANCE: ");
            if(pvcell = monocristalina){
                file.print(getIrradiance(0));
                writeCurrent();
            }else
                file.print(getIrradiance(1));
            file.println("W/m^2");
            // close the file:
            Serial.println("done.");
        } else {
            // if the file didn't open, print an error:
            Serial.println("error opening test.txt");
        }
}

void IRRADIANCE::writeCurrentVoltage(){
    float shuntvoltage = 0;
    float busvoltage = 0;
    float current_mA = 0;
    float loadvoltage = 0;
    float power_mW = 0;

    shuntvoltage = _ina219.getShuntVoltage_mV();
    busvoltage = _ina219.getBusVoltage_V();
    current_mA = _ina219.getCurrent_mA();
    power_mW = _ina219.getPower_mW();
    loadvoltage = busvoltage + (shuntvoltage / 1000);
  
    Serial.print("Bus Voltage:   "); Serial.print(busvoltage); Serial.println(" V");
    Serial.print("Shunt Voltage: "); Serial.print(shuntvoltage); Serial.println(" mV");
    Serial.print("Load Voltage:  "); Serial.print(loadvoltage); Serial.println(" V");
    Serial.print("Current:       "); Serial.print(current_mA); Serial.println(" mA");
    Serial.print("Power:         "); Serial.print(power_mW); Serial.println(" mW");
    Serial.println("");
}


void IRRADIANCE::getTimeTemperature(){
    
     if (_rtc.lostPower()) {
        Serial.println("RTC lost power, let's set the time!");
        // When time needs to be set on a new device, or after a power loss, the
        // following line sets the RTC to the date & time this sketch was compiled
        _rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
        // This line sets the RTC with an explicit date & time, for example to set
        // January 21, 2014 at 3am you would call:
        // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));
    }
    _now = _rtc.now();
    Serial.print(_now.hour(), DEC);
    Serial.print(':');
    Serial.print(_now.minute(), DEC);
    Serial.print(':');
    Serial.print(_now.second(), DEC);
    Serial.print(')');
    Serial.print(" temp: ");
    Serial.println(_rtc.getTemperature());

}


void IRRADIANCE::readSD(){
    // re-open the file for reading:
    _irradianceFile = SD.open("001.txt");
    if (_irradianceFile) {
        Serial.println("001.txt:");

        // read from the file until there's nothing else in it:
        while (_irradianceFile.available()) {
            Serial.write(_irradianceFile.read());
        }
        // close the file:
        _irradianceFile.close();
    } else {
        // if the file didn't open, print an error:
        Serial.println("error opening test.txt");
    }
}

void IRRADIANCE::movePVcell(){
    digitalWrite(3, HIGH);
    delay(1);

    _topleft = analogRead(A1);
    _topright = analogRead(A2);
    _downleft = analogRead(A3);
    _downright = analogRead(A4);

    if (_topleft > _topright+2) {
        OCR1A = OCR1A + 5;
        delay(_waittime);
    }
    if (_downleft > _downright+2) {
        OCR1A = OCR1A + 5;
        delay(_waittime);
    }
    if (_topleft < _topright+2) {
        OCR1A = OCR1A - 5;
        delay(_waittime);
    }
    if (_downleft < _downright+2) {
        OCR1A = OCR1A - 5;
        delay(_waittime);
    }
    if (OCR1A > 5000) {
        OCR1A = 5000;
    }
    if (OCR1A < 1000) {
        OCR1A = 1000;
    }
    if (_topleft > _downleft) {
        OCR1B = OCR1B - 5;
        delay(_waittime);
    }
    if (_topright > _downright+2) {
        OCR1B = OCR1B - 5;
        delay(_waittime);
    }
    if (_topleft < _downleft+2) {
        OCR1B = OCR1B + 5;
        delay(_waittime);
    }
    if (_topright < _downright) {
        OCR1B = OCR1B + 5;
        delay(_waittime);
    }
    if (OCR1B > 4200) {
        OCR1B = 4200;
    }
    if (OCR1B < 3000) {
        OCR1B = 3000;
    }
    digitalWrite(3, LOW);
}

void IRRADIANCE::showCommands(){
  if(!_definedCommands){
      compareCommands();
  }
  Serial.println("--------------------------------------------------------");
  Serial.println("|\t\t Shield Irradiance \t\t\t|");
  Serial.println("| Digite o comando para o modo de operacao");
  Serial.println("| 1 - Armazenar DATA - HORA - Irradiancia(W/m^2)");
  Serial.println("| 2 - Armazenar DATA - Irradiancia(W/m^2)");
  Serial.println("| 3 - Armazenar HORA - Irradiancia(W/m^2)");
  Serial.println("| a - Tesao e Corrente Canal 1");
  Serial.println("| b - Tesao e Corrente Canal 2");
  Serial.println("| c - Tesao e Corrente Canal 3");
  Serial.println("| t - Tempo ATtiny85");
  Serial.println("| d -  Apagar dados armazenados");
  Serial.println("| s - Desativar monitor Serial\n\n");
}

String IRRADIANCE::getValue(String data, char separator, int index)
{
  int found = 0;
  int strIndex[] = {0, -1};
  int maxIndex = data.length()-1;

  for(int i=0; i<=maxIndex && found<=index; i++){
    if(data.charAt(i)==separator || i==maxIndex){
        found++;
        strIndex[0] = strIndex[1]+1;
        strIndex[1] = (i == maxIndex) ? i+1 : i;
    }
  }

  return found>index ? data.substring(strIndex[0], strIndex[1]) : "";
}


void IRRADIANCE::setTimeMeasure(){

  String timeRead = "";
  char character;
  while(Serial.available()) {
       character = Serial.read();
       timeRead.concat(character);
  }
  
  if (timeRead != "" && timeRead.indexOf(":") >  0) {
    String hourIrradiance = getValue(timeRead,':',0);
    String minuteIrradiance = getValue(timeRead,':',1);
    String secondIrradiance = getValue(timeRead,':',2);
    if((hourIrradiance.toInt() + minuteIrradiance.toInt() + secondIrradiance.toInt()) > 0){
      _definedTime = true;
      Serial.print("Intervalo de ");
      Serial.print(hourIrradiance);
      Serial.print(":");
      Serial.print(minuteIrradiance);
      if(secondIrradiance.toInt() > 0){
         Serial.print(":");
         Serial.print(secondIrradiance);
      }
      Serial.println("definido com sucesso \n");
      _definedCommands =  false;
      _future = _rtc.now() + TimeSpan(0, hourIrradiance.toInt(), minuteIrradiance.toInt(),secondIrradiance.toInt());
      _timeRead = _future.unixtime();
      Serial.println(_timeRead);
      Serial.println(_rtc.now().unixtime());
    }else{
      Serial.println("Digite um tempo válido no formato h:m:s");
    }
  }
}


void IRRADIANCE::compareCommands(){

  char character;
  while(Serial.available()) {
     character = Serial.read();
     if(character != '\n' && character != '\r'){
       switch (character){
        case '1':
          _definedCommands = true;
          Serial.println("caso 1");
          Serial.println("| Defina o intervalo de tempo (h:m:s)\n");
          while(!_definedTime){
            setTimeMeasure();
          }
          break;
        case '2':
          Serial.println("caso 2");
          break;
        case '3':
          Serial.println("caso 3");
          break;
        case 'a':
          Serial.println("caso a");
          break;
        case 'b':
          Serial.println("caso b");
          break;
        case 'c':
          Serial.println("caso c");
          break;
        case 'd':
          Serial.println("Deletando Dados");
          //showAdvancedCommands();
          break;
        case 's':
          Serial.println("Desativando Monitor Serial");
          //showAdvancedCommands();
          break;
        default:
          Serial.println("Digite um comando válido");
          break;
       }
     }
  }
  _definedTime = false;

}


void IRRADIANCE::runTimeSensor(){
  if(!_definedCommands){
    compareCommands();
  }
}