/**
 * 全局变量定义文件
 * 所有的宏定义及其他全局变量，统一在当前文件中定义管理
 */

#if !defined(CONFIG_H)
#define CONFIG_H
// ========== 核心：解决符号重复定义的编译宏（必须最顶部） ==========
// ========== 终极配置：释放最大内存 + 禁用冲突功能 ==========
// 启用ESP32内置NimBLE（核心宏，解决头文件冲突）
//#define ARDUINO_ESP32_BLE_ENABLE_NIMBLE 1
//#define ARDUINO_ESP32_BLE_DISABLE_BLUEDROID 1
#define CONFIG_NIMBLE_ENABLED 1
// 官方BLE头文件（底层自动调用内置NimBLE，无需NimBLEDevice.h）
#include <DebugUtils.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLECharacteristic.h>
#include <BLE2902.h>
//#include <NimBLEDevice.h>
#include "Arduino.h"
#include <math.h>
#include <ArduinoJson.h>
#include <MQTT.h>
#include "DateTime.h"
#include <WiFiUdp.h>
#include <WiFi.h>  // 对应 ESP8266WiFi库
// #include <WString.h>

//**************************** 结构体 定义开始*******************************
// #define MAGIC_NUMBER 0xAA
// #define DEFAULT_STASSID "lovelamp"
// #define DEFAULT_STAPSW  "lovelamp"

// #define leapYear(y)  ((y%4==0)&&(y%100!=0)||(y%400==0))    //宏定义,判断是否是闰年
// #define Use_Serial Serial // 打印开启
// //日期结构体
// typedef struct
// {
// 	int years=2023;
// 	int months=8;
// 	int days=1;
// }date;
// date today;
//date lastWaterDate;
// 用于存上次的WIFI和密码
struct config_type
{
  char stassid[50]="";
  char stapsw[50]="";
  uint8_t magic;
};
config_type config_wifi;
//时间段结构
// //哈希值

// 全局变量
typedef struct
{
  // char mqttServer[50]="charge.jy400.cn";
  // uint16_t port = 58613;
  // char userName[20]="HscIot";
  // char Psw[20]="Ahau&1234";
   char mqttServer[50]="121.40.47.37";
  uint16_t port = 1883;
  char userName[20]="user-client";
  char Psw[20]="123456";
  char subTopic[50]="FangCangS2C/";
  char pubTopic[50]="FangCangC2S/";
  uint8_t subQos=0;
  uint8_t pubQos=0;
  uint8_t execCycle = 30; // 循环周期（默认30分钟）
  String otaUrl = "http://121.40.47.37:9090/HscIot/file/download/FCGate.ino.bin";
} global;
//植物信息变量

//节点个体变量
typedef struct
{
   String nodeId = "";
   String nodeName = "FangCang";
    // uint16_t sensorUp=0;//20240522 传感器最高最低值
    // uint16_t sensorDown=0;
   String version = "1.0";
   String hashCode="";
  // String groupId="";
  //uint8_t plantCount=0;
 //  LinkedList<plant *> plantList=LinkedList<plant *>();
}node;
//**************************** 单个json的最大长度，修改最大不能超过512
#define MAX_JSON_LEN 512 //
typedef struct  {
    uint8_t src; // 2 = BLE
    char jsonStr[MAX_JSON_LEN];
}Command;

#define CMD_POOL_SIZE 5 // 根据实际并发需求设定池大小

// 1. 预分配的静态内存池（避免碎片）
Command cmd_pool[CMD_POOL_SIZE]; 

// 2. 两个队列句柄
QueueHandle_t xFreeCmdQueue;   // 空闲队列：存放可用 Command 的指针
QueueHandle_t g_commandQueue;  // 工作队列：存放已填充数据的 Command 指针
SemaphoreHandle_t  g_stateMutex;//线程中与loop 可能读取485冲突
//**************************** 结构体 定义结束*******************************

//**************************** 初始设置参数 定义开始*******************************


#define wifi_Led 3 //  黄色的灯 wifi
#define mqtt_Led 6 //  绿色的灯 mqtt
#define ble_Led 2 //  蓝色的灯 BLE
#define mySerial Serial1
#define TXD_PIN 5
#define RXD_PIN 4
// #define UART_NUM (UART_NUM_1) // 表示第几个串口 UART0，UART1，UART2
#define FOUR_G_SERIAL Serial0
#define FOUR_G_BAUD 9600
#define TXD_PIN_4G 21
#define RXD_PIN_4G 20
#define Success 1U
#define Failure 0U
bool Mqtt_Connect_Status=false;
bool PIN_Led_State = 0;
 unsigned long lastLostMqtt=0;
// 系统固件信息
  unsigned char Sersor_addr[5][8]=
  {
    {0x01, 0x03, 0x02, 0x5C, 0x00, 0x2E, 0x04, 0x7C}, //读取索引9 设置值
    {0x01, 0x03, 0x00, 0x00, 0x00, 0x2E, 0xC5, 0xD6},//读取索引5 状态值 
    {0x01, 0x03, 0x03, 0x85, 0x00, 0x01, 0x95, 0xA7},//Frequency_addr 频率 索引7
    {0x01, 0x02, 0x00, 0x40, 0x00, 0x45, 0xB8, 0x2D},//错误状态值 在索引3和4
    {0x02, 0x03, 0x00, 0x00, 0x00, 0x03, 0x05, 0xF8}//外接三合一传感器
  };
// unsigned char FaultState_addr[8]={0x01, 0x02, 0x00, 0x40, 0x00, 0x45, 0xB8, 0x2D};
// unsigned char Frequency_addr[8]={0x01, 0x03, 0x03, 0x85, 0x00, 0x01, 0x95, 0xA7};
// unsigned char CO2_addr[8]={0x02, 0x03, 0x00, 0x00, 0x00, 0x03, 0x05, 0xF8};
//DB db;                                            // 操作存储的数据库帮助类

//**************************** 初始设置参数 定义结束*******************************

node MyNode;
global MyGlobal;
WiFiClient mqttclient; // 定义wifiClient实例
WiFiUDP ntpUDP;
// static QueueHandle_t g_commandQueue; // 全局队列
// static SemaphoreHandle_t  g_stateMutex;
static uint8_t net_Lock=0;
//-------------------------------------------------------------------------------全局变量
static BLECharacteristic* g_pSensorChar = nullptr;
static BLECharacteristic* g_pTxChar = nullptr;
// 订阅状态（由回调更新）
volatile bool g_sensorSubscribed = false;
volatile bool g_txSubscribed = false;

// 通知触发标志
volatile bool g_sensorNotifyPending = false;
String g_sensorDataStr;

static uint16_t g_peerMtu = MAX_JSON_LEN; //蓝牙大小。
//**************************** 蓝牙Ble功能 定义结束*******************************

//**************************** MQTT功能 定义开始*******************************
//------mqtt头定义-----------------
//String MQTT_SERVER = "114.213.145.88"; // mqtt服务器
// String MQTT_SERVER = "charge.jy400.cn"; // mqtt服务器
// int MQTT_SERVERPORT = 1883;         // 端口

//String HTTP_SERVER = "http://116.62.148.209"; // http服务器
//int HTTP_SERVERPORT = 8080;
//  #define MQTT_USERNAME    "HscIot"
//  #define MQTT_KEY         "Ahau&1234"

//#define MQTT_KEY ""
//static bool g_mqttConnected=false;
#define MQTT_KEEPALIVE 60
#define MQTT_SOCKET_TIMEOUT 120

#define MQTT_MAX_PACKET_SIZE 1536 // buffer bytes,决定了发送数据包的大小，当一次性发送的数据长度大于这个，大概率无法送达
MQTTClient client(MQTT_MAX_PACKET_SIZE); // 定义mqtt的实例

// 数据上传时间间隔配置（单位：毫秒）
#define SENSOR_UPLOAD_INTERVAL 300000    // 传感器数据上传间隔 5分钟
#define DEVICE_UPLOAD_INTERVAL 300000    // 设备数据上传间隔 5分钟
//#define HEAP_PRINT_INTERVAL 300000       // 堆内存打印间隔 5分钟
// static bool mqttConnect=false;
// static uint8_t mqttCount=0;
#endif // CONFIG_H


