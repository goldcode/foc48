#include "serialCom.h"

// The Stim Board if Triggering the camera



unsigned int nInterval = 3;
unsigned int nRow = 8;
unsigned int iRow = 0;
char deviceName[]="LightStimulator";
char deviceVersion[]="19";

//Boards Produced:
// 1: eStim 
// 2: eStim inverted               Stimulationsboard_Arduino_V5.7 UV an PB0
// 3: light Stim with micro switch Stimulationboard_Light_V3.0    UV an PD4
// 4: light Stim with Trigger      Stimulationboard_Light_trig    UV an PD4






int boardVersion=2;


unsigned int uvFlash=8; //uv Flash Duration in ms
unsigned long now;
unsigned long uvTimer;

unsigned long inter[] = {1500, 1500, 1500, 1500, 1500, 1500, 1500, 1500}; // Time period between Stimulations in ms
unsigned int  pulse[] = {   2,    2,    2,    2,    2,    2,    2,    2}; // Flash duration in ms
unsigned int nFlash[] = {   5,    5,    5,    5,    5,    5,    5,    5}; // Number of flashes per stimulation
unsigned int iFlash[] = {   0,    0,    0,    0,    0,    0,    0,    0}; // Number of flashes per stimulation
unsigned long timer[] = {   1,    1,    1,    1,    1,    1,    1,    1};
bool        bActive[] = {   1,    1,    1,    1,    1,    1,    1,    1}; // mark active rows

           //ROW                         A       B          C         D            E         F          G       H                  
           //NAMES Pos                  PD2     PD6        PB4       PB6          PC7       PF6         PF4    PF0
           //nameNeg                    PD3     PD4        PD7       PB5          PC6       PF7         PF5    PF1
volatile unsigned char * portPos[] = { &PORTD, &PORTD,   &PORTB,   &PORTB,     &PORTC,    &PORTF,    &PORTF, &PORTF};
volatile unsigned char * portNeg[] = { &PORTD, &PORTD,   &PORTD,   &PORTB,    &PORTC,    &PORTF,    &PORTF, &PORTF};

const unsigned char pinPos[] = { 0b00000100,0b01000000,0b00010000,0b01000000,0b10000000,0b01000000,0b00010000,0b00000001};
const unsigned char pinNeg[] = { 0b00001000,0b00010000,0b10000000,0b00100000,0b01000000,0b10000000,0b00100000,0b00000010};





int bLight      = false;
int bFlash      = false;
int bStimulate  = false;
int bPermanent  = false;
int bSequential = false;
int bSendStim   = false;
int bExtTrig    = false;

unsigned long trigTimer;
int trigRate = 25;   //trigger Rate in ms   50ms = 20Hz

bool bElectrical;

// Tissues should not be lit for more that ~3mins
// If longe exposure is needed set bUvAuto=false
// or uvDuration to whatever value
int bUvAuto      = 1;
unsigned long uvDuration     = 180;       //turn UV Off after 180 sec
unsigned long long uvOffTime = 0;

char sbuf[50];
unsigned long long tic, toc;
unsigned int dd, dt;
unsigned int ms_to_us=1000;

int stimPinOffState=HIGH;
//int uvPinOffState=LOW;

void lightOn(void);
//void lightOff(void);

//int uvPin   = 8; //LED Stim Board = 4 eStim=8
int trigPin = 2; //LED Stim Board = 4 eStim=8

void setTimer() {  
  timer[0]=now;
  if (bSequential){
    for (iRow = 1; iRow < nRow; iRow++)
       timer[iRow] = timer[iRow-1]+nFlash[iRow-1]*trigRate;    
  } else {
    for (iRow = 1; iRow < nRow; iRow++)
       timer[iRow] = timer[iRow-1]; 
  }
}

void setup() {
  TXLED0;
  RXLED0;
  
  // Higher speeds do not appear to be reliable
  now=millis();
  Serial.begin(9600);
  //DDRD   =  DDRD | 0b00010000; //to use portd bit4 that is pin25 of MEGA32U
  //PORTD &=         0b11101111; //set to 0 will light the UV  
  
  switch (boardVersion){
    case 1:   //eStim
      DDRB  = DDRB | (0b01110000); //PB4, PB5, PB6
      PORTB |=       0b01110000;  
      DDRC  = DDRC | (0b11000000); //PC6, PC7
      PORTC |=       0b11000000;
      DDRD  = DDRD | (0b11011100); //PD2, PD3, PD4, PD6, PD7
      PORTD =       0b11011100;
      DDRF  = DDRF | (0b11110011); //PF0, PF1, PF4, PF5, PF6, PF7
      PORTF |=       0b11110011;  
      //
      bElectrical=1;
      bSequential=1;
      break;
    case 2:  //eStim inverted
      DDRB  = DDRB | (0b01110001); //PB4, PB5, PB6
      PORTB |=        0b01110001;  
      DDRC  = DDRC | (0b11000000); //PC6, PC7
      PORTC |=        0b11000000;
      DDRD  = DDRD | (0b11011100); //PD2, PD3, PD4, PD6, PD7
      PORTD =         0b11011100;
      DDRF  = DDRF | (0b11110011); //PF0, PF1, PF4, PF5, PF6, PF7
      PORTF |=        0b11110011;
      bElectrical=1;
      bSequential=1;     

      break;
    case 3:  //lightStim with micro switch
      DDRB   = DDRB | (0b01010000); // PB4, , PB6,
      PORTB |=         0b01010000 ;
      DDRC   = DDRC | (0b10000000); // , PC7
      PORTC |=         0b10000000 ;
      DDRD   = DDRD | (0b01011000); //, PD3, PD4 , PD6,    PD4=UV Pin
      PORTD |=         0b01001000 ;
      DDRF   = DDRF | (0b01010001); //PF0, , PF4, , PF6,
      PORTF |=         0b01010001 ;
      bElectrical=0;
      break;
    case 4:  //lightStim with trigger
      DDRB   = DDRB | (0b01010001); // PB4, , PB6
      PORTB |=         0b01010000 ;
      DDRC   = DDRC | (0b10000000); // , PC7
      PORTC |=         0b10000000 ;
      DDRD   = DDRD | (0b01011000); //, PD3, , PD6,       PD4=UV Pin
      PORTD |=         0b01001000 ;
      DDRF   = DDRF | (0b01010001); //PF0, , PF4, , PF6,
      PORTF |=         0b01010001 ;
      bElectrical=0;
      pinMode(trigPin, INPUT);
      digitalWrite(trigPin, HIGH);
      attachInterrupt(digitalPinToInterrupt(trigPin),   getTrigger, CHANGE);
      break;
    default:;
  }
  //
  bSendStim=false; 
  bStimulate=false;
  bPermanent=false;
  bLight = false;
  bExtTrig=false;  
  bFlash=false;  
  uvTimer  =now;
  trigTimer=now;
  lightOff();
  setTimer();
}

void getTrigger() {  
  if (digitalRead(trigPin)){ //RISING
      if (bFlash){
        sendUvFlash();
      }
  } else { //FALLING      
      if (bStimulate) {
        sendPulse();
      }
  }  
}

void sendUvFlash(){
  dd=uvFlash*ms_to_us;
  lightOn();
  delayMicroseconds(dd);
  //delay(uvFlash);
  lightOff();
}

void stimSwitchOn(void) {  
  for (int iRow=0; iRow < nRow; iRow++) {
      digitalWrite(pinPos[iRow], !stimPinOffState);
  }  
}

void stimSwitchOff(void) {  
  for (int iRow=0; iRow < nRow; iRow++) {
      digitalWrite (pinPos[iRow], stimPinOffState);
  }  
}

void setStimulation(int rr){
  for (int iRow=0; iRow < nRow; iRow++) {
      inter[iRow]=rr;
  }  
}

void setPulseDuration(int pd){
  for (int iRow=0; iRow < nRow; iRow++) {
      pulse[iRow]=pd;
  }  
}

void setPulseNumber(int n){
  for (iRow=0; iRow < nRow; iRow++) {
    nFlash[iRow] = n;
  }
}

void lightOn(void) {
  switch (boardVersion) {
    case 1:
      PORTD &= 0b11101111;  //eStim
      break;
   case 2:
      PORTB &= 0b11111110;  //eStim Inverted
      break;
   case 3:
      PORTD &= 0b11101111;  //lightStim Micro Switch
      break;
   case 4:
      PORTD &= 0b11101111;  //lightStim Trigger Pin
      break;
    default:;
  }  
}

void lightOff(void) {
  switch (boardVersion) {
    case 1:
       PORTD |= 0b00010000;   //eStim
       break;
    case 2:
       PORTB |= 0b00000001;   //eStim Inverted
       break;;
    case 3:
       PORTD |= 0b00010000;  //lightStim Micro Switch
       break;
    case 4:
       PORTD |= 0b00010000;  //PD4
       break;
    default:;
  }
  //digitalWrite(uvPin, HIGH);
}



void lightSwitchOn(void) {  
  uvOffTime = now + 1000 * uvDuration;
  lightOn();
  bLight=1;
}

void lightSwitchOff(void) {  
  bLight=0;
  lightOff();
}


void sendPulseBi(){
  //BiPhasic pulses are *always* sequential to not overload usb
  for (iRow=iRow%nRow; iRow < nRow; iRow++) {
   //now=millis();
   
   if (bActive[iRow] && now >= timer[iRow]) {
       if (iFlash[iRow] < nFlash[iRow]) {
          dd=pulse[iRow]*ms_to_us;          
          //
          *portPos[iRow] &= ~pinPos[iRow];   //set LOW          
          delayMicroseconds(dd);
          *portPos[iRow] |=  pinPos[iRow];     //set HIGH
          delayMicroseconds(1000);
          *portNeg[iRow] &= (~pinNeg[iRow]);   //set LOW          
          delayMicroseconds(dd);
          *portNeg[iRow] |=  pinNeg[iRow];   //set HIGH 
          //
          iFlash[iRow]++;
          break;          
        } else {
          iFlash[iRow]=0;
          timer[iRow] = timer[iRow] + inter[iRow];
        }
    }
  }  
}


void sendPulse(){
  if (bPermanent){
    dd=pulse[1]*ms_to_us;
    stimSwitchOn();
    delayMicroseconds(dd);  
    stimSwitchOff();
  } else if (bSequential){
    for (iRow=iRow%nRow; iRow < nRow; iRow++) {
      //now=millis();
      if (now >= timer[iRow]) {
        if (iFlash[iRow] < nFlash[iRow]) {
          dd=pulse[iRow]*ms_to_us;            
          digitalWrite (pinPos[iRow], LOW);
          delayMicroseconds(dd);
          //delay(pulse[iRow]);
          digitalWrite (pinPos[iRow], HIGH);
          iFlash[iRow]++;
          break;
          
        } else {
          iFlash[iRow]=0;
          timer[iRow] = timer[iRow] + inter[iRow];
        }
      }
    }
  } else {  
    dd=pulse[iRow]*ms_to_us;
    //turn lights on
    for (iRow=0; iRow < nRow; iRow++) {
      if (now >= timer[iRow]) {
        if (iFlash[iRow] < nFlash[iRow]) {
          digitalWrite (pinPos[iRow], LOW);
          iFlash[iRow]++;
        }  else {
          iFlash[iRow]=0;
          //if (iRow==0){
          //Serial.print("time is "); Serial.print(now);Serial.print(" timer was ");Serial.println(timer[iRow]);
          //}
          timer[iRow] = timer[iRow] + inter[iRow];
        }
      }
    }
    delayMicroseconds(dd);
    //turn all lights off
    for (iRow=iRow%nRow; iRow < nRow; iRow++){
        digitalWrite (pinPos[iRow], HIGH);
    }
  }  
}


void loop() {
  //
  now=millis();  
  //
  if (bUvAuto && now > uvOffTime)
       lightSwitchOff();
  //
  if (bLight) {
    lightOn();
  } else {
    lightOff();
  }
  if (! bExtTrig && now > trigTimer ) {    
    trigTimer=trigTimer+trigRate;     
    if (bStimulate){
      (bElectrical) ? sendPulseBi() : sendPulse();
    }
    if (bFlash)
      sendUvFlash();  
  }  
  // 
  if (Serial.available() > 0){
    MY_CMD cc;
    myGET_CMD(cc);
    //         
    switch (cc.cCMD) {
      case  NULL:    // Could not read Command      
        Serial.println("Failed to read Command");
        break;
      case '0':   //48:    //'0'      
        Serial.println(deviceName);
        break;
      case 'v':   //48:    //'0'      
        Serial.println(deviceVersion);
        Serial.print("Board Version: ");
        Serial.println(boardVersion);
        Serial.println(__FILE__);
        Serial.print("bElectrical: ");
        Serial.println(bElectrical);
        break;
      case 97: //a
        lightSwitchOn();
        bStimulate = 0;
        break;
      case 98: // b
        bLight = 0;
        bStimulate = 0;
        lightSwitchOff();
        break;
      case 99: //c
        lightSwitchOff();
        bStimulate = 1;
        setTimer();
        break;
      case 'f' :
        Serial.println("Flash UV Light Off");
        lightSwitchOff();
        bFlash=false;
        break;
      case 'F':        
        Serial.println("Flash UV Light On");
        if (cc.nCount >0 ){
           Serial.print("uvFlash Duration to");
           Serial.println(cc.nData[0]);
           uvFlash=cc.nData[0];                         
        }
        bFlash=true;
        lightSwitchOff();
        //restart internal trigger
        trigTimer=now;
        break;
      case 'm':        
        //return now to calculate timer offset
        Serial.println(now);
        break;
      case 'n':        
        Serial.println("UV AutoOff Off");
        //bLight=0;
        bUvAuto=false;
        break;
      case 'N':                     //can be used for tetanic Stimulation 200*50fmp=4 sec tetanus
         if (cc.nCount ==0 ){
           Serial.print("Pulse Duration is ");
           Serial.println(pulse[0]);         
        } else {
          if (cc.nCount >0 ){
            Serial.print("Set Pulse Number from ");
            Serial.print(nFlash[0]);
            Serial.print(" to ");
            Serial.println(cc.nData[0]);
            setPulseNumber(cc.nData[0]);        
          }
          if (cc.nCount >1 ){
            Serial.print("Set Pulse Duration from ");
            Serial.print(pulse[0]);
            Serial.print(" ms to ");
            Serial.println(cc.nData[1]);
            setPulseDuration(cc.nData[1]);           
          }
        }
        break;                    
      case 'p' :
        Serial.println("Permanent Stim Off");
        bPermanent=0;          
        stimSwitchOff();
        break;
      case 'P':        
        Serial.println("Permanent Stim On");
        bStimulate=1;
        bPermanent=1;
        stimSwitchOn();
        Serial.println("Permanent On");
        break;
      case 'R':        
        if (cc.nCount >0 ){
            Serial.print("Set only Row "); Serial.print(cc.nData[0]);
            for (iRow=0; iRow<nRow; iRow++)
               bActive[iRow]=0;
            bActive[cc.nData[0]]=1;
            
        } else {
          Serial.print("Set Rows 1-8 ");
          for (iRow=0; iRow<nRow; iRow++)
               bActive[iRow]=1;           
        }
        Serial.println(" active");
        break;
      case 's' :
        Serial.println("Stimulation Off");
        bStimulate=false;
        bPermanent=0;
        break;
      case 'S':
        //
        if (cc.nCount == 0 ){
          Serial.print("Stimulation On, Interval (RR) is  ");
          Serial.println(inter[0]);
          bStimulate=true;
        } else if (cc.nData[0]==0){
          Serial.println("Stimulation Off");
          bStimulate=false;          
        }else{
           Serial.print("Stimulation On, set RR to ");
           Serial.println(cc.nData[0]);
           setStimulation(cc.nData[0]);
           bStimulate=true;
        }   
        //restart internal trigger
        setTimer();
        trigTimer=now;
        break;                    
      case 't' :
        Serial.println("External Trigger Off");
        bExtTrig=false;
        //use internal Trigger
        trigTimer=now;
        break;
      case 'T':        
        Serial.println("External Trigger On");
        bExtTrig=true;
        break;
      case 'u' :        
        Serial.println("UV Light Off");
        lightSwitchOff();
        bFlash=false;
        break;
      case 'U':        
        bFlash=false;
        Serial.print("Permanent UV Light On, ");        
        //
        bExtTrig=false;
        //
        if (cc.nCount >0 ){
           Serial.println(" set AutoOff, ");
           if ( cc.nData[0] == 0 ){              
              bUvAuto=false;
           } else {
              bUvAuto=true;
              uvDuration=cc.nData[0];              
           }
        }
        
        if (bUvAuto){
          Serial.print("AutoOff in ");
          Serial.println( uvDuration );
        } else {
          Serial.println("AutoOff is off");
        }
        //
        bFlash=false;
        //
        lightSwitchOn();
        //
        break;
      case 'W':        
        if (cc.nCount >0 ){
            Serial.print("Set Puls width to "); Serial.print(cc.nData[0]);
            for (iRow=0; iRow<nRow; iRow++)
               pulse[iRow]=cc.nData[0];
        } else {
          Serial.print("Puls width is "); Serial.print(pulse[0]);
        }
        break;
      case 'X' :
        Serial.println("X On");
        //bPermanent=true;
        //bStimulate=true;
        //bExtTrig=true;
        //stimSwitchOn();
        //bFlash=true;
        //lightSwitchOff();        
        //restart internal trigger
        //trigTimer=now;
        
        break;
        
        case 'x' :
        Serial.println("X Off");
        bPermanent=false;
        bStimulate=false;
        bExtTrig=true;
        bFlash=true;
        lightSwitchOff();
        break;      
      default:;
    }    
  }
}
