#include <SPI.h>
#include <SD.h>
#include <SoftwareSerial.h>

const int chipSelect = 4;
SoftwareSerial HC06(A1, A2); // A1: RX, A2: TX

#define LED1_R 3
#define LED1_G 5
#define LED2_R 6
#define LED2_G 9 

#define BUTTON1 2
#define BUTTON2 7

String first = "";
String second = "";
int targetLED = 0; 

int led1State = 0; // 0: 꺼짐, 1: 빨강, 2: 깜빡임, 3: 초록
int led2State = 0;

unsigned long previousMillis = 0;
const long interval = 300;
bool blinkState = false;
bool sdCardConnected = false;

void setup() {
  Serial.begin(9600);
  HC06.begin(9600); 
  
  pinMode(LED1_R, OUTPUT);
  pinMode(LED1_G, OUTPUT);
  pinMode(LED2_R, OUTPUT);
  pinMode(LED2_G, OUTPUT);
  
  pinMode(BUTTON1, INPUT_PULLUP);
  pinMode(BUTTON2, INPUT_PULLUP);

  analogWrite(LED1_R, 0); analogWrite(LED1_G, 0);
  analogWrite(LED2_R, 0); analogWrite(LED2_G, 0);
}

void loop() {
  if (!sdCardConnected) {
    if (SD.begin(chipSelect)) {
      sdCardConnected = true;
      led1State = 1;
      led2State = 1;

      File myFile = SD.open("setup.txt");
      if (myFile) {
        int lineCount = 1;
        while (myFile.available()) {
          String line = myFile.readStringUntil('\n');
          line.trim(); 
          
          if (line.length() > 0) {
            if (lineCount == 1) first = line;
            else if (lineCount == 2) second = line;
            lineCount++;
          }
        }
        myFile.close();
      }
    } else {
      led1State = 0; led2State = 0; targetLED = 0;
      return; 
    }
  } else {
    if (!SD.exists("setup.txt")) { 
      sdCardConnected = false;
      first = ""; second = "";
      led1State = 0; led2State = 0; targetLED = 0;
      SD.end(); 
      return;
    }
  }

  if (HC06.available()) {
    String btSignal = HC06.readStringUntil('\n');
    btSignal.trim();

    if (btSignal.length() > 0) {
      if (btSignal == "20715") {
        if (first == "20715") { led1State = 2; targetLED = 1; }
        else if (second == "20715") { led2State = 2; targetLED = 2; }
      } 
      else if (btSignal == "20923") {
        if (first == "20923") { led1State = 2; targetLED = 1; }
        else if (second == "20923") { led2State = 2; targetLED = 2; }
      }
    }
  }

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    blinkState = !blinkState;
  }

  bool btn1Pressed = !digitalRead(BUTTON1);
  bool btn2Pressed = !digitalRead(BUTTON2);

  if (btn1Pressed && (led1State == 1 || led1State == 2)) {
    led1State = 3;
  } else if (!btn1Pressed && led1State == 3) {
    led1State = 1;
    targetLED = 0;
  }
  
  if (btn2Pressed && (led2State == 1 || led2State == 2)) {
    led2State = 3;
  } else if (!btn2Pressed && led2State == 3) {
    led2State = 1;
    targetLED = 0;
  }

  if (led1State == 1) { analogWrite(LED1_R, 254); analogWrite(LED1_G, 0); }
  else if (led1State == 2) { analogWrite(LED1_R, blinkState ? 254 : 0); analogWrite(LED1_G, 0); }
  else if (led1State == 3) { analogWrite(LED1_R, 0); analogWrite(LED1_G, 254); }
  else { analogWrite(LED1_R, 0); analogWrite(LED1_G, 0); }

  if (led2State == 1) { analogWrite(LED2_R, 254); analogWrite(LED2_G, 0); }
  else if (led2State == 2) { analogWrite(LED2_R, blinkState ? 254 : 0); analogWrite(LED2_G, 0); }
  else if (led2State == 3) { analogWrite(LED2_R, 0); analogWrite(LED2_G, 254); }
  else { analogWrite(LED2_R, 0); analogWrite(LED2_G, 0); }
}
