#include <WiFi.h>
#include <WiFiUdp.h>
#include <WebServer.h>

// ------------------- Motor Driver Pins -------------------
#define RMF 25
#define RMR 26
#define LMF 27
#define LMR 14

// PWM settings
#define PWM_FREQ 2000
#define PWM_RESOLUTION 8

// PWM Channels
#define CH_RMF 0
#define CH_RMR 1
#define CH_LMF 2
#define CH_LMR 3

int motorSpeed = 150;

// ------------------- WiFi AP -------------------
const char* ssid = "Robot_AP";
const char* password = "12345678";

// ------------------- UDP -------------------
WiFiUDP udp;
const int udpPort = 4210;
char incomingPacket[255];

// ------------------- Web Debug -------------------
WebServer server(80);
String lastCommand = "NONE";
unsigned long lastPacketTime = 0;

// ------------------- Setup PWM -------------------
void setupPWM() {
  ledcAttachChannel(RMF, PWM_FREQ, PWM_RESOLUTION, CH_RMF);
  ledcAttachChannel(RMR, PWM_FREQ, PWM_RESOLUTION, CH_RMR);
  ledcAttachChannel(LMF, PWM_FREQ, PWM_RESOLUTION, CH_LMF);
  ledcAttachChannel(LMR, PWM_FREQ, PWM_RESOLUTION, CH_LMR);
}

// ------------------- Motor Control -------------------
void setMotors(int LM, int RM) {
  LM = constrain(LM, -255, 255);
  RM = constrain(RM, -255, 255);

  // Left Motor
  if (LM >= 0) {
    ledcWrite(CH_LMF, LM);
    ledcWrite(CH_LMR, 0);
  } else {
    ledcWrite(CH_LMF, 0);
    ledcWrite(CH_LMR, -LM);
  }

  // Right Motor
  if (RM >= 0) {
    ledcWrite(CH_RMF, RM);
    ledcWrite(CH_RMR, 0);
  } else {
    ledcWrite(CH_RMF, 0);
    ledcWrite(CH_RMR, -RM);
  }
}

void stopMotors() {
  setMotors(0, 0);
}

// ------------------- Command Logic -------------------
void processCommand(String cmd) {
  cmd.trim();
  lastCommand = cmd;

  Serial.println("CMD: " + cmd);

  if (cmd == "FORWARD") {
    setMotors(motorSpeed, motorSpeed);
  }
  else if (cmd == "BACKWARD") {
    setMotors(-motorSpeed, -motorSpeed);
  }
  else if (cmd == "LEFT") {
    setMotors(-motorSpeed, motorSpeed);
  }
  else if (cmd == "RIGHT") {
    setMotors(motorSpeed, -motorSpeed);
  }
  else if (cmd == "STOP") {
    stopMotors();
  }
}

// ------------------- Web Debug Page -------------------
void handleRoot() {
  String html = "<!DOCTYPE html><html>";
  html += "<head><meta http-equiv='refresh' content='1'>";
  html += "<style>body{font-family:Arial;text-align:center;} h1{color:#333;} .box{font-size:24px;margin-top:20px;}</style>";
  html += "</head><body>";

  html += "<h1>🤖 ESP32 Robot Debug</h1>";
  html += "<div class='box'>Last Command: <b>" + lastCommand + "</b></div>";
  html += "<div class='box'>Last Packet (ms ago): <b>" + String(millis() - lastPacketTime) + "</b></div>";

  html += "</body></html>";

  server.send(200, "text/html", html);
}

// ------------------- Setup -------------------
void setup() {
  Serial.begin(115200);

  setupPWM();

  WiFi.softAP(ssid, password);
  Serial.print("AP IP: ");
  Serial.println(WiFi.softAPIP());

  udp.begin(udpPort);
  Serial.println("UDP listening...");

  // Web server
  server.on("/", handleRoot);
  server.begin();
}

// ------------------- Loop -------------------
void loop() {
  int packetSize = udp.parsePacket();

  if (packetSize) {
    lastPacketTime = millis();

    int len = udp.read(incomingPacket, 255);
    if (len > 0) incomingPacket[len] = 0;

    String cmd = String(incomingPacket);

    // Debug info
    Serial.print("From IP: ");
    Serial.println(udp.remoteIP());

    processCommand(cmd);
  }

  // Fail-safe stop
  if (millis() - lastPacketTime > 500) {
    stopMotors();
  }

  server.handleClient();
}