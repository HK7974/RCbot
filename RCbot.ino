#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

#define UP 1
#define DOWN 2
#define LEFT 3
#define RIGHT 4
#define TOGGLESTART 8
#define FORWARD 1
#define BACKWARD -1
#define FRONT_RIGHT_MOTOR 0
#define BACK_RIGHT_MOTOR 3
#define FRONT_LEFT_MOTOR 2
#define BACK_LEFT_MOTOR 1
#define LEDPIN 15
#define STOP 64

bool state =0;
struct MOTOR_PINS
{
  int pinIN1;
  int pinIN2;    
};

std::vector<MOTOR_PINS> motorPins = 
{
  {32, 33},  //FRONT_RIGHT_MOTOR
  {21, 19},  //BACK_LEFT_MOTOR
  {27, 26},  //FRONT_LEFT_MOTOR
  {23, 22},  //BACK_RIGHT_MOTOR   
};

const char* ssid     = "RCWifi";
const char* password = "willthiswork";

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");


const char* htmlHomePage PROGMEM = R"HTMLHOMEPAGE(
<!DOCTYPE html>
<!DOCTYPE html>
<html>
  <head>
<title>Control Station</title>
  <meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
    <style>
    .unselectable {
    -webkit-touch-callout: none;
    -webkit-user-select: none;
    -khtml-user-select: none;
    -moz-user-select: none;
    -ms-user-select: none;
    user-select: none;
    }
    .straight {
      font-size:100px;
      color:blue;
    }
    .turns {
      font-size:100px;
      color:yellow;
    }
    td {
      background-color:black;
      border-radius:15%;
      box-shadow: 5px 5px #888888;
    }
    td:active {
      transform: translate(5px,5px);
      box-shadow: none; 
    }
    .empty{
	background-color:rgba(0,0,0,0);
	box-shadow: none;
	}
	.toggle{
background-color:red;
color:green;
font-size: 100px;
}
  
    }
    </style>
  </head>
  <body class="unselectable" align="center" style="background-color:aquamarine">
     
    <h1 style="color: blue;text-align:center;">RC Bot Control</h1>

    <table id="TableofFire" style="width:400px;margin:auto;table-layout:fixed" CELLSPACING=10>
      <tr>
        <td  class="empty"><span class="empty" >&nbsp</span></td>
        <td ontouchstart='SendMessage("1")' ontouchend='SendMessage("0")'><span class="straight" >&#8679;</span></td>
        <td  class="empty"><span class="straight" >&nbsp;</span></td>
      </tr>
      
      <tr>
        <td ontouchstart='SendMessage("3")' ontouchend='SendMessage("0")'><span class="turns" >&#8678;</span></td>
        <td ontouchstart='SendMessage("8")' class="toggle">&#9881;	</td>    
        <td ontouchstart='SendMessage("4")' ontouchend='SendMessage("0")'><span class="turns" >&#8680;</span></td>
      </tr>
      
      <tr>
        <td class="empty")'>&nbsp</td>
        <td ontouchstart='SendMessage("2")' ontouchend='SendMessage("0")'><span class="straight" >&#8681;</span></td>
        <td class="empty">&nbsp</td>
      </tr>
    
      
    </table>

    <script>
      var webSocketUrl = "ws:\/\/" + window.location.hostname + "/ws";
      var websocket;
      
      function initWebSocket() 
      {
        websocket = new WebSocket(webSocketUrl);
        websocket.onopen    = function(event){};
        websocket.onclose   = function(event){setTimeout(initWebSocket, 2000);};
        websocket.onmessage = function(event){};
      }

      function SendMessage(value) 
      {
        websocket.send(value);
      }
          
      window.onload = initWebSocket;
      document.getElementById("mainTable").addEventListener("touchend", function(event){
        event.preventDefault()
      });      
    </script>
    
  </body>
</html> 

)HTMLHOMEPAGE";


void rotateMotor(int motorNumber, int motorDirection)
{
  if (motorDirection == FORWARD)
  {
    digitalWrite(motorPins[motorNumber].pinIN1, HIGH);
    digitalWrite(motorPins[motorNumber].pinIN2, LOW);    
  }
  else if (motorDirection == BACKWARD)
  {
    digitalWrite(motorPins[motorNumber].pinIN1, LOW);
    digitalWrite(motorPins[motorNumber].pinIN2, HIGH);     
  }
  else
  {
    digitalWrite(motorPins[motorNumber].pinIN1, LOW);
    digitalWrite(motorPins[motorNumber].pinIN2, LOW);       
  }
}
void clearmotor(){
  for (int i = 0; i < motorPins.size(); i++)
  {
    pinMode(motorPins[i].pinIN1, OUTPUT);
    pinMode(motorPins[i].pinIN2, OUTPUT);    
  }
}
void processCarMovement(String inputValue)
{
  if (inputValue.toInt()==TOGGLESTART){state= !state; pinMode(LEDPIN,state) ;}
  if(state){
  Serial.printf("Got value as %s %d\n", inputValue.c_str(), inputValue.toInt());  
  switch(inputValue.toInt())
  {
    clearmotor();
    case UP:
      rotateMotor(FRONT_RIGHT_MOTOR, FORWARD);
      rotateMotor(BACK_RIGHT_MOTOR, FORWARD);
      rotateMotor(FRONT_LEFT_MOTOR, FORWARD);
      rotateMotor(BACK_LEFT_MOTOR, FORWARD);       
      Serial.println("UP");             
      break;
  
    case DOWN:
      rotateMotor(FRONT_RIGHT_MOTOR, BACKWARD);
      rotateMotor(BACK_RIGHT_MOTOR, BACKWARD);
      rotateMotor(FRONT_LEFT_MOTOR, BACKWARD);
      rotateMotor(BACK_LEFT_MOTOR, BACKWARD);   
      Serial.println("Down");
      break;
  
    case LEFT:
      rotateMotor(FRONT_RIGHT_MOTOR, FORWARD);
      rotateMotor(BACK_RIGHT_MOTOR, FORWARD);
      rotateMotor(FRONT_LEFT_MOTOR, BACKWARD);
      rotateMotor(BACK_LEFT_MOTOR, BACKWARD);   
      Serial.println("Left");
      break;
  
    case RIGHT:
      rotateMotor(FRONT_RIGHT_MOTOR, BACKWARD);
      rotateMotor(BACK_RIGHT_MOTOR, BACKWARD);
      rotateMotor(FRONT_LEFT_MOTOR, FORWARD);
      rotateMotor(BACK_LEFT_MOTOR, FORWARD);  
      Serial.println("RIGHT");
      break;

    default:
      rotateMotor(FRONT_RIGHT_MOTOR, STOP);
      rotateMotor(BACK_RIGHT_MOTOR, STOP);
      rotateMotor(FRONT_LEFT_MOTOR, STOP);
      rotateMotor(BACK_LEFT_MOTOR, STOP);    
      break;
  }
}}

void handleRoot(AsyncWebServerRequest *request) 
{
  request->send_P(200, "text/html", htmlHomePage);
}

void handleNotFound(AsyncWebServerRequest *request) 
{
    request->send(404, "text/plain", "File Not Found");
}


void onWebSocketEvent(AsyncWebSocket *server, 
                      AsyncWebSocketClient *client, 
                      AwsEventType type,
                      void *arg, 
                      uint8_t *data, 
                      size_t len) 
{                      
  switch (type) 
  {
    case WS_EVT_CONNECT:
      Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
      break;
    case WS_EVT_DISCONNECT:
      Serial.printf("WebSocket client #%u disconnected\n", client->id());
      processCarMovement("0");
      break;
    case WS_EVT_DATA:
      AwsFrameInfo *info;
      info = (AwsFrameInfo*)arg;
      if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) 
      {
        std::string myData = "";
        myData.assign((char *)data, len);
        processCarMovement(myData.c_str());       
      }
      break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
      break;
    default:
      break;  
  }
}

void setUpPinModes()
{
  for (int i = 0; i < motorPins.size(); i++)
  {
    pinMode(motorPins[i].pinIN1, OUTPUT);
    pinMode(motorPins[i].pinIN2, OUTPUT);    
  }
}


void setup(void) 
{
  pinMode(LEDPIN,OUTPUT);
  setUpPinModes();
  Serial.begin(115200);

  WiFi.softAP(ssid, password);
  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  server.on("/", HTTP_GET, handleRoot);
  server.onNotFound(handleNotFound);
  
  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);
  
  server.begin();
  Serial.println("HTTP server started");
  /*delay(2000);
  rotateMotor(FRONT_RIGHT_MOTOR, FORWARD);
  delay(2000);
  rotateMotor(FRONT_RIGHT_MOTOR, BACKWARD);
  delay(2000);
  rotateMotor(FRONT_RIGHT_MOTOR, STOP);
  delay(2000);
 rotateMotor(BACK_RIGHT_MOTOR, FORWARD);
  delay(2000);
  rotateMotor(BACK_RIGHT_MOTOR, BACKWARD);
  delay(2000);
  rotateMotor(FRONT_RIGHT_MOTOR, STOP);
  rotateMotor(BACK_RIGHT_MOTOR, STOP);
  delay(2000);
 rotateMotor(FRONT_LEFT_MOTOR, FORWARD);
  delay(2000);
  rotateMotor(FRONT_LEFT_MOTOR, BACKWARD);
  delay(2000);
  rotateMotor(FRONT_LEFT_MOTOR, STOP);
  delay(2000);
 rotateMotor(BACK_LEFT_MOTOR, FORWARD);
  delay(2000);
  rotateMotor(BACK_LEFT_MOTOR, BACKWARD);
  delay(2000);
  rotateMotor(BACK_LEFT_MOTOR, STOP);
  delay(100);


  */
}

void loop() 
{
  ws.cleanupClients(); 
}