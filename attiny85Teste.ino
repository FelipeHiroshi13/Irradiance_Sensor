#include <SoftwareSerial.h>
SoftwareSerial swsri(3,4);

char c = ' ';
bool secondsTime = false;
bool minuteTime = false;
bool hourTime = false;
int timeConfigured = 0;

byte arduino = 1;
byte RX = 3;
byte TX = 4;

void setup()
{
  pinMode(arduino,OUTPUT);
  swsri.begin(9600);
  digitalWrite(arduino,LOW);
  while(!((secondsTime || minuteTime || hourTime) && timeConfigured != 0)){
    configureTime();
  }

}

void loop()
{ 
  //turnOnArduino();
  if((secondsTime || minuteTime || hourTime) && timeConfigured != 0){
    swsri.println('1');
  }
  configureTime();
  turnOffArduino();
}

//TROCAR TIPO DE TEMPO
void configureTime(){
  while (swsri.available () > 0){
      static char input[5];
      static uint8_t i;
      char c = swsri.read ();
      if(c == '.' && timeConfigured > 0){
       turnOffArduino();
      }
      if(c >  57 && c < 62){
        secondsTime = false;
        minuteTime= false;
        hourTime = false;
        timeConfigured = 0;
        if(c == ':'){
          secondsTime = true;
        }else if(c == ';'){
          minuteTime= true;
        }else if(c == '<'){
          hourTime = true;
        }
      }else{
        if ( c != '\r' && i < 15 ){
          input[i++] = c;
        }else{
          input[i] = '\0';
          i = 0;
          int number = atoi( input );
          if(number != 0){
            timeConfigured = convertToSeconds(number);
            swsri.println('1');
            break;
          }
        }
      }
    }
}

void turnOffArduino(){
  digitalWrite(arduino, HIGH); 
  digitalWrite(RX, LOW);
  digitalWrite(TX, LOW);
  delayLong(timeConfigured);
  turnOnArduino();
  delay(5000);
  
}

void delayLong(int time){
  for(int i = 0; i < time; i++){
    delay(1000);
  }
}
void turnOnArduino(){
  digitalWrite(arduino, LOW);
  digitalWrite(RX, HIGH);
  digitalWrite(TX, HIGH);
}

int convertToSeconds(int number){
  if(minuteTime){
    number =  number * 60;
  }else if(hourTime){
    number = number *3600;
  }
  return number;
}