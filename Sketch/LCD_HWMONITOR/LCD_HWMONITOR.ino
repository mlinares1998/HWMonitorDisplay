/*
HARDWARE MONITOR EXTERNAL Screen
POWERED BY ARDUINO
"INSERT GPL"
CREDITS
*/
//************************************ Init *****************************************//
//Libs
#include <EEPROM.h>
#include "src\NewLiquidCrystal\LiquidCrystal.h" //NewLiquidCrystal from Francisco Malpartida https://bitbucket.org/fmalpartida/
#include "src\DHT11-Library\dht.h" //DHT11 Library by Rob Tillaart https://github.com/RobTillaart/ 
#include "src\Arduino-IRremote\src\IRremote.h" //IRremote Library by Rafi Khan https://github.com/z3t0
#include "src\arduino-new-tone\NewTone.h" //NewTone Library by Tim Eckel https://bitbucket.org/teckel12/
#include "src\Bounce2\src\Bounce2.h" //Bounce2 Library by Thomas O Fredericks https://github.com/thomasfredericks

//Define Pins
const byte IR_DIODE = 2;
const byte LCD_D7 = 3;
const byte LCD_D6 = 4;
const byte LCD_D5 = 5;
const byte LCD_BL = 6;
const byte LCD_D4 = 7;
const byte LCD_E = 8;
const byte LCD_RS = 9;
const byte BUZZER = 10;
const byte LED_DATAPIN = 11;
const byte LED_LATCHPIN = 12;
const byte LED_CLOCKPIN = 13;
const byte FWU_PIN = A0;
const byte PWR_PHYS = A1;
const byte CFG_PHYS = A2;
const byte FORWARDS_PHYS = A3;
const byte OK_PHYS = A4;
const byte TEMP_HUMIDITY = A5;

//LCD Screen Init
LiquidCrystal lcd(LCD_RS,LCD_E,LCD_D4,LCD_D5,LCD_D6,LCD_D7); //4-bit Mode

//DHT11 Init
dht DHT;

//IR Receiver Init
IRrecv IRRECEIVE(IR_DIODE);
decode_results IR_RESULT;

//IR Remote buttons
volatile bool IR_ACTIVE;
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

//Config Variables
byte BL_BRIGHTNESS; //Brightness value (0-250)
byte OLD_BL_BRIGHTNESS; //Brightness Rollback if settings are not applied
bool BUZZER_CFG = true; //BUZZER Setting Read from EEPROM
bool BUZZER_ON; //Enable / Disable BUZZER

//STATUS LEDS (0x00 = OFF, 0x01 = RED, 0x02 = GREEN, 0x03 = YELLOW)
byte STATUS_LED;
byte CPU_LED;
byte GPU_LED;
byte RAM_LED;

//Monitoring Variables
char CPU_TEMP[4];
char CPU_FAN[5];
char CPU_CLK[5];
char CPU_C0_CLK[5];
char CPU_C0_VID[5];
char CPU_C1_CLK[5];
char CPU_C1_VID[5];
char CPU_C2_CLK[5];
char CPU_C2_VID[5];
char CPU_C3_CLK[5];
char CPU_C3_VID[5];
char CPU_USAGE[4];
char CPU_VCORE[5];
char CPU_VSOC[5];
char GPU_TEMP[4];
char GPU_CLK[5];
char VRAM_CLK[5];
char GPU_FAN{5};
char GPU_FPS[4];
char GPU_USAGE[4];
char GPU_VCORE[5];
char RAM_USED[7];
char RAM_FREE[7];
char PAGE_FILE_USAGE[4];
char VRAM_USED[6];
char RAM_CLK[5];
char RAM_CONTROLLER_CLK[5];
char UP_SPEED[5];
char DW_SPEED[5];
byte ROOM_TEMP;
byte ROOM_HUMIDITY;

//Selected Mode and Page tracking
byte MODE;
byte scroll_counter;
byte scroll_delay;
unsigned long current_millis; //To wait without using delay()

//DHT11 Reading Delay
byte DHT_Counter;

//Serial Monitoring
const static byte PROGMEM numChars = 255;
char receivedChars[numChars];
boolean newData = false;
bool timeoutOK;

//LCD Lines Arrays
char dataformat[8];

//Button Variables
//Using Bounce2 Library to deal with button bounce
Bounce debouncerPWR = Bounce();
Bounce debouncerCFG = Bounce();
Bounce debouncerOK = Bounce();
Bounce debouncerFORWARDS = Bounce();
bool TEST_BUTTON = false;
bool OK_BUTTON = false;
bool FORWARDS_BUTTON = false;
byte selected_item;
bool CFG_USING_BUTTONS;

//Version
const char version[4] = "3.0";

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
pinMode(TEMP_HUMIDITY,INPUT);
debouncerPWR.interval(25);
debouncerCFG.interval(25);
debouncerOK.interval(25);
debouncerFORWARDS.interval(25);
IRRECEIVE.enableIRIn(); //Enable IR Reception
attachInterrupt(digitalPinToInterrupt(IR_DIODE), check_IR, CHANGE);//IR Interrupt
lcd.begin(20,4); //LCD Start
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
    CHARLOAD(1);
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
    CHARLOAD(1);
    lcd.setCursor(10,0);
    lcd.write(byte(7));    
    lcd.setCursor(8,1);
    lcd.write(byte(6));
    lcd.write(byte(0));
    lcd.write(byte(1));
    lcd.write(byte(2));
    lcd.setCursor(9,2);
    lcd.write(byte(3));
    lcd.write(byte(4));
    lcd.write(byte(5));
    lcd.setCursor(17,3);
    lcd.print(version);
    //Show Splash for 4 seconds
    //Leds GREEN Progressively
    STATUS_LED = 0x02;
    update_cpanel();
    NBDelay(1000);
    CPU_LED = 0x02;
    update_cpanel();
    NBDelay(500);
    CHARLOAD(2);
    NBDelay(500);
    GPU_LED = 0x02;
    update_cpanel();
    NBDelay(500);
    CHARLOAD(3);
    NBDelay(200);
    CHARLOAD(2);
    NBDelay(300);
    RAM_LED = 0x02;
    update_cpanel();
    NBDelay(1000);
    lcd.clear();
    return;
}

void wait_serial() {
    //Clear Char Arrays
    memset(CPU_TEMP, 0, sizeof CPU_TEMP);
    memset(CPU_FAN, 0, sizeof CPU_FAN);
    memset(CPU_CLK, 0, sizeof CPU_CLK);
    memset(CPU_USAGE, 0, sizeof CPU_USAGE);
    memset(GPU_TEMP, 0, sizeof GPU_TEMP);
    memset(CPU_VCORE, 0, sizeof CPU_VCORE);
    memset(CPU_VSOC, 0, sizeof CPU_VSOC);
    memset(GPU_CLK, 0, sizeof GPU_CLK);
    memset(VRAM_CLK,0 , sizeof VRAM_CLK);
    memset(GPU_FAN, 0, sizeof GPU_FAN);
    memset(GPU_FPS, 0, sizeof GPU_FPS);
    memset(GPU_USAGE, 0, sizeof GPU_USAGE);
    memset(GPU_VCORE, 0, sizeof GPU_VCORE);
    memset(RAM_USED, 0, sizeof RAM_USED);
    memset(RAM_FREE, 0, sizeof RAM_FREE);
    memset(PAGE_FILE_USAGE, 0, sizeof PAGE_FILE_USAGE);
    memset(VRAM_USED, 0, sizeof VRAM_USED);
    memset(RAM_CLK,0 , sizeof RAM_CLK);
    memset(RAM_CONTROLLER_CLK, 0, sizeof RAM_CONTROLLER_CLK);
    memset(UP_SPEED, 0, sizeof UP_SPEED);
    memset(DW_SPEED, 0, sizeof DW_SPEED);
    memset(dataformat, 0, sizeof dataformat);
    memset(CPU_C0_CLK, 0, sizeof CPU_C0_CLK);
    memset(CPU_C0_VID, 0, sizeof CPU_C0_VID);
    memset(CPU_C1_CLK, 0, sizeof CPU_C1_CLK);
    memset(CPU_C1_VID, 0, sizeof CPU_C1_VID);
    memset(CPU_C2_CLK, 0, sizeof CPU_C2_CLK);
    memset(CPU_C2_VID, 0, sizeof CPU_C2_VID);
    memset(CPU_C3_CLK, 0, sizeof CPU_C3_CLK);
    memset(CPU_C3_VID, 0, sizeof CPU_C3_VID);
    Serial.begin(115200); //Serial Start
    //STATUS_LED = YELLOW OTHERS = OFF
    CPU_LED = 0x00;
    GPU_LED = 0x00;
    RAM_LED = 0x00;
    STATUS_LED = 0x03;
    update_cpanel();
    //Print Waiting message
    CHARLOAD(5);
    lcd.clear();
    lcd.setCursor(1,0);
    lcd.print(F("WAITING FOR HELPER"));
    lcd.setCursor(9,2);
    lcd.write(byte(0));
    lcd.write(byte(1));
    lcd.write(byte(2));
    lcd.write(byte(3));
    lcd.write(byte(4));
    lcd.setCursor(7,3);
    lcd.write(byte(5));
    lcd.write(byte(6));
    current_millis = millis();
    int dots = 1;
    while(Serial.available() == 0){
        if(dots > 3) {dots = 0;}
        //STATUS LED = YELLOW, BLINKS EVERY SECONDç
        if(millis() - current_millis >= 1000) {
            switch (STATUS_LED) {
                case 0x00:
                STATUS_LED = 0x03;
                break;

                case 0x03:
                STATUS_LED = 0x00;
                break;
            }
            update_cpanel();
            current_millis = millis();
            Serial.println(F("<WAITING FOR HELPER>"));
            CHARLOAD(5 + dots);
            dots++;
        }
        check_on_off();
    }
    //STATUS LED = GREEN
    STATUS_LED = 0x02;
    update_cpanel();
    lcd.clear();
    //Desync FIX
    lcd.setCursor(5,0);
    lcd.print(F("CONNECTED"));
    CHARLOAD(9);
    lcd.setCursor(9,2);
    lcd.write(byte(0));
    lcd.write(byte(1));
    lcd.write(byte(2));
    lcd.setCursor(9,3);
    lcd.write(byte(3));
    lcd.write(byte(4));
    lcd.write(byte(5));
    NBDelay(1000);
    lcd.clear();
    //Reset Buttons and Delays
    OK_BUTTON = false;
    FORWARDS_BUTTON = false;
    scroll_counter = 1;
    scroll_delay = 0;
    timeoutOK = false;
    return;
}

void monitor() {
while(true) {
    //LOAD CUSTOM SYMBOL CHARS
    CHARLOAD(13);
    //Clear Arrays
    memset(dataformat, 0, sizeof dataformat);
    //Clear IR button values
    IR_test = false;
    TEST_BUTTON = false;
    //Check if timneout is reached
    if(timeoutOK) {lcd.clear();timeout(); wait_serial();}
    //Receives Data from PC
    get_serial();
    //Refresh Values
    wait_to_refresh();
    //Enter selected mode
    if(MODE == 1) {EASYMode();}
    else if(MODE == 2 || MODE == 3) {AdvancedMode();}
    }
}

void EASYMode() {
    lcd.setCursor(0,0);
    lcd.print(F("CPU:"));
    snprintf(dataformat,8,"%3s%cC",CPU_TEMP,char(223));lcd.print(dataformat);
    snprintf(dataformat,8," %4sV",CPU_VCORE);lcd.print(dataformat);
    snprintf(dataformat,8," %3s%%",CPU_USAGE);lcd.print(dataformat);
    lcd.setCursor(0,1);
    lcd.print(F("GPU:"));
    snprintf(dataformat,8,"%3s%cC",GPU_TEMP,char(223));lcd.print(dataformat);
    snprintf(dataformat,8," %4sV",GPU_VCORE);lcd.print(dataformat);
    snprintf(dataformat,8," %3s%%",GPU_USAGE);lcd.print(dataformat);
    lcd.setCursor(0,2);
    lcd.print(F("RAM:"));
    snprintf(dataformat,8,"%6s",RAM_USED);lcd.print(dataformat);lcd.print(F("/"));
    snprintf(dataformat,8,"%-6s",RAM_FREE),lcd.print(dataformat);lcd.print(F(" MB"));
    lcd.setCursor(0,3);
    lcd.print(F("NET:"));
    snprintf(dataformat,8,"%4s",DW_SPEED);lcd.print(dataformat);lcd.print(F("/"));
    snprintf(dataformat,8,"%-4s",UP_SPEED),lcd.print(dataformat);lcd.print(F("M"));
    snprintf(dataformat,8,"%3s",GPU_FPS),lcd.print(dataformat);lcd.print(F("FPS"));
    return;
}
void AdvancedMode() {
    switch(scroll_counter) {
    //CPU CLOCKS/TEMP
    case 1:
    lcd.setCursor(4,0);
    lcd.print(F("<CPU CLOCKS>"));
    lcd.setCursor(0,1);
    lcd.print(F("AVG:"));snprintf(dataformat,8,"%4sM",CPU_CLK);lcd.print(dataformat);
    snprintf(dataformat,8," %3s%cC",CPU_TEMP,char(223));lcd.print(dataformat);
    snprintf(dataformat,8," %3s%%",CPU_USAGE);lcd.print(dataformat);
    lcd.setCursor(0,2);
    lcd.print(F("C0:"));snprintf(dataformat,8,"%4sM",CPU_C0_CLK);lcd.print(dataformat);
    lcd.print(F("    C1:"));snprintf(dataformat,8,"%4sM",CPU_C1_CLK);lcd.print(dataformat);
    lcd.setCursor(0,3);
    lcd.print(F("C2:"));snprintf(dataformat,8,"%4sM",CPU_C2_CLK);lcd.print(dataformat);
    lcd.print(F("    C3:"));snprintf(dataformat,8,"%4sM",CPU_C3_CLK);lcd.print(dataformat);
    break;
    //GPU VCORES
    case 2:
    lcd.setCursor(4,0);
    lcd.print(F("<CPU VCORES>"));
    lcd.setCursor(0,1);
    lcd.print(F("CORE:"));snprintf(dataformat,8,"%4sV",CPU_VCORE);lcd.print(dataformat);
    lcd.print(F(" SoC:"));snprintf(dataformat,8,"%4sV",CPU_VSOC);lcd.print(dataformat);
    lcd.setCursor(0,2);
    lcd.print(F("C0:"));snprintf(dataformat,8,"%4sV",CPU_C0_VID);lcd.print(dataformat);
    lcd.print(F("    C1:"));snprintf(dataformat,8,"%4sV",CPU_C1_VID);lcd.print(dataformat);
    lcd.setCursor(0,3);
    lcd.print(F("C2:"));snprintf(dataformat,8,"%4sV",CPU_C2_VID);lcd.print(dataformat);
    lcd.print(F("    C3:"));snprintf(dataformat,8,"%4sV",CPU_C3_VID);lcd.print(dataformat);
    break;
    case 3:
    //GPU
    lcd.setCursor(7,0);
    lcd.print(F("<GPU0>"));
    lcd.setCursor(0,1);
    lcd.print(F("TMP:"));
    snprintf(dataformat,8,"%3s%cC",GPU_TEMP,char(223));lcd.print(dataformat);
    snprintf(dataformat,8," %4sV",GPU_VCORE);lcd.print(dataformat);
    snprintf(dataformat,8," %3s%%",GPU_USAGE);lcd.print(dataformat);
    lcd.setCursor(0,2);
    lcd.print(F("CLK:"));snprintf(dataformat,8,"%4sM",GPU_CLK);lcd.print(dataformat);
    lcd.print(F(" MCLK:"));snprintf(dataformat,8,"%4sM",VRAM_CLK);lcd.print(dataformat);
    lcd.setCursor(0,3);
    lcd.print(F("VRAM:"));snprintf(dataformat,8,"%5sMB",VRAM_USED);lcd.print(dataformat);
    snprintf(dataformat,8,"  %3s",GPU_FPS),lcd.print(dataformat);lcd.print(F("FPS"));
    break;
    //RAM STATS
    case 4:
    lcd.setCursor(7,0);
    lcd.print(F("<RAM>"));
    lcd.setCursor(0,1);
    lcd.print(F("CLK:"));snprintf(dataformat,8,"%4sM",RAM_CLK);lcd.print(dataformat);
    lcd.print(F(" CTRL:"));snprintf(dataformat,8,"%4sM",RAM_CONTROLLER_CLK);lcd.print(dataformat);
    lcd.setCursor(0,2);
    lcd.print(F("U/F:"));
    snprintf(dataformat,8,"%6s",RAM_USED);lcd.print(dataformat);lcd.print(F("/"));
    snprintf(dataformat,8,"%-6s",RAM_FREE),lcd.print(dataformat);lcd.print(F(" MB"));
    lcd.setCursor(0,3);
    lcd.print(F("PAGE FILE USAGE:"));snprintf(dataformat,8,"%3s%%",PAGE_FILE_USAGE);lcd.print(dataformat);
    break;
    //Thermals
    case 5:
    byte DELTACPU = (atoi(CPU_TEMP) - byte(DHT.temperature));
    byte DELTAGPU = (atoi(GPU_TEMP) - byte(DHT.temperature));
    lcd.setCursor(5,0);
    lcd.print(F("<THERMALS>"));
    lcd.setCursor(0,1);
    lcd.print(F("CPU:"));snprintf(dataformat,8,"%3s%cC",CPU_TEMP,char(223));lcd.print(dataformat);
    lcd.print(F("  GPU:"));snprintf(dataformat,8,"%3s%cC",GPU_TEMP,char(223));lcd.print(dataformat);
    lcd.setCursor(0,2);
    lcd.print(F("ROOM:"));snprintf(dataformat,8,"%3i%cC",byte(DHT.temperature),char(223));lcd.print(dataformat);
    lcd.print(F("  HUM:"));snprintf(dataformat,8,"%3i%%",byte(DHT.humidity));lcd.print(dataformat);
    lcd.setCursor(0,3);
    lcd.write(byte(0));lcd.print(F("CPU"));snprintf(dataformat,8,"%3i%cC",DELTACPU,char(223));lcd.print(dataformat);
    lcd.print(F(" "));lcd.write(byte(0));lcd.print(F("GPU"));snprintf(dataformat,8,"%3i%cC",DELTAGPU,char(223));lcd.print(dataformat);
    break;
    //FANS
    case 6:
    break;
    }
return;
}

//Settings Menu
void config() {
    bool welcome_menu = true;
    bool modes_menu = false;
    bool BL_menu = false;
    selected_item = 1;
    if(IR_test) {CFG_USING_BUTTONS = false;}
    else if(TEST_BUTTON) {CFG_USING_BUTTONS = true;}
    //Disable CPU and GPU LEDS
    CPU_LED = 0x00;
    GPU_LED = 0x00;
    RAM_LED = 0x00;
    update_cpanel();
    //Show Settings splash for 3 seconds
    lcd.clear();
    lcd.setCursor(4,0);
    lcd.print(F("SETTINGS"));
    CHARLOAD(4);
    for(int i = 0; i <=5; i++) {
        lcd.setCursor(8,1);
        NBDelay(200);
        lcd.write(byte(0));
        lcd.setCursor(8,1);
        NBDelay(200);
        lcd.write(byte(1));
    }
    lcd.clear();
    while(welcome_menu || modes_menu || BL_menu) {
        while(welcome_menu) {
            //Selection menu
            if (!CFG_USING_BUTTONS) {
                lcd.setCursor(0,0);
                lcd.print(F("(1) MONITOR MODE"));
                lcd.setCursor(0,1);
                lcd.print(F("(2) BRIGHTNESS"));
                }
            else if(CFG_USING_BUTTONS) {
                lcd.setCursor(0,0);
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
            if(IR_1 && !CFG_USING_BUTTONS) {selected_item = 1; welcome_menu = false; modes_menu = true;break;}
            else if(IR_2 && !CFG_USING_BUTTONS) {welcome_menu = false; BL_menu = true; lcd.clear(); break;}
            else if(FORWARDS_BUTTON && CFG_USING_BUTTONS) {selected_item++;if(selected_item > 2) {selected_item = 1;}}
            else if(OK_BUTTON && CFG_USING_BUTTONS && selected_item == 1) {selected_item = 1; welcome_menu = false; modes_menu = true; break;}
            else if(OK_BUTTON && CFG_USING_BUTTONS && selected_item == 2) {welcome_menu = false; BL_menu = true;lcd.clear(); break;}
            else if((IR_back && !CFG_USING_BUTTONS) || (TEST_BUTTON && CFG_USING_BUTTONS)) {welcome_menu = false;lcd.clear();break;}
            else if(IR_ACTIVE && CFG_USING_BUTTONS) {CFG_USING_BUTTONS = false;lcd.clear();}
            else if(!IR_ACTIVE && !CFG_USING_BUTTONS) {CFG_USING_BUTTONS = true; selected_item = 1;lcd.clear();}
        }
        //Mode Selector
        while(modes_menu) {
            lcd.clear();
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
            if(IR_1 && !CFG_USING_BUTTONS) {MODE = 1; modes_menu = false; break;}
            else if(IR_2 && !CFG_USING_BUTTONS) {MODE = 2; modes_menu = false; break;}
            else if(IR_3 && !CFG_USING_BUTTONS) {MODE = 3; modes_menu = false;break;}
            else if(FORWARDS_BUTTON && CFG_USING_BUTTONS) {selected_item++;if(selected_item > 3) {selected_item = 1;}}
            else if(OK_BUTTON && CFG_USING_BUTTONS && selected_item == 1) {MODE = 1; modes_menu = false;break;}
            else if(OK_BUTTON && CFG_USING_BUTTONS && selected_item == 2) {MODE = 2; modes_menu = false;break;}
            else if(OK_BUTTON && CFG_USING_BUTTONS && selected_item == 3) {MODE = 3; modes_menu = false;break;}
            else if((IR_back && !CFG_USING_BUTTONS) || (TEST_BUTTON && CFG_USING_BUTTONS)) {welcome_menu = true; modes_menu = false;selected_item = 1;lcd.clear();break;}
            else if(IR_ACTIVE && CFG_USING_BUTTONS) {CFG_USING_BUTTONS = false;}
            else if(!IR_ACTIVE && !CFG_USING_BUTTONS) {CFG_USING_BUTTONS = true; selected_item = 1;}
        }
        
        //Brightness Selector
        while(BL_menu) {
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
            else if(FORWARDS_BUTTON && current_bl == 10) {BL_BRIGHTNESS = 25;analogWrite(LCD_BL,BL_BRIGHTNESS); lcd.clear();}
            else if(OK_BUTTON || IR_play){BL_menu = false;}
            else if(IR_forwards && current_bl < 10) {BL_BRIGHTNESS += 25;analogWrite(LCD_BL,BL_BRIGHTNESS);}
            else if(IR_backwards && current_bl > 1) {BL_BRIGHTNESS -= 25;analogWrite(LCD_BL,BL_BRIGHTNESS);}
            else if(IR_back || TEST_BUTTON) {
                if(IR_back) {CFG_USING_BUTTONS = false;}
                else{CFG_USING_BUTTONS = true;}
                welcome_menu = true; 
                BL_menu = false;
                analogWrite(LCD_BL,OLD_BL_BRIGHTNESS); 
                BL_BRIGHTNESS = OLD_BL_BRIGHTNESS;
                lcd.clear();
                selected_item = 1;
                break;
                }
            else {}
        }
    }
    //Save values to EEPROM
    EEPROM_UPDATE();
    //Reset Variables
    IR_forwards = false;
    IR_backwards = false;
    IR_back = false;
    IR_play = false;
    OK_BUTTON = false;
    FORWARDS_BUTTON = false;
    //Reset Scroll counter
    scroll_counter = 1;
    scroll_delay = 0;
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
        case 0xFF6897: IR_0 = true;OK_tone(); IR_ACTIVE = true;break;
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
    //If timeout isn't reached
    if(!timeoutOK) {
    //Wait 500ms until refresh
    NBDelay(500);
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
    if(scroll_counter == 7) {scroll_counter = 1;lcd.clear();}
    else if(scroll_counter == 0) {scroll_counter = 6;lcd.clear();}
    //LED Updating
    //Convert Char Arrays to numeric variables
    unsigned long RAM_FREE_ULONG = atoi(RAM_FREE);
    byte CPU_TEMP_BYTE = atoi(CPU_TEMP);
    byte GPU_TEMP_BYTE = atoi(GPU_TEMP);
    //CPU
    if(CPU_TEMP_BYTE < 60) {CPU_LED = 0x02;}
    else if((CPU_TEMP_BYTE >= 60) && (CPU_TEMP_BYTE < 80)) {CPU_LED = 0x03;}
    else if(CPU_TEMP_BYTE >= 80) {CPU_LED = 0x01;}
    //GPU
    if(GPU_TEMP_BYTE < 60) {GPU_LED = 0x02;}
    else if((GPU_TEMP_BYTE >= 60) && (GPU_TEMP_BYTE < 80)) {GPU_LED = 0x03;}
    else if(GPU_TEMP_BYTE >= 80) {GPU_LED = 0x01;}
    //RAM
    if(RAM_FREE_ULONG >= 2000) {RAM_LED = 0x02;}
    else if((RAM_FREE_ULONG >= 1000) && (RAM_FREE_ULONG < 1999)) {RAM_LED = 0x03;}
    else if(RAM_FREE_ULONG < 999) {RAM_LED = 0x01;}
    update_cpanel();
    //Increase DHT Delay Counter
    DHT_Counter++;
    return;
    }
    //If timeout is reached
    else {return;}
}

//Reads Sensors and Parse Downloaded Data
void getsensors() {
    //Read DHT11 every 1.5s and judge the state according to the return value
    if(DHT_Counter == 3) {int chk = DHT.read11(TEMP_HUMIDITY); DHT_Counter = 0;}
    // Decode Serial Data Array
    char *array_table[252];
    int i = 0;
    //Create an Array separating received data
    array_table[i] = strtok(receivedChars,":");
    while(array_table[i] != NULL) {
        array_table[++i] = strtok(NULL,":");
    }
    for(int i = 0; array_table[i] != NULL; i +=2) {
        if(String(array_table[i]) == "UR") {String(array_table[i+1]).toCharArray(RAM_USED,7);}
        else if(String(array_table[i]) == "FR") {String(array_table[i+1]).toCharArray(RAM_FREE,7);}
        else if(String(array_table[i]) == "UV") {String(array_table[i+1]).toCharArray(VRAM_USED,6);}
        else if(String(array_table[i]) == "PF") {String(array_table[i+1]).toCharArray(PAGE_FILE_USAGE,4);}
        else if(String(array_table[i]) == "RC") {String(array_table[i+1]).toCharArray(RAM_CLK,5);}
        else if(String(array_table[i]) == "RCC") {String(array_table[i+1]).toCharArray(RAM_CONTROLLER_CLK,5);}
        else if(String(array_table[i]) == "CCA") {String(array_table[i+1]).toCharArray(CPU_CLK,5);}
        else if(String(array_table[i]) == "C0C") {String(array_table[i+1]).toCharArray(CPU_C0_CLK,5);}
        else if(String(array_table[i]) == "C0V") {String(array_table[i+1]).toCharArray(CPU_C0_VID,5);}
        else if(String(array_table[i]) == "C1C") {String(array_table[i+1]).toCharArray(CPU_C1_CLK,5);}
        else if(String(array_table[i]) == "C1V") {String(array_table[i+1]).toCharArray(CPU_C1_VID,5);}
        else if(String(array_table[i]) == "C2C") {String(array_table[i+1]).toCharArray(CPU_C2_CLK,5);}
        else if(String(array_table[i]) == "C2V") {String(array_table[i+1]).toCharArray(CPU_C2_VID,5);}
        else if(String(array_table[i]) == "C3C") {String(array_table[i+1]).toCharArray(CPU_C3_CLK,5);}
        else if(String(array_table[i]) == "C3V") {String(array_table[i+1]).toCharArray(CPU_C3_VID,5);}
        else if(String(array_table[i]) == "CUA") {String(array_table[i+1]).toCharArray(CPU_USAGE,4);}
        else if(String(array_table[i]) == "CT") {String(array_table[i+1]).toCharArray(CPU_TEMP,4);}
        else if(String(array_table[i]) == "CVT") {String(array_table[i+1]).toCharArray(CPU_VCORE,5);}
        else if(String(array_table[i]) == "CVS") {String(array_table[i+1]).toCharArray(CPU_VSOC,5);}
        else if(String(array_table[i]) == "GV") {String(array_table[i+1]).toCharArray(GPU_VCORE,5);}
        else if(String(array_table[i]) == "GT") {String(array_table[i+1]).toCharArray(GPU_TEMP,4);}
        else if(String(array_table[i]) == "GC") {String(array_table[i+1]).toCharArray(GPU_CLK,5);}
        else if(String(array_table[i]) == "VRC") {String(array_table[i+1]).toCharArray(VRAM_CLK,5);}
        else if(String(array_table[i]) == "GU") {String(array_table[i+1]).toCharArray(GPU_USAGE,4);}
        else if(String(array_table[i]) == "FP") {String(array_table[i+1]).toCharArray(GPU_FPS,4);}
        else if(String(array_table[i]) == "UP") {String(array_table[i+1]).toCharArray(UP_SPEED,5);}
        else if(String(array_table[i]) == "DW") {String(array_table[i+1]).toCharArray(DW_SPEED,5);}
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
    CHARLOAD(4);
    //LCD Startup, load brightness value stored in EEPROM
    analogWrite(LCD_BL, 250);
     //STATUS_LED = YELLOW
    STATUS_LED = 0x03;
    CPU_LED = 0x03;
    GPU_LED = 0x03;
    RAM_LED = 0x03;
    update_cpanel();
    //Welcome Message
    lcd.setCursor(2,0);
    lcd.print(F("FIRMWARE UPDATE"));
    lcd.setCursor(9,2);
    lcd.write(byte(0));
    lcd.write(byte(1));
    lcd.write(byte(2));
    lcd.write(byte(3));
    lcd.write(byte(4));
    lcd.setCursor(7,3);
    lcd.write(byte(5));
    lcd.write(byte(6));
    lcd.setCursor(17,3);
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
    //First Chunk
    Serial.println(F("<WAITINGB1>"));
    //Read data until char is fully received or timeout
    current_millis = millis();
    while(!newData && !timeoutOK) {recvWithStartEndMarkers();}
    showNewData();
    //Get sensors data
    getsensors();
    delay(10);
    //Second Chunk
    Serial.println(F("<WAITINGB2>"));
    //Read data until char is fully received or timeout
    current_millis = millis();
    while(!newData && !timeoutOK) {recvWithStartEndMarkers();}
    showNewData();
    //Get sensors data
    getsensors();
    return;
}

void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;
    //Increase Timeout if Serial fails
    if(Serial.available() == 0) {
        //Activate TimeoutOK bool
        if(millis() - current_millis == 2000) {timeoutOK = true;}
    }
    while (Serial.available() > 0 && newData == false) {
        timeoutOK = false;
        current_millis = millis();
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
    //Close Serial Port
    Serial.end();
    int counter = 5;
    STATUS_LED = 0x01;
    CPU_LED = 0x01;
    GPU_LED = 0x01;
    RAM_LED = 0x01;
    update_cpanel();
    current_millis = millis();
    CHARLOAD(10);
    lcd.setCursor(9,2);
    lcd.write(byte(0));
    lcd.write(byte(1));
    lcd.write(byte(2));
    lcd.write(byte(3));
    lcd.write(byte(4));
    lcd.setCursor(7,3);
    lcd.write(byte(5));
    lcd.write(byte(6));
    lcd.setCursor(17,3);
    int dots = 1;
    while(counter > 0) {
        check_on_off();
        lcd.setCursor(4,0);
        lcd.print(F("HELPER ERROR"));
        lcd.setCursor(3,1);
        lcd.print(F("RECONNECT IN:"));
        lcd.print(counter);
        if(millis() - current_millis >= 1000) {
            switch (STATUS_LED) {
                case 0x00:
                STATUS_LED = 0x01;
                CPU_LED = 0x01;
                GPU_LED = 0x01;
                RAM_LED = 0x01;
                break;

                case 0x01:
                STATUS_LED = 0x00;
                CPU_LED = 0x00;
                GPU_LED = 0x00;
                RAM_LED = 0x00;
                break;
            }
        CHARLOAD(10 + dots);
        dots++;
        if (dots > 1) {dots = 0;}
        update_cpanel();
        current_millis = millis();
        counter--;
        }
    }
    return;
}

//Read Config stored in EEPROM
void EEPROM_READ() {
    //Brigthness is stored in Index 0
    BL_BRIGHTNESS = EEPROM.read(0);
    OLD_BL_BRIGHTNESS = BL_BRIGHTNESS;
    //Mode is stored in Index 1
    MODE = EEPROM.read(1);
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
    if(BUZZER_CFG && BUZZER_ON) {NewTone(BUZZER,583,100);}
    return;
}

// Restarts program from beginning but does not reset the peripherals and registers
void soft_Reset() {asm volatile ("  jmp 0");}

//LOAD CUSTOM LOGOS FROM FLASH TO RAM
void CHARLOAD(int mode) {

    //WELCOME SCREEN
    static const byte PROGMEM CHAR_0_WELCOME_A[8] = {0x00,0x00,0x00,0x1F,0x11,0x11,0x11,0x1F};
    static const byte PROGMEM CHAR_1_WELCOME_A[8] = {0x15,0x15,0x00,0x1F,0x11,0x11,0x11,0x1F};
    static const byte PROGMEM CHAR_2_WELCOME_A[8] = {0x00,0x00,0x00,0x1F,0x11,0x11,0x11,0x1F};
    static const byte PROGMEM CHAR_3_WELCOME_A[8] = {0x1F,0x11,0x11,0x11,0x1F,0x10,0x0F,0x02};
    static const byte PROGMEM CHAR_4_WELCOME_A[8] = {0x1F,0x11,0x11,0x11,0x1F,0x00,0x1F,0x00};
    static const byte PROGMEM CHAR_5_WELCOME_A[8] = {0x1F,0x12,0x14,0x18,0x10,0x00,0x1F,0x02};
    static const byte PROGMEM CHAR_6_WELCOME_A[8] = {0x06,0x03,0x01,0x00,0x00,0x00,0x00,0x00};
    static const byte PROGMEM CHAR_7_WELCOME_A[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x04};

    static const byte PROGMEM CHAR_0_WELCOME_B[8] = {0x0F,0x10,0x10,0x11,0x11,0x11,0x11,0x10};
    static const byte PROGMEM CHAR_1_WELCOME_B[8] = {0x1F,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    static const byte PROGMEM CHAR_2_WELCOME_B[8] = {0x1E,0x01,0x01,0x11,0x11,0x11,0x11,0x01};
    static const byte PROGMEM CHAR_3_WELCOME_B[8] = {0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x0F};
    static const byte PROGMEM CHAR_4_WELCOME_B[8] = {0x00,0x00,0x1F,0x0E,0x04,0x00,0x00,0x1F};
    static const byte PROGMEM CHAR_5_WELCOME_B[8] = {0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x1E};
    static const byte PROGMEM CHAR_6_WELCOME_B[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    static const byte PROGMEM CHAR_7_WELCOME_B[8] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

    static const byte PROGMEM CHAR_0_WELCOME_C[8] = {0x0F,0x10,0x10,0x11,0x11,0x11,0x11,0x10};
    static const byte PROGMEM CHAR_1_WELCOME_C[8] = {0x1F,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
    static const byte PROGMEM CHAR_2_WELCOME_C[8] = {0x1E,0x01,0x01,0x01,0x01,0x01,0x01,0x01};
    static const byte PROGMEM CHAR_3_WELCOME_C[8] = {0x10,0x10,0x10,0x10,0x10,0x10,0x10,0x0F};
    static const byte PROGMEM CHAR_4_WELCOME_C[8] = {0x00,0x00,0x1F,0x0E,0x04,0x00,0x00,0x1F};
    static const byte PROGMEM CHAR_5_WELCOME_C[8] = {0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x1E};

    //FIRMWARE UPDATE SCREEN
    static const byte PROGMEM CHAR_0_FWU[8] = {0x00,0x01,0x01,0x01,0x01,0x09,0x15,0x08};
    static const byte PROGMEM CHAR_1_FWU[8] = {0x1F,0x00,0x0E,0x08,0x0C,0x08,0x00,0x1F};
    static const byte PROGMEM CHAR_2_FWU[8] = {0x1F,0x00,0x11,0x11,0x15,0x0A,0x00,0x1F};
    static const byte PROGMEM CHAR_3_FWU[8] = {0x1F,0x00,0x11,0x11,0x11,0x0E,0x00,0x1F};
    static const byte PROGMEM CHAR_4_FWU[8] = {0x00,0x10,0x10,0x10,0x10,0x10,0x10,0x00};
    static const byte PROGMEM CHAR_5_FWU[8] = {0x1F,0x10,0x12,0x12,0x12,0x10,0x10,0x10};
    static const byte PROGMEM CHAR_6_FWU[8] = {0x1F,0x01,0x05,0x05,0x05,0x01,0x01,0x01};

    //WAIT FOR SERIAL SCREEN
    static const byte PROGMEM CHAR_0_SERIAL_A[8] = {0x00,0x00,0x00,0x00,0x04,0x0A,0x04,0x00};
    static const byte PROGMEM CHAR_1_SERIAL_A[8] = {0x0F,0x10,0x10,0x10,0x10,0x10,0x10,0x0F};
    static const byte PROGMEM CHAR_2_SERIAL_A[8] = {0x1F,0x00,0x00,0x00,0x00,0x00,0x00,0x1F};
    static const byte PROGMEM CHAR_3_SERIAL_A[8] = {0x1F,0x00,0x00,0x00,0x00,0x00,0x00,0x1F};
    static const byte PROGMEM CHAR_4_SERIAL_A[8] = {0x00,0x10,0x10,0x10,0x10,0x10,0x10,0x00};
    static const byte PROGMEM CHAR_5_SERIAL_A[8] = {0x1F,0x10,0x12,0x12,0x12,0x10,0x10,0x10};
    static const byte PROGMEM CHAR_6_SERIAL_A[8] = {0x1F,0x01,0x05,0x05,0x05,0x01,0x01,0x01};

    static const byte PROGMEM CHAR_1_SERIAL_B[8] = {0x0F,0x10,0x10,0x16,0x16,0x10,0x10,0x0F};
    static const byte PROGMEM CHAR_2_SERIAL_B[8] = {0x1F,0x00,0x00,0x06,0x06,0x00,0x00,0x1F};
    static const byte PROGMEM CHAR_3_SERIAL_B[8] = {0x1F,0x00,0x00,0x06,0x06,0x00,0x00,0x1F};

    //CONNECTION OK
    static const byte PROGMEM CHAR_0_OK[8] = {0x0F,0x10,0x10,0x13,0x14,0x14,0x14,0x14};
    static const byte PROGMEM CHAR_1_OK[8] = {0x1F,0x00,0x00,0x12,0x0A,0x0A,0x0A,0x0B};
    static const byte PROGMEM CHAR_2_OK[8] = {0x1E,0x01,0x01,0x05,0x09,0x11,0x01,0x01};
    static const byte PROGMEM CHAR_3_OK[8] = {0x14,0x14,0x14,0x14,0x13,0x10,0x10,0x0F};
    static const byte PROGMEM CHAR_4_OK[8] = {0x0B,0x0A,0x0A,0x0A,0x12,0x00,0x00,0x1F};
    static const byte PROGMEM CHAR_5_OK[8] = {0x01,0x01,0x11,0x09,0x05,0x01,0x01,0x1E};

    //HELPER ERROR
    static const byte PROGMEM CHAR_0_FAIL_A[8] = {0x00,0x01,0x01,0x01,0x01,0x09,0x15,0x08};
    static const byte PROGMEM CHAR_1_FAIL_A[8] = {0x1F,0x04,0x0A,0x0E,0x08,0x06,0x00,0x1F};
    static const byte PROGMEM CHAR_2_FAIL_A[8] = {0x1F,0x00,0x12,0x1B,0x12,0x12,0x00,0x1F};
    static const byte PROGMEM CHAR_3_FAIL_A[8] = {0x1F,0x00,0x04,0x0A,0x0A,0x04,0x00,0x1F};
    static const byte PROGMEM CHAR_4_FAIL_A[8] = {0x1E,0x01,0x09,0x0D,0x09,0x09,0x01,0x1E};
    static const byte PROGMEM CHAR_5_FAIL_A[8] = {0x1F,0x10,0x15,0x12,0x15,0x10,0x10,0x10};
    static const byte PROGMEM CHAR_6_FAIL_A[8] = {0x1F,0x01,0x15,0x09,0x15,0x01,0x01,0x01};

    static const byte PROGMEM CHAR_5_FAIL_B[8] = {0x1F,0x10,0x10,0x10,0x10,0x10,0x10,0x10};
    static const byte PROGMEM CHAR_6_FAIL_B[8] = {0x1F,0x01,0x01,0x01,0x01,0x01,0x01,0x01};


    //CFG SCREEN


    //DELTA SYMBOL
    static const byte PROGMEM CHAR_DELTA[8] = {0x00,0x00,0x04,0x0E,0x1F,0x00,0x00,0x00};
    //Custom Chars BUFFER
    byte RAMCHARS[8] = {};
    switch(mode) {
        case 1:
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_0_WELCOME_A[i];
            lcd.createChar(0,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_1_WELCOME_A[i];
            lcd.createChar(1,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_2_WELCOME_A[i];
            lcd.createChar(2,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_3_WELCOME_A[i];
            lcd.createChar(3,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_4_WELCOME_A[i];
            lcd.createChar(4,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_5_WELCOME_A[i];
            lcd.createChar(5,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_6_WELCOME_A[i];
            lcd.createChar(6,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_7_WELCOME_A[i];
            lcd.createChar(7,(uint8_t *)RAMCHARS);
            break;
        case 2:
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_0_WELCOME_B[i];
            lcd.createChar(0,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_1_WELCOME_B[i];
            lcd.createChar(1,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_2_WELCOME_B[i];
            lcd.createChar(2,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_3_WELCOME_B[i];
            lcd.createChar(3,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_4_WELCOME_B[i];
            lcd.createChar(4,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_5_WELCOME_B[i];
            lcd.createChar(5,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_6_WELCOME_B[i];
            lcd.createChar(6,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_7_WELCOME_B[i];
            lcd.createChar(7,(uint8_t *)RAMCHARS);
            break;
        case 3:
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_0_WELCOME_C[i];
            lcd.createChar(0,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_1_WELCOME_C[i];
            lcd.createChar(1,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_2_WELCOME_C[i];
            lcd.createChar(2,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_3_WELCOME_C[i];
            lcd.createChar(3,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_4_WELCOME_C[i];
            lcd.createChar(4,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_5_WELCOME_C[i];
            lcd.createChar(5,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_6_WELCOME_B[i];
            lcd.createChar(6,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_7_WELCOME_B[i];
            lcd.createChar(7,(uint8_t *)RAMCHARS);
            break;
        case 4:
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_0_FWU[i];
            lcd.createChar(0,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_1_FWU[i];
            lcd.createChar(1,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_2_FWU[i];
            lcd.createChar(2,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_3_FWU[i];
            lcd.createChar(3,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_4_FWU[i];
            lcd.createChar(4,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_5_FWU[i];
            lcd.createChar(5,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_6_FWU[i];
            lcd.createChar(6,(uint8_t *)RAMCHARS);
            break;
        case 5:
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_0_SERIAL_A[i];
            lcd.createChar(0,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_1_SERIAL_A[i];
            lcd.createChar(1,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_2_SERIAL_A[i];
            lcd.createChar(2,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_3_SERIAL_A[i];
            lcd.createChar(3,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_4_SERIAL_A[i];
            lcd.createChar(4,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_5_SERIAL_A[i];
            lcd.createChar(5,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_6_SERIAL_A[i];
            lcd.createChar(6,(uint8_t *)RAMCHARS);
            break;
        case 6:
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_1_SERIAL_B[i];
            lcd.createChar(1,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_2_SERIAL_A[i];
            lcd.createChar(2,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_3_SERIAL_A[i];
            lcd.createChar(3,(uint8_t *)RAMCHARS);
            break;
        case 7:
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_2_SERIAL_B[i];
            lcd.createChar(2,(uint8_t *)RAMCHARS);
            break;
        case 8:
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_3_SERIAL_B[i];
            lcd.createChar(3,(uint8_t *)RAMCHARS);
            break;
        case 9:
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_0_OK[i];
            lcd.createChar(0,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_1_OK[i];
            lcd.createChar(1,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_2_OK[i];
            lcd.createChar(2,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_3_OK[i];
            lcd.createChar(3,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_4_OK[i];
            lcd.createChar(4,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_5_OK[i];
            lcd.createChar(5,(uint8_t *)RAMCHARS);
            break;
        case 10:
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_0_FAIL_A[i];
            lcd.createChar(0,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_1_FAIL_A[i];
            lcd.createChar(1,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_2_FAIL_A[i];
            lcd.createChar(2,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_3_FAIL_A[i];
            lcd.createChar(3,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_4_FAIL_A[i];
            lcd.createChar(4,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_5_FAIL_A[i];
            lcd.createChar(5,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_6_FAIL_A[i];
            lcd.createChar(6,(uint8_t *)RAMCHARS);
            break;
        case 11:
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_5_FAIL_B[i];
            lcd.createChar(5,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_6_FAIL_B[i];
            lcd.createChar(6,(uint8_t *)RAMCHARS);
            break;
        case 12:
            //CFG
        case 13:
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_DELTA[i];
            lcd.createChar(0,(uint8_t *)RAMCHARS);
        }
}
//Checks On-Off buttons while waiting.
void NBDelay(int time) {
    current_millis = millis(); 
    while (millis() - current_millis <= time) {check_on_off();}
    }

