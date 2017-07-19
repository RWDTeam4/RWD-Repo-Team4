static String ack = "ACK";  


byte charCount = 0;
byte motorCount = 0;
byte motorDirection = B00000000;
long command = 0x0;
boolean enable = true;
unsigned long currentTime;
static byte commandLength = 15; //Total Length of command
static byte subCommandLength = 3;
boolean commandReady = false;

unsigned long timerM1;
unsigned long timerM2;
unsigned long timerM3;
unsigned long timerM4;
unsigned long autoTimeout;
static unsigned long timeout = 30000; //30 seconds

static unsigned short dutyLimit = 254; //Actual max value allowed
unsigned short dutyAmountM1 = 0;
unsigned short dutyAmountM2 = 0;
unsigned short dutyAmountM3 = 0;
unsigned short dutyAmountM4 = 0;

boolean drivingM1 = false;
boolean drivingM2 = false;
boolean drivingM3 = false;
boolean drivingM4 = false;
  
void setup() {
  Serial.begin(57600);
  initializeMcm();
  delay(100);
  handshake();
}

void loop(){
  currentTime = millis();
  if(currentTime - autoTimeout >= timeout) {
    disableMcm();
  }
  if(enable){
    char serialCommandCharArray[commandLength] = "";
    if(Serial.available()){
      String serialCommand = Serial.readStringUntil('\r\n');
      Serial.println(serialCommand);
      serialCommand.toCharArray(serialCommandCharArray, sizeof(serialCommandCharArray));
      commandReady = true;
      autoTimeout = millis();
    }
    if(commandReady){
      for(int i =0; i<4; i++ ){
        byte dcp = i*subCommandLength;
        command = serialCommandCharArray[dcp];
        int dutyAmount = 0;
        for(int j = dcp+1; j<= dcp+2; j++){
          dutyAmount += charToHex(serialCommandCharArray[j])<<(((dcp+2)-j)*4);
        }

        if(dutyAmount>dutyLimit){
          dutyAmount = dutyLimit;
        }
        switch(command<<i){
          case 0x44: //D
          case 0x64: //d
          case 0x88:
          case 0xc8:
          case 0x110:
          case 0x190:
          case 0x220:
          case 0x320:
            disableMcm();
            break;
    
          case 0x46: //F Motor 1
          case 0x66: //f Motor 1
            analogWrite(6, dutyAmount);
            motorDirection |= B00000001;
            motorDirection &= ~B00000010;
            break;
          case 0x8c : //F Motor 2
          case 0xcc : //f Motor 2
            analogWrite(9, dutyAmount);
            motorDirection |= B00000100;
            motorDirection &= ~B00001000;
          	break;
          case 0x118 : //F Motor 3
          case 0x198 : //f Motor 3
            analogWrite(10, dutyAmount);
            motorDirection |= B00100000;
            motorDirection &= ~B00010000;
          case 0x230 : //F Motor 4
          case 0x330 : //f Motor 4
            analogWrite(11, dutyAmount);
            motorDirection |= B10000000;
            motorDirection &= ~B01000000;
          	break;
    
          case 0x52: //R Motor 1
          case 0x72: //r Motor 1
            //Reverse Stuff
            analogWrite(6, dutyAmount);
            motorDirection |= B00000010;
            motorDirection &= ~B00000001;
            break;
          case 0xa4: //R Motor 2
          case 0xe4: //r Motor 2
            analogWrite(9, dutyAmount);
            motorDirection |= B00001000;
            motorDirection &= ~B00000100;
            break;
          case 0x148: //R Motor 3
          case 0x1c8: //r Motor 3
            analogWrite(10, dutyAmount);
            motorDirection |= B00010000;
            motorDirection &= ~B00100000;
            break;
          case 0x290: //R Motor 4
          case 0x390: //r Motor 4
            analogWrite(11, dutyAmount);
            motorDirection |= B01000000;
            motorDirection &= ~B10000000;
            break;
    
          case 0x53: //S  Stops the motor (turns all ports to digital low)
          case 0x73: //s
            analogWrite(6, 0);
            PORTD &= ~B01000000;
            motorDirection &= B11111100;
            break;
          case 0xa6: //S  Stops the motor (turns all ports to digital low)
          case 0xe6: //s
            analogWrite(9, 0);
            PORTD &= ~B01000000;
            motorDirection &= B11110011;
            break;
          case 0x14c: //S  Stops the motor (turns all ports to digital low)
          case 0x1cc: //s
            analogWrite(10, 0);
            PORTD &= ~B01000000;
            motorDirection &= B11001111;
            break;
          case 0x298: //S  Stops the motor (turns all ports to digital low)
          case 0x398: //s
            analogWrite(11, 0);
            PORTD &= ~B01000000;
            motorDirection &= B00111111;
            break;
          default: //Unhandled Command
            break; 
        }
      }
      writeCommand();
    }
  } else {
    //Go into blocking wait call in order to reset
    handshake();
    resetMCM();
  }
  
}

int charToHex(char c){
  if(c >= 0x30 && c <= 0x39){
    return c-0x30;
  }
  if(c>= 0x41 && c <= 0x46){
    return c - 0x37;
  }
  if(c >= 0x61 && c<= 0x66){
    return c-0x57;
  }
  return 0;
}

void writeCommand(){
  PORTC &= B11111101; //Pin A1 Low
  shiftOut(A0, A2, MSBFIRST, motorDirection); //Serial Pin, Clock Pin, Bit direction, Value
  PORTC |= B00000010; //Pin A1 High
}

void resetMCM(){
  motorCount = 0;
  dutyAmountM1 = 0;
  dutyAmountM2 = 0;
  dutyAmountM3 = 0;
  dutyAmountM4 = 0;
  enable = true;
  command = 0x0;
  
  commandReady = false;

  initializeMcm();
}

void initializeMcm(){
  Serial.println("Motor Control Module is ready");

  pinMode(2,  INPUT);//Motor 1 Encoder A => Digital Pin 2
  pinMode(3,  INPUT);//Motor 1 Encoder B => Digital Pin 3
  pinMode(4,  INPUT);//Motor 2 Encoder A => Digital Pin 4
  pinMode(5,  INPUT);//Motor 2 Encoder B => Digital Pin 5
  pinMode(7,  INPUT);//Motor 3 Encoder A => Digital Pin 7
  pinMode(8,  INPUT);//Motor 3 Encoder B => Digital Pin 8
  pinMode(12, INPUT);//Motor 4 Encoder A => Digital Pin 12
  pinMode(13, INPUT);//Motor 4 Encoder B => Digital Pin 13
  pinMode(6,  OUTPUT);//Motor 1 Output PWM => Digital Pin 6
  pinMode(9,  OUTPUT);//Motor 2 Output PWM => Digital Pin 9
  pinMode(10, OUTPUT);//Motor 3 Output PWM => Digital Pin 10
  pinMode(11, OUTPUT);//Motor 4 Output PWM => Digital Pin 11
  pinMode(A0, OUTPUT);//Serial Output Pin => Analog Pin 0
  pinMode(A1, OUTPUT);//Serial Clear Pin => Analog Pin 1
  pinMode(A2, OUTPUT);//Clock Pin for the output => Analog Pin 2

  writeCommand();

  timerM1 = millis();
  timerM2 = millis();
  timerM3 = millis();
  timerM4 = millis();
  autoTimeout = millis();

  digitalWrite(6, LOW);
  digitalWrite(9, LOW);
  digitalWrite(10, LOW);
  digitalWrite(11, LOW);

  digitalWrite(A0, LOW);
  digitalWrite(A1, LOW);
  digitalWrite(A2, LOW);

  Serial.println("Setup Finished for MCM");

  charCount = 0;
}

void handshake(){
  String input = "";
  Serial.println("MCM Beginning Handshake...");
  while(input.indexOf("ACK") < 0){
    if(Serial.available()){
      input = Serial.readStringUntil('\r\n');
      // Serial.println(input); //Debug Input is correctly read
      delay(100);
    }
  }
  Serial.println("ACK");
  delay(10);
  Serial.println("MCM Handshake Successful");
}

void disableMcm() {
  Serial.println("DISABLED:MCM");
  drivingM1 = drivingM2 = drivingM3 = drivingM4 = enable = false;
  command=0x0;
  PORTD &= ~B01000000; //Pin 6 Low
  PORTB &= ~B00001110; //Pins 9,10,11 Low
  motorDirection &= B00000000;  
}
