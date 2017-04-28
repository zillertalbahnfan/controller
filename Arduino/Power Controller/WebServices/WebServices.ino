#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <Adafruit_MotorShield.h>


const char* ssid = "ZILLERTALBAHN";
const char* password = "ABCDEFGH";
IPAddress ip(192, 168, 0, 150);  //Node static IP
IPAddress gateway(192, 168, 0, 1);
IPAddress subnet(255,255,255,0);


ESP8266WebServer server(80);
Adafruit_SSD1306 display = Adafruit_SSD1306();
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
Adafruit_DCMotor *myMotor = AFMS.getMotor(1);


String methods[8] = {"/status", "/loco/start", "/loco/coast", "/loco/brake", "/loco/list", "/loco/select", "/loco/jenbach", "/loco/mayrhofen"};


String locomotive_names[11] = {"Nr 1. Raiumnd", "Nr 2. Zillertal", "Nr 3. Tirol","Nr 6. Hobbylok ", "D11", "D12", "D13", "D14", "D15", "D16", "VT1"};
int locomotive_max_speeds[11] = {};
int locomotive_stall_speeds[11] = {};
int locomotive_accelerations[11] = {};


bool loco_selected = false;
String current_loco = "No loco selected";
String current_action = "stopped";
String current_direction = "Mayrhofen";
String current_mode = "AUTO";
String last_request = "";
String last_message = "";


int current_speed = 0;
int max_speed = 50;
int stall_speed = 5;
int acceleration = 50000;

int starting = 4;
int coasting = 1;
int braking = 7;

int selected_loco;

int i = 0;
int max_i = 100000;


void startLoco() {
  if (current_speed == 0) {
    current_speed = stall_speed;
  }
  if (current_speed >= max_speed) {
    current_action = "moving";
    current_speed = max_speed;
  }
  else
  {
    current_speed += starting;
  }
  runMotor();
}

void coastLoco() {
  if (current_speed <= stall_speed) {
    current_action = "stopped";
    current_speed = 0;
  }
  else
  {
    current_speed -= coasting;
  }
  runMotor();
}


void brakeLoco() {
  if (current_speed <= stall_speed) {
    current_action = "stopped";
    current_speed = 0;
  }
  else
  {
    current_speed -= braking;
  }
  runMotor();
}

void runMotor() {
  myMotor->setSpeed(current_speed * 3);
  myMotor->run(FORWARD);
}


void doCycle(int this_i) {
  if (this_i == max_i) {
    i = 0;
  }
  if (i%acceleration == 0) {
    if (current_action == "starting") {
     startLoco();
    }
    else
    {
      if (current_action == "coasting") {
        coastLoco();
      }
      else
      {
        if (current_action == "braking") {
          brakeLoco();
        }
      }
    }
  }
}

bool isMoving() {
  bool is_moving = true;
  if (!loco_selected) {
    is_moving = false;
  }
  else
  {
    if (current_action == "stopped") {
      is_moving = false;
    }
  }
  return is_moving;
}


bool canStop() {
  bool can_stop = false;
  if (isMoving()) {
    can_stop = true;
  }
  return can_stop;
}


bool canStart() {
  bool can_start = true;
  if (!loco_selected) {
    can_start = false;
  }
  return can_start;
}




void setup(void){
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  WiFi.config(ip, gateway, subnet);
  
  // initialize the OLED display
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  displayStatus("LOADING...", true);
  delay(500); 
  displayStatus("IP: " + String(ip), true);
  delay(1000); 

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
  }
  AFMS.begin(100000);
  startServer();
}



void loop(void){
  server.handleClient();
  i++;
  doCycle(i);
}


// Display functions

void displayInfo(bool clear_display) {
  if (clear_display) {
      display.clearDisplay();
    }
    display.setCursor(0,0);
    display.println("ZB | " + current_mode + " | " + current_action);
    display.display();          
}

void displayLoco(bool clear_display) {
  if (clear_display) {
      display.clearDisplay();
    }
    displayInfo(false);
    display.setCursor(0,8);
    display.println(current_loco);
    display.display();          
}

void displayAction(bool clear_display) {
    String speed_string = String(current_speed) + "km/h -> " + current_direction ;
    if (clear_display) {
      display.clearDisplay();
    }
    displayLoco(false);
    display.setCursor(0,16);
    display.println(speed_string);
    display.display();      
  }

void displayStatus(String text, bool clear_display) {
    if (clear_display) {
      display.clearDisplay();
    }
    displayAction(false);
    display.setCursor(0,24);
    display.println(text);
    display.display();      
}


// HTTP Code

void startServer() {
  server.on("/", handleRoot);
  server.on("/status", handleStatus);
  server.on("/loco/start", handleStart);
  server.on("/loco/coast", handleCoast);
  server.on("/loco/brake", handleBrake);
  server.on("/loco/list", handleList);
  server.on("/loco/select", handleSelect);
  server.on("/loco/jenbach", handleDirectionJenbach);
  server.on("/loco/mayrhofen", handleDirectionMayrhofen);
  server.onNotFound(handleNotFound);
  server.begin();
  displayStatus("HTTP:server started", true);
  delay(1000);
  displayStatus("HTTP: ready", true); 
}


void handleURL(String response, bool success) {
  int status = 200;
  if (!success)
  {
    status = 423;
  }
  server.send(status, "application/json", response);
  if (server.uri() != "/status") {
    last_request = server.uri();
    displayStatus(String(status) + ": " + server.uri(), true);    
  }
}


String messageBuilder(bool success, String action) {
  String response = "{";
  response += "\"success\":" + String(success);
  if (!success) {
    if (!loco_selected) {
      response += ", \"message\": \"Unable to " + action + ", no loco currently selected\"";     
    }
    else
    {
      response += ", \"message\": \"Unable to " + action + ", loco is currently " + current_action +"\"";     
    }
  }
  else
  {
    response += ", \"next\": \"refresh\"";
  }
  response += "}";
  return response;
}

// HTTP Handlers


// handles 404 errors
void handleNotFound(){
  String response = "{success:false, information:{";
  response += "uri:";
  response += server.uri();
  response += ", method:";
  response += (server.method() == HTTP_GET)?"GET":"POST";
  response += ", arguments: [";
  for (uint8_t i=0; i<server.args(); i++){
    response +=  server.argName(i) + ":'" + server.arg(i) + "',";
  }
  response += "]}}";
  handleURL(response, 404);
}


// returns available methods
void handleRoot() {
  String response = "{\"methods\":[\"/\"";
  for (int i = 0; i < 8; i++) {
    response += ",\"" + methods[i] + "\"";
  }
  response += "], \"success\":true}";
  handleURL(response, 200);
}


// returns a list of the possible locomotives
void handleList() {
  String response = "{\"locomotives\":[";
  for (int i = 0; i < 10; i++) {
    response += "\"" + locomotive_names[i] + "\",";
  }
  response += "\"" + locomotive_names[10] + "\"";
  response += "], \"success\":true}";
  handleURL(response, true);
}


// handles starting the loco
void handleStart() {
  bool can_start = canStart();
  if (can_start) {
    current_action = "starting";  
  }
  handleURL(messageBuilder(can_start, "start"), can_start);
}


// handles coasting the loco
void handleCoast() {
  bool can_stop = canStop();
  if (can_stop) {
    current_action = "coasting";
  }
  handleURL(messageBuilder(can_stop, "coast"), can_stop);    
}


// handles braking the loco
void handleBrake() {
  bool can_stop = canStop();
  if (can_stop) {
    current_action = "braking";
  }
  handleURL(messageBuilder(can_stop, "brake"), can_stop);    
}


bool changeDirection(String new_direction) {
  bool is_moving = isMoving();
  bool can_change = false;
  if (!is_moving) {
    current_direction = new_direction;
    can_change = true;
  }
  return can_change;  
}

// handles changing the direction of the loco towards Jenbach
void handleDirectionJenbach() {
  bool can_change = changeDirection("Jenbach");
  String response = messageBuilder(can_change, "change direction -> Jenbach");
  handleURL(response, can_change);
}


// handles changing the direction of the loco towards Mayrhofen
void handleDirectionMayrhofen() {
  bool can_change = changeDirection("Mayrhofen");
  String response = messageBuilder(can_change, "change direction -> Mayrhofen");
  handleURL(response, can_change);
}


// handles selecting the loco
// TODO actually select the loco
void handleSelect() {
  int id = (server.arg(0)).toInt();
  current_loco = locomotive_names[id];
  loco_selected = true;
  String response = "{\"locomotive\":{";
  response += "\"id\": "+ String(id);
  response += ", \"name\": \"" + current_loco + "\"";
  response += "}";
  response += ", \"next\": \"refresh\"";
  response += ", \"success\":true}";
  handleURL(response, true);
}


// TODO send pin data
void handleStatus() {
  String response = "{\"success\":true,";
  response += "\"current_loco\": \"" + String(current_loco) + "\",";
  response += "\"current_action\": \"" + String(current_action) + "\",";
  response += "\"current_direction\": \"" + String(current_direction) + "\",";
  response += "\"current_speed\": \"" + String(current_speed) + "\",";
  response += "\"current_mode\": \"" + String(current_mode) + "\",";
  response += "\"raw_pins\":[]";
  response += "}";
  handleURL(response, true);
}


