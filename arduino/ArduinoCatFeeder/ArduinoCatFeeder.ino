// ArduinoCatFeed
// by Juan-Manuel Fluxa <mfluxa@gmail.com


#include <Servo.h> 
#include <Wire.h>
#include <DS1307new.h>
#include <LiquidCrystal.h>

//servo vars
Servo servoFood;
Servo servoPan;
Servo servoTilt;
int servoFoodPin = 13;
int servoPanPin = 6;
int servoTiltPin = 5;
int servoPWMMin = 600;
int servoPWMMax = 2400;
const int foodOpen = 180;
const int foodClosed = 0;
const int servoPTIni = 90;
int waitTimeForClosing = 5000; //5 secs
unsigned long timeSinceChangedLCD = 0;

//clock vars
uint16_t startAddr = 0x0000;  // Start address to store in the NV-RAM
uint16_t lastAddr;            // new address for storing in NV-RAM

String feedTime = "07:00";     //format "00:00"
String feedReset = "07:01";
boolean fedToday;

//LCD vars
LiquidCrystal lcd(12, 11, 10, 16, 7, 8, 9, 2, 3, 4, 14); //14 = Analog 0 | 16 = Analog 2
boolean showCurrent = false;


void setup() 
{ 
  //servo setup
  servoFood.attach(servoFoodPin,servoPWMMin,servoPWMMax);
  servoFood.write(foodClosed);    //starts servo at 0
  
  servoPan.attach(servoPanPin,500,2500);
  servoPan.write(servoPTIni);
  
  servoTilt.attach(servoTiltPin,500,2500);
  servoTilt.write(servoPTIni);
  
  
  //clock setup
  RTC.setRAM(0, (uint8_t *)&startAddr, sizeof(uint16_t));    // Store startAddr in NV-RAM address 0x08
  RTC.ctrl = 0x00;    // 0x00=disable SQW pin, 0x10=1Hz, 0x11=4096Hz, 0x12=8192Hz, 0x13=32768Hz
  RTC.setCTRL();
  
  //LCD setup
  lcd.begin(8,2);
  
  Serial.begin(9600);
  
} 
 
//DECLARE METHODS 
void feed()
{
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("now...");
  lcd.setCursor(0,1);
  lcd.print("feeding");
  
  servoFood.write(foodOpen);
  delay(waitTimeForClosing);
  servoFood.write(foodClosed);
  //delay(waitTimeForClosing);
  //servoFood.write(foodOpen);
  //delay(waitTimeForClosing);
  //servoFood.write(foodClosed);
  
}

String getNow()
{
  RTC.getTime();
  String stringNow = "";
  if (RTC.hour < 10)
  {
    stringNow += "0";
  } 
  stringNow += String(RTC.hour, DEC);
  stringNow += ":";
  
  if (RTC.minute < 10)
  {
    stringNow += "0";
  }
  stringNow += String(RTC.minute, DEC);
  
  //Serial.print(stringNow);
  
  return stringNow;
}

String getNowFull()
{
  RTC.getTime();
  String stringNow = "";
  if (RTC.hour < 10)
  {
    stringNow += "0";
  } 
  stringNow += String(RTC.hour, DEC);
  stringNow += ":";
  
  if (RTC.minute < 10)
  {
    stringNow += "0";
  }
  
  stringNow += String(RTC.minute, DEC);
  stringNow += String(":");
  
  if (RTC.second < 10)
  {
    stringNow += "0";
  }
  
  stringNow += String(RTC.second, DEC);
  
  return stringNow;
}

void printLCD()
{
  if(showCurrent)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("  now");
    lcd.setCursor(0,1);
    lcd.print(getNowFull());
  }
  else
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("foodtime");
    lcd.setCursor(0,1);
    lcd.print(" " + feedTime);
  }
  
  showCurrent = !showCurrent;
}

//COMMANDS
char cmdTilt = 'T';
char cmdPan = 'P';
char cmdFood = 'F';
char cmdPanLeft = 'L';
char cmdPanRight = 'R';
char cmdTiltUp = 'U';
char cmdTiltDown = 'D';

const int maxPan = 160;
const int maxTilt = 160;
const int minVal = 0;

int currentTilt = servoPTIni;
int currentPan = servoPTIni;

// protocol: cmd -> 1 byte | arg -> 3 bytes | '\n' -> 1byte = 5 bytes
boolean processcmd = false;
boolean parsecmd;
char cmd;
char arg[4];
int cmdArg;

void processCommand() {
  if(cmd == cmdTilt) {
    currentTilt = cmdArg;
    if(currentTilt > maxTilt) {
      currentTilt = maxTilt;
    }
    servoTilt.write(currentTilt);
    
  } else if(cmd == cmdPan) {
    currentPan = cmdArg;
    if(currentPan > maxPan) {
      currentPan = maxPan;
    }
    servoPan.write(currentPan);
  } else if(cmd == cmdFood) {
    servoFood.write(cmdArg);
  } else if(cmd == cmdPanLeft) {
    currentPan -= cmdArg;
    if(currentPan < minVal) {
      currentPan = minVal;
    }
    servoPan.write(currentPan);
  } else if(cmd == cmdPanRight) {
    currentPan += cmdArg;
    if(currentPan > maxPan) {
      currentPan = maxPan;
    }
    servoPan.write(currentPan);
  } else if(cmd == cmdTiltUp) {
    currentTilt += cmdArg;
    if(currentTilt > maxTilt) {
      currentTilt = maxTilt;
    }
    servoTilt.write(currentTilt);
  } else if(cmd == cmdTiltDown) {
    currentTilt -= cmdArg;
    if(currentTilt < minVal) {
      currentTilt = minVal;
    }
    servoTilt.write(currentTilt);
  }
}

// Serial instruction

void serialEvent() {
  
  if( Serial.available() >= 5) {
    cmd = Serial.read();
    arg[0] = (char)Serial.read();
    arg[1] = (char)Serial.read();
    arg[2] = (char)Serial.read();
    arg[3] = '\0';
    if(Serial.read() == '\n') {
      parsecmd = true;
    }
  }
  
  if(parsecmd) {
    parsecmd = false;
    Serial.print("Command: ");
    Serial.print(cmd);
    Serial.print(" | ARG: ");
    cmdArg = atoi(arg);
    Serial.print(cmdArg);
    Serial.print('\n');
    processCommand();
    cmd = '\0';
    arg[0] = '\0';
  }
  
}



//LOOP
void loop() 
{
  RTC.getTime();
  String stringNow = getNow();
  
  if(fedToday)
  {
    if(stringNow == feedReset)
    {
      fedToday = false;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("see you");
      lcd.setCursor(0,1);
      lcd.print("tomorrow");
      delay(2000);
    }
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("cat food");
    lcd.setCursor(0,1);
    lcd.print("ready!");
    delay(2000);
  }
  else
  {
   //check time and feed 
   if(stringNow == feedTime)
   {
     fedToday = true;
     feed();
   } 
  }
  
  if(millis() - timeSinceChangedLCD > 3000) {
    printLCD();
    timeSinceChangedLCD = millis();
  }
    
  //delay(5000);
}
