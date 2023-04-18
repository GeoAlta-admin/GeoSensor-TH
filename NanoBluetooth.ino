/*
  Bluetooth 
  This device is considered a bulletin board or Peripheral device.
 */
#include <ArduinoBLE.h>

BLEService geoServiceSensorValues(serviceUuid);
BLEService geoServiceBattery("180F");
BLEService geoServiceButton("161DFFF1-0974-4F62-8DC3-E61C3DCAD1D5");
BLEService geoServiceDeviceDetails("161DFFF2-0974-4F62-8DC3-E61C3DCAD1D5");
BLEService geoServiceOperatingParams("161DFFF3-0974-4F62-8DC3-E61C3DCAD1D5");

// The complete UUID is provided by the Bluetooth spec.
// 00002A6E-0000-1000-8000-00805F9B34FB
BLECharacteristic myTempCChar("2A6E", BLERead | BLENotify, 8);
BLECharacteristic myHumidityChar("2A6F", BLERead | BLENotify, 8);
BLECharacteristic myHeatIndexCChar("2A7A", BLERead | BLENotify, 20);
BLEUnsignedCharCharacteristic myBatteryLevelChar("2A19", BLERead | BLENotify);
BLECharacteristic myModelNumChar("2A24", BLERead | BLENotify, GEOMODELNUMBER);
BLECharacteristic myFirmwareRevChar("2A26", BLERead | BLENotify, GEOFIRMWARENUMBER);

BLEIntCharacteristic myButtonChar("161DFF01-0974-4F62-8DC3-E61C3DCAD1D5", BLERead | BLEIndicate);

BLEIntCharacteristic mySetCycleTime("161Dff02-0974-4F62-8DC3-E61C3DCAD1D5", BLERead | BLEWrite | BLEIndicate);
BLEIntCharacteristic mySetTempDelta("161Dff03-0974-4F62-8DC3-E61C3DCAD1D5", BLERead | BLEWrite | BLEIndicate);
BLEIntCharacteristic mySetHumidityDelta("161Dff04-0974-4F62-8DC3-E61C3DCAD1D5", BLERead | BLEWrite | BLEIndicate);
BLEIntCharacteristic mySetHeatIndexDelta("161Dff05-0974-4F62-8DC3-E61C3DCAD1D5", BLERead | BLEWrite | BLEIndicate);
BLEIntCharacteristic mySetBatteryDelta("161Dff06-0974-4F62-8DC3-E61C3DCAD1D5", BLERead | BLEWrite | BLEIndicate);
BLEIntCharacteristic mySetKeepAliveTimer("161Dff07-0974-4F62-8DC3-E61C3DCAD1D5", BLERead | BLEWrite | BLEIndicate);


/*
const float TemperatureDELTA = 0.1;
const float HumitityDELTA = 0.1;
const float HeatIndexDELTA = 0.5;
const int BATTERYLEVELDELTA = 2;
 */

BLECharacteristic myDeviceNameChar("2A00", BLERead | BLENotify, GEODEVICENAME);
BLECharacteristic myManufacNameChar("2A29", BLERead | BLENotify, GEOCOMPANYNAME);

char cTemperature[10];
char cHumitity[10];
char cHeatIndex[10];

/*
    Initiailize the Bluetooth communicaitons.
 */
bool initBluetooth() {
  pinMode(LED_BUILTIN, OUTPUT); // initialize the built-in LED pin

  if (!BLE.begin()) {
    Serial.println("failed to initialize BLE!");
    while (1);
    return false;
  }
  Serial.println("BLE initialized");


  // Set the initial values.
  myTempCChar.descriptor("Temperature Value");
  setFloatCharacteristic(myTempCChar, lastSensorReading.TemperatureC, 100);
  setFloatCharacteristic(myHumidityChar, lastSensorReading.Humidity, 100);
  setFloatCharacteristic(myHeatIndexCChar, lastSensorReading.HeatIndexC, 100);
  //myModelNumChar.writeValue(GEOMODELNUMBER);
  //myFirmwareRevChar.writeValue(GEOFIRMWARENUMBER);

  // Add characteristics to service
  geoServiceSensorValues.addCharacteristic(myTempCChar);
  geoServiceSensorValues.addCharacteristic(myHumidityChar);
  geoServiceSensorValues.addCharacteristic(myHeatIndexCChar);

  geoServiceButton.addCharacteristic(myButtonChar);
  geoServiceBattery.addCharacteristic(myBatteryLevelChar);

  geoServiceDeviceDetails.addCharacteristic(myModelNumChar);
  geoServiceDeviceDetails.addCharacteristic(myFirmwareRevChar);
  geoServiceDeviceDetails.addCharacteristic(myDeviceNameChar);
  geoServiceDeviceDetails.addCharacteristic(myManufacNameChar);

  geoServiceOperatingParams.addCharacteristic(mySetTempDelta);
  geoServiceOperatingParams.addCharacteristic(mySetKeepAliveTimer);
  geoServiceOperatingParams.addCharacteristic(mySetBatteryDelta);
  geoServiceOperatingParams.addCharacteristic(mySetHeatIndexDelta);
  geoServiceOperatingParams.addCharacteristic(mySetHumidityDelta);
  geoServiceOperatingParams.addCharacteristic(mySetCycleTime);

  // Add the service
  BLE.addService(geoServiceSensorValues); 
  BLE.addService(geoServiceDeviceDetails);
  BLE.addService(geoServiceButton);
  BLE.addService(geoServiceBattery);
  BLE.addService(geoServiceOperatingParams);

  BLE.setEventHandler(BLEConnected, connectHandler);
  BLE.setEventHandler(BLEDisconnected, disconnectHandler);
  //BLE.setConnectionInterval(5, 11);
  BLE.setConnectionInterval(0x0006, 0x0c80); // 7.5 ms minimum, 4 s maximum
  //BLE.setSuperivisionTimeout(3200); //This does not exist.

  /*
    NOTE that if you set the device name and then the Local name, the Local Name is never
         actually reported.

      BLE.setDeviceName(GEODEVICENAME); //GEODEVICENAME
   */


  // Assign the handlers for the return values
  mySetCycleTime.setEventHandler(BLEUpdated, onSetCycleTimeWritten);
  mySetTempDelta.setEventHandler(BLEUpdated, onSetTempDeltaWritten);
  mySetHumidityDelta.setEventHandler(BLEUpdated, onSetHumidityDeltaWritten);
  mySetHeatIndexDelta.setEventHandler(BLEUpdated, onSetHeatIndexDeltaWritten);
  mySetBatteryDelta.setEventHandler(BLEUpdated, onSetBatteryDeltaWritten);
  mySetKeepAliveTimer.setEventHandler(BLEUpdated, onSetKeepAliveTimerWritten);

  myButtonChar.setEventHandler(BLEIndicate, onButtonPressConfirmed);

  // Time to get noticed.  
  char devicename[sizeof(GEODEVICENAME)+22];
  String address = BLE.address().substring(12);
  address.replace(":","");
  address.toUpperCase();
  char address_char[5];
  address.toCharArray(address_char, 5);

  strcpy(devicename,GEODEVICENAME);
  strcat(devicename,"-");
  strcat(devicename,address_char);

  Serial.print("Device Name: ");
  Serial.println(devicename);

  BLE.setDeviceName(devicename);
  BLE.setLocalName(devicename);  // Set name for connection
  BLE.setAdvertisedService(geoServiceSensorValues); // Advertise service
  BLE.setAdvertisedServiceUuid(serviceUuid);

  BLE.advertise(); 

  Serial.print("Peripheral device MAC: "); Serial.println(BLE.address());
  Serial.println("Waiting for connections...");

  return true;

}


int BatteryLevel(){
  int battery = analogRead(A0);
  int batteryLevel = map(battery, 0, 1023, 0, 100);
  return batteryLevel;
}

/*
  Function to report on the battery level if it has changed enough.
 */
void ReportBatteryLevel(){
  int battery = analogRead(A0);
  //float voltage = battery * (5.00 / 1023.00) * 2; //convert the value to a true voltage.
  int batterylevel = map(battery, 0, 1023, 0, 100);
  //Serial.print("Battery Level is now: ");  Serial.println(batterylevel);
  if (batterylevel != lastBatteryLevelReported){
    // Is the difference in battery level enough to report?
    if (batterylevel < 3.0){
      setFloatCharacteristic(myBatteryLevelChar,batterylevel,100);
      lastBatteryLevelReported = batterylevel;
    } else if ((lastBatteryLevelReported - batterylevel) > BATTERYLEVELDELTA){
      setFloatCharacteristic(myBatteryLevelChar,batterylevel,100);
      lastBatteryLevelReported = batterylevel;
    }
  }
}
void checkButtonState(){
  char buttonValue = digitalRead(PINBUTTON);
  
  bool buttonChanged = (myButtonChar.value() != buttonValue);
  if (buttonChanged) {
    // button state changed, update characteristics
    myButtonChar.writeValue(buttonValue);
  }
  
}

void checkClientValues(){
  if (mySetTempDelta.value() != TemperatureDELTA){
    Serial.println("Temp Delta changed");
  }
}

// Write the Float value to the characteristic asa byte array.
void setFloatCharacteristic (BLECharacteristic bleChar, float val, int dec){
  //int intFloat = int(val * dec) %100;
  char buffer[10];
  sprintf(buffer, "%d.%02d", int(val), int(val * dec) %100);
  bleChar.writeValue(buffer);
}

// listen for BLE connect events:
void connectHandler(BLEDevice central) {
  // central connected event handler
  Serial.print("Connected event, central: "); Serial.println(central.address());
  digitalWrite(LED_BUILTIN, HIGH);
}

// listen for BLE disconnect events:
void disconnectHandler(BLEDevice central) {
  // central disconnected event handler
  Serial.print("Disconnected event, central: "); Serial.println(central.address());
  digitalWrite(LED_BUILTIN, LOW);
}

/*
    The handlers for when a characterisitc receives a value
 */
void onSetCycleTimeWritten(BLEDevice central, BLECharacteristic characteristic) {
  Serial.print("Received Cycle Time Seconds value: "); Serial.println(mySetCycleTime.value());
  // We have the new cycle time, now we need to convert it to the right data type
  // What we get is going to be in seconds, so we will need to multiply it by 1000
  // cycleTimeSeconds = mySetCycleTime.value() * 1000;      // unsigned int
}
void onSetTempDeltaWritten(BLEDevice central, BLECharacteristic characteristic) {
  Serial.print("Received Temperature DELTA value: "); Serial.println(mySetTempDelta.value());
  // TemperatureDELTA = mySetTempDelta.value();             // float
}
void onSetHumidityDeltaWritten(BLEDevice central, BLECharacteristic characteristic) {
  Serial.print("Received Humitity DELTA value: "); Serial.println(mySetHumidityDelta.value());
  // HumitityDELTA = mySetHumidityDelta.value();            // float
}
void onSetHeatIndexDeltaWritten(BLEDevice central, BLECharacteristic characteristic) {
  Serial.print("Received Heat Index DELTA value: "); Serial.println(mySetHeatIndexDelta.value());
  // HeatIndexDELTA = mySetHeatIndexDelta.value();          // float
}
void onSetBatteryDeltaWritten(BLEDevice central, BLECharacteristic characteristic) {
  Serial.print("Received Battery Level DELTA value: "); Serial.println(mySetBatteryDelta.value());
  // BATTERYLEVELDELTA = mySetBatteryDelta.value();         // int
}
void onSetKeepAliveTimerWritten(BLEDevice central, BLECharacteristic characteristic) {
  Serial.print("Received keep alive cycle value: "); Serial.println(mySetKeepAliveTimer.value());
  // keepalivecycle = mySetKeepAliveTimer.value() * 1000;   // unsigned long
}

void onButtonPressConfirmed(BLEDevice central, BLECharacteristic characteristic) {
  Serial.println("The button press confirmed");
}
