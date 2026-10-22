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
