#include "IRRADIANCE.h"

void IRRADIANCE::setup(uint8_t sensor){
    _sensor = sensor;

    _definedCommands = false;

    Serial.begin(9600);

    while (!Serial) ;

    _setupRTC();
    //_setupINA219();
    _setupSD();
    _configureSensor();

    if(_isMonitor){
      if(_isConfigured){
        showFileConfigured();
        changeConfig();
      }else{
        showCommands();
      }
    }else{
      _definedCommands = true;
    }   
}


//Tratamento resposta Usuario [S/N]
char IRRADIANCE::_yesOrNo(){
  char c;
  while(true){
    c = Serial.read();
    if(c == 'S' || c == 'N')
      return c; 
  }
}

void IRRADIANCE::changeConfig(){
  Serial.println("Deseja Mudar a configuracao? [S/N]\n\n");
  if(_yesOrNo() == 'S'){
    showCommands();
    _isConfigured = false;
  }else{          
    return;                    
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
        _rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
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
}

void IRRADIANCE::_configureSensor(){
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
      _sencondATtinny = EEPROMReadInt(17);
    }
  }else{
      _isMonitor = true;
      _isConfigured =false;
  }
}

void IRRADIANCE::clearEEPROM(){
  for (int nL = 0; nL < 20; nL++) { 
      EEPROM.write(nL, 0);
  } 
}

void IRRADIANCE::setMonitor(bool monitor){
  EEPROM.write(8, monitor);
}

void IRRADIANCE::_setupINA219(){
    if (! _ina219.begin()) {
        Serial.println("Failed to find INA219 chip");
        while (1) { delay(10); }
    }
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


//TODO VERIFICAR TEMPERATURA (Talvez precise de cast)
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
    _currentFile = SD.open("001.txt", FILE_WRITE);
    if(_currentFile){
        Serial.print("Writing current...");
        _formatTime(_currentFile);
        _currentFile.println(" TESTE");
        // _currentFile.print(_ina219.getCurrent_mA()/1000);
        // _currentFile.println("mA");
    }else{
        // if the file didn't open, print an error:
        Serial.println("error opening test.txt");
    }
    _currentFile.close();
}

//TODO VERIFICAR IRRADIANCIA TEMPO DE MEDIDA
void IRRADIANCE::writeIrradiance(placas pvcell){
    // if(pvcell == monocristalina){
    //     _irradianceFile = SD.open(_filename, FILE_WRITE);
    // }else{
    //     _irradianceFile = SD.open("poliT.txt", FILE_WRITE);
    // }

    // _textIrradiance(_irradianceFile, pvcell);

    // _irradianceFile.close();
}

void IRRADIANCE::_textIrradiance(File file, placas pvcellMeasure){
  DateTime now;
  now = _rtc.now();
  if (file) {
    Serial.print("Writing to Irradiance.txt...");
    _formatTime(file);
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

void IRRADIANCE::_formatTime(File file){
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
    file.print(') ');
}

void IRRADIANCE::readSD(){
    _irradianceFile = SD.open("001.txt");
    if (_irradianceFile) {
        Serial.println("001.txt:");
        while (_irradianceFile.available()) {
            Serial.write(_irradianceFile.read());
        }
        _irradianceFile.close();
    } else {
        Serial.println("error opening test.txt");
    }
}

void IRRADIANCE::showCommands(){
  // if(!_definedCommands){
  //     compareCommands();
  // }
  Serial.println("--------------------------------------------------------");
  Serial.println("|\t\t Shield Irradiance \t\t\t|");
  Serial.println("| Digite o comando para o modo de operacao");
  Serial.println("| 0 - Definir Intervalo de Medida");
  Serial.println("| 1 - Armazenar DATA - HORA - Irradiancia(W/m^2)");
  Serial.println("| 2 - Armazenar HORA - Irradiancia(W/m^2)");
  Serial.println("| a - Tensao e Corrente Canal 1");
  Serial.println("| b - Tensao e Corrente Canal 2");
  Serial.println("| c - Tensao e Corrente Canal 3");
  Serial.println("| t - Tempo ATtiny85");
  Serial.println("| d -  Apagar dados armazenados");
  Serial.println("| s - Desativar monitor Serial\n\n");
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
  if(sensor == pvcell){        //Time Sensor Irradiance;
    EEPROMWriteInt(2, hour);
    EEPROMWriteInt(4, minute);
    EEPROMWriteInt(6, second);
  } else if (sensor == attiny){ //Time ATtinny
    EEPROMWriteInt(13, hour);
    EEPROMWriteInt(15, minute);
    EEPROMWriteInt(17, second);
  }
  if(!_isConfigured){
    _isConfigured = true;
    _setFlagConfigure();
  }
}

void IRRADIANCE::_setFlagConfigure(){
  if(EEPROMReadInt(0) != 1234){
    EEPROMWriteInt(0, 1234);
  }
}

void IRRADIANCE::showTime(){
    Serial.print(_rtc.now().year(), DEC);Serial.print('/');
    Serial.print(_rtc.now().month(), DEC);Serial.print('/');
    Serial.print(_rtc.now().day(), DEC);Serial.print(" (");
    Serial.print(") ");Serial.print(_rtc.now().hour(), DEC);
    Serial.print(':');Serial.print(_rtc.now().minute(), DEC);
    Serial.print(':');Serial.print(_rtc.now().second(), DEC);
    Serial.println();
}

void IRRADIANCE::_reloadTimeRead(){
    if(_hourSensor != 0 || _minuteSensor != 0 || _secondSensor != 0){
      _future = _rtc.now() + TimeSpan(0, _hourSensor, _minuteSensor, _secondSensor);
    }else if (_hourATtinny != 0 || _minuteATtinny != 0 || _sencondATtinny != 0){
      _future = _rtc.now() + TimeSpan(0, _hourATtinny, _minuteATtinny, _sencondATtinny);
    }
    
    _timeMeasured= _future.unixtime();
}

void IRRADIANCE::compareCommands(){
  char character;
  while(!_isConfigured) {
     character = Serial.read();
     if(character != '\n' && character != '\r'){
       switch (character){
        case '0':
          defineTime(pvcell);
          break;
        case '1':
          Serial.println("---DATA - HORA - Irradiancia(W/m^2)---");
          _caseTime = 0;
          break;
        case '2':
          Serial.println("---HORA - Irradiancia(W/m^2)---");
          _caseTime = 1;
          break;
        case 'a':
          Serial.println("Tensao e Corrente Canal 1");
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
          clearEEPROM();
          Serial.println("Dados EEPROM deletados com sucesso");
          break;
        case 's':
          Serial.println("Desativando Monitor Serial");
          EEPROM.write(8, 0);
          _isMonitor = false;
          break;
        default:
          break;
       }
     }
  }
  _finishConfigure();
}


void IRRADIANCE::defineTime(sensors sensor){
    bool readChange = false;
    bool timeConfigured = false;
    char onChangeTime;

    setTimeSensors(sensor);
    Serial.println(" definido com sucesso");
    _reloadTimeRead();
    if(sensor == attiny){
      _hourSensor = _minuteSensor = _secondSensor = 0;                  //Zerar tempo de medida, usar apenas do Attiny (MODO 2)
      _writeTimeEEPROM(0, 0, 0, pvcell);
      EEPROM.write(12, 1);      //Set true a attiny
    }else{
      _hourATtinny = _minuteATtinny = _sencondATtinny = 0;                  //Zerar tempo de medida, usar apenas do Attiny (MODO 2)
      _writeTimeEEPROM(0, 0, 0, attiny);
      EEPROM.write(12, 0);      //Set true a attiny
    }
}

void IRRADIANCE::setTimeSensors(sensors sensor){
    Serial.println("| Defina hora (0-12)\n");
    sensor == pvcell ?  _hourSensor = getTimeMeasure() : _hourATtinny = getTimeMeasure();
    Serial.println("| Defina minutos (0-60)\n");
    sensor == pvcell ? _minuteSensor = getTimeMeasure(): _minuteATtinny = getTimeMeasure();
    Serial.println("| Defina segundos (0-60)\n");
    sensor == pvcell ? _secondSensor =  getTimeMeasure() : _sencondATtinny = getTimeMeasure();
    if(sensor == pvcell)
      _writeTimeEEPROM(_hourSensor, _minuteSensor, _secondSensor, sensor);
    else
      _writeTimeEEPROM(_hourATtinny, _minuteATtinny, _sencondATtinny, sensor);
    showTimeConfigured(sensor); 
}

int IRRADIANCE::getTimeMeasure(){
  String timeRead;
  while(true){
    if(Serial.available() > 0){
      timeRead = Serial.readString();
      if(timeRead.length() > 2){
        return timeRead.toInt();
      }
    }
  }
}

//TODO VERIFICAR TEMPO INA 
void IRRADIANCE::checkTimeRead(sensors sensor){
  if(_timeMeasured == _rtc.now().unixtime()){
    //writeIrradiance(0);
    //writeCurrent();
    Serial.println("teste");
  }

    // if(sensor == pvcell){
    //     if(_timeMeasured == _rtc.now().unixtime()){
    //         //writeIrradiance(0);
    //         Serial.println("teste");
    //         _reloadTimeRead();
    //     }
    // }else if(sensor == ina219){
    //     if(_timeMeasured == _rtc.now().unixtime()){
    //         writeCurrent();
    //         _reloadTimeRead();
    //     }
    // }
}

void IRRADIANCE::_finishConfigure(){
  Serial.println("Deseja Fazer outra configuracao? [S/N]\n");
  if(_yesOrNo() == 'S'){
    _isConfigured = false;
    showCommands();
  }else{
    Serial.println("Arquivo configurado com sucesso\n");
    _isConfigured = true;
  }
}

void IRRADIANCE::runTimeSensor(){
  if(_isMonitor && !_isConfigured){ 
    compareCommands();
  } 
  checkTimeRead(pvcell);
  checkTimeRead(ina219);
  delay(1000);
}

//TODO REFATORAR CODIGO (CAST TEMPERATURA)
//TODO ACORDAR O ARDUINO E JA LER OS VALORES E DESLIGAR
//TODO DEFINIR NUMERO DE CANAIS