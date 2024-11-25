#include "serialCom.h"

// The Stim Board if Triggering the camera



unsigned int nInterval = 3;
unsigned const int nRow = 8;
unsigned int iRow = 0;
char deviceName[]="LightStimulator";
char deviceVersion[]="23";

//Boards Produced:
// 1: eStim 
// 2: eStim inverted               Stimulationsboard_Arduino_V5.7 UV an PB0
// 3: light Stim with micro switch Stimulationboard_Light_V3.0    UV an PD4
// 4: light Stim with Trigger      Stimulationboard_Light_trig    UV an PD4

int boardVersion=4;


unsigned int uvFlash=8000; //uv Flash Duration in us
unsigned long long now;
unsigned long long last;
unsigned long long uvTimer;
unsigned long long trigTimer;
unsigned long long iterationCounter;
unsigned long long iterationTimer;
unsigned long long milli_to_micro = 1000;
unsigned long long sec_to_micro = 1000*milli_to_micro;

float micro_to_milli = 0.001;
unsigned long long ul_tmp=0.0;

/* 
 *                 set Default Stimulation 
 */

//unsigned long stimInterval[]   = {1000000, 1000000,  500000,  500000,  333333,  333333,  200000,  200000}; // Time period between Stimulations in us
//unsigned long stimInterval[]   = {1000000, 1000000, 1000000, 1000000, 1000000, 1000000, 1000000, 1000000}; // Time period between Stimulations in us
unsigned long stimInterval[]   = {  333333,  333333,  333333,  333333, 333333,  333333,  333333,  333333}; // Time period between Stimulations in us
unsigned long pulseDuration[]  = {   2000,    2000,    2000,    2000,    2000,    2000,    2000,    2000}; // Flash duration in ms
unsigned long pulseInterval[]  = {   4000,    4000,    4000,    4000,    4000,    4000,    4000,    4000}; // Flash interval in ms


/*
 * Have Stimulation 15sec on and 15sec off
 */






bool          bPulseOn[]       = {   0,    0,    0,    0,    0,    0,    0,    0}; // Flash interval in ms
bool          bActive[]        = {   1,    1,    1,    1,    1,    1,    1,    1}; // mark active rows
unsigned int  iFlash[]         = {   0,    0,    0,    0,    0,    0,    0,    0}; // Number of flashes per stimulation
unsigned int  nFlash[]         = {   2,    2,    2,    2,    2,    2,    2,    2}; // Number of flashes per stimulation

unsigned long long stimTimer[]      = {   1,    1,    1,    1,    1,    1,    1,    1};
unsigned long long pulseOnTimer[]   = {   1,    1,    1,    1,    1,    1,    1,    1};
unsigned long long pulseOffTimer[]  = {   1,    1,    1,    1,    1,    1,    1,    1};
unsigned long long toggleTimer[]    = {   1,    1,    1,    1,    1,    1,    1,    1}; // Time period between Stimulations in us



           //ROW                         A       B          C         D          E         F           G      H                  
           //NAMES Pos                  PD3     PD6        PB4       PB6        PC7       PF6         PF4    PF0
           //nameNeg                    PD2     PD4        PD7       PB5        PC6       PF7         PF5    PF1
volatile unsigned char * portPos[] = { &PORTD, &PORTD,   &PORTB,   &PORTB,    &PORTC,    &PORTF,    &PORTF, &PORTF};
volatile unsigned char * portNeg[] = { &PORTD, &PORTD,   &PORTD,   &PORTB,    &PORTC,    &PORTF,    &PORTF, &PORTF};

volatile unsigned char *  dirPos[] = {  &DDRD,  &DDRD,    &DDRB,    &DDRB,    &DDRC,     &DDRF,      &DDRF,  &DDRF};
volatile unsigned char *  dirNeg[] = {  &DDRD,  &DDRD,    &DDRD,    &DDRB,    &DDRC,     &DDRF,      &DDRF,  &DDRF};

const unsigned char pinPos[] = { 0b00001000,0b01000000,0b00010000,0b01000000,0b10000000,0b01000000,0b00010000,0b00000001};
const unsigned char pinNeg[] = { 0b00000100,0b00010000,0b10000000,0b00100000,0b01000000,0b10000000,0b00100000,0b00000010};

//                                       PD2
unsigned char pinTrig              = 0b00000010;
unsigned char intTrig              =    2;          //trigger interrupt
volatile unsigned char *  portTrig =   &PORTD;
volatile unsigned char *   dirTrig =   &DDRD;
volatile unsigned char *    portIn =   &PIND;

//                                       PD4
unsigned char pinUV                = 0b00010000;
volatile unsigned char *  portUV   =   &PORTD;
volatile unsigned char *  dirUV    =    &DDRD;


//int trigPin = 2; //LED Stim Board = 4 eStim=8
//int uvPin   = 8; //LED Stim Board = 4 eStim=8

/*********************************************************
***      Set Defaults                                  ***
*********************************************************/

int bLight      = false;
int bFlash      = false;
int bStimulate  = true;
int bPermanent  = false;
int bSequential = false;
int bExtTrig    = false;

/*******************************************************/

int trigRate = 50;   //trigger Rate in ms   50ms = 20Hz

bool bElectrical = false;

// Tissues should not be lit for more that ~3mins
// If longe exposure is needed set bUvAuto=false
// or uvDuration to whatever value
int bUvAuto      = 1;
unsigned long uvDuration     = 180;       //turn UV Off after 180 sec
unsigned long long uvOffTime = 0;

char sbuf[50];
unsigned int dd;

int stimPinOffState=HIGH;
//int uvPinOffState=LOW;

void lightOn(void);



void setTimer() {  
  Serial.println("set Timer");
  stimTimer[0]=now;
  if (bSequential){
    for (iRow = 1; iRow < nRow; iRow++)
       stimTimer[iRow] = stimTimer[iRow-1]+nFlash[iRow-1]*pulseInterval[iRow];
  } else {
    for (iRow = 1; iRow < nRow; iRow++)
       stimTimer[iRow] = stimTimer[iRow-1];    
  }
  
  for (iRow = 0; iRow < nRow; iRow++){
    pulseOnTimer[iRow] =stimTimer[iRow];    
    pulseOffTimer[iRow]=pulseOnTimer[iRow]+pulseDuration[iRow];    
    toggleTimer[iRow] = pulseOffTimer[iRow];
    Serial.print(" Row: ");Serial.print(iRow);
    Serial.print(" Now: ");Serial.print( (unsigned long) now);
    Serial.print(" stimTimer: ");Serial.print( (unsigned long) stimTimer[iRow]);
    Serial.print(" pulseOnTimer: "); Serial.print( (unsigned long) pulseOnTimer[iRow]);
    Serial.print(" pulseOffTimer: ");Serial.print( (unsigned long) pulseOffTimer[iRow]);
    Serial.println();
  }
}

void setup() {
  TXLED0;
  RXLED0;  
  // Higher speeds do not appear to be reliable
  now=micros();
  //Serial.begin(9600);
  Serial.begin(115200);
  //DDRD   =  DDRD | 0b00010000; //to use portd bit4 that is pin25 of MEGA32U
  //PORTD &=         0b11101111; //set to 0 will light the UV  
  switch (boardVersion){
    case 1:   //eStim
      //
      bElectrical=1;
      bSequential=1;
      break;
    case 2:  //eStim inverted
      bElectrical=1;
      bSequential=1;     

      break;
    case 3:  //lightStim with micro switch
      bElectrical=0;
      break;
    case 4:  //lightStim with trigger
      bElectrical=0;      
      break;
    default:;    
  }
  //
  /*
   * set pin input
   */
  *dirTrig  &=  ~pinTrig;
  //set trigger pin high (Pullup)
  *portTrig |= pinTrig;  
  // set Interrupt 
  attachInterrupt(intTrig, getTrigger, CHANGE);
  
  // set UV pin out
  *dirUV   |= pinUV;
  // set stimulation pins
  for (int i=0; i< nRow;i++)
    *dirPos[i] |= pinPos[i];
  //  
  uvTimer  =now;
  trigTimer=now;
  lightOff();
  setTimer();
  iterationTimer=now;
  iterationCounter=0;
}

void getTrigger() {
  if ( (*portIn & pinTrig) != 0   ){
      if (bFlash){
        sendUvFlash();
      }
  } else { //FALLING      
      if (bStimulate) {
        startPulse();
      }
  }  
}

void startPulse(){
    //turn lights on
    for (int iRow=0; iRow < nRow; iRow++) {
      if (! bActive[iRow])
         continue;
      if (now < stimTimer[iRow])
         continue;                  
      if (! bPulseOn[iRow] && now >= pulseOnTimer[iRow]){
        //start pulse        
        bPulseOn[iRow]=true;
        *portPos[iRow]&=~pinPos[iRow]; // on        //digitalWrite (pinPos[iRow], LOW);
        pulseOffTimer[iRow]= now + pulseDuration[iRow];
        pulseOnTimer[iRow] = now + pulseInterval[iRow];
      }
  }  // end rows
}


void sendUvFlash(){
  lightOn();
  delayMicroseconds(uvFlash);
  //delay(uvFlash);
  lightOff();
}

void stimSwitchOn(void) {  
  bStimulate=true;
  for (int iRow=0; iRow < nRow; iRow++)  stimOn(iRow);
}

void stimSwitchOff(void) {  
  bStimulate=false;
  for (int iRow=0; iRow < nRow; iRow++)
      stimOff(iRow);      
  //  
}

void stimOff(int iRow) {  
      *portPos[iRow]|=pinPos[iRow]; // off
}

void stimOn(int iRow) {  
      *portPos[iRow]&=~pinPos[iRow];
}

void setStimulation(int rr){
  for (int iRow=0; iRow < nRow; iRow++) {
      stimInterval[iRow]=rr;
  }  
}

void setPulseDuration(int pd){
  for (int iRow=0; iRow < nRow; iRow++) {
      pulseDuration[iRow]=pd*milli_to_micro;
  }  
}

void setPulseNumber(int n){
  for (iRow=0; iRow < nRow; iRow++) {
    nFlash[iRow] = n;
  }
}

void lightOn(void) {  
  *portUV &= ~pinUV;  
}

void lightOff(void) {
  *portUV |= pinUV;
}



void lightSwitchOn(void) {  
  uvOffTime = now + uvDuration * sec_to_micro;
  lightOn();
  bLight=1;
}

void lightSwitchOff(void) {  
  bLight=0;
  lightOff();
}

void stopPulse(){
   //turn lights off
   for (int iRow=0; iRow < nRow; iRow++) {
      if (! bActive[iRow])
         continue;
      if (now < stimTimer[iRow])
         continue;                  
      if ( bPulseOn[iRow] && now >= pulseOffTimer[iRow]){
        //end pulse
        bPulseOn[iRow]=false;
       *portPos[iRow]|=pinPos[iRow]; // off //digitalWrite (pinPos[iRow], HIGH);
       iFlash[iRow]++;
      }
      //
      if (iFlash[iRow] == nFlash[iRow]){  //end of pulse train
        iFlash[iRow]=0;        
        stimTimer[iRow]   =stimTimer[iRow] + stimInterval[iRow];
        pulseOnTimer[iRow]=stimTimer[iRow];        
     }      
   }  // end rows
}

void readSerial(MY_CMD cc);


void loop() {
  //  
  last=now;
  now=micros();
  // work-around for micros overflow issue (every 70mins)
  if (last > now)
    setTimer();  
  //
  iterationCounter++;
  //
  if (bUvAuto && now > uvOffTime)
    lightSwitchOff();
  //
  if (bLight) {
    lightOn();
  } else {
    lightOff();
  }   
  if (! bExtTrig  && bStimulate)
      startPulse();
  // in case pulse start is triggered externally check if is due stopping
  if (! bPermanent)
      stopPulse();
 
  if (Serial.available() > 0){
    MY_CMD cc;
    myGET_CMD(cc);
    readSerial(cc);
  }  
}

void printLongLong(uint64_t n){
  //Print unsigned long long integers (uint_64t)
  //CC (BY-NC) 2020
  //M. Eric Carr / paleotechnologist.net
  unsigned char temp;
  String result=""; //Start with a blank string
  if(n==0){Serial.println(0);return;} //Catch the zero case
  while(n){
    temp = n % 10;
    result=String(temp)+result; //Add this digit to the left of the string
    n=(n-temp)/10;      
    }//while
  Serial.println(result);
}

void readSerial(MY_CMD cc){
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
      case 'I':        
        Serial.print("IterationCounter: ");
        Serial.print( (long) iterationCounter);
        Serial.print(" IterationTimer: ");
        Serial.print( (long unsigned ) iterationTimer );
        Serial.print(" ms, Time per Iteration: ");
        Serial.print((long unsigned) ((now-iterationTimer) / iterationCounter));
        Serial.println("  µs per iteration");
        iterationCounter=0;
        iterationTimer=now;
        break;  
      case 'm':        
        //return now to calculate timer offset
        Serial.println( (unsigned long) now);
        break;
      case 'n':        
        Serial.println("UV AutoOff Off");
        //bLight=0;
        bUvAuto=false;
        break;
      case 'N':                     //can be used for tetanic Stimulation 200*50fmp=4 sec tetanus
         if (cc.nCount ==0 ){
           Serial.print("Pulse Duration is ");
           Serial.println(pulseDuration[0]);         
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
            Serial.print(pulseDuration[0]);
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
        //bStimulate=1;
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
        stimSwitchOff();
        bPermanent=false;
        break;
      case 'S':
        //
        if (cc.nCount ==1) {
           //unsigned long long stimInterval[]   = {1000000, 1000000, 1000000, 1000000, 1000000, 1000000, 1000000, 1000000}; // Time period between Stimulations in us
            ul_tmp = milli_to_micro*cc.nData[0];
            Serial.print("Set Stimulation Interval (RR) to ");
            Serial.print((float) ul_tmp);
            Serial.println(" us");
            for (int iRow=0; iRow < nRow; iRow++)
                   stimInterval[iRow]=ul_tmp;
        } else if (cc.nCount ==2) {
            ul_tmp = milli_to_micro*cc.nData[0];
            iRow   = cc.nData[1]-1;
            Serial.print("Set Stimulation Interval (RR) if Row ");
            Serial.print(iRow+1);
            Serial.print(" to ");
            Serial.print((float) ul_tmp);
            Serial.println(" us");            
            stimInterval[iRow]=ul_tmp;            
        }
        Serial.println("Stimulation On, Interval (RR) is:");
        for (iRow=0; iRow<nRow; iRow++)
              printLongLong(stimInterval[iRow]);
        //      
        bStimulate=true;
        bPermanent=false;
        //restart internal trigger
        setTimer();
        //stimSwitchOn();
        Serial.print("bSequential "); Serial.println(bSequential);
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
               pulseDuration[iRow]=cc.nData[0]*milli_to_micro;
        } else {
          Serial.println("Puls width is "); 
          for (iRow=0; iRow<nRow; iRow++)
            Serial.println( pulseDuration[iRow] );
        }
        break;      
      default:;
    }    
  }
  
