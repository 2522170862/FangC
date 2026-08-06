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

