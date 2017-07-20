/*
 Example sketch for the PS3 Bluetooth library - developed by Kristian Lauszus
 For more information visit my blog: http://blog.tkjelectronics.dk/ or
 send me an e-mail:  kristianl@tkjelectronics.com
 */

#include <PS3BT.h>
#include <usbhub.h>

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#include <SPI.h>
#endif

//#define DBLE 122 //Dead Band Lower End
//#define DBUE 132 //Dead Band Upper End
USB Usb;
//USBHub Hub1(&Usb); // Some dongles have a hub inside

BTD Btd(&Usb); // You have to create the Bluetooth Dongle instance like so
/* You can create the instance of the class in two ways */
// PS3BT PS3(&Btd); // This will just create the instance
// PS3BT PS3(&Btd, 0x00, 0x1A, 0x7D, 0xDA, 0x71, 0x13); // This will also store the bluetooth address - this can be obtained from the dongle when running the sketch
// PS3BT PS3(&Btd, 0xE0, 0xAE, 0x5E, 0x29, 0x8F, 0x51); // This will also store the bluetooth address - this can be obtained from the dongle when running the sketch
PS3BT PS3(&Btd, 0x00, 0x1A, 0x7D, 0xDA, 0x71, 0x11); //CHINA CRAP CONTROLLER (Black PS3 controller, lightweight)

bool printTemperature, printAngle;
// static String endDelimiter = "\r\n";
unsigned long timer = 0;
void setup() {
  Serial.begin(57600);
  Serial.setTimeout(100);
#if !defined(__MIPSEL__)
  while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif
  if (Usb.Init() == -1) {
    Serial.println(F("OSC did not start"));
    // Serial.print(endDelimiter);
    while (1); //halt
  }
  Serial.println(F("PS3 Bluetooth Library Started"));
  // Serial.print(endDelimiter);
  delay(100);
  handshake();
  timer = millis();
}
void loop() {
  Usb.Task();

  if(millis() - timer >= 25){
    if (PS3.PS3Connected || PS3.PS3NavigationConnected) 
    {
      //Always want both joystick values, ALWAYS
//      if (PS3.getAnalogHat(LeftHatX) > DBUE || PS3.getAnalogHat(LeftHatX) < DBLE || PS3.getAnalogHat(LeftHatY) > DBUE || PS3.getAnalogHat(LeftHatY) < DBLE || PS3.getAnalogHat(RightHatX) > DBUE || PS3.getAnalogHat(RightHatX) < DBLE || PS3.getAnalogHat(RightHatY) > DBUE || PS3.getAnalogHat(RightHatY) < DBLE) 
//      {
        Serial.print(F("@LX:")); //LeftHatX
        Serial.println(PS3.getAnalogHat(LeftHatX));
        // Serial.print(endDelimiter);
        Serial.print(F("@LY:")); //LeftHatY
        Serial.println(PS3.getAnalogHat(LeftHatY));
        // Serial.print(endDelimiter);
        // if (PS3.PS3Connected) 
        // { // The Navigation controller only have one joystick
        Serial.print(F("@RX:")); //RightHatX
        Serial.println(PS3.getAnalogHat(RightHatX));
        // Serial.print(endDelimiter);
        Serial.print(F("@RY:")); //RightHatY
        Serial.println(PS3.getAnalogHat(RightHatY));
        // Serial.print(endDelimiter);
        // }
//      }
  
      // Analog button values can be read from almost all buttons
      if (PS3.getAnalogButton(L2) || PS3.getAnalogButton(R2)) 
      {
        Serial.print(F("@L2:"));
        Serial.println(PS3.getAnalogButton(L2));
        // Serial.print(endDelimiter);
        // Serial.println();
        if (PS3.PS3Connected) 
        {
          Serial.print(F("@R2:"));
          Serial.println(PS3.getAnalogButton(R2));
          // Serial.print(endDelimiter);
        }
      }
  
      if (PS3.getButtonClick(PS)) 
      {
        Serial.println(F("@PS:255")); //PS
        // Serial.print(endDelimiter);
        PS3.disconnect();
      }
      else 
      {
        if (PS3.getButtonClick(TRIANGLE)) {
          Serial.println(F("@TT:255")); //Traingle
          // Serial.print(endDelimiter);
          PS3.setRumbleOn(RumbleLow);
        }
        if (PS3.getButtonClick(CIRCLE)) {
          Serial.println(F("@OO:255")); //Circle
          // Serial.print(endDelimiter);
          PS3.setRumbleOn(RumbleHigh);
        }
        if (PS3.getButtonClick(CROSS))
          Serial.println(F("@XX:255")); //Cross
          // Serial.print(endDelimiter);
        if (PS3.getButtonClick(SQUARE))
          Serial.println(F("@SS:255")); //Square
          // Serial.print(endDelimiter);
  
        if (PS3.getButtonClick(UP)) 
        {
          Serial.println(F("@UP:255")); //Up
          // Serial.print(endDelimiter);
          if (PS3.PS3Connected) {
            PS3.setLedOff();
            PS3.setLedOn(LED4);
          }
        }
        if (PS3.getButtonClick(RIGHT)) {
          Serial.println(F("@RT:255")); //Right
          // Serial.print(endDelimiter);
          if (PS3.PS3Connected) {
            PS3.setLedOff();
            PS3.setLedOn(LED1);
          }
        }
        if (PS3.getButtonClick(DOWN)) {
          Serial.println(F("@DN:255")); //Down
          // Serial.print(endDelimiter);
          if (PS3.PS3Connected) {
            PS3.setLedOff();
            PS3.setLedOn(LED2);
          }
        }
        if (PS3.getButtonClick(LEFT)) {
          Serial.println(F("@LT:255")); //Left
          // Serial.print(endDelimiter);
          if (PS3.PS3Connected) {
            PS3.setLedOff();
            PS3.setLedOn(LED3);
          }
        }
  
        if (PS3.getButtonClick(L1))
          Serial.println(F("@L1:255"));
          // Serial.print(endDelimiter);
        if (PS3.getButtonClick(L3))
          Serial.println(F("@L3:255"));
          // Serial.print(endDelimiter);
        if (PS3.getButtonClick(R1))
          Serial.println(F("@R1:255"));
          // Serial.print(endDelimiter);
        if (PS3.getButtonClick(R3))
          Serial.println(F("@R3:255"));
          // Serial.print(endDelimiter);
  
        if (PS3.getButtonClick(SELECT)) {
          Serial.print(F("@SE:255 - ")); //Select
          PS3.printStatusString();
          Serial.println();
        }
        if (PS3.getButtonClick(START)) {
          Serial.println(F("@ST:255")); //Start
          // Serial.print(endDelimiter);
          printAngle = !printAngle;
        }
      }
  //#if 1 // Set this to 1 in order to see the angle of the controller
      if (printAngle) 
      {
        Serial.print(F("Pitch: "));
        Serial.println(PS3.getAngle(Pitch));
  //      Serial.print(endDelimiter);
        Serial.print(F("Roll: "));
        Serial.println(PS3.getAngle(Roll));
  //      Serial.print(endDelimiter);
      }
  //#endif
    }
  #if 0 // Set this to 1 in order to enable support for the Playstation Move controller
    else if (PS3.PS3MoveConnected) {
      if (PS3.getAnalogButton(T)) {
        Serial.print(F("\r\nT: "));
        Serial.println(PS3.getAnalogButton(T));
      }
      if (PS3.getButtonClick(PS)) {
        Serial.print(F("\r\nPS"));
        PS3.disconnect();
      }
      else {
        if (PS3.getButtonClick(SELECT)) {
          Serial.print(F("\r\nSelect"));
          printTemperature = !printTemperature;
        }
        if (PS3.getButtonClick(START)) {
          Serial.print(F("\r\nStart"));
          printAngle = !printAngle;
        }
        if (PS3.getButtonClick(TRIANGLE)) {
          Serial.print(F("\r\nTriangle"));
          PS3.moveSetBulb(Red);
        }
        if (PS3.getButtonClick(CIRCLE)) {
          Serial.print(F("\r\nCircle"));
          PS3.moveSetBulb(Green);
        }
        if (PS3.getButtonClick(SQUARE)) {
          Serial.print(F("\r\nSquare"));
          PS3.moveSetBulb(Blue);
        }
        if (PS3.getButtonClick(CROSS)) {
          Serial.print(F("\r\nCross"));
          PS3.moveSetBulb(Yellow);
        }
        if (PS3.getButtonClick(MOVE)) {
          PS3.moveSetBulb(Off);
          Serial.print(F("\r\nMove"));
          Serial.print(F(" - "));
          PS3.printStatusString();
        }
      }
      if (printAngle) {
        Serial.print(F("\r\nPitch: "));
        Serial.println(PS3.getAngle(Pitch));
        Serial.print(F("\tRoll: "));
        Serial.println(PS3.getAngle(Roll));
      }
      else if (printTemperature) {
        Serial.print(F("\r\nTemperature: "));
        Serial.println(PS3.getTemperature());
      }
    }
  #endif
    timer = millis();
  }
}

void handshake(){
  Serial.println("RFM Beginning Handshake...");
  String input = "";
  while(input.indexOf("ACK") < 0){
    if(Serial.available()){
      input = Serial.readStringUntil('\r\n');
      delay(100);
    }
  }
  Serial.println("ACK");
  delay(10);
  Serial.println("RFM Handshake Successful");
}
