#include "IRRADIANCE.h"

SoftwareSerial swsri(8,9);

void IRRADIANCE::setup(){
  Serial.begin(19200);
  swsri.begin(9600);

  pinMode(13, OUTPUT);

  _setupRTC();
  _setupSD();
  _setupINA219();

  _writeFile();

  fileConfigure();

  _configureSensor();

  if(_isConfigured){
    _setFlagConfigure();
  }
  
  while (!Serial) ;

  Serial.println("Sensor Configurado");
}

void IRRADIANCE::fileConfigure(){
  Serial.println("file");
  if(SD.exists("config.txt")){
    Serial.println("config.txt");
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
        //Serial.flush();
        //abort();
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
  if (! INA219_1.begin()) {
    Serial.println("Failed to find INA219_1 chip");
    while (1) { delay(10); }
  }
  if (! INA219_2.begin()) {
    Serial.println("Failed to find INA219_2 chip");
    while (1) { delay(10); }
  }
  if (! INA219_3.begin()) {
    Serial.println("Failed to find INA219_3 chip");
    while (1) { delay(10); }
  }
  INA219_1.setCalibration_16V_400mA();
  INA219_2.setCalibration_16V_400mA();
  INA219_3.setCalibration_16V_400mA();
  Serial.println("SENSOR INiCIALIZADO");
}

void IRRADIANCE::writeINA219_1(File file){
  file.print(INA219_1.getBusVoltage_V());
  file.print(',');
  file.print(INA219_1.getCurrent_mA());
  file.print(',');
}

void IRRADIANCE::writeINA219_2(File file){
  file.print(INA219_2.getBusVoltage_V());
  file.print(',');
  file.print(INA219_2.getCurrent_mA());
  file.print(',');
}

void IRRADIANCE::writeINA219_3(File file){
  file.print(INA219_3.getBusVoltage_V());
  file.print(',');
  file.println(INA219_3.getCurrent_mA());
}

void IRRADIANCE::_configureSensor(){
  if(EEPROMReadInt(0) == 1234){     
    _isConfigured = true;
    _numberTime = EEPROMReadInt(2);
    _typeTime =  EEPROM.read(4);
    _isATtinny = EEPROM.read(5);
    _numberChanels = EEPROMReadInt(6);
    Serial.println(_numberTime);
    Serial.println(_typeTime);
  }else{
    _isConfigured =false;
  }
  if(_numberTime > 0){
    Serial.println("Enviando dados na flash");
    _sendTimeATtiny85();
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
  char command = input == 0 ? _readFile() : readCommand();
  switch (command){
  case 'p':
    EEPROM.write(5, 1);
    _isATtinny = true;
    _setTime(input);
    break;
  case 'a':
    Serial.println("Alta velocidade");
    EEPROM.write(5, 0);
    _isATtinny = false;
    _setNumberChannels(input);
    break;
  case 'd':
    _deleteFile(input);
    break;
  case '0':
    return;
    break;
  default:
    break;
  }
}

char IRRADIANCE::readCommand(){
  while(true){
    if (Serial.available() > 0) {
      char command = Serial.read();
      if(command != '\n' && command != '\r' && command != ' '){
        return command;
      }
    }else{
      return '/0';
    }
  }
}

int IRRADIANCE::timeRead(){
  int number;
  while (Serial.available () > 0){
    static char input[5];
    static uint8_t i;
    char c = Serial.read ();
    if(c == 's' || c == 'm' || c == 'h'){
      timeTimeRead = c;
    }
    if ( c != '\r' && i < 15 && c > 47 && c < 58){
      input[i++] = c;
    }else{
      input[i] = '\0';
      i = 0;
       number = atoi( input );
      if(number != 0){
        return number;;
      }
    }
  }
  return number;
}

void IRRADIANCE::_setTime(int input){
  int time = timeRead();
  switch (timeTimeRead){
    case 's':
      EEPROM.write(4, ':');
      _typeTime = ':';
      break;
    case 'm':
      EEPROM.write(4, ';');
      _typeTime = ';';
      break;
    case 'h':
      EEPROM.write(4, '<');
      _typeTime = '<';
      break;
    default:
      break;
  }
  _numberTime = time;
  isTimeset= false;
  _sendTimeATtiny85();
  EEPROMWriteInt(2, time); 
  _isConfigured = true;
  _setFlagConfigure();
}

void IRRADIANCE::_sendTimeATtiny85(){
  Serial.println("Enviando tempo para ATtiny");
  while(!isTimeset){
    swsri.println(_typeTime);
    delay(10);
    if(_numberTime > 9){
      swsri.print(_numberTime/10);
      swsri.println(_numberTime%10);
    }else{
      swsri.println(_numberTime);
    }   
    char c = swsri.read();
    if(c == '1')
      isTimeset = true;
  }
  Serial.println("Tempo Enviado");
}

void IRRADIANCE::_setNumberChannels(int input){
  int number = (input == 0 ? _readFile() : readCommand()) - '0';
  EEPROMWriteInt(6, number); 
  _numberChanels = number;
  _isConfigured = true;
  _setFlagConfigure();
}

void IRRADIANCE::_deleteFile(int input){
  char typeFile = input == 0 ? _readFile() : readCommand();
  if(typeFile == 'p'){
    Serial.println("dados.csv deletado");
    SD.remove("dados.csv"); 
  }else if(typeFile == 'a'){
    Serial.println("realtime.csv deletado");
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
    Serial.println("erro config.txt");
  }
  return '\0';
}

void IRRADIANCE::_headerFile(){
  if(!SD.exists(_filename)){
    _file = SD.open(_filename, FILE_WRITE);
    if(_file){
      _file.print("Data, Hora, Irradiancia, Corrente1, Tensao1, Corrente2, Tensao2, Corrente3, Tensao3");
      if(_filename == "realTime.CSV"){
        if(_numberChanels == 1){
          _file.println(",300 Hz");
        }else if(_numberChanels ==2){
          _file.println(",200 Hz");
        }else{
          _file.println(",100 Hz");
        }
      }else{
        _file.println(" ");
      }
      _file.close();
    }else{
      Serial.println("Erro abrir arquivo");
    }
  }
}

void IRRADIANCE::_writeFile(){
  _filename = _isATtinny ?  "dados.CSV" : "realTime.CSV";
  _headerFile();
  _file = SD.open(_filename, FILE_WRITE);
  if(_file){
    _formatTime(_file);
    _file.print(getIrradiance());
    _file.print(',');
    writeINA219_1(_file);
    writeINA219_2(_file);
    writeINA219_3(_file);
    _file.close();
    Serial.println("dados gravados");
  }else{
    Serial.println("Erro abrir arquivo");
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
    file.print(now.second(), DEC);
    file.print(',');
}

float IRRADIANCE::getISC_AD627(){
    float V_OC;

    V_OC = (float(analogRead(_sensor))/ float(1023))*3.3;;
    return(V_OC/float(470))/float(0.1);
}


float IRRADIANCE::getIrradiance(){
  I_SC = getISC_AD627();
  temperature_RTC = _rtc.getTemperature();;
  return(I_SC * _G_STC) / (_I_SC_STC_MONO + _U_STC*(temperature_RTC - _TEMPERATURE_STC)); 
}

void IRRADIANCE::speedMode(){
  while(true){
  _filename = "realTime.CSV";
  _headerFile();
  _file = SD.open(_filename, FILE_WRITE);
    if(_file){ 
      _file.print(',');
      _file.print(',');
      _file.print(',');
    if(_numberChanels ==  1){
      writeINA219_1(_file);
      _file.println(',');
    }else if(_numberChanels ==  2){
      writeINA219_1(_file);
      writeINA219_2(_file);
      _file.println(',');
    }else{
      writeINA219_1(_file);
      writeINA219_2(_file);
      writeINA219_3(_file);
    }
    _file.close();
    }else{
      Serial.println("Erro abrir arquivo");
    }
    if (Serial.available() > 0) {
      char command = Serial.read();
      if(command == 's'){
        _isConfigured = false;
        break;
      }
    }
  }
  Serial.println("Alta Velocidade desativada");
  EEPROM.write(5, 1);
  _isATtinny = true;
}

void IRRADIANCE::runTimeSensor(){
  // if(firsTimeRead){
  //   Serial.println("Gravando dados...");
  //   _writeFile();
  //   firsTimeRead = false;
  // }
  if (Serial.available() > 0){
    compareCommands(1);
  }
  if(_numberChanels > 0){
    Serial.print("Alta velocidade com "); Serial.print(_numberChanels); Serial.println(" canais"); 
    while(readCommand() != 's');
    Serial.println("Speed Mode");
    speedMode();
  }
}
