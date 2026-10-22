#include <Arduino.h>
#line 1 "C:\\Users\\25221\\Desktop\\FC\\4G\\FCGate_4G\\FCGate_4G.ino"
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
#line 45 "C:\\Users\\25221\\Desktop\\FC\\4G\\FCGate_4G\\FCGate_4G.ino"
void initCommandPool();
#line 57 "C:\\Users\\25221\\Desktop\\FC\\4G\\FCGate_4G\\FCGate_4G.ino"
bool FourG_Init();
#line 111 "C:\\Users\\25221\\Desktop\\FC\\4G\\FCGate_4G\\FCGate_4G.ino"
String generateUniqueDevice();
#line 128 "C:\\Users\\25221\\Desktop\\FC\\4G\\FCGate_4G\\FCGate_4G.ino"
void Pin_Int();
#line 148 "C:\\Users\\25221\\Desktop\\FC\\4G\\FCGate_4G\\FCGate_4G.ino"
void mqttTask(void *parameter);
#line 167 "C:\\Users\\25221\\Desktop\\FC\\4G\\FCGate_4G\\FCGate_4G.ino"
void coordinatorTask(void* param);
#line 190 "C:\\Users\\25221\\Desktop\\FC\\4G\\FCGate_4G\\FCGate_4G.ino"
void setup();
#line 234 "C:\\Users\\25221\\Desktop\\FC\\4G\\FCGate_4G\\FCGate_4G.ino"
bool publishSensorData(uint8_t code);
#line 245 "C:\\Users\\25221\\Desktop\\FC\\4G\\FCGate_4G\\FCGate_4G.ino"
bool serverCommandPending();
#line 263 "C:\\Users\\25221\\Desktop\\FC\\4G\\FCGate_4G\\FCGate_4G.ino"
void loop();
#line 1 "C:\\Users\\25221\\Desktop\\FC\\4G\\FCGate_4G\\1Public.ino"
bool CharArrCmp(char *p, char *q, int len);
#line 16 "C:\\Users\\25221\\Desktop\\FC\\4G\\FCGate_4G\\1Public.ino"
String GetTime();
#line 50 "C:\\Users\\25221\\Desktop\\FC\\4G\\FCGate_4G\\1Public.ino"
static bool readFourGLineSafe(String &line, uint32_t timeoutMs);
#line 79 "C:\\Users\\25221\\Desktop\\FC\\4G\\FCGate_4G\\1Public.ino"
static String parseFourGTimeLine(const String &line);
#line 97 "C:\\Users\\25221\\Desktop\\FC\\4G\\FCGate_4G\\1Public.ino"
String GetSafeTime();
#line 132 "C:\\Users\\25221\\Desktop\\FC\\4G\\FCGate_4G\\1Public.ino"
unsigned short CRC_16_HEX(unsigned char *buf, int len);
#line 153 "C:\\Users\\25221\\Desktop\\FC\\4G\\FCGate_4G\\1Public.ino"
bool rs485_send(unsigned char *hex,uint16_t back_len,unsigned char data[]);
#line 340 "C:\\Users\\25221\\Desktop\\FC\\4G\\FCGate_4G\\1Public.ino"
bool SetData(uint16_t pos,uint16_t value);
#line 355 "C:\\Users\\25221\\Desktop\\FC\\4G\\FCGate_4G\\1Public.ino"
bool SetStatusData(uint16_t pos,uint16_t value);
#line 9 "C:\\Users\\25221\\Desktop\\FC\\4G\\FCGate_4G\\2NvsFlash.ino"
esp_err_t FlashInit();
#line 62 "C:\\Users\\25221\\Desktop\\FC\\4G\\FCGate_4G\\2NvsFlash.ino"
void saveWifiConfig();
#line 83 "C:\\Users\\25221\\Desktop\\FC\\4G\\FCGate_4G\\2NvsFlash.ino"
void saveMqttConfig();
#line 103 "C:\\Users\\25221\\Desktop\\FC\\4G\\FCGate_4G\\2NvsFlash.ino"
esp_err_t Write_String(const char* key, const char* value);
#line 136 "C:\\Users\\25221\\Desktop\\FC\\4G\\FCGate_4G\\3Ble.ino"
void stopBLE();
#line 148 "C:\\Users\\25221\\Desktop\\FC\\4G\\FCGate_4G\\3Ble.ino"
void restartBLE(String newName);
#line 2 "C:\\Users\\25221\\Desktop\\FC\\4G\\FCGate_4G\\8CoordinatorTask.ino"
void PushJson(uint8_t type,String jsonStr,uint8_t qos);
#line 14 "C:\\Users\\25221\\Desktop\\FC\\4G\\FCGate_4G\\doiot.ino"
static bool queueFourGCommandJson(String jsonStr);
#line 41 "C:\\Users\\25221\\Desktop\\FC\\4G\\FCGate_4G\\doiot.ino"
void queueFourGCommandPayload(String receivedString);
#line 143 "C:\\Users\\25221\\Desktop\\FC\\4G\\FCGate_4G\\doiot.ino"
void clrRxBuffer(void);
#line 159 "C:\\Users\\25221\\Desktop\\FC\\4G\\FCGate_4G\\doiot.ino"
unsigned int sendCommand(const char *Command, const char *Response, unsigned long Timeout, unsigned char Retry);
#line 204 "C:\\Users\\25221\\Desktop\\FC\\4G\\FCGate_4G\\doiot.ino"
void subscribeTopic(String topic);
#line 240 "C:\\Users\\25221\\Desktop\\FC\\4G\\FCGate_4G\\doiot.ino"
bool publishDataToServer(String msg);
#line 45 "C:\\Users\\25221\\Desktop\\FC\\4G\\FCGate_4G\\FCGate_4G.ino"
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

#line 1 "C:\\Users\\25221\\Desktop\\FC\\4G\\FCGate_4G\\1Public.ino"
bool CharArrCmp(char *p, char *q, int len)
{
  bool flag = true;
  for (int i = 0; i < len; i++)
  {
    if (*p != *q)
    {
      flag = false;
      break;
    }
    p++;
    q++;
  }
  return flag;
}
String GetTime()  //获取时间
{
  String timeString="",RxBuffer="";
 bool flag=false;
 // clrRxBuffer();

    for (unsigned char n = 0; n < 5; n++)
    {
      debugPrint("send AT Command:");
      FOUR_G_SERIAL.write("AT+TIME?\r\n");
     unsigned short Time_Cont = 0;
      while (Time_Cont++ < 1000)
      {
        delay(1);
        RxBuffer=ReadBuffer();
        // Serial.print(".");	//1.8.x的IDE不加这个会有bug
        if (RxBuffer.indexOf("+TIME")>0)
        {
          timeString=RxBuffer.substring(9,28);
          //  timeString.replace("T"," ");
          //Serial.println(Time_Cont);
          //lastTimeStr=timeString;
          flag=true;
          break;
        }
      }
      if(flag) break;
    }
	return timeString;
}
//------------------------------------------------------------------------------------------

//------------------------------------------------------------------------------------------20241008 加入

static bool readFourGLineSafe(String &line, uint32_t timeoutMs)
{
  line = "";
  uint32_t startMs = millis();
  while (millis() - startMs < timeoutMs) {
    while (FOUR_G_SERIAL.available()) {
      char c = (char)FOUR_G_SERIAL.read();
      if (c == '\r') {
        continue;
      }
      if (c == '\n') {
        line.trim();
        if (line.length() > 0) {
          return true;
        }
        line = "";
      } else {
        line += c;
        if (line.length() > MAX_JSON_LEN) {
          line = "";
        }
      }
    }
    delay(1);
  }
  line.trim();
  return line.length() > 0;
}

static String parseFourGTimeLine(const String &line)
{
  int timePos = line.indexOf("+TIME=");
  if (timePos < 0) {
    return "";
  }

  int firstQuote = line.indexOf('"', timePos);
  int secondQuote = line.indexOf('"', firstQuote + 1);
  if (firstQuote >= 0 && secondQuote > firstQuote) {
    String timeString = line.substring(firstQuote + 1, secondQuote);
    if (timeString.length() >= 19) {
      return timeString.substring(0, 19);
    }
  }
  return "";
}

String GetSafeTime()
{
  static String lastTimeString = "";
  if (!takeFourGSerial(1500)) {
    return lastTimeString;
  }

  for (unsigned char n = 0; n < 3; n++) {
    debugPrint("send AT Command:");
    FOUR_G_SERIAL.write("AT+TIME?\r\n");
    uint32_t startMs = millis();
    while (millis() - startMs < 1000) {
      String line = "";
      if (!readFourGLineSafe(line, 100)) {
        continue;
      }

      if (line.indexOf("nodeId") > -1) {
        queueFourGCommandPayload(line);
        continue;
      }

      String parsedTime = parseFourGTimeLine(line);
      if (parsedTime.length() == 19) {
        lastTimeString = parsedTime;
        releaseFourGSerial();
        return lastTimeString;
      }
    }
  }

  releaseFourGSerial();
  return lastTimeString;
}

unsigned short CRC_16_HEX(unsigned char *buf, int len) {
    unsigned short crc = 0xFFFF;
    for (int pos = 0; pos < len; pos++) {
        crc ^= buf[pos];
        for (int i = 8; i != 0; i--) {
            if ((crc & 0x0001) != 0) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    // 移除字节交换操作，返回原始CRC值
    return crc;
}


/*/-----------------------------------------------------------------------------------------------------------
*/
//-----------------------------------------------------------------------------------------------------------
bool rs485_send(unsigned char *hex,uint16_t back_len,unsigned char data[]) //modbus指令，返回数组长度，返回数组名称
{
   bool flag=false;
  unsigned short crc=0;
  unsigned short CRC_Tmp=0;
  unsigned char localData[128] = {0};
  if (back_len == 0 || back_len > sizeof(localData)) {
    return false;
  }
  unsigned char *rxData = data ? data : localData;
 if (xSemaphoreTake(g_stateMutex, pdMS_TO_TICKS(5000)) == pdTRUE) //500ms加锁
 {
  while(mySerial.available()>0) mySerial.read();
  mySerial.write(hex,8);
  unsigned long startMs = millis();
  while(millis() - startMs < 500)
   {
    if(mySerial.available() >= back_len)
    {
       size_t readLen = mySerial.readBytes(rxData,back_len);
       if (readLen != back_len) {
        break;
       }
        crc=(rxData[back_len-1]<<8)|rxData[back_len-2]; //收到数据的crc校验值
       CRC_Tmp=CRC_16_HEX(rxData,back_len-2);//验证校验位是否正确
      if(crc==CRC_Tmp)
       flag=true;
      else
      flag=false;
      break;
    }
    vTaskDelay(5 / portTICK_PERIOD_MS);
  }
    while(mySerial.available()>0) mySerial.read();
    xSemaphoreGive(g_stateMutex); //解锁
 }
  else {
    // ❌ 超时未获取到锁的处理逻辑
    debugPrintln(F("[WARN] Mutex timeout! Skipping resource access."));
    flag=false;
}
return flag;
}
//-------------------------------------------------------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------------------------------------------------------
//封装传感器数据
String GetSensorData(uint8_t code,String hashCode) {
   // debugPrintln("GetData() called with: " + kind);
  String TopicContent="";
 uint16_t RESPONSE_LENGTH=0;
  bool result = false;
  JsonDocument doc;
  doc["code"] = code;
  doc["hashCode"] = hashCode;
  doc["nodeId"] = MyNode.nodeId;
  doc["timestamp"] = GetSafeTime();
  switch(code)
  {
   case 1://读取当前数据
    {
      uint16_t value[5];
      RESPONSE_LENGTH=Sersor_addr[0][5]*2+5;
      unsigned char out[RESPONSE_LENGTH];
      result = rs485_send(Sersor_addr[0], RESPONSE_LENGTH,out);
     if(result)
       {
       // value[0]=(out[5]<<8)+out[6];//当前开关状态设定值
        value[0]=(out[3]<<8)+out[4];//当前空调运行模式
        value[1]=(out[23]<<8)+out[24];//当前温度
        value[2]=(out[37]<<8)+out[38];//当前湿度
        value[3]=(out[93]<<8)+out[94];//当前二氧化碳
        result=false;
        RESPONSE_LENGTH=7;
        unsigned char tmpOut[7];
        result = rs485_send(Sersor_addr[2], RESPONSE_LENGTH,tmpOut);
       if(result)
         value[4]=(tmpOut[3] << 8) | tmpOut[4];
       else value[4]=0;
        JsonObject dataObj = doc["data"].to<JsonObject>();
        dataObj["curMode"] = value[0];   //当前工作模式
        dataObj["curtemp"] = value[1];   //当前湿度设
        dataObj["curHum"] = value[2];   //当前温度
        dataObj["curCO2"] = value[3];   //当前 CO2浓度
        dataObj["curFreq"] = value[4];   //空调运行频率
       }
      else
        doc["result"] = false;
      break;
    }
     case 4://读取设定数据
     {
       uint16_t value[5];
      RESPONSE_LENGTH=Sersor_addr[1][5]*2+5;
      unsigned char out[RESPONSE_LENGTH];
      result = rs485_send(Sersor_addr[1], RESPONSE_LENGTH,out);
      if(result)
       {
        value[0]=(out[5]<<8)+out[6];//当前开关状态设定值
        value[1]=(out[7]<<8)+out[8];//当前模式状态设定值
        value[2]=(out[11]<<8)+out[12];//当前温度设定值
        value[3]=(out[31]<<8)+out[32];//当前湿度设定值
        value[4]=(out[61]<<8)+out[62];//当前二氧化碳设定值

        JsonArray dataArray = doc["data"].to<JsonArray>();
        JsonObject statusObj = dataArray.createNestedObject();
        statusObj["type"] = "statusSet";
        statusObj["status"] = String(value[0]);
        JsonObject modeObj = dataArray.createNestedObject();
        modeObj["type"] = "modeSet";
        modeObj["status"] = String(value[1]);
        JsonObject tempObj = dataArray.createNestedObject();
        tempObj["type"] = "tempSet";
        tempObj["status"] = String(value[2]);
        JsonObject humObj = dataArray.createNestedObject();
        humObj["type"] = "humSet";
        humObj["status"] = String(value[3]);
        JsonObject co2Obj = dataArray.createNestedObject();
        co2Obj["type"] = "CO2Set";
        co2Obj["status"] = String(value[4]);
        }
      else
        doc["result"] = false;
      break;
      }
    case 5://读取错误状态
     {
       unsigned char value[9];
       unsigned char out[14] = {0};
      result = rs485_send(Sersor_addr[3], 14,out);
      if(result)
       {
          for(uint8_t i = 0; i < 9; i++)
          {
            value[i] = out[i + 3];
          }
         String faultHexStr = "";
         for(uint8_t i = 0; i < 9; i++)
         {
          if(value[i] < 0x10)
            faultHexStr += "0";
           faultHexStr += String(value[i], HEX);
          if(i < 8) faultHexStr += " ";
         }
        JsonArray dataArray = doc["data"].to<JsonArray>();
        JsonObject faultObj = dataArray.createNestedObject();
        faultObj["type"] = "AC";
        faultObj["faultState"] = faultHexStr;
       }
      else
       doc["result"] = false;
        break;
       }
     case 6://读取外接三合一传感器 建大仁科
     {

       uint16_t value[3];
       unsigned char out[11] = {0};
      result = rs485_send(Sersor_addr[4], 11,out);
      if(result)
       {
           JsonObject dataObj = doc["data"].to<JsonObject>();
          // 解析湿度值（索引3-4，大端格式）
          value[0] = (out[3] << 8) | out[4];
          float humidity = value[0] / 10.0;

          // 解析温度值（索引5-6，大端格式，有符号数）
          value[1]= (out[5] << 8) | out[6];
          float temperature = value[1] / 10.0;

          // 解析CO2浓度（索引7-8，大端格式）
          value[2] = (out[7] << 8) | out[8];
          dataObj["curtemp"] = temperature;
          dataObj["curHum"] = humidity;
          dataObj["curCO2"] = value[2];
       }
      else
       doc["result"] = false;
        break;
       }
  }
  //------------------------------------------
  serializeJson(doc, TopicContent);
  return TopicContent;
}
//-------------------------------------------------------------------------------------------------------------------------------------

//---------------------------------------------------------------------------------
bool SetData(uint16_t pos,uint16_t value)
{
    unsigned char cmd[8]={0x01,0x06};
    unsigned short CRC_Tmp=0;
    cmd[2]=pos>>8;
    cmd[3]=pos& 0xFF;
    cmd[4]=value>>8;
    cmd[5]=value& 0xFF;
    CRC_Tmp=CRC_16_HEX(cmd,6);//验证校验位是否正确
    cmd[6]=CRC_Tmp& 0xFF;
    cmd[7]=CRC_Tmp>>8;
    bool result=rs485_send(cmd,8,0);
    return result;
}

bool SetStatusData(uint16_t pos,uint16_t value)
{
    unsigned char cmd[8]={0x01,0x02};
    unsigned short CRC_Tmp=0;
    cmd[2]=pos>>8;
    cmd[3]=pos& 0xFF;
    cmd[4]=value>>8;
    cmd[5]=value& 0xFF;
    CRC_Tmp=CRC_16_HEX(cmd,6);
    cmd[6]=CRC_Tmp& 0xFF;
    cmd[7]=CRC_Tmp>>8;
    bool result=rs485_send(cmd,8,0);
    return result;
}

#line 1 "C:\\Users\\25221\\Desktop\\FC\\4G\\FCGate_4G\\2NvsFlash.ino"
#include "nvs_flash.h"
static const char *WF = "wifi";
static const char *MQ = "mqtt";
static const char *SE = "sensor";
#define NVS_NAMESPACE "storage"
/*
初始化闪存
*/
esp_err_t FlashInit()
{
  esp_err_t err;
  err = nvs_flash_init();
  // nvs_flash_erase();
  if (err != ESP_OK)
  {
    err = nvs_flash_erase();
    if (err == ESP_OK)
      err = nvs_flash_init();
  }
  return err;
}

/*
读取wifi,mqtt,传感器enable
*/
void loadConfig()
{
  // esp_err_t err = FlashInit();
  // if (err != ESP_OK)
  //   return;
  //size_t wifi_len = sizeof(config_wifi); // esp32 c3
  size_t mqtt_len = sizeof(MyGlobal); // esp32 c3
  //size_t sensor_len = sizeof(MySensor); // esp32 c3
  nvs_handle_t handle;

 esp_err_t err = nvs_open("List", NVS_READWRITE, &handle);
  // if (err == ESP_OK)
  //   err = nvs_get_blob(handle, WF, &config_wifi, &wifi_len); // &wifi_len
  // nvs_close(handle);

  err = nvs_open("List1", NVS_READWRITE, &handle);
  if (err == ESP_OK)
    err = nvs_get_blob(handle, MQ, &MyGlobal, &mqtt_len); // &mqtt_len
  nvs_close(handle);

  // err = nvs_open("List2", NVS_READWRITE, &handle);
  // if (err == ESP_OK)
  //   err = nvs_get_blob(handle, SE, &MySensor, &sensor_len); // &mqtt_len
  // nvs_close(handle);
  // debugPrintln(" ");
  // debugPrintln("-----NsvRead config-----");
  // Use_debugPrint("stassid:");
  // Use_debugPrintln(config_wifi.stassid);
  // Use_debugPrint("stapsw:");
  // Use_debugPrintln(config_wifi.stapsw);
  // Use_debugPrintln("-------------------");
}

/*
写入wifi
*/
void saveWifiConfig()
{
  // esp_err_t err = FlashInit();
  // if (err != ESP_OK)
  //   return;
  nvs_handle_t handle;
  size_t wifi_len = sizeof(config_wifi); // esp32 c3
 esp_err_t err = nvs_open("List", NVS_READWRITE, &handle);
  if (err == ESP_OK)
  {
    err = nvs_set_blob(handle, WF, &config_wifi, wifi_len); // wifi_len
    if (err == ESP_OK)
      nvs_commit(handle);
  }
  nvs_close(handle);
  debugPrintln(F("---NsvSave WIFIconfig---"));
}

/*
写入mqtt
*/
void saveMqttConfig()
{
  // esp_err_t err = FlashInit();
  // if (err != ESP_OK)
  //   return;
  nvs_handle_t handle;
  size_t mqtt_len = sizeof(MyGlobal); // esp32 c3 
 esp_err_t err = nvs_open("List1", NVS_READWRITE, &handle);
  if (err == ESP_OK)
  {
    err = nvs_set_blob(handle, MQ, &MyGlobal, mqtt_len); // wifi_len
    if (err == ESP_OK)
      nvs_commit(handle);
  }
  nvs_close(handle);
  debugPrintln(F("---NsvSave MQTTconfig---"));
}


//-----------------------------------length写入记录字符串长度的地址，addr是字符串的地址，str为要保存的字符串 flag第一存储
esp_err_t Write_String(const char* key, const char* value)
{
  // esp_err_t err = FlashInit();
  // if (err != ESP_OK)
  //   return err;
    nvs_handle_t my_handle;

    // 打开命名空间
   esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK) return err;

    // 写入字符串（自动处理长度）
    err = nvs_set_str(my_handle, key, value);
    if (err != ESP_OK) {
        nvs_close(my_handle);
        return err;
    }

    // 提交更改
    err = nvs_commit(my_handle);
    nvs_close(my_handle);
    return err;
}

//--------------------------------------------------------------
String Read_String(const char* key)
{
    char out_value[20];
    // esp_err_t err = FlashInit();
    // if (err != ESP_OK)
    //   return "";
    nvs_handle_t my_handle;
    size_t len = sizeof(out_value);
   esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READONLY, &my_handle);
    if (err != ESP_OK) return "";

    // 读取字符串
    err = nvs_get_str(my_handle, key, out_value,&len);
    nvs_close(my_handle);
    String Str = out_value;
    return Str;
}




#line 1 "C:\\Users\\25221\\Desktop\\FC\\4G\\FCGate_4G\\3Ble.ino"
//----------------------------------------------------定义蓝牙解析使用的class们 Start------------------------
/// @brief 蓝牙通讯的响应结构
/// 蓝牙传输，一般不超过512个字节，初始设定Mtu = 512

//String jsonString;
#define SERVICE_UUID           "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_RX "6E400002-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_SENSOR "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID_TX "6E400004-B5A3-F393-E0A9-E50E24DCCA9E"
BLEAdvertising* pAdvertising;
// ========== BLE服务器回调 ==========
class MyServerCallbacks : public BLEServerCallbacks {
public:
    void onConnect(BLEServer* pServer) {
        digitalWrite(ble_Led, HIGH);
        g_peerMtu = BLEDevice::getMTU();
        g_sensorNotifyPending=true;
        g_sensorDataStr = "{\"code\":101,\"nodeId\":\"" + MyNode.nodeId+"\"}";
    }

    void onDisconnect(BLEServer* pServer) { 
        digitalWrite(ble_Led, LOW);
        g_sensorSubscribed = false;
        debugPrintln("BLE连接断开,重启广播");
        pServer->startAdvertising();
    }
     void onMtuChanged(BLEServer *pServer,  uint16_t mtu)
     {
      //debugPrintln("MTU changed to: " + String(mtu));
      g_peerMtu = mtu;
    }
};

// ========== BLE写入回调 ==========
class ControlCallback : public BLECharacteristicCallbacks {
public:
 void onWrite(BLECharacteristic* pChar) override {
        // Step 1: 获取原始数据
        const uint8_t* data = pChar->getData();
        size_t len = pChar->getLength(); // 注意：用 getLength()，不是 getValueLength()
        if (len == 0 || len >= MAX_JSON_LEN) return;
        // Step 2: 清理控制字符（只保留可打印 ASCII）
        char cleanBuf[MAX_JSON_LEN] = {0};
        size_t outIdx = 0;
        for (size_t i = 0; i < len && outIdx < MAX_JSON_LEN - 1; i++) {
            char c = (char)data[i];
            if (c >= 32 && c <= 126) {
                cleanBuf[outIdx++] = c;
            }
        }
        if (outIdx == 0) return;



         Command* pCmd = NULL;

        // 1. 从空闲池获取一个 Command 块（如果池满则阻塞等待）
        if (xQueueReceive(xFreeCmdQueue, &pCmd, pdMS_TO_TICKS(5000)) == pdTRUE) { 
            // 2. 安全地填充数据
            memcpy(pCmd->jsonStr, cleanBuf, outIdx);
            pCmd->jsonStr[outIdx] = '\0'; // 确保结尾
            pCmd->src = 2;
            // 3. 将填充好的指针发送到工作队列
            xQueueSend(g_commandQueue, &pCmd, pdMS_TO_TICKS(5000));
        }
        vTaskDelay(pdMS_TO_TICKS(1000));
        // Step 3: 构造命令（局部变量，栈上分配）
        // Command cmd;
        // cmd.src = 2;
        // memcpy(cmd.jsonStr, cleanBuf, outIdx);
        // cmd.jsonStr[outIdx] = '\0'; // 确保结尾
        // xQueueSendToBack(g_commandQueue, &cmd, pdMS_TO_TICKS(10));
        // Step 4: ⚠️ 在 ISR 中发送队列 → 必须用 FromISR 版本！
    //     BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    //     xQueueSendToBackFromISR(g_commandQueue, &cmd, &xHigherPriorityTaskWoken);

    //     // Step 5: 如果有高优先级任务被唤醒，立即调度
    //     portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
    //    //  debugPrintf("收到BLE指令:%s\n", cmd.jsonStr);
    }
};

// ========== BLE订阅回调 ==========
class SensorCallback : public BLECharacteristicCallbacks {
public:
    void onSubscribe(BLECharacteristic* pChar, uint16_t subValue) {
        g_sensorSubscribed = (subValue != 0);
        debugPrintln(g_sensorSubscribed ? "传感器通道已订阅" : "传感器通道取消订阅");
    }
};

// ========== BLE初始化函数 ==========
void BleInit() {
    // 初始化内置NimBLE（官方API）
    BLEDevice::init(MyNode.nodeName.c_str());
    BLEDevice::setPower(ESP_PWR_LVL_P9); // 设置发射功率
    BLEDevice::setMTU(MAX_JSON_LEN); // 设置
    // 创建BLE服务器
    BLEServer* pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());
    // 创建BLE服务
    BLEService* pService = pServer->createService(SERVICE_UUID);

    // 创建接收特征值（RX）
    BLECharacteristic* pRxChar = pService->createCharacteristic(
        CHARACTERISTIC_UUID_RX,
        BLECharacteristic::PROPERTY_WRITE | BLECharacteristic::PROPERTY_WRITE_NR
    );
    pRxChar->setCallbacks(new ControlCallback());
    // 创建传感器特征值（NOTIFY）
    g_pSensorChar = pService->createCharacteristic(
        CHARACTERISTIC_UUID_SENSOR,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_NOTIFY
    );
    g_pSensorChar->addDescriptor(new BLE2902()); // 添加2902描述符
    g_pSensorChar->setCallbacks(new SensorCallback());


    // 启动服务
    pService->start();

    // 配置并启动广播
    //BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setMinInterval(128);
    pAdvertising->setMaxInterval(256);
    BLEDevice::startAdvertising();

    debugPrintln("BLE初始化完成");
}
//---------------------------
/**
 * @brief 停止当前的 BLE 广播
 */
void stopBLE() {
  if (pAdvertising != nullptr) {
    pAdvertising->stop();
    debugPrintln("BLE Advertising Stopped.");
  } else {
    debugPrintln("No BLE Advertising object to stop.");
  }
}
//-----------------------------------------------
/**
 * @brief 重启 BLE 广播，使用当前 currentDeviceName 变量中的名称
 */
void restartBLE(String newName) {
    // 更新全局变量存储的名称
    //currentDeviceName = newName;

    // 1. 停止当前广告
    stopBLE();

    // 2. 创建新的广告数据对象
    BLEAdvertisementData advertisementData = BLEAdvertisementData();

    // 3. 设置新的设备名称到广告数据中
    advertisementData.setName(newName.c_str());

    // 4. 将更新后的广告数据设置到广告实例
    pAdvertising->setAdvertisementData(advertisementData);

    // 5. 重新开始广播
    pAdvertising->start();

    debugPrint("BLE Advertising Restarted with new name: ");
    debugPrintln(newName);
}
//-------------------------------------------------------------

#line 1 "C:\\Users\\25221\\Desktop\\FC\\4G\\FCGate_4G\\7OTA.ino"
// #include <Update.h>
// #include "Ml307OtaTypes.h"

// static Ml307HttpEvent g_ml307Event;

// static bool ml307ReadByte(char &value, uint32_t timeoutMs) {
//   uint32_t start = millis();
//   while (millis() - start < timeoutMs) {
//     if (FOUR_G_SERIAL.available()) {
//       value = (char)FOUR_G_SERIAL.read();
//       return true;
//     }
//     delay(1);
//   }
//   return false;
// }

// static void ml307Drain(uint32_t quietMs = 50) {
//   uint32_t lastRead = millis();
//   while (millis() - lastRead < quietMs) {
//     while (FOUR_G_SERIAL.available()) {
//       FOUR_G_SERIAL.read();
//       lastRead = millis();
//     }
//     delay(1);
//   }
// }

// static String ml307CsvField(const String &line, uint8_t index) {
//   int start = 0;
//   for (uint8_t i = 0; i < index; i++) {
//     start = line.indexOf(',', start);
//     if (start < 0) return "";
//     start++;
//   }

//   int end = line.indexOf(',', start);
//   String value = (end < 0) ? line.substring(start) : line.substring(start, end);
//   value.trim();
//   if (value.length() >= 2 && value[0] == '"' && value[value.length() - 1] == '"') {
//     value = value.substring(1, value.length() - 1);
//   }
//   return value;
// }

// static int ml307HexNibble(char c) {
//   if (c >= '0' && c <= '9') return c - '0';
//   if (c >= 'a' && c <= 'f') return c - 'a' + 10;
//   if (c >= 'A' && c <= 'F') return c - 'A' + 10;
//   return -1;
// }

// static String ml307HexEncode(const String &value) {
//   const char *hex = "0123456789ABCDEF";
//   String encoded;
//   encoded.reserve(value.length() * 2);
//   for (size_t i = 0; i < value.length(); i++) {
//     uint8_t b = (uint8_t)value[i];
//     encoded += hex[b >> 4];
//     encoded += hex[b & 0x0F];
//   }
//   return encoded;
// }

// static bool ml307HexDecodeToString(const String &hex, String &decoded) {
//   decoded = "";
//   decoded.reserve(hex.length() / 2);
//   for (size_t i = 0; i + 1 < hex.length(); i += 2) {
//     int hi = ml307HexNibble(hex[i]);
//     int lo = ml307HexNibble(hex[i + 1]);
//     if (hi < 0 || lo < 0) return false;
//     decoded += (char)((hi << 4) | lo);
//   }
//   return true;
// }

// static bool ml307ParseUrl(const String &url, String &protocol, String &host, String &path) {
//   int protocolEnd = url.indexOf("://");
//   if (protocolEnd <= 0) return false;

//   protocol = url.substring(0, protocolEnd);
//   protocol.toLowerCase();
//   if (protocol != "http" && protocol != "https") return false;

//   int hostStart = protocolEnd + 3;
//   int pathStart = url.indexOf('/', hostStart);
//   if (pathStart < 0) {
//     host = url.substring(hostStart);
//     path = "/";
//   } else {
//     host = url.substring(hostStart, pathStart);
//     path = url.substring(pathStart);
//   }

//   host.trim();
//   return host.length() > 0;
// }

// static size_t ml307ParseContentLength(const String &headers) {
//   String lower = headers;
//   lower.toLowerCase();
//   int pos = lower.indexOf("content-length:");
//   if (pos < 0) return 0;

//   pos += strlen("content-length:");
//   while (pos < (int)headers.length() && (headers[pos] == ' ' || headers[pos] == '\t')) {
//     pos++;
//   }

//   int end = pos;
//   while (end < (int)headers.length() && isDigit(headers[end])) {
//     end++;
//   }

//   if (end <= pos) return 0;
//   return (size_t)headers.substring(pos, end).toInt();
// }

// static bool ml307ReadHexChar(char &value, uint32_t timeoutMs) {
//   while (true) {
//     if (!ml307ReadByte(value, timeoutMs)) return false;
//     if (value != '\r' && value != '\n') return true;
//   }
// }

// static bool ml307ReadAndWriteHexContent(size_t byteCount, size_t &writtenLength, bool writeToUpdate, uint32_t timeoutMs) {
//   uint8_t buffer[512];
//   size_t bufferLen = 0;
//   writtenLength = 0;

//   char firstChar = 0;
//   if (!ml307ReadHexChar(firstChar, timeoutMs)) return false;
//   bool quoted = firstChar == '"';
//   bool hasPendingHexChar = !quoted;
//   char pendingHexChar = firstChar;

//   for (size_t i = 0; i < byteCount; i++) {
//     char highChar = 0;
//     char lowChar = 0;
//     if (hasPendingHexChar) {
//       highChar = pendingHexChar;
//       hasPendingHexChar = false;
//     } else if (!ml307ReadHexChar(highChar, timeoutMs)) {
//       return false;
//     }

//     if (!ml307ReadHexChar(lowChar, timeoutMs)) {
//       return false;
//     }

//     int high = ml307HexNibble(highChar);
//     int low = ml307HexNibble(lowChar);
//     if (high < 0 || low < 0) {
//       return false;
//     }

//     buffer[bufferLen++] = (uint8_t)((high << 4) | low);
//     if (bufferLen == sizeof(buffer)) {
//       if (writeToUpdate && Update.write(buffer, bufferLen) != bufferLen) {
//         return false;
//       }
//       writtenLength += bufferLen;
//       bufferLen = 0;
//     }
//   }

//   if (bufferLen > 0) {
//     if (writeToUpdate && Update.write(buffer, bufferLen) != bufferLen) {
//       return false;
//     }
//     writtenLength += bufferLen;
//   }

//   if (quoted) {
//     char closingQuote = 0;
//     if (!ml307ReadHexChar(closingQuote, timeoutMs) || closingQuote != '"') {
//       return false;
//     }
//   }
//   return true;
// }

// static void ml307ConsumeLineEnding(uint32_t timeoutMs) {
//   char c = 0;
//   uint32_t start = millis();
//   while (millis() - start < timeoutMs) {
//     if (!ml307ReadByte(c, timeoutMs)) return;
//     if (c == '\n') return;
//   }
// }

// static bool ml307ParseHttpLine(String line) {
//   line.trim();
//   g_ml307Event.line = line;

//   if (line == "OK") {
//     g_ml307Event.type = Ml307HttpEvent::OK;
//     return true;
//   }

//   if (line.indexOf("ERROR") >= 0) {
//     g_ml307Event.type = Ml307HttpEvent::ERROR;
//     return true;
//   }

//   if (line.startsWith("+MHTTPCREATE:")) {
//     g_ml307Event.type = Ml307HttpEvent::CREATE;
//     int colon = line.indexOf(':');
//     g_ml307Event.httpId = line.substring(colon + 1).toInt();
//     return true;
//   }

//   if (line.startsWith("+MHTTPURC: \"header\"")) {
//     g_ml307Event.type = Ml307HttpEvent::HEADER;
//     g_ml307Event.httpId = ml307CsvField(line, 1).toInt();
//     g_ml307Event.statusCode = ml307CsvField(line, 2).toInt();

//     String decodedHeaders;
//     if (ml307HexDecodeToString(ml307CsvField(line, 4), decodedHeaders)) {
//       g_ml307Event.contentLength = ml307ParseContentLength(decodedHeaders);
//     }
//     return true;
//   }

//   if (line.startsWith("+MHTTPURC: \"err\"")) {
//     g_ml307Event.type = Ml307HttpEvent::ERROR;
//     g_ml307Event.httpId = ml307CsvField(line, 1).toInt();
//     g_ml307Event.errorCode = ml307CsvField(line, 2).toInt();
//     return true;
//   }

//   g_ml307Event.type = Ml307HttpEvent::OTHER;
//   return true;
// }

// static bool ml307ReadNextHttpEvent(bool writeContent, uint32_t timeoutMs) {
//   g_ml307Event = Ml307HttpEvent();
//   String line = "";
//   int commaCount = 0;
//   uint32_t start = millis();

//   while (millis() - start < timeoutMs) {
//     char c = 0;
//     if (!ml307ReadByte(c, timeoutMs)) return false;
//     if (c == '\r') continue;

//     if (line.length() == 0 && c == '\n') {
//       continue;
//     }

//     line += c;
//     if (line.startsWith("+MHTTPURC: \"content\"")) {
//       if (c == ',') {
//         commaCount++;
//       }

//       if (commaCount == 5) {
//         g_ml307Event.type = Ml307HttpEvent::CONTENT;
//         g_ml307Event.httpId = ml307CsvField(line, 1).toInt();
//         g_ml307Event.contentLength = (size_t)ml307CsvField(line, 2).toInt();
//         g_ml307Event.sumLength = (size_t)ml307CsvField(line, 3).toInt();
//         g_ml307Event.currentLength = (size_t)ml307CsvField(line, 4).toInt();
//         g_ml307Event.line = line;

//         bool ok = ml307ReadAndWriteHexContent(g_ml307Event.currentLength, g_ml307Event.writtenLength, writeContent, timeoutMs);
//         ml307ConsumeLineEnding(1000);
//         return ok;
//       }
//     }

//     if (c == '\n') {
//       return ml307ParseHttpLine(line);
//     }

//     if (line.length() > 2048 && !line.startsWith("+MHTTPURC: \"content\"")) {
//       g_ml307Event.type = Ml307HttpEvent::ERROR;
//       g_ml307Event.line = line;
//       return false;
//     }
//   }

//   return false;
// }

// static void ml307SendRawCommand(const String &command) {
//   debugPrint(F("send AT Command:"));
//   debugPrintln(command);
//   FOUR_G_SERIAL.print(command);
//   FOUR_G_SERIAL.print("\r\n");
// }

// static bool ml307SendCommandWaitOk(const String &command, uint32_t timeoutMs) {
//   ml307Drain();
//   ml307SendRawCommand(command);

//   uint32_t start = millis();
//   while (millis() - start < timeoutMs) {
//     if (!ml307ReadNextHttpEvent(false, timeoutMs)) {
//       return false;
//     }

//     if (g_ml307Event.type == Ml307HttpEvent::OK) return true;
//     if (g_ml307Event.type == Ml307HttpEvent::ERROR) return false;
//   }
//   return false;
// }

// static bool ml307CreateHttp(const String &protocol, const String &host, int &httpId) {
//   ml307SendCommandWaitOk("AT+MHTTPDEL=0", 3000);
//   ml307Drain();
//   ml307SendRawCommand("AT+MHTTPCREATE=\"" + protocol + "://" + host + "\"");

//   bool gotOk = false;
//   bool gotCreate = false;
//   uint32_t start = millis();
//   while (millis() - start < 10000) {
//     if (!ml307ReadNextHttpEvent(false, 10000)) {
//       return false;
//     }

//     if (g_ml307Event.type == Ml307HttpEvent::OK) {
//       gotOk = true;
//     } else if (g_ml307Event.type == Ml307HttpEvent::CREATE) {
//       httpId = g_ml307Event.httpId;
//       gotCreate = true;
//     } else if (g_ml307Event.type == Ml307HttpEvent::ERROR) {
//       return false;
//     }

//     if (gotOk && gotCreate) return true;
//   }
//   return false;
// }

// static void ml307CloseHttp(int httpId) {
//   if (httpId >= 0) {
//     ml307SendCommandWaitOk("AT+MHTTPDEL=" + String(httpId), 5000);
//   }
// }

// static bool ml307BeginHttpGet(const String &protocol, const String &path, int httpId) {
//   if (protocol == "https") {
//     if (!ml307SendCommandWaitOk("AT+MHTTPCFG=\"ssl\"," + String(httpId) + ",1,0", 10000)) {
//       return false;
//     }
//   }

//   if (!ml307SendCommandWaitOk("AT+MHTTPCFG=\"encoding\"," + String(httpId) + ",0,0", 10000)) {
//     return false;
//   }

//   if (!ml307SendCommandWaitOk("AT+MHTTPCFG=\"encoding\"," + String(httpId) + ",1,1", 10000)) {
//     return false;
//   }

//   String request = "AT+MHTTPREQUEST=" + String(httpId) + ",1,0," + ml307HexEncode(path);
//   return ml307SendCommandWaitOk(request, 10000);
// }

// static bool ml307WaitForOtaBody(int httpId) {
//   bool updateStarted = false;
//   bool gotHeader = false;
//   size_t expectedLength = 0;
//   size_t totalWritten = 0;
//   uint32_t lastProgressPrint = 0;

//   while (true) {
//     if (!ml307ReadNextHttpEvent(updateStarted, 60000)) {
//       debugPrintln(F("HTTP OTA read timeout or parse failed"));
//       return false;
//     }

//     if (g_ml307Event.type == Ml307HttpEvent::OTHER || g_ml307Event.type == Ml307HttpEvent::OK) {
//       continue;
//     }

//     if (g_ml307Event.httpId >= 0 && g_ml307Event.httpId != httpId) {
//       continue;
//     }

//     if (g_ml307Event.type == Ml307HttpEvent::ERROR) {
//       debugPrint(F("HTTP OTA module error:"));
//       debugPrintln(g_ml307Event.errorCode);
//       return false;
//     }

//     if (g_ml307Event.type == Ml307HttpEvent::HEADER) {
//       gotHeader = true;
//       expectedLength = g_ml307Event.contentLength;
//       debugPrint(F("HTTP OTA status:"));
//       debugPrintln(g_ml307Event.statusCode);
//       debugPrint(F("HTTP OTA length:"));
//       debugPrintln((uint32_t)expectedLength);

//       if (g_ml307Event.statusCode != 200) {
//         return false;
//       }

//       size_t beginSize = expectedLength > 0 ? expectedLength : UPDATE_SIZE_UNKNOWN;
//       if (!Update.begin(beginSize, U_FLASH)) {
//         debugPrint(F("Update.begin failed:"));
//         debugPrintln(Update.getError());
//         return false;
//       }
//       updateStarted = true;
//       continue;
//     }

//     if (g_ml307Event.type == Ml307HttpEvent::CONTENT) {
//       if (!gotHeader || !updateStarted) {
//         debugPrintln(F("HTTP OTA content before header"));
//         return false;
//       }

//       totalWritten += g_ml307Event.writtenLength;
//       if (millis() - lastProgressPrint > 1000) {
//         lastProgressPrint = millis();
//         debugPrint(F("HTTP OTA progress:"));
//         debugPrint((uint32_t)totalWritten);
//         debugPrint(F("/"));
//         debugPrintln((uint32_t)expectedLength);
//       }

//       if (expectedLength > 0 && g_ml307Event.sumLength >= expectedLength) {
//         break;
//       }

//       if (expectedLength == 0 && g_ml307Event.currentLength == 0) {
//         break;
//       }
//     }
//   }

//   if (expectedLength > 0 && totalWritten != expectedLength) {
//     debugPrint(F("HTTP OTA size mismatch:"));
//     debugPrint((uint32_t)totalWritten);
//     debugPrint(F("/"));
//     debugPrintln((uint32_t)expectedLength);
//     return false;
//   }

//   if (!Update.end(true)) {
//     debugPrint(F("Update.end failed:"));
//     debugPrintln(Update.getError());
//     return false;
//   }

//   if (!Update.isFinished()) {
//     debugPrintln(F("Update not finished"));
//     return false;
//   }

//   return true;
// }

// static bool ml307HttpOta(const String &otaUrl) {
//   String protocol;
//   String host;
//   String path;
//   int httpId = -1;

//   if (!ml307ParseUrl(otaUrl, protocol, host, path)) {
//     debugPrintln(F("Invalid OTA URL"));
//     return false;
//   }

//   if (!ml307CreateHttp(protocol, host, httpId)) {
//     debugPrintln(F("HTTP create failed"));
//     return false;
//   }

//   bool ok = false;
//   if (ml307BeginHttpGet(protocol, path, httpId)) {
//     ok = ml307WaitForOtaBody(httpId);
//   } else {
//     debugPrintln(F("HTTP GET failed"));
//   }

//   ml307CloseHttp(httpId);
//   return ok;
// }

// void updateBin() {
//   MyGlobal.otaUrl.trim();
//   if (MyGlobal.otaUrl.length() == 0) {
//     debugPrintln(F("OTA URL is empty"));
//     return;
//   }

//   debugPrintln(F("Start ML307R HTTP OTA"));
//   Mqtt_Connect_Status = false;
//   lastLostMqtt = millis();

//   net_Lock = 1;
//   uint32_t lockStart = millis();
//   while (net_Lock < 2 && millis() - lockStart < 3000) {
//     delay(10);
//   }
//   net_Lock = 2;

//   bool ok = ml307HttpOta(MyGlobal.otaUrl);

//   net_Lock = 0;
//   lastLostMqtt = millis();

//   if (ok) {
//     debugPrintln(F("OTA success, restarting"));
//     delay(300);
//     ESP.restart();
//   }

//   Update.abort();
//   debugPrintln(F("OTA failed"));
// }

#line 1 "C:\\Users\\25221\\Desktop\\FC\\4G\\FCGate_4G\\8CoordinatorTask.ino"
// ----------------------------------------------------------------发送数据type 1 mqtt， 2 ble
void PushJson(uint8_t type,String jsonStr,uint8_t qos)
{
    jsonStr.trim();
    if (jsonStr.length() == 0) {
      debugPrintln(F("Skip empty payload"));
      return;
    }
    switch(type)
     {
      case 1://mqtt
       // client.publish(MyGlobal.pubTopic, jsonStr.c_str(), false,qos); 
        publishDataToServer(jsonStr);
        
        break;
      case 2://ble
      {
          debugPrintln(F("BLE data"));
          g_sensorDataStr = jsonStr;
          g_sensorNotifyPending = true;
          break;
      }

     }
}
// ============ 任务3：指令协调器（核心） ============
void handleData(String jsonStr,uint8_t src)
{
        JsonDocument docJson;// 20240507 arduinoJosn 升级为7.04
        // 反序列化数据
        DeserializationError error = deserializeJson(docJson, jsonStr, DeserializationOption::NestingLimit(8));
        if (error)
        {
          debugPrintln(F("deserializeJson() failed"));
          docJson.clear();//手动释放了dynamicDocument占据的内存
          String TopicContent = "{\"code\":100,\"data\":{\"nodeId\":\"" + MyNode.nodeId+"\"}}";
          PushJson(src,TopicContent,0);
          return;
        }
        // 获取解析后的数据信息
        JsonObject doc = docJson.as<JsonObject>();
        String clientId = doc["nodeId"].as<String>();
        String hashCode = doc["hashCode"].as<String>();
        if (clientId == MyNode.nodeId&&hashCode!=MyNode.hashCode)
        {
          //---------------------------------------OTA升级 !obj["your_key"].isNull()
          // if(!doc["version"].isNull())
          // {

          //  if ( !doc["version"].as<String>().equals(MyNode.version))
          //   {
          //     String version="";
          //     //if(MyNode.version=="0.0"||MyNode.version=="")
          //      // flag = true;  //第一次写入version
          //      MyGlobal.otaUrl=doc["otaUrl"].as<String>();
          //     if (MyGlobal.otaUrl != "")
          //     {
          //     version = doc["version"].as<String>();
          //     Write_String("version",version.c_str());
          //     delay(100);
          //     updateBin();  //测试OTA升级
          //     }
          //     if(!version.equals(MyNode.version))
          //     {
          //     Write_String("version",MyNode.version.c_str());//升级失败版本号写回去
          //     delay(100);
          //     }
          //   }
          // }
          //---------------------------------------
          // debugPrintln("doc[\"code\"] exists: " + String(doc.containsKey("code")));
          // debugPrintln("code value: " + String(doc["code"].as<String>()));
          if (!doc["code"].isNull())
          {
              uint8_t code=doc["code"].as<uint8_t>();
              debugPrint(F("handleData code="));
              debugPrintln(code);
              String responseStr="";
              switch(code)
              {
                case 0://-获取版本
                  {
                    if(MyNode.version=="0.0"||MyNode.version.isEmpty())
                      MyNode.version=Read_String("version");
                     if(MyNode.nodeName.isEmpty())
                      MyNode.nodeName=Read_String("nodeName");
                      String TopicContent = "{\"code\":0,\"hashCode\":\""+hashCode+"\",\"data\":{\"nodeId\":\"" + clientId +"\",\"nodeName\":\""+MyNode.nodeName+"\",\"version\":\""+MyNode.version+"\"}}";
                      PushJson(src,TopicContent,0);
                      break;
                  }
                  case 1://读取传感器数据
                  {
                    responseStr=GetSensorData(1,hashCode);
                    PushJson(src,responseStr,1);
                    break;
                  }
                  case 2://设置数据
                  {
                      JsonArray dataArray = doc["data"].as<JsonArray>();
                      for (JsonObject item : dataArray)
                      {
                        if (!item["type"].isNull())
                        {
                          String type = item["type"].as<String>();
                          uint16_t value = item["setvalue"].as<uint16_t>();
                          // 根据 type 设置对应的数据
                          if (type == "statusSet") {
                            SetStatusData(1, value);
                          } else if (type == "modeSet") {
                            SetData(2, value);
                          } else if (type == "tempSet") {
                            SetData(4, value);
                          } else if (type == "humSet") {
                            SetData(14, value);
                          } else if (type == "CO2Set") {
                            if (SetData(29, value)) {
                              SetData(66, 2);
                            }
                          } else {
                            continue;
                          }
                          vTaskDelay(pdMS_TO_TICKS(250));
                        }
                      }
                    responseStr=GetSensorData(4,hashCode);
                    PushJson(src,responseStr,1);
                    break;
                  }
                  case 3://设置网关配置
                  {
                      JsonObject obj = doc["data"].as<JsonObject>();

                      if (!obj["nameSet"].isNull())
                      {
                        MyNode.nodeName=obj["nameSet"].as<String>();
                        Write_String("nodeName",MyNode.nodeName.c_str());
                       // BLEsetDeviceName();
                       stopBLE();
                        restartBLE(MyNode.nodeName);
                        responseStr = "{\"code\":3,\"hashCode\":\""+hashCode+"\",\"data\":{\"nodeId\":\"" + clientId +"\",\"nodeName\":\""+MyNode.nodeName+"\"}}";
                      }
                    //responseStr=GetData(2,hashCode);
                    PushJson(src,responseStr,1);

                    break;
                  }
                  case 4://读取设备设定数据
                  {
                    responseStr=GetSensorData(4,hashCode);
                    PushJson(src,responseStr,0);
                    break;
                  }
                   case 5://读取设备故障状态GetFaultStateData
                  {
                    responseStr=GetSensorData(5,hashCode);
                    PushJson(src,responseStr,1);
                    break;
                  }
                  case 6://测试二氧化碳传感器
                  {
                    responseStr=GetSensorData(6,hashCode);
                    PushJson(src,responseStr,1);
                    break;
                   }
                 case 7://设置灯光，加湿器，消毒，风阀
                  {
                      JsonArray dataArray = doc["data"].as<JsonArray>();
                      String setResult = "fail";
                      String setType = "";

                      for (JsonObject item : dataArray)
                      {
                        if (!item["type"].isNull())
                        {
                          String type = item["type"].as<String>();
                          uint16_t value = item["setvalue"].as<uint16_t>();
                          bool result = false;

                          // 根据 type 设置对应的数据
                          if (type == "lightSet") {
                            result = SetData(42, value);
                          } else if (type == "humidifierSet") {
                            result = SetData(41, value);
                          } else if (type == "damperSet") {
                            result = SetData(40, value);
                          } else if (type == "disinfectSet") {
                            result = SetData(43, value);
                          }

                          setType = type;
                          setResult = result ? "success" : "fail";
                        }
                      }

                      responseStr = "{\"code\":7,\"hashCode\":\""+hashCode+"\",\"nodeId\":\"" + clientId +"\",\"data\":{\"type\":\""+setType+"\",\"result\":\""+setResult+"\"}}";
                      PushJson(src,responseStr,1);
                      break;
                  }
                  // case 8://ble 配网wifi
                  // {
                  //   JsonObject obj = doc["data"].as<JsonObject>();
                  //   strcpy(config_wifi.stassid, obj["stassid"]);
                  //   strcpy(config_wifi.stapsw, obj["stapsw"]);
                  //   net_Lock=1;
                  //   while(net_Lock<2)vTaskDelay(10 / portTICK_PERIOD_MS);  // 阻塞 10ms ，完全让出 CPU
                  //   bool flag= wifi_Connect(true);
                  //   net_Lock=0;
                  //   responseStr="{\"code\":8,\"hashCode\":\""+hashCode+"\",\"nodeId\":\"" + clientId +"\",\"result\":"+flag+"}";
                  //   PushJson(src,responseStr,0);
                  //   break;
                  // }
                  case 9://ble 配网mqtt
                  {
                    //初始化MyGlobal
                    JsonObject obj = doc["data"].as<JsonObject>();
                    strcpy( MyGlobal.mqttServer, obj["mqttServer"]);
                    MyGlobal.port=obj["port"].as<uint16_t>();
                    strcpy( MyGlobal.userName, obj["userName"]);
                    strcpy( MyGlobal.Psw, obj["Psw"]);
                    MyGlobal.pubQos=obj["pubQos"].as<uint8_t>();
                    MyGlobal.subQos=obj["subQos"].as<uint8_t>();
                   if (!obj["otaUrl"].isNull())
                    MyGlobal.otaUrl=obj["otaUrl"].as<String>();
                    net_Lock=1;
//                    Mqtt_init();
                    while(net_Lock<2)vTaskDelay(10 / portTICK_PERIOD_MS);  // 阻塞 10ms ，完全让出 CPU
                   bool flag=FourG_Init();
                   if(flag) saveMqttConfig();
                    net_Lock=0;
                     responseStr="{\"code\":9,\"hashCode\":\""+hashCode+"\",\"nodeId\":\"" + clientId +"\",\"result\":"+flag+"}";
                    PushJson(src,responseStr,0);
                    break;
                  }
                  case 10:    //---------------------透传modbus指令 {"data":{"modbus":[11,12],"returnLen":10}} 
                  {
                   JsonObject obj = doc["data"].as<JsonObject>();
                    uint16_t len=obj["returnLen"].as<uint16_t>();
                    unsigned char out[len];
                    String faultHexStr="";
                    JsonArray modbusArray = obj["modbus"].as<JsonArray>();
                     // 4. 获取嵌套的 modbus 数组
                    if (modbusArray.size() != 8) {
                      debugPrint("Modbus 报文长度错误! 期望: 8, 实际: ");
                      debugPrintln(modbusArray.size());
                      return; // 长度不符，直接中断处理
                    }
                  // 5. 声明固定为8字节的静态数组，并遍历赋值
                  unsigned char modbusCmd[8]; 
                  for (uint8_t i = 0; i < 8; i++) 
                    modbusCmd[i] = modbusArray[i].as<unsigned char>();
                    bool result=rs485_send(modbusCmd,len,out);  
                    if(result)
                    {
                      for(uint8_t i = 0; i < len; i++)
                      {
                        if(out[i] < 0x10) faultHexStr += "0";
                        faultHexStr += String(out[i], HEX);
                        if(i < len-1) faultHexStr += " ";
                      }
                    }
                    else
                     faultHexStr= "fail";
                   
                    responseStr="{\"code\":10,\"hashCode\":\""+hashCode+"\",\"nodeId\":\"" + clientId +"\",\"result\":\""+faultHexStr+"\"}";
                    PushJson(src,responseStr,1);
                   break;
                  }
                  case 99:    //---------------------重启设备
                  {
                    ESP.restart(); break;
                  }
              }
          }
          MyNode.hashCode=hashCode;
        }
        doc.clear();
        docJson.clear();//手动释放了dynamicDocument占据的内存
}


#line 1 "C:\\Users\\25221\\Desktop\\FC\\4G\\FCGate_4G\\doiot.ino"
bool takeFourGSerial(uint32_t timeoutMs) {
  if (g_fourGMutex == NULL) {
    return true;
  }
  return xSemaphoreTake(g_fourGMutex, pdMS_TO_TICKS(timeoutMs)) == pdTRUE;
}

void releaseFourGSerial() {
  if (g_fourGMutex != NULL) {
    xSemaphoreGive(g_fourGMutex);
  }
}

static bool queueFourGCommandJson(String jsonStr) {
  jsonStr.trim();
  int begin = jsonStr.indexOf('{');
  int end = jsonStr.lastIndexOf('}');
  if (begin < 0 || end <= begin) {
    return false;
  }
  jsonStr = jsonStr.substring(begin, end + 1);

  if (xFreeCmdQueue == NULL || g_commandQueue == NULL) {
    return false;
  }

  Command* pCmd = NULL;
  if (xQueueReceive(xFreeCmdQueue, &pCmd, 0) != pdTRUE) {
    return false;
  }

  snprintf(pCmd->jsonStr, sizeof(pCmd->jsonStr), "%s", jsonStr.c_str());
  pCmd->src = 1;
  if (xQueueSend(g_commandQueue, &pCmd, 0) != pdTRUE) {
    xQueueSend(xFreeCmdQueue, &pCmd, 0);
    return false;
  }
  return true;
}

void queueFourGCommandPayload(String receivedString) {
  int start = 0;
  while (start < (int)receivedString.length()) {
    int split = receivedString.indexOf("}{", start);
    if (split < 0) {
      queueFourGCommandJson(receivedString.substring(start));
      break;
    }
    queueFourGCommandJson(receivedString.substring(start, split + 1));
    start = split + 1;
  }
}

String ReadBuffer() {
  // 清空缓冲区
  // memset(RxBuffer, 0, RxBufferLength);
  // BufferCount = 0;
   String receivedString="",json="";
  if (FOUR_G_SERIAL.available()) {
    // 读取串口接收到的完整指令
     receivedString = FOUR_G_SERIAL.readString(); // 直接读取串口接收到的字符串
   // cleardebugPrintBuffer();
    // 将读取到的指令复制到缓冲区中
    //receivedString.toCharArray(RxBuffer, RxBufferLength);
   // BufferCount = receivedString.length(); // 更新缓冲区长度
   debugPrintln("ReadBuffer receive :");
   debugPrintln(receivedString); // 输出接收到的信息

   if(receivedString.indexOf("nodeId")>-1)
    {
      queueFourGCommandPayload(receivedString);
      return receivedString;
      int pos=receivedString.indexOf("}{");
     //   debugPrint.println(pos); 
       if (xFreeCmdQueue == NULL || g_commandQueue == NULL) {
        return receivedString;
       }
       if(pos == -1)
         // HandleMsg(receivedString);
          {
              Command* pCmd = NULL;
           if (xQueueReceive(xFreeCmdQueue, &pCmd, pdMS_TO_TICKS(5000)) == pdTRUE)
            { 
            // 2. 安全地填充数据
            snprintf(pCmd->jsonStr, sizeof(pCmd->jsonStr), "%s", receivedString.c_str());
            pCmd->src = 1;

            // 3. 将填充好的指针发送到工作队列
            xQueueSend(g_commandQueue, &pCmd, pdMS_TO_TICKS(5000));
           }
        }
      else
      while(pos>0)
      {
        json=receivedString.substring(0,pos+1);
         // debugPrint.println(json); // 输出接收到的信息
        receivedString=receivedString.substring(pos+1);
         //  debugPrint.println(receivedString); // 输出接收到的信息
         pos=receivedString.indexOf("}{");
         //HandleMsg(json);
           Command* pCmd = NULL;
         if (xQueueReceive(xFreeCmdQueue, &pCmd, pdMS_TO_TICKS(5000)) == pdTRUE)
            { 
            // 2. 安全地填充数据
            snprintf(pCmd->jsonStr, sizeof(pCmd->jsonStr), "%s", json.c_str());
            pCmd->src = 1;

            // 3. 将填充好的指针发送到工作队列
            xQueueSend(g_commandQueue, &pCmd, pdMS_TO_TICKS(5000));
           }
      }
     
    }
  else
    {
        if(receivedString.indexOf("MQTT_CONNECT")>-1)
        {
           Mqtt_Connect_Status=true;
           digitalWrite(mqtt_Led,HIGH);
           //mqttConnectedEvent(false);
        }
          
        else if(receivedString.indexOf("MQTT_DISCONNECT")>-1) 
        {
           
           digitalWrite(mqtt_Led,LOW);
           if(Mqtt_Connect_Status)
           lastLostMqtt= millis();
           Mqtt_Connect_Status=false;
        }
          
    }

    //if(strstr(RxBuffer, "nodeId") != NULL)

   // else if (strstr(RxBuffer, "+CME ERROR: 767") != NULL)
     //     connectMQTTServer();
  }

  return receivedString;
}
//-------------------------------------------------------------------------
void clrRxBuffer(void) {
   while(FOUR_G_SERIAL.available())FOUR_G_SERIAL.read();
// 	memset(RxBuffer, 0, RxBufferLength);      //清空
// 	BufferCount = 0;
 }
//-------------------------------------------------------------------------
bool enterFourGAtMode() {
  clrRxBuffer();
  delay(1100);
  FOUR_G_SERIAL.print("+++");
  delay(1100);
  clrRxBuffer();

  return sendCommand("AT\r\n", "OK", 1000, 3) == Success;
}
//-------------------------------------------------------------------------
unsigned int sendCommand(const char *Command, const char *Response, unsigned long Timeout, unsigned char Retry){
	//clrRxBuffer();
  if (!takeFourGSerial((Timeout * Retry) + 1000)) {
    return Failure;
  }
  String RxBuffer="";
	for (unsigned char n = 0; n < Retry; n++){
		debugPrint("send AT Command:");
		// debugPrint.println(Command);
		// FOUR_G_SERIAL.print(Command);
    clrRxBuffer();
    debugPrint(Command);
		FOUR_G_SERIAL.write(Command);
		unsigned long Time_Cont = 0;
		while (Time_Cont++ < Timeout)
		{
      delay(1);
			RxBuffer=ReadBuffer();
			// debugPrint.print(".");	//1.8.x的IDE不加这个会有bug
			//if (strstr(RxBuffer, Response) != NULL)
      if(RxBuffer.indexOf(Response)>-1)
      {
				debugPrint("receive AT Command:");
				debugPrint(RxBuffer); //输出接收到的信息
				//clrRxBuffer();
        releaseFourGSerial();
				return Success;
			}
			
		}
    debugPrint("AT Command Timeout");
		Time_Cont = 0;
	}
  // 输出重试次数用尽信息
  debugPrintln("AT Command Retry Exceeded");
	debugPrint("receive AT Command:");
	debugPrintln(RxBuffer);//输出接收到的信息
	//clrRxBuffer();
  releaseFourGSerial();
	return Failure;
}
//-------------------------------------------------------------------------

//-------------------------------------------------------------------------

void subscribeTopic(String topic) {
  char send_buf[100] = {0};
  memset(send_buf, 0, 100); // 清空缓冲区

  // 构建 AT 指令字符串
  sprintf(send_buf, "AT+MQSUB=\"1\",\"2\",\"4\",\"%s\"\r\n", topic.c_str());

  // 发送 AT 指令并等待响应
  if (sendCommand(send_buf, "OK\r\n", 3000, 10) == Success) {
    debugPrintln("Subscribe topic successfully!");
  } else {
    debugPrintln("Failed to subscribe topic!");
  }
}

//-------------------------------------------------------------------------

bool disableMqttAutoPackets() {
  const int bufferSize = 150;
  char send_buf[bufferSize];

  memset(send_buf, 0, bufferSize);
  sprintf(send_buf, "AT+REGSET=\"0\",\"0\"\r\n");
  if (sendCommand(send_buf, "OK\r\n", 10000, 5) != Success)
    return false;

  memset(send_buf, 0, bufferSize);
  sprintf(send_buf, "AT+HEARTSET=\"0\",\"5\",\"600000\",\"48\"\r\n");
  if (sendCommand(send_buf, "OK\r\n", 10000, 5) != Success)
    return false;

  return true;
}

//-------------------------------------------------------------------------

bool publishDataToServer(String msg) {
   msg.trim();
   if (msg.length() == 0) {
    debugPrintln(F("Skip empty MQTT publish"));
    return false;
   }
   if (!takeFourGSerial(1000)) {
    return false;
   }
   unsigned long now = millis();
   if (g_lastMqttPublishMs > 0 && now - g_lastMqttPublishMs < 180) {
    vTaskDelay(pdMS_TO_TICKS(180 - (now - g_lastMqttPublishMs)));
   }
   FOUR_G_SERIAL.println(msg);
   FOUR_G_SERIAL.flush();
   g_lastMqttPublishMs = millis();
   vTaskDelay(pdMS_TO_TICKS(180));
   releaseFourGSerial();
   return true;
}

//-------------------------------------------------------------------------
bool initMQTT(String nodeId,String mqttTopic,String subTopic)
{
  debugPrintln("InitMQTTServer......");
    const int bufferSize = 150;
    char send_buf[bufferSize];
    // 清空缓冲区
    memset(send_buf, 0, bufferSize);
        // 格式化字符串
    sprintf(send_buf, "AT+DTUTASK=\"1\",\"20\"\r\n");
     if (sendCommand(send_buf, "OK\r\n", 10000, 1) != Success) 
        return false;
     memset(send_buf, 0, bufferSize);
    // 格式化字符串
    sprintf(send_buf, "AT+MQTTIP=\"%s\",\"%u\"\r\n", MyGlobal.mqttServer, MyGlobal.port);
    // 发送命令并检查结果
    if (sendCommand(send_buf, "OK\r\n", 10000, 5) != Success) 
        return false;
   memset(send_buf, 0, bufferSize);
    // 格式化字符串
    sprintf(send_buf, "AT+MQTT=\"%s\",\"%s\",\"%s\"\r\n",nodeId.c_str(), MyGlobal.userName, MyGlobal.Psw);
    // 发送命令并检查结果
    if (sendCommand(send_buf, "OK\r\n", 10000, 5) != Success) 
        return false; 
 
     memset(send_buf, 0, bufferSize);
    // 格式化字符串 AT+MQPUB="1","0","4","C2S/1143445796"   sprintf(send_buf, "AT+MQSUB=1,0,4,\"%s\"\r\n", topic.c_str());
    sprintf(send_buf, "AT+MQPUB=\"1\",\"2\",\"4\",\"%s\"\r\n",mqttTopic.c_str());
    // 发送命令并检查结果
    if (sendCommand(send_buf, "OK\r\n", 10000, 5) != Success) 
        return false;

  subscribeTopic(subTopic);
  memset(send_buf, 0, bufferSize);
    // 格式化字符串
    sprintf(send_buf, "AT+REGSET=\"0\",\"0\"\r\n");
    // 发送命令并检查结果
    if (sendCommand(send_buf, "OK\r\n", 10000, 5) != Success) 
        return false;

  memset(send_buf, 0, bufferSize);
    // 格式化字符串
    sprintf(send_buf, "AT+HEARTSET=\"0\",\"5\",\"600000\",\"48\"\r\n");
    // 发送命令并检查结果
    if (sendCommand(send_buf, "OK\r\n", 10000, 5) != Success) 
         return false;
  memset(send_buf, 0, bufferSize);
    // 格式化字符串
    sprintf(send_buf, "AT+REST\r\n");
    // 发送命令并检查结果
    if (sendCommand(send_buf, "OK\r\n", 10000, 5) != Success) 
          return false;
    else
         return true;
}
//-------------------------------------------------------------------------
bool isMqttConRight()
{

  char copy[30]; // 注意加1为了存储空终止符
  strcpy(copy, MyNode.nodeId.c_str());

  const int bufferSize = 150;
  char send_buf[bufferSize];
    memset(send_buf, 0, bufferSize);
    sprintf(send_buf, "AT+MQTT?\r\n");
    // 发送命令并检查结果
    if (sendCommand(send_buf, copy, 10000, 3) != Success) 
        return false;
    else
       return true;

}

bool isMqttConnect()
{
  const uint32_t timeoutMs = 3000;
  const char *command = "AT+DTUCON?\r\n";

  if (!takeFourGSerial(timeoutMs + 1000)) {
    return false;
  }

  clrRxBuffer();
  debugPrint("send AT Command:");
  debugPrint(command);
  FOUR_G_SERIAL.write(command);

  uint32_t startMs = millis();
  while (millis() - startMs < timeoutMs) {
    String response = ReadBuffer();
    if (response.indexOf("+DTUCON=\"1\"") >= 0) {
      debugPrintln(F("[4G] DTU is connected"));
      releaseFourGSerial();
      return true;
    }
    if (response.indexOf("+DTUCON=\"0\"") >= 0) {
      debugPrintln(F("[4G] DTU is disconnected"));
      releaseFourGSerial();
      return false;
    }
    delay(1);
  }

  debugPrintln(F("[4G] DTU status query timeout"));
  releaseFourGSerial();
  return false;
}

