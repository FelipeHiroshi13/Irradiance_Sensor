#include "IRRADIANCE.h"

void IRRADIANCE::setup(){
  Serial.begin(9600);

  while (!Serial) ;

  _setupRTC();
  _setupSD();
  _setupINA219();

  fileConfigure();

  _configureSensor();
  if(!_isConfigured){
    compareCommands(1);
  }
  if(_isConfigured){
    _setFlagConfigure();
  }
}

void IRRADIANCE::fileConfigure(){
  if(SD.exists("config.txt")){
    _configFile = SD.open("config.txt");
    compareCommands(0);
    _configFile.close();
  }
  if(_isConfigured){
    _setFlagConfigure();
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
  if (!SD.begin(4)) {
      Serial.println("initialization SD  failed!");
      while (1);
  }
}

void IRRADIANCE::_setupINA219(){
    if (! _ina219.begin()) {
        Serial.println("Failed to find INA219 chip");
        while (1) { delay(10); }
    }
    _ina219.setCalibration_16V_400mA();
    Serial.println("INA219");
}

void IRRADIANCE::_configureSensor(){
  if(EEPROMReadInt(0) == 1234){         //FLAG EEPROM configured
    _isConfigured = true;
    _numberTime = EEPROMReadInt(2);
    _typeTime =  EEPROM.read(4);
    _isATtinny = EEPROM.read(5);
    _numberChanels = EEPROMReadInt(6);
  }else{
      _isConfigured =false;
  }
}

void IRRADIANCE::_setFlagConfigure(){
  if(EEPROMReadInt(0) != 1234){
    EEPROMWriteInt(0, 1234);
  }
}

int IRRADIANCE::EEPROMReadInt(int address) {
   byte hiByte = EEPROM.read(address);
   byte loByte = EEPROM.read(address + 1);
   return word(hiByte, loByte); 
}

void IRRADIANCE::EEPROMWriteInt(int address, int value) {
   byte hiByte = highByte(value);
   byte loByte = lowByte(value);

   EEPROM.write(address, hiByte);
   EEPROM.write(address + 1, loByte);   
} 

void IRRADIANCE::compareCommands(int input){
  while(!_isConfigured){
    char command = input == 0 ? _readFile() : readCommand();
    switch (command){
    case 'p':
      EEPROM.write(5, 1);
      _isATtinny = true;
      _setTime(input);
      break;
    case 'a':
      _setNumberChannels(input);
      break;
    case 'd':
      _deleteFile(input);
      break;
    
    default:
      break;
    }
  }
}

char IRRADIANCE::readCommand(){
  while(true){
    if (Serial.available() > 0) {
      char command = Serial.read();
      if(command != '\n' && command != '\r' && command != ' '){
        return command;
      }
    }
  }
}

void IRRADIANCE::_setTime(int input){
  int time = (input == 0 ? _readFile() : readCommand()) - '0';
  char typeTime =  input == 0 ? _readFile() : readCommand();
  switch (typeTime){
    case 's':
      EEPROM.write(4, typeTime);
      _typeTime = 's';
      break;
    case 'm':
      EEPROM.write(4, typeTime);
      _typeTime = 'm';
      break;
    case 'h':
      EEPROM.write(4, typeTime);
      _typeTime = 'h';
      break;
    default:
      break;
  }
  _numberTime = time;
  EEPROMWriteInt(2, time); 
  _isConfigured = true;
}

void IRRADIANCE::_setNumberChannels(int input){
  int number = (input == 0 ? _readFile() : readCommand()) - '0';
  EEPROMWriteInt(6, number); 
  _isConfigured = true;
}

void IRRADIANCE::_deleteFile(int input){
  char typeFile = input == 0 ? _readFile() : readCommand();
  if(typeFile == 'p'){
    Serial.println("dados");
    SD.remove("dados.csv"); 
  }else if(typeFile == 'a'){
    Serial.println("realTime");
    SD.remove("realTime.csv");
  }
}

char IRRADIANCE::_readFile(){
  if(_configFile){
    while(true){
      char command = _configFile.read();
      if(command != '\n' && command != '\r' && command != ' ')
        return command;
    }
  }else{
    Serial.println("erro config.file");
  }
  return '\0';
}

void IRRADIANCE::_writeFile(){
  _filename = _isATtinny ?  "dados.CSV" : "realTime.CSV";
  if(!SD.exists(_filename)){
    _file = SD.open(_filename, FILE_WRITE);
    if(_file){
      Serial.println("header");
      _file.println("Data, Hora, Irradiancia, Corrente1, Tensao1, Corrente2, Tensao2, Corrente3, Tensao3");
      _file.close();
    }else{
      Serial.println("Erro Header");
    }
  }
  _file = SD.open(_filename, FILE_WRITE);
  if(_file){
    _formatTime(_file);
    Serial.println("entrei");
    //IRRADIANCE
    //CORRENTE/TENSAO CANAL1
    //CORRENTE/TENSAO CANAL2
    //CORRENTE/TENSAO CANAL3
    _file.close();
  }else{
    Serial.println("open file");
  }
}

void IRRADIANCE::_formatTime(File file){
    DateTime now;
    now = _rtc.now();
    file.print(now.day(), DEC);
    file.print('/');
    file.print(now.month(), DEC);
    file.print('/');
    file.print(now.year(), DEC);
    file.print(',');
    file.print(now.hour(), DEC);
    file.print(':');
    file.print(now.minute(), DEC);
    file.print(':');
    file.println(now.second(), DEC);
}

void IRRADIANCE::checkTimeRead(){
  DateTime now;
  now = _rtc.now();
  switch (_typeTime){
  case 's':
    if(now.second()%_numberTime == 0)
      _writeFile();
    break;
  case 'm':
    if(now.second()%_numberTime == 0)
      _writeFile();
    break;
  case 'h':
    if(now.second()%_numberTime == 0)
      _writeFile();
    break;
  default:
    break;
  }
}

void IRRADIANCE::runTimeSensor(){
  checkTimeRead();
  delay(1000);
}

// P 3M 
// A 3
// D P
// D A
