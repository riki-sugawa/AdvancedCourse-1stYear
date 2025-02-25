#define TINY_GSM_MODEM_SIM7080
#define JSON_OBJECT_SIZE

#include <TinyGsmClient.h>
#include <M5Stack.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>
#include <stdio.h>

const int capacity = JSON_OBJECT_SIZE(2);
StaticJsonDocument<capacity> json_data;
char buffer[255];

const char apn[] = "ppsm.jp";
const char lte_user[] = "pp@sim";
const char lte_pass[] = "jpn";

const int rx_pin 16;
const int tx_pin 17;

//gasのurl
const char* gas_url = "https://script.google.com/macros/s/AKfycbz9aDiciZl1bY3dXUXgHQdXa11v569RTmywIdSNYmc_ORMiW0v3zbEFs1Gy8VgkQ_di/exec";

TinyGsm modem(Serial);
TinyGsmClient client(modem);

int a;
int b;
int c;
int data;

unsigned long counter = 0;

void setup() {
  
  M5.begin();
  M5.Lcd.setTextSize(3);
  M5.Lcd.fillScreen(BLACK);
  M5.Lcd.setTextColor(WHITE,BLACK);
  delay(500);
  Serial.begin(115200, SERIAL_8N1, RX, TX);
  delay(3000);

  M5.Lcd.println("wait...");
  
  //モデム初期化
  M5.Lcd.println("Initializing modem (モデム初期化)...");
  Serial.println("Initializing modem...");
  modem.init();
  String modemInfo = modem.getModemInfo();
  M5.Lcd.println("Modem Info: " + modemInfo);
  M5.Lcd.println("conect to the Internet");
  Serial.println("Modem Info: " + modemInfo);

  //接続
  M5.Lcd.println("Connecting to "+ String(apn));
  while (!modem.gprsConnect(apn, lte_user, lte_pass)) {
    M5.Lcd.println("NG.");
    delay(10000);
    M5.Lcd.println("retry");
  }
  M5.Lcd.println("OK");

  //接続まち
  M5.Lcd.print("Waiting for network...");
  if (!modem.waitForNetwork()) {
    M5.Lcd.println("fail");
    M5.Lcd.println("Please wait 10 sec...");
    delay(10000);
    return;
  }
  
}

void loop() {
  // put your main code here, to run repeatedly:

  M5.update();  //M5Stackのボタンの操作状況を更新する関数

  if(M5.BtnA.wasReleased()){
    
    //ボタン”A”が押された時の動作
    a = 1;
    M5.Lcd.fillScreen(BLACK); //画面をクリア
    M5.Lcd.setTextColor(RED, BLACK);  //文字色と背景色
    M5.Lcd.setCursor(0, 10);  //文字の位置指定
    M5.Lcd.println("Red Rectangle 1");  //文字の表示
    M5.Lcd.fillRect(135, 95, 50, 50, RED); //図形の表示：四角形
    delay(5000);

    //データ送信プログラム開始
    counter++;
    json_data["data"] = a;
    json_data["counter"] = counter;

    serializeJson(json_data, Serial);
    Serial.println("");
    serializeJson(json_data, buffer, sizeof(buffer));
    
    HTTPClient http;
    http.begin(gas_url); //http通信確率
    http.addHeader("content-type", "application/json");
    http.POST((uint8_t)buffer, strlen(buffer)); //データ送信？    
  } 

}