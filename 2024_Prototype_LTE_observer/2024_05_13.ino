#define TINY_GSM_MODEM_SIM7080
#define GSM_

const char apn[] = "ppsm.jp";
const char lte_user[] = "pp@sim";
const char lte_pass[] = "jpn";

#include <TinyGsmClient.h>
#include <M5Stack.h>
#include <ArduinoHttpClient.h>

const int rx_pin 16:
const int tx_pin 17;

TinyGsm modem(serial);
TinyGsmClient client(modem);

int a;
int b;
int c;

void setup() {
  M5.begin();
  M5.Lcd.setTextSize(3);
  delay(500);
  Serial1.begin(115200, SERIAL_8N1, rx_pin, tx_pin);
  delay(3000);

  M5.Lcd.println("wait...");
  
  //モデム初期化
  M5.Lcd.println("Initializing modem...");
  if(!modem.init()){
    M5.Lcd.println("wait again...");
    return;
  } 
  String modemInfo = modem.getModemInfo();
  SerialMon.println("Modem Info: " + modemInfo);
  
　//接続
  SerialMon.println("Connecting to "+ String(apn));
  while (!modem.gprsConnect(apn, lte_user, lte_pass)) {
    SerialMon.println("NG.");
    delay(10000);
    SerialMon.println("retry");
}

void loop() {
  // put your main code here, to run repeatedly:

  M5.update();  //M5Stackのボタンの操作状況を更新する関数

  if(M5.BtnA.wasReleased()){    //
    //ボタン”A”が押された時の動作
    a = 1;
    M5.Lcd.fillScreen(BLACK); //画面をクリア
    M5.Lcd.setTextColor(RED, BLACK);  //文字色と背景色
    M5.Lcd.setCursor(0, 10);  //文字の位置指定
    M5.Lcd.println("Red Rectangle 1");  //文字の表示
    M5.Lcd.fillRect(135, 95, 50, 50, RED); //図形の表示：四角形
    
  } else if(M5.BtnB.wasReleased()){
    //ボタン”B”が押された時の動作
    b = 2;
    M5.Lcd.fillScreen(BLACK); //画面をクリア
    M5.Lcd.setTextColor(GREEN, BLACK);  //文字色と背景色
    M5.Lcd.setCursor(0, 10);  //文字の位置指定
    M5.Lcd.println("Green Circle 2"); //文字の表示
    M5.Lcd.fillCircle(160, 120, 25, GREEN); //図形の表示：円
    
  } else if(M5.BtnC.wasReleased()){
    //ボタン”C”が押された時の動作
    c = 3;
    M5.Lcd.fillScreen(BLACK); //画面をクリア
    M5.Lcd.setTextColor(BLUE, BLACK); //文字色と背景色
    M5.Lcd.setCursor(0, 10);  //文字の位置指定
    M5.Lcd.println("Blue Triangle 3");  //文字の表示
    M5.Lcd.fillTriangle(135, 145, 185, 145, 160, 95, BLUE);  //図形の表示：三角形
  }

}
