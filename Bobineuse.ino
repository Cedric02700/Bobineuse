#include "BluetoothSerial.h" 

BluetoothSerial SerialBt;

//********** Constants **************************************************************

int BrocheDir = 23;
int BrocheStep = 19;
int BrocheEn = 22;
int GuideDir = 17;
int GuideStep = 5;
int GuideEn = 18;

//********** Variables *************************************************************

  //*** Debug ***

int DebugBroche = 0;
int DebugGuide = 0;
    
  //*** Moteurs ***

int StepDelay = 0;
int StepDelayGuide = 0; 
int StepDelayValue = 0;
boolean BrocheTourne = false;
boolean GuideTourne = false;
int PasMoteurGuide = 3200;
int PasMoteurBroche = 3200;

  //*** Acceleration ***

int AccelSpeed = 5000;
int AccelSpeedStep = 0;
boolean Accel = true; 
int StepDelayOld = 0;
int StepDelayGuideOld = 0;
boolean AccelEnCours = false;

  //*** Syncho broche/Guide

int StepBrocheGuide = 0;
//int ComptePasGuide = 0;
int PasVisGuide = 100;   //en centieme de mm
int PasParCent = 0;
float Ratio = 0.0;

  //*** Bobinage ***

int Diametre = 1;
int LgBobine = 1;
int NSpire = 1;
int Vitesse = 60;
int Offset = 0;
boolean Sens = 0;   // 0=gauche, 1=droite
boolean BobinageEnCours = false;
int CompteTours = 0;
int ComptePas = 0;
    
  //*** BT ***
String currentLine = "";
String command = "";
String value = "";
boolean newCommand = false;
boolean initialise = false;

  //*** Timer ***
unsigned long BrocheTimerStart = 0;
unsigned long BrocheTimerNext = 0;
unsigned long BrocheTimer = 0;
unsigned long GuideTimerStart = 0;
unsigned long GuideTimerNext = 0;
unsigned long GuideTimer = 0;
boolean BrocheStepState = LOW;
boolean GuideStepState = LOW;

//********** SETUP *****************************************************************

void setup() 
{
  Serial.begin(115200);
  SerialBt.begin("Bobineuse"); 
  pinMode(BrocheDir, OUTPUT);
  pinMode(BrocheStep, OUTPUT);
  pinMode(BrocheEn, OUTPUT);
  digitalWrite(BrocheEn, HIGH);
  digitalWrite(BrocheStep, LOW);
  pinMode(GuideDir, OUTPUT);
  pinMode(GuideStep, OUTPUT);
  pinMode(GuideEn, OUTPUT);
  digitalWrite(GuideEn, HIGH);
  digitalWrite(GuideStep, LOW);
  PasParCent = PasMoteurGuide / PasVisGuide;
  //StepDelayGuide = (1000000/(PasMoteurBroche*2));  //60 trs/min
}

//********** Read BT serial port ****************************************************

void receiveBt()
{
  if(SerialBt.available())
  {
    String currentLine = "";
    while(SerialBt.available())
    {
      //char inChar;    
      char inChar = SerialBt.read();
      currentLine += inChar;
      if(inChar == ':')
      {
        command = currentLine;
        command.remove(((command.length()) - 1) , 1);       //Remove ':' at the end of command
        Serial.print("Command: "); Serial.println(command);
        currentLine = "";      
      }
      if(inChar == '#')
      {
        value = currentLine;
        value.remove(((value.length()) - 1), 1);           //Remove '#' at the end of value
        Serial.print("Value: "); Serial.println(value);
        currentLine = "";
//        newCommand = true;     //New command received
        execute();
      }    
    }
  }
}

//********** Send command ***************************************************************

void sendCommand(String com, String val)
{
  SerialBt.print(com);SerialBt.print(":");SerialBt.print(val);SerialBt.print("#");  
  Serial.print("Envoi commande: ");Serial.println(com);  
}


////********** Execute the new command ****************************************************
//
void execute()
{
  Serial.println("void execute()");
  Serial.println("{");
  
  //******* Diametre
  if((command.compareTo("Diametre")) == 0)   
  {
    Diametre = value.toInt();
  }

  //******* LgBobine
  if((command.compareTo("LgBobine")) == 0)   
  {
    LgBobine = value.toInt();
  }
  
  //******* Vitesse
  if((command.compareTo("Vitesse")) == 0)
  {
    Vitesse = value.toInt();
    StepDelayValue = (1000000/(PasMoteurBroche * 2 * (Vitesse / 60)));
    Serial.print("StepDelayValue: "); Serial.println(StepDelayValue);
    StepDelay = StepDelayValue;
    Ratio = (float)Diametre / (float)PasVisGuide;
    Serial.print("Ratio: "); Serial.println(Ratio);
    //StepBrocheGuide = (PasMoteurBroche / (PasParCent * Diametre));
    StepDelayGuide = StepDelay / Ratio;
    Serial.print("StepDelayGuide: "); Serial.println(StepDelayGuide);
  }

  //******* Offset
  if((command.compareTo("Offset")) == 0)   
  {
    Offset = value.toInt();
  }
  
  //******* Sens
  if((command.compareTo("Sens")) == 0)   
  {
    if((value.compareTo("G")) == 0) digitalWrite(GuideDir, LOW);   
    if((value.compareTo("D")) == 0) digitalWrite(GuideDir, HIGH);
  }

  //******* Start
  if((command.compareTo("Start")) == 0)
  {
    digitalWrite(BrocheEn, LOW);
    digitalWrite(GuideEn, LOW);
    digitalWrite (BrocheDir, HIGH);
    NSpire = (LgBobine * 100) / Diametre;
    BobinageEnCours = true;
    CompteTours = 0;
    ComptePas = 0;
    Accel = true;
    AccelEnCours = false; 
    Serial.print("Diametre: "); Serial.println(Diametre);
    Serial.print("Nb. spires: "); Serial.println(NSpire);
    Serial.print("Vitesse: "); Serial.println(Vitesse);
    Serial.print("Offset: "); Serial.println(Offset);
    Serial.print("Sens: "); Serial.println(Sens);
    Serial.print("Pas broche par pas guide: "); Serial.println(StepBrocheGuide);
//    BrocheTimerStart = micros();
//    BrocheTimerNext = BrocheTimerStart + StepDelay;
//    GuideTimerStart = micros();
//    GuideTimerNext = GuideTimerStart + StepDelayGuide;
  }

  //*** Avance
  if((command.compareTo("Avance")) == 0)       //
  {
    Serial.print(command); Serial.print(" "); Serial.println(value);
    Vitesse = value.toInt();
    StepDelayValue = (1000000 / (PasMoteurBroche * 2 * (Vitesse / 60)));
    StepDelay = StepDelayValue;
    Serial.print("Step delay: "); Serial.println(StepDelay);
    digitalWrite (BrocheEn, LOW);
    digitalWrite (BrocheDir, LOW);
    BrocheTourne = true;
    Accel = true;
    AccelEnCours = false;    
  }

  //*** Recule
  if((command.compareTo("Recule")) == 0)       //
  {
    Serial.print(command); Serial.print(" "); Serial.println(value);
    Vitesse = value.toInt();
    StepDelayValue = (1000000 / (PasMoteurBroche * 2 * (Vitesse / 60)));
    StepDelay = StepDelayValue;
    Serial.print("Step delay: "); Serial.println(StepDelay);
    digitalWrite (BrocheEn, LOW);
    digitalWrite (BrocheDir, HIGH);
    BrocheTourne = true;
    Accel = true; 
    AccelEnCours = false;  
  }

  //*** GuideGauche
  if((command.compareTo("GuideGauche")) == 0)       //
  {
    Serial.print(command); Serial.print(" "); Serial.println(value);
    Vitesse = value.toInt();
    StepDelayValue = (1000000 / (PasMoteurBroche * 2 * (Vitesse / 60)));
    StepDelay = StepDelayValue;
    Serial.print("Step delay: "); Serial.println(StepDelay);
    digitalWrite (GuideEn, LOW);
    digitalWrite (GuideDir, LOW);
    GuideTourne = true;
    Accel = true;  
    AccelEnCours = false;  
  }

  //*** GuideDroite
  if((command.compareTo("GuideDroite")) == 0)       //
  {
    Serial.print(command); Serial.print(" "); Serial.println(value);
//    AccelSpeed = value.toInt(); //debug
    Vitesse = value.toInt();
    StepDelayValue = (1000000 / (PasMoteurBroche * 2 * (Vitesse / 60)));
    StepDelay = StepDelayValue;
    Serial.print("Step delayVlaue: "); Serial.println(StepDelayValue);
    digitalWrite (GuideEn, LOW);
    digitalWrite (GuideDir, HIGH);
    GuideTourne = true;
    Accel = true;
    AccelEnCours = false;    
  }

  //*** Stop
  if((command.compareTo("Stop")) == 0)       //
  {
    Serial.print(command); Serial.print(" "); Serial.println(value);
    digitalWrite(BrocheEn, HIGH);
    digitalWrite(GuideEn, HIGH);
    BrocheTourne = false;
    GuideTourne = false;
    BobinageEnCours = false;
    AccelEnCours = false;
    Accel = true; 
    sendCommand("n", String(CompteTours)); 
  }

  //*** Pause
  if((command.compareTo("Pause")) == 0)       //
  {
    digitalWrite(BrocheEn, HIGH);
    digitalWrite(GuideEn, HIGH);
    Serial.print(command); Serial.print(" "); Serial.println(value);
    BobinageEnCours = false;
    StepDelay = StepDelayValue;
    sendCommand("n", String(CompteTours));
  }

  //*** Reprise
  if((command.compareTo("Reprise")) == 0)       //
  {
    digitalWrite(BrocheEn, LOW);
    digitalWrite(GuideEn, LOW);
    Serial.print(command); Serial.print(" "); Serial.println(value);
    StepDelay = StepDelayValue;
    BobinageEnCours = true;
    AccelEnCours = false;
    Accel = true;  
  }
  
  Serial.println("}");
  Serial.println("");
//  newCommand = false;       //Command taken
}

//********** AvanceUnPasGuide ****************************************************************

void AvanceUnPasGuide()
{
  digitalWrite(GuideStep, HIGH);
  delayMicroseconds (StepDelayValue);
  digitalWrite(GuideStep, LOW);
  delayMicroseconds (StepDelayValue);
}

//********** AvanceUnPasBroche ****************************************************************

void AvanceUnPasBroche()
{
  if(Accel == true) Acceleration();
  digitalWrite(BrocheStep, HIGH);
  delayMicroseconds (StepDelay);
  digitalWrite(BrocheStep, LOW);
  delayMicroseconds (StepDelay);
  //ComptePas++;
  //ComptePasGuide++;
}

//********** Bobine *********************************************************************

void Bobine()
{
  if(Accel == true) Acceleration();
  
  BrocheTimer = micros();
  if((BrocheTimer - BrocheTimerStart) >= StepDelay)
  {
    if(BrocheStepState == HIGH) 
    {
      digitalWrite(BrocheStep, LOW);
      BrocheStepState = LOW;
      ComptePas++;
    }
    else 
    {
      digitalWrite(BrocheStep, HIGH);
      BrocheStepState = HIGH;
    }
    BrocheTimerStart = BrocheTimerNext;
    BrocheTimerNext = BrocheTimerStart + StepDelay;
  }

  GuideTimer = micros();
  if((GuideTimer - GuideTimerStart) >= StepDelayGuide)
  {
    //int Debug = GuideTimer - GuideTimerStart;
    //Serial.print("Timer - Start: "); Serial.println(Debug);
    if(GuideStepState == HIGH) 
    {
      digitalWrite(GuideStep, LOW);
      GuideStepState = LOW;
    }
    else 
    {
      digitalWrite(GuideStep, HIGH);
      GuideStepState = HIGH;
    }
    GuideTimerStart = GuideTimerNext;
    GuideTimerNext = GuideTimerStart + StepDelayGuide;
    GuideTimer = micros();
    if(GuideTimer <= GuideTimerStart)
    {
      GuideTimerStart = micros();
      GuideTimerNext = GuideTimerNext + StepDelayGuide;
    }
  }
  
  if(ComptePas == PasMoteurBroche) 
  {
    ComptePas = 0;
    CompteTours++;
  }
  
  if(CompteTours >= NSpire)
  {
    digitalWrite(BrocheEn, HIGH);
    digitalWrite(GuideEn, HIGH);
    BobinageEnCours = false;
    AccelEnCours = false;
    Accel = true;
    delay(100);
    sendCommand("b", "1");
    delay(100);
    sendCommand("n", String(CompteTours));
  }
}

//********** Acceleration *********************************************************************

void Acceleration()
{
  if(AccelEnCours == false)
  {
    AccelEnCours = true;
    StepDelayOld = StepDelay; //Sauvegarde la valeur 
    StepDelay = (1000000/(PasMoteurBroche * 2 * (60 / 60))); //Demarre a 60 trs/min
    StepDelayGuide = (float)StepDelay / Ratio;
    AccelSpeedStep = AccelSpeed; //Initialise AccelSpeedStep

    BrocheTimerStart = micros();
    BrocheTimerNext = BrocheTimerStart + StepDelay;
    GuideTimerStart = micros();
    GuideTimerNext = GuideTimerStart + StepDelayGuide;  
  }
  
  AccelSpeedStep--;
  if(AccelSpeedStep <= 0)
  {
    StepDelay--;
    StepDelayGuide = (float)StepDelay / Ratio;
    AccelSpeedStep = AccelSpeed;
  }
  if(StepDelayOld >= StepDelay)  //Fin de l'acceleration
  {
    StepDelay = StepDelayValue;
    StepDelayGuide = (float)StepDelay / Ratio;
    AccelEnCours = false;
    Accel = false;
  }
}

//********** LOOP ***********************************************************************

void loop() 
{
  if(BrocheTourne == true) AvanceUnPasBroche();
  if(GuideTourne == true) AvanceUnPasGuide();
  if(BobinageEnCours == true) Bobine(); 
  receiveBt();
}
