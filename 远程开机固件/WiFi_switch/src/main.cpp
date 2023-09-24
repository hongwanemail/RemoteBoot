/*
作者：创客宏万
B站主页：https://space.bilibili.com/602398876
时间：2023年7月17日
版本：0.2 改进联网方式

*/

#include <Arduino.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <Ticker.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>

#define WIFI_DEBUG 2            // 1：smartConfig配网模式;2:softap配网;
#define RELAYPIN 3              // 设置继电器引脚为Pin
#define LedIN 2                 // 设置led指示灯引脚为Pin
#define ONENET_DISCONNECTED 1   // 已经断开
#define ONENET_CONNECTED 2      // 已经连接上
#define ONENET_RECONNECT 3      // 重连成功
#define VER "ESP8266_MQTT_V0.2" // 版本号

// 设置热点的名字
#define AP_SSID "esp_device_AP"
// 设置热点的密码
#define AP_PSW "hongwans"

// 设置wifi的ip地址(wifi的ip地址好像不能和网关地址在同一个网段)
IPAddress local_IP(192, 168, 1, 1);
// 设置网关
IPAddress gateway(192, 168, 0, 1);
// 设置子网掩码
IPAddress subnet(255, 255, 255, 0);
// wifiServer服务
WiFiServer WiFi_Server(8899);
// 临时wifi连接
WiFiClient WiFi_Client;

String ssid = "";     // wifi账号
String password = ""; // wifi密码

/*OneNet*/
PubSubClient mqttClient;
const char *mqttServer = "183.230.40.39"; // mqtt服务器
const uint16_t mqttPort = 6002;           // 端口号
// 修改下面三个即可
#define onenet_productId "612963"    // 产品ID
#define onenet_deviceId "1108681844" // 设备ID
#define onenet_apiKey "sulaos"     // 鉴权信息
//{"device_id":"xxx","api_key":"xxx"}   //生成设备二维码内容

int bright = 0;
int state;
Ticker delayTimer;
WiFiClient espClient;

/* 延时N秒 */
void delayNs(uint8_t m)
{
  for (uint8_t index = 0; index < m; index++)
  {
    delay(1000);
    ESP.wdtFeed(); // 喂狗
  }
}

/* 延时重启 */
void delayRestart(float t)
{
  Serial.print("Restart after ");
  Serial.print(t);
  Serial.println("s");
  delayTimer.attach(t, []()
                    {
    Serial.println("\r\nRestart now!");
    ESP.restart(); });
}

/* 自动连接 */
bool autoConfig()
{
  WiFi.begin();
  // 正常情况下，10秒即可链接
  for (int i = 0; i < 10; i++)
  {
    if (WiFi.status() == WL_CONNECTED)
    {
      Serial.println("AutoConfig Success");
      Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
      Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
      WiFi.printDiag(Serial);
      return true;
    }
    else
    {
      digitalWrite(LedIN, !digitalRead(LedIN));
      Serial.print("AutoConfig Waiting......");
      Serial.println(WiFi.status());
      delay(1000); // 这个地方一定要加延时，否则极易崩溃重启
    }
  }
  Serial.println("AutoConfig Faild!");
  return false;
}
int smartCount = 0;
/* 一键配网 */
void smartConfig()
{
  WiFi.mode(WIFI_STA);
  Serial.println("\r\nWait for Smartconfig");
  WiFi.beginSmartConfig();
  while (1)
  {
    Serial.print(".");
    digitalWrite(LedIN, !digitalRead(LedIN));
    if (WiFi.smartConfigDone())
    {
      Serial.println("SmartConfig Success");
      Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
      Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
      WiFi.setAutoConnect(true); // 设置自动连接
      break;
    }
    smartCount++;
    if (smartCount >= 60)
    {
      // 防止一直处于联网中
      delayRestart(1);
      break;
    }
    delay(1000); // 这个地方一定要加延时，否则极易崩溃重启
  }
}

/* 连接OneNet */
int connectToOneNetMqtt()
{
  int cnt = 0;
  while (!mqttClient.connected())
  {
    ESP.wdtFeed();
    cnt++;
    Serial.println("Connect to OneNet MQTT...");

    if (mqttClient.connect(onenet_deviceId, onenet_productId, onenet_apiKey))
    {
      Serial.println("connect success!");
      return ONENET_RECONNECT;
    }
    else
    {
      Serial.print("connect fail!");
      Serial.println(" try again in 5 seconds");
      delayNs(5);
    }
    if (cnt >= 10)
    {
      // 只做10次连接到OneNet，连接不上重启8266
      cnt = 0;
      delayRestart(1);
    }
  }
  return ONENET_CONNECTED;
}
String rx_data = "";
/* 云端下发 */
void callback(char *topic, byte *payload, unsigned int length)
{
  Serial.print("Message:");
  // Serial.print("Message arrived [");
  // Serial.print(topic);
  // Serial.print("] ");

  for (int i = 0; i < length; i++)
  {
    rx_data += char(payload[i]);
  }

  // 如果开头不是  {  清除
  if (!rx_data.startsWith("{"))
  {
    rx_data = "";
  }

  if (rx_data.endsWith("}"))
  {
    Serial.println(rx_data);
    // 开始解析json
    DynamicJsonDocument doc(2048);
    deserializeJson(doc, rx_data);
    JsonObject root = doc.as<JsonObject>();

    // 远程开关
    if (root.containsKey("on"))
    {
      const String on = root["on"];
      if (on == "true")
      {
        digitalWrite(RELAYPIN, LOW);
        delay(500);
        digitalWrite(RELAYPIN, HIGH);
      }
      else
      {
        digitalWrite(RELAYPIN, LOW);
        delay(10000);
        digitalWrite(RELAYPIN, HIGH);
      }
    }

    doc.clear();
    rx_data = "";
  }
}

/* 发布信息 */
void Temp_pubMQTTmsg(uint32_t data)
{
  char msg[50];
  char tmp[28];
  char d[3];
  snprintf(tmp, sizeof(tmp), "{\"Temp\":%d}", data);
  uint16_t streamLen = strlen(tmp);

  d[0] = '\x03';
  d[1] = (streamLen >> 8);
  d[2] = (streamLen & 0xFF);
  snprintf(msg, sizeof(msg), "%c%c%c%s", d[0], d[1], d[2], tmp);
  mqttClient.publish("$dp", (uint8_t *)msg, streamLen + 3, false);
}

bool m_softap = false;
/* softap配网 */
void softap()
{
  // 设置wifi模块为AP模式
  WiFi.mode(WIFI_AP);
  // 设置IP地址网关和子网掩码
  WiFi.softAPConfig(local_IP, gateway, subnet);
  // 设置wifi的名字和密码
  WiFi.softAP(AP_SSID, AP_PSW);
  // 打印wifi模块的ip地址
  Serial.println("IP address = ");
  Serial.print(WiFi.softAPIP());
  // 启动wifiserver服务
  WiFi_Server.begin();
  // 设置连接发送数据不延时
  WiFi_Server.setNoDelay(true);
  Serial.println("Server online.");
  m_softap = true;
}

// a写入字符串长度，b是起始位，str为要保存的字符串
void set_String(int a, int b, String str)
{
  EEPROM.write(a, str.length()); // EEPROM第a位，写入str字符串的长度
  // 把str所有数据逐个保存在EEPROM
  for (int i = 0; i < str.length(); i++)
  {
    EEPROM.write(b + i, str[i]);
  }
  EEPROM.commit();
}

// a位是字符串长度，b是起始位
String get_String(int a, int b)
{
  String data = "";
  // 从EEPROM中逐个取出每一位的值，并链接
  for (int i = 0; i < a; i++)
  {
    data += char(EEPROM.read(b + i));
  }
  return data;
}

/* 初始化系统 */
void initSystem()
{
  int cnt = 0;
  Serial.begin(115200);
  Serial.println("\r\n\r\nStart ESP8266 MQTT");
  Serial.print("Firmware Version:");
  Serial.println(VER);
  Serial.print("SDK Version:");
  Serial.println(ESP.getSdkVersion());
  ESP.wdtEnable(5000); // 初始化看门狗

  Serial.print("ESP Board MAC Address:  ");
  Serial.println(WiFi.macAddress()); // 串口输出当前ESP32/ESP8266的MAC地址

  // 初始化继电器pin
  pinMode(RELAYPIN, OUTPUT);
  digitalWrite(RELAYPIN, HIGH);
  // led输出
  pinMode(LedIN, OUTPUT);

  // 初始化 EEPROM 存储
  EEPROM.begin(1024);

  if (WIFI_DEBUG == 1) // 开启 smartConfig 一键配网模式
  {
    if (!autoConfig())
    {
      Serial.println("Start smartConfig");
      smartConfig();
    }
  }
  else
  {
    ssid = get_String(EEPROM.read(0), 1);
    password = get_String(EEPROM.read(100), 101);
    if (ssid == "" || password == "")
    {
      // 不存在 开启配网
      softap(); // 开启 softap 一键配网模式
      return;
    }

    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED)
    {
      digitalWrite(LedIN, !digitalRead(LedIN));
      delay(1000);
      cnt++;
      Serial.print(".");
      if (cnt >= 10) // 10秒够了
      {
        cnt = 0;
        softap(); // 开启 softap 一键配网模式
        break;
      }
    }
  }
  Serial.print("WIFI Connect \r\n");
  IPAddress address;
  WiFi.hostByName("ESP-RELAY", address);
}

/* 初始化ONENET通信 */
void initOneNetMqtt()
{
  mqttClient.setServer(mqttServer, mqttPort);
  mqttClient.setClient(espClient);
  mqttClient.setCallback(callback);
  Serial.println("initOneNetMqtt ok");
}

/* 初始化 */
void setup()
{
  initSystem();
  initOneNetMqtt();
  // 初始化完成 亮灯
  digitalWrite(LedIN, LOW);
}

unsigned int myTime = 0;
unsigned int ledTime = 0;
/* 主函数 */
void loop()
{
  ESP.wdtFeed(); // 喂狗
  if (m_softap)
  {
    // 等待3分钟联网
    if ((millis() - myTime) >= 1000 * 60 * 3)
    {
      myTime = millis();
      // 重启系统
      delayRestart(1);
    }

    if ((millis() - ledTime) >= 1000)
    {
      ledTime = millis();
      digitalWrite(LedIN, !digitalRead(LedIN));
    }

    // 检测服务器端是否有活动的客户端连接
    if (WiFi_Server.hasClient())
    {
      // 判断是否有新的连接
      WiFi_Client = WiFi_Server.available();
      Serial.println("new connect");
    }
    if (WiFi_Client && WiFi_Client.connected())
    {
      // 从Telnet客户端获取数据，并推送到URAT端口
      while (WiFi_Client.available())
      {
        rx_data += char(WiFi_Client.read());
      }

      // 如果开头不是  {  清除
      if (!rx_data.startsWith("{"))
      {
        rx_data = "";
      }

      if (rx_data.endsWith("}"))
      {
        // 开始解析json
        DynamicJsonDocument doc(2048);
        deserializeJson(doc, rx_data);
        JsonObject root = doc.as<JsonObject>();
        int Restart = 0;
        // ssid
        if (root.containsKey("ssid"))
        {
          const String r_ssid = root["ssid"];
          set_String(0, 1, r_ssid);
          Restart++;
        }
        // pass
        if (root.containsKey("pass"))
        {
          const String r_pass = root["pass"];
          set_String(100, 101, r_pass);
          Restart++;
        }
        doc.clear();
        Serial.println(rx_data);
        rx_data = "";

        if (Restart == 2)
        {
          // 重启系统
          delayRestart(1);
        }
      }
    }
    return;
  }

  state = connectToOneNetMqtt();
  // Serial.println(WiFi.status());
  if (state == ONENET_RECONNECT) // 重连
  {
    mqttClient.loop();
  }
  else if (state == ONENET_CONNECTED) // 连接成功
  {
    // 发布消息测试 后期可以改造成 发送电脑开机状态
    //  Temp_pubMQTTmsg(random(10, 60));
    mqttClient.loop();
  }
}
