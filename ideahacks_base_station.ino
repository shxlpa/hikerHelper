// SSD1306 display connected to I2C
// Data => I2C SDA (A4)
// Clk => I2C SCL (A5)
// Rst => (D4)
#define DEBUG 128 

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// SENSOR RELATED GLOBAL VARIABLES
// Temp & humidity setup
#include "Adafruit_Si7021.h"
bool enableHeater = false;
uint8_t loopCnt = 0;
Adafruit_Si7021 sensor = Adafruit_Si7021();
// Soil setup
#include "Adafruit_seesaw.h"
Adafruit_seesaw ss;
// Sensor related global function declarations
void sensorSetup();
void sensorLoop(); // sensor funcs run temp/humid and soil sensor loops
void tempHumidSetup();
void tempHumidLoop();
void soilSetup();
void soilLoop();
// Fire risk variables & calculation
bool tempRisk = 0; // from tempHumid
bool humRisk = 0; // from tempHumid 
bool capRisk = 0; // from soil moisture sensor
bool fireRisk();

char message[20] = "HELLO";
char received_message[20] = "GOODBYE";
bool inbox = false;

void setup() {
  sensorSetup();
}

void loop() {
  sensorLoop();
}

// ••••••••••••••••
// SENSORS!
// ••••••••••••••••
void sensorSetup() {
  Serial.begin(115200); // AVA: You can remove this if you set the baud rate elsewhere.
  tempHumidSetup();
  soilSetup();
}

void sensorLoop() {
  tempHumidLoop();
  soilLoop();
}

void tempHumidSetup() {
  if (!sensor.begin()) {
    if (DEBUG) {
      Serial.println("Did not find Si7021 sensor!");
    }
    while (true)
      ;
  }
}

void tempHumidLoop() {
  double humidity = sensor.readHumidity();
  double temperature = sensor.readTemperature();

  // If the humidity is at a risky level, set humRisk to true.
  if (humidity <= 30) {
    humRisk = 1;
  }

  if (temperature >= 27) {
    tempRisk = 1;
  }
  
 // NOTE FROM SHILPA: from sample sketch; might not be necessary:
  // Toggle heater enabled state every 30 seconds
  // An ~1.8 degC temperature increase can be noted when heater is enabled
  if (++loopCnt == 30) {
    enableHeater = !enableHeater;
    sensor.heater(enableHeater);
    loopCnt = 0;
  }
}

// SOIL SENSOR
void soilSetup() {
  if (!ss.begin(0x36)) {
    if (DEBUG) {
      Serial.println("ERROR! seesaw not found");
    }
    while(1) delay(1);
  } 
}

void soilLoop() {
  float tempC = ss.getTemp();
  uint16_t capread = ss.touchRead(0);
  
  // if the moisture is at a risky, fire danger level
  if (capread <= 700) {
    capRisk = 1;
  }
}

bool fireRisk(bool tempRisk, bool humRisk, bool capRisk) {
  if (tempRisk == 1 && humRisk == 1 && capRisk == 1) {
    return 1;
  } else {
    return 0;
  }
}
