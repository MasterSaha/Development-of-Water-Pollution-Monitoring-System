/*
  SD card datalogger

  This example shows how to log data from three analog sensors
  to an SD card using the SD library.

  The circuit:
   analog sensors on analog ins 0, 1, and 2
   SD card attached to SPI bus as follows:
 ** MOSI - pin 11/51
 ** MISO - pin 12/50
 ** CLK - pin 13/52
 ** CS - pin 4/53 (for MKRZero SD: SDCARD_SS_PIN)

  created  24 Nov 2010
  modified 9 Apr 2012
  by Tom Igoe

  This example code is in the public domain.

*/

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>
RF24 radio(7, 8); // CE, CSN

const byte address[6] = "00001";
const byte address2[6] = "10000";

/*#include <SD.h>
const int chipSelect = 53;*/

int Cycle;

#include <TinyGPSPlus.h>
#include <SoftwareSerial.h>

// Choose two Arduino pins to use for software serial
int RXPin = 2;
int TXPin = 3;

int GPSBaud = 9600;

// Create a TinyGPS++ object
TinyGPSPlus gps;

// Create a software serial port called "gpsSerial"
SoftwareSerial gpsSerial(RXPin, TXPin);
int sensorPin = A2;  // Turbidity

float calibration = 19.60;   //change this value to calibrate
const int analogInPin = A1;  // pH 
int sensorValue = 0;
unsigned long int avgValue;
float b;
int buf[10],temp;

#define TdsSensorPin A0  // TDS
#define VREF 5.0                    // analog reference voltage(Volt) of the ADC
#define SCOUNT  30                  // sum of sample point
int analogBuffer[SCOUNT];           // store the analog value in the array, read from ADC
int analogBufferTemp[SCOUNT];
int analogBufferIndex = 0,copyIndex = 0;
float averageVoltage = 0,tdsValue = 0,temperature = 25;        ////
int getMedianNum(int bArray[], int iFilterLen)              //Special Function for TDS
{
      int bTab[iFilterLen];
      for (byte i = 0; i<iFilterLen; i++)
      bTab[i] = bArray[i];
      int i, j, bTemp;
      for (j = 0; j < iFilterLen - 1; j++) 
      {
      for (i = 0; i < iFilterLen - j - 1; i++) 
          {
        if (bTab[i] > bTab[i + 1]) 
            {
        bTemp = bTab[i];
            bTab[i] = bTab[i + 1];
        bTab[i + 1] = bTemp;
         }
      }
      }
      if ((iFilterLen & 1) > 0)
    bTemp = bTab[(iFilterLen - 1) / 2];
      else
    bTemp = (bTab[iFilterLen / 2] + bTab[iFilterLen / 2 - 1]) / 2;
      return bTemp;
}

void displayInfo()
{
  if (gps.location.isValid())
  {
    Serial.print("Latitude: ");
    Serial.println(gps.location.lat(), 6);
    Serial.print("Longitude: ");
    Serial.println(gps.location.lng(), 6);
    Serial.print("Altitude: ");
    Serial.println(gps.altitude.meters());
  }
  else
  {
    Serial.println("Location: Not Available");
  }
  
  Serial.print("Date: ");
  if (gps.date.isValid())
  {
    Serial.print(gps.date.month());
    Serial.print("/");
    Serial.print(gps.date.day());
    Serial.print("/");
    Serial.println(gps.date.year());
  }
  else
  {
    Serial.println("Not Available");
  }

  Serial.print("Time: ");
  if (gps.time.isValid())
  {
    if (gps.time.hour() < 10) Serial.print(F("0"));
    Serial.print(gps.time.hour());
    Serial.print(":");
    if (gps.time.minute() < 10) Serial.print(F("0"));
    Serial.print(gps.time.minute());
    Serial.print(":");
    if (gps.time.second() < 10) Serial.print(F("0"));
    Serial.print(gps.time.second());
    Serial.print(".");
    if (gps.time.centisecond() < 10) Serial.print(F("0"));
    Serial.println(gps.time.centisecond());
  }
  else
  {
    Serial.println("Not Available");
  }

  Serial.println();
  Serial.println();

}

void setup()
{ 

  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(TdsSensorPin,INPUT); // TDS
  gpsSerial.begin(GPSBaud);

  int Cycle=0;

  Serial.println("Setup ok");

   radio.begin();
  radio.openWritingPipe(address);
  radio.setPALevel(RF24_PA_MIN);
  radio.stopListening();

  // Open serial communications and wait for port to open:
  //Serial.begin(9600);

/* Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  Serial.print("Initializing SD card...");

  // see if the card is present and can be initialized:
  if (!SD.begin(chipSelect)) {
    Serial.println("Card failed, or not present");
    // don't do anything more:
    while (1);
  }
  Serial.println("card initialized.");*/
}

void loop() {
  // put your main code here, to run repeatedly:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
  Serial.print("Cycle No : ");
  Serial.println(Cycle);
  Cycle=Cycle+1;
  int sensorValue = analogRead(sensorPin);
 // Serial.println(sensorValue);
  int turbidity = map(sensorValue, 0, 640, 100, 0);
  Serial.print("Turbidity Value : ");
  Serial.println(turbidity);
  //delay(100);
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  for(int i=0;i<10;i++)
{
  buf[i]=analogRead(analogInPin);
  delay(30);
  }
  for(int i=0;i<9;i++)
  {
  for(int j=i+1;j<10;j++)
  {
  if(buf[i]>buf[j])
  {
  temp=buf[i];
  buf[i]=buf[j];
  buf[j]=temp;
  }
  }
  }
  avgValue=0;
  for(int i=2;i<8;i++)
  avgValue+=buf[i];
  float pHVol=(float)avgValue*5.0/1024/6;
  float phValue = -5.70 * pHVol + calibration;
  Serial.print("pH Value = ");
  Serial.println(phValue);
  //delay(500);
  /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

  static unsigned long analogSampleTimepoint = millis();
   if(millis()-analogSampleTimepoint > 40U)                          //every 40 milliseconds,read the analog value from the ADC
   {
     analogSampleTimepoint = millis();
     analogBuffer[analogBufferIndex] = analogRead(TdsSensorPin);     //read the analog value and store into the buffer
     analogBufferIndex++;
     if(analogBufferIndex == SCOUNT) 
         analogBufferIndex = 0;
   }   
   static unsigned long printTimepoint = millis();
   if(millis()-printTimepoint > 800U)
   {
      printTimepoint = millis();
      for(copyIndex=0;copyIndex<SCOUNT;copyIndex++)
        analogBufferTemp[copyIndex]= analogBuffer[copyIndex];
      averageVoltage = getMedianNum(analogBufferTemp,SCOUNT) * (float)VREF / 1024.0;     // read the analog value more stable by the median filtering algorithm, and convert to voltage value
      float compensationCoefficient=1.0+0.02*(temperature-25.0);                         //temperature compensation formula: fFinalResult(25^C) = fFinalResult(current)/(1.0+0.02*(fTP-25.0));
      float compensationVolatge=averageVoltage/compensationCoefficient;                  //temperature compensation
      tdsValue=(133.42*compensationVolatge*compensationVolatge*compensationVolatge - 255.86*compensationVolatge*compensationVolatge + 857.39*compensationVolatge)*0.5; //convert voltage value to tds value
      //Serial.print("voltage:");
      //Serial.print(averageVoltage,2);
      //Serial.print("V   ");
      Serial.print("TDS Value : ");
      Serial.print(tdsValue,0);
      Serial.println(" ppm");
      Serial.println("  ");
   }

  String dataString = "";
  // read three sensors and append to the string:
  for (int analogPin = 0; analogPin < 3; analogPin++)
  {
    //int sensor = analogRead(analogPin);
    //dataString += String(sensor);
    if (analogPin == 0)
    {
      dataString += "S ";
      dataString += String(tdsValue);
    }
    if (analogPin == 1)
    {
      dataString += ",H ";
      dataString += String(phValue);
    }
    if (analogPin == 2)
    {
      dataString += ",T ";
      dataString += String(turbidity);
    }
    //dataString += String(sensor);
  }
  
  int str_len = dataString.length() + 1;
  char char_array[str_len];
  dataString.toCharArray(char_array ,str_len);
  radio.write(&char_array, sizeof(char_array));
  
  //Serial.println(dataString);
  //Serial.println(str_len);
  Serial.println("Transmitted Data :");
  Serial.println(char_array);
  Serial.println("  ");


   // read three sensors and append to the string:
  for (int analogPin = 0; analogPin < 3; analogPin++) {
    int sensor = analogRead(analogPin);
    dataString += String(sensor);
    if (analogPin < 2) {
      dataString += ",";
    }
  }

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
 /* File dataFile = SD.open("datalog.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (dataFile) {
    dataFile.println(dataString);
    dataFile.close();
    // print to the serial port too:
    Serial.println(dataString);
  }
  // if the file isn't open, pop up an error:
  else {
    Serial.println("error opening datalog.txt");*/
  
   
   
    // This sketch displays information every time a new sentence is correctly encoded.
  while (gpsSerial.available() > 0)
    if (gps.encode(gpsSerial.read()))
    
    void   displayInfo();

  // If 5000 milliseconds pass and there are no characters coming in
  // over the software serial port, show a "No GPS detected" error
  if (millis() > 5000 && gps.charsProcessed() < 10)
  {
    const char gpsdata[] = "No GPS detected";
    radio.write(&gpsdata, sizeof(gpsdata));
    Serial.println("Transmitted GPS Data : ");
    Serial.println("No GPS detected");    
    Serial.println(" ");

   // while(true);
   }
////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    Serial.println(" ");
    Serial.println(" ");
    Serial.println(" ");
    delay(1000);
   }
