/*Uno.ino
 * 
 * By Matt Skarha
 * Input Devices and Music Interaction Laboratory, McGill University
 * 
 * Description: This Arduino sketch should be uploaded to 
 * the Arduino Uno that has the Pololu Dual G2 High-Power
 * Motor Driver 18v18 Shield for Arduino mounted on top
 * with appropriate soldering connections. Also, the 5V
 * Relay Module should be soldered to this Uno. See
 * circuit diagram for soldering instructions.
 * 
 * The purpose of this sketch is to read serial messages
 * from Max/MSP that come from the Sony DS4 controller over
 * Bluetooth. These messages correspond to how fast and which
 * direction the motor should turn. Also, if we are using the
 * motor, we need to power the electromagnetic clutch, which 
 * is done via the 5V relay module. 
 * 
 * The following libraries need to be installed 
 * from Sketch -> Include Library -> Manage Libraries...:
 *     - "DualG2HighPowerMotorShield" by Pololu
 * 
 * Be sure to select Arduino Uno from 
 * Tools -> Board: "%%%" -> Arduino AVR Boards

 */     

#include "DualG2HighPowerMotorShield.h"

DualG2HighPowerMotorShield24v14 md;

int value;

void stopIfFault()
{
  if (md.getM1Fault())
  {
    md.disableDrivers();
  delay(1);
    Serial.println("M1 fault");
    while (1);
  }
  if (md.getM2Fault())
  {
    md.disableDrivers();
  delay(1);
    Serial.println("M2 fault");
    while (1);
  }
}

void setup()
{
  Serial.begin(9600);
  Serial.println("Dual G2 High Power Motor Shield");
  md.init();
  md.calibrateCurrentOffsets();
  pinMode(13, OUTPUT);

  delay(10);
}

void loop() {
  md.enableDrivers();
  int x;
  int v1; // this variable specifies which direction to rotate
  int v2; // this variable specifies how fast to rotate in that direction
  
  if (Serial.available() > 1 ){
      v1 = Serial.read();
      v2 = Serial.read();
   
    if (v1 == 111) { 
      x = map(v2, 0, 255, 0, 400); // map range of bluetooth byte to range of motor speed
    }
    if (v1 == 112) {
      x = map(v2, 0, 255, 0, -400);
    }
    Serial.println(x);
    if (x != 0){ // if we want to use the motor
      md.enableDrivers(); 
      delay(1);  // The drivers require a maximum of 1ms to elapse when brought out of sleep mode.
      digitalWrite(13, HIGH); // power the electromagnetic clutch via the relay module
      md.setM1Speed(x); // set the motor speed and direction 
  } else {
    md.setM1Speed(0);
    digitalWrite(13, LOW); // unpower the clutch to let pendulum swing freely
  }
  }
  


}
