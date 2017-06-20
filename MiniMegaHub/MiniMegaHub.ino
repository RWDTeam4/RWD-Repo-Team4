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


int lyValue = 0;
int lxValue = 0;
boolean commandReady = false;
boolean lxReady = false;
boolean lyReady = false;


void setup() 
{
  // put your setup code here, to run once:
  Serial.begin(57600);
  Serial1.begin(57600); //AC
  Serial2.begin(57600); //MCM
  Serial3.begin(57600); //RF
  Serial.println("?????????");
  CMD_Readme();
  
}

void loop() 
{
  // put your main code here, to run repeatedly:
  if (Serial.available()>0)
  {
    readCMD();
    // readPS3Command();
  }
  if (Serial1.available())
  {
    readPS3Command();
     // Serial.write(Serial1.read());
  }
  if (Serial2.available())
  {
     Serial.write(Serial2.read());
  }
  if (Serial3.available())
  {
     Serial.write(Serial3.read());
    
  }
}

void readCMD()
{ 
    Serial.println("\n Serial Command Ready!");
    String ModuleN = "";      
    String MCmd = "";

    String inputCMD = Serial.readStringUntil('\n');
    Serial.println(inputCMD);
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
          Serial.print("\n [Module]: ");
          Serial.print(ModuleN);        
          Serial.print("\n [Command]: ");
          Serial.print(MCmd);
          Serial.print("]: ");
          
          if ((ModuleN.toInt() <4) && (ModuleN.toInt() >0))
          {
            
              Serial.print("Command OK");        
              ModuleCMD(ModuleN.toInt(), MCmd);
          }
          else
          {
            Serial.println("Module Channel Error!!!!");            
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
      
      Serial.print("\n [Serial1]: ");
      Serial.println(MCMD);
      Serial1.write(MCMD.c_str());
      
      break;
    }
    case(2):
    {
      Serial.print("\n [Serial2]: ");
      Serial.println(MCMD);
      Serial2.write(MCMD.c_str());
      break;
    }
    case(3):
    {
      Serial.print("\n [Serial3]: ");
      Serial.println(MCMD);
      Serial3.write(MCMD.c_str());
      break;
    }
    default:
    {
      Serial.println("Module Selection Off Range!");
      break;
    }
  }
}


void CMD_Readme()
{
  Serial.println("");
  Serial.println("Module 1 = Weapon Module");
  Serial.println("Module 2 = Motor Module");
  Serial.println("Module 3 = RF Module");
  Serial.println("Input format: Module>Command ");
  Serial.println("example= 2>0:50");
}

String createDriveCommand(int yValue, int xValue){
  String motorCommand = "";
  //Angle and magnitude calculations for the individual wheels
  int inY = 127 - yValue;
  int inX = xValue - 127;
  inY = inY >= 128 ? 127 : inY;
  inX = inX >= 128 ? 127 : inX;
  inY = inY <= -128 ? -127 : inY;
  inX = inX <= -128 ? -127 : inX;
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
    // if(myArcCosine > M_PI_6 && myArcCosine < M_PI_3){
    //   //Quadrant 1
    //   rfScaleFactor = myArcCosine > M_PI_4 ?  sin(1.5*(myArcCosine - M_PI_4)): sin(-1.5*(myArcCosine - M_PI_4));
    // }
    // if(myArcCosine > M_PI_2_3 && myArcCosine < M_PI_5_6){
    //   //Quadrant 2
    //   lfScaleFactor = myArcCosine > M_PI_3_4 ? sin(1.5*(myArcCosine - M_PI_3_4)): sin(-1.5*(myArcCosine - M_PI_3_4));
    // }
  if(myArcCosine >  M_PI_4 && myArcCosine < M_PI_3_4 && myArcSin < -1*M_PI_4){
      //All Motors Reverse
      leftFrontState  = 'R';
      rightFrontState = 'R';
      leftRearState   = 'R';
      rightRearState  = 'R';
  }
    // if(myArcCosine > M_PI_6 && myArcCosine < M_PI_3){
    //   //Quadrant 4
    //   lfScaleFactor = myArcCosine > M_PI_4 ? sin(1.5*(myArcCosine - M_PI_4)): sin(-1.5*(myArcCosine - M_PI_4));
    // }
    // if(myArcCosine > M_PI_2_3 && myArcCosine < M_PI_5_6){
    //   //Quadrant 3
    //   lfScaleFactor = myArcCosine > M_PI_3_4 ? sin(1.5*(myArcCosine - M_PI_3_4)): sin(-1.5*(myArcCosine - M_PI_3_4));
    // }
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
    // if(myArcSin < M_PI_3 && myArcSin > M_PI_6 ){
    //   //Quadrant 2
    //   //Scale Left Front and Right Rear 
    //   lfScaleFactor = myArcSin > M_PI_4 ? sin(1.5*(myArcSin-M_PI_4)) : sin(-1.5*(myArcSin-M_PI_4));
    // }
    // if(myArcSin > -1*M_PI_3 && myArcSin < -1*M_PI_6) {
    //   //Quadrant 3
    //   //Scale Right Front and Left Rear
    //   rfScaleFactor = myArcSin > -1*M_PI_4 ? sin(1.5*(myArcSin+M_PI_2)) : sin(-1.5*(myArcSin+M_PI_2));
    // }
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
    // if(myArcSin < M_PI_3 && myArcSin > M_PI_6 ){ 
    //   //Quadrant 1
    //   //Scale Left Rear and Right Front 
    //   rfScaleFactor = myArcSin > M_PI_4 ? sin(1.5*(myArcSin-M_PI_4)) : sin(-1.5*(myArcSin-M_PI_4));
    // }
    // if(myArcSin > -1*M_PI_3 && myArcSin < -1*M_PI_6) {
    //   //Quadrant 4
    //   //Scale Right Rear and Left Front
    //   lfScaleFactor = myArcSin > -1*M_PI_4 ? sin(1.5*(myArcSin+M_PI_2)) : sin(-1.5*(myArcSin+M_PI_2));
    // }
  
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
  Serial.println(motorCommand);
  return motorCommand;
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

void readPS3Command(){
  String serialResponse = Serial1.readStringUntil('\r\n');
  char buf [serialResponse.length()];  
  serialResponse.toCharArray(buf, sizeof(buf));
  //debug stuff
  byte delimiter = serialResponse.indexOf(":");

  String input = serialResponse.substring(0,delimiter);
  String inputValue = serialResponse.substring(delimiter+1);
  int value = inputValue.toInt();
  
  //Only Processing the commands for the sticks
  if(input == "@LY"){
     lyValue = value;
     lyReady = true;
  } else if( input == "@LX") {
    lxValue = value;
    lxReady = true;
  }

  if(lxReady && lyReady){
    Serial2.print(createDriveCommand(lyValue, lxValue));
    lxReady = false;
    lyReady = false;
  }
  
}


