#include "IRRADIANCE.h"

void IRRADIANCE::setup(uint8_t sensor){
  _sensor = sensor;

  Serial.begin(9600);

  while (!Serial) ;

  _setupRTC();
  _configureSensor();
  _setupSD();
  _setupINA219();

  if(_isMonitor){
    if(_isConfigured){
      showFileConfigured();
    }else{
      showCommands();
    }
  }
  if(!_isATtinny){
    changeConfig();
  }else{
    if(_timeConfigure())
      _reloadTimeRead();
  }

}


bool IRRADIANCE::_timeConfigure(){
  if(_hourSensor != 0 || _minuteSensor != 0 || _secondSensor != 0){
    return true;
  }
  return false;
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
  String _filename = _isATtinny ?  "realTime.CSV" : "dados.CSV";
  if (!SD.begin(4)) {
      Serial.println("initialization failed!");
      while (1);
  }
}

void IRRADIANCE::_headerFile(File file){
  if(file){
    file.println("Data, Hora, Irradiancia, Corrente1, Tensao1, Corrente2, Tensao2, Corrente3, Tensao3");
    file.close();
  }else{
    Serial.println("Erro ao abrir o arquivo");
  }
}

void IRRADIANCE::showFileConfigured(){
  Serial.println("------------Arquivo-de-Configuracao-------------------");
  Serial.print("\tIntervalo entre medidas: ");Serial.print(_hourSensor);Serial.print(":");
  Serial.print(_minuteSensor);Serial.print(":");Serial.println(_secondSensor);
  Serial.print("\tMedicao Alta Velocidade: ");Serial.println(_isSpeedMode  ? "true" : "false");        
  Serial.print("\tNumero de canais: ");Serial.println(_numberChanels);
  Serial.print("\tAcionamento periodico: ");Serial.println(_isATtinny  ? "true" : "false");   
  Serial.println("-------------------------------------------------------");
}

void IRRADIANCE::_configureSensor(){
  if(EEPROMReadInt(0) == 1234){         //FLAG EEPROM configured
    _isConfigured = true;
    _hourSensor = EEPROMReadInt(2);
    _minuteSensor = EEPROMReadInt(4);
    _secondSensor = EEPROMReadInt(6);
    _isMonitor = EEPROM.read(8);
    _isSpeedMode = EEPROM.read(9);
    _numberChanels = EEPROMReadInt(10);
    _isATtinny = EEPROM.read(12);
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


//TODO REFAZER
void IRRADIANCE::writeCurrent(){
    // _currentFile = SD.open("001.txt", FILE_WRITE);
    // if(_currentFile){
    //     Serial.print("Writing current...");
    //     _formatTime(_currentFile);
    //     _currentFile.println(" TESTE");
    //     // _currentFile.print(_ina219.getCurrent_mA()/1000);
    //     // _currentFile.println("mA");
    // }else{
    //     // if the file didn't open, print an error:
    //     Serial.println("error opening test.txt");
    // }
    // _currentFile.close();
}

void IRRADIANCE::writeIrradiance(placas pvcellMeasure){
  DateTime now;
  File periodicFile = SD.open("DadosPeriodicos.csv",FILE_WRITE);
  now = _rtc.now();
  if (periodicFile) {
    Serial.print("Writing to Irradiance.txt...");
    _formatTime(periodicFile);
    // if(pvcellMeasure = monocristalina){
    //     file.print(getIrradiance(0));
    // }else
    //     file.print(getIrradiance(1));
    // file.println("W/m^2");
    // close the file:
    Serial.println("done.");
  } else {
    Serial.println("error opening teste DadosPeriodicos.csv");
  }
  periodicFile.close(); 
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
}

void IRRADIANCE::showCommands(){
  Serial.println("--------------------------------------------------------");
  Serial.println("|\t\t Shield Irradiance \t\t\t|");
  Serial.println("| Digite o comando para o modo de operacao");
  Serial.println("| 0 - Definir intervalo de Medidas no Modo Periódico");
  Serial.println("| 1 - Ativar/Desativar Medidas Periódicas");
  Serial.println("| 2 - Apagar arquivo com Medidas peródicas");
  Serial.println("| 3 - Definir numero de canais de alta velocidade");
  Serial.println("| 4 - Ativar/Desativar Medicoes de alta velocidade");
  Serial.println("| 5 - Desativar Monitor Serial");
}


void IRRADIANCE::showTimeConfigured(){
    Serial.print("Intervalo de ");
    Serial.print(_hourSensor);
    Serial.print(":");
    Serial.print(_minuteSensor);
    Serial.print(":");
    Serial.print(_secondSensor);
}

void IRRADIANCE::_writeTimeEEPROM(int hour, int minute, int second){
  EEPROMWriteInt(2, hour);
  EEPROMWriteInt(4, minute);
  EEPROMWriteInt(6, second);

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
    _future = _rtc.now() + TimeSpan(0, _hourSensor, _minuteSensor, _secondSensor);
    _timeMeasured= _future.unixtime();
}

void IRRADIANCE::_changeMode(int byteEEPROM, bool varTochange){
  bool isActive = EEPROM.read(byteEEPROM);
  EEPROM.write(12, !isActive);
  varTochange = !isActive;
  if(isActive)
    Serial.print("Desativando o ");
  else 
    Serial.print("Ativando o ");
  
  _isConfigured = true;
}

void IRRADIANCE::_setNumberChannels(){
  int number = getNumberSerial();
  EEPROMWriteInt(10, number); 
  _isConfigured = true;
  Serial.print(number);
  Serial.println(" canais ativados com sucesso!\n");
}


void IRRADIANCE::compareCommands(){
  char character;
  while(!_isConfigured) {
     character = Serial.read();
     if(character != '\n' && character != '\r'){
       switch (character){
        case '0':
          defineTime();
          break;
        case '1':
          _changeMode(12, _isATtinny);
          Serial.println("o Acionamento Periodico\n");
          break;
        case '2':
          //Apagar aquivo com Medidas 
          break;
        case '3':
          _setNumberChannels();
          break;
        case '4':
          _changeMode(9, _isSpeedMode);
          Serial.println("o modo de Alta Velocidade\n");
          break;
        case '5':
          Serial.println("Desativando Monitor Serial\n");
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


void IRRADIANCE::defineTime(){
    bool readChange = false;
    bool timeConfigured = false;
    char onChangeTime;

    setTimeSensors();
    Serial.println(" definido com sucesso");
    _reloadTimeRead();
}

void IRRADIANCE::setTimeSensors(){
    Serial.println("| Defina hora (0-12)\n");
    _hourSensor = getNumberSerial();
    Serial.println("| Defina minutos (0-60)\n");
    _minuteSensor = getNumberSerial();
    Serial.println("| Defina segundos (0-60)\n");
    _secondSensor =  getNumberSerial();
    _writeTimeEEPROM(_hourSensor, _minuteSensor, _secondSensor);
    showTimeConfigured(); 
}

int IRRADIANCE::getNumberSerial(){
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
void IRRADIANCE::checkTimeRead(){
  if(_timeMeasured == _rtc.now().unixtime() && _isATtinny){
    //writeIrradiance(0);
    writeCurrentVoltage();
    _reloadTimeRead();
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

void IRRADIANCE::attinyMode(){
  writeIrradiance(0);
  writeCurrent();
  //TODO SINAL DESLIGAR ARDUINO
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
  if(_isMonitor && !_isConfigured && !_isATtinny){ 
    compareCommands();
  } 
  checkTimeRead();
  delay(1000);
}

//TODO REFATORAR CODIGO (CAST TEMPERATURA)
//TODO DEFINIR NUMERO DE CANAIS
//TODO MEDICAO REAL TIME