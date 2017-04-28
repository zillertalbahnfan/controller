#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ArduinoJson.h>
#include <Adafruit_MotorShield.h>
 
 
#define SELECT_LOCO 0
#define AUTO_MAN 16
#define KILLSWITCH 2

#define START 15
#define COAST 12
#define STOP 13
#define DIRECTION 14



Adafruit_SSD1306 display = Adafruit_SSD1306();
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
Adafruit_DCMotor *myMotor = AFMS.getMotor(1);




String railway_name = "ZILLERTALBAHN";

// MAN is pure manual, buttons don't light, only the potentiometer and the reverse/forward work, is still governed by property of the loco selected
// AUTO is computer controlled, buttons to start, coast and brake, potentiometer does nothing

String current_mode = "AUTO";
int potentiometer_value = 0;
int last_potentiometer_value = 1;

float current_speed = 0.0;
float stall_speed = 10.0;
float max_speed = 50.0;



void displayController(bool clear_display) {
    if (clear_display) {
      display.clearDisplay();
    }
    display.setCursor(0,0);
    display.println(railway_name + " | " + current_mode);
    display.display();    
}

void displayLocomotive(bool clear_display) {
    if (clear_display) {
      display.clearDisplay();
    }
    displayController(false);
    display.setCursor(0,8);
    display.println("");
    display.display();    
}

void displayMessage(String text, bool clear_display) {
    if (clear_display) {
      display.clearDisplay();
    }
    displayLocomotive(false);
    display.setCursor(0,16);
    display.println(text);
    display.display();      
}

  
void setup() {  
  Serial.begin(9600);
  AFMS.begin(100000);
  
  // initialize the OLED display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  displayMessage("LOADING...", true); 
  display.display(); 
  delay(1000);
  displayController(true);
  
  // set the modes for the pins
  pinMode(2, INPUT_PULLUP);
  pinMode(0, INPUT_PULLUP);
  pinMode(15, INPUT_PULLUP);
  pinMode(13, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
}


void select_loco() {
    displayMessage("SELECTING LOCO", true);
    delay(1000);  
}



void read_onboard_switches() {
  if (! digitalRead(SELECT_LOCO) ) {
    select_loco();
  }
  
  if (! digitalRead(AUTO_MAN) ) {
    displayMessage("AUTO_MAN", true);
    delay(100);
  }

  if (! digitalRead(KILLSWITCH) ) {
    displayMessage("KILLSWITCH", true);
    delay(100);
  }
}

void readInput(String input) {
  
}

void loop() {

  //read_onboard_switches();

  // check to see what mode the controller is in. 
  if (current_mode == "MAN") {
    //potentiometer_value = analogRead(SPEED_CONTROLLER);
    //if (potentiometer_value != last_potentiometer_value) {
    //  current_speed = ((potentiometer_value)/1023)*max_speed;
    //  display_action = displayMessage("Speed : " + String(potentiometer_value) + "km/h", true);
    //  last_potentiometer_value = potentiometer_value;
    // }
  }
  else
  { 
    bool two = digitalRead(2);
    bool zero = digitalRead(0);
    bool fifteen = digitalRead(15);
    bool thirteen = digitalRead(13);
    bool twelve = digitalRead(12);
    bool fourteen = digitalRead(14);
    displayMessage("2:" + String(two) + "/0:" + String(zero) + "/15:" + String(fifteen) + "/13:" + String(thirteen) + "/12:" + String(twelve) + "/14:" + String(fourteen), true);       
    delay(1000);
  }
}
