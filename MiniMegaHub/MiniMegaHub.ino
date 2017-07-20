#include <math.h>
#include <Servo.h>
#include <Adafruit_NeoPixel.h>

#ifdef __AVR__
  #include <avr/power.h>
#endif

//Extra Unit Circle values used in judging boundry calculations from input
#ifndef M_PI_6
  #define M_PI_6 0.52359877559829887307
#endif

#ifndef M_PI_3
  #define M_PI_3 1.04719755119659774615
#endif

#ifndef M_PI_3_4
  #define M_PI_3_4 2.35619449019234492884
#endif

#ifndef M_PI_2_3
  #define M_PI_2_3 2.09439510239319549230
#endif

#ifndef M_PI_5_6
  #define M_PI_5_6 2.61799387799149436538
#endif

//Quick way to replace what Serial channel a module is using
#define MASTER_MODULE_SERIAL Serial
#define MOTOR_CONTROL_SERIAL Serial1
#define BLUETOOTH_SERIAL Serial2
#define NOT_USED Serial3

//Comment out line below to remove debug statements (will speed up processing overall)
#define DEBUG_ROBOT 1
//Comment out line below for test platform
#define PROD 1

#define DBLE 120 //Dead Band Lower End
#define DBUE 135 //Dead Band Upper End
#define STICK_CENTER 127.5 //Dead Center of Stick Values

//Timers
unsigned long commandTimer = 0;
unsigned long currentTime = 0;
unsigned long ledTimer = 0;
unsigned long servoTimer = 0;
unsigned long electroMagnetUse = 0;
unsigned long electroMagnetStartTime = 0;
unsigned long electroMagnetEndTime = 0;

//Timeouts
static unsigned long motorCommandTimeout = 5000000; // 5 seconds
static unsigned long ledTimeout = 50000; // 0.05 seconds
static unsigned long servoTimeout = 50000; // 0.05 seconds
static unsigned long electroMagnetTimeout = 30000000;

//Bluetooth To Motor Control Variables
float lyValue = 0;
float lxValue = 0;
float ryValue = 0;
float rxValue = 0;

boolean commandReady = false;
boolean lxReady = false;
boolean lyReady = false;
boolean rxReady = false;
boolean ryReady = false;

//Modules Ready
boolean mcmReady = false;
boolean rfReady = false;
boolean acReady = false;

//Servo & AC Variables
static byte AC_MOTOR_MAX_PWM = 128;
static byte AC_MOTOR_MIN_PWM = 0;
static byte LIMIT_MAX = 255;
static byte LIMIT_MIN = 0;
static byte SERVO_LIMIT = 90;

//Pins 
static byte DAC_PIN = 2;
static byte SERVO_PIN = 4;
static byte LED_PIN = 20;
static byte ELECTROMAGNET_PIN = 21; 

int ADC_PWM = 0;

Servo dogClutchServo;

boolean acSpinUp = false;
boolean servoTriggered = false;
boolean electroMagnetDisabled = false;
boolean electroMagnetOn = false;

String ack = "ACK";

//LED(s)
static byte NUMBER_OF_LEDS = 3;
int rStart = 127;
int gStart = 127;
int bStart = 127;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMBER_OF_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

void setup() {  
  // put your setup code here, to run once:
  MASTER_MODULE_SERIAL.begin(57600);
  Serial1.begin(57600); 
  Serial2.begin(57600); 
  Serial3.begin(57600); 
  Serial1.setTimeout(100);
  Serial2.setTimeout(100);
  Serial3.setTimeout(100);
  
  #ifdef DEBUG_ROBOT
    MASTER_MODULE_SERIAL.println("?????????");
    commandReadMe();
    MASTER_MODULE_SERIAL.println("Starting Servo...."); 
  #endif

  dogClutchServo.attach(SERVO_PIN,550,2370);
  
  pinMode(LED_PIN, OUTPUT);
  pinMode(ELECTROMAGNET_PIN, OUTPUT);
  pinMode(DAC_PIN, OUTPUT);

  digitalWrite(ELECTROMAGNET_PIN, HIGH);

  acInitialization();
  delay(1000);
  servoInitialization();
  pixels.begin();
  delay(5000);

  resetMotorCommand();
  
  MOTOR_CONTROL_SERIAL.println(ack);
  BLUETOOTH_SERIAL.println(ack);
}

void loop() {
  currentTime = micros();
  // put your main code here, to run repeatedly:

  if(electroMagnetOn && !electroMagnetDisabled){
    if((currentTime - electroMagnetStartTime + electroMagnetUse) >= electroMagnetTimeout){
      shutdownElectroMagnet();
      electroMagnetDisabled = true;
    }
  }

  if(servoTriggered){
    if(currentTime - servoTimer >= servoTimeout){
      servoReset();
    }
  }
  
  if(currentTime - commandTimer >= motorCommandTimeout){
    MOTOR_CONTROL_SERIAL.println(resetMotorCommand());
  }

  if(currentTime - ledTimer >= ledTimeout){
    for(int i=0;i<NUMBER_OF_LEDS;i++){
      // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
      pixels.setPixelColor(i, pixels.Color((rStart+5)%255,(gStart + 10)%255,(bStart + 20)%255)); 
      pixels.show();
    }
    ledTimer = micros();
  }
  
  #ifdef DEBUG_ROBOT
    if (MASTER_MODULE_SERIAL.available()){
      readCMD();
    }
  #endif
  
  if(MOTOR_CONTROL_SERIAL.available()){
    String mcmInput = MOTOR_CONTROL_SERIAL.readStringUntil('\r\n');
    if(mcmInput.indexOf("DISABLED") >= 0){
      mcmReady = false;
    }
    if(mcmReady){
      #ifdef DEBUG_ROBOT
        MASTER_MODULE_SERIAL.println(mcmInput);
      #endif
    } else {
      if(mcmInput.indexOf(ack) >= 0){
        #ifdef DEBUG_ROBOT
          MASTER_MODULE_SERIAL.println("Motor Control Module ACK Recieved");
        #endif
        mcmReady = true; 
      } else {
       MOTOR_CONTROL_SERIAL.println(ack);
       #ifdef DEBUG_ROBOT
        MASTER_MODULE_SERIAL.println(mcmInput);
       #endif
      }
    }
  }
  
  if (BLUETOOTH_SERIAL.available()){
    if(rfReady){
      readPS3Command();
    } else {
      String rfInput = BLUETOOTH_SERIAL.readStringUntil('\r\n');
      if(rfInput.indexOf(ack) >= 0){
        #ifdef DEBUG_ROBOT
          MASTER_MODULE_SERIAL.println("Bluetooth Module ACK Recieved");
        #endif
        rfReady = true;
      } else {
        BLUETOOTH_SERIAL.println(ack);
        #ifdef DEBUG_ROBOT
          MASTER_MODULE_SERIAL.println(rfInput);
        #endif
      }
    }
  }
}

#ifdef DEBUG_ROBOT
void readCMD(){ 
  MASTER_MODULE_SERIAL.println("\n Serial Command Ready!");
  String ModuleN = "";
  int moduleNumber = -1;      
  String MCmd = "";

  String inputCMD = MASTER_MODULE_SERIAL.readStringUntil('\n');
  MASTER_MODULE_SERIAL.println(inputCMD);
  for (int i =0; i< inputCMD.length(); i++){
    if (inputCMD.charAt(i) == '>'){
      for (int j = 0;j < i;j++){
        ModuleN+=inputCMD[j];
      }
      moduleNumber = ModuleN.toInt();
      for (int j = i+1;j<inputCMD.length()+1; j++){
        MCmd+=inputCMD[j];
      }
      
      MASTER_MODULE_SERIAL.print("\n [Module]: ");
      MASTER_MODULE_SERIAL.print(ModuleN);        
      MASTER_MODULE_SERIAL.print("\n [Command]: ");
      MASTER_MODULE_SERIAL.print(MCmd);
      MASTER_MODULE_SERIAL.print("]: ");
        
      if ((moduleNumber <6) && (moduleNumber >0)){
          MASTER_MODULE_SERIAL.print("Command OK");        
          ModuleCMD(moduleNumber, MCmd);
      }else{
        MASTER_MODULE_SERIAL.println("Module Channel Error!!!!");            
        commandReadMe();            
      }
    }
  }
}

void ModuleCMD(int MCH, String MCMD){
  switch(MCH){
    case 1:{
      MASTER_MODULE_SERIAL.print("\n [Serial1]: ");
      MASTER_MODULE_SERIAL.println(MCMD);
      Serial1.write(MCMD.c_str());
      break;
    } case 2: {
      MASTER_MODULE_SERIAL.print("\n [Serial2]: ");
      MASTER_MODULE_SERIAL.println(MCMD);
      Serial2.write(MCMD.c_str());
      break;
    } case 3: {
      MASTER_MODULE_SERIAL.print("\n [Serial3]: ");
      MASTER_MODULE_SERIAL.println(MCMD);
      Serial3.write(MCMD.c_str());
      break;
    } case 4: {
      MASTER_MODULE_SERIAL.print("\n [AC Motor]: ");
      MASTER_MODULE_SERIAL.println(MCMD);
      int pwmInput = MCMD.toInt();
      if(pwmInput < AC_MOTOR_MAX_PWM && pwmInput > -1){
        analogWrite(DAC_PIN, ADC_PWM);
      } else {
        MASTER_MODULE_SERIAL.println("AC Motor Input Value Out Of Allowable Range!");
      }
      break;
    } case 5: {
      MASTER_MODULE_SERIAL.print("\n [Servo Motor]: ");
      int servoPwm = MCMD.toInt();
      servoAction(servoPwm);
      break;
    } default: {
      MASTER_MODULE_SERIAL.println("Module Selection Out Of Range!");
      break;
    }
  }
}

//Print Out to User so that they know what inputs are possible
void commandReadMe(){
  MASTER_MODULE_SERIAL.println("");
  MASTER_MODULE_SERIAL.println("Module 1 = Weapon Module");
  MASTER_MODULE_SERIAL.println("Module 2 = Motor Module");
  MASTER_MODULE_SERIAL.println("Module 3 = RF Module");
  MASTER_MODULE_SERIAL.println("Module 4 = AC Clutch Spinup");
  MASTER_MODULE_SERIAL.println("Module 5 = Servo Motor Position");
  MASTER_MODULE_SERIAL.println("Input format: Module>Command ");
  MASTER_MODULE_SERIAL.println("example= 2>0:50");
}

#endif


/**
 *  Using Joystick values, creates the command to be sent to the Motor Control Module
 */
String createDriveCommand(int lyValue, int lxValue, int ryValue, int rxValue){
  String motorCommand = "";
  String steeringCommand = "";
  String movementCommand = "";

  steeringCommand = calculateSteering(rxValue, ryValue);
  movementCommand = calculateMovement(lxValue, lyValue);

  motorCommand = mixSteeringAndMovement(movementCommand, steeringCommand);
  return motorCommand;
}

/**
 *  Only uses the x axis value to judge whether you're stearing left or right
 */
String calculateSteering(float rxValue, float ryValue){
  String motorCommand = "";
  char leftFrontState = 'S';
  char rightFrontState = 'S';
  char leftRearState = 'S';
  char rightRearState = 'S';
  if(rxValue > STICK_CENTER){
    leftFrontState = 'F';
    leftRearState = 'F';
    rightFrontState = 'R';
    rightRearState = 'R';
  } 
  if(rxValue < STICK_CENTER){
    leftFrontState = 'R';
    leftRearState = 'R';
    rightFrontState = 'F';
    rightRearState = 'F';
  }

  float motorIntensityRaw = abs((rxValue-STICK_CENTER) / STICK_CENTER) * 255.0;
  motorIntensityRaw = motorIntensityRaw > 255.0 ? 255.0 : motorIntensityRaw; //Cap this value
  String motorIntensityString = calcMotorValueToHex(motorIntensityRaw);

  #ifndef PROD
  motorCommand = leftFrontState + motorIntensityString + leftRearState + motorIntensityString;
  motorCommand += rightFrontState + motorIntensityString + rightRearState + motorIntensityString;
  #endif
  #ifdef PROD
  motorCommand = leftFrontState + motorIntensityString + rightFrontState + motorIntensityString;
  motorCommand += leftRearState + motorIntensityString + rightRearState + motorIntensityString;
  #endif
  return motorCommand;
}

/**
 *  Basic Mecanum movement, derives motor values based on angles and directional ma
 */
String calculateMovement(float xVal, float yVal){
  
  String motorCommand = "";

  float inY = STICK_CENTER - yVal;
  float inX = xVal - STICK_CENTER;
  inY = inY >= STICK_CENTER ? STICK_CENTER : inY;
  inX = inX >= STICK_CENTER ? STICK_CENTER : inX;
  inY = inY <= -STICK_CENTER ? -STICK_CENTER : inY;
  inX = inX <= -STICK_CENTER ? -STICK_CENTER : inX;
  float magnitude = sqrt(sq(inY) + sq(inX));
  float myArcSin = asin(inY/magnitude);
  float myArcCosine = acos(inX/magnitude);
  char leftFrontState = 'S';
  char rightFrontState = 'S';
  char leftRearState = 'S';
  char rightRearState = 'S';
  float lfScaleFactor = 1;
  float rfScaleFactor = 1;
  float lrScaleFactor = 1;
  float rrScaleFactor = 1;
  float leftFront = 0;
  float rightFront = 0;
  float leftRear = 0;
  float rightRear = 0;

  //Set Motor Direction
  if(myArcCosine > M_PI_4 && myArcCosine < M_PI_3_4 && myArcSin > M_PI_4){
    //All Motors Forward
    leftFrontState  = 'F';
    rightFrontState = 'F';
    leftRearState   = 'F';
    rightRearState  = 'F';
  }
  if(myArcCosine >  M_PI_4 && myArcCosine < M_PI_3_4 && myArcSin < -M_PI_4){
      //All Motors Reverse
      leftFrontState  = 'R';
      rightFrontState = 'R';
      leftRearState   = 'R';
      rightRearState  = 'R';
  }
  if(myArcSin > -M_PI_4 && myArcSin < M_PI_4 && myArcCosine > M_PI_3_4){
      //Left  Front Forward
      //Right Front Revers
      //Left  Rear  Reverse
      //Right Rear  Forward
      leftFrontState  = 'F';
      rightFrontState = 'R';
      leftRearState   = 'R';
      rightRearState  = 'F';
  }
  if(myArcSin > -M_PI_4 && myArcSin < M_PI_4 && myArcCosine < M_PI_4){
      //Left Front Reverse
      //Right Front Forward
      //Left Rear Forward
      //Right Rear Reverse
      leftFrontState  = 'R';
      rightFrontState = 'F';
      leftRearState   = 'F';
      rightRearState  = 'R';
  }
  if(myArcCosine > M_PI_6 && myArcCosine < M_PI_3 && myArcSin < M_PI_3 && myArcSin > M_PI_6){
    //Quadrant 1
    rfScaleFactor = myArcCosine > M_PI_4 ?  sin(1.5*(myArcCosine - M_PI_4)): sin(-1.5*(myArcCosine - M_PI_4));
  }
  if(myArcCosine > M_PI_2_3 && myArcCosine < M_PI_5_6 && myArcSin < M_PI_3 && myArcSin > M_PI_6){
    //Quadrant 2
    lfScaleFactor = myArcCosine > M_PI_3_4 ? sin(1.5*(myArcCosine - M_PI_3_4)): sin(-1.5*(myArcCosine - M_PI_3_4));
  }
  if(myArcCosine > M_PI_2_3 && myArcCosine < M_PI_5_6 && myArcSin > -M_PI_3 && myArcSin < -M_PI_6){
    //Quadrant 3
    rfScaleFactor = myArcCosine > M_PI_3_4 ? sin(1.5*(myArcCosine - M_PI_3_4)): sin(-1.5*(myArcCosine - M_PI_3_4));
  }
  if(myArcCosine > M_PI_6 && myArcCosine < M_PI_3 && myArcSin > -M_PI_3 && myArcSin < -M_PI_6){
    //Quadrant 4
    lfScaleFactor = myArcCosine > M_PI_4 ? sin(1.5*(myArcCosine - M_PI_4)): sin(-1.5*(myArcCosine - M_PI_4));
  }
  //Set Motor Speed
  //Calculate Base motor speeds then convert to strings to be used
  leftFront = lfScaleFactor*magnitude;
  rightFront = rfScaleFactor*magnitude;
  leftRear = rfScaleFactor*magnitude;
  rightRear = lfScaleFactor*magnitude;

  #ifdef PROD
  motorCommand = leftFrontState + calcMotorValueToHex(leftFront);
  motorCommand += leftRearState + calcMotorValueToHex(leftRear);
  motorCommand += rightFrontState + calcMotorValueToHex(rightFront);
  motorCommand += rightRearState + calcMotorValueToHex(rightRear);
  #endif
  #ifndef PROD
  motorCommand = leftFrontState + calcMotorValueToHex(leftFront);
  motorCommand += rightFrontState + calcMotorValueToHex(rightFront);
  motorCommand += leftRearState + calcMotorValueToHex(leftRear);
  motorCommand += rightRearState + calcMotorValueToHex(rightRear);
  #endif

  return motorCommand;
}

/**
 *  Simple averaging of the steering joystick and the movement joystick commands
 */
String mixSteeringAndMovement(String steeringCommand, String movementCommand){
  String finishedMotorCommand = "";
  char steering[steeringCommand.length()];
  char movement[movementCommand.length()];
  steeringCommand.toCharArray(steering, sizeof steering);
  movementCommand.toCharArray(movement, sizeof movement);
  for(int i =0; i<4; i++ ){
    byte dcp = i*3; //direction char position
    int steeringInt = 0;
    int movementInt = 0;
    int steeringModifier = directionModifier(steering[dcp]);
    int movementModifier = directionModifier(movement[dcp]);
    
    for(int j = dcp+1; j<= dcp+2; j++){
      steeringInt += charToHex(steering[j])<<((dcp + 2 - j)*4);
      movementInt += charToHex(movement[j])<<((dcp + 2 - j)*4);
    }
    char newDirection = 'S';
    float newMotorValue = (steeringModifier*steeringInt+movementModifier*movementInt)/2.0;
    if(newMotorValue > 0){
      newDirection = 'F';
    }
    if(newMotorValue < 0 ){
      newDirection = 'R';
    }

    finishedMotorCommand += newDirection + calcMotorValueToHex(newMotorValue);
  }

  return finishedMotorCommand;
}

int directionModifier(char dir){
  switch(dir){
    case 0x46: //F => Forward
      return 1;
    case 0x52: //R => Reverse
      return -1;
    case 0x53: //S => Stop
    default:   //Any garbage that tries to get in
      return 0;
  }
}

//ConvertFloat into two character hex.
String calcMotorValueToHex(float raw){
  int rawInt = (int) ((raw/STICK_CENTER)*255.0);
  rawInt = rawInt > 255 ? 255 : rawInt;
  String motorValInHex = String(rawInt, HEX);
  if(rawInt <= 0xF){
    motorValInHex = "0" + motorValInHex;
  }
  return motorValInHex;
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

String resetMotorCommand(){
  lxValue = lyValue = rxValue = ryValue = STICK_CENTER;
  lxReady = lyReady = rxReady = ryReady = false;
  commandTimer = micros();
  String stopMotors = "S00S00S00S00";
  return stopMotors;
}

void readPS3Command(){
  String serialResponse = BLUETOOTH_SERIAL.readStringUntil('\r\n');
  byte delimiter = serialResponse.indexOf(":");

  String input = serialResponse.substring(0,delimiter);
  String inputValue = serialResponse.substring(delimiter+1);
  float value = inputValue.toFloat();

  //Required Dead Band Check
  if(value >= DBLE && value <= DBUE){
    value = STICK_CENTER;
  }
  
  //Only Processing the commands for the sticks
  if(input == "@LY"){
     lyValue = value;
     lyReady = true;
  }
  if( input == "@LX"){
    lxValue = value;
    lxReady = true;
  } 
  if( input == "@RY"){
    ryValue = value;
    ryReady = true;
  }
  if( input == "@RX"){
    rxValue = value;
    rxReady = true;
  }

  if( input == "@SS" ){
    if(!acSpinUp){
      acSpinUp = true;
      analogWrite(DAC_PIN,AC_MOTOR_MAX_PWM);
    } else {
      acSpinUp = false;
      analogWrite(DAC_PIN,AC_MOTOR_MIN_PWM);
    }
  }

  if(input == "@TT"){
    servoTrigger();
  }
  if(!electroMagnetDisabled){
    if(input == "@R1"){
      powerElectroMagnet();
    }
  
    if(input == "@L1"){
      shutdownElectroMagnet();
    }
  }


  if(lxReady && lyReady && rxReady && ryReady){
    commandTimer = micros();
    String driveCommand = "";
    boolean leftStickInDeadband = lyValue > DBLE && lyValue < DBUE && lxValue > DBLE && lxValue < DBUE;
    boolean rightStickInDeadband = ryValue > DBLE && ryValue < DBUE && rxValue > DBLE && rxValue < DBUE;
    if( !leftStickInDeadband && !rightStickInDeadband ){
      driveCommand = createDriveCommand(lyValue, lxValue, ryValue, rxValue); 
    }else if( leftStickInDeadband && !rightStickInDeadband){
      driveCommand = calculateSteering(rxValue, ryValue);
    }else if( !leftStickInDeadband && rightStickInDeadband) {
      driveCommand = calculateMovement(lxValue, lyValue);
    }else{
      driveCommand = resetMotorCommand();
    }
    MOTOR_CONTROL_SERIAL.println(driveCommand);
    #ifdef DEBUG_ROBOT
    MASTER_MODULE_SERIAL.println(driveCommand);
    #endif
    lxReady = false;
    lyReady = false;
    rxReady = false;
    ryReady = false;
  }
}

void servoInitialization(){
  #ifdef DEBUG_ROBOT
  MASTER_MODULE_SERIAL.println("Initialization Of Servo Unit Started....");
  #endif
  servoAction(0);
  delay(200);
  servoAction(LIMIT_MAX);
  delay(200);
  servoAction(0);
  #ifdef DEBUG_ROBOT
  MASTER_MODULE_SERIAL.println("Servo Unit READY!!!");
  #endif
}

void acInitialization(){
  #ifdef DEBUG_ROBOT
  MASTER_MODULE_SERIAL.println("Initialization Of AC Unit Started....");
  #endif
  analogWrite(DAC_PIN, AC_MOTOR_MAX_PWM);
  delay(200);
  analogWrite(DAC_PIN, AC_MOTOR_MIN_PWM);
  #ifdef DEBUG_ROBOT
  MASTER_MODULE_SERIAL.println("AC Unit Ready.");
  #endif
}

//Maps LIMIT_MIN <-> LIMIT_MAX to Angle value for the servo up to SERVO_LIMIT
void servoAction(int pwmValue){
  int mappedValue = map(pwmValue, LIMIT_MIN, LIMIT_MAX,0,SERVO_LIMIT);
  dogClutchServo.write(mappedValue);
}

void servoTrigger(){
  servoAction(LIMIT_MAX);
  servoTimer = micros();
  servoTriggered = true;
}

void servoReset(){
  servoTriggered = false;
  servoAction(LIMIT_MIN);
}

void powerElectroMagnet(){
  electroMagnetOn = true;
  digitalWrite(ELECTROMAGNET_PIN, LOW);
  electroMagnetStartTime = micros();
}

void shutdownElectroMagnet(){
  digitalWrite(ELECTROMAGNET_PIN, HIGH);
  electroMagnetEndTime = micros();
  electroMagnetUse += (electroMagnetEndTime - electroMagnetStartTime);
  electroMagnetOn = false;
}
