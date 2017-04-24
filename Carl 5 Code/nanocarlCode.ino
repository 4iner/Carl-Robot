//Libraries
#include <IRremote.h>
#include <IRremoteInt.h>

///////////////////////REMOTE CONTROL HEX VALUES//////////////////////////// use these to add another 'case' if you would like to use more buttons on the IR remote control
#define rc_chm 0xFFA25D
#define rc_ch 0xFF629D
#define rc_chp 0xFFE21D
#define rc_rw 0xFF22DD
#define rc_ff 0xFF02FD
#define rc_pr 0xFFc23D
#define rc_minus 0xFFE01F
#define rc_plus 0xFFA857
#define rc_EQ 0xFF906F
#define rc_0 0xFF6897
#define rc_100p 0xFF6897
#define rc_200p 0xFFB04F
#define rc_1 0xFF30CF
#define rc_2 0xFF18E7
#define rc_3 0xFF7A85
#define rc_4 0xFF10EF
#define rc_5 0xFF38C7
#define rc_6 0xFF5AA5
#define rc_7 0xFF42BD
#define rc_8 0xFF4AB5
#define rc_9 0xFF52AD
//////////////////////END OF REMOTE CONTROL HEX VALUES//////////////////////

//pins
#define IRPIN2 2 //TCRT pin
#define IRPIN1 3 //TCRT pin 
#define SPKR 4 //speaker pin
#define TRIGPIN 5 //ultrasonic trigger pin
#define ECHOPIN 6 //ultrasonic echo pin
#define RECVPIN 8 //ultrasonic receiver pin
#define MPIN1B A0 //motor 1 pin B
#define MPIN1A A1 //motor 1 pin A
#define MPIN2A A2 //motor 2 pin A
#define MPIN2B A3 //motor 2 pin B
#define GPIN A5 //LED green pin
#define BPIN A6 //LED blue pin
#define RPIN A7 //LED red pin

//variables
int rSpeed = 255; //robot's max speed (used in Move() calculations)
int distance = 0; //distance recorded by Ultrasonic
int modeClr[] = {0 , 0 , 0}; //rgb array for color of current mode
IRrecv irrecv(RECVPIN); //IR remote variables
decode_results results; //decoder
int irSpeed1 = 0; //remote speeds
int irSpeed2 = 0;
int TS1 = 80;  //turn speed 1: the speed at which the dominating motor turns at
int TS2 = -75; //turn speed 2: the speed at which the turning motor turns at
int FS = 90;   //forward speed: the speed at which both motors turn at
int carlMode = 0; //carl modes. 0 = line following, 1 = IR RC mode, 2 = Ultrasonic mode. 0 is the default mode so the robot will always be in line follow mode

void setup() {
  ChangeLEDColor(128, 0, 0); //red on startup
  Serial.begin(115200); //serial for serial port
  pinMode(ECHOPIN, INPUT); //dont mind these
  pinMode(TRIGPIN, OUTPUT);
  pinMode(IRPIN1, INPUT);
  pinMode(IRPIN2, INPUT);
  pinMode(SPKR, OUTPUT);
  irrecv.enableIRIn();
  digitalWrite(SPKR, HIGH);
  delay(1000);
  digitalWrite(SPKR, LOW);
  ChangeLEDColor(0, 255, 0); //green on good startup
}

void checkUltrasonic() { //check distance infront of ultrasonic sensor
  long duration, cm;
  digitalWrite(TRIGPIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIGPIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIGPIN, LOW);
  duration = pulseIn(ECHOPIN, HIGH); //duration is now a value in milliseconds of how long it took for a sound wave to exit the sensor, and bounce off of an object back into the sensor
  cm = duration / 34 / 2; //using the speed of sound, we find distance from object infront of us. look up "basic kinematics : finding distance using velocity and time"
  distance = (int)cm;
  //Serial.println(distance);
}

void checkLineSensors() {//this function reads TCRTs for 1 and 0 (0 = line, 1 = white) and turns motors accordingly
  // read pins
  int ir1 = digitalRead(IRPIN1);
  int ir2 = digitalRead(IRPIN2);
  Serial.print(ir1);
  Serial.print("left sensor & right sensor");
  Serial.println(ir2); //test print
  //follow the line
  if (ir1 == 1 && ir2 == 0)
  {
    Move(TS1, TS2); //if you don't know what these mean, pls read whole code up there
  }
  else if (ir1 == 0 && ir2 == 1)
  {
    Move(TS2, TS1);
  }
  else if (ir1 == 0 && ir2 == 0)
  {
    Move(FS, FS);
  }
  else if (ir1 == 1 && ir2 == 1)
  {
    Move(0, 0);
  }
}

void loop() {
  checkRemote(); //always check for remote
  switch (carlMode) { //do actions according to current carlMode
    case 0:
      checkLineSensors();
      modeClr[0] = 0; modeClr[1] = 255; modeClr[2] = 0;
      break;
    case 1:
      modeClr[0] = 255; modeClr[1] = 50; modeClr[2] = 0;
      Move(irSpeed1, irSpeed2);
      break;
    case 2:
      modeClr[0] = 50; modeClr[1] = 255; modeClr[2] = 0;
      Move(80, 80);
      checkUltrasonic();
      while (distance < 10) { //robot turns left if it sees something in front of it that is too close
        Move(-75, 75);
        delay(25);
        checkRemote();   //check Remote so that it doesn't get stuck in an infinite loop
        checkUltrasonic();
      }
      break;
  }
}
void ChangeLEDColor(int R, int G, int B) {
  analogWrite(RPIN, R);
  analogWrite(GPIN, G);
  analogWrite(BPIN, B);
}

void checkRemote() {
  if (irrecv.decode(&results)) { //if the receiver received a signal, it will decode it to one of the cases below. if you would like to 
    Serial.println(results.value, HEX);//add your own 'case', or button from the IR remote to put it simply, find the variable for your 
    switch (results.value) { //button from the HEX list at the top of this code (for example, rc_2 means the remote control button 2), and
      case 0xFFA25D:          //just type case <button>: <code> break;        where <button> is your button, <code> is what you want to happen
        Serial.println("HEYY IM WORKING!");  //when the button is pressed, and break; must always be typed after your code is done.
        break;
      case rc_2:
        irSpeed1 = 80;
        irSpeed2 = 80;
        break;
      case rc_4:
        irSpeed1 = 0;
        irSpeed2 = 80;
        break;
      case rc_6:
        irSpeed1 = 80;
        irSpeed2 = 0;
        break;
      case rc_8:
        irSpeed1 = -80;
        irSpeed2 = -80;
        break;
      case rc_5:
        irSpeed1 = 0;
        irSpeed2 = 0;
        break;
      case rc_1:
        irSpeed1 = 60;
        irSpeed2 = 80;
        break;
      case rc_3:
        irSpeed1 = 80;
        irSpeed2 = 60;
        break;
      case rc_7:
        irSpeed1 = -60;
        irSpeed2 = -80;
        break;
      case rc_9:
        irSpeed1 = -80;
        irSpeed2 = -60;
        break;
      case rc_pr: //switching between line follow / IR remote mode
        carlMode++;
        Serial.println(carlMode);
        if (carlMode > 2) carlMode = 0;
        irSpeed1 = 0;
        irSpeed2 = 0;
        break;
    }
    irrecv.resume();
  }
}

void Move(int m1, int m2) { //m1 and m2 are ints from 0-100, which is % motors will run at.
  if (m1 < 0) {             //only one pin of the motors needs to get an analogWrite(because current will only move from a high to low potential so giving both pins high would defeat the purpose of electronics)
    analogWrite(MPIN1A, rSpeed * abs(m1) / 100);
    analogWrite(MPIN1B, 0);
  }
  else
  {
    analogWrite(MPIN1A, 0); 
    analogWrite(MPIN1B, rSpeed * m1 / 100);
  }
  if (m2 < 0) {
    analogWrite(MPIN2A, rSpeed * abs(m2) / 100);
    analogWrite(MPIN2B, 0);
  }
  else
  {
    analogWrite(MPIN2A, 0);
    analogWrite(MPIN2B, rSpeed * m2 / 100);
  }
  if (abs(m1) > 0 || abs(m2) > 0) ChangeLEDColor(255, 255, 0); //only change led if one of the motors is moving
  else ChangeLEDColor(modeClr[0], modeClr[1], modeClr[2]);
}

