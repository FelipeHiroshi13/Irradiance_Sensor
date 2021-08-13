#include "IRRADIANCE.h"

void IRRADIANCE::setup(uint8_t sensor){
    _sensor = sensor;

    _definedCommands = false;
    _definedTime = false;

    Serial.begin(9600);

    while (!Serial) ;

    _setupRTC();
    //_setupINA219();
    _setupSD();
    _configureSensor();

    if(_isMonitor){
      showFileConfigured();
      showCommands();
    }else{
      _definedCommands = true;
      _definedTime = true;
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

    //TODO nome arquivo com data
    //sprintf(_filename, "%02d%02d%02d.txt", _rtc.now().day(),_rtc.now().month(), _rtc.now().year());

    //Serial.print(_filename);

    if (!SD.begin(4)) {
        Serial.println("initialization failed!");
        while (1);
    }
    //Serial.println("initialization done.");
    //Serial.println("")

    _irradianceFile = SD.open("teste.txt");

    // if the file opened okay, write to it:
    /*if (!_irradianceFile) {
        Serial.println("error opening irradiance.txt");
    } */ 

    _irradianceFile.close();  

}

void IRRADIANCE::showFileConfigured(){
  Serial.println("------------Arquivo-de-Configuracao-------------------");
  Serial.print("\tIntervalo entre medidas: ");Serial.print(_hourSensor);Serial.print(":");
  Serial.print(_minuteSensor);Serial.print(":");Serial.println(_secondSensor);
  Serial.print("\tReal Time: ");Serial.println(_isRealTime  ? "true" : "false");        
  Serial.print("\tNumero de canais: ");Serial.println(_numberChanels);
  Serial.print("\tAcionamento periodico: ");Serial.println(_isATttinny  ? "true" : "false");   
  if(_isATttinny){
    Serial.print("\tTempo Acionamento Periodico: ");Serial.print(_hourATtinny);Serial.print(":");
    Serial.print(_minuteATtinny);Serial.print(":");Serial.println(_sencondATtinny);
  }  
  Serial.println("-------------------------------------------------------");
  Serial.println("");
  Serial.println("");
  Serial.println("");
}

void IRRADIANCE::_configureSensor(){
  EEPROM.write(8, 1);
  if(EEPROMReadInt(0) == 1234){         //FLAG EEPROM configured
    _isConfigured = true;
    _hourSensor = EEPROMReadInt(2);
    _minuteSensor = EEPROMReadInt(4);
    _secondSensor = EEPROMReadInt(6);
    _isMonitor = EEPROM.read(8);
    _isRealTime = EEPROM.read(9);
    _numberChanels = EEPROMReadInt(10);
    _isATttinny = EEPROM.read(12);
    if(_isATttinny){
      _hourATtinny = EEPROMReadInt(13);
      _minuteATtinny = EEPROMReadInt(15);
      _sencondATtinny = EEPROMReadInt(16);
    }

  }else{
      _isMonitor = true;
      _isConfigured =false;
  }
}

void IRRADIANCE::clearEEPROM(){
  for (int nL = 0; nL < 100; nL++) { 
      EEPROM.write(nL, 0);
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

void IRRADIANCE::EEPROMWriteInt(int address, int value) {
   byte hiByte = highByte(value);
   byte loByte = lowByte(value);

   EEPROM.write(address, hiByte);
   EEPROM.write(address + 1, loByte);   
}

int IRRADIANCE::EEPROMReadInt(int address) {
   byte hiByte = EEPROM.read(address);
   byte loByte = EEPROM.read(address + 1);
   
   return word(hiByte, loByte); 
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
    Serial.print("entrei");
    _currentFile = SD.open("001.txt", FILE_WRITE);
    if(_currentFile){
        Serial.print("Writing current...");
        _formatTime(ina219, _currentFile);
        _currentFile.println(" TESTE");
        // _currentFile.print(_ina219.getCurrent_mA()/1000);
        // _currentFile.println("mA");
    }else{
        // if the file didn't open, print an error:
        Serial.println("error opening test.txt");
    }
    _currentFile.close();
}

void IRRADIANCE::writeIrradiance(placas pvcell){
    if(pvcell == monocristalina){
        _irradianceFile = SD.open(_filename, FILE_WRITE);
    }else{
        _irradianceFile = SD.open("poliT.txt", FILE_WRITE);
    }

    _textIrradiance(_irradianceFile, pvcell);

    _irradianceFile.close();

}

int IRRADIANCE::getRTCseconds(){
    DateTime now;
    now = _rtc.now();
    return now.second();
}

void IRRADIANCE::_textIrradiance(File file, placas pvcellMeasure){
  DateTime now;
  now = _rtc.now();
  if (file) {
    Serial.print("Writing to Irradiance.txt...");
    _formatTime(pvcell, file);
    file.println("TESTE");
    // if(pvcellMeasure = monocristalina){
    //     file.print(getIrradiance(0));
    // }else
    //     file.print(getIrradiance(1));
    // file.println("W/m^2");
    // close the file:
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening teste Irradiance.txt");
  }
  file.close(); 
}

void IRRADIANCE::writeCurrentVoltage(){
    float busvoltage = 0;
    float current_mA = 0;

    busvoltage = _ina219.getBusVoltage_V();
    current_mA = _ina219.getCurrent_mA();
  
    Serial.print("Bus Voltage:   "); Serial.print(busvoltage); Serial.println(" V");
    Serial.print("Current:       "); Serial.print(current_mA); Serial.println(" mA");
    Serial.println("");
}

void IRRADIANCE::_formatTime(sensors sensor, File file){
    DateTime now;
    now = _rtc.now();
    if(_caseTime == 0){
        file.print(now.day(), DEC);
        file.print('/');
        file.print(now.month(), DEC);
        file.print('/');
        file.print(now.year(), DEC);
    }
    file.print('(');
    file.print(now.hour(), DEC);
    file.print(':');
    file.print(now.minute(), DEC);
    file.print(':');
    file.print(now.second(), DEC);
    file.print(')');
    if(sensor = pvcell){
        file.print(" IRRADIANCE: ");
    }else{
        file.print(" CURRENT: ");
    }
    
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

void IRRADIANCE::showCommands(){
  if(!_definedCommands){
      compareCommands();
  }
  Serial.println("--------------------------------------------------------");
  Serial.println("|\t\t Shield Irradiance \t\t\t|");
  Serial.println("| Digite o comando para o modo de operacao");
  Serial.println("| 1 - Armazenar DATA - HORA - Irradiancia(W/m^2)");
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

void IRRADIANCE::setTimeMeasure(String timeRead, sensors sensor){
  if (timeRead != "" && timeRead.indexOf(":") >  0) {
    String hourSensor = getValue(timeRead,':',0);
    String minuteSensor = getValue(timeRead,':',1);
    String secondSensor = getValue(timeRead,':',2);
    if((hourSensor.toInt() + minuteSensor.toInt() + secondSensor.toInt()) > 0){
      _definedTime = true;
      Serial.println("definido com sucesso \n");
      _definedCommands =  false;
      if(sensor == pvcell){
        _hourSensor = hourSensor.toInt();
        _minuteSensor = minuteSensor.toInt();
        _secondSensor = secondSensor.toInt();
        showTimeConfigured(pvcell);
        _writeTimeEEPROM(_hourSensor, _minuteSensor, _secondSensor, sensor);
      }else if(sensor == attiny){
        _hourATtinny = hourSensor.toInt();
        _minuteATtinny = minuteSensor.toInt();
        _sencondATtinny = secondSensor.toInt();
        _writeTimeEEPROM(_hourSensor, _minuteSensor, _secondSensor, sensor);
        EEPROM.write(12, 1);
      }
      _future = _rtc.now() + TimeSpan(0, hourSensor.toInt(), minuteSensor.toInt(),secondSensor.toInt());
      _reloadTimeRead();
      _writeTimeEEPROM(_hourSensor, _minuteSensor, _secondSensor, 0);
      _setFlagConfigure();
    }else{
      Serial.println("Digite um tempo válido no formato h:m:s");
    }

  }
}

void IRRADIANCE::showTimeConfigured(sensors sensor){
    Serial.print("Intervalo de ");
    Serial.print(sensor==pvcell ? _hourSensor: _hourATtinny);
    Serial.print(":");
    Serial.print(sensor==pvcell ? _minuteSensor: _minuteATtinny);
    if(_secondSensor > 0){
        Serial.print(":");
        Serial.print(sensor==pvcell ? _secondSensor: _sencondATtinny);
    }
}

void IRRADIANCE::_writeTimeEEPROM(int hour, int minute, int second, sensors sensor){
  // Serial.println();
  // Serial.println("AAAAAAAAAAAAAAAAAAA");
  // Serial.println(second);

  if(sensor == pvcell){        //Time Sensor Irradiance;
    EEPROMWriteInt(2, hour);
    EEPROMWriteInt(4, minute);
    EEPROMWriteInt(6, second);
  } else if (sensor == attiny){
    EEPROMWriteInt(14, hour);
    EEPROMWriteInt(16, minute);
    EEPROMWriteInt(18, second);
  }
}

void IRRADIANCE::_setFlagConfigure(){
  if(EEPROMReadInt(0) != 1234){
    EEPROMWriteInt(0, 1234);
  }
}



void IRRADIANCE::showTime(){
    DateTime now;
    now = _rtc.now();
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(" (");
    Serial.print(") ");
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.println();
}

void IRRADIANCE::_reloadTimeRead(){
    _future = _rtc.now() + TimeSpan(0, _hourSensor, _minuteSensor, _secondSensor);
    _timeMeasured= _future.unixtime();
}

void IRRADIANCE::compareCommands(){

  char character;
  while(Serial.available()) {
     character = Serial.read();
     if(character != '\n' && character != '\r'){
       switch (character){
        case '1':
          Serial.println("---DATA - HORA - Irradiancia(W/m^2)---");
          _caseTime = 0;
          defineTime(pvcell);
          break;
        // case '2':
        //   Serial.println("---DATA - Irradiancia(W/m^2)---");
        //   defineTime(pvcell);
        //   break;
        case '3':
          _caseTime = 1;
          Serial.println("---HORA - Irradiancia(W/m^2)---");
          defineTime(pvcell);
          break;
        case 'a':
          Serial.println("Tensao e Corrente Canal 1");
          defineTime(ina219);
          break;
        case 'b':
          Serial.println("Tensao e Corrente Canal 2");
          break;   
        case 'c':
          Serial.println("Tensao e Corrente Canal 3");
          break;
        case 't':
          Serial.println("Defina tempo Acionamento Periodico (h:m:s)");
          defineTime(attiny);
          break;
        case 'd':
          Serial.println("Deletando Dados");
          //showAdvancedCommands();
          break;
        case 's':
          Serial.println("Desativando Monitor Serial");
          EEPROM.write(0, 8);
          _isMonitor = false;
          //showAdvancedCommands();
          break;
        default:
          Serial.println("Digite um comando valido");
          break;
       }
     }
  }
  _definedTime = false;
}

void IRRADIANCE::defineTime(sensors sensor){
    bool readChange = false;
    bool timeConfigured = false;
    char onChangeTime;
    
    if(sensor == pvcell){
      if(_hourSensor != 0 || _minuteSensor != 0 || _secondSensor != 0)
        timeConfigured = true;
    }
    if(sensor == attiny){
      if(_hourATtinny != 0 || _minuteATtinny != 0 || _sencondATtinny != 0)
        timeConfigured = true;
    }


    if(timeConfigured){
      Serial.print("Deseja trocar o tempo definido de ");
      showTimeConfigured(sensor);
      Serial.println(" ? [S/N]");
      while(!readChange) {
        onChangeTime = Serial.read();
        if(onChangeTime == 'S' || onChangeTime == 'N')
          readChange = true; 
      }
    }
    if(onChangeTime == 'S'){
      Serial.println("| Defina o intervalo de tempo (h:m:s)\n");
      while(!_definedTime){
        String timeRead = "";
        char character;
        while(Serial.available()) {
          character = Serial.read();
          timeRead.concat(character);
        }
        setTimeMeasure(timeRead, sensor);
      }
    }else{
      Serial.println("Tempo nao alterado");
      return;
    }
    
}

//TODO VERIFICAR TEMPO INA 
void IRRADIANCE::checkTimeRead(sensors sensor){
    if(sensor == pvcell){
        if(_timeMeasured == _rtc.now().unixtime()){
            writeIrradiance(0);
            _reloadTimeRead();
        }
    }
    // }else if(sensor == ina219){
    //     if(_timeMeasured == _rtc.now().unixtime()){
    //         writeCurrent();
    //         _reloadTimeRead();
    //     }
    // }
}



void IRRADIANCE::runTimeSensor(){
  if(_isMonitor){ 
    compareCommands();
  } 
  //showTime();
  checkTimeRead(pvcell);
  checkTimeRead(ina219);
  delay(1000);
}