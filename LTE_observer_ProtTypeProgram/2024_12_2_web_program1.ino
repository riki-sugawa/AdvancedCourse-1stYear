#include <HardwareSerial.h>

HardwareSerial sim7080Serial(1); // RX=GPIO16, TX=GPIO17

#define SIM7080_RX 16
#define SIM7080_TX 17

// Sleep and wakeup intervals (milliseconds)
#define SLEEP_INTERVAL 300000 // 5 minutes
#define WAKEUP_INTERVAL 1000 // 1 second

void setup() {
  Serial.begin(115200);
  sim7080Serial.begin(115200, SERIAL_8N1, SIM7080_RX, SIM7080_TX);
  delay(1000);
  Serial.println("ESP32 SIM7080 GNSS");

  initSIM7080();
}

void loop() {
  getGNSS();
  sim7080Sleep();
  delay(SLEEP_INTERVAL);
  sim7080WakeUp();
  delay(WAKEUP_INTERVAL);
}

void initSIM7080() {
  sendATcommand("AT+CFUN=1");
  sendATcommand("AT+CGNSPWR=1"); // Enable GNSS
}

void getGNSS() {
  sim7080Serial.println("AT+CGNSINF");
  delay(500);

  String response = "";
  while (sim7080Serial.available()) {
    response += (char)sim7080Serial.read();
  }

  int index = response.indexOf("+CGNSINF:");
  if (index > 0) {
    String gnssInfo = response.substring(index + 9);
    Serial.println("GNSS Info:");
    Serial.println(gnssInfo);
  } else {
    Serial.println("Unable to retrieve GNSS info");
  }
}

void sim7080Sleep() {
  sendATcommand("AT+CSCLK=1"); // Enable sleep mode
}

void sim7080WakeUp() {
  sendATcommand("AT+CSCLK=0"); // Disable sleep mode
}

void sendATcommand(String command) {
  sim7080Serial.println(command);
  delay(500);

  while (sim7080Serial.available()) {
    char c = sim7080Serial.read();
    Serial.print(c);
  }
}