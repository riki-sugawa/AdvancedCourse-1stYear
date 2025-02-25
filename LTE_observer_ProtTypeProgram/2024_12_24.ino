#include <M5Stack.h>
#include <SoftwareSerial.h>
#include <HTTPClient.h>

SoftwareSerial sim7600(16, 17);  

const char* apn = "ppsim.jp";  
const char* user = "pp@sim";         
const char* pass = "jpn";

/*
const String sim_apn = "ppsim.jp";  
const String sim_user = "pp@sim";
const String sim_pass = "jpn";
*/

//const char* ssid     = "TP-Link_146C";
//const char* password = "52777882";
const char* deployid = "AKfycbyn3lh2EIiGJLmwycbL_wMe4bBCWjd01676xPmYAMv5kteDBWWmkTetyJap5VSfnLDI";
uint32_t    interval = 30; // unit:sec
uint16_t    chipid   = ESP.getEfuseMac() % 10000;

void setup() {
  
  M5.begin();
  Serial.begin(115200);  
  sim7600.begin(9600);
  M5.Lcd.setTextSize(2);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setCursor(0, 0);

  delay(3000);  
//ATコマンド開始
  Serial.println("Initializing SIM7600...");// SIM7600モジュールにATコマンドで通信確認
  sendATCommand("AT", 1000);  // ATコマンドで応答確認
  sendATCommand("AT+CSQ", 1000);  // 信号強度を確認
  sendATCommand("AT+CGATT=1", 3000);  // GPRS接続を開始

  // APNの設定
  String apnCommand = "AT+CGDCONT=1,\"IP\",\"" + String(apn) + "\"";
  String user_passCommand = "AT+CGAUTH = 1, 1, \"" + String(user) + "\"", \"jpn\"\r\n"
  sendATCommand(apnCommand.c_str(), 3000);

  sendATCommand("AT+CGACT=1,1", 5000);  // GPRS接続を有効化
  sendATCommand("AT+CGPADDR", 3000);  // 接続後、IPアドレスを確認
}

void loop() {
  
}

void sendATCommand(const char* command, int timeout) {
  // ATコマンドを送信し、応答を待つ
  String response = "";
  sim7600.println(command);
  unsigned long start = millis();

  while (millis() - start < timeout) {
    if (sim7600.available()) {
      char c = sim7600.read();
      response += c;
    }
  }

  Serial.print("Sent: ");
  Serial.println(command);
  Serial.print("Response: ");
  Serial.println(response);
  delay(1000);
}