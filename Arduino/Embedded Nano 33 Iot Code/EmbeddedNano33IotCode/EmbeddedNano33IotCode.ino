
/*EmbeddedNano33IotCode.ino
 * 
 * By Matt Skarha
 * Input Devices and Music Interaction Laboratory, McGill University
 * 
 * Description: This Arduino sketch should be uploaded to 
 * the Arduino Nano 33 Iot embedded in each pendulum arm of 
 * Le Bâton. It communicates with Max/MSP by sending and receiving
 * WiFi messages using the WifiNINA library. It reads/sends 
 * IMU data and receives/outputs LED mappings.
 * 
 * Important to include your network's name and password in the
 * arduino_secrets.h header file. 
 * 
 * The following libraries need to be installed 
 * from Sketch -> Include Library -> Manage Libraries...:
 *     - "WiFiNINA" by Arduino
 *     - "Adafruit NeoPixel" by Adafruit
 *     - "Adafruit BNO055" by Adafruit
 *     - "Adafruit Unified Sensor" by Adafruit
 * 
 * The following board package needs to be installed
 * from Tools -> Board: "%%%" -> Boards Manager...
 *     - "Arduino SAMD Boards (32-bits ARM Cortex-MO+)" by Arduino
 */     


#include <SPI.h>
#include <WiFiNINA.h>
#include <WiFiUdp.h>
//#include <avr/dtostrf.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
 #include <avr/power.h> // Required for 16 MHz Adafruit Trinket
#endif
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BNO055.h>
#include <utility/imumaths.h>
#include "arduino_secrets.h"
  
Adafruit_BNO055 bno = Adafruit_BNO055(55);

int status = WL_IDLE_STATUS;

bool send_only_grav_accel_data = true; 

int out_port = 61640; // top = 61638, middle = 61639, bottom = 61640
 
///////please enter your sensitive data in the Secret tab/arduino_secrets.h
char ssid[] = SECRET_SSID;        // your network SSID (name)
char pass[] = SECRET_PASS;    // your network password (use for WPA, or use as key for WEP)
int keyIndex = 0;            // your network key Index number (needed only for WEP)

unsigned int localPort = 2390;      // local port to listen on

char packetBuffer[255]; //buffer to hold incoming packet
char  ReplyBuffer[] = "01001111";       // a string to send back
int rgbs[36];
unsigned char *ptr = NULL;

struct Led{
  int r;
  int g;
  int b;
};

Led led1;
Led led2;
Led led3;
Led led4;
Led led5;
Led led6;
Led led7;
Led led8;
Led led9;
Led led10;
Led led11; 
Led led12;

WiFiUDP Udp;

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1:
#define LED_PIN     16

// How many NeoPixels are attached to the Arduino?
#define LED_COUNT  12

// NeoPixel brightness, 0 (min) to 255 (max)
#define BRIGHTNESS 50

// Declare our NeoPixel strip object:
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRBW + NEO_KHZ800);
// Argument 1 = Number of pixels in NeoPixel strip
// Argument 2 = Arduino pin number (most are valid)
// Argument 3 = Pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
//  while (!Serial) {
//    ; // wait for serial port to connect. Needed for native USB port only
//  }
    Serial.println("Orientation Sensor Test"); Serial.println("");
  
  /* Initialise the sensor */
  if(!bno.begin())
  {
    /* There was a problem detecting the BNO055 ... check your connections */
    Serial.print("Ooops, no BNO055 detected ... Check your wiring or I2C ADDR!");
    while(1);
  }
  
  delay(1000);
    
  bno.setExtCrystalUse(true);

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }

  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION) {
    Serial.println("Please upgrade the firmware");
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }
  Serial.println("Connected to wifi");
  printWifiStatus();

  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  Udp.begin(localPort);
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)
}

void loop() {
  if (send_only_grav_accel_data) {
    sensors_event_t gravityData;
    bno.getEvent(&gravityData, Adafruit_BNO055::VECTOR_GRAVITY);
    Serial.print(F("Gravity_x: "));
    Serial.print((float)gravityData.acceleration.x);
    Serial.println(" ");
    Serial.print(F("Gravity_y: "));
    Serial.print((float)gravityData.acceleration.y);
    Serial.println(" ");
    uint8_t sys, gyro, accel, mag = 0;
    bno.getCalibration(&sys, &gyro, &accel, &mag);
    Serial.print(F("Calibration: "));
    Serial.print(sys, DEC);
    Serial.print(F(" "));
    Serial.print(gyro, DEC);
    Serial.print(F(" "));
    Serial.print(accel, DEC);
    Serial.print(F(" "));
    Serial.println(mag, DEC);
    char big[400];
    sprintf(big, "%f %f %i %i %i %i", gravityData.acceleration.x, gravityData.acceleration.y, sys, gyro, accel, mag);
    Udp.beginPacket(Udp.remoteIP(), out_port);
    Udp.write(big);
    Udp.endPacket();
    delay(50);
  }
  else{
    sensors_event_t orientationData , angVelocityData , linearAccelData, magnetometerData, accelerometerData, gravityData;
    bno.getEvent(&orientationData, Adafruit_BNO055::VECTOR_EULER);
    bno.getEvent(&angVelocityData, Adafruit_BNO055::VECTOR_GYROSCOPE);
    bno.getEvent(&linearAccelData, Adafruit_BNO055::VECTOR_LINEARACCEL);
    bno.getEvent(&magnetometerData, Adafruit_BNO055::VECTOR_MAGNETOMETER);
    bno.getEvent(&accelerometerData, Adafruit_BNO055::VECTOR_ACCELEROMETER);
    bno.getEvent(&gravityData, Adafruit_BNO055::VECTOR_GRAVITY);
    
    Serial.print(F("Orientation: "));
    Serial.print((float)orientationData.orientation.x);
    Serial.println(" ");
    Serial.print(F("Gyro: "));
    Serial.print((float)angVelocityData.gyro.x);
    Serial.println(" ");
    Serial.print(F("Gravity: "));
    Serial.print((float)gravityData.acceleration.x);
    Serial.println(" ");
    /* Also send calibration data for each sensor. */
    uint8_t sys, gyro, accel, mag = 0;
    bno.getCalibration(&sys, &gyro, &accel, &mag);
    Serial.print(F("Calibration: "));
    Serial.print(sys, DEC);
    Serial.print(F(" "));
    Serial.print(gyro, DEC);
    Serial.print(F(" "));
    Serial.print(accel, DEC);
    Serial.print(F(" "));
    Serial.println(mag, DEC);
  
    char big[4000];
    sprintf(big, "%f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %f %i %i %i %i", orientationData.orientation.x, orientationData.orientation.y, orientationData.orientation.z, angVelocityData.gyro.x, angVelocityData.gyro.y, angVelocityData.gyro.z, linearAccelData.acceleration.x, linearAccelData.acceleration.y, linearAccelData.acceleration.z, magnetometerData.magnetic.x, magnetometerData.magnetic.y, magnetometerData.magnetic.z, accelerometerData.acceleration.x, accelerometerData.acceleration.y, accelerometerData.acceleration.z, gravityData.acceleration.x, gravityData.acceleration.y, gravityData.acceleration.z, sys, gyro, accel, mag);
    //sprintf(big, "%f", event.orientation.x);
    Udp.beginPacket(Udp.remoteIP(), out_port);
    Udp.write(big);
    Udp.endPacket();
    delay(50);
  }
  int packetSize = Udp.parsePacket();
  if (packetSize) {
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remoteIp = Udp.remoteIP();
    Serial.print(remoteIp);
    Serial.print(", port ");
    Serial.println(Udp.remotePort());

    // read the packet into packetBufffer
    int len = Udp.read(packetBuffer, 255);
    if (len > 0) {
      packetBuffer[len] = 0;
    }
    Serial.println("Contents:");

    decodeMessage(packetBuffer);
//    Serial.println(led12.r);
//    Serial.println(led12.g);
//    Serial.println(led12.b);
    Led pendulum[12] = {led1, led2, led3, led4, led5, led6, led7, led8, led9, led10, led11, led12};
    
    for(int i=0; i<LED_COUNT; i++) { // For each pixel...

    strip.setPixelColor(i, strip.Color(pendulum[i].r, pendulum[i].g, pendulum[i].b));

    strip.show();   // Send the updated pixel colors to the hardware.

  }

  }
    
    
  }
      
    
void decodeMessage(char* messg)
{
  char buffer[255] = "";
  strcpy(buffer, messg);
  strtok(buffer, " ,");
  led1.r = atoi(strtok(NULL, " ,"));
  led1.g = atoi(strtok(NULL, " ,"));
  led1.b = atoi(strtok(NULL, " ,"));
  led2.r = atoi(strtok(NULL, " ,"));
  led2.g = atoi(strtok(NULL, " ,"));
  led2.b = atoi(strtok(NULL, " ,"));
  led3.r = atoi(strtok(NULL, " ,"));
  led3.g = atoi(strtok(NULL, " ,"));
  led3.b = atoi(strtok(NULL, " ,"));
  led4.r = atoi(strtok(NULL, " ,"));
  led4.g = atoi(strtok(NULL, " ,"));
  led4.b = atoi(strtok(NULL, " ,"));
  led5.r = atoi(strtok(NULL, " ,"));
  led5.g = atoi(strtok(NULL, " ,"));
  led5.b = atoi(strtok(NULL, " ,"));
  led6.r = atoi(strtok(NULL, " ,"));
  led6.g = atoi(strtok(NULL, " ,"));
  led6.b = atoi(strtok(NULL, " ,"));
  led7.r = atoi(strtok(NULL, " ,"));
  led7.g = atoi(strtok(NULL, " ,"));
  led7.b = atoi(strtok(NULL, " ,"));
  led8.r = atoi(strtok(NULL, " ,"));
  led8.g = atoi(strtok(NULL, " ,"));
  led8.b = atoi(strtok(NULL, " ,"));
  led9.r = atoi(strtok(NULL, " ,"));
  led9.g = atoi(strtok(NULL, " ,"));
  led9.b = atoi(strtok(NULL, " ,"));
  led10.r = atoi(strtok(NULL, " ,"));
  led10.g = atoi(strtok(NULL, " ,"));
  led10.b = atoi(strtok(NULL, " ,"));
  led11.r = atoi(strtok(NULL, " ,"));
  led11.g = atoi(strtok(NULL, " ,"));
  led11.b = atoi(strtok(NULL, " ,"));
  led12.r = atoi(strtok(NULL, " ,"));
  led12.g = atoi(strtok(NULL, " ,"));
  led12.b = atoi(strtok(NULL, " ,"));
  
  
}

  
void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your board's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}
