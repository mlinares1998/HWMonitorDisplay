/*
HARDWARE MONITOR EXTERNAL Screen
POWERED BY ARDUINO
"INSERT GPL"
CREDITS
*/
//************************************ Init *****************************************//

//F() Macro Reimplementation
//Credits: https://forum.arduino.cc/index.php?topic=310410.0
#define FS(a)  (reinterpret_cast<const __FlashStringHelper *>(a))

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
byte OLD_scroll_delay;
bool BUZZER_CFG; //BUZZER Setting Read from EEPROM
bool BUZZER_ON; //Enable / Disable BUZZER

//Selected Mode and Page tracking
byte MODE;
byte scroll_counter;
byte scroll_timer;
byte scroll_delay;
unsigned long current_millis; //To wait without using delay()
unsigned long refresh_delay;

//STATUS LEDS (0x00 = OFF, 0x01 = RED, 0x02 = GREEN, 0x03 = YELLOW)
byte STATUS_LED;
byte CPU_LED;
byte GPU_LED;
byte RAM_LED;

//Monitoring Variables
char CPU_TEMP[4];
char CPU_FAN[5];
char CH1_FAN[5];
char CH2_FAN[5];
char CPU_CLK[5];
char CPU_C0_CLK[5];
char CPU_C0_VID[5];
char CPU_C1_CLK[5];
char CPU_C1_VID[5];
char CPU_C2_CLK[5];
char CPU_C2_VID[5];
char CPU_C3_CLK[5];
char CPU_C3_VID[5];
char CPU_C4_CLK[5];
char CPU_C4_VID[5];
char CPU_C5_CLK[5];
char CPU_C5_VID[5];
char CPU_C6_CLK[5];
char CPU_C6_VID[5];
char CPU_C7_CLK[5];
char CPU_C7_VID[5];
char CPU_C8_CLK[5];
char CPU_C8_VID[5];
char CPU_C9_CLK[5];
char CPU_C9_VID[5];
char CPU_C10_CLK[5];
char CPU_C10_VID[5];
char CPU_C11_CLK[5];
char CPU_C11_VID[5];
char CPU_C12_CLK[5];
char CPU_C12_VID[5];
char CPU_C13_CLK[5];
char CPU_C13_VID[5];
char CPU_C14_CLK[5];
char CPU_C14_VID[5];
char CPU_C15_CLK[5];
char CPU_C15_VID[5];
char CPU_USAGE[4];
char CPU_VCORE[5];
char CPU_VSOC[5];
char CPU_POWER[6];
char SOC_POWER[6];
char CPU_TOTAL_POWER[6];
char GPU_POWER[6];
char GPU_TEMP[4];
char GPU_CLK[5];
char VRAM_CLK[5];
char GPU_FAN[5];
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
char DAY[3];
char MONTH[3];
char YEAR[5];
char HOUR[3];
char MINUTE[3];
char SECOND[3];
char GMT[6];
byte ROOM_TEMP;
byte ROOM_HUMIDITY;

//DHT11 Reading Delay
byte DHT_Counter;

//Serial Monitoring
const static byte PROGMEM numChars = 128;
char * receivedChars = malloc(numChars);
boolean newData = false;
bool timeoutOK;

//Version
const char PROGMEM version[] = "3.0";
//LCD String Arrays
const char PROGMEM str_1[] = "WAITING FOR HELPER";
const char PROGMEM str_2[] = "CONNECTED";
const char PROGMEM str_3[] = "CPU:";
const char PROGMEM str_4[] = "GPU:";
const char PROGMEM str_5[] = "RAM:";
const char PROGMEM str_6[] = "NET:";
const char PROGMEM str_7[] = "FPS";
const char PROGMEM str_8[] = " MB";
const char PROGMEM str_9[] = "<CPU0>";
const char PROGMEM str_10[] = "AVG:";
const char PROGMEM str_11[] = "T:";
const char PROGMEM str_12[] = "CORE:";
const char PROGMEM str_13[] = "SoC:";
const char PROGMEM str_14[] = "USE:";
const char PROGMEM str_15[] = "PWR:";
const char PROGMEM str_16[] = "<GPU0>";
const char PROGMEM str_17[] = "CLK:";
const char PROGMEM str_18[] = "MCLK:";
const char PROGMEM str_19[] ="VRAM:";
const char PROGMEM str_20[] ="<RAM>";
const char PROGMEM str_21[] = "CTRL:";
const char PROGMEM str_22[] = "U/F:";
const char PROGMEM str_23[] = "PAGE:";
const char PROGMEM str_24[] = "<THERMALS>";
const char PROGMEM str_25[] = "ROOM:";
const char PROGMEM str_26[] = " HUMD:";
const char PROGMEM str_27[] = "<FANs (RPM)>";
const char PROGMEM str_28[] = "CH1:";
const char PROGMEM str_29[] = "CH2:";
const char PROGMEM str_30[] = "<POWERS>";
const char PROGMEM str_31[] = "C+S:";
const char PROGMEM str_32[] = "<CLOCK>";
const char PROGMEM str_33[] = "GMT:";
const char PROGMEM str_34[] = "SETTINGS";
const char PROGMEM str_35[] = "( ) ";
const char PROGMEM str_36[] = "*";
const char PROGMEM str_37[] = "MONITOR MODE";
const char PROGMEM str_38[] = "BRIGHTNESS";
const char PROGMEM str_39[] = "SCROLL DELAY";
const char PROGMEM str_40[] = "SPEAKER";
const char PROGMEM str_41[] = "EASY MODE";
const char PROGMEM str_42[] = "AUTO SCROLL";
const char PROGMEM str_43[] = "MANUAL SCROLL";
const char PROGMEM str_44[] = "C00:";
const char PROGMEM str_45[] = "C01:";
const char PROGMEM str_46[] = "C02:";
const char PROGMEM str_47[] = "C03:";
const char PROGMEM str_48[] = "C04:";
const char PROGMEM str_49[] = "C05:";
const char PROGMEM str_50[] = "C06:";
const char PROGMEM str_51[] = "C07:";
const char PROGMEM str_52[] = "C08:";
const char PROGMEM str_53[] = "C09:";
const char PROGMEM str_54[] = "C10:";
const char PROGMEM str_55[] = "C11:";
const char PROGMEM str_56[] = "C12:";
const char PROGMEM str_57[] = "C13:";
const char PROGMEM str_58[] = "C14:";
const char PROGMEM str_59[] = "C15:";
const char PROGMEM str_60[] = "1";
const char PROGMEM str_61[] = "2";
const char PROGMEM str_62[] = "3";
const char PROGMEM str_63[] = "4";
const char PROGMEM str_64[] = "SECONDS";
const char PROGMEM str_65[] = "ON";
const char PROGMEM str_66[] = "OFF";


//LCD Line Array
char dataformat[9];

//Button Variables
//Using Bounce2 Library to deal with button bounce
Bounce debouncerPWR = Bounce();
Bounce debouncerCFG = Bounce();
Bounce debouncerOK = Bounce();
Bounce debouncerFORWARDS = Bounce();
bool TEST_BUTTON = false;
bool OK_BUTTON = false;
bool FORWARDS_BUTTON = false;
bool CFG_USING_BUTTONS;

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
    //Disable BUZZER
    BUZZER_ON = false;
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
    //Load Settings from EEPROM
    EEPROM_READ();
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
    lcd.print(FS(version));
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
    ClearArrays();
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
    lcd.print(FS(str_1));
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
    byte dots = 1;
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
        Serial.println(F("H"));
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
    CHARLOAD(2);
    lcd.setCursor(6,0);
    lcd.print(FS(str_2)); 
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
    scroll_timer = 0;
    timeoutOK = false;
    //Start Delay Timer
    refresh_delay = millis();
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
    lcd.setCursor(0,0);lcd.print(FS(str_3));ShowDevTemp(CPU_TEMP);
    lcd.setCursor(10,0);ShowDevVolt(CPU_VCORE);
    lcd.setCursor(16,0);ShowDevUsage(CPU_USAGE);

    lcd.setCursor(0,1);lcd.print(FS(str_4));ShowDevTemp(GPU_TEMP);
    lcd.setCursor(10,1);ShowDevVolt(GPU_VCORE);
    lcd.setCursor(16,1);ShowDevUsage(GPU_USAGE);

    lcd.setCursor(0,2);lcd.print(FS(str_5));
    ShowRAMSTATS();

    lcd.setCursor(0,3);
    lcd.print(FS(str_6));
    ShowNICSpeed(DW_SPEED, UP_SPEED);
    if(GPU_FPS[0] != 0) {ShowFPS(GPU_FPS);}
    return;
}
void AdvancedMode() {
    switch(scroll_counter) {
    //CPU CLOCKS/TEMP/VCORES
    case 1:
    lcd.setCursor(7,0);
    lcd.print(FS(str_9));
    lcd.setCursor(0,1);
    lcd.print(FS(str_10));ShowDevClk(CPU_CLK);lcd.print(F("hz"));
    lcd.setCursor(13,1);
    lcd.print(FS(str_11));ShowDevTemp(CPU_TEMP);
    lcd.setCursor(0,2);
    lcd.print(FS(str_12));ShowDevVolt(CPU_VCORE);
    lcd.setCursor(11,2);
    lcd.print(FS(str_13));ShowDevVolt(CPU_VSOC);
    lcd.setCursor(0,3);
    lcd.print(FS(str_14));ShowDevUsage(CPU_USAGE);
    lcd.setCursor(10,3);
    lcd.print(FS(str_15));ShowPowerUsage(CPU_POWER);
    break;
    case 2:
    if(CPU_C0_CLK[0] != 0) {ShowCPUCore(0, str_44, CPU_C0_CLK, CPU_C0_VID);}
    if(CPU_C1_CLK[0] != 0) {ShowCPUCore(1, str_45, CPU_C1_CLK, CPU_C1_VID);}
    if(CPU_C2_CLK[0] != 0) {ShowCPUCore(2, str_46, CPU_C2_CLK, CPU_C2_VID);}
    if(CPU_C3_CLK[0] != 0) {ShowCPUCore(3, str_47, CPU_C3_CLK, CPU_C3_VID);}
    break;
    case 3:
    if(CPU_C4_CLK[0] != 0) {ShowCPUCore(0, str_48, CPU_C4_CLK, CPU_C4_VID);}
    if(CPU_C5_CLK[0] != 0) {ShowCPUCore(1, str_49, CPU_C5_CLK, CPU_C5_VID);}
    if(CPU_C6_CLK[0] != 0) {ShowCPUCore(2, str_50, CPU_C6_CLK, CPU_C6_VID);}
    if(CPU_C7_CLK[0] != 0) {ShowCPUCore(3, str_51, CPU_C7_CLK, CPU_C7_VID);}
    break;
    case 4:
    if(CPU_C8_CLK[0] != 0) {ShowCPUCore(0, str_52, CPU_C8_CLK, CPU_C8_VID);}
    if(CPU_C9_CLK[0] != 0) {ShowCPUCore(1, str_53, CPU_C9_CLK, CPU_C9_VID);}
    if(CPU_C10_CLK[0] != 0) {ShowCPUCore(2, str_54, CPU_C10_CLK, CPU_C10_VID);}
    if(CPU_C11_CLK[0] != 0) {ShowCPUCore(3, str_55, CPU_C11_CLK, CPU_C11_VID);}
    break;
    case 5:
    if(CPU_C12_CLK[0] != 0) {ShowCPUCore(0, str_56, CPU_C12_CLK, CPU_C12_VID);}
    if(CPU_C13_CLK[0] != 0) {ShowCPUCore(1, str_57, CPU_C13_CLK, CPU_C13_VID);}
    if(CPU_C14_CLK[0] != 0) {ShowCPUCore(2, str_58, CPU_C14_CLK, CPU_C14_VID);}
    if(CPU_C15_CLK[0] != 0) {ShowCPUCore(3, str_59, CPU_C15_CLK, CPU_C15_VID);}
    break;
    case 6:
    //GPU
    lcd.setCursor(7,0);
    lcd.print(FS(str_16));
    lcd.setCursor(2,1);
    lcd.print(FS(str_11));
    ShowDevTemp(GPU_TEMP);
    lcd.setCursor(10,1);
    ShowDevVolt(GPU_VCORE);
    lcd.setCursor(16,1);
    ShowDevUsage(GPU_USAGE);
    lcd.setCursor(0,2);
    lcd.print(FS(str_17));ShowDevClk(GPU_CLK);lcd.setCursor(10,2);
    lcd.print(FS(str_18));ShowDevClk(VRAM_CLK);
    lcd.setCursor(0,3);
    ShowGPUVRAM(VRAM_USED);
    if(GPU_FPS[0] != 0){lcd.setCursor(13,3);ShowFPS(GPU_FPS);}
    break;
    //RAM STATS
    case 7:
    lcd.setCursor(7,0);
    lcd.print(FS(str_20));
    lcd.setCursor(0,1);
    lcd.print(FS(str_17));ShowDevClk(RAM_CLK);lcd.setCursor(10,1);
    lcd.print(FS(str_21));ShowDevClk(RAM_CONTROLLER_CLK);
    lcd.setCursor(0,2);
    lcd.print(FS(str_22));
    ShowRAMSTATS();
    lcd.setCursor(0,3);
    lcd.print(FS(str_23));ShowDevUsage(PAGE_FILE_USAGE);
    break;
    //Thermals
    case 8:
    lcd.setCursor(5,0);
    lcd.print(FS(str_24));
    lcd.setCursor(0,1);
    lcd.print(FS(str_3));ShowDevTemp(CPU_TEMP);lcd.setCursor(11,1);
    lcd.print(FS(str_4));ShowDevTemp(GPU_TEMP);
    lcd.setCursor(0,2);
    lcd.print(FS(str_25));ShowRoomTemp();
    lcd.print(FS(str_26));ShowRoomHumidity();
    lcd.setCursor(0,3);
    ShowDevDelta(str_3, CPU_TEMP);
    ShowDevDelta(str_4, GPU_TEMP);
    break;
    //FANS
    case 9:
    lcd.setCursor(4,0);
    lcd.print(FS(str_27));
    lcd.setCursor(0,1);
    lcd.print(FS(str_3));ShowFANSpeed(CPU_FAN);
    lcd.setCursor(12,1);
    lcd.print(FS(str_4));ShowFANSpeed(GPU_FAN);
    lcd.setCursor(0,2);
    lcd.print(FS(str_28));ShowFANSpeed(CH1_FAN);
    lcd.setCursor(12,2);
    lcd.print(FS(str_29));ShowFANSpeed(CH2_FAN);
    break;
    //POWERS
    case 10:
    lcd.setCursor(6,0);
    lcd.print(FS(str_30));
    lcd.setCursor(0,1);
    lcd.print(FS(str_3));ShowPowerUsage(CPU_POWER);
    lcd.print(FS(str_13));ShowPowerUsage(SOC_POWER);
    lcd.setCursor(0,2);
    lcd.print(FS(str_31));ShowPowerUsage(CPU_TOTAL_POWER);
    lcd.setCursor(0,3);
    lcd.print(FS(str_4));ShowPowerUsage(GPU_POWER);
    break;
    //TIME/DATE
    case 11:
    lcd.setCursor(7,0);
    lcd.print(FS(str_32));
    lcd.setCursor(5,1);ShowDate(DAY, MONTH, YEAR);
    lcd.setCursor(6,2);ShowHour(HOUR, MINUTE, SECOND);
    lcd.setCursor(5,3);ShowGMT();
    break;
    }
    return;
}

void ShowCPUCore(byte line, char* CORE, char* CLK, char* VID) {
    lcd.setCursor(0,line);
    lcd.print(FS(CORE));ShowDevClk(CLK);lcd.print(F("hz"));
    lcd.setCursor(15,line);snprintf(dataformat,8,"%4sV",VID);lcd.print(dataformat);
    return;
}
void ShowDevTemp(char* ID) {snprintf(dataformat,8,"%3s%cC",ID,char(223));lcd.print(dataformat);return;}
void ShowDevClk(char* ID) {snprintf(dataformat,8,"%4sM",ID);lcd.print(dataformat);return;}
void ShowDevVolt(char* ID) {snprintf(dataformat,8,"%4sV",ID);lcd.print(dataformat);return;}
void ShowDevUsage(char* ID) {snprintf(dataformat,8,"%3s%%",ID);lcd.print(dataformat);return;}
void ShowFPS(char* ID) {snprintf(dataformat,8,"%3s",ID),lcd.print(dataformat);lcd.print(FS(str_7));return;}
void ShowGPUVRAM(char* ID) {lcd.print(FS(str_19));snprintf(dataformat,8,"%5sMB",ID);lcd.print(dataformat);return;}
void ShowFANSpeed(char* ID) {snprintf(dataformat,8,"%4s",ID);lcd.print(dataformat);return;}
void ShowRoomTemp() {snprintf(dataformat,8,"%3i%cC",byte(DHT.temperature),char(223));lcd.print(dataformat);return;}
void ShowRoomHumidity() {snprintf(dataformat,8,"%3i%%",byte(DHT.humidity));lcd.print(dataformat);return;}
void ShowPowerUsage(char* ID) {snprintf(dataformat,8,"%5sW",ID);lcd.print(dataformat);}
void ShowGMT() {lcd.print(FS(str_33));snprintf(dataformat,8," %5s",GMT);lcd.print(dataformat);return;}

void ShowRAMSTATS() {
    snprintf(dataformat,8,"%6s/",RAM_USED);lcd.print(dataformat);
    snprintf(dataformat,8,"%-6s",RAM_FREE),lcd.print(dataformat);lcd.print(FS(str_8));
    return;
    }
void ShowNICSpeed(char* DW, char* UP) {
    snprintf(dataformat,8,"%4s/",DW);lcd.print(dataformat);
    snprintf(dataformat,8,"%-4sM",UP),lcd.print(dataformat);
    return;
}

void ShowDevDelta(char* IDNAME, char* ID) {
    byte DELTA = atoi(ID) - byte(DHT.temperature);
    lcd.write(byte(0));lcd.print(FS(IDNAME));snprintf(dataformat,8,"%3i%cC",DELTA,char(223));lcd.print(dataformat);
    return;
}

void ShowDate(char* DAY, char* MONTH, char* YEAR) {
    snprintf(dataformat,8,"%2s/",DAY);lcd.print(dataformat);
    snprintf(dataformat,8,"%2s/",MONTH);lcd.print(dataformat);
    snprintf(dataformat,8,"%4s",YEAR);lcd.print(dataformat);
    return;
}

void ShowHour(char* HOUR, char* MINUTE, char* SECOND) {
    snprintf(dataformat,8,"%2s:",HOUR);lcd.print(dataformat);
    snprintf(dataformat,8,"%2s:",MINUTE);lcd.print(dataformat);
    snprintf(dataformat,8,"%2s",SECOND);lcd.print(dataformat);
    return;
}


//Settings Menu
void config() {
    bool welcome_menu = true;
    bool modes_menu = false;
    bool BL_menu = false;
    bool scroll_menu = false;
    bool speaker_menu = false;
    byte selected_item = 1;
    if(IR_test) {CFG_USING_BUTTONS = false;}
    else {CFG_USING_BUTTONS = true;}
    //Disable CPU and GPU LEDS
    CPU_LED = 0x00;
    GPU_LED = 0x00;
    RAM_LED = 0x00;
    update_cpanel();
    //Show Settings splash for 3 seconds
    lcd.clear();
    lcd.setCursor(7,0);
    lcd.print(FS(str_34));
    while(welcome_menu || modes_menu || BL_menu || scroll_menu || speaker_menu) {
        while(welcome_menu) {
                lcd.setCursor(2,0);
                lcd.print(FS(str_35));lcd.print(FS(str_37));
                lcd.setCursor(2,1);
                lcd.print(FS(str_35));lcd.print(FS(str_38));
                lcd.setCursor(2,2);
                lcd.print(FS(str_35));lcd.print(FS(str_39));
                lcd.setCursor(2,3);
                lcd.print(FS(str_35));lcd.print(FS(str_40));
            //Selection menu
            if (!CFG_USING_BUTTONS) {
                lcd.setCursor(3,0);
                lcd.print(FS(str_60));
                lcd.setCursor(3,1);
                lcd.print(FS(str_61));
                lcd.setCursor(3,2);
                lcd.print(FS(str_62));
                lcd.setCursor(3,3);
                lcd.print(FS(str_63));

                }
            else if(CFG_USING_BUTTONS) {
                switch(selected_item) {
                case 1:
                lcd.setCursor(3,0);
                lcd.print(FS(str_36));
                break;
                case 2:
                lcd.setCursor(3,1);
                lcd.print(FS(str_36));
                break;
                case 3:
                lcd.setCursor(3,2);
                lcd.print(FS(str_36));
                break;
                case 4:
                lcd.setCursor(3,3);
                lcd.print(FS(str_36));
                break;
                }
            }
            //Clear IR and physical buttons values
            IR_1 = false;
            IR_2 = false;
            IR_3 = false;
            IR_4 = false;
            IR_back = false;
            TEST_BUTTON = false;
            OK_BUTTON = false;
            FORWARDS_BUTTON = false;
            IR_ACTIVE = false;
            while(!IR_ACTIVE && !TEST_BUTTON && !OK_BUTTON && !FORWARDS_BUTTON) {check_on_off();}
            if(IR_1 && !CFG_USING_BUTTONS) {selected_item = 1; welcome_menu = false; modes_menu = true;break;}
            else if(IR_2 && !CFG_USING_BUTTONS) {welcome_menu = false; BL_menu = true; lcd.clear(); break;}
            else if(IR_3 && !CFG_USING_BUTTONS) {welcome_menu = false; scroll_menu = true; break;}
            else if(IR_4 && !CFG_USING_BUTTONS) {welcome_menu = false; selected_item = BUZZER_CFG; speaker_menu = true; break;}
            else if(FORWARDS_BUTTON && CFG_USING_BUTTONS) {selected_item++;if(selected_item > 4) {selected_item = 1;}}
            else if(OK_BUTTON && CFG_USING_BUTTONS && selected_item == 1) {selected_item = 1; welcome_menu = false; modes_menu = true; break;}
            else if(OK_BUTTON && CFG_USING_BUTTONS && selected_item == 2) {welcome_menu = false; BL_menu = true;lcd.clear(); break;}
            else if(OK_BUTTON && CFG_USING_BUTTONS && selected_item == 3) {welcome_menu = false; scroll_menu = true; break;}
            else if(OK_BUTTON && CFG_USING_BUTTONS && selected_item == 4) {welcome_menu = false; selected_item = BUZZER_CFG; speaker_menu = true; break;}
            else if((IR_back && !CFG_USING_BUTTONS) || (TEST_BUTTON && CFG_USING_BUTTONS)) {welcome_menu = false;lcd.clear();break;}
            else if(IR_ACTIVE && CFG_USING_BUTTONS) {CFG_USING_BUTTONS = false;lcd.clear();}
            else if(!IR_ACTIVE && !CFG_USING_BUTTONS) {CFG_USING_BUTTONS = true; selected_item = 1;lcd.clear();}
        }
        //Mode Selector
        while(modes_menu) {
            lcd.clear();
            lcd.setCursor(4,0);
            lcd.print(FS(str_37));
            lcd.setCursor(2,1);
            lcd.print(FS(str_35));lcd.print(FS(str_41));
            lcd.setCursor(2,2);
            lcd.print(FS(str_35));lcd.print(FS(str_42));
            lcd.setCursor(2,3);
            lcd.print(FS(str_35));lcd.print(FS(str_43));
            if(IR_ACTIVE) {
            lcd.setCursor(3,1);
            lcd.print(FS(str_60));
            lcd.setCursor(3,2);
            lcd.print(FS(str_61));
            lcd.setCursor(3,3);
            lcd.print(FS(str_62));
            }
            if(CFG_USING_BUTTONS) {
                if(selected_item == 1) {lcd.setCursor(3,1);lcd.print(FS(str_36));}
                if(selected_item == 2) {lcd.setCursor(3,2);lcd.print(FS(str_36));}
                if(selected_item == 3) {lcd.setCursor(3,3);lcd.print(FS(str_36));}
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
            while(!IR_ACTIVE && !TEST_BUTTON && !OK_BUTTON && !FORWARDS_BUTTON) {check_on_off();}
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
            lcd.setCursor(5,0);
            lcd.print(FS(str_38));
            lcd.setCursor(3,2);
            lcd.print(F("-"));
            lcd.setCursor(5,2);
            while(current_bl != 0) {
                lcd.print(char(255));
                current_bl--;
            }
            current_bl = BL_BRIGHTNESS / 25;
            lcd.setCursor(current_bl + 5,2);
            while(current_bl != 0) {
                lcd.print(F(" "));
                current_bl--;
            }
            lcd.setCursor(16,2);
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

        while(scroll_menu) {
            lcd.clear();
            lcd.setCursor(4,0);
            lcd.print(FS(str_39));
            lcd.setCursor(4,2);
            snprintf(dataformat,8,"%3i ",(scroll_delay / 2));lcd.print(dataformat);
            lcd.print(FS(str_64));
            //Clean IR buttons values, show options and wait until one of each is pushed.
            IR_back = false;
            IR_forwards = false;
            IR_backwards = false;
            TEST_BUTTON = false;
            OK_BUTTON = false;
            FORWARDS_BUTTON = false;
            IR_ACTIVE = false;
            while(!IR_ACTIVE && !TEST_BUTTON && !OK_BUTTON && !FORWARDS_BUTTON) {check_on_off();}
            if((IR_forwards && !CFG_USING_BUTTONS) || (FORWARDS_BUTTON && CFG_USING_BUTTONS)) 
                {
                if(scroll_delay < 180) {scroll_delay +=2;}
                else if(scroll_delay >= 180) {scroll_delay = 2;}
                }
            else if(IR_backwards && !CFG_USING_BUTTONS) 
                {
                if(scroll_delay > 2) {scroll_delay -=2;}
                else if(scroll_delay <= 2) {scroll_delay = 180;}
                }
            else if((IR_play && !CFG_USING_BUTTONS) || (OK_BUTTON && CFG_USING_BUTTONS)) {scroll_menu = false;break;}
            else if((IR_back && !CFG_USING_BUTTONS) || (TEST_BUTTON && CFG_USING_BUTTONS)) 
                {
                scroll_delay = OLD_scroll_delay; 
                welcome_menu = true; 
                scroll_menu = false;
                selected_item = 1;
                lcd.clear();
                break;}
            else if(IR_ACTIVE && CFG_USING_BUTTONS) {CFG_USING_BUTTONS = false;}
            else if(!IR_ACTIVE && !CFG_USING_BUTTONS) {CFG_USING_BUTTONS = true;}
        }
        while(speaker_menu) {
            lcd.clear();
            lcd.setCursor(7,0);
            lcd.print(FS(str_40));
            lcd.setCursor(2,2);
            lcd.print(FS(str_35));lcd.print(FS(str_65));
            lcd.setCursor(2,3);
            lcd.print(FS(str_35));lcd.print(FS(str_66));
            if(selected_item == 0) {lcd.setCursor(3,3);lcd.print(FS(str_36));}
            if(selected_item == 1) {lcd.setCursor(3,2);lcd.print(FS(str_36));}
            //Clean IR buttons values, show options and wait until one of each is pushed.
            IR_back = false;
            IR_forwards = false;
            IR_backwards = false;
            TEST_BUTTON = false;
            OK_BUTTON = false;
            FORWARDS_BUTTON = false;
            IR_ACTIVE = false;
            while(!IR_ACTIVE && !TEST_BUTTON && !OK_BUTTON && !FORWARDS_BUTTON) {check_on_off();}
            if((IR_forwards && !CFG_USING_BUTTONS) || (FORWARDS_BUTTON && CFG_USING_BUTTONS)) 
                {
                if(selected_item >= 1) {selected_item = 0;}
                else {selected_item++;}
                }
            else if(IR_backwards && !CFG_USING_BUTTONS) 
                {
                if(selected_item > 0) {selected_item--;}
                else {selected_item = 1;}
                }
            else if((IR_play && !CFG_USING_BUTTONS) || (OK_BUTTON && CFG_USING_BUTTONS)) {BUZZER_CFG = selected_item; speaker_menu = false;break;}
            else if((IR_back && !CFG_USING_BUTTONS) || (TEST_BUTTON && CFG_USING_BUTTONS)) 
                {
                welcome_menu = true; 
                speaker_menu = false;
                selected_item = 1;
                lcd.clear();
                break;}
            else if(IR_ACTIVE && CFG_USING_BUTTONS) {CFG_USING_BUTTONS = false;}
            else if(!IR_ACTIVE && !CFG_USING_BUTTONS) {CFG_USING_BUTTONS = true;}
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
    scroll_timer = 0;
    refresh_delay = millis();
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
    while(millis() - refresh_delay < 500) {check_on_off();}
    refresh_delay = millis();
    //If settings button is pushed, go to settings
    if(IR_test || TEST_BUTTON) {config();}
    //Pause if IR Play is pushed (MODE 2)
    else if(MODE == 2 && (!IR_play && !OK_BUTTON)) {
        scroll_timer++;
        //Change Page when counter reachs set delay
        if(scroll_timer == (scroll_delay)) {
            scroll_counter++;
            scroll_timer = 0;
            if(CPU_C4_CLK[0] == 0 && scroll_counter == 3) {scroll_counter = 6;}
            if(CPU_C8_CLK[0] == 0 && scroll_counter == 4) {scroll_counter = 6;}
            if(CPU_C12_CLK[0] == 0 && scroll_counter == 5) {scroll_counter = 6;}
            lcd.clear();}
    }

    //Manual Control (MODE 3)
    else if(MODE == 3 && (IR_backwards || IR_forwards || FORWARDS_BUTTON)) {
        //Change Page if Forwards or Backwards buttons are pushed.
        if (IR_backwards) {
            scroll_counter--;
            IR_backwards = false;
            if(CPU_C12_CLK[0] == 0 && scroll_counter == 5) {scroll_counter--;}
            if(CPU_C8_CLK[0] == 0 && scroll_counter == 4) {scroll_counter--;}
            if(CPU_C4_CLK[0] == 0 && scroll_counter == 3) {scroll_counter--;}
            lcd.clear();
            }
            if(IR_forwards || FORWARDS_BUTTON) {
            scroll_counter++;
            IR_forwards = false; 
            FORWARDS_BUTTON = false;
            if(CPU_C4_CLK[0] == 0 && scroll_counter == 3) {scroll_counter = 6;}
            if(CPU_C8_CLK[0] == 0 && scroll_counter == 4) {scroll_counter = 6;}
            if(CPU_C12_CLK[0] == 0 && scroll_counter == 5) {scroll_counter = 6;}
            lcd.clear();}
            }
    //Back to Start/End
    if(scroll_counter == 12) {scroll_counter = 1;lcd.clear();}
    else if(scroll_counter == 0) {scroll_counter = 11;lcd.clear();}
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
    char *array_table[125];
    int i = 0;
    //Create an Array separating received data
    array_table[i] = strtok(receivedChars,":");
    while(array_table[i] != NULL) {
        array_table[++i] = strtok(NULL,":");
    }
    for(int i = 0; array_table[i] != NULL; i +=2) {
        char* ID = array_table[i];
        char* Value = array_table[i+1];
        if(strcmp(ID,"UR") == 0) {strncpy(RAM_USED, Value,7);}
        else if(strcmp(ID,"FR") == 0) {strncpy(RAM_FREE, Value,7);}
        else if(strcmp(ID,"UV") == 0) {strncpy(VRAM_USED, Value,6);}
        else if(strcmp(ID,"PF") == 0) {strncpy(PAGE_FILE_USAGE, Value,4);}
        else if(strcmp(ID,"RC") == 0) {strncpy(RAM_CLK, Value,5);}
        else if(strcmp(ID,"RCC") == 0) {strncpy(RAM_CONTROLLER_CLK, Value,5);}
        else if(strcmp(ID,"CCA") == 0) {strncpy(CPU_CLK, Value,5);}
        else if(strcmp(ID,"C0C") == 0) {strncpy(CPU_C0_CLK, Value,5);}
        else if(strcmp(ID,"C0V") == 0) {strncpy(CPU_C0_VID, Value,5);}
        else if(strcmp(ID,"C1C") == 0) {strncpy(CPU_C1_CLK, Value,5);}
        else if(strcmp(ID,"C1V") == 0) {strncpy(CPU_C1_VID, Value,5);}
        else if(strcmp(ID,"C2C") == 0) {strncpy(CPU_C2_CLK, Value,5);}
        else if(strcmp(ID,"C2V") == 0) {strncpy(CPU_C2_VID, Value,5);}
        else if(strcmp(ID,"C3C") == 0) {strncpy(CPU_C3_CLK, Value,5);}
        else if(strcmp(ID,"C3V") == 0){strncpy(CPU_C3_VID, Value,5);}
        else if(strcmp(ID,"C4C") == 0) {strncpy(CPU_C4_CLK, Value,5);}
        else if(strcmp(ID,"C4V") == 0) {strncpy(CPU_C4_VID, Value,5);}
        else if(strcmp(ID,"C5C") == 0) {strncpy(CPU_C5_CLK, Value,5);}
        else if(strcmp(ID,"C5V") == 0) {strncpy(CPU_C5_VID, Value,5);}
        else if(strcmp(ID,"C6C") == 0) {strncpy(CPU_C6_CLK, Value,5);}
        else if(strcmp(ID,"C6V") == 0) {strncpy(CPU_C6_VID, Value,5);}
        else if(strcmp(ID,"C7C") == 0) {strncpy(CPU_C7_CLK, Value,5);}
        else if(strcmp(ID,"C7V") == 0) {strncpy(CPU_C7_VID, Value,5);}
        else if(strcmp(ID,"C8C") == 0) {strncpy(CPU_C8_CLK, Value,5);}
        else if(strcmp(ID,"C8V") == 0) {strncpy(CPU_C8_VID, Value,5);}
        else if(strcmp(ID,"C9C") == 0) {strncpy(CPU_C9_CLK, Value,5);}
        else if(strcmp(ID,"C9V") == 0) {strncpy(CPU_C9_VID, Value,5);}
        else if(strcmp(ID,"C10C") == 0) {strncpy(CPU_C10_CLK, Value,5);}
        else if(strcmp(ID,"C10V") == 0) {strncpy(CPU_C10_VID, Value,5);}
        else if(strcmp(ID,"C11C") == 0) {strncpy(CPU_C11_CLK, Value,5);}
        else if(strcmp(ID,"C11V") == 0) {strncpy(CPU_C11_VID, Value,5);}
        else if(strcmp(ID,"C12C") == 0) {strncpy(CPU_C12_CLK, Value,5);}
        else if(strcmp(ID,"C12V") == 0) {strncpy(CPU_C12_VID, Value,5);}
        else if(strcmp(ID,"C13C") == 0) {strncpy(CPU_C13_CLK, Value,5);}
        else if(strcmp(ID,"C13V") == 0) {strncpy(CPU_C13_VID, Value,5);}
        else if(strcmp(ID,"C14C") == 0) {strncpy(CPU_C14_CLK, Value,5);}
        else if(strcmp(ID,"C14V") == 0) {strncpy(CPU_C14_VID, Value,5);}
        else if(strcmp(ID,"C15C") == 0) {strncpy(CPU_C15_CLK, Value,5);}
        else if(strcmp(ID,"C15V") == 0) {strncpy(CPU_C15_VID, Value,5);}
        else if(strcmp(ID,"CUA") == 0) {strncpy(CPU_USAGE, Value,4);}
        else if(strcmp(ID,"CT") == 0) {strncpy(CPU_TEMP, Value,4);}
        else if(strcmp(ID,"CPF") == 0) {strncpy(CPU_FAN, Value,5);}
        else if(strcmp(ID,"CH1") == 0) {strncpy(CH1_FAN, Value,5);}
        else if(strcmp(ID,"CH2") == 0) {strncpy(CH2_FAN, Value,5);}
        else if(strcmp(ID,"CVT") == 0) {strncpy(CPU_VCORE, Value,5);}
        else if(strcmp(ID,"CVS") == 0) {strncpy(CPU_VSOC, Value,5);}
        else if(strcmp(ID,"CW") == 0) {strncpy(CPU_POWER, Value,6);}
        else if(strcmp(ID,"SW") == 0) {strncpy(SOC_POWER, Value,6);}
        else if(strcmp(ID,"CWT") == 0) {strncpy(CPU_TOTAL_POWER, Value,6);}
        else if(strcmp(ID,"GW") == 0) {strncpy(GPU_POWER, Value,6);}
        else if(strcmp(ID,"GV") == 0) {strncpy(GPU_VCORE, Value,5);}
        else if(strcmp(ID,"GT") == 0) {strncpy(GPU_TEMP, Value,4);}
        else if(strcmp(ID,"GPF") == 0) {strncpy(GPU_FAN, Value,5);}
        else if(strcmp(ID,"GC") == 0) {strncpy(GPU_CLK, Value,5);}
        else if(strcmp(ID,"VRC") == 0) {strncpy(VRAM_CLK, Value,5);}
        else if(strcmp(ID,"GU") == 0) {strncpy(GPU_USAGE, Value,4);}
        else if(strcmp(ID,"FP") == 0) {strncpy(GPU_FPS, Value,4);}
        else if(strcmp(ID,"UP") == 0) {strncpy(UP_SPEED, Value,5);}
        else if(strcmp(ID,"DW") == 0) {strncpy(DW_SPEED, Value,5);}
        else if(strcmp(ID,"YY") == 0) {strncpy(YEAR, Value,5);}
        else if(strcmp(ID,"MM") == 0) {strncpy(MONTH, Value,3);}
        else if(strcmp(ID,"DD") == 0) {strncpy(DAY, Value,3);}
        else if(strcmp(ID,"HH") == 0) {strncpy(HOUR, Value,3);}
        else if(strcmp(ID,"MN") == 0) {strncpy(MINUTE, Value,3);}
        else if(strcmp(ID,"SS") == 0) {strncpy(SECOND, Value,3);}
        else if(strcmp(ID,"GM") == 0) {strncpy(GMT, Value,6);}
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
    lcd.print(FS(version));
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
    Serial.println(F("A"));
    read_chunk();
    if(timeoutOK) {return;}
    //Second Chunk
    Serial.println(F("B"));
    read_chunk();
    if(timeoutOK) {return;}
    //Third Chunk
    Serial.println(F("C"));
    read_chunk();
    if(timeoutOK) {return;}
    //Forth Chunk
    Serial.println(F("D"));
    read_chunk();
    if(timeoutOK) {return;}
    //Fifth Chunk
    Serial.println(F("E"));
    read_chunk();
    if(timeoutOK) {return;}
    return;
}

void read_chunk() {
    //Read data until char is fully received or timeout
    current_millis = millis();
    while(!newData && !timeoutOK) {recvWithStartEndMarkers();}
    if(timeoutOK) {return;}
    showNewData();
    //Get sensors data
    getsensors();
    return;}

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
    //Buzzer CFG is stored in Index 2
    BUZZER_CFG = EEPROM.read(2);
    //Scroll Delay is stored in index 3
    scroll_delay = EEPROM.read(3);
    OLD_scroll_delay = scroll_delay;
    //Checking if values are valid, if not rolling back to defaults
    if((BL_BRIGHTNESS == 0 || BL_BRIGHTNESS % 25 != 0) 
        || (MODE < 1 || MODE > 3)
        || (BUZZER_CFG < 0 || BUZZER_CFG > 1)
        || (scroll_delay < 0 || scroll_delay > 180 || scroll_delay % 2 != 0)) {
        lcd.setCursor(5,0);
        lcd.print(F("BAD CONFIG"));
        lcd.setCursor(6,1);
        lcd.print(F("ROLLBACK"));
        STATUS_LED = 0x03;
        CPU_LED = 0x03;
        GPU_LED = 0x03;
        RAM_LED = 0x03;
        update_cpanel();
        BL_BRIGHTNESS = 250;
        EEPROM.update(0,BL_BRIGHTNESS);
        MODE = 1;
        EEPROM.update(1,MODE);
        BUZZER_CFG = 1;
        EEPROM.update(2,BUZZER_CFG);
        scroll_delay = 2;
        EEPROM.update(3,scroll_delay);
        analogWrite(LCD_BL, BL_BRIGHTNESS);
        delay(10000);
        soft_Reset();
    }
    return;
}

//Save new CFG on EEPROM
void EEPROM_UPDATE() {
    EEPROM.update(0,BL_BRIGHTNESS);
    EEPROM.update(1,MODE);
    EEPROM.update(2,BUZZER_CFG);
    EEPROM.update(3,scroll_delay);
    OLD_BL_BRIGHTNESS = BL_BRIGHTNESS;
    OLD_scroll_delay = scroll_delay;
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

    static const byte PROGMEM CHAR_2_WELCOME_C[8] = {0x1E,0x01,0x01,0x01,0x01,0x01,0x01,0x01};

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

    static const byte PROGMEM CHAR_1_SERIAL_B[8] = {0x0F,0x10,0x10,0x16,0x16,0x10,0x10,0x0F};
    static const byte PROGMEM CHAR_2_SERIAL_B[8] = {0x1F,0x00,0x00,0x06,0x06,0x00,0x00,0x1F};

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
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_0_WELCOME_A[i];
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
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_0_WELCOME_B[i];
            lcd.createChar(0,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_1_WELCOME_B[i];
            lcd.createChar(1,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_2_WELCOME_C[i];
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
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_2_SERIAL_A[i];
            lcd.createChar(3,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_4_FWU[i];
            lcd.createChar(4,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_5_FWU[i];
            lcd.createChar(5,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_6_FWU[i];
            lcd.createChar(6,(uint8_t *)RAMCHARS);
            break;
        case 6:
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_1_SERIAL_B[i];
            lcd.createChar(1,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_2_SERIAL_A[i];
            lcd.createChar(2,(uint8_t *)RAMCHARS);
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_2_SERIAL_A[i];
            lcd.createChar(3,(uint8_t *)RAMCHARS);
            break;
        case 7:
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_2_SERIAL_B[i];
            lcd.createChar(2,(uint8_t *)RAMCHARS);
            break;
        case 8:
            for(byte i = 0; i <8; i++) RAMCHARS[i] = CHAR_2_SERIAL_B[i];
            lcd.createChar(3,(uint8_t *)RAMCHARS);
            break;
        case 9:
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

//Clear Arrays before receiving Serial Data
void ClearArrays() {
    CPU_TEMP[0] = '\0'; 
    CPU_FAN[0] = '\0'; 
    CH1_FAN[0] = '\0';
    CH2_FAN[0] = '\0';
    CPU_CLK[0] = '\0'; 
    CPU_C0_CLK[0] = '\0'; 
    CPU_C0_VID[0] = '\0'; 
    CPU_C1_CLK[0] = '\0'; 
    CPU_C1_VID[0] = '\0'; 
    CPU_C2_CLK[0] = '\0'; 
    CPU_C2_VID[0] = '\0'; 
    CPU_C3_CLK[0] = '\0'; 
    CPU_C3_VID[0] = '\0'; 
    CPU_C4_CLK[0] = '\0'; 
    CPU_C4_VID[0] = '\0'; 
    CPU_C5_CLK[0] = '\0'; 
    CPU_C5_VID[0] = '\0'; 
    CPU_C6_CLK[0] = '\0'; 
    CPU_C6_VID[0] = '\0'; 
    CPU_C7_CLK[0] = '\0'; 
    CPU_C7_VID[0] = '\0'; 
    CPU_C8_CLK[0] = '\0'; 
    CPU_C8_VID[0] = '\0'; 
    CPU_C9_CLK[0] = '\0'; 
    CPU_C9_VID[0] = '\0'; 
    CPU_C10_CLK[0] = '\0'; 
    CPU_C10_VID[0] = '\0'; 
    CPU_C11_CLK[0] = '\0'; 
    CPU_C11_VID[0] = '\0'; 
    CPU_C12_CLK[0] = '\0'; 
    CPU_C12_VID[0] = '\0'; 
    CPU_C13_CLK[0] = '\0'; 
    CPU_C13_VID[0] = '\0'; 
    CPU_C14_CLK[0] = '\0'; 
    CPU_C14_VID[0] = '\0'; 
    CPU_C15_CLK[0] = '\0'; 
    CPU_C15_VID[0] = '\0'; 
    CPU_USAGE[0] = '\0'; 
    CPU_VCORE[0] = '\0'; 
    CPU_VSOC[0] = '\0';
    CPU_POWER[0] = '\0';
    SOC_POWER[0] = '\0';
    CPU_TOTAL_POWER[0] = '\0';
    GPU_POWER[0] = '\0';
    GPU_TEMP[0] = '\0'; 
    GPU_CLK[0] = '\0'; 
    VRAM_CLK[0] = '\0'; 
    GPU_FAN[0] = '\0'; 
    GPU_FPS[0] = '\0'; 
    GPU_USAGE[0] = '\0'; 
    GPU_VCORE[0] = '\0'; 
    RAM_USED[0] = '\0'; 
    RAM_FREE[0] = '\0'; 
    PAGE_FILE_USAGE[0] = '\0'; 
    VRAM_USED[0] = '\0'; 
    RAM_CLK[0] = '\0'; 
    RAM_CONTROLLER_CLK[0] = '\0'; 
    UP_SPEED[0] = '\0'; 
    DW_SPEED[0] = '\0';
    DAY[0] = '\0';
    MONTH[0] = '\0';
    YEAR[0] = '\0';
    HOUR[0] = '\0';
    MINUTE[0] = '\0';
    SECOND[0] = '\0';
    GMT[0] = '\0';
    return;
}