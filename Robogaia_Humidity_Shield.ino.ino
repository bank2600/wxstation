#include "Arduino.h"
#include <Wire.h>

#define I2C_ADDR 0x40

// I2C commands
#define RH_READ             0xE5 
#define TEMP_READ           0xE3 
#define POST_RH_TEMP_READ   0xE0 
#define RESET               0xFE 
#define USER1_READ          0xE7 
#define USER1_WRITE         0xE6 

// compound commands
byte SERIAL1_READ[]      ={ 0xFA, 0x0F };
byte SERIAL2_READ[]      ={ 0xFC, 0xC9 };

bool _si_exists = false;

//*********************************************************
void setup() 
//*********************************************************
{
   // initialize serial communication at 9600 bits per second:
   Serial.begin(9600);
  
   //initialize I2C
   Wire.begin();
   Wire.beginTransmission(I2C_ADDR);

}//end func

//*********************************************************
void loop() 
//*********************************************************
{ 
  float temp;
  int humidity;
  
  temp = getCelsiusTemperature() ;
  Serial.print("Temperature Celsius= ");
  Serial.print(temp); 
  
  temp = getFahrenheitTemperature();
  Serial.print(" Temperature Fahrenheit= ");
  Serial.print(temp);
  
  humidity = getHumidityPercent();
  Serial.print(" Humidity %= ");
  Serial.println(humidity);
  
  delay(200); 
  
}//end func

//*********************************************************
float getFahrenheitTemperature() 
//*********************************************************
{
    float c = getCelsiusTemperature();
    return (1.8 * c) + 32.00f;
}//end func

//*********************************************************
float getCelsiusTemperature() 
//*********************************************************
{
    byte tempbytes[2];
    command(TEMP_READ, tempbytes);
    long tempraw = (long)tempbytes[0] << 8 | tempbytes[1];
    return (((175.72f * tempraw)/ 65536.0f) - 46.85f);
}//end func

//*********************************************************
int getCelsiusPostHumidity() 
//*********************************************************
{
    byte tempbytes[2];
    command(POST_RH_TEMP_READ, tempbytes);
    long tempraw = (long)tempbytes[0] << 8 | tempbytes[1];
    return ((175.72f * tempraw) /65536.0f) - 46.85f;
}//end func

//*********************************************************
unsigned int getHumidityPercent() 
//*********************************************************
{
    byte humbytes[2];
    command(RH_READ, humbytes);
    long humraw = (long)humbytes[0] << 8 | humbytes[1];
    return ((125 * humraw) >> 16) - 6;
}//end func

//*********************************************************
unsigned int getHumidityBasisPoints() 
//*********************************************************
{
    byte humbytes[2];
    command(RH_READ, humbytes);
    long humraw = (long)humbytes[0] << 8 | humbytes[1];
    return ((12500 * humraw) >> 16) - 600;
}//end func

//*********************************************************
void command(byte cmd, byte * buf ) 
//*********************************************************
{
    writeReg(&cmd, sizeof cmd);
    readReg(buf, 2);
}//end func

//*********************************************************
void writeReg(byte * reg, int reglen) 
//*********************************************************
{
    Wire.beginTransmission(I2C_ADDR);
    for(int i = 0; i < reglen; i++) {
        reg += i;
        Wire.write(*reg); 
    }
    Wire.endTransmission();
}//end func

//*********************************************************
int readReg(byte * reg, int reglen) 
//*********************************************************
{
    Wire.requestFrom(I2C_ADDR, reglen);
    while(Wire.available() < reglen) {
    }
    for(int i = 0; i < reglen; i++) { 
        reg[i] = Wire.read(); 
    }
    return 1;
}//end func


//*********************************************************
int getSerialBytes(byte * buf) 
//*********************************************************
{
    writeReg(SERIAL1_READ, sizeof SERIAL1_READ);
    readReg(buf, 6);
 
    writeReg(SERIAL2_READ, sizeof SERIAL2_READ);
    readReg(buf + 6, 6);
    
    // could verify crc here and return only the 8 bytes that matter
    return 1;
}//end func

//*********************************************************
int getDeviceId() 
//*********************************************************
{
    byte serial[12];
    getSerialBytes(serial);
    int id = serial[6];
    return id;
}//end func

//*********************************************************
void setHeater(bool on) 
//*********************************************************
{
    byte userbyte;
    if (on) 
    {
        userbyte = 0x3E;
    } 
    else 
    {
        userbyte = 0x3A;
    }
    byte userwrite[] = {USER1_WRITE, userbyte};
    writeReg(userwrite, sizeof userwrite);
}//end func

