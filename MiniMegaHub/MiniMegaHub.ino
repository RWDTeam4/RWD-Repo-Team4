#include <math.h>
#include <Servo.h>

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

#define DBLE 120 //Dead Band Lower End
#define DBUE 135 //Dead Band Upper End
#define STICK_CENTER 127.5 //Dead Center of Stick Values

unsigned long commandTimer = 0;
unsigned long currentTime = 0;

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

boolean mcmReady = false;
boolean rfReady = false;
boolean acReady = false;

//Servo & AC Variables
int ACMotor_MAX = 180;
int Limit_MAX = 255;
int Limit_MIN = 0;
int Stepping = 300;

int DAC_OUT1 = 2;
int SERVO_OUT1 = 4;

int ADC_PWM = 255;

Servo servo1;

boolean acSpinUp = false;
boolean engageClutch = false;

String ack = "ACK";

void setup() {

  servo1.attach(SERVO_OUT1,550,2370);
  
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
  CMD_Readme();
  MASTER_MODULE_SERIAL.println("Starting Servo...."); 
  #endif
  
  
  pinMode(DAC_OUT1, OUTPUT);
  DAC_Conversion(255,DAC_OUT1);
  delay(1000);
  servoInitialization();
  
  
  delay(5000);

  resetMotorCommand();
  
  MOTOR_CONTROL_SERIAL.println(ack);
  BLUETOOTH_SERIAL.println(ack);
}

void loop() 
{
  currentTime = micros();
  // put your main code here, to run repeatedly:
  if(currentTime - commandTimer >= 5000000){
    resetMotorCommand();
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
          
          if ((moduleNumber <5) && (moduleNumber >0)){
              MASTER_MODULE_SERIAL.print("Command OK");        
              ModuleCMD(moduleNumber, MCmd);
          }
          else
          {
            MASTER_MODULE_SERIAL.println("Module Channel Error!!!!");            
            CMD_Readme();            
          }
         
      }
    }
}

void ModuleCMD(int MCH, String MCMD){
  //char buffer[MCMD.length()+1];
  //sprint(buffer, MCMD);
  switch(MCH){
    case(1):
    {
      MASTER_MODULE_SERIAL.print("\n [Serial1]: ");
      MASTER_MODULE_SERIAL.println(MCMD);
      Serial1.write(MCMD.c_str());
      
      break;
    }
    case(2):
    {
      MASTER_MODULE_SERIAL.print("\n [Serial2]: ");
      MASTER_MODULE_SERIAL.println(MCMD);
      Serial2.write(MCMD.c_str());
      break;
    }
    case(3):
    {
      MASTER_MODULE_SERIAL.print("\n [Serial3]: ");
      MASTER_MODULE_SERIAL.println(MCMD);
      Serial3.write(MCMD.c_str());
      break;
    }
    case(4):
    {
      ADC_PWM = MCMD.toInt();
      if(ADC_PWM < 256 && ADC_PWM > -1){
        DAC_Conversion(ADC_PWM, DAC_OUT1);
      } else {
        MASTER_MODULE_SERIAL.println("AC Motor Input Value Out Of Range!");
      }
    }
    default:
    {
      MASTER_MODULE_SERIAL.println("Module Selection Out Of Range!");
      break;
    }
  }
}


void CMD_Readme(){
  MASTER_MODULE_SERIAL.println("");
  MASTER_MODULE_SERIAL.println("Module 1 = Weapon Module");
  MASTER_MODULE_SERIAL.println("Module 2 = Motor Module");
  MASTER_MODULE_SERIAL.println("Module 3 = RF Module");
  MASTER_MODULE_SERIAL.println("Module 4 = AC Clutch Spinup");
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

  float motorIntensityRaw = (rxValue / 127.0) * 255.0;
  motorIntensityRaw = motorIntensityRaw > 255.0 ? 255.0 : motorIntensityRaw; //Cap this value
  String motorIntensityString = calcMotorValueToHex(motorIntensityRaw);

  motorCommand = leftFrontState + motorIntensityString + leftRearState + motorIntensityString;
  motorCommand += rightFrontState + motorIntensityString + rightRearState + motorIntensityString;
  
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

  motorCommand = leftFrontState + calcMotorValueToHex(leftFront);
  motorCommand += leftRearState + calcMotorValueToHex(leftRear);
  motorCommand += rightFrontState + calcMotorValueToHex(rightFront);
  motorCommand += rightRearState + calcMotorValueToHex(rightRear);

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
      steeringInt = charToHex(steering[j])<<(dcp + 2 - j);
      movementInt = charToHex(movement[j])<<(dcp + 2 - j);
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

void resetMotorCommand(){
  lxValue = lyValue = rxValue = ryValue = STICK_CENTER;
  lxReady = lyReady = rxReady = ryReady = false;
  commandTimer = micros();
  String stopMotors = "S00S00S00S00";
  MOTOR_CONTROL_SERIAL.print(stopMotors);
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
//    if(!acSpinUp){
//      acSpinUp = true;
//      DAC_Conversion(ADC_PWM, DAC_OUT1);
//    } else {
//      acSpinUp = false;
//      DAC_Conversion(ADC_PWM, DAC_OUT1);
//    }
  }

  if(input == "@TT"){
    // engageDogClutch
  }
  if(input == "@R1"){
    //Increase ADC_PWM value and cut off at upper limit
    ADC_PWM = ADC_PWM + 10 <= ACMotor_MAX ? ADC_PWM + 10 : ACMotor_MAX;
  }
  if(input == "@L1"){
    //Decrease ADC_PWM value and cut off at lower limit
    ADC_PWM = ADC_PWM - 10 <= 0 ? ADC_PWM - 10 : 0;
  }

  if(lxReady && lyReady && rxReady && ryReady){
    commandTimer = micros();
    String driveCommand = createDriveCommand(lyValue, lxValue, ryValue, rxValue);
    MOTOR_CONTROL_SERIAL.print(driveCommand);
    #ifdef DEBUG_ROBOT
    MASTER_MODULE_SERIAL.println(driveCommand);
    #endif
    lxReady = false;
    lyReady = false;
    rxReady = false;
    ryReady = false;
  }
}

void RAMP_UP (int NOW, int MAX,  int PIN){
  ADC_PWM = 10;
  DAC_Conversion(10, DAC_OUT1);
}

void RAMP_DOWN (int NOW, int MIN,  int PIN){
  for(NOW; NOW > MIN; NOW++){
    delay(100);
    DAC_Conversion(NOW, DAC_OUT1);
    Serial.println(NOW);  
  }
  ADC_PWM = MIN;
}

void servoInitialization(){
  #ifdef DEBUG_ROBOT
  MASTER_MODULE_SERIAL.println("Initialization Of DAC Unit Started....");
  #endif
  DAC_Conversion(255,DAC_OUT1);
  Servo_Action(0);
  delay(200);
  ADC_PWM = 0;
  DAC_Conversion(ADC_PWM, DAC_OUT1);
  #ifdef DEBUG_ROBOT
  MASTER_MODULE_SERIAL.println("DAC Unit READY!!!");
  #endif
}

void DAC_Conversion(int PWM, int PIN){
  analogWrite(PIN, PWM);
  Servo_Action(PWM);
}

void Servo_Action(int PWMValue){
  int val = map(PWMValue, Limit_MIN, Limit_MAX,0,180);
  servo1.write(val);
}


