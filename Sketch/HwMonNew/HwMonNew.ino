/*
PC HARDWARE MONITOR DISPLAY FOR ARDUINO
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

//Libraries
#include <EEPROM.h>
#include <hd44780.h>
#include <hd44780ioClass/hd44780_pinIO.h> // Arduino pin i/o class header
#include "src\DHTlib\dht.h" //DHT11 Library by Rob Tillaart https://github.com/RobTillaart/ 
#include "src\IRMP\src\irmp.h" //IRremote Library by Rafi Khan https://github.com/z3t0
#include "src\arduino-new-tone\NewTone.h" //NewTone Library by Tim Eckel https://bitbucket.org/teckel12/
#include "src\Bounce2\src\Bounce2.h" //Bounce2 Library by Thomas O Fredericks https://github.com/thomasfredericks

//F() Macro Reimplementation
//Credits: https://forum.arduino.cc/index.php?topic=310410.0
#define FS(a)  (reinterpret_cast<const __FlashStringHelper *>(a))

//Pins
#define IR 2
#define LCD_D7 3
#define LCD_D6 4
#define LCD_D5 5
#define LCD_BL 6
#define LCD_D4 7
#define LCD_E 8
#define LCD_RS 9
#define BUZZER 10
#define LED_DATA 11
#define LED_LATCH 12
#define LED_CLOCK 13
#define FWU_SW A0
#define PWR_BTN A1
#define CFG_BTN A2
#define NEXT_BTN A3
#define OK_BTN A4
#define DHT11 A5


//Hardware Class
class Hardware {
  public:
  Hardware();
  void setBrightness(byte value);
  void updateCPanel(byte idx, byte value);
  void updateCPanel(byte LED0, byte LED1, byte LED2, byte LED3);
  private:
  byte currentBrightness;
  byte cpanel;
};

void Hardware::setBrightness(byte value) {
  currentBrightness = value;
  analogWrite(LCD_BL,value);
}

void Hardware::updateCPanel(byte idx, byte value) {
  switch(value) {
    case 0x00:
      bitWrite(cpanel,(7-(2*idx)),0);
      bitWrite(cpanel,(7-(2*idx)-1),0);
      break;
    case 0x01:
      bitWrite(cpanel,(7-(2*idx)),0);
      bitWrite(cpanel,(7-(2*idx)-1),1);
      break;
    case 0x02:
      bitWrite(cpanel,(7-(2*idx)),1);
      bitWrite(cpanel,(7-(2*idx)-1),0);
      break;
    case 0x03:
      bitWrite(cpanel,(7-(2*idx)),1);
      bitWrite(cpanel,(7-(2*idx)-1),1);
      break;
    default:
      return;
  }
  for (int j = 0; j < 8; j++) 
      {
      // Output low level to latchPin
      digitalWrite(LED_LATCH, LOW);
      // Send serial data to 74HC595
      shiftOut(LED_DATA, LED_CLOCK, LSBFIRST, cpanel);
      // Output high level to latchPin, and 74HC595 will update the data to the parallel output port.
      digitalWrite(LED_LATCH, HIGH);
      }
}

void Hardware::updateCPanel(byte LED0, byte LED1, byte LED2, byte LED3) {
  cpanel = LED3 + (LED2 << 2) + (LED1 << 4) + (LED0 << 6);
  for (int j = 0; j < 8; j++) 
      {
      // Output low level to latchPin
      digitalWrite(LED_LATCH, LOW);
      // Send serial data to 74HC595
      shiftOut(LED_DATA, LED_CLOCK, LSBFIRST, cpanel);
      // Output high level to latchPin, and 74HC595 will update the data to the parallel output port.
      digitalWrite(LED_LATCH, HIGH);
      }
}

Hardware::Hardware() {
  setBrightness(0);
}
//HW management objects
hd44780_pinIO lcd(LCD_RS,LCD_E,LCD_D4,LCD_D5,LCD_D6,LCD_D7);
Hardware hardware;

void setup() {
  pinMode(IR,INPUT);
  pinMode(LCD_D7,OUTPUT);
  pinMode(LCD_D6,OUTPUT);
  pinMode(LCD_D5,OUTPUT);
  pinMode(LCD_BL,OUTPUT);
  pinMode(LCD_D4,OUTPUT);
  pinMode(LCD_E,OUTPUT);
  pinMode(LCD_RS,OUTPUT);
  pinMode(BUZZER, OUTPUT);
  pinMode(LED_DATA,OUTPUT);
  pinMode(LED_LATCH,OUTPUT);
  pinMode(LED_CLOCK,OUTPUT);
  pinMode(FWU_SW, INPUT_PULLUP);;
  pinMode(DHT11,INPUT);
  lcd.begin(20,4);
  hardware.setBrightness(255);
  lcd.print("Hello World!");
  hardware.updateCPanel(0x03,0x03,0x03,0x03);
  delay(2000);
  hardware.updateCPanel(0x02,0x02,0x02,0x02);
  delay(2000);
  hardware.updateCPanel(0x01,0x01,0x01,0x01);
  delay(2000);
  hardware.updateCPanel(0x00,0x00,0x00,0x00);
  delay(2000);
  hardware.updateCPanel(0,0x01);
  delay(1000);
  hardware.updateCPanel(0,0x02);
  delay(1000);
  hardware.updateCPanel(0,0x03);
  delay(1000);
  hardware.updateCPanel(1,0x01);
  delay(1000);
  hardware.updateCPanel(1,0x02);
  delay(1000);
  hardware.updateCPanel(1,0x03);
  delay(1000);
  hardware.updateCPanel(2,0x01);
  delay(1000);
  hardware.updateCPanel(2,0x02);
  delay(1000);
  hardware.updateCPanel(2,0x03);
  delay(1000);
  hardware.updateCPanel(3,0x01);
  delay(1000);
  hardware.updateCPanel(3,0x02);
  delay(1000);
  hardware.updateCPanel(3,0x03);
  delay(1000);
  
  hardware.updateCPanel(0,0x02);
  delay(1000);
  hardware.updateCPanel(1,0x02);
  delay(1000);
  hardware.updateCPanel(2,0x02);
  delay(1000);
  hardware.updateCPanel(3,0x02);
  delay(1000);
  hardware.updateCPanel(0,0x01);
  delay(1000);
  hardware.updateCPanel(1,0x01);
  delay(1000);
  hardware.updateCPanel(2,0x01);
  delay(1000);
  hardware.updateCPanel(3,0x01);
  delay(1000);
  hardware.updateCPanel(0,0x00);
  delay(1000);
  hardware.updateCPanel(1,0x00);
  delay(1000);
  hardware.updateCPanel(2,0x00);
  delay(1000);
  hardware.updateCPanel(3,0x00);
  delay(1000);
}

void loop() {
}
