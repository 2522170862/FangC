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
