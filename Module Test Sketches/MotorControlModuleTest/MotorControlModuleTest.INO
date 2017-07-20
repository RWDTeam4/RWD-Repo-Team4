 //const String ModuleID = "MCMx2_Sync;
//const float SoftWareVer = 0.09;
#define TotalCH = 2;

int PWMLimit = 253;
int PWM_Current = 0;
int PWM_Set = 0;
unsigned long usNow_1;

/*
int IN1 = 8;
int IN2 = 7;
int IN3 = 12;
int IN4 = 13;
*/
int SR = 0;

int SER_Pin = A0;
int RCLK_Pin = A2;
int SRCLR_Pin = A1;
int MotorAPWM = 6;
int MotorBPWM = 9;
int MotorCPWM = 10;
int MotorDPWM = 11;
int motorPWM_A = 0;
int motorPWM_B = 0;
int motorPWM_C = 0;
int motorPWM_D = 0;

void setup() 
{ 
  // put your setup code here, to run once:
  Serial.begin(57600); 
  pinMode(SER_Pin, OUTPUT);
  pinMode(RCLK_Pin, OUTPUT);
  pinMode(SRCLR_Pin, OUTPUT);
  pinMode(MotorAPWM, OUTPUT);
  pinMode(MotorBPWM, OUTPUT);  
  pinMode(MotorCPWM, OUTPUT);
  pinMode(MotorDPWM, OUTPUT); 
  digitalWrite(SRCLR_Pin, LOW);
  shiftOut(SER_Pin, RCLK_Pin, MSBFIRST, SR);
  digitalWrite(SRCLR_Pin, HIGH);
  Serial.println("Motor Control Online");
  //motorSTOP();
//  intializeModule();
  
}

void loop() 
{

  if (Serial.available()>0)
  {
  readCMD();
  }

 

}


void readCMD()
{ 
    Serial.println("\n Serial Command Ready!");
    String MotorCH = "";      
    String MotorPWM = "";

    String inputCMD = Serial.readStringUntil('\n');
    Serial.println(inputCMD);
    for (int i =0; i< inputCMD.length(); i++)
    {
      if (inputCMD.charAt(i) == ':')
      {

        for (int j = 0;j < i;j++)
        {

          MotorCH+=inputCMD[j];
        }
        for (int j = i+1;j<inputCMD.length()+1; j++)
        {
          MotorPWM+=inputCMD[j];
        }
          Serial.print("\n [CMD]: ");
          Serial.print(inputCMD);        
          Serial.print("\n [");
          Serial.print(MotorCH.toInt());
          Serial.print("]: ");
          Serial.print(MotorPWM.toInt());
          if ((MotorCH.toInt() < 5) && (MotorCH.toInt() > -1))
          {
            if ((MotorPWM.toInt() < 254) && (MotorCH.toInt() > -254))
            {
              PWM_Set = MotorPWM.toInt();
              PWM_Current = MotorPWM.toInt();
              motorPWM(MotorCH.toInt(), MotorPWM.toInt());
            }
            else
            {
              Serial.print("\n Motor PWM Error! -253 ~ +253");        
            }
          }
          else
          {
            Serial.print("\n Motor Channel Error! 0 = All Channels");            
          }
         
      }
    }
    
    //motorPWM(inputCMD.toInt());
    /*for(int i = 0; i< inputCMD.length(); i++)
    {
      if (inputCMD[i]    
    }
    */
    
  
}


void motorPWM(int CHvalue, int PWMvalue)
{
  //Serial.print("\n [Input]: ");
  //Serial.print(value);
  PWMvalue = PWMvalue > PWMLimit ? PWMLimit: PWMvalue;
  PWMvalue = PWMvalue < -PWMLimit ? -PWMLimit: PWMvalue;
  
  
  switch(CHvalue)
  {
    case  (0):
    {
      motorPWM_A = PWMvalue;
      motorPWM_B = PWMvalue;
      motorPWM_C = PWMvalue;
      motorPWM_D = PWMvalue;
      if (PWMvalue > 0)
      {
        analogWrite(MotorAPWM, motorPWM_A);
        analogWrite(MotorBPWM, motorPWM_B);
        analogWrite(MotorCPWM, motorPWM_C);
        analogWrite(MotorDPWM, motorPWM_D);
        SR = bitWrite( SR, 0, 1);
        SR = bitWrite( SR, 1, 0);
        SR = bitWrite( SR, 2, 1);
        SR = bitWrite( SR, 3, 0);
        SR = bitWrite( SR, 4, 0);
        SR = bitWrite( SR, 5, 1);
        SR = bitWrite( SR, 6, 0);
        SR = bitWrite( SR, 7, 01);
        break;
      }
      if (PWMvalue < 0)
      {
        analogWrite(MotorAPWM, 255 - motorPWM_A);
        analogWrite(MotorBPWM, 255 - motorPWM_B);
        analogWrite(MotorAPWM, 255 - motorPWM_C);
        analogWrite(MotorBPWM, 255 - motorPWM_D);
        SR = bitWrite( SR, 0, 0);
        SR = bitWrite( SR, 1, 1);
        SR = bitWrite( SR, 2, 0);
        SR = bitWrite( SR, 3, 1);
        SR = bitWrite( SR, 4, 1);
        SR = bitWrite( SR, 5, 0);
        SR = bitWrite( SR, 6, 1);
        SR = bitWrite( SR, 7, 0);
        SR = B01011010;
        break;
      }
      if (PWMvalue == 0)
      {
        analogWrite(MotorAPWM, motorPWM_A);
        analogWrite(MotorBPWM, motorPWM_B);
        analogWrite(MotorAPWM, motorPWM_C);
        analogWrite(MotorBPWM, motorPWM_D);
        SR = bitWrite( SR, 0, 0);
        SR = bitWrite( SR, 1, 0);
        SR = bitWrite( SR, 2, 0);
        SR = bitWrite( SR, 3, 0);
        SR = bitWrite( SR, 4, 0);
        SR = bitWrite( SR, 5, 0);
        SR = bitWrite( SR, 6, 0);
        SR = bitWrite( SR, 7, 0);
        break;        
      }
      
      break;
    }
    case  (1):
    {
      motorPWM_A = PWMvalue;
      if (PWMvalue > 0)
      {
        analogWrite(MotorAPWM, motorPWM_A);
        SR = bitWrite( SR, 0, 1);
        SR = bitWrite( SR, 1, 0);
        break;
      }
      if (PWMvalue < 0)
      {
        analogWrite(MotorAPWM, 255 - motorPWM_A);
        SR = bitWrite( SR, 0, 0);
        SR = bitWrite( SR, 1, 1);
        break;
      }
      if (PWMvalue == 0)
      {
        analogWrite(MotorAPWM, motorPWM_A);
        SR = bitWrite( SR, 0, 0);
        SR = bitWrite( SR, 1, 0);
        break;
      }      
      break;
    }
    case  (2):
    {
      motorPWM_B = PWMvalue;
      if (PWMvalue > 0)
      {
        analogWrite(MotorBPWM, motorPWM_B);
        SR = bitWrite( SR, 2, 1);
        SR = bitWrite( SR, 3, 0);
        break;
      }
      if (PWMvalue < 0)
      {
        analogWrite(MotorBPWM, 255 - motorPWM_B);
        SR = bitWrite( SR, 2, 0);
        SR = bitWrite( SR, 3, 1);
        break;
      }
      if (PWMvalue == 0)
      {
        analogWrite(MotorBPWM, motorPWM_B);
        SR = bitWrite( SR, 2, 0);
        SR = bitWrite( SR, 3, 0);
        break;        
      }
      break;
    }
    case  (3):
    {
      motorPWM_C = PWMvalue;
      if (PWMvalue > 0)
      {
        analogWrite(MotorCPWM, motorPWM_C);
        SR = bitWrite( SR, 4, 0);
        SR = bitWrite( SR, 5, 1);
        break;
      }
      if (PWMvalue < 0)
      {
        analogWrite(MotorCPWM, 255 - motorPWM_C);
        SR = bitWrite( SR, 4, 1);
        SR = bitWrite( SR, 5, 0);
        break;
      }
      if (PWMvalue == 0)
      {
        analogWrite(MotorCPWM, motorPWM_C);
        SR = bitWrite( SR, 4, 0);
        SR = bitWrite( SR, 5, 0);
        break;        
      }
      break;
    }    
    case  (4):
    {
      motorPWM_D = PWMvalue;
      if (PWMvalue > 0)
      {
        analogWrite(MotorDPWM, motorPWM_D);
        SR = bitWrite( SR, 6, 0);
        SR = bitWrite( SR, 7, 1);
        break;
      }
      if (PWMvalue < 0)
      {
        analogWrite(MotorDPWM, 255 - motorPWM_D);
        SR = bitWrite( SR, 6, 1);
        SR = bitWrite( SR, 7, 0);
        break;
      }
      if (PWMvalue == 0)
      {
        analogWrite(MotorDPWM, motorPWM_D);
        SR = bitWrite( SR, 7, 0);
        SR = bitWrite( SR, 6, 0);
        break;        
      }
      break;
    }
    default:
    {
      break;  
    }
    
  }
  
  Serial.print("\n A:[");
  Serial.print(motorPWM_A);
  Serial.print("] B:[");
  Serial.print(motorPWM_B);
  Serial.print("] C:[");
  Serial.print(motorPWM_C);
  Serial.print("] D:[");
  Serial.print(motorPWM_D);
  Serial.print("]");
  IOcommand();
}

 
void motorSTOP()
{
  /*
  analogWrite(MotorAPWM, 0);
  analogWrite(MotorBPWM, 0);
  digitalWrite(MotorAIN, LOW);
  digitalWrite(MotorAIN2, LOW);
  digitalWrite(MotorBIN, LOW);
  digitalWrite(MotorBIN2, LOW);
*/
}

void IOcommand()
{

  Serial.print("\n Array: [");
  Serial.print(SR, DEC);
  Serial.print("] ");
  Serial.print(SR, BIN);
  digitalWrite(SRCLR_Pin, LOW);
  shiftOut(SER_Pin, RCLK_Pin, MSBFIRST, SR);
  digitalWrite(SRCLR_Pin, HIGH);
}


