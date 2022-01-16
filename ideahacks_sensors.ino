// SENSOR RELATED GLOBAL VARIABLES
// Temp & humidity setup
#include "Adafruit_Si7021.h"
bool enableHeater = false;
uint8_t loopCnt = 0;
Adafruit_Si7021 sensor = Adafruit_Si7021();
// Soil setup
#include "Adafruit_seesaw.h"
Adafruit_seesaw ss;
// USound setup
#include <HCSR04.h>
HCSR04 hc(5, 6);
int uSoundDist;
// Sensor related global function declarations
void sensorSetup();
void sensorLoop(); // sensor funcs run temp/humid and soil sensor loops
void tempHumidSetup();
void tempHumidLoop();
void soilSetup();
void soilLoop();
void uSoundSetup();
void uSoundLoop();
// Fire risk variables & calculation
bool tempRisk = 0; // from tempHumid
bool humRisk = 0; // from tempHumid 
bool capRisk = 0; // from soil moisture sensor
bool brushRisk = 0; // reads the height, density of the brush in from the ultrasonic sensor
bool fireRisk();

//// WHAT AVA CAN REMOVE (below)
void setup() {
  sensorSetup();
}

void loop() {
  sensorLoop();
}
//// WHAT AVA CAN REMOVE (above)

// ALL SENSOR SPECIFIC FUNCTIONS DECLARED BELOW:
void sensorSetup() {
  Serial.begin(115200); // AVA: You can remove this if you set the baud rate elsewhere.
  tempHumidSetup();
  soilSetup();
  uSoundSetup();
}

void sensorLoop() {
  tempHumidLoop();
  soilLoop();
  uSoundLoop();
}

// ULTRASONIC SENSOR SETUP
void uSoundSetup() {
  // nothing..
}

void uSoundLoop() {
   uSoundDist = hc.dist();
   if (uSoundDist < 30) {
    brushRisk = 1;
   }
}


// TEMPERATURE & HUMIDITY SENSOR
void tempHumidSetup() {
  if (!sensor.begin()) {
    Serial.println("Did not find Si7021 sensor!");
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
    Serial.println("ERROR! seesaw not found");
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
  if (tempRisk == 1 && humRisk == 1 && capRisk == 1 && brushRisk) {
    return 1;
  } else {
    return 0;
  }
}
