/* ELEC 439:SENSOR FALL 2016
 * PROJECT: BUILDING LIGHTING CONTROL
 * PROJECT VIDEO: https://youtu.be/iQTKwuidgeo
 * 
 * MERT ATILA SAKAOGULLARI
 * Yusuf Orkun Yenikaya
 * 
 * 
 * Required Libraries:
 *  1-Blynk:
 *   https://github.com/blynkkk/blynk-library/releases/tag/v0.4.3
 *  2-Simple Timer:
 *   https://github.com/schinken/SimpleTimer
 */
 
//---------------------------------------------------------PINS----------------------------------------------------------------

//Light Pins
      int LightEn = 3; //LightBulb's Enable for setting the gradualt brightness
      int LightIN1 = 4;
      int LightIN2 = 5;
      
//Motor Pins
      int MotorEn = 6; //Motor's Enable, which determines it's speed and which we will set to max because our system is is based on it's working time
      int MotIN3 = 7; //This two inputs set the direction of the curtain motor
      int MotIN4 = 8;
   
//Photoresistor Pins
      int WallRight = A0;
      int WallLeft = A1;
      int BottomRight = A2;
      int BottomLeft = A3;
      int WindowRight = A4;
      int WindowLeft = A5;
//------------------------------------------------------VARIABLES---------------------------------------------------------------

      int InsBr = 0; //Brightness of inside
      int OutBr = 0; //Brightness of outside
      double c=0; //The position data of our curtain at that moment, c is on a scale from 0 to cs, in seconds at time domain
      int UpLight= 0; //Maximum value for light brightness we can obtain in the house
      int LowLight= 1023; //Minimum value for light brightness we can obtain in the house

//What we should calibrate:
      double cs = 1.5; //Time needed for opening or closing curtain fully, (in seconds)
      int DesUpTh = 50; //Upper Theshold of the desired interval
      int DesLowTh = 100; //Lower Theshold of the lower interval



/////BLYNK
#define BLYNK_PRINT Serial
#include <SPI.h>
#include <Ethernet.h>
#include <BlynkSimpleEthernet.h>
#include <SimpleTimer.h>
char auth[] = "09f00cd10a4e49cc954a6322a8467aa4";

SimpleTimer timer;

void myTimerEvent()
{
  Blynk.virtualWrite(V0, InsBr);
  Blynk.virtualWrite(V2, OutBr);
}

void setup() {

//Light Pins
  pinMode(LightEn,OUTPUT);
  pinMode(LightIN1,OUTPUT);
  pinMode(LightIN2,OUTPUT);
   
//Motor Pins
  pinMode(MotorEn,OUTPUT); 
  pinMode(MotIN3,OUTPUT);
  pinMode(MotIN4,OUTPUT); 
  
  Serial.begin(9600);

  Blynk.begin(auth);   /////BLYNK, connecting to Blynk server with our token, which is special for our project
  timer.setInterval(1000L, myTimerEvent); //  Here we set interval to 1sec and which function to call
  }

  BLYNK_WRITE(V1) {  /////BLYNK, GettingData from user through mobile interface. 
  //V1 is the virtual pin, connected to the slider
  int pinValue = param.asInt(); 
  
   if(pinValue>1) {
    DesUpTh=pinValue*90+50;
    DesLowTh=pinValue*90-50;
    } else {
       DesUpTh=0;
       DesLowTh=50;
       }
} 
  

void loop() {
  
  Blynk.run(); /////BLYNK
  timer.run(); // Initiates SimpleTimer
  
  //--Measuring average brightness
  InsBr = (analogRead(WallRight)+analogRead(WallLeft)+analogRead(BottomLeft)+analogRead(BottomRight))/4;
  OutBr = (analogRead(WindowRight)+analogRead(WindowRight))/2;
  
  int diff = ((DesUpTh+DesLowTh)/2)-InsBr; //understanding how much we are away from the desired interval so that we can give the optimum reaction

  //Giving reaction depending on the outside conditions
  if (InsBr>DesUpTh){ 
      Light(0); 
      if(OutBr>DesUpTh){
          diff = map(diff, 0, 1023, cs, 0); //We carry the diff at photresistors' interval to curtain's time interval to know our step size
          CurtainDown(diff);
          }; 
    } 
      else if (InsBr<DesLowTh){
        diff = map(diff, 0, 1023, 0, 255); //We carry the diff at photresistors' interval to Enablers' interval to know our step size
        Light(diff);
        if(OutBr>DesLowTh){
           CurtainUp(cs); //collect the curtain completely to benefit outside's light maximum
        }
      };
}

void CurtainUp(double t){  //collects curtain
    //t is time is seconds, for how long we want the curtain motor to work
    //we need a quick check
    if(c+t>cs){ //curtain should not exceed it's final position
      t=cs-c;
      }
    analogWrite(MotorEn, 200);
    digitalWrite(MotIN3,LOW);
    digitalWrite(MotIN4,HIGH);
    delay(t*1000); 
    digitalWrite(MotorEn, 0);
    c=c+t;  //updating the curtain's position
}

void CurtainDown(double t){  //release curtain
   //we need another quick check
    if(c-t<0){ //curtainmotor should not collect the curtain when it is fully collected
      t=c;
      }
    analogWrite(MotorEn, 121);
    digitalWrite(MotIN3,HIGH);
    digitalWrite(MotIN4,LOW);
    delay(t*1000); 
    digitalWrite(MotorEn, 0);
    c=c-t; //updating the curtain's position
}

void Light(int t){ //Function for increasing Light Brightness
    analogWrite(LightEn, t);
    digitalWrite(LightIN1,LOW);
    digitalWrite(LightIN2,HIGH);
}
