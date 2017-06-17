  char incoming[12] = "";
  byte charCount = 0;
  byte motorCount = 0;
  byte motorDirection = B00000000;
  long command = 0x0;
  boolean enable = true;
  unsigned long currentTime;
  static byte commandLength = 12; //Total Length of command
  static byte subCommandLength = 3;
  boolean commandReadComplete = false;
  boolean commandReady = false;

  unsigned long timerM1;
  unsigned long timerM2;
  unsigned long timerM3;
  unsigned long timerM4;

  unsigned short dutyLimit = 254; //Actual max value allowed
  unsigned short dutyAmountM1 = 0;
  unsigned short dutyAmountM2 = 0;
  unsigned short dutyAmountM3 = 0;
  unsigned short dutyAmountM4 = 0;

  boolean drivingM1 = false;
  boolean drivingM2 = false;
  boolean drivingM3 = false;
  boolean drivingM4 = false;

//  boolean forwardM1 = true;
//  boolean forwardM2 = true;
//  boolean forwardM3 = true;
//  boolean forwardM4 = true;
  
void setup()
{
  Serial.begin(57600);
  Serial.println("<Arduino is ready>");

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
//  pinMode(A3, OUTPUT);//Motor 4 Ourput Direction => Analog Pin 3

  writeCommand();

  timerM1 = micros();
  timerM2 = micros();
  timerM3 = micros();
  timerM4 = micros();

  digitalWrite(6, LOW);
  digitalWrite(9, LOW);
  digitalWrite(10, LOW);
  digitalWrite(11, LOW);

  digitalWrite(A0, LOW);
  digitalWrite(A1, LOW);
  digitalWrite(A2, LOW);
  digitalWrite(A3, LOW);

  Serial.println("Setup Finished");

  charCount = 0;
}

void loop()
{
//  delay(100);
  if(enable){
    currentTime = micros();
    if(commandReadComplete){
      commandReadComplete = false;
      commandReady = false;
      motorCount = 0;
      charCount = 0;
    }
    commandReady = charCount >= commandLength;
    boolean pulseChangeM1 = currentTime - timerM1 >= dutyAmountM1;
    boolean pulseChangeM2 = currentTime - timerM2 >= dutyAmountM2;
    boolean pulseChangeM3 = currentTime - timerM3 >= dutyAmountM3;
    boolean pulseChangeM4 = currentTime - timerM4 >= dutyAmountM4;
    if(Serial.available() && !commandReady){
      incoming[charCount] = Serial.read();
      charCount++;
    }
    if(commandReady && !commandReadComplete){
      unsigned int dutyAmount = 0;
      byte directionChar = motorCount*subCommandLength;
      command = incoming[directionChar];
      for(int i = directionChar+1; i< directionChar+subCommandLength; i++){
        dutyAmount += charToHex(incoming[i])<<(((directionChar+2)-i)*4);
      }

      if(dutyAmount>dutyLimit){
        dutyAmount = dutyLimit;
      }
      switch(motorCount){
        case 0:
          dutyAmountM1 = dutyAmount;
//          Serial.print("Motor 1: ");
//          Serial.println(dutyAmountM1);
          break;
        case 1:
          dutyAmountM2 = dutyAmount;
//          Serial.print("Motor 2: ");
//          Serial.println(dutyAmountM2);
          break;
        case 2:
          dutyAmountM3 = dutyAmount;
//          Serial.print("Motor 3: ");
//          Serial.println(dutyAmountM3);
          break;
        case 3:
          dutyAmountM4 = dutyAmount;
//          Serial.print("Motor 4: ");
//          Serial.println(dutyAmountM4);
          commandReadComplete = true;
          break;
      }
      motorCount++;
    }
    
      
    switch(command<<motorCount){
      case 0x44: //D
      case 0x64: //d
      case 0x88:
      case 0xc8:
      case 0x110:
      case 0x190:
      case 0x220:
      case 0x320:
        drivingM1 = drivingM2 = drivingM3 = drivingM4 = enable = false;
        command=0x0;
        PORTD &= ~B01000000; //Pin 6 Low
        PORTB &= ~B00001110; //Pins 9,10,11 Low
        motorDirection &= B00000000;
        break;

      case 0x46: //F Motor 1
      case 0x66: //f Motor 1
        timerM1 = micros();
        PORTD &= ~B01000000;
        motorDirection |= B00000001;
        motorDirection &= ~B00000010;
        pulseChangeM1 = false; // Set to false to prevent instant forward to reverse
        drivingM1 = true;
        command = 0x0;
        break;
      case 0x8c : //F Motor 2
      case 0xcc : //f Motor 2
      	timerM2 = micros();
      	PORTB &= ~B00000010;
        motorDirection |= B00000100;
        motorDirection &= ~B00001000;
      	pulseChangeM2 = false;
      	drivingM2 = true;
      	command = 0x0;
      	break;
      case 0x118 : //F Motor 3
      case 0x198 : //f Motor 3
      	timerM3 = micros();
      	PORTB &= ~B00000100;
        motorDirection |= B00100000;
        motorDirection &= ~B00010000;
      	pulseChangeM3 = false;
      	drivingM3 = true;
      	command = 0x0;
      case 0x230 : //F Motor 4
      case 0x330 : //f Motor 4
      	timerM4 = micros();
      	PORTB &= ~B00001000;
        motorDirection |= B10000000;
        motorDirection &= ~B01000000;
      	pulseChangeM4 = false;
      	drivingM4 = true;
      	command = 0x0;
      	break;

      case 0x52: //R Motor 1
      case 0x72: //r Motor 1
        //Reverse Stuff
        timerM1 = micros();
        PORTD &= ~B01000000;
        motorDirection |= B00000010;
        motorDirection &= ~B00000001;
        pulseChangeM1 = false; // Set to false to prevent instant forward to reverse
        drivingM1 = true;
        command = 0x0;
        break;
      case 0xa4: //R Motor 2
      case 0xe4: //r Motor 2
        //Reverse Stuff
        timerM2 = micros();
        PORTB &= ~B00000010;
        motorDirection |= B00001000;
        motorDirection &= ~B00000100;
        pulseChangeM2 = false;
        drivingM2 = true;
        command = 0x0;
        break;
      case 0x148: //R Motor 3
      case 0x1c8: //r Motor 2
        //Reverse Stuff
        timerM3 = micros();
        PORTB &= ~B00000100;
        motorDirection |= B00010000;
        motorDirection &= ~B00100000;
        pulseChangeM3 = false;
        drivingM3 = true;
        command = 0x0;
        break;
      case 0x290: //R Motor 4
      case 0x390: //r Motor 2
        timerM4 = micros();
        PORTB &= ~B00001000;
        motorDirection |= B01000000;
        motorDirection &= ~B10000000;
        pulseChangeM4 = false;
        drivingM4 = true;
        command = 0x0;
        break;

      case 0x53: //S  Stops the motor (turns all ports to digital low)
      case 0x73: //s
        PORTD &= ~B01000000;
        motorDirection &= B11111100;
        drivingM1 = false;
        break;
      case 0xa6: //S  Stops the motor (turns all ports to digital low)
      case 0xe6: //s
        PORTD &= ~B01000000;
        motorDirection &= B11110011;
        drivingM2 = false;
        break;
      case 0x14c: //S  Stops the motor (turns all ports to digital low)
      case 0x1cc: //s
        PORTD &= ~B01000000;
        motorDirection &= B11001111;
        drivingM3 = false;
        break;
      case 0x298: //S  Stops the motor (turns all ports to digital low)
      case 0x398: //s
        PORTD &= ~B01000000;
        motorDirection &= B00111111;
        drivingM4 = false;
        break;
      default: //Unhandled Command
        break; 
    }
    
	  if(pulseChangeM1 && drivingM1){
	    dutyAmountM1 = 255 - dutyAmountM1;
	    PORTD ^= B01000000; 
	    timerM1 = micros();
	  }
	  if(pulseChangeM2 && drivingM2){
	    dutyAmountM2 = 255 - dutyAmountM2;
	    PORTB ^= B00000010;
	    timerM2 = micros();
	  }
	  if(pulseChangeM3 && drivingM3){
	    dutyAmountM3 = 255 - dutyAmountM3;
	    PORTB ^= B00000100; 
	    timerM3 = micros();
	  }
	  if(pulseChangeM4 && drivingM4){
	    dutyAmountM4 = 255 - dutyAmountM4;
	    PORTB ^= B00001000; 
	    timerM4 = micros();
	  }

    writeCommand();
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
  PORTC |= B00000010; //Pin A1 Low
}

