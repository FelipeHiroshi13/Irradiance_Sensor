#include "IRRADIANCE.h"

void IRRADIANCE::setup(uint8_t sensor){
    _sensor = sensor;

    _definedCommands = false;
    _definedTime = false;

    Serial.begin(9600);

    while (!Serial) ;

    showCommands();

    _setupRTC();
    //_setupINA219();
    _setupSD();
    
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
    _configureFile = SD.open("con.txt");
    

    if(_configureFile){
      _isConfigured = true;
      _readTime();
      _setMonitor();
      _setRealTime();
      _setNumberChannels();
    }else{
       Serial.println("No Configure.txt");
      _isConfigured = false;
    }
  
    // if the file opened okay, write to it:
    /*if (!_irradianceFile) {
        Serial.println("error opening irradiance.txt");
    } */ 

    _irradianceFile.close();  
    _configureFile.close();

}

void IRRADIANCE::readSetup(char input[]){
  int i = 0;
  char c;
  //myFile = SD.open("CONFIG.txt");
  while((c = _configureFile.read()) != '=');
  while ((c = _configureFile.read()) != '\n') { 
    if(c != ' ' && c != '\r'){
      input[i] = c; 
      i++;
    }
  }

  input[i] = '\0'; 
  Serial.println(input);
}

void IRRADIANCE::_readTime(){
  char time_define[10];
  readSetup(time_define);
  setTimeMeasure(time_define);
}

void IRRADIANCE::_setMonitor(){
  char isMonitorText[10];
  readSetup(isMonitorText);
  if(strcmp(isMonitorText, "true") == 0){
    _isMonitor = true;
  }
  // if(strcmp(isMonitorText, "false") == 0){
  //   _isMonitor = false;
  // }else{
  //   Serial.println("ERRO VALOR DE MONITOR INVALIDO");
  // }
}

void IRRADIANCE::_setRealTime(){
  char isRealTimeText[10];
  readSetup(isRealTimeText);
  if(strcmp(isRealTimeText, "true") == 0)
    _isRealTime = true;
  // } else if(strcmp(isRealTimeText, "false") == 0){
  //   _isRealTime = false;
  // }else{
  //   Serial.println("ERRO VALOR DE REAL TIME INVALIDO");
  // }
}

void IRRADIANCE::_setNumberChannels(){
  char numberChannels[3];
  readSetup(numberChannels);
  _numberChanels = numberChannels[0] - '0';
  Serial.println(_numberChanels);
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
    _now = _rtc.now();
    return _now.second();
}

void IRRADIANCE::_textIrradiance(File file, placas pvcellMeasure){
  _now = _rtc.now();
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
    if(_caseTime == 0){
        file.print(_now.day(), DEC);
        file.print('/');
        file.print(_now.month(), DEC);
        file.print('/');
        file.print(_now.year(), DEC);
    }
    file.print('(');
    file.print(_now.hour(), DEC);
    file.print(':');
    file.print(_now.minute(), DEC);
    file.print(':');
    file.print(_now.second(), DEC);
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
  //Serial.println("| 2 - Armazenar DATA - Irradiancia(W/m^2)");
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

void IRRADIANCE::setTimeMeasure(String timeRead){
  
  if (timeRead != "" && timeRead.indexOf(":") >  0) {
    String hourSensor = getValue(timeRead,':',0);
    String minuteSensor = getValue(timeRead,':',1);
    String secondSensor = getValue(timeRead,':',2);
    if((hourSensor.toInt() + minuteSensor.toInt() + secondSensor.toInt()) > 0){
      _definedTime = true;
      Serial.print("Intervalo de ");
      Serial.print(hourSensor);
      Serial.print(":");
      Serial.print(minuteSensor);
      if(secondSensor.toInt() > 0){
         Serial.print(":");
         Serial.print(secondSensor);
      }
      Serial.println("definido com sucesso \n");
      _definedCommands =  false;
      _hourSensor = hourSensor.toInt();
      _minuteSensor = minuteSensor.toInt();
      _secondSensor = secondSensor.toInt();
      _future = _rtc.now() + TimeSpan(0, hourSensor.toInt(), minuteSensor.toInt(),secondSensor.toInt());
      _reloadTimeRead();

    }else{
      Serial.println("Digite um tempo válido no formato h:m:s");
    }
  }
}



void IRRADIANCE::showTime(){
    _now = _rtc.now();
    Serial.print(_now.year(), DEC);
    Serial.print('/');
    Serial.print(_now.month(), DEC);
    Serial.print('/');
    Serial.print(_now.day(), DEC);
    Serial.print(" (");
    Serial.print(") ");
    Serial.print(_now.hour(), DEC);
    Serial.print(':');
    Serial.print(_now.minute(), DEC);
    Serial.print(':');
    Serial.print(_now.second(), DEC);
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
        case 'd':
          Serial.println("Deletando Dados");
          //showAdvancedCommands();
          break;
        case 's':
          Serial.println("Desativando Monitor Serial");
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
    _definedCommands = true;

    Serial.println("| Defina o intervalo de tempo (h:m:s)\n");
    while(!_definedTime){
      String timeRead = "";
      char character;
      while(Serial.available()) {
        character = Serial.read();
        timeRead.concat(character);
      }
      setTimeMeasure(timeRead);
    }
}


void IRRADIANCE::checkTimeRead(sensors sensor){
    if(sensor == pvcell){
        if(_timeMeasured == _rtc.now().unixtime()){
            writeIrradiance(0);
            _reloadTimeRead();
        }
    }else if(sensor == ina219){
        if(_timeMeasured == _rtc.now().unixtime()){
            writeCurrent();
            _reloadTimeRead();
        }
    }
}



void IRRADIANCE::runTimeSensor(){

  if(!_definedCommands && !_isConfigured){
    compareCommands();
  }
  //showTime();
  checkTimeRead(pvcell);
  checkTimeRead(ina219);
  delay(1000);
}