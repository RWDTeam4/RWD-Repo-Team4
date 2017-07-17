#include <math.h>

#ifndef M_PI_6
  #define M_PI_6 0.52359877559829887307
#endif

#ifndef M_PI_3
  #define M_PI_3 1.04719755119659774615
#endif

//Create M_PI_2_3, M_PI_3_4, M_PI_5_6

#ifndef M_PI_3_4
  #define M_PI_3_4 2.35619449019234492884
#endif

#ifndef M_PI_2_3
  #define M_PI_2_3 2.09439510239319549230
#endif

#ifndef M_PI_5_6
  #define M_PI_5_6 2.61799387799149436538
#endif

#define HubSerial Serial
#define McmSerial Serial1
#define RfSerial Serial2
#define AcSerial Serial3
#define DebugRobot 1

int lyValue = 0;
int lxValue = 0;
int ryValue = 0;
int rxValue = 0;
boolean commandReady = false;
boolean lxReady = false;
boolean lyReady = false;
boolean rxReady = false;
boolean ryReady = false;
boolean mcmReady = false;
boolean rfReady = false;
boolean acReady = false;

String ack = "ACK";

void setup() 
{
  // put your setup code here, to run once:
  HubSerial.begin(57600);
  Serial1.begin(57600); //AC
  Serial2.begin(57600); //MCM
  Serial3.begin(57600); //RF
  Serial1.setTimeout(100);
  Serial2.setTimeout(100);
  Serial3.setTimeout(100);
  #ifdef DebugRobot
  HubSerial.println("?????????");
  CMD_Readme();
  #endif
  delay(5000);
  McmSerial.println(ack);
  RfSerial.println(ack);
}

void loop() 
{
  // put your main code here, to run repeatedly:
  if (HubSerial.available()){
    readCMD();
  }
  if(McmSerial.available()){
    String mcmInput = McmSerial.readStringUntil('\r\n');
    if(mcmInput.indexOf("DISABLED") >= 0){
      mcmReady = false;
    }
    if(mcmReady){
      #ifdef DebugRobot
      HubSerial.println(mcmInput);
      #endif
    } else {
      if(mcmInput.indexOf(ack) >= 0){
        mcmReady = true;
      } else {
       #ifdef DebugRobot
       HubSerial.println(mcmInput);
       #endif
      }
    }
  }
  if (RfSerial.available()){
    if(rfReady){
      readPS3Command();
    } else {
      String rfInput = RfSerial.readStringUntil('\r\n');
      if(rfInput.indexOf(ack) >= 0){
        rfReady = true;
      } else {
      
        #ifdef DebugRobot
        HubSerial.println(rfInput);
        #endif
      }
    }
  }
}

void readCMD()
{ 
    HubSerial.println("\n Serial Command Ready!");
    String ModuleN = "";      
    String MCmd = "";

    String inputCMD = HubSerial.readStringUntil('\n');
    HubSerial.println(inputCMD);
    for (int i =0; i< inputCMD.length(); i++)
    {
      if (inputCMD.charAt(i) == '>')
      {

        for (int j = 0;j < i;j++)
        {

          ModuleN+=inputCMD[j];
        }
        for (int j = i+1;j<inputCMD.length()+1; j++)
        {
          MCmd+=inputCMD[j];
        }
          HubSerial.print("\n [Module]: ");
          HubSerial.print(ModuleN);        
          HubSerial.print("\n [Command]: ");
          HubSerial.print(MCmd);
          HubSerial.print("]: ");
          
          if ((ModuleN.toInt() <4) && (ModuleN.toInt() >0))
          {
            
              HubSerial.print("Command OK");        
              ModuleCMD(ModuleN.toInt(), MCmd);
          }
          else
          {
            HubSerial.println("Module Channel Error!!!!");            
            CMD_Readme();            
          }
         
      }
    }
}

void ModuleCMD(int MCH, String MCMD)
{
  //char buffer[MCMD.length()+1];
  //sprint(buffer, MCMD);
  switch(MCH)
  {
    case(1):
    {
      
      HubSerial.print("\n [Serial1]: ");
      HubSerial.println(MCMD);
      Serial1.write(MCMD.c_str());
      
      break;
    }
    case(2):
    {
      HubSerial.print("\n [Serial2]: ");
      HubSerial.println(MCMD);
      Serial2.write(MCMD.c_str());
      break;
    }
    case(3):
    {
      HubSerial.print("\n [Serial3]: ");
      HubSerial.println(MCMD);
      Serial3.write(MCMD.c_str());
      break;
    }
    default:
    {
      HubSerial.println("Module Selection Off Range!");
      break;
    }
  }
}


void CMD_Readme()
{
  HubSerial.println("");
  HubSerial.println("Module 1 = Weapon Module");
  HubSerial.println("Module 2 = Motor Module");
  HubSerial.println("Module 3 = RF Module");
  HubSerial.println("Input format: Module>Command ");
  HubSerial.println("example= 2>0:50");
}

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
String calculateSteering(int rxValue, int ryValue) {
  String motorCommand = "";
  char leftFrontState = 'S';
  char rightFrontState = 'S';
  char leftRearState = 'S';
  char rightRearState = 'S';
  if(rxValue > 127.5){
    leftFrontState = 'F';
    leftRearState = 'F';
    rightFrontState = 'R';
    rightRearState = 'R';
  } 
  if(rxValue < 127.5){
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
String calculateMovement(int lxValue, int lyValue){
  
  String motorCommand = "";

  float inLy = 127.5 - lyValue;
  float inLx = lxValue - 127.5;
  inLy = inLy >= 127.5 ? 127.5 : inLy;
  inLx = inLx >= 127.5 ? 127.5 : inLx;
  inLy = inLy <= -127.5 ? -127.5 : inLy;
  inLx = inLx <= -127.5 ? -127.5 : inLx;
  float magnitude = sqrt(sq(inLy) + sq(inLx));
  float myArcSin = asin(inLy/magnitude);
  float myArcCosine = acos(inLx/magnitude);
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
  if(myArcCosine >  M_PI_4 && myArcCosine < M_PI_3_4 && myArcSin < -1*M_PI_4){
      //All Motors Reverse
      leftFrontState  = 'R';
      rightFrontState = 'R';
      leftRearState   = 'R';
      rightRearState  = 'R';
  }
  if(myArcSin > -1*M_PI_4 && myArcSin < M_PI_4 && myArcCosine > M_PI_3_4){
      //Left  Front Forward
      //Right Front Revers
      //Left  Rear  Reverse
      //Right Rear  Forward
      leftFrontState  = 'F';
      rightFrontState = 'R';
      leftRearState   = 'R';
      rightRearState  = 'F';
  }
  if(myArcSin > -1*M_PI_4 && myArcSin < M_PI_4 && myArcCosine < M_PI_4){
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
  if(myArcCosine > M_PI_2_3 && myArcCosine < M_PI_5_6 && myArcSin > -1*M_PI_3 && myArcSin < -1*M_PI_6){
    //Quadrant 3
    rfScaleFactor = myArcCosine > M_PI_3_4 ? sin(1.5*(myArcCosine - M_PI_3_4)): sin(-1.5*(myArcCosine - M_PI_3_4));
  }
  if(myArcCosine > M_PI_6 && myArcCosine < M_PI_3 && myArcSin > -1*M_PI_3 && myArcSin < -1*M_PI_6){
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
    case 0x46: //F
      return 1;
    case 0x52: //R
      return -1;
    case 0x53: //S
    default:
      return 0;

  }
}

//ConvertFloat into two character hex.
String calcMotorValueToHex(float raw){
  int rawInt = (int) ((raw/127.0)*255.0);
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

void readPS3Command(){
  String serialResponse = RfSerial.readStringUntil('\r\n');
  byte delimiter = serialResponse.indexOf(":");

  String input = serialResponse.substring(0,delimiter);
  String inputValue = serialResponse.substring(delimiter+1);
  int value = inputValue.toInt();
  
  //Only Processing the commands for the sticks
  if(input == "@LY"){
     lyValue = value;
     lyReady = true;
  }
  if( input == "@LX") {
    lxValue = value;
    lxReady = true;
  } 
  if( input == "@RY") {
    ryValue = value;
    ryReady = true;
  }
  if( input == "@RX") {
    rxValue = value;
    rxReady = true;
  }

  if(lxReady && lyReady && rxReady && ryReady){
    String driveCommand = createDriveCommand(lyValue, lxValue, ryValue, rxValue);
    McmSerial.print(driveCommand);
    #ifdef DebugRobot
    HubSerial.println(driveCommand);
    #endif
    lxReady = false;
    lyReady = false;
    rxReady = false;
    ryReady = false;
  }
}

