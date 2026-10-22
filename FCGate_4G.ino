//#include <ArduinoJson.h>
//#include <BLEDevice.h>
#include "config.h"

SemaphoreHandle_t g_fourGMutex = NULL;
static unsigned long g_lastMqttPublishMs = 0;
volatile bool g_commandProcessing = false;


//-----------------------------A-3 函数声明开始-----------------//
//bool wifi_Connect(bool flag);
void handleData(String jsonStr,uint8_t src);
void BleInit();
void BleConnect();
//void GetCO2Data();
//void saveWifiConfig();
void loadConfig();
String Read_String(const char* key);
//void Mqtt_init();
//void Mqtt_Domain();
//bool MQTT_reconnect(bool);
void Publish(String Topic, String Content, short int Qos);
void PublishJson();
void updateBin();
bool publishDataToServer(String);
bool isMqttConRight();
bool isMqttConnect();
bool initMQTT(String nodeId,String mqttTopic,String subTopic);
bool disableMqttAutoPackets();
bool enterFourGAtMode();
bool takeFourGSerial(uint32_t timeoutMs);
void releaseFourGSerial();
String ReadBuffer();
String GetSensorData(uint8_t code,String hashCode);
// void sensorUploadTask(void *parameter);
// void uart_init();
// void GetDataFromMqtt();
// void HandleTransaction();
//bool GetTemp(float *temp);
// void printStr(String str);
//void printInt(int str);
//---------------------------A-3 函数声明结束------------------------//

//-----------------------------A-3 函数-----------------//
void initCommandPool() {
    // 创建能容纳 5 个指针的队列
    xFreeCmdQueue = xQueueCreate(CMD_POOL_SIZE, sizeof(Command*));
    g_commandQueue = xQueueCreate(CMD_POOL_SIZE, sizeof(Command*));

    // 将所有内存块的地址放入空闲队列
    for (int i = 0; i < CMD_POOL_SIZE; i++) {
        Command* pCmd = &cmd_pool[i];
        xQueueSend(xFreeCmdQueue, &pCmd, portMAX_DELAY);
    }
}
//-----------------------------------------------------------------------------------
bool FourG_Init() {
  FOUR_G_SERIAL.begin(FOUR_G_BAUD, SERIAL_8N1, RXD_PIN_4G, TXD_PIN_4G);
  FOUR_G_SERIAL.setTimeout(20);
  Mqtt_Connect_Status = false;
  enterFourGAtMode();
  const uint8_t maxAttempts = 3;
  bool flag = false;

  for (uint8_t attempt = 1; attempt <= maxAttempts && !flag; attempt++) {
    flag = isMqttConRight();
    if (!flag) {
      debugPrint(F("[4G] MQTT configuration attempt "));
      debugPrintln(attempt);
      if (!initMQTT(MyNode.nodeId, MyGlobal.pubTopic, MyGlobal.subTopic)) {
        flag = false;
      } else {
        delay(1000);
        flag = isMqttConRight();
      }
    }

    if (flag && disableMqttAutoPackets() && isMqttConnect()) {
      Mqtt_Connect_Status = true;
      lastLostMqtt = millis();
      digitalWrite(mqtt_Led, HIGH);
      //mqttConnectedEvent(true);
      //subscribeTopic(subTopic);
      break;
    }

    flag = false;
    Mqtt_Connect_Status = false;
    if (attempt < maxAttempts) {
      debugPrintln(F("[4G] MQTT is not connected, retrying"));
      delay(3000);
    }
  }

  if (!flag) {
    digitalWrite(mqtt_Led, LOW);
  }
  //FourGMqttTopic = MyGlobal.pubTopic;
  //FourGSubTopic = MyGlobal.subTopic;

  debugPrintln(F("4G Module Initialized"));
  debugPrint(F("Node ID: "));
  debugPrintln(MyNode.nodeId);
  debugPrint(F("4G pins TX="));
  debugPrint(TXD_PIN_4G);
  debugPrint(F(", RX="));
  debugPrintln(RXD_PIN_4G);
  return flag;
}
// // 0-1 生成唯一设备
String generateUniqueDevice()
{
    uint64_t chipId = ESP.getEfuseMac();  // 读取 eFuse 中的 Base MAC

    // 提取低 48 位（即真实 MAC）
    uint64_t mac48 = chipId & 0xFFFFFFFFFFFFULL;

    // 格式化为 12 位大写 HEX 字符串
    char idStr[13];
    snprintf(idStr, sizeof(idStr),
             "%012llX", mac48);  // llX 表示 unsigned long long

    return String(idStr);
}
// 0-2时间模块初始化

// 0-3引脚初始化
void Pin_Int()
{
  // analogWriteFreq(1000);            // 频率设置为1kHz，即周期为1ms
  // analogWriteRange(1024);           // 范围设置为1000，即占空比步长为1us
  analogReadResolution(10);
  pinMode(wifi_Led, OUTPUT);
  pinMode(mqtt_Led, OUTPUT);
  //pinMode(Ctr, OUTPUT);
  pinMode(ble_Led, OUTPUT);

  // digitalWrite(D3 , HIGH);
  // digitalWrite(D4 , HIGH);
//  pinMode(Ctr, OUTPUT);
//  digitalWrite(Ctr,HIGH);
//  pinMode(D6, OUTPUT);
}
//---------------------------------------------freeRTOS多任务

/// @brief
/// @param parameter
void mqttTask(void *parameter)
{
  
   while (true)
   {
       if (net_Lock == 0) {
           if (takeFourGSerial(0)) {
               ReadBuffer();
               releaseFourGSerial();
           }
       } else if (net_Lock == 1) {
           net_Lock = 2;
       }
       vTaskDelay(10 / portTICK_PERIOD_MS); // 休眠 10ms
   }
   debugPrintln(F("Ending taskMqtt"));

}
//--------------------------------
void coordinatorTask(void* param)
{

  debugPrintln(F("Begin taskCoordinator"));
 while (true)
   {
    Command* pReceivedCmd;
 // 1. 阻塞等待接收数据指针
        if (xQueueReceive(g_commandQueue, &pReceivedCmd, pdMS_TO_TICKS(5000)) == pdTRUE) {
            
            // 2. 处理业务逻辑
            g_commandProcessing = true;
            handleData(pReceivedCmd->jsonStr, pReceivedCmd->src);
            g_commandProcessing = false;
            
            // 3. ⚠️ 核心操作：用完归还给空闲池（而不是 vPortFree）
            xQueueSend(xFreeCmdQueue, &pReceivedCmd, portMAX_DELAY);
        }
   }
   debugPrintln(F("Ending taskCoordinator"));
}

//------------------------------------------- void setup() ------------------------------------------
void setup()
{
  debugBegin(115200);
  mySerial.begin(9600 , SERIAL_8N1 , RXD_PIN , TXD_PIN);
  lastLostMqtt=millis();
  MyNode.version=Read_String("version");
  if(MyNode.version=="") MyNode.version="0.0";
//   analogWriteFreq(1000);            // 频率设置为1kHz，即周期为1ms
//   analogWriteRange(1000);           // 范围设置为1000，即占空比步长为1us
  MyNode.nodeId = generateUniqueDevice(); // 根据板子本身的mac地址生成设备名称
  MyNode.nodeName=Read_String("nodeName");
  if(MyNode.nodeName=="") MyNode.nodeName="FangCang";
  Pin_Int();     // 初始化引脚
  loadConfig();  // 读取信息 WIFI
  g_stateMutex = xSemaphoreCreateMutex();
  g_fourGMutex = xSemaphoreCreateMutex();
  initCommandPool();
  FourG_Init();
  //SpiffMount();  // 挂载文件系统
  delay(200);
  // wifi_Connect(); // 联网
  BleInit();
  //   // 创建队列
  //   g_commandQueue = xQueueCreate(5, sizeof(Command));
  //   if (!g_commandQueue) {
  //       debugPrintln("队列创建失败");
  //       while (1) vTaskDelay(1);
  //   }
  // bool result = wifi_Connect(false);
  //  Mqtt_init();
  //  delay(1000);



   debugPrint(F("nodeId:"));
   debugPrint(MyNode.nodeId.c_str());
   debugPrint(F(" nodeName:"));
   debugPrintln(MyNode.nodeName);
   xTaskCreate(mqttTask, "Mqtt", 4096, NULL, 1, NULL);//6144
   xTaskCreate(coordinatorTask, "Coordinator", 6144, NULL, 2, NULL); // 高优先级
   debugPrint(F("version:"));
     debugPrintln(F(MyNode.version.c_str()));
 }

bool publishSensorData(uint8_t code)
{
    String sensorData = GetSensorData(code, String(millis()));
    if (!publishDataToServer(sensorData)) {
        debugPrint(F("Sensor publish failed code="));
        debugPrintln(code);
        return false;
    }
    return true;
}
//遇到服务器指令立即让路
bool serverCommandPending()
{
    if (g_commandProcessing) {
        return true;
    }
    if (g_commandQueue != NULL && uxQueueMessagesWaiting(g_commandQueue) > 0) {
        return true;
    }
    if (FOUR_G_SERIAL.available() > 0 && takeFourGSerial(0)) {
        ReadBuffer();
        releaseFourGSerial();
        return g_commandQueue != NULL && uxQueueMessagesWaiting(g_commandQueue) > 0;
    }
    return false;
}

//------------------------------------------- void void loop()  ------------------------------------------

void loop()
{
    static unsigned long lastSensorUpload = 0;
    static unsigned long lastDeviceUpload = 0;
     unsigned long currentMillis = millis();
    if (g_sensorNotifyPending) {
        g_sensorNotifyPending = false;
        if (g_pSensorChar) {
            if (g_sensorDataStr.length() < g_peerMtu) {
                debugPrintln(F(g_sensorDataStr.c_str()));
                g_pSensorChar->setValue(g_sensorDataStr.c_str());
                g_pSensorChar->notify();
              }
            else
             debugPrintln(F("发送数据超过BLE的MTU长度!"));
            }
    }
     if (Mqtt_Connect_Status && net_Lock == 0) 
     {
         if (currentMillis - lastSensorUpload >= SENSOR_UPLOAD_INTERVAL && !serverCommandPending()) 
             {
            const uint8_t sensorCodes[] = {1, 4, 6};
            String sensorData[3];
            bool abortSensorUpload = false;
            for (uint8_t i = 0; i < 3; i++) {
                if (serverCommandPending()) {
                    abortSensorUpload = true;
                    break;
                }
                sensorData[i] = GetSensorData(sensorCodes[i], String(millis()));
                if (serverCommandPending()) {
                    abortSensorUpload = true;
                    break;
                }
            }
            for (uint8_t i = 0; i < 3; i++) {
                  if (abortSensorUpload || serverCommandPending()) {
                      abortSensorUpload = true;
                      break;
                  }
                  if (sensorData[i].length() > 0) {
                      publishDataToServer(sensorData[i]);
                      vTaskDelay(pdMS_TO_TICKS(100));
                 }
             }
             if (!abortSensorUpload) {
                 lastSensorUpload = currentMillis;
             }

         }
         
    } 
     if (net_Lock == 0 && !Mqtt_Connect_Status&&millis()-lastLostMqtt>120000)  //mqtt状态 当掉线超过2分钟，系统重启。
        ESP.restart(); 

    delay(10);
}
