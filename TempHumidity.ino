#include "DHT.h"

// Feather HUZZAH ESP8266 note: use pins 3, 4, 5, 12, 13 or 14 --
// Pin 15 can work but DHT must be disconnected during program upload.

// Uncomment whatever type of sensor you are using!
#define DHTTYPE DHT11   // DHT 11
//#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

// Connect pin 1 (on the left) of the sensor to +5V
// NOTE: If using a board with 3.3V logic like an Arduino Due connect pin 1
// to 3.3V instead of 5V!
// Connect pin 2 of the sensor to whatever your PINSENSORDATA is
// Connect pin 3 (on the right) of the sensor to GROUND (if your sensor has 3 pins)
// Connect pin 4 (on the right) of the sensor to GROUND and leave the pin 3 EMPTY (if your sensor has 4 pins)
// Connect a 10K resistor from pin 2 (data) to pin 1 (power) of the sensor

// Initialize DHT sensor.
// Note that older versions of this library took an optional third parameter to
// tweak the timings for faster processors.  This parameter is no longer needed
// as the current DHT reading algorithm adjusts itself to work on faster procs.
DHT dht(PINSENSORDATA, DHTTYPE);


void initTempHumitity() {
  //Serial.println("Initializing DHT...");
  SensorReadings mySensorReadings;
  dht.begin();
  delay(5000);
  
  Serial.println("Taking initial sensor readings.");
  takeReading(mySensorReadings);
  lastSensorReading.HeatIndexC = mySensorReadings.HeatIndexC;
  lastSensorReading.TemperatureC = mySensorReadings.TemperatureC;
  lastSensorReading.Humidity = mySensorReadings.Humidity;

  Serial.print("Initial Reading Temperature (C): "); Serial.print(lastSensorReading.TemperatureC);
  Serial.print("\tHumidity: "); Serial.print(lastSensorReading.Humidity);
  Serial.print("\tHeatIndexC: "); Serial.println(lastSensorReading.HeatIndexC);
}

bool takeReading(SensorReadings &myReadings){
  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  //float f = dht.readTemperature(true);

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t)) {
    Serial.println(F("Failed to read from DHT sensor!"));
    if (isnan(h)){
      Serial.println("No Humidity reading.");
    }
    if (isnan(t)){
      Serial.println("No Temperature reading.");
    }
    return false;
  }

  // Compute heat index in Fahrenheit (the default)
  //float hif = dht.computeHeatIndex(f, h);
  // Compute heat index in Celsius (isFahreheit = false)
  float hic = dht.computeHeatIndex(t, h, false);

  myReadings.TemperatureC = t;
  myReadings.Humidity = h;
  myReadings.HeatIndexC = hic;
  //myReadings.TemperatureF = f;
  //myReadings.HeatIndexF = hif;

  //Serial.print(millis()); Serial.print("\t"); Serial.print("Temp C: "); Serial.print(t); Serial.print("\t\tHumidity: "); Serial.print(h); Serial.print("\t\tIndex: "); Serial.println(hic);

  return true;
}

void ReportSensorReadings(SensorReadings &myReadings){
  // Check the readings to see if they are large enough of a difference since last reporting.
  bool isChange = false;
  if (abs(lastSensorReading.TemperatureC - myReadings.TemperatureC) > TemperatureDELTA){
      Serial.print("Temperature Change - Old Value: "); Serial.print(lastSensorReading.TemperatureC);
      Serial.print("\tNew Value: "); Serial.print(myReadings.TemperatureC);
      Serial.print("\t");
    setFloatCharacteristic (myTempCChar, myReadings.TemperatureC, 100);
    lastSensorReading.TemperatureC = myReadings.TemperatureC;
    isChange = true;
  } 
  if (abs(lastSensorReading.HeatIndexC - myReadings.HeatIndexC) > HeatIndexDELTA){
      Serial.print("Heat Index Change - Old Value: "); Serial.print(lastSensorReading.HeatIndexC);
      Serial.print("\tNew Value: "); Serial.print(myReadings.HeatIndexC);
      Serial.print("\t");
    setFloatCharacteristic (myHeatIndexCChar, myReadings.HeatIndexC, 100);
    lastSensorReading.HeatIndexC = myReadings.HeatIndexC;
    isChange = true;
  }
  if (abs(lastSensorReading.Humidity - myReadings.Humidity) > HumitityDELTA){
      Serial.print("Humidity Change - Old Value: "); Serial.print(lastSensorReading.Humidity);
      Serial.print("\tNew Value: "); Serial.print(myReadings.Humidity);
      Serial.print("\t");
    setFloatCharacteristic (myHumidityChar, myReadings.Humidity, 100);
    lastSensorReading.Humidity = myReadings.Humidity;
    isChange = true;
  }

  if (isChange){
    keepalivetimer = newReadingTime;
    Serial.println(); // Just starting a new line.
  } else if (newReadingTime - keepalivetimer >= keepalivecycle) {
    // This is the keep alive blip so that the sensor does not loose connection.
    Serial.println("Keep Alive Triggered");
    setFloatCharacteristic (myTempCChar, myReadings.TemperatureC, 100);
    lastSensorReading.TemperatureC = myReadings.TemperatureC;
    keepalivetimer = newReadingTime;
  }
}
