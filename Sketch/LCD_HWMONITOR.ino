/*
HARDWARE MONITOR EXTERNAL Screen
POWERED BY ARDUINO
"INSERT GPL"
CREDITS
*/
//************************************ Init *****************************************//
//Libs
#include <LiquidCrystal.h>
#include <dht.h>
#include <IRremote.h>
#include <EEPROM.h>
#include <NewTone.h>
#include <Bounce2.h>

//Define Pins
const int PROGMEM IR_DIODE = 2;
const int PROGMEM LCD_D7 = 3;
const int PROGMEM LCD_D6 = 4;
const int PROGMEM LCD_D5 = 5;
const int PROGMEM LCD_BL = 6;
const int PROGMEM LCD_D4 = 7;
const int PROGMEM LCD_E = 8;
const int PROGMEM LCD_RS = 9;
const int PROGMEM BUZZER = 10;
const int PROGMEM LED_DATAPIN = 11;
const int PROGMEM LED_LATCHPIN = 12;
const int PROGMEM LED_CLOCKPIN = 13;
const int PROGMEM FWU_PIN = A0;
const int PROGMEM PWR_PHYS = A1;
const int PROGMEM CFG_PHYS = A2;
const int PROGMEM OK_PHYS = A3;
const int PROGMEM FORWARDS_PHYS = A4;
const int PROGMEM TEMP_HUMIDITY = A5;

//LCD Screen Init
LiquidCrystal lcd(LCD_RS,LCD_E,LCD_D4,LCD_D5,LCD_D6,LCD_D7); //4-bit Mode

//DHT11 Init
dht DHT;

//IR Receiver Init
IRrecv IRRECEIVE(IR_DIODE);
decode_results IR_RESULT;

//IR Remote buttons
volatile bool IR_on_off;
volatile bool IR_menu;
volatile bool IR_test;
volatile bool IR_plus;
volatile bool IR_back;
volatile bool IR_backwards;
volatile bool IR_forwards;
volatile bool IR_play;
volatile bool IR_minus;
volatile bool IR_clear;
volatile bool IR_0;
volatile bool IR_1;
volatile bool IR_2;
volatile bool IR_3;
volatile bool IR_4;
volatile bool IR_5;
volatile bool IR_6;
volatile bool IR_7;
volatile bool IR_8;
volatile bool IR_9;
volatile bool IR_ACTIVE;

//Config Variables
int BL_BRIGHTNESS; //Brightness value (0-250)
int OLD_BL_BRIGHTNESS; //Brightness Rollback if settings are not applied
bool BUZZER_CFG = true; //BUZZER Setting Read from EEPROM
bool BUZZER_ON; //Enable / Disable BUZZER

//STATUS LEDS (0x00 = OFF, 0x01 = RED, 0x02 = GREEN, 0x03 = YELLOW)
byte STATUS_LED;
byte CPU_LED;
byte GPU_LED;
byte RAM_LED;

//Monitoring Variables
uint8_t CPU_TEMP = 0;
int CPU_FAN = 0;
int CPU_CLK = 0;
uint8_t CPU_USAGE = 0;
float CPU_VCORE = 0;
uint8_t GPU_TEMP = 0;
int GPU_CLK = 0;
int GPU_FAN = 0;
int GPU_FPS = 0;
float GPU_VCORE = 0;
unsigned long RAM_USED = 0;
unsigned long RAM_FREE = 0;
uint8_t ROOM_TEMP = 0;
uint8_t ROOM_HUMIDITY = 0;

//Selected Mode and Page tracking
uint8_t MODE;
uint8_t scroll_counter = 1;
uint8_t scroll_delay = 0;
unsigned long current_millis; //To wait without using delay()

//DHT11 Reading Delay
uint8_t DHT_Counter;

//Serial Monitoring
const static byte PROGMEM numChars = 255;
char receivedChars[numChars];
boolean newData = false;
unsigned long timeoutcounter = 0;

//LCD Lines Arrays
char line0[17];
char line1[17];
char line2[17];
char line3[17];

//Button Variables
//Using Bounce2 Library to deal with button bounce
Bounce debouncerPWR = Bounce();
Bounce debouncerCFG = Bounce();
Bounce debouncerOK = Bounce();
Bounce debouncerFORWARDS = Bounce();
bool TEST_BUTTON = false;
bool OK_BUTTON = false;
bool FORWARDS_BUTTON = false;
uint8_t selected_item;
bool CFG_USING_BUTTONS;

//Version
const char version[4] = "2.2";

//Logo Chars Normal Mode
const byte line0_0[][8] = {B00000,B00000,B11111,B10000,B10010,B10010,B10010,B10000};
const byte line0_1[][8] = {B00000,B00000,B11111,B00001,B01001,B01001,B01001,B00001};
const byte line0_1_ALT [][8] = {B00000,B00000,B11111,B00001,B00001,B00001,B00001,B00001};
const byte line0_2[][8] = {B00000,B00000,B00011,B00011,B00011,B00011,B11111,B11111};
const byte line0_3[][8] = {B00000,B00000,B11000,B11000,B11000,B11000,B11111,B11111};
const byte line1_0[][8] = {B10000,B10000,B10100,B10011,B10000,B11111,B00000,B00000};
const byte line1_1[][8] = {B00001,B00001,B00101,B11001,B00001,B11111,B00000,B00000};
const byte line1_2[][8] = {B11111,B11111,B00011,B00011,B00011,B00011,B00000,B00000};
const byte line1_3[][8] = {B11111,B11111,B11000,B11000,B11000,B11000,B00000,B00000};

//Logo Chars FWU
const byte line0_0_FWU[][8] = {B00000,B00001,B00001,B00001,B00001,B01001,B10101,B01000};
const byte line0_1_FWU[][8] = {B11111,B00000,B01110,B01000,B01100,B01000,B00000,B11111};
const byte line0_2_FWU[][8] = {B11111,B00000,B10001,B10001,B10101,B01010,B00000,B11111};
const byte line0_3_FWU[][8] = {B11111,B00000,B10001,B10001,B10001,B01110,B00000,B11111};
const byte line0_4_FWU[][8] = {B00000,B10000,B10000,B10000,B10000,B10000,B10000,B00000};
const byte line1_0_FWU[][8] = {B11111,B10000,B10010,B10010,B10010,B10000,B10000,B10000};
const byte line1_1_FWU[][8] = {B11111,B00001,B00101,B00101,B00101,B00001,B00001,B00001};

//Setup
void setup()
{
//Pins I/O
pinMode(IR_DIODE,INPUT);
pinMode(LCD_D7,OUTPUT);
pinMode(LCD_D6,OUTPUT);
pinMode(LCD_D5,OUTPUT);
pinMode(LCD_BL,OUTPUT);
pinMode(LCD_D4,OUTPUT);
pinMode(LCD_E,OUTPUT);
pinMode(LCD_RS,OUTPUT);
pinMode(BUZZER, OUTPUT);
pinMode(LED_DATAPIN,OUTPUT);
pinMode(LED_LATCHPIN,OUTPUT);
pinMode(LED_CLOCKPIN,OUTPUT);
pinMode(FWU_PIN, INPUT_PULLUP);
debouncerPWR.attach(PWR_PHYS,INPUT_PULLUP);
debouncerCFG.attach(CFG_PHYS,INPUT_PULLUP);
debouncerOK.attach(OK_PHYS,INPUT_PULLUP);
debouncerFORWARDS.attach(FORWARDS_PHYS,INPUT_PULLUP);
debouncerPWR.interval(25);
debouncerCFG.interval(25);
debouncerOK.interval(25);
debouncerFORWARDS.interval(25);
pinMode(TEMP_HUMIDITY,INPUT);
IRRECEIVE.enableIRIn(); //Enable IR Reception
attachInterrupt(digitalPinToInterrupt(IR_DIODE), check_IR, CHANGE);//IR Interrupt
lcd.begin(16,2); //LCD Start
}

//*********************************************************************************************************

//Loop
void loop() {
    standby();
    welcome();
    wait_serial();
    monitor();
}

//Main Functions
void standby() {
    //Load Settings from EEPROM
    EEPROM_READ();
    //Disable BUZZER
    BUZZER_ON = false;
    //Init Welcome Logo Chars
    lcd.createChar(0,(uint8_t *)line0_0);
    lcd.createChar(1,(uint8_t *)line0_1);
    lcd.createChar(2,(uint8_t *)line0_2);
    lcd.createChar(3,(uint8_t *)line0_3);
    lcd.createChar(4,(uint8_t *)line1_0);
    lcd.createChar(5,(uint8_t *)line1_1);
    lcd.createChar(6,(uint8_t *)line1_2);
    lcd.createChar(7,(uint8_t *)line1_3);
    //Reset Power Buttons
    IR_on_off = false;
    //Serial OFF
    Serial.end();
    //LCD Clear and shutdown
    lcd.clear();
    analogWrite(LCD_BL,0);
    //STATUS_LED = RED, CPU and GPU LEDS OFF
    STATUS_LED = 0x01;
    CPU_LED = 0x00;
    GPU_LED = 0x00;
    RAM_LED = 0x00;
    update_cpanel();
    //Wait until ON/OFF Button is pushed
    while(!IR_on_off) {check_BUTTONS();}
    BUZZER_ON = true;
    OK_tone();
    return;   
}

void welcome() {
    //LCD Startup
    analogWrite(LCD_BL, BL_BRIGHTNESS);
    //Welcome Message
    lcd.clear();
    lcd.setCursor(6,0);
    lcd.write(byte(0));
    lcd.write(byte(1));
    lcd.write(byte(2));
    lcd.write(byte(3));
    lcd.setCursor(6,1);
    lcd.write(byte(4));
    lcd.write(byte(5));
    lcd.write(byte(6));
    lcd.write(byte(7));
    lcd.setCursor(13,1);
    lcd.print(version);
    //Show Splash for 3 seconds
    //Leds GREEN Progressively
    STATUS_LED = 0x02;
    update_cpanel();
    current_millis = millis();
    while(millis() - current_millis <= 1000) {check_on_off();}
    current_millis = millis();
    lcd.createChar(1,(uint8_t *)line0_1_ALT);
    CPU_LED = 0x02;
    update_cpanel();
    while(millis() - current_millis <= 350) {check_on_off();}
    lcd.createChar(1,(uint8_t *)line0_1);
    current_millis = millis();
    while(millis() - current_millis <= 650) {check_on_off();}
    GPU_LED = 0x02;
    update_cpanel();
    current_millis = millis();
    while(millis() - current_millis <= 1000) {check_on_off();}
    RAM_LED = 0x02;
    update_cpanel();
    current_millis = millis();
    while(millis() - current_millis <= 1000) {check_on_off();}
    lcd.clear();
    return;
}

void wait_serial() {
    Serial.begin(115200); //Serial Start
    //STATUS_LED = YELLOW OTHERS = OFF
    CPU_LED = 0x00;
    GPU_LED = 0x00;
    RAM_LED = 0x00;
    STATUS_LED = 0x03;
    update_cpanel();
    //Print Waiting message
    lcd.clear();
    lcd.setCursor(2,0);
    lcd.print(F("WAITING FOR"));
    lcd.setCursor(5,1);
    lcd.print(F("HELPER"));
    current_millis = millis();
    while(Serial.available() == 0){
        //STATUS LED = YELLOW, BLINKS EVERY SECONDç
        if(millis() - current_millis >= 1000) {
            switch (STATUS_LED) {
                case 0x00:
                STATUS_LED = 0x03;
                update_cpanel();
                current_millis = millis();
                break;

                case 0x03:
                STATUS_LED = 0x00;
                update_cpanel();
                current_millis = millis();
                break;
            }
            Serial.println(F("<WAITING FOR HELPER>"));
        }
    check_on_off();
    }
    //STATUS LED = GREEN
    STATUS_LED = 0x02;
    update_cpanel();
    lcd.clear();
    lcd.setCursor(3,0);
    //Desync FIX
    lcd.print(F("CONNECTED"));
    delay(500);
    //Reset Buttons and Delays
    OK_BUTTON = false;
    FORWARDS_BUTTON = false;
    scroll_counter = 1;
    scroll_delay = 0;
    timeoutcounter = 0;
    return;
}

void monitor() {
    while(true) {
        //Clear IR button values
        IR_test = false;
        TEST_BUTTON = false;
        wait_to_refresh();
        //Enter selected mode
        if(MODE == 1) {
            lcd.setCursor(0,0);
            sprintf(line0, "CPU:%-3i" "C" "   %%:%-3i", CPU_TEMP,CPU_USAGE);
            lcd.print(line0);
            lcd.setCursor(0,1);
            sprintf(line1, "GPU:%-3i" "C" "  %-3iFPS", GPU_TEMP,GPU_FPS);
            lcd.print(line1);
        }
        else if (MODE == 2 || MODE == 3) {
            switch(scroll_counter) {
            //CPU STATS
            case 1:
                lcd.setCursor(0,0);
                sprintf(line0, "CPU:%-3i" "C" " SP:%-4i", CPU_TEMP, CPU_FAN);
                lcd.print(line0);
                lcd.setCursor(0,1);
                sprintf(line1, "CLK:%-4iM" " %%:%-3i", CPU_CLK,CPU_USAGE);
                lcd.print(line1);
                break;
            //GPU STATS
            case 2:
                lcd.setCursor(0,0);
                sprintf(line0, "GPU:%-3i" "C" " SP:%-4i", GPU_TEMP, GPU_FAN);
                lcd.print(line0);
                lcd.setCursor(0,1);
                sprintf(line1, "CLK:%-4iM" " %-3iFPS", GPU_CLK,GPU_FPS);
                lcd.print(line1);
                break;
            //CPU,GPU VCORE Performance
            case 3:
                //VCORE Float reading to String
                char GPU_VCORE_STR[5];
                char CPU_VCORE_STR[5];
                dtostrf(GPU_VCORE,4, 2, GPU_VCORE_STR);
                dtostrf(CPU_VCORE,4, 2, CPU_VCORE_STR);
                lcd.setCursor(0,0);
                sprintf(line0, "CPU:%-4sV" "  %%:%-3i", CPU_VCORE_STR, CPU_USAGE);
                lcd.print(line0);
                sprintf(line1, "GPU:%-4sV" " %-3iFPS", GPU_VCORE_STR, GPU_FPS);
                lcd.setCursor(0,1);
                lcd.print(line1);
                break;
            //RAM STATS
            case 4:
                lcd.setCursor(0,0);
                sprintf(line0, "USED RAM:" "%-5u" "MB", RAM_USED);
                lcd.print(line0);
                lcd.setCursor(0,1);
                sprintf(line1, "FREE RAM:" "%-5u" "MB", RAM_FREE);
                lcd.print(line1);
                break;
            //Room Temp/Humidity
            case 5:
                lcd.setCursor(0,0);
                sprintf(line0, "RoomTemp:" "%-3i" "\xDF" "C", int(DHT.temperature));
                lcd.print(line0);
                lcd.setCursor(0,1);
                sprintf(line1, "Humidity:" "%-3i" "%%", int(DHT.humidity));
                lcd.print(line1);
                break;
            }
        }
    }
}

//Settings Menu
void config() {
    //Disable CPU and GPU LEDS
    CPU_LED = 0x00;
    GPU_LED = 0x00;
    RAM_LED = 0x00;
    update_cpanel();
    //Show Settings splash for 3 seconds
    lcd.clear();
    lcd.setCursor(4,0);
    lcd.print(F("SETTINGS"));
    current_millis = millis();
    while(millis() - current_millis <= 2000) {check_on_off();}
    selected_item = 1;
    if(IR_test) {CFG_USING_BUTTONS = false; selected_item = 1; config_nosplash(true);}
    else if(TEST_BUTTON) {CFG_USING_BUTTONS = true; selected_item = 1; config_nosplash(true);}
}

void config_nosplash(bool clear) {
    while(true) {
        if(clear) {lcd.clear();}
        //Selection menu
        if (!CFG_USING_BUTTONS) {
            lcd.print(F("(1) MONITOR MODE"));
            lcd.setCursor(0,1);
            lcd.print(F("(2) BRIGHTNESS"));
            }
        else if(CFG_USING_BUTTONS) {
            switch(selected_item) {
            case 1:
            lcd.print(F("(*) MONITOR MODE"));
            lcd.setCursor(0,1);
            lcd.print(F("( ) BRIGHTNESS"));
            break;
            case 2:
            lcd.print(F("( ) MONITOR MODE"));
            lcd.setCursor(0,1);
            lcd.print(F("(*) BRIGHTNESS"));
            break;
            }
        }
        //Clear IR and physical buttons values
        IR_1 = false;
        IR_2 = false;
        IR_back = false;
        TEST_BUTTON = false;
        OK_BUTTON = false;
        FORWARDS_BUTTON = false;
        IR_ACTIVE = false;
        while(!IR_ACTIVE && !TEST_BUTTON && !OK_BUTTON && !FORWARDS_BUTTON) {check_on_off();}
        if(IR_1 && !CFG_USING_BUTTONS) {selected_item = 1; modes_select(true);break;}
        else if(IR_2 && !CFG_USING_BUTTONS) {brightness_select(true);break;}
        else if(IR_back && !CFG_USING_BUTTONS) {lcd.clear();break;}
        else if(FORWARDS_BUTTON && CFG_USING_BUTTONS) {selected_item++;if(selected_item > 2) {selected_item = 1;}}
        else if(OK_BUTTON && CFG_USING_BUTTONS && selected_item == 1) {selected_item = 1; modes_select(true);break;}
        else if(OK_BUTTON && CFG_USING_BUTTONS && selected_item == 2) {brightness_select(true);break;}
        else if(TEST_BUTTON && CFG_USING_BUTTONS) {lcd.clear();break;}
        else if(IR_ACTIVE && CFG_USING_BUTTONS) {CFG_USING_BUTTONS = false; clear = true;}
        else if(!IR_ACTIVE && !CFG_USING_BUTTONS) {CFG_USING_BUTTONS = true; selected_item = 1; clear = true;}
    }
        //Save values to EEPROM
        EEPROM_UPDATE();
        //Reset Variables
        IR_forwards = false;
        IR_backwards = false;
        IR_back = false;
        OK_BUTTON = false;
        FORWARDS_BUTTON = false;
        //Reset Scroll counter
        scroll_counter = 1;
        scroll_delay = 0;
        return;
}

//Mode Selector
void modes_select(bool clear) {
    while(true) {
        if(clear) {lcd.clear();clear = false;}
        if(IR_ACTIVE) {
            lcd.print(F("(1)  (2)   (3)"));
            lcd.setCursor(0,1);
            lcd.print(F("EASY AUTO MANUAL"));
        }
        else if(!IR_ACTIVE) {
            switch(selected_item) {
            case 1:
                lcd.setCursor(0,0);
                lcd.print(F("(*)  ( )   ( )"));
                lcd.setCursor(0,1);
                lcd.print(F("EASY AUTO MANUAL"));
                break;
            case 2:
                lcd.setCursor(0,0);
                lcd.print(F("( )  (*)   ( )"));
                lcd.setCursor(0,1);
                lcd.print(F("EASY AUTO MANUAL"));
                break;
            case 3:
                lcd.setCursor(0,0);
                lcd.print(F("( )  ( )   (*)"));
                lcd.setCursor(0,1);
                lcd.print(F("EASY AUTO MANUAL"));
                break;
            }
        }
        //Clean IR buttons values, show options and wait until one of each is pushed.
        IR_1 = false;
        IR_2 = false;
        IR_3 = false;
        IR_back = false;
        TEST_BUTTON = false;
        OK_BUTTON = false;
        FORWARDS_BUTTON = false;
        IR_ACTIVE = false;
        while(!IR_ACTIVE && !IR_back && !TEST_BUTTON && !OK_BUTTON && !FORWARDS_BUTTON) {check_on_off();}
        if(IR_1 && !CFG_USING_BUTTONS) {MODE = 1;break;}
        else if(IR_2 && !CFG_USING_BUTTONS) {MODE = 2;break;}
        else if(IR_3 && !CFG_USING_BUTTONS) {MODE = 3;break;}
        else if(IR_back && !CFG_USING_BUTTONS) {selected_item = 1; config_nosplash(true);}
        else if(FORWARDS_BUTTON && CFG_USING_BUTTONS) {selected_item++;if(selected_item > 3) {selected_item = 1;} clear = false;}
        else if(OK_BUTTON && CFG_USING_BUTTONS && selected_item == 1) {MODE = 1;break;}
        else if(OK_BUTTON && CFG_USING_BUTTONS && selected_item == 2) {MODE = 2;break;}
        else if(OK_BUTTON && CFG_USING_BUTTONS && selected_item == 3) {MODE = 3;break;}
        else if(TEST_BUTTON && CFG_USING_BUTTONS) {selected_item = 1; config_nosplash(true);}
        else if(IR_ACTIVE && CFG_USING_BUTTONS) {CFG_USING_BUTTONS = false; clear = true;}
        else if(!IR_ACTIVE && !CFG_USING_BUTTONS) {CFG_USING_BUTTONS = true; selected_item = 1; clear = true;}
    }
    return;
}

//BL CPANEL
void brightness_select(bool clear) {
    while(true) {
        if(clear) {lcd.clear();}
        //BL Level (1-10) Scale
        int current_bl = BL_BRIGHTNESS / 25;
        //Dynamic Interface Drawing
        lcd.setCursor(3,0);
        lcd.print(F("BRIGHTNESS"));
        lcd.setCursor(1,1);
        lcd.print(F("-"));
        lcd.setCursor(3,1);
        while(current_bl != 0) {
            lcd.print(char(255));
            current_bl--;
        }
        current_bl = BL_BRIGHTNESS / 25;
        lcd.setCursor(current_bl + 3,1);
        while(current_bl != 0) {
            lcd.print(F(" "));
            current_bl--;
        }
        lcd.setCursor(14,1);
        lcd.print(F("+"));
        current_bl = BL_BRIGHTNESS / 25;
        //Reset IR Values until and wait until a button is pushed
        IR_play = false;
        IR_forwards = false;
        IR_backwards = false;
        IR_back = false;
        TEST_BUTTON = false;
        OK_BUTTON = false;
        FORWARDS_BUTTON = false;
        IR_ACTIVE = false;
        while(!IR_ACTIVE && !TEST_BUTTON && !OK_BUTTON && !FORWARDS_BUTTON) {check_on_off();}
        if(FORWARDS_BUTTON && current_bl < 10) {BL_BRIGHTNESS += 25;analogWrite(LCD_BL,BL_BRIGHTNESS);}
        else if(FORWARDS_BUTTON && current_bl == 10) {BL_BRIGHTNESS = 25;analogWrite(LCD_BL,BL_BRIGHTNESS); clear = true;}
        else if(OK_BUTTON){return;}
        else if(TEST_BUTTON) {analogWrite(LCD_BL,OLD_BL_BRIGHTNESS); BL_BRIGHTNESS = OLD_BL_BRIGHTNESS;selected_item = 1;CFG_USING_BUTTONS = true; config_nosplash(true);}
        else if(IR_forwards && current_bl < 10) {BL_BRIGHTNESS += 25;analogWrite(LCD_BL,BL_BRIGHTNESS);}
        else if(IR_backwards && current_bl > 1) {BL_BRIGHTNESS -= 25;analogWrite(LCD_BL,BL_BRIGHTNESS);}
        else if(IR_play){break;}
        else if(IR_back) {analogWrite(LCD_BL,OLD_BL_BRIGHTNESS); BL_BRIGHTNESS = OLD_BL_BRIGHTNESS; selected_item = 1;CFG_USING_BUTTONS = false; config_nosplash(true);}
        else {}
    }
    return;
}
//******************************************************************************************************************

//*******************************************AUX FUNCTIONS^**********************************************************
//IR codes receiver
void check_IR() {
    if(IRRECEIVE.decode(&IR_RESULT) == false) {} //No button pressed 
    else {
    switch(IR_RESULT.value) {
        //BUTTON 0
        case 0xFF6897: IR_0 = true;OK_tone(); IR_ACTIVE = true; break;
        //BUTTON 1
        case 0xFF30CF: IR_1 = true;OK_tone(); IR_ACTIVE = true;break;
        //BUTTON 2 
        case 0xFF18E7: IR_2 = true;OK_tone(); IR_ACTIVE = true;break;
        //BUTTON 3
        case 0xFF7A85: IR_3 = true;OK_tone(); IR_ACTIVE = true;break;
        //BUTTON 4
        case 0xFF10EF: IR_4 = true;OK_tone(); IR_ACTIVE = true;break;
        //BUTTON 5
        case 0xFF38C7: IR_5 = true;OK_tone(); IR_ACTIVE = true;break;
        //BUTTON 6
        case 0xFF5AA5: IR_6 = true;OK_tone(); IR_ACTIVE = true;break;
        //BUTTON 7
        case 0xFF42BD: IR_7 = true;OK_tone(); IR_ACTIVE = true;break;
        //BUTTON 8
        case 0xFF4AB5: IR_8 = true;OK_tone(); IR_ACTIVE = true;break;
        //BUTTON 9
        case 0xFF52AD: IR_9 = true;OK_tone(); IR_ACTIVE = true;break;
        //PWR BUTTON
        case 0xFFA25D: IR_on_off = !IR_on_off;OK_tone(); IR_ACTIVE = true;break;
        //MENU BUTTON
        case 0xFFE21D: IR_menu = true;OK_tone(); IR_ACTIVE = true;break;
        //TEST BUTTON
        case 0xFF22DD: IR_test = true;OK_tone(); IR_ACTIVE = true;break;
        //RETURN BUTTON 
        case 0xFFC23D: IR_back = true;OK_tone(); IR_ACTIVE = true;break;
        //PLUS BUTTON 
        case 0xFF02FD: IR_plus = true;OK_tone(); IR_ACTIVE = true;break;
        //MINUS BUTTON 
        case 0xFF9867: IR_minus = true;OK_tone(); IR_ACTIVE = true;break;
        //NEXT BUTTON 
        case 0xFF906F: IR_forwards = true;OK_tone(); IR_ACTIVE = true;break;
        //BACK BUTTON
        case 0xFFE01F: IR_backwards = true;OK_tone(); IR_ACTIVE = true;break;
        //PLAY BUTTON 
        case 0xFFA857: IR_play = !IR_play;OK_tone(); IR_ACTIVE = true;break;
        //CLEAR BUTTON
        case 0xFFB04F: IR_clear = true;OK_tone(); IR_ACTIVE = true;break;
    }
    //Clear IR Receive variable value (Avoid infinite loop)
    IR_RESULT.value = 0x000000;
    IRRECEIVE.resume(); //resume reading
    return; 
    }
}

//Refresh Delay and Data Reception from PC
void wait_to_refresh() {
    //Receives Data from PC
    get_serial();
    //Wait 500ms until refresh
    current_millis = millis();
    while (millis() - current_millis <= 500) {check_on_off();}
    //If settings button is pushed, go to settings
    if(IR_test || TEST_BUTTON) {config();}
    //Pause if IR Play is pushed (MODE 2)
    else if(MODE == 2 && (!IR_play && !OK_BUTTON)) {
        scroll_delay++;
        //Change Page every 2s
        if(scroll_delay == 4) {scroll_counter++;scroll_delay = 0;lcd.clear();}
    }

    //Manual Control (MODE 3)
    else if(MODE == 3 && (IR_backwards || IR_forwards || FORWARDS_BUTTON)) {
        //Change Page if Forwards or Backwards buttons are pushed.
        if (IR_backwards) {scroll_counter--;IR_backwards = false;lcd.clear();}
        if (IR_forwards || FORWARDS_BUTTON) {scroll_counter++;IR_forwards = false; FORWARDS_BUTTON = false;lcd.clear();}
    }
    //Back to Start/End
    if(scroll_counter == 6) {scroll_counter = 1;lcd.clear();}
    else if(scroll_counter == 0) {scroll_counter = 5;lcd.clear();}
    //Get sensors data
    getsensors();
    //LED Updating
    if(CPU_TEMP < 60) {CPU_LED = 0x02;}
    else if((CPU_TEMP >= 60) && (CPU_TEMP < 80)) {CPU_LED = 0x03;}
    else if(CPU_TEMP >= 80) {CPU_LED = 0x01;}
    if(GPU_TEMP < 60) {GPU_LED = 0x02;}
    else if((GPU_TEMP >= 60) && (GPU_TEMP < 80)) {GPU_LED = 0x03;}
    else if(GPU_TEMP >= 80) {GPU_LED = 0x01;}
    if(RAM_FREE >= 2000) {RAM_LED = 0x02;}
    else if((RAM_FREE >= 1000) && (RAM_FREE < 1999)) {RAM_LED = 0x03;}
    else if(RAM_FREE < 999) {RAM_LED = 0x01;}
    update_cpanel();
    //Increase DHT Delay Counter
    DHT_Counter++;
    return;
}

//Reads Sensors and Parse Downloaded Data
void getsensors() {
    //Read DHT11 every 1.5s and judge the state according to the return value
    if(DHT_Counter == 3) {int chk = DHT.read11(TEMP_HUMIDITY); DHT_Counter = 0;}
    // Decode Serial Data Array
    char *array_table[20];
    int i = 0;
    //Create an Array separating received data
    //OPTIMIZE THIS PLZ :D
    array_table[i] = strtok(receivedChars,",");
    while(array_table[i] != NULL) {
        array_table[++i] = strtok(NULL,",");
    }
    for(int i = 0; array_table[i] != NULL; i +=2) {
        if(String(array_table[i]) == (F("UR"))) {RAM_USED = String(array_table[i+1]).toInt();}
        else if(String(array_table[i]) == (F("FR"))) {RAM_FREE = String(array_table[i+1]).toInt();}
        else if(String(array_table[i]) == (F("CC"))) {CPU_CLK = String(array_table[i+1]).toInt();}
        else if(String(array_table[i]) == (F("CU"))) {CPU_USAGE = String(array_table[i+1]).toInt();}
        else if(String(array_table[i]) == (F("CT"))) {CPU_TEMP = String(array_table[i+1]).toInt();}
        else if(String(array_table[i]) == (F("CV"))) {CPU_VCORE = String(array_table[i+1]).toFloat();}
        else if(String(array_table[i]) == (F("GV"))) {GPU_VCORE = String(array_table[i+1]).toFloat();}
        else if(String(array_table[i]) == (F("GT"))) {GPU_TEMP = String(array_table[i+1]).toInt();}
        else if(String(array_table[i]) == (F("GC"))) {GPU_CLK = String(array_table[i+1]).toInt();}
        else if(String(array_table[i]) == (F("FP"))) {GPU_FPS = String(array_table[i+1]).toInt();}
    }
    return;
}

//Check if Power button is pressed, ON -> OFF (RESET)
void check_on_off() {
    if(IR_on_off == false) {OK_tone();delay(100);soft_Reset();}
    check_BUTTONS();
    return;
}
//Checks if byttons are pressed.
void check_BUTTONS() {
    debouncerPWR.update();
    debouncerCFG.update();
    debouncerOK.update();
    debouncerFORWARDS.update();
    //Down Flank Detection
    if(digitalRead(FWU_PIN) == LOW) {lcd.clear();FWU_MODE();}
    if(debouncerPWR.fell()) {IR_ACTIVE = false; IR_on_off = !IR_on_off;}
    if(debouncerCFG.fell()) {IR_ACTIVE = false; OK_tone();TEST_BUTTON = true;}
    if(debouncerOK.fell()) {IR_ACTIVE = false; OK_tone();OK_BUTTON = !OK_BUTTON;}
    if(debouncerFORWARDS.fell()) {IR_ACTIVE = false; OK_tone();FORWARDS_BUTTON = true;}
    }

//FIRMWARE UPDATE MODE
void FWU_MODE() {
    lcd.createChar(0,(uint8_t *)line0_0_FWU);
    lcd.createChar(1,(uint8_t *)line0_1_FWU);
    lcd.createChar(2,(uint8_t *)line0_2_FWU);
    lcd.createChar(3,(uint8_t *)line0_3_FWU);
    lcd.createChar(4,(uint8_t *)line0_4_FWU);
    lcd.createChar(5,(uint8_t *)line1_0_FWU);
    lcd.createChar(6,(uint8_t *)line1_1_FWU);
    //LCD Startup, load brightness value stored in EEPROM
    analogWrite(LCD_BL, 250);
     //STATUS_LED = YELLOW
    STATUS_LED = 0x03;
    CPU_LED = 0x03;
    GPU_LED = 0x03;
    RAM_LED = 0x03;
    update_cpanel();
    //Welcome Message
    lcd.setCursor(0,0);
    lcd.print(F("FIRMWARE"));
    lcd.setCursor(1,1);
    lcd.print(F("UPDATE"));
    lcd.setCursor(9,1);
    lcd.write(byte(5));
    lcd.write(byte(6));
    lcd.setCursor(11,0);
    lcd.write(byte(0));
    lcd.write(byte(1));
    lcd.write(byte(2));
    lcd.write(byte(3));
    lcd.write(byte(4));
    lcd.setCursor(13,1);
    lcd.print(version);
    current_millis = millis();
    while(true) {
        //STATUS LED = YELLOW, BLINKS EVERY SECOND
        if(millis() - current_millis >= 1000) {
            if((digitalRead(FWU_PIN) == HIGH)) {lcd.clear(); lcd.setCursor(5,0); lcd.print(F("REBOOT"));delay(1000);soft_Reset();}
            switch (STATUS_LED) {
                case 0x00:
                STATUS_LED = 0x03;
                CPU_LED = 0x03;
                GPU_LED = 0x03;
                RAM_LED = 0x03;
                update_cpanel();
                NewTone(BUZZER,3800,500);
                current_millis = millis();
                break;

                case 0x03:
                STATUS_LED = 0x00;
                CPU_LED = 0x00;
                GPU_LED = 0x00;
                RAM_LED = 0x00;
                update_cpanel();
                NewTone(BUZZER,3800,500);
                current_millis = millis();
                break;
            }
        }
    }
}

//74HC595 (CPANEL) output
void update_cpanel() {
    byte CPANEL = RAM_LED + (GPU_LED << 2) + (CPU_LED << 4) + (STATUS_LED << 6);
    for (int j = 0; j < 8; j++) 
    {
    // Output low level to latchPin
    digitalWrite(LED_LATCHPIN, LOW);
    // Send serial data to 74HC595
    shiftOut(LED_DATAPIN, LED_CLOCKPIN, LSBFIRST, CPANEL);
    // Output high level to latchPin, and 74HC595 will update the data to the parallel output port.
    digitalWrite(LED_LATCHPIN, HIGH);
    }
}

//Receives data from PC
void get_serial() {;
    Serial.println(F("<WAITING>"));
    while(!newData) {recvWithStartEndMarkers();}
    showNewData();
    return;
}

void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;
    //Increase Timeout if Serial fails
    while (Serial.available() == 0) {
        if(timeoutcounter == 1000000) {lcd.clear();timeout();}
        timeoutcounter++;}
    while (Serial.available() > 0 && newData == false) {
        timeoutcounter = 0;
        rc = Serial.read();
        if (recvInProgress == true) {
            if (rc != endMarker) {
                receivedChars[ndx] = rc;
                ndx++;
                if (ndx >= numChars) {
                    ndx = numChars - 1;
                }
            }
            else {
                receivedChars[ndx] = '\0'; // terminate the string
                recvInProgress = false;
                ndx = 0;
                newData = true;
            }
        }

        else if (rc == startMarker) {
            recvInProgress = true;
        }
    }
    return;
}

//Prepare to receive a new String from Serial
void showNewData() {if (newData == true) {newData = false;}}

//Timeout if Serial Connection Interrupts
void timeout() {
    int counter = 5;
    STATUS_LED = 0x01;
    CPU_LED = 0x01;
    GPU_LED = 0x01;
    RAM_LED = 0x01;
    update_cpanel();
    current_millis = millis();
    while(counter > 0) {
        check_on_off();
        lcd.setCursor(1,0);
        lcd.print(F("HELPER ERROR"));
        lcd.setCursor(0,1);
        lcd.print(F("RECONNECT IN:"));
        lcd.print(counter);
        if(millis() - current_millis >= 1000) {
            switch (STATUS_LED) {
                case 0x00:
                STATUS_LED = 0x01;
                CPU_LED = 0x01;
                GPU_LED = 0x01;
                RAM_LED = 0x01;
                update_cpanel();
                current_millis = millis();
                counter--;
                break;

                case 0x01:
                STATUS_LED = 0x00;
                CPU_LED = 0x00;
                GPU_LED = 0x00;
                RAM_LED = 0x00;
                update_cpanel();
                current_millis = millis();
                counter--;
                break;
            }
        }
    }
    wait_serial();
    monitor();
}

//Read Config stored in EEPROM
void EEPROM_READ() {
    //Brigthness is stored in Index 0
    BL_BRIGHTNESS = EEPROM.read(0);
    OLD_BL_BRIGHTNESS = BL_BRIGHTNESS;
    //Mode is stored in Index 1
    MODE = EEPROM.read(1);
    EEPROM_CHECK();
    return;    
}  

//Check if EEPROM CFG is valid
void EEPROM_CHECK() {
    //Checking if values are valid, if not rolling back to defaults
    if((BL_BRIGHTNESS == 0 || BL_BRIGHTNESS % 25 != 0) || (MODE < 1 || MODE > 3)) {
        BL_BRIGHTNESS = 250;
        EEPROM.update(0,BL_BRIGHTNESS);
        MODE = 1;
        EEPROM.update(1,MODE);
        analogWrite(LCD_BL, BL_BRIGHTNESS);
        lcd.setCursor(3,0);
        lcd.print(F("BAD CONFIG"));
        lcd.setCursor(4,1);
        lcd.print(F("ROLLBACK"));
        STATUS_LED = 0x01;
        update_cpanel();
        delay(3000);
    }
    return;
}

//Save new CFG on EEPROM
void EEPROM_UPDATE() {
    EEPROM.update(0,BL_BRIGHTNESS);
    EEPROM.update(1,MODE);
    OLD_BL_BRIGHTNESS = BL_BRIGHTNESS;
    lcd.clear();
    return;    
}

//Plays a Tone when a button is pressed
void OK_tone() {
    if(BUZZER_CFG) {if(BUZZER_ON) {NewTone(BUZZER,583,100);}}
    return;
}

// Restarts program from beginning but does not reset the peripherals and registers
void soft_Reset() {asm volatile ("  jmp 0");  }