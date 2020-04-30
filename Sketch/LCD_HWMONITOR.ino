//Libraries
#include <LiquidCrystal.h>
#include <dht.h>
#include <IRremote.h>
#include <EEPROM.h>

//Pins
int IR_DIODE = 2;
int D7 = 3;
int D6 = 4;
int D5 = 5;
int LCD_BL = 6;
int D4 = 7;
int TEMP_HUMIDITY = 8;
int LCD_RW = 9;
int LCD_RS = 10;
int LED_DATAPIN = 11;
int LED_LATCHPIN = 12;
int LED_CLOCKPIN = 13;
int FWU_PIN = A0;

//LCD Screen Init
LiquidCrystal lcd(LCD_RS,LCD_RW,D4,D5,D6,D7); //4-bit Mode

//DHT11 Init
dht DHT;

//IR Diode Init
IRrecv IRRECEIVE(IR_DIODE);
decode_results IR_RESULT;

//Variables
unsigned long current_millis; //To wait without using delay()
int BL_BRIGHTNESS; //Brightness value (0-250)
int OLD_BL_BRIGHTNESS;
//IR Remote buttons
volatile bool IR_on_off = false;
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

//STATUS LEDS
int STATUS_LED = 0x00;
int CPU_LED = 0x00;
int GPU_LED = 0x00;

//Monitoring Variables
int CPU_TEMP = 0;
int CPU_FAN = 0;
int CPU_CLK = 0;
int CPU_USAGE = 0;
float CPU_VCORE = 0;
int GPU_TEMP = 0;
int GPU_CLK = 0;
int GPU_FAN = 0;
int GPU_FPS = 0;
float GPU_VCORE = 0;
unsigned int RAM_USED = 0;
unsigned int RAM_FREE = 0;
int ROOM_TEMP = 0;
int ROOM_HUMIDITY = 0;
//Selected Mode and Page tracking
int MODE;
int scroll_counter = 1;
int scroll_delay = 0;
//Serial Monitoring
const byte numChars = 255;
char receivedChars[numChars];
boolean newData = false;

//LCD Lines Arrays
char line0[17];
char line1[16];

int DHT_Counter;
//Setup
void setup()
{
//Pins I/O
pinMode(D7,OUTPUT);
pinMode(D6,OUTPUT);
pinMode(D5,OUTPUT);
pinMode(D4,OUTPUT);
pinMode(LCD_BL,OUTPUT);
pinMode(TEMP_HUMIDITY,INPUT);
pinMode(IR_DIODE,INPUT);
pinMode(LCD_RW,OUTPUT);
pinMode(LCD_RS,OUTPUT);
pinMode(LED_DATAPIN,OUTPUT);
pinMode(LED_LATCHPIN,OUTPUT);
pinMode(LED_CLOCKPIN,OUTPUT);
pinMode(FWU_PIN, INPUT);
IRRECEIVE.enableIRIn(); //IR Diode data reception enable
lcd.begin(16,2); //LCD Start
attachInterrupt(digitalPinToInterrupt(IR_DIODE), check_IR, CHANGE);//IR Interrupt
}

//loop
void loop() {
    standby();
}


//Main Functions

void standby() {
    check_FWU();
    scroll_counter = 1;
    scroll_delay = 0;
    //Serial OFF
    Serial.end();
    //LCD Clear and shutdown
    lcd.clear();
    analogWrite(LCD_BL,0);
    //STATUS_LED = RED, CPU and GPU LEDS OFF
    STATUS_LED = 0x01;
    CPU_LED = 0x00;
    GPU_LED = 0x00;
    update_cpanel();
    //Wait until ON/OFF Button is pushed
    while(!IR_on_off) {}
    welcome();    
}

void welcome() {
    EEPROM_READ();
    //LCD Startup, load brightness value stored in EEPROM
    analogWrite(LCD_BL, BL_BRIGHTNESS);
    //STATUS_LED = GREEN
    STATUS_LED = 0x02;
    update_cpanel();
    //Welcome Message
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("HARDWARE MONITOR");
    lcd.setCursor(0,1);
    lcd.print("MAJL 2DOMEL V1.0");
    //Show Splash for 3 seconds
    unsigned long current_millis = millis();
    while(millis() - current_millis <= 3000) {check_on_off();}
    //STATUS_LED = GREEN
    STATUS_LED = 0x02;
    update_cpanel();
    lcd.clear();
    IR_play = false;
    wait_serial();
    monitor();
}
void monitor() {
    //Clear IR button values
    IR_test = false;
    wait_to_refresh();
    //Enter selected mode
    if(MODE == 1) {
        lcd.setCursor(0,0); 
        lcd.setCursor(0,0);
        sprintf(line0, "CPU:%-3i" "C" "   %%:%-3i", CPU_TEMP,CPU_USAGE);
        lcd.print(line0);
        lcd.setCursor(0,1);
        sprintf(line1, "GPU:%-3i" "C" "  %-3iFPS", GPU_TEMP,GPU_FPS);
        lcd.print(line1);
    }
    else if (MODE == 2 || MODE == 3) {advanced_menu();}
monitor(); 
}

void advanced_menu() {
    switch(scroll_counter) {
        case 1:
            lcd.setCursor(0,0);
            sprintf(line0, "CPU:%-3i" "C" " SP:%-4i", CPU_TEMP, CPU_FAN);
            lcd.print(line0);
            lcd.setCursor(0,1);
            sprintf(line1, "CLK:%-4iM" " %%:%-3i", CPU_CLK,CPU_USAGE);
            lcd.print(line1);
            break;
        //POC
        case 2:
            lcd.setCursor(0,0);
            sprintf(line0, "GPU:%-3i" "C" " SP:%-4i", GPU_TEMP, GPU_FAN);
            lcd.print(line0);
            lcd.setCursor(0,1);
            sprintf(line1, "CLK:%-4iM" " %-3iFPS", GPU_CLK,GPU_FPS);
            lcd.print(line1);
            break;
        case 3:
            //Transformamos el valor de VCORE a String
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
        case 4:
            lcd.setCursor(0,0);
            sprintf(line0, "USED RAM:" "%-5u" "MB", RAM_USED);
            lcd.print(line0);
            lcd.setCursor(0,1);
            sprintf(line1, "FREE RAM:" "%-5u" "MB", RAM_FREE);
            lcd.print(line1);
            break;
        case 5:
            lcd.setCursor(0,0);
            sprintf(line0, "RoomTemp:" "%-3i" "\xDF" "C", int(DHT.temperature));
            lcd.print(line0);
            lcd.setCursor(0,1);
            sprintf(line1, "Humidity:" "%-3i" "%%", int(DHT.humidity));
            lcd.print(line1);
            break;
    }
    return;
}

void config() {
    //Disable CPU and GPU LEDS
    CPU_LED = 0x00;
    GPU_LED = 0x00;
    update_cpanel();
    //Show Settings splash for 3 seconds
    lcd.clear();
    lcd.setCursor(4,0);
    lcd.write("SETTINGS");
    unsigned long current_millis = millis();
    while(millis() - current_millis <= 2000) {check_on_off();}
    config_nosplash();
    IR_play = false;
    IR_forwards = false;
    IR_backwards = false;
    IR_back = false;
    return;
}
void config_nosplash() {
    //Clear IR buttons values
    IR_1 = false;
    IR_2 = false;
    IR_back = false;
    //Selection menu
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.write("(1) MONITOR MODE");
    lcd.setCursor(0,1);
    lcd.write("(2) BRIGHTNESS");
    //Wait until one of each button is pushed.
    while(!IR_1 && !IR_2 && !IR_back) {check_on_off();}
    if(IR_1) {modes_select();}
    else if(IR_2) {brightness_select(true);}
    else if(IR_back) {lcd.clear(); monitor();}
    EEPROM_UPDATE();
    return;
}

void modes_select() {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.write("(1)  (2)   (3)");
    lcd.setCursor(0,1);
    lcd.write("EASY ADV. MANUAL");
    //Clean IR buttons values, show options and wait until one of each is pushed.
    IR_1 = false;
    IR_2 = false;
    IR_3 = false;
    IR_back = false;
    while(!IR_1 && !IR_2 && !IR_3 && !IR_back) {check_on_off();}
     //Save desired mode value to EEPROM, then return
    if(IR_1) {
        MODE = 1;
    }
    else if(IR_2) {
        MODE = 2;
    }
    else if(IR_3) {
        MODE = 3;
    }
    else if (IR_back) {config_nosplash();}
    scroll_counter = 1;
    scroll_delay = 0;
    return;
}

void brightness_select(bool brightness) {
    if (brightness){lcd.clear();}
    int current_bl = BL_BRIGHTNESS / 25;
    lcd.setCursor(3,0);
    lcd.print("BRIGHTNESS");
    lcd.setCursor(1,1);
    lcd.print("-");
    lcd.setCursor(3,1);
    while(current_bl != 0) {
        lcd.print(char(255));
        current_bl--;
    }
    current_bl = BL_BRIGHTNESS / 25;
    lcd.setCursor(current_bl + 3,1);
    while(current_bl != 0) {
        lcd.print(" ");
        current_bl--;
    }
    lcd.setCursor(14,1);
    lcd.print("+");
    IR_play = false;
    IR_forwards = false;
    IR_backwards = false;
    IR_back = false;
    while(!IR_play && !IR_forwards && !IR_backwards && !IR_back) {check_on_off();}
    current_bl = BL_BRIGHTNESS / 25;
    if(IR_forwards && current_bl < 10) {
        BL_BRIGHTNESS += 25;
        analogWrite(LCD_BL,BL_BRIGHTNESS);
        brightness_select(false);
    }
    else if(IR_backwards && current_bl > 1) {
        BL_BRIGHTNESS -= 25;
        analogWrite(LCD_BL,BL_BRIGHTNESS);
        brightness_select(false);
    }
    else if(IR_play){IR_play = false; return;}
    else if(IR_back) {analogWrite(LCD_BL,OLD_BL_BRIGHTNESS); BL_BRIGHTNESS = OLD_BL_BRIGHTNESS; config_nosplash();}
    else {brightness_select(false);}
}

//Subfunctions

//IR codes receiver
void check_IR() {
    if(IRRECEIVE.decode(&IR_RESULT) == false) {} //No button pressed 
    else {
    switch(IR_RESULT.value) {
        case 0xFF6897: //BUTTON 0
        IR_0 = true;
        break;
        case 0xFF30CF: //BUTTON 1
        IR_1 = true;
        break;
        case 0xFF18E7: //BUTTON 2 
        IR_2 = true;
        break;
        case 0xFF7A85: //BUTTON 3
        IR_3 = true;
        break;
        case 0xFF10EF: //BUTTON 4
        IR_4 = true;;
        break;
        case 0xFF38C7: //BUTTON 5
        IR_5 = true;
        break;
        case 0xFF5AA5: //BUTTON 6
        IR_6 = true;
        break;
        case 0xFF42BD: //BUTTON 7
        IR_7 = true;
        break;
        case 0xFF4AB5: //BUTTON 8
        IR_8 = true;
        break;
        case 0xFF52AD: //BUTTON 9
        IR_9 = true;
        break;
        case 0xFFA25D: //BUTTON PWR
        IR_on_off = !IR_on_off;
        break;
        case 0xFFE21D: //BUTTON MENU
        IR_menu = true;
        break;
        case 0xFF22DD: //BUTTON TEST
        IR_test = true;
        break;
        case 0xFFC23D: //BUTTON RETURN
        IR_back = true;
        break;
        case 0xFF02FD: //BUTTON PLUS
        IR_plus = true;
        break;
        case 0xFF9867: //BUTTON MINUS
        IR_minus = true;
        break;
        case 0xFF906F: //BUTTON NEXT
        IR_forwards = true;
        break;
        case 0xFFE01F: //BUTTON BACK
        IR_backwards = true;
        break;
        case 0xFFA857: //BUTTON PLAY
        IR_play = !IR_play;
        break;
        case 0xFFB04F: //BUTTON CLEAR
        IR_clear = true;
        break;
    }
    //Clear IR Receive variable value (Avoid infinite loop)
    IR_RESULT.value = 0x000000;
    IRRECEIVE.resume(); //resume reading
    return; 
    }
}

void wait_to_refresh() {
    //Wait 1.2 Seconds until refresh
    unsigned long current_millis = millis();
    while(millis() - current_millis <=200) {
        check_on_off();
        //Get serial data
        get_serial();
        //If settings button is pushed, go to settings
        if(IR_test) {
            config();
        }
    }
    if(MODE == 2 && !IR_play) {
        scroll_delay++;
        if(scroll_delay == 10) {scroll_counter++;scroll_delay = 0;lcd.clear();}
        if(scroll_counter == 6) {scroll_counter = 1;lcd.clear();}
    }
    else if(MODE == 3 && (IR_backwards || IR_forwards)) {
        if (IR_backwards && scroll_counter > 1) {scroll_counter--;IR_backwards = false;lcd.clear();}
        if (IR_forwards && scroll_counter < 5) {scroll_counter++;IR_forwards = false;lcd.clear();}
    }
    //Get sensors data
    getsensors();
    //LED Updating
    if(CPU_TEMP <= 50) {CPU_LED = 0x02;}
    else if((CPU_TEMP > 50) && (CPU_TEMP < 80)) {CPU_LED = 0x03;}
    else if(CPU_TEMP >= 80) {CPU_LED = 0x01;}
    if(GPU_TEMP <= 50) {GPU_LED = 0x02;}
    else if((GPU_TEMP > 50) && (GPU_TEMP < 80)) {GPU_LED = 0x03;}
    else if(GPU_TEMP >= 80) {GPU_LED = 0x01;}
    update_cpanel();
    DHT_Counter++;
    return;
}

void getsensors() {
    // read DHT11 every 1.2s and judge the state according to the return value
    if(DHT_Counter == 5) {int chk = DHT.read11(TEMP_HUMIDITY); DHT_Counter = 0;}
    // Decode CHAR DATA
    char *array_table[20];
    int i = 0;
    array_table[i] = strtok(receivedChars,",");
    while(array_table[i] != NULL) {
        array_table[++i] = strtok(NULL,",");
    }
    RAM_USED = String(array_table[1]).toInt();
    RAM_FREE = String(array_table[3]).toInt();
    CPU_CLK = String(array_table[5]).toInt();
    CPU_USAGE = String(array_table[7]).toInt();
    CPU_TEMP = String(array_table[9]).toInt();
    CPU_VCORE = String(array_table[11]).toFloat();
    GPU_VCORE = String(array_table[13]).toFloat();
    GPU_TEMP = String(array_table[15]).toInt();
    GPU_CLK = String(array_table[17]).toInt();
    GPU_FPS = String(array_table[19]).toInt();
    return;
}

//Check if Power button is pressed, ON -> OFF, OFF -> ON
void check_on_off() {
    check_FWU();
    if(IR_on_off == false) {loop();}
    }

void check_FWU() {if(digitalRead(FWU_PIN) == LOW) {lcd.clear();FWU_MODE();}}    

void FWU_MODE() {
    //LCD Startup, load brightness value stored in EEPROM
    analogWrite(LCD_BL, 250);
     //STATUS_LED = YELLOW
    STATUS_LED = 0x03;
    CPU_LED = 0x03;
    GPU_LED = 0x03;
    update_cpanel();
    //Welcome Message
    lcd.setCursor(4,0);
    lcd.print("FWU MODE");
    lcd.setCursor(1,1);
    lcd.print("UPLOAD SKETCH");
    while(true) {
        //STATUS LED = YELLOW, BLINKS EVERY SECOND
        if(millis() - current_millis >= 1000) {
            switch (STATUS_LED) {
                case 0x00:
                STATUS_LED = 0x03;
                CPU_LED = 0x03;
                GPU_LED = 0x03;
                update_cpanel();
                current_millis = millis();
                break;

                case 0x03:
                STATUS_LED = 0x00;
                CPU_LED = 0x00;
                GPU_LED = 0x00;
                update_cpanel();
                current_millis = millis();
                break;
            }
        }
    };
    FWU_MODE();
}

//74HC595 (CPANEL) output
void update_cpanel() {
    byte CPANEL = STATUS_LED + (CPU_LED << 2) + (GPU_LED << 4);
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

void wait_serial() {
    scroll_delay = 0;
    Serial.begin(115200); //Serial Start
    //STATUS_LED = OFF
    CPU_LED = 0x00;
    GPU_LED = 0x00;
    STATUS_LED = 0x03;
    update_cpanel();
    //Print Waiting message
    lcd.clear();
    lcd.setCursor(2,0);
    lcd.print("WAITING FOR");
    lcd.setCursor(5,1);
    lcd.print("HELPER");
    unsigned long current_millis = millis();
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
            check_on_off();
            Serial.println("<WAITING FOR HELPER>");
        }
    }
    STATUS_LED = 0x02;
    update_cpanel();
    lcd.clear();
    lcd.setCursor(3,0);
    //Desync FIX
    lcd.print("CONNECTING");
    delay(500);
    monitor();
}

void get_serial() {;
    Serial.println("<WAITING>");
    check_on_off();
    while(!newData) {recvWithStartEndMarkers();check_on_off();}
    showNewData();
    delay(200);
    return;
}

void recvWithStartEndMarkers() {
    static boolean recvInProgress = false;
    static byte ndx = 0;
    char startMarker = '<';
    char endMarker = '>';
    char rc;
    int timeoutcounter = 0;
    while (Serial.available() == 0) {
        timeoutcounter++;
        if(timeoutcounter == 5000) {lcd.clear();timeout();}
    }
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
}

void showNewData() {
    if (newData == true) {
        newData = false;
    }
}

void timeout() {
    int counter = 5;
    STATUS_LED = 0x01;
    CPU_LED = 0x01;
    GPU_LED = 0x01;
    update_cpanel();
    unsigned long current_millis = millis();
    while(counter > 0) {
        check_on_off();
        lcd.setCursor(1,0);
        lcd.print("HELPER ERROR");
        lcd.setCursor(0,1);
        sprintf(line1, "RECONNECT IN:%i", counter);
        lcd.print(line1);
        if(millis() - current_millis >= 1000) {
            switch (STATUS_LED) {
                case 0x00:
                STATUS_LED = 0x01;
                CPU_LED = 0x01;
                GPU_LED = 0x01;
                update_cpanel();
                current_millis = millis();
                counter--;
                break;

                case 0x01:
                STATUS_LED = 0x00;
                CPU_LED = 0x00;
                GPU_LED = 0x00;
                update_cpanel();
                current_millis = millis();
                counter--;
                break;
            }
        }
    }
    wait_serial();
}

void EEPROM_READ() {
    //Brigthness is stored in Index 0
    BL_BRIGHTNESS = EEPROM.read(0);
    OLD_BL_BRIGHTNESS = BL_BRIGHTNESS;
    //Mode is stored in Index 1
    MODE = EEPROM.read(1);
    EEPROM_CHECK();
    return;    
}  

void EEPROM_CHECK() {
    //Checking if values are valid, if not rolling back to defaults
    if((BL_BRIGHTNESS == 0 || BL_BRIGHTNESS % 25 != 0) || (MODE < 1 || MODE > 3)) {
        BL_BRIGHTNESS = 250;
        EEPROM.update(0,BL_BRIGHTNESS);
        MODE = 1;
        EEPROM.update(1,MODE);
        analogWrite(LCD_BL, BL_BRIGHTNESS);
        lcd.setCursor(3,0);
        lcd.write("BAD CONFIG");
        lcd.setCursor(4,1);
        lcd.write("ROLLBACK");
        STATUS_LED = 0x01;
        update_cpanel();
        delay(3000);
        return;
    }
}

void EEPROM_UPDATE() {
    EEPROM.update(0,BL_BRIGHTNESS);
    EEPROM.update(1,MODE);
    OLD_BL_BRIGHTNESS = BL_BRIGHTNESS;
    lcd.clear();
    return;    
}