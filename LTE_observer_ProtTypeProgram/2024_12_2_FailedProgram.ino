#define TINY_GSM_MODEM_SIM7600
#define SerialMon Serial
#define SerialAT Serial1

const char apn[] = "ppsm.jp";
const char lte_user[] = "pp@sim";
const char lte_pass[] = "jpn";

#include <TinyGsmClient.h>
#include <M5Stack.h>
#include <ArduinoHttpClient.h>

#define RX 16
#define TX 17

TinyGsm modem(SerialAT);
TinyGsmClient client(modem);

int a;
int b;
int c;

void setup() {
  delay(500);
  SerialMon.begin(115200);
  SerialAT.begin(115200, SERIAL_8N1, RX, TX);
  delay(3000);

  SerialMon.println("Wait...");
  //モデム初期化
  SerialMon.println("Initializing modem...");
  modem.init();
  String modemInfo = modem.getModemInfo();
  SerialMon.println("Modem Info: " + modemInfo);
  //接続
  SerialMon.println("Connecting to "+ String(apn));
  while (!modem.gprsConnect(apn, lte_user, lte_pass)) {
    SerialMon.println("NG.");
    delay(10000);
    SerialMon.println("retry");
  }
  SerialMon.println("OK.");
  //接続まち
  SerialMon.print("Waiting for network...");
  if (!modem.waitForNetwork()) {
    SerialMon.println(" fail");
    delay(10000);
    return;
  }
  SerialMon.println(" success");
  bool res = modem.isGprsConnected();
  SerialMon.printf("GPRS status: %s\n", res ? "connected" : "not connected");

  //各種情報を出力
  String ccid = modem.getSimCCID();
  SerialMon.println("CCID: "+ ccid);
  String imei = modem.getIMEI();
  SerialMon.println("IMEI: "+ imei);
  String cop = modem.getOperator();
  SerialMon.println("Operator: "+ cop);
  IPAddress local = modem.localIP();
  SerialMon.print("Local IP: ");
  SerialMon.println(local);
  int csq = modem.getSignalQuality();
  SerialMon.print("Signal quality: ");
  SerialMon.println(csq);

  //乱数を初期化
  randomSeed(analogRead(25));
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
