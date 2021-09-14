#include<SoftwareSerial.h>
#include <DS3232RTC.h>
#include <Time.h> 
#include <TinyWireM.h> 

SoftwareSerial swsri(3,4);
char c = ' ';
bool secondsTime = false;
bool minuteTime = false;
bool hourTime = false;
int timeConfigured = 0;

byte led = 1;

void setup()
{
  pinMode(led,OUTPUT);
  swsri.begin(9600);
  digitalWrite(led,LOW);
}

void loop()
{ 
  char c = swsri.read();
  configureTime(c);
  defineTime();
  turnOnArduino();
}


void configureTime(char c){
  if(c == ':'){
    secondsTime = true;
  }else if(c == ';'){
    minuteTime= true;
  }else if(c == '<'){
    hourTime = true;
  } 
}

void defineTime(){
  if((secondsTime || minuteTime || hourTime) && timeConfigured == 0){
    while (swsri.available () > 0){
      static char input[5];
      static uint8_t i;
      char c = swsri.read ();
      if ( c != '\r' && i < 15 ){
        input[i++] = c;
      }else{
        input[i] = '\0';
        i = 0;
        int number = atoi( input );
        if(number != 0){
          timeConfigured = number; 
          break;
        }
      }
    }
  }
}

void turnOnArduino(){
  time_t t = now();
  bool isTurnOnArduino = false;
  if(secondsTime && second(t)%timeConfigured == 0){
    isTurnOnArduino= true;
  }
  if(minuteTime && minute(t)%timeConfigured == 0){
    isTurnOnArduino= true;
  }
  if(hourTime && hour(t)%timeConfigured == 0){
    isTurnOnArduino= true;
  }
  if(isTurnOnArduino){
    digitalWrite(led,HIGH);
    swsri.print(1);
    turnOffArduino();
  }
}

void turnOffArduino(){
  while(true){
    char c = swsri.read ();
    if(c == '1'){
      digitalWrite(led,LOW);
      break;
    }
  }
}
