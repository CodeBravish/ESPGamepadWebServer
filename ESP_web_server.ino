#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <Arduino_JSON.h>
#include <Hash.h>

const char* ssid = "ESP_AccessPoint";
const char* password = "123456";
float throttle = 0;
float turnDirection = 0;

#define SPEED_L 14
#define MOTOR_LF 13  
#define MOTOR_LB 12
#define MOTOR_RF 11
#define MOTOR_RB 10
#define SPEED_R 9

AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
    <head>
        <meta charset="UTF-8" />
        <meta name="viewport" content="width=device-width, initial-scale=1.0" />
        <title>Document</title>
        <link rel="stylesheet" href="style.css" />
        <script src="script.js" defer></script>
    </head>
    <body>
        <h1 title="Made with ♥ by Sahdeek Thompson">Remote Control</h1>
        <h2>Gamepad: <span id="gp-connection-state">Disconnected ❌</span></h2>
        <p>DIRECTION AXIS</p>
        <ul>
            <li>X: <span id="x-axis">-</span></li>
        </ul>
        <div>THROTTLE: <span id="throttle">-</span></div>
        <br>
        <div>DATA <br><span id="data"></span></div>
        <script>
// WEBSOCKET ==============
var gateway = `ws://${window.location.hostname}/ws`
var websocket

window.addEventListener("load", (e) => {
    WebSocketInit()
})

function WebSocketInit() {
    console.log("Attempting to Establish a WebSocket Connection...")
    websocket = new WebSocket(gateway)
    dataHTML.innerHTML += gateway + "<br/>"
    websocket.onopen = onOpen
    websocket.onclose = onClose
    websocket.onmessage = onMessage
}

function onOpen(e) {
    console.log("Connection Opened")
    console.log(e)
}
function onClose(e) {
    console.log("Connection Closed")
    setTimeout(WebSocketInit, 2000)
}
function onMessage(e) {
    console.log(e.data)
    let dataObject = e.data
    dataHTML.innerHTML += dataObject + "<br/>"
}
function sendLoop() {
    websocket.send("Input," + xAxisValue +","+ throttleValue + ",")
}

// GAMEPAD ================
const gpConnectionState = document.getElementById("gp-connection-state")
const throttle = document.getElementById("throttle")
const xAxis = document.getElementById("x-axis")
const dataHTML = document.getElementById("data")
const TRIGGER_ID = 7
const POLLING_TIME_MS = 10
const DEAD_ZONE = 0.1
let xAxisValue = 0
let throttleValue = 0

function gamepadLoop(pollTime, gamepadIndex) {
    setInterval(() => {
        gp = navigator.getGamepads()[gamepadIndex]

        throttleValue = gp.buttons[TRIGGER_ID].value
        throttle.textContent = throttleValue

        if (gp.axes[0] > DEAD_ZONE || gp.axes[0] < -1 * DEAD_ZONE) {
            xAxisValue = gp.axes[0]
            xAxis.textContent = xAxisValue
        } else {
            xAxisValue = 0
            xAxis.textContent = xAxisValue

        }
        sendLoop()
    }, pollTime)
    sendLoop(100)
}

window.addEventListener("gamepadconnected", (e) => {
    gpConnectionState.textContent = "Connected 🟢"
    gamepadLoop(POLLING_TIME_MS, e.gamepad.index)
    console.log(e.gamepad)
})

window.addEventListener("gamepaddisconnected", (e) => {
    gpConnectionState.textContent = "Disconnected ❌"
})


        </script>
    </body>
</html>
)rawliteral";

// void initialiseSPIFFS() 
// {
//     if (!SPIFFS.begin(true)) {
//         Serial.println("Error occured while Mounting SPIFFS");
//     }
//     Serial.println("SPIFFS Mounted Successfully");
// }

void initialiseAccessPoint() 
{
    Serial.print("Setting up Access Point...");
    WiFi.softAP(ssid, password);

    IPAddress IP = WiFi.softAPIP();
    Serial.print("IP Address: ");
    Serial.println(IP);
}

void updateClients() 
{
    ws.textAll("Received");
}

void handleWebSocketMessage(void *arg, uint8_t *data, size_t len) 
{
    AwsFrameInfo *info = (AwsFrameInfo*)arg;
    if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT) {
        data[len] = 0;
        String res = (char*)data;
        if (strstr((char*)data, "Input,") != NULL)
        {
            // vector<string> v;

            // stringstream ss(res);

            // while (ss.good())
            // {
            //     string substr;
            //     getline(ss, substr, ',');
            //     v.push_back(substr);
            // }

            // float throttle = stof(v[1]);
            // float turnDirection = stof(v[2]);

            Serial.println((char*)data);

            updateClients();
        }
    }
}

void onEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
             void *arg, uint8_t *data, size_t len)
{
    switch (type)
    {
    case WS_EVT_CONNECT:
        Serial.printf("WebSocket client #%u connected from %s\n", client->id(), client->remoteIP().toString().c_str());
        break;
    case WS_EVT_DISCONNECT:
        Serial.printf("WebSocket client #%u disconnected\n", client->id());
        break;
    case WS_EVT_DATA:
        handleWebSocketMessage(arg, data, len);
        break;
    case WS_EVT_PONG:
    case WS_EVT_ERROR:
        break;
    }
}

void initialiseWebSocket()
{
    ws.onEvent(onEvent);
    server.addHandler(&ws);
}

void setup()
{
    Serial.begin(115200);
    initialiseAccessPoint();
    // initialiseSPIFFS();
    initialiseWebSocket();

    // server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    //     request->send(SPIFFS, "/index.html", "text.html");
    // });

    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
              { request->send_P(200, "text/html", index_html); });
    server.begin();
}

void loop()
{
    ws.cleanupClients();
}