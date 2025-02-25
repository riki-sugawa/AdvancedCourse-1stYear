//https://github.com/Xinyuan-LilyGO/LilyGo-T-PCIE/blob/master/examples/SIM7600/SIM7600.inoを元にして作成
//通信モデムの設定
#define TINY_GSM_MODEM_SIM7600

//シリアルモニタに出力するためのserialを設定
#define SerialMon Serial

//通信モデムとESP32がやりとりするためのSerialポートを設定
#define SerialAT Serial1

#include <TinyGsmClient.h>
#include <Ticker.h>
#include <ArduinoHttpClient.h>

//通信先を設定。自分の通信会社のAPNとUser名、パスワードを設定
const char apn[] = "iijmio.jp";
const char gprsUser[] = "mio@iij";
const char gprsPass[] = "iij";

//アクセス先の設定 http://mkbtm.net//SIM7600/test.txt
//httpsを使いたい時には、BasicHttpsClientのサンプルを見る
const char server[]   = "mkbtm.net";
const char resource[] = "/SIM7600/test.txt";
const int  port       = 80;

TinyGsm modem(SerialAT);

Ticker tick;//LEDの点滅用

#define uS_TO_S_FACTOR          1000000ULL  //Conversion factor for micro seconds to seconds 
#define TIME_TO_SLEEP           60          //Time ESP32 will go to sleep (in seconds) 
//ボーdの設定
#define PIN_TX                  27
#define PIN_RX                  26
#define UART_BAUD               115200

/*
#define PWR_PIN                 4
#define LED_PIN                 12
#define POWER_PIN               25
#define IND_PIN                 36
*/


void setup()
{
  SerialMon.begin(115200);
  delay(10);
  
  /*
  // Onboard LED light, it can be used freely
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // POWER_PIN : This pin controls the power supply of the SIM7600
  pinMode(POWER_PIN, OUTPUT);
  digitalWrite(POWER_PIN, HIGH);

  // PWR_PIN ： This Pin is the PWR-KEY of the SIM7600
  // The time of active low level impulse of PWRKEY pin to power on module , type 500 ms
  pinMode(PWR_PIN, OUTPUT);
  digitalWrite(PWR_PIN, HIGH);
  delay(500);
  digitalWrite(PWR_PIN, LOW);

  // IND_PIN: It is connected to the SIM7600 status Pin,
  // through which you can know whether the module starts normally.
  pinMode(IND_PIN, INPUT);

  attachInterrupt(IND_PIN, []() {
    detachInterrupt(IND_PIN);
    // If SIM7600 starts normally, then set the onboard LED to flash once every 1 second
    tick.attach_ms(1000, []() {
      digitalWrite(LED_PIN, !digitalRead(LED_PIN));
    });
  }, CHANGE);
  */

  SerialMon.println("Wait...");

  delay(3000);

  SerialAT.begin(UART_BAUD, SERIAL_8N1, PIN_RX, PIN_TX);

  //モデムの初期化
  SerialMon.println("Initializing modem...");
  if (!modem.init()) {
    SerialMon.println("Failed to restart modem, delaying 10s and retrying");
    return;
  }
  SerialMon.println("enter setNetwork Mode");

//接続開始
  bool result;
  do {
    result = modem.setNetworkMode(38);//2 Automatic, 13 GSM only,  38 LTE only,  51 GSM and LTE only
    delay(500);
  } while (result != true);

  SerialMon.println("Waiting for network...");
  if (!modem.waitForNetwork()) {
    delay(10000);
    return;
  }

  if (modem.isNetworkConnected()) {
    SerialMon.println("Network connected");
  }

  SerialMon.print("Connecting to:");
  SerialMon.println(apn);
  if (!modem.gprsConnect(apn, gprsUser, gprsPass)) {
    delay(10000);
    return;
  }

  bool res = modem.isGprsConnected();
  SerialMon.print("GPRS status:");
  SerialMon.println(res);

  IPAddress local = modem.localIP();
  SerialMon.print("Local IP:");
  SerialMon.println(local);

  int csq = modem.getSignalQuality();
  SerialMon.print("Signal quality:");
  SerialMon.println(csq);


  //httpの接続開始
  TinyGsmClient client(modem);
  HttpClient    http(client, server, port);

  SerialMon.println("HTTP GET request... ");
  int err = http.get(resource);
  if (err != 0) {
    SerialMon.println("failed to connect");
    delay(10000);
    return;
  }

//httpの通信結果を表示
  String body = http.responseBody();
  SerialMon.println("============Response===========");
  SerialMon.println(body);
  SerialMon.println("============Response end========");

  http.stop();
  SerialMon.println("Server disconnected");

  modem.poweroff();//モデムオフ
  SerialMon.println("Poweroff.");
}



void loop()
{
//スリープへ
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  delay(200);
  esp_deep_sleep_start();
}