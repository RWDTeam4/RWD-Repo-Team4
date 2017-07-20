#include <Servo.h>



int ACMotor_MAX = 180;
int Limit_MAX = 255;
int Limit_MIN = 0;
int Stepping = 300;
//UNO
//int DAC_OUT1 = 3;
//int SERVO_OUT1 = 5;

//Mega
int DAC_OUT1 = 2;
int SERVO_OUT1 = 4;

int ADC_PWM = 255;

Servo servo1;
void setup() 
{ 
  // put your setup code here, to run once:
  servo1.attach(SERVO_OUT1,550,2370);
  Serial.begin(57600);
   while (!Serial) 
   {
    
   }
  Serial.print("Staring...."); 
  pinMode(DAC_OUT1, OUTPUT);
  DAC_Conversion(255,DAC_OUT1);
  delay(1000);
  intialization();
}

void loop() 
{
  if (Serial.available()>0)
  {
  readCMD();
  }
  //Serial.println(ADC_PWM);
  //analogWrite(ADC_PWM, DAC_OUT1);
}

void readCMD()
{ 
    Serial.println("\n Serial Command Ready!");
    //char TempChar;
    String inputCMD = Serial.readStringUntil('\n');
    Serial.print("\nINPUT CMD: ");
    Serial.print(inputCMD);
    /*
    if ((inputCMD == "U") ||(inputCMD == "u"))
    {
      Serial.print("\nRAMPING UP: ");
      Serial.print(ADC_PWM);  
      RAMP_UP(ADC_PWM, 150, DAC_OUT1);
     
    }
    if ((inputCMD == "D") ||(inputCMD == "d"))
    {
       Serial.print("\nRAMPING DOWN: ");
       RAMP_DOWN(ADC_PWM, 0, DAC_OUT1);
    }
    */
    if ((inputCMD.toInt() < 256) || (inputCMD.toInt() > -1))
    {
       ADC_PWM = inputCMD.toInt();
       DAC_Conversion(ADC_PWM, DAC_OUT1);
    }
    else
    {
      Serial.println("ERROR");
    }
}

void RAMP_UP (int NOW, int MAX,  int PIN)
{

  ADC_PWM = 10;
  DAC_Conversion(10, DAC_OUT1);
  /*
  for(NOW; NOW < MAX; NOW++)
  {
    delay(50);
    DAC_Conversion(NOW, DAC_OUT1);
    Serial.println(NOW);  
  }
  ADC_PWM = MAX;
  DAC_Conversion(ADC_PWM, DAC_OUT1);
  Serial.print("\nFINAL NOW: ");  
  Serial.print(ADC_PWM);  
  */
  

  
}


void RAMP_DOWN (int NOW, int MIN,  int PIN)
{
    for(NOW; NOW > MIN; NOW++)
  {
    delay(100);
    DAC_Conversion(NOW, DAC_OUT1);
    Serial.println(NOW);  
  }
  ADC_PWM = MIN;
}

void intialization()
{
  Serial.println("Initialization....");
  DAC_Conversion(255,DAC_OUT1);
  Servo_Action(0);
  delay(200);
  ADC_PWM = 0;
  DAC_Conversion(ADC_PWM, DAC_OUT1);
  Serial.println("READY!!!");
}

void DAC_Conversion(int PWM, int PIN)
{
  analogWrite(PIN, PWM);
  Servo_Action(PWM);
}

void Servo_Action(int PWMValue)
{
  int val = map(PWMValue, Limit_MIN, Limit_MAX,0,180);
  servo1.write(val);
}
