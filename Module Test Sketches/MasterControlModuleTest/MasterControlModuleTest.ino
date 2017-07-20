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
  }
  if (Serial1.available())
  {
     Serial.write(Serial1.read());
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

