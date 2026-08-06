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
