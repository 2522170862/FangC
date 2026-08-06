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
String GetTime()  //获取当前时间
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
