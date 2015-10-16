#include <OneWire.h>
#include <DallasTemperature.h>
#define ONE_WIRE_BUS 5
#define TEMPERATURE_PRECISION 9

#include <OLED_I2C.h>
OLED  myOLED(SDA, SCL, 8);

#include "DHT.h"
#define DHTPIN 2
#define DHTTYPE DHT22

#include <EEPROM.h> 


OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
DeviceAddress insideThermometer = {0x28, 0xFF, 0xE4, 0x31, 0x69, 0x14, 0x04, 0xD7};
DeviceAddress outsideThermometer = {0x28, 0xFF, 0x89, 0x14, 0x69, 0x14, 0x04, 0xDB};

DHT dht(DHTPIN, DHTTYPE);

extern uint8_t RusFont[];
extern uint8_t SmallFont[];
extern uint8_t MediumNumbers[];


int minT = 18; 
//int maxT = 29; 

int butPin = 5;
int ledPin = 13;

unsigned long currentMillis;

long timeOfWork = 0; //hours
float prevMillis1 = 0;
float prevMillis2 = 0;
float prevMillis3 = 0;

float interval = 25000; //25sec
float intervalHour =3600000; //1 hour
float intervalDay = 86400000; // 1 day


void setup()
{
  myOLED.begin();
  myOLED.setFont(SmallFont);

  sensors.begin();
  sensors.setResolution(insideThermometer, TEMPERATURE_PRECISION);
  sensors.setResolution(outsideThermometer, TEMPERATURE_PRECISION);

  dht.begin();

  pinMode (ledPin, OUTPUT);
  pinMode (butPin, INPUT);

  timeOfWork = EEPROMReadlong(); //load from EEPROM

}


void loop()
{
  currentMillis = millis(); 
  
  if (currentMillis - prevMillis2 >= interval || currentMillis < prevMillis2) //update display 25 sec

  {
    prevMillis2 = currentMillis;
    myOLED.clrScr();
    Display();
  }

  if (currentMillis - prevMillis3 >= intervalDay || currentMillis < prevMillis3) //write to EEPROM ones per day
  {
    prevMillis3 = currentMillis;
    EEPROMWritelong();   
  }

}

void Display()
{
  myOLED.setFont(SmallFont);
  myOLED.printNumI(timeOfWork, 80, 25);

  myOLED.drawRect(0, 0, 127, 63);
  myOLED.drawLine(0, 15, 127, 15);

  sensors.requestTemperatures();
  float t1 = sensors.getTempC(insideThermometer);
  float t2 = sensors.getTempC(outsideThermometer);

  myOLED.print(String(t1, 1), 25, 20);
  myOLED.print(String(t2, 1), 25, 35);
  myOLED.print("T1:", 4, 20);
  myOLED.print("T2:", 4, 35);


  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (isnan(t) || isnan(h))
  {
    myOLED.print("Failed DHT", 4, 4);
  }

  else
  {
    myOLED.print("T:", 20, 4);
    myOLED.print(String(t, 1), 35, 4);

    myOLED.print("H:", 65, 4);
    myOLED.print(String(h, 1), 80, 4);
  }

  myOLED.setFont(RusFont);
  if (t1 < minT && t2 < minT) //Heating...
  {
    myOLED.print("Yfuhtd DRK", 4, 50); // Нагрев ВКЛ
    
    if (currentMillis - prevMillis1 >= intervalHour || currentMillis < prevMillis1)
  {
    prevMillis1 = currentMillis;
    timeOfWork++;
  }
  }
  else
  {
    myOLED.print("Yfuhtd DSRK", 4, 50); //Нагрев ВЫКЛ
  }
  myOLED.update();
}

void EEPROMWritelong()
      {
        int address = 0; 
        long value = timeOfWork;
      //Decomposition from a long to 4 bytes by using bitshift.
      //One = Most significant -> Four = Least significant byte
      byte four = (value & 0xFF);
      byte three = ((value >> 8) & 0xFF);
      byte two = ((value >> 16) & 0xFF);
      byte one = ((value >> 24) & 0xFF);

      //Write the 4 bytes into the eeprom memory.
      EEPROM.write(address, four);
      EEPROM.write(address + 1, three);
      EEPROM.write(address + 2, two);
      EEPROM.write(address + 3, one);
      }

//This function will return a 4 byte (32bit) long from the eeprom
//at the specified address to adress + 3.
long EEPROMReadlong()
      {
      long address = 0;
      //Read the 4 bytes from the eeprom memory.
      long four = EEPROM.read(address);
      long three = EEPROM.read(address + 1);
      long two = EEPROM.read(address + 2);
      long one = EEPROM.read(address + 3);

      //Return the recomposed long by using bitshift.
      return ((four << 0) & 0xFF) + ((three << 8) & 0xFFFF) + ((two << 16) & 0xFFFFFF) + ((one << 24) & 0xFFFFFFFF);
      }

