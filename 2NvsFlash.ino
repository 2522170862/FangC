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



