#include <M5Stack.h>
#include <stdint.h>
#include <vector>
#include "TFTTerminal.h"

/*serial通信のinclude

#include <WiFi.h>
#include <WiFiMulti.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

*/

TFT_eSprite Disbuff      = TFT_eSprite(&M5.Lcd);
TFT_eSprite TerminalBuff = TFT_eSprite(&M5.Lcd);
TFTTerminal terminal(&TerminalBuff);
TaskHandle_t xhandle_lte_event = NULL;
SemaphoreHandle_t command_list_samap;


const int capacity = JSON_OBJECT_SIZE(2);           //serial通信をする
StaticJsonDocument<capacity> json_request;
char buffer[255];


uint32_t    interval = 10; // unit:sec
uint16_t    chipid   = ESP.getEfuseMac() % 10000;
const char* deployid = "AKfycbw1vfUnV_E6B9O0tWaKMPKXkvahMsXIG-jrpk2SS2a1kGAAz59iQy1_p_H7kp7x8VnKsg";

const char *send_url = "https://script.google.com/macros/s/AKfycbw1vfUnV_E6B9O0tWaKMPKXkvahMsXIG-jrpk2SS2a1kGAAz59iQy1_p_H7kp7x8VnKsg/exec";

//String url = "https://script.google.com/macros/s/" + String(deployid) + "/exec?chipid=ESP" + String(chipid) + "&val0=" + String(val0) + "&val1=" + String(val1) + "&val2=" + String(val2);

unsigned long counter = 0;
unsigned long tick = 0;
const char* data = "Hello World";

const String sim_apn = "ppsim.jp";  
const String sim_user = "pp@sim";
const String sim_pass = "jpn";

typedef enum {
    kQUERY_MO = 0,
    KTEST_MO,
    kASSIGN_MO,
    kACTION_MO,
    kQUERY_MT,
    kTEST_MT,
    kASSIGN_MT,
    kACTION_MT,
    kINFORM
} LTEMsg_t;

typedef enum {
    kErrorSendTimeOUT = 0xe1,
    kErrorReError     = 0xe2,
    kErroeSendError   = 0xe3,
    kSendReady        = 0,
    kSending          = 1,
    kWaitforMsg       = 2,
    kWaitforRead      = 3,
    kReOK
} LTEState_t;

struct ATCommand {
    uint8_t command_type;
    String str_command;
    uint16_t send_max_number;
    uint16_t max_time;
    uint8_t state;
    String read_str;
    uint16_t _send_count;
    uint16_t _send_time_count;

} user;

using namespace std;
vector<ATCommand> serial_at;
String zmmi_str;
void LTEModuleTask(void* arg) {
    int Number = 0;
    String restr;
    while (1) {
        xSemaphoreTake(command_list_samap, portMAX_DELAY);

        if (Serial2.available() != 0) {
            String str = Serial2.readString();
            restr += str;

            if (restr.indexOf("\r\n") != -1) {
            }

            if (restr.indexOf("+ZMMI:") != -1) {
                zmmi_str = restr;
            } else if ((restr.indexOf("OK") != -1) ||
                       (restr.indexOf("ERROR") != -1)) {
                Serial.print(restr);
                if (restr.indexOf("OK") != -1) {
                    if ((serial_at[0].command_type == kACTION_MO) ||
                        (serial_at[0].command_type == kASSIGN_MO)) {
                        serial_at.erase(serial_at.begin());
                        Serial.printf("erase now %d\n", serial_at.size());
                    } else {
                        serial_at[0].read_str = restr;
                        serial_at[0].state    = kWaitforRead;
                    }
                } else if (restr.indexOf("ERROR") != -1) {
                    serial_at[0].state = kErrorReError;
                } else {
                }
                restr.clear();
            }
        }

        if (serial_at.empty() != true) {
            Number = 0;
            switch (serial_at[0].state) {
                case kSendReady:
                    Serial.printf(serial_at[0].str_command.c_str());
                    Serial2.write(serial_at[0].str_command.c_str());
                    serial_at[0].state = kSending;
                    break;
                case kSending:

                    if (serial_at[0]._send_time_count > 0) {
                        serial_at[0]._send_time_count--;
                    } else {
                        serial_at[0].state = kWaitforMsg;
                    }
                    /* code */
                    break;
                case kWaitforMsg:
                    if (serial_at[0]._send_count > 0) {
                        serial_at[0]._send_count--;
                        serial_at[0]._send_time_count = serial_at[0].max_time;
                        Serial.printf(serial_at[0].str_command.c_str());
                        Serial2.write(serial_at[0].str_command.c_str());
                        restr.clear();
                        serial_at[0].state = 1;
                    } else {
                        serial_at[0].state = kErrorSendTimeOUT;
                    }
                    /* code */
                    break;
                case kWaitforRead:
                    /* code */
                    break;
                case 4:
                    /* code */
                    break;
                case kErrorSendTimeOUT:
                    /* code */
                    break;
                case 0xe2:
                    /* code */
                    break;
                default:
                    break;
            }
        }
        xSemaphoreGive(command_list_samap);
        delay(10);
    }
}

void AddMsg(String str, uint8_t type, uint16_t sendcount, uint16_t sendtime) {
    struct ATCommand newcommand;
    newcommand.str_command      = str;
    newcommand.command_type     = type;
    newcommand.max_time         = sendtime;
    newcommand.send_max_number  = sendcount;
    newcommand.state            = 0;
    newcommand._send_count      = sendcount;
    newcommand._send_time_count = sendtime;
    xSemaphoreTake(command_list_samap, portMAX_DELAY);
    serial_at.push_back(newcommand);
    xSemaphoreGive(command_list_samap);
}

uint8_t readSendState(uint32_t number) {
    xSemaphoreTake(command_list_samap, portMAX_DELAY);
    uint8_t restate = serial_at[number].state;
    xSemaphoreGive(command_list_samap);
    return restate;
}

uint32_t getATMsgSize() {
    xSemaphoreTake(command_list_samap, portMAX_DELAY);
    uint32_t restate = serial_at.size();
    xSemaphoreGive(command_list_samap);
    return restate;
}
String ReadMsgstr(uint32_t number) {
    xSemaphoreTake(command_list_samap, portMAX_DELAY);
    String restate = serial_at[number].read_str;
    xSemaphoreGive(command_list_samap);
    return restate;
}

bool EraseFirstMsg() {
    xSemaphoreTake(command_list_samap, portMAX_DELAY);
    serial_at.erase(serial_at.begin());
    xSemaphoreGive(command_list_samap);
    return true;
}

uint16_t GetstrNumber(String Str, uint32_t* ptrbuff) {
    uint16_t count = 0;
    String Numberstr;
    int indexpos = 0;
    while (Str.length() > 0) {
        indexpos = Str.indexOf(",");
        if (indexpos != -1) {
            Numberstr      = Str.substring(0, Str.indexOf(","));
            Str            = Str.substring(Str.indexOf(",") + 1, Str.length());
            ptrbuff[count] = Numberstr.toInt();
            count++;
        } else {
            ptrbuff[count] = Str.toInt();
            count++;
            break;
        }
    }
    return count;
}

vector<String> restr_v;
uint16_t GetstrNumber(String StartStr, String EndStr, String Str) {
    uint16_t count = 0;
    String Numberstr;
    int indexpos = 0;

    Str = Str.substring(Str.indexOf(StartStr) + StartStr.length(),
                        Str.indexOf(EndStr));
    Str.trim();
    restr_v.clear();

    while (Str.length() > 0) {
        indexpos = Str.indexOf(",");
        if (indexpos != -1) {
            Numberstr = Str.substring(0, Str.indexOf(","));
            Str       = Str.substring(Str.indexOf(",") + 1, Str.length());
            restr_v.push_back(Numberstr);
            count++;
        } else {
            restr_v.push_back(Numberstr);
            ;
            count++;
            break;
        }
    }
    return count;
}

String getReString(uint16_t Number) {
    if (restr_v.empty()) {
        return String("");
    }
    return restr_v.at(Number);
}

uint16_t GetstrNumber(String StartStr, String EndStr, String Str,
                      uint32_t* ptrbuff) {
    uint16_t count = 0;
    String Numberstr;
    int indexpos = 0;

    Str = Str.substring(Str.indexOf(StartStr) + StartStr.length(),
                        Str.indexOf(EndStr));
    Str.trim();

    while (Str.length() > 0) {
        indexpos = Str.indexOf(",");
        if (indexpos != -1) {
            Numberstr      = Str.substring(0, Str.indexOf(","));
            Str            = Str.substring(Str.indexOf(",") + 1, Str.length());
            ptrbuff[count] = Numberstr.toInt();
            count++;
        } else {
            ptrbuff[count] = Str.toInt();
            count++;
            break;
        }
    }
    return count;
}

uint32_t numberbuff[128];
String readstr;
uint8_t restate;


//データ送信プログラム
void addRowToGoogleSheets(float val0, float val1, float val2) {

//URL
  //String url = "https://script.google.com/macros/s/" + String(deployid) + "/exec?chipid=ESP" + String(chipid) + "&val0=" + String(val0) + "&val1=" + String(val1) + "&val2=" + String(val2);
  String url =  String(send_url) + "?chipid=ESP" + String(chipid) + "&val0=" + String(val0) + "&val1=" + String(val1) + "&val2=" + String(val2);
  
  terminal.println("[HTTP] begin...");
  terminal.println(url);
  terminal.println("[HTTP] POST...");
// コンテンツタイプの設定
  AddMsg("AT+HTTPPARA=\"CONTENT\",\"application/json\"\r\n", kASSIGN_MT, 1000, 1000);
  while ((readSendState(0) == kSendReady) || (readSendState(0) == kSending) ||
          (readSendState(0) == kWaitforMsg))
      delay(50);
  restate = readSendState(0);
  readstr = ReadMsgstr(0).c_str();
  EraseFirstMsg();

  terminal.print(readstr);

// AT commmand : URLの設定 & http post（今回はURLにデータを追加している）
  AddMsg("AT+HTTPPARA=\"URL\",\"" + url + "\"\r\n", kASSIGN_MT, 1000, 1000);
  while ((readSendState(0) == kSendReady) || (readSendState(0) == kSending) ||
          (readSendState(0) == kWaitforMsg))
      delay(50);
  restate = readSendState(0);
  readstr = ReadMsgstr(0).c_str();
  EraseFirstMsg();

  terminal.print(readstr);
  
  delay(5000);
  terminal.println("[HTTP] GET...");

// HTTP GET リクエスト
  AddMsg("AT+HTTPACTION=0\r\n", kASSIGN_MT, 5000, 5000);
  while ((readSendState(0) == kSendReady) || (readSendState(0) == kSending) ||
          (readSendState(0) == kWaitforMsg))
      delay(50);
  restate = readSendState(0);
  readstr = ReadMsgstr(0).c_str();
  EraseFirstMsg();

  terminal.print(readstr);

  delay(10000);

}


void setup() {
    // put your setup code here, to run once:
    M5.begin(true, true, true, false);
    Serial2.begin(115200, SERIAL_8N1, 5, 13);

    Disbuff.createSprite(320, 20);
    Disbuff.fillRect(0, 0, 320, 20, BLACK);
    Disbuff.drawRect(0, 0, 320, 20, Disbuff.color565(36, 36, 36));
    Disbuff.pushSprite(0, 0);

    TerminalBuff.createSprite(120, 220);
    TerminalBuff.fillRect(0, 0, 120, 220, BLACK);
    TerminalBuff.drawRect(0, 0, 120, 220, Disbuff.color565(36, 36, 36));
    TerminalBuff.pushSprite(0, 20);
    terminal.setGeometry(0, 20, 120, 220);

    pinMode(2, OUTPUT);
    digitalWrite(2, 0);

    Disbuff.setTextColor(WHITE);
    Disbuff.setTextSize(1);
    for (int i = 0; i < 100; i++) {
        Disbuff.fillRect(0, 0, 320, 20, Disbuff.color565(36, 36, 36));
        Disbuff.pushSprite(0, 0);
        Disbuff.setCursor(7, 7);
        Disbuff.printf("Reset Module %02d", i);
        Disbuff.pushSprite(0, 0);
        delay(10);
    }
    digitalWrite(2, 1);

    xTaskCreate(LTEModuleTask, "LTEModuleTask", 1024 * 2, (void*)0, 4,
                &xhandle_lte_event);
    command_list_samap = xSemaphoreCreateMutex();
    xSemaphoreGive(command_list_samap);

    terminal.println("Initializing SIM7600...");

// モジュール起動確認
    AddMsg("AT\r\n", kQUERY_MT, 1000, 1000);
    while ((readSendState(0) == kSendReady) || (readSendState(0) == kSending) ||
           (readSendState(0) == kWaitforMsg))
        delay(50);
    restate = readSendState(0);
    readstr = ReadMsgstr(0).c_str();
    EraseFirstMsg();

    terminal.print(readstr);

// 機能の追加（フルモード）
    AddMsg("AT+CFUN=1\r\n", kQUERY_MT, 1000, 1000);
    while ((readSendState(0) == kSendReady) || (readSendState(0) == kSending) ||
           (readSendState(0) == kWaitforMsg))
        delay(50);
    restate = readSendState(0);
    readstr = ReadMsgstr(0).c_str();
    EraseFirstMsg();

    terminal.print(readstr);

// LTE接続
    AddMsg("AT+CGATT=1\r\n", kQUERY_MT, 1000, 1000);
    while ((readSendState(0) == kSendReady) || (readSendState(0) == kSending) ||
           (readSendState(0) == kWaitforMsg))
        delay(50);
    restate = readSendState(0);
    readstr = ReadMsgstr(0).c_str();
    EraseFirstMsg();

    terminal.print(readstr);

//　通信強度
    AddMsg("AT+CSQ\r\n", kQUERY_MT, 1000, 1000);
    while ((readSendState(0) == kSendReady) || (readSendState(0) == kSending) ||
           (readSendState(0) == kWaitforMsg))
        delay(50);
    restate = readSendState(0);
    readstr = ReadMsgstr(0).c_str();
    EraseFirstMsg();

    terminal.print(readstr);

//　インターネット登録状況
    AddMsg("AT+CREG?\r\n", kQUERY_MT, 1000, 1000);
    while ((readSendState(0) == kSendReady) || (readSendState(0) == kSending) ||
           (readSendState(0) == kWaitforMsg))
        delay(50);
    restate = readSendState(0);
    readstr = ReadMsgstr(0).c_str();
    EraseFirstMsg();

    terminal.print(readstr);

// APN設定
    AddMsg("AT+CGDCONT = 1, \"IP\", \"" + sim_apn + "\"\r\n", kASSIGN_MT, 1000, 1000);
    while ((readSendState(0) == kSendReady) || (readSendState(0) == kSending) ||
           (readSendState(0) == kWaitforMsg))
        delay(50);
    restate = readSendState(0);
    readstr = ReadMsgstr(0).c_str();
    EraseFirstMsg();

    terminal.print(readstr);

/*
// 認証情報の設定
    AddMsg("AT+CGAUTH = 1, \"" + sim_user + "\", \"" + sim_pass + "\"\r\n", kASSIGN_MT, 1000, 1000);
    while ((readSendState(0) == kSendReady) || (readSendState(0) == kSending) ||
           (readSendState(0) == kWaitforMsg))
        delay(50);
    restate = readSendState(0);
    readstr = ReadMsgstr(0).c_str();
    EraseFirstMsg();

    terminal.print(readstr);
*/

// PDPコンテキスト有効化
    AddMsg("AT+CGACT=1,1\r\n", kQUERY_MT, 1000, 1000);
    while ((readSendState(0) == kSendReady) || (readSendState(0) == kSending) ||
           (readSendState(0) == kWaitforMsg))
        delay(50);
    restate = readSendState(0);
    readstr = ReadMsgstr(0).c_str();
    EraseFirstMsg();

    terminal.print(readstr);

// HTTP機能の初期化
    AddMsg("AT+HTTPINIT\r\n", kQUERY_MT, 1000, 1000);
    while ((readSendState(0) == kSendReady) || (readSendState(0) == kSending) ||
           (readSendState(0) == kWaitforMsg))
        delay(50);
    restate = readSendState(0);
    readstr = ReadMsgstr(0).c_str();
    EraseFirstMsg();

    terminal.print(readstr);

// PDPコンテキストを設定
    AddMsg("AT+HTTPPARA=\"CID\",1\r\n", kQUERY_MT, 1000, 1000);
    while ((readSendState(0) == kSendReady) || (readSendState(0) == kSending) ||
           (readSendState(0) == kWaitforMsg))
        delay(50);
    restate = readSendState(0);
    readstr = ReadMsgstr(0).c_str();
    EraseFirstMsg();

    terminal.print(readstr);

}

void loop() {
    M5.update();

//Aボタンを押したら送信されます
    if (M5.BtnA.wasPressed()) {
      float val0 = random(0, 1000) / 10.0;
      float val1 = random(0, 1000) / 10.0;
      float val2 = random(0, 1000) / 10.0;

      terminal.print("ESP");
      terminal.println(chipid);
      terminal.printf("[val0] %.1f\n[val1] %.1f\n[val2] %.1f\n", val0, val1, val2);
      addRowToGoogleSheets(val0, val1, val2);
      delay(interval * 1000);
    }

//Cボタンを押したら通信強度とインターネット登録状況が表示
    if (M5.BtnC.wasPressed()) {

      while(1){
        AddMsg("AT+CSQ\r\n", kQUERY_MT, 1000, 1000);
        while ((readSendState(0) == kSendReady) || (readSendState(0) == kSending) ||
              (readSendState(0) == kWaitforMsg))
            delay(50);
        restate = readSendState(0);
        readstr = ReadMsgstr(0).c_str();
        EraseFirstMsg();

        terminal.print(readstr);

        AddMsg("AT+CREG?\r\n", kQUERY_MT, 1000, 1000);
        while ((readSendState(0) == kSendReady) || (readSendState(0) == kSending) ||
              (readSendState(0) == kWaitforMsg))
            delay(50);
        restate = readSendState(0);
        readstr = ReadMsgstr(0).c_str();
        EraseFirstMsg();

        terminal.print(readstr);

        delay(500);
        M5.update();

        //Aボタンを押したら電話が掛けられる
　　　　　　//※番号を入力したら電話が掛けられるらしいが、試してもなにも起こらなかった. SIM cardを変えたらできるかもしれない.
        if (M5.BtnA.wasPressed()) {
            AddMsg("ATD13800088888;\r\n", kQUERY_MT, 1000, 1000);
            while ((readSendState(0) == kSendReady) ||
                  (readSendState(0) == kSending) ||
                  (readSendState(0) == kWaitforMsg))
                delay(50);
            Serial.printf("Read state = %d \n", readSendState(0));
            readstr = ReadMsgstr(0).c_str();
            Serial.print(readstr);
            while (1) {
                M5.update();
                if (M5.BtnA.wasPressed()) break;
                delay(100);
            }
            EraseFirstMsg();
            AddMsg("AT+CHUP\r\n", kASSIGN_MO, 1000, 1000);
        }

        //Bボタンを押したら再起動
        if (M5.BtnB.wasPressed()) {
        delay(1000);
        esp_sleep_enable_timer_wakeup(10000);
        esp_sleep_enable_ext0_wakeup((gpio_num_t)M5.BtnA.wasPressed(), 0);
        esp_deep_sleep_start();
        }
      }
    }

    if (M5.BtnB.wasPressed()) {
        delay(1000);
        esp_sleep_enable_timer_wakeup(10000);
        esp_sleep_enable_ext0_wakeup((gpio_num_t)M5.BtnA.wasPressed(), 0);
        esp_deep_sleep_start();
        }
}


/*本来のhttp post方法（仮）-----------------------------------------------------

// URL を設定
AddMsg("AT+HTTPPARA=\"URL\",\"" + url + "\"\r\n", kASSIGN_MT, 1000, 1000);

// コンテンツのタイプを設定（JSON 例）
AddMsg("AT+HTTPPARA=\"CONTENT\",\"application/json\"\r\n", kASSIGN_MT, 1000, 1000);

// 送信データの準備（例：50バイトのデータを10秒以内に送信）
AddMsg("AT+HTTPDATA=50,10000\r\n", kASSIGN_MT, 1000, 1000);
delay(100);  // 少し待機

// 送信する JSON データを設定（例）
AddMsg("{\"temperature\":25.5,\"humidity\":60}\r\n", kASSIGN_MT, 1000, 1000);
delay(100);

// HTTP POST を開始
AddMsg("AT+HTTPACTION=1\r\n", kASSIGN_MT, 1000, 1000);
delay(5000);  // 応答待機

// レスポンスを取得（HTTP GET）
AddMsg("AT+HTTPREAD\r\n", kASSIGN_MT, 1000, 1000);

--------------------------------------------------------------------------------*/