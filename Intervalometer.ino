#include <EEPROM.h>

#define BUFFERSIZE 80
#define TRUE 1
#define FALSE 0

#define STATUSPIN 13
#define IRPIN 12

long Delay = 10000;
long Exposure = 10000;

long TimeClosed = 0;
long EndDelay = 0;

int ExposureAddress = 1;
int DelayAddress = 10;

int CurrentExposure = 0;
int ExposureCount = 0;

char COMMANDEND = 0x0D;
char CommandBuffer[BUFFERSIZE + 1];
int BufferUsed= 0;

#define STOP 1
#define OPEN 2
#define CLOSE 3
#define WAIT 4

int Mode = STOP;

void setup() 
{
  Serial.begin(9600);
  
  Mode = STOP;

  pinMode(STATUSPIN, OUTPUT);
  pinMode(IRPIN, OUTPUT);
  
  digitalWrite(STATUSPIN, LOW);
  digitalWrite(IRPIN, LOW);
  
  EEPROM_read(ExposureAddress,&Exposure);
  EEPROM_read(DelayAddress,&Delay);
  
  Report();
}

void loop() 
{
  static char Response[40];
  
  while (Serial.available())
  {
    if(BufferUsed < BUFFERSIZE)
    {
      Serial.readBytes(&CommandBuffer[BufferUsed], 1);
      Serial.print(CommandBuffer[BufferUsed]);
      
      if (CommandBuffer[BufferUsed] == '\r' )
      {
        CommandBuffer[BufferUsed] = '\0';
        if(strstr(CommandBuffer,"T"))
        {
          CameraSnap(IRPIN);
        }
        else if(strstr(CommandBuffer,"report"))
        {
          Report();
        }
        else if(strstr(CommandBuffer,"exposure"))
        {
          ChangeExposure(&CommandBuffer[9]);
          Report();
        }
        else if(strstr(CommandBuffer,"delay"))
        {
          ChangeDelay(&CommandBuffer[6]);
          Report();
        }
        else if(strstr(CommandBuffer,"count"))
        {
          ChangeCount(&CommandBuffer[5]);
          Report();
        }
        else if(strstr(CommandBuffer,"go"))
        {
          ExposureInit();
        }
        BufferUsed = 0; 
      }
      else
      {
         BufferUsed ++;
      }
    }
    else
    {
      BufferUsed = 0;
    }  
  }
  
  RunExposure();
  
}

void ExposureInit(void)
{
  long TimeNow;
    
  CurrentExposure = 1;
  if(CurrentExposure < ExposureCount)
  {
    TimeNow = millis();
    Mode = OPEN;  
    TimeClosed = TimeNow + Exposure;
    CameraSnap(IRPIN);
    digitalWrite(STATUSPIN,HIGH);
    Serial.print("Exposure "); Serial.print(CurrentExposure);
    Serial.println(" Shutter Open");
  }
}

void RunExposure(void)
{
    long TimeNow;
    
    TimeNow = millis();
    
    if( (Mode == OPEN) && (TimeNow > TimeClosed))
    {
        Mode = CLOSE;
        CameraSnap(IRPIN);
        digitalWrite(STATUSPIN,LOW);
        EndDelay = TimeNow + Delay;
        Serial.print("Exposure "); Serial.print(CurrentExposure);
       Serial.println(" Shutter Closed");
    }
    if( (Mode == CLOSE) && (TimeNow > EndDelay))
    {
        CurrentExposure++;
        if(CurrentExposure <= ExposureCount)
        {
          Mode = OPEN;  
          TimeClosed = TimeNow + Exposure;
          CameraSnap(IRPIN);
          digitalWrite(STATUSPIN,HIGH);
          Serial.print("Exposure "); Serial.print(CurrentExposure);
          Serial.println(" ShutterOpen");
        }
        else
        {
          Mode = STOP;
          Serial.println("Finished");
        }
    }
}

void Report(void)
{
  Serial.print  ("Exposure ");
  Serial.println(Exposure);
  Serial.print  ("Delay ");
  Serial.println(Delay);
  Serial.print  ("Count ");
  Serial.println(ExposureCount);
}

void ChangeExposure(char *command)
{
  long _Exposure;
  
  //Serial.println(command);
  
  _Exposure = atol(command);
  Exposure = _Exposure; 
  EEPROM_write(ExposureAddress,Exposure);
}

void ChangeDelay(char *command)
{
  long _Delay;
  
  _Delay = atol(command);
  Delay = _Delay; 
  EEPROM_write(DelayAddress,Delay);
}

void ChangeCount(char *command)
{
  long Count;
  
  Count = atol(command);
  ExposureCount = Count;
}  

  
int EEPROM_write(int ee, long value)
{
    const byte* p = (const byte*)(const void*)&value;
    unsigned int i;
    for (i = 0; i < sizeof(long); i++)
          EEPROM.write(ee++, *p++);
    return i;
}

int EEPROM_read(int ee, long *value)
{
    byte* p = (byte*)(void*)value;
    unsigned int i;
    for (i = 0; i < sizeof(long); i++)
          *p++ = EEPROM.read(ee++);
    return i;
}  



// This 39kHz loop from http://zovirl.com/2008/11/12/building-a-universal-remote-with-an-arduino
/* Modulate pin at 39 kHz for give number of microseconds */
void on(int pin, int time) 
{
  static const int period = 25;
  // found wait_time by measuring with oscilloscope
  static const int wait_time = 9;  

  for (time = time/period; time > 0; time--) {
    digitalWrite(pin, HIGH);
    delayMicroseconds(wait_time);
    digitalWrite(pin, LOW);
    delayMicroseconds(wait_time);
  }
}


void CameraSnap(int pin)
{
// These Timing are from: http://www.bigmike.it/ircontrol/
on(pin,2000);
//This Delay is broken into 3 lines because the delayMicroseconds() is only accurate to 16383. http://arduino.cc/en/Reference/DelayMicroseconds
delayMicroseconds(7830);
delayMicroseconds(10000);
delayMicroseconds(10000);
on(pin,390);
delayMicroseconds(1580);
on(pin,410);
delayMicroseconds(3580);
on(pin,400);
}


