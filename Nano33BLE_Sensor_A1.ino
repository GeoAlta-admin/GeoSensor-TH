/*
  Temp and Humidity Sensor
 */
#include <ArduinoBLE.h>
#include "DHT.h"
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

const char* GEODEVICENAME = "GeoSense";
const char* GEOCOMPANYNAME = "GeoComfort";
const char* GEOMODELNUMBER = "V1";
const char* GEOFIRMWARENUMBER = "0.1.0";

const int PINBLUETX = 3;      // Digital pin connected to the TX port of the Bluetooth module
const int PINBLUERX = 2;      // Digital pin connected to the RX port of the Bluetooth module
const int PINSENSORDATA = 4;  // Digital pin connected to the DHT sensor
const int PINBUTTON = 5;      // Digital pin connected to the BUTTON

//const int ledPin = 10;
const int ledPin = LED_BUILTIN; // set ledPin to on-board LED

const uint8_t buttonPress[2] = {0x00, 0x01};
unsigned int cycleTimeSeconds = 5000; //Milliseconds
float TemperatureDELTA = 0.2;
float HumitityDELTA = 5.0;
float HeatIndexDELTA = 0.5;
int BATTERYLEVELDELTA = 2;

/*
  UUID's
  Service   161D41C4-0974-4F62-8DC3-E61C3DCAD1D5
 */
const char* serviceUuid = "161DFFF1-0974-4F62-8DC3-E61C3DCAD1D5";
//const char* tempCUuid = "161D41C5-0974-4F62-8DC3-E61C3DCAD1D5";
//const char* tempFUuid = "161D41C6-0974-4F62-8DC3-E61C3DCAD1D5";
//const char* humidUuid = "161D41C7-0974-4F62-8DC3-E61C3DCAD1D5";
//const char* hidxCUuid = "161D41C8-0974-4F62-8DC3-E61C3DCAD1D5";
//const char* hidxFUuid = "161D41C9-0974-4F62-8DC3-E61C3DCAD1D5";

// struct of the http request
struct SensorReadings {
  float TemperatureC = 0.0; // For Celcius temperature //  float TemperatureF = 0; // For Fahrenheit temperature
  float Humidity = 0.0;    // For the URL we look for COOL
  float HeatIndexC = 0.0; // For Celcius heat index //  float HeatIndexF = 0; // For Fahrenheit heat index
};

// Function Prototypes.
bool initBluetooth();
void ReportBatteryLevel();
void initTempHumitity();
bool takeReading(SensorReadings &myReadings);
void ReportSensorReadings(SensorReadings &myReadings);
void setFloatCharacteristic (BLECharacteristic bleChar, float val, int dec);
void connectHandler(BLEDevice central);
void disconnectHandler(BLEDevice central);
void checkClientValues();
void onSetCycleTimeWritten(BLEDevice central, BLECharacteristic characteristic);
void onSetTempDeltaWritten(BLEDevice central, BLECharacteristic characteristic);
void onSetHumidityDeltaWritten(BLEDevice central, BLECharacteristic characteristic);
void onSetHeatIndexDeltaWritten(BLEDevice central, BLECharacteristic characteristic);
void onSetBatteryDeltaWritten(BLEDevice central, BLECharacteristic characteristic);
void onSetKeepAliveTimerWritten(BLEDevice central, BLECharacteristic characteristic);


// variables will change:
//int buttonState = 0;  // variable for reading the pushbutton status
SensorReadings lastSensorReading;

float lastBatteryLevelReported = 0;
unsigned long lastReadingTime = millis();
unsigned long newReadingTime;
unsigned long lastBroadcastTime; // Used to ensure we don't loose connection.
//unsigned long time;
unsigned long keepalivecycle = 100000; // initiate with 2 minutes.
unsigned long keepalivetimer = 0;




void setup() {
  Serial.begin(9600);
  delay(2000);

  Serial.println("Startup ... ");
  pinMode(PINBUTTON, INPUT); // initialize the pushbutton pin as an input:

  initTempHumitity();

  if (!initBluetooth()){
    // We don't have Bluetooth working.
    Serial.println("Bluetooth initialization didn't work.");
  }
/*
  if (takeReading(mySensorReadings) == false){
    Serial.print(".");
    Serial.println("Didn't get a temerature reading.");
  } else {
    Serial.print("|");
    ReportBatteryLevel();
    ReportSensorReadings(mySensorReadings);
  }
*/

  Serial.println("Startup Completed.");
}

/*
  Continuous running

  NOTE: Do not use "delay" in any of the loop or functions as this will 
        cause 
 */
void loop() {
  SensorReadings mySensorReadings;
  //BLE.poll(); // poll for Bluetooth® Low Energy events
  BLEDevice central = BLE.central();

  // if a central is connected to the peripheral:
  if (central) {
    while (central.connected()){
      /* There are two considerations here. 
        The first is that we need to check for a button press and that means we can't 
        be "delay"ing for to long. If we don't delay though and need to write to the 
        Nano a new firmware it might not accept it.

        The second is that we don't want to send Bluetooth messages every millisecond
        as that might just be to much.

        To accomplish all this we do not do a delay (the Delay funciton causes Bluetooth connection issues)
        but rather just have a clock value check for the sensor readings.
      */
      // Check for anything that the Client/Hub is sending to the Peripheral
      checkClientValues();

      // check if the pushbutton is pressed. If it is, the buttonState is HIGH:
      checkButtonState(); // TODO ***** After the button has been pressed we want to reset.

/*
      if (buttonState == HIGH) {
        // turn LED on: 
        digitalWrite(ledPin, HIGH);
        //Serial.print("BP");
        buttonState = LOW;
      } else {
        // turn LED off:
        digitalWrite(ledPin, LOW);
      }
*/
      newReadingTime = millis();
      if (newReadingTime - lastReadingTime >= cycleTimeSeconds){
        // We can take another reading
        if (takeReading(mySensorReadings) == false){
          Serial.println("Didn't get a temerature reading.");
        } else {
          ReportBatteryLevel();
          ReportSensorReadings(mySensorReadings);
        }
        lastReadingTime = newReadingTime; // Reset for the next time to read.
      }

    } // keep looping while connected
    Serial.println("\nDisconnected\n\n");
    // When we disconnect the handler is called to take care of the cleanup.
  }
  BLE.poll(); // Making sure we keep BLE going/listening.
    
}
