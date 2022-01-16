// SSD1306 display connected to I2C
// Data => I2C SDA (A4)
// Clk => I2C SCL (A5)
// Rst => (D4)
#define DEBUG 128 

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#define OLED_RESET     4 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3D ///< See datasheet for Address; 0x3D for 128x64, 0x3C for 128x32
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

#include <string.h>;

// joystick
#define JOY_CENTER 511
#define JOY_SCALING 50
int joyx = 0;
int joyy = 0;
int cursorX = 64; // cursor x and y
int cursorY = 32;
int deltax = 0;
int deltay = 0;
bool isClicked = false;
#define STICK_X A1
#define STICK_Y A2
#define SW_PIN 8

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
// Fire risk variables & calculation
bool tempRisk = 0; // from tempHumid
bool humRisk = 0; // from tempHumid 
bool capRisk = 0; // from soil moisture sensor
bool fireRisk();

char message[20] = "HELLO";
char received_message[20] = "GOODBYE";
bool inbox = false;

void setup() {
  SSD1306_setup();
  sensorSetup();
}

void loop() {
  getJoystickStatus();
  SSD1306_writing_msg();
  sensorLoop();
}

// ••••••••••••••••
// OLED & JOYSTICK!
// ••••••••••••••••

void getJoystickStatus() {
  joyx = analogRead(STICK_X);
  joyy = analogRead(STICK_Y);
  isClicked = !digitalRead(SW_PIN);
  // joyx and y range from 0 to 1022. 511 is the center, dead zone.

  // 1 pixel per tick
  deltay = (joyx > 0) ? 1 : -1;
  deltax = (joyy > 0) ? 1 : -1;

  // in testing: multi pixels per tick, scaled
//  deltax = (joyy - JOY_CENTER) / JOY_SCALING;
//  deltay = (joyx - JOY_CENTER) / JOY_SCALING;
  
  if (joyx < JOY_CENTER * 1.1 && joyx > JOY_CENTER * 0.9) {
    deltax = 0;
  }
  if (joyy < JOY_CENTER * 1.1 && joyy > JOY_CENTER * 0.9) {
    deltay = 0;
  }

  cursorX += deltax;
  cursorY += deltay;

  if (cursorX >= 128) {
    cursorX = 128;
  }
  if (cursorX <= 0) {
    cursorX = 0;
  }
  if (cursorY >= 128) {
    cursorY = 128;
  }
  if (cursorY <= 0) {
    cursorY = 0;
  }
}

void SSD1306_setup() {
  Serial.begin(115200);

  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if(!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    if (DEBUG) {
      Serial.println(F("SSD1306 allocation failed"));
    }
    for(;;); // Don't proceed, loop forever
  }

  // Clear the buffer
  display.clearDisplay();
  display.display();
}

void SSD1306_writing_msg() {
  bool sent = false;

  while (!sent) {
    // Check if we've received a message
    if (inbox) {
      SSD1306_receiving_msg();
    }

    // joystick data is already in
    int x = cursorX;
    int y = cursorY;

    if (isClicked == true) {
      // Figure out where the click occurred
      if ((x > 5) && (x < 17) && (y > 20) && (y < 33)) {
        strncat(message, 'Q', 1);
      }
      else if ((x > 17) && (x < 29) && (y > 20) && (y < 33)) {
        strncat(message, 'W', 1);
      }
      else if ((x > 29) && (x < 41) && (y > 20) && (y < 33)) {
        strncat(message, 'E', 1);
      }
      else if ((x > 41) && (x < 53) && (y > 20) && (y < 33)) {
        strncat(message, 'R', 1);
      }
      else if ((x > 53) && (x < 65) && (y > 20) && (y < 33)) {
        strncat(message, 'T', 1);
      }
      else if ((x > 65) && (x < 77) && (y > 20) && (y < 33)) {
        strncat(message, 'Y', 1);
      }
      else if ((x > 77) && (x < 89) && (y > 20) && (y < 33)) {
        strncat(message, 'U', 1);
      }
      else if ((x > 89) && (x < 101) && (y > 20) && (y < 33)) {
        strncat(message, 'I', 1);
      }
      else if ((x > 101) && (x < 113) && (y > 20) && (y < 33)) {
        strncat(message, 'O', 1);
      }
      else if ((x > 113) && (x < 125) && (y > 20) && (y < 33)) {
        strncat(message, 'P', 1);
      }

      else if ((x > 11) && (x < 23) && (y > 33) && (y < 46)) {
        strncat(message, 'A', 1);
      }
      else if ((x > 23) && (x < 35) && (y > 33) && (y < 46)) {
        strncat(message, 'S', 1);
      }
      else if ((x > 35) && (x < 47) && (y > 33) && (y < 46)) {
        strncat(message, 'D', 1);
      }
      else if ((x > 47) && (x < 59) && (y > 33) && (y < 46)) {
        strncat(message, 'F', 1);
      }
      else if ((x > 59) && (x < 71) && (y > 33) && (y < 46)) {
        strncat(message, 'G', 1);
      }
      else if ((x > 71) && (x < 83) && (y > 33) && (y < 46)) {
        strncat(message, 'H', 1);
      }
      else if ((x > 83) && (x < 95) && (y > 33) && (y < 46)) {
        strncat(message, 'J', 1);
      }
      else if ((x > 95) && (x < 107) && (y > 33) && (y < 46)) {
        strncat(message, 'K', 1);
      }
      else if ((x > 107) && (x < 119) && (y > 33) && (y < 46)) {
        strncat(message, 'L', 1);
      }

      else if ((x > 17) && (x < 29) && (y > 46) && (y < 59)) {
        strncat(message, 'Z', 1);
      }
      else if ((x > 29) && (x < 41) && (y > 46) && (y < 59)) {
        strncat(message, 'X', 1);
      }
      else if ((x > 41) && (x < 53) && (y > 46) && (y < 59)) {
        strncat(message, 'C', 1);
      }
      else if ((x > 53) && (x < 65) && (y > 46) && (y < 59)) {
        strncat(message, 'V', 1);
      }
      else if ((x > 65) && (x < 77) && (y > 46) && (y < 59)) {
        strncat(message, 'B', 1);
      }
      else if ((x > 77) && (x < 89) && (y > 46) && (y < 59)) {
        strncat(message, 'N', 1);
      }
      else if ((x > 89) && (x < 101) && (y > 46) && (y < 59)) {
        strncat(message, 'M', 1);
      }
      else if ((x > 101) && (x < 113) && (y > 46) && (y < 59)) {
        // Backspace
        char* pos = message;
        int current_length = 0;
        while (*pos != '\0') {
          current_length++;
          pos++;
        }
        if (pos > 0) {
          message[current_length] = '\0';
        }
      }

      else if ((x > 113) && (x < 125) && (y > 0) && (y < 13)) {
        // Send
        sent = true;
      }
      
      clear_screen();
    }
    
    keyboard();
    joystick(x, y);
    display.display();
    display.clearDisplay();
  }

  // Send the message once we've pressed send
  SSD1306_sending_msg();
}

void SSD1306_receiving_msg() {
  char* msg_ptr = received_message;
  char receiving_message[20] = "";
  
  while (*msg_ptr != '\0') {
    // Written message
    strncat(receiving_message, msg_ptr, 1);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(4,4);
    display.clearDisplay();
    char temp_receiving_message[23] = "";
    strcpy(temp_receiving_message, receiving_message);
    strcat(temp_receiving_message, "...");
    display.println(temp_receiving_message);
    
    // Line below message
    display.drawLine(0, 13, 127, 13, WHITE);
  
    // Receiving message
    display.setTextSize(2);
    display.setCursor(10,25);
    display.println("receiving");
    display.setCursor(4,40);
    display.println("message...");
  
    // Display screen
    display.display();

    delay(1000); //temporary
    msg_ptr++;
    display.clearDisplay();
  }
  // Display final message on screen
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(4,4);
  display.println(receiving_message);
    
  // Line below message
  display.drawLine(0, 13, 127, 13, WHITE);
  
  // Message received
  display.setTextSize(2);
  display.setCursor(15,25);
  display.println("message");
  display.setCursor(10,40);
  display.println("received");
  
  // Display screen
  display.display();
  delay(10000);

  // Clear received message global variable once the message is fully received
  received_message[0] = '\0';
  clear_screen();
}

void SSD1306_sending_msg() {
  double total_length = strlen(message);
  char sending_message[20] = "";
  char* msg_ptr = message;
  
  while (*msg_ptr != '\0') {
    // Written message
    strncat(sending_message, msg_ptr, 1);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(4,4);
    display.clearDisplay();
    char temp_sending_message[23] = "";
    strcpy(temp_sending_message, sending_message);
    strcat(temp_sending_message, "...");
    display.println(temp_sending_message);
    
    // Line below message
    display.drawLine(0, 13, 127, 13, WHITE);
  
    // Sending message
    display.setTextSize(2);
    display.setCursor(20,18);
    display.println("sending");
    display.setCursor(4,33);
    display.println("message...");
  
    // Status bar
    display.drawLine(0, 55, 127, 55, WHITE);
    display.drawLine(0, 63, 127, 63, WHITE);
    display.drawLine(0, 55, 0, 63, WHITE);
    display.drawLine(127, 55, 127, 63, WHITE);

    char* pos = sending_message;
    double current_length = 0;
    while (*pos != '\0') {
      current_length++;
      pos++;
    }
    double percentage = current_length / total_length;
    display.drawLine(1, 56, percentage*127, 56, WHITE);
    display.drawLine(1, 57, percentage*127, 57, WHITE);
    display.drawLine(1, 58, percentage*127, 58, WHITE);
    display.drawLine(1, 59, percentage*127, 59, WHITE);
    display.drawLine(1, 60, percentage*127, 60, WHITE);
    display.drawLine(1, 61, percentage*127, 61, WHITE);
    display.drawLine(1, 62, percentage*127, 62, WHITE);
  
    // Display screen
    display.display();

    // if ... continue;
    delay(1000); //temporary
    msg_ptr++;
    display.clearDisplay();
  }

  // Clear message global variable once the message is fully sent
  message[0] = '\0';
  clear_screen();
}

void SSD1306_compass_msg() {
  //
}

void keyboard() {
  // Written message
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(4,4);
  display.clearDisplay();
  display.println(message);

  // Line below message
  display.drawLine(0, 13, 127, 13, WHITE);

  // Keyboard horizontal lines
  display.drawLine(5, 20, 124, 20, WHITE);
  display.drawLine(5, 33, 124, 33, WHITE);
  display.drawLine(11, 46, 118, 46, WHITE);
  display.drawLine(17, 59, 112, 59, WHITE);

  // Keyboard vertical lines
  display.drawLine(5, 20, 5, 33, WHITE);
  display.drawLine(17, 20, 17, 33, WHITE);
  display.drawLine(29, 20, 29, 33, WHITE);
  display.drawLine(41, 20, 41, 33, WHITE);
  display.drawLine(53, 20, 53, 33, WHITE);
  display.drawLine(65, 20, 65, 33, WHITE);
  display.drawLine(77, 20, 77, 33, WHITE);
  display.drawLine(89, 20, 89, 33, WHITE);
  display.drawLine(101, 20, 101, 33, WHITE);
  display.drawLine(113, 20, 113, 33, WHITE);
  display.drawLine(125, 20, 125, 33, WHITE);

  display.drawLine(11, 33, 11, 46, WHITE);
  display.drawLine(23, 33, 23, 46, WHITE);
  display.drawLine(35, 33, 35, 46, WHITE);
  display.drawLine(47, 33, 47, 46, WHITE);
  display.drawLine(59, 33, 59, 46, WHITE);
  display.drawLine(71, 33, 71, 46, WHITE);
  display.drawLine(83, 33, 83, 46, WHITE);
  display.drawLine(95, 33, 95, 46, WHITE);
  display.drawLine(107, 33, 107, 46, WHITE);
  display.drawLine(119, 33, 119, 46, WHITE);

  display.drawLine(17, 46, 17, 59, WHITE);
  display.drawLine(29, 46, 29, 59, WHITE);
  display.drawLine(41, 46, 41, 59, WHITE);
  display.drawLine(53, 46, 53, 59, WHITE);
  display.drawLine(65, 46, 65, 59, WHITE);
  display.drawLine(77, 46, 77, 59, WHITE);
  display.drawLine(89, 46, 89, 59, WHITE);
  display.drawLine(101, 46, 101, 59, WHITE);
  display.drawLine(113, 46, 113, 59, WHITE);

  // Keyboard letters
  display.setCursor(9,23);
  display.println("Q");
  display.setCursor(21,23);
  display.println("W");
  display.setCursor(33,23);
  display.println("E");
  display.setCursor(45,23);
  display.println("R");
  display.setCursor(57,23);
  display.println("T");
  display.setCursor(69,23);
  display.println("Y");
  display.setCursor(81,23);
  display.println("U");
  display.setCursor(93,23);
  display.println("I");
  display.setCursor(105,23);
  display.println("O");
  display.setCursor(117,23);
  display.println("P");

  display.setCursor(15,36);
  display.println("A");
  display.setCursor(27,36);
  display.println("S");
  display.setCursor(39,36);
  display.println("D");
  display.setCursor(51,36);
  display.println("F");
  display.setCursor(63,36);
  display.println("G");
  display.setCursor(75,36);
  display.println("H");
  display.setCursor(87,36);
  display.println("J");
  display.setCursor(99,36);
  display.println("K");
  display.setCursor(111,36);
  display.println("L");
  
  display.setCursor(21,49);
  display.println("Z");
  display.setCursor(33,49);
  display.println("X");
  display.setCursor(45,49);
  display.println("C");
  display.setCursor(57,49);
  display.println("V");
  display.setCursor(69,49);
  display.println("B");
  display.setCursor(81,49);
  display.println("N");
  display.setCursor(93,49);
  display.println("M");
  display.setCursor(105,49);
  display.println("<");

  // Send button
  display.drawLine(113, 0, 125, 0, WHITE);
  display.drawLine(113, 13, 125, 13, WHITE);
  display.drawLine(113, 0, 113, 13, WHITE);
  display.drawLine(125, 0, 125, 13, WHITE);
  display.setCursor(117,3);
  display.println(">");

  // Compass button
//  display.drawLine(0, 50, 12, 50, WHITE);
//  display.drawLine(0, 63, 12, 63, WHITE);
//  display.drawLine(0, 50, 0, 63, WHITE);
//  display.drawLine(12, 50, 12, 63, WHITE);
//  display.setCursor(4,53);
//  display.println("O");
  
  // Display screen
  //display.display();
}

void joystick(int x, int y) {
  display.setCursor(x,y);
  display.println("o");
  //display.display();
}

void clear_screen() {
  // Clear buffer and display blank screen
  display.clearDisplay();
  display.display();
}

// ••••••••••••••••
// SENSORS!
// ••••••••••••••••
void sensorSetup() {
  Serial.begin(115200); // AVA: You can remove this if you set the baud rate elsewhere.
  tempHumidSetup();
}

void sensorLoop() {
  tempHumidLoop();
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
