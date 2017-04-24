
/*The MIT License (MIT)

  Copyright (C) 2015, Miguel Medeiros, Dominic Phillips

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
  THE SOFTWARE.

  http://www.esp8266.com/viewtopic.php?f=32&t=3780
  https://github.com/sandeepmistry/esp8266-Arduino/blob/master/esp8266com/esp8266/libraries/ESP8266WebServer/src/ESP8266WebServer.cpp
*/

// Include Librairies
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <FS.h>
#include <ArduinoOTA.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "Adafruit_NeoPixel.h"
#ifdef __AVR__
#include <avr/power.h>
#endif

//host name
#define HOSTNAME "CarlRobot - "

//SSID & pw
const char* ap_default_ssid = "Mufasa"; //<<SSID of network
const char* ap_default_psk = "pissword"; // pass

// IO Extender
#include <Wire.h>  // Wire.h library is required to use SX1509 lib
#include <sx1509_library.h>  // Include the SX1509 library
const byte SX1509_ADDRESS = 0x3E;  // SX1509 I2C address (00)
const byte interruptPin = 2; // not used yet
sx1509Class sx1509(SX1509_ADDRESS);

// Variables
byte lineSensorArray [5];

// LED Variables
#define PIN 5 // PinOut for LED Array
#define NUMPIXELS 1 // Number of RGB LEDs in Array
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int R = 128;
int G = 0;
int B = 0;
// LED Variables END

// General Variables
bool rMoving = false; // If the Robot is Moving
unsigned int rSpeed = 255; // Forward and Reverse Motor Speed
unsigned int rTSpeed = 190; // Left and Right Turning Speed
float rModifier = 1.0;

//motor pin #s
#define mot1PinA 0 // Brown A-IA  DO on nodemcu
#define mot1PinB 1  //Red  A -IB   D1 on nodemcu
#define mot2PinA 2  // orange  B - IA  = D2
#define mot2PinB 3  // yellow  B-IB = D3
// General Variables END

// Variables End

void setup() {
  String station_ssid = "";
  String station_psk = "";

  Serial.begin(115200);
  delay(100);

  Serial.println("\r\n");
  Serial.print("Chip ID: 0x");
  Serial.println(ESP.getChipId(), HEX);

  // Set Hostname.
  String hostname(HOSTNAME);
  hostname += String(ESP.getChipId(), HEX);
  WiFi.hostname(hostname);

  // Check WiFi connection
  if (WiFi.getMode() != WIFI_STA)
  {
    WiFi.mode(WIFI_STA);
    delay(10);
  }

  // ... Begin with sdk config.
  WiFi.begin();

  // Go into software AP mode & print IP.
  WiFi.mode(WIFI_AP);
  delay(10);
  WiFi.softAP(ap_default_ssid, ap_default_psk);
  Serial.print("IP address: ");
  Serial.println(WiFi.softAPIP());

  // Start OTA server.
  ArduinoOTA.setHostname((const char *)hostname.c_str());
  ArduinoOTA.begin();

  // LED Setup
  pixels.begin(); // This initializes the NeoPixel library.
  for (int i = 0; i < NUMPIXELS; i++) {
    pixels.setPixelColor(i, pixels.Color(R, G, B)); // Moderately bright green color.
    pixels.show(); // This sends the updated pixel color to the hardware
  }

  // SX1509 Setup
  Serial.println("Beginning Setup");
  sx1509.init();  // Initialize the SX1509, does Wire.begin()
  byte clockDivider = 1;
  sx1509.ledDriverInit(mot1PinA);
  sx1509.ledDriverInit(mot1PinA, clockDivider, LOGARITHMIC);
  sx1509.ledDriverInit(mot1PinB);
  sx1509.ledDriverInit(mot1PinB, clockDivider, LOGARITHMIC);
  sx1509.ledDriverInit(mot2PinA);
  sx1509.ledDriverInit(mot2PinA, clockDivider, LOGARITHMIC);
  sx1509.ledDriverInit(mot2PinB);
  sx1509.ledDriverInit(mot2PinB, clockDivider, LOGARITHMIC);
  // setup for QRE113 sensors
  sx1509.pinDir(10, OUTPUT);  // Set SX1509 pin 10 as an output - enables all IR leds

  sx1509.pinDir(11, INPUT);  // Set SX1509 pin 10 as an output - enables all IR leds
  sx1509.pinDir(12, INPUT);  // Set SX1509 pin 10 as an output - enables all IR leds
  sx1509.pinDir(13, INPUT);  // Set SX1509 pin 10 as an output - enables all IR leds
  sx1509.pinDir(14, INPUT);  // Set SX1509 pin 10 as an output - enables all IR leds
  sx1509.pinDir(15, INPUT);  // Set SX1509 pin 10 as an output - enables all IR leds
  // SX1509 Setup END

  //random delay
  delay(500);
  ChangeLEDColor(0, 128, 0);
}

void checkLineSensors () {
  // turn on IR LEDs
  sx1509.writePin(10, HIGH);  // Write pin HIGH
  // read pins
  lineSensorArray[0] = sx1509.readPin(11);  // read line sensor 0 (s5), nearest power switch
  lineSensorArray[1] = sx1509.readPin(12);  // read line sensor 1 (s4)
  lineSensorArray[2] = sx1509.readPin(13);  // read line sensor 2 (s1)
  lineSensorArray[3] = sx1509.readPin(14);  // read line sensor 3 (s3)
  lineSensorArray[4] = sx1509.readPin(15);  // read line sensor 4 (s2), farest from power switch
  if (lineSensorArray[1] == 1 && lineSensorArray[3] == 0)
  {
    Move(75, 0);
  }
  else if (lineSensorArray[1] == 0 && lineSensorArray[3] == 1)
  {
    Move(0, 75);
  }
  else if (lineSensorArray[1] == 0 && lineSensorArray[3] == 0)
  {
    Move(80, 80);
  }
  else if (lineSensorArray[1] == 1 && lineSensorArray[3] == 1 && rMoving)
  {
    Stop();
    ChangeLEDColor(0, 0, 128); // blue
  }
  // print results
  for (int i = 0; i < 5; i++)  {
    Serial.print (lineSensorArray[i]);
    Serial.print (" ");
  }
  Serial.println (" ");
  Serial.println (" ");
  // turn off IR LEDs
  sx1509.writePin(10, LOW);  // Write pin low
}

void loop() {
  ArduinoOTA.handle();
  yield();
  checkLineSensors ();
  delay (1);
}

// LEDs
void ChangeLEDColor(int r, int g, int b) {
  for (int i = 0; i < NUMPIXELS; i++) { // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
    pixels.setPixelColor(i, pixels.Color(r, g, b)); // Moderately bright green color.
    pixels.show(); // This sends the updated pixel color to the hardware
  }
}
// LEDs END

//Move function for the robot
void Move(int m1, int m2) { //m1 and m2 are ints from 0-100, which is % motors will run at.
  sx1509.pwm(mot1PinB, rSpeed * m1 / 100);
  sx1509.pwm(mot1PinA, 0);
  sx1509.pwm(mot2PinB, rSpeed * m2 / 100);
  sx1509.pwm(mot2PinA, 0);
  rMoving = true;
}//end Move

// Stop Code Parsing
void Stop() {
  sx1509.pwm(mot1PinA, 0);
  sx1509.pwm(mot1PinB, 0);
  sx1509.pwm(mot2PinA, 0);
  sx1509.pwm(mot2PinB, 0);
  rMoving = false;
  ChangeLEDColor(128, 128, 0);
}
// Stop Code Parsing END
