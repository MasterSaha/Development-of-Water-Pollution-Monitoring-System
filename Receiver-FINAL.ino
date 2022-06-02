#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x3F,16,2);  // set the LCD address to 0x3F for a 16 chars and 2 line display

RF24 radio(7, 8); // CE, CSN

const byte address[6] = "00001";
const byte address2[6] = "10000";

void setup() {
  Serial.begin(9600);
  radio.begin();
  radio.openReadingPipe(0, address);
  radio.setPALevel(RF24_PA_MIN);
  radio.startListening();

  lcd.init();
  lcd.clear();         
  lcd.backlight();   
  // Print a message on both lines of the LCD.
  lcd.setCursor(0,0);   //Set cursor to character 2 on line 0
  lcd.print("S 10,H 7.2,T 53");
  
  lcd.setCursor(0,1);   //Move cursor to character 2 on line 1
  lcd.print("No GPS Detected");
}

void loop() {
  if (radio.available()) {
    char text[32] = "";
    radio.read(&text, sizeof(text));
    Serial.println(text);
  }
  if (radio.available()) {
    char gpsdata[32] = "";
    radio.read(&gpsdata, sizeof(gpsdata));
    Serial.println(gpsdata);
  }

  
    delay(1000);
  }
