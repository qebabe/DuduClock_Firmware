#line 1 "/Users/wangshixiao/Downloads/DuduClock_Firmware/net.cpp"
#include <WiFi.h>
#include <WebServer.h>
#include <NetworkClientSecure.h>
#include <ArduinoJson.h>
#include "ArduinoZlib.h"
#include "common.h"
#include "PreferencesUtil.h"
#include "tftUtil.h"
#include "task.h"
#include "arduino.h"
#include "DuduUtil.h"

char* stringToCharArray(String input) {
  // 创建一个新的 char 数组，长度 = 字符串长度 + 1（末尾 '\0'）
  char* tt = new char[input.length() + 1];
  // 使用 String 自带的 toCharArray 方法复制内容
  input.toCharArray(tt, input.length() + 1);
  return tt;
}
// 和风天气身份认证，需要替换成你们自己的
char PrivateKey[] = "MC4CAQAwBQYDK2VwBCIEIITdRIErG+dV4zUwzrNR3OtFg2PbimwiAGzNYwWpvXhL";   // 私钥
char PublicKey[] = "MCowBQYDK2VwAyEA5Yb5bKbINXHF4xs145CGiMaM1znmbv3lOX264bbAbig=";        // 公钥
String KeyID = "KDGWMJF5XW";                                                              // 凭据ID
String ProjectID = "2F2DGVW8QH";                                                          // 项目ID
String ApiHost = "my2x88mtw4.re.qweatherapi.com";                                         // API Host

void sendNTPpacket(IPAddress &address);
void startAP();
void startServer();
void scanWiFi();
void handleNotFound();
void handleRoot();
void handleConfigWifi();
void restartSystem(String msg, bool endTips);
String urlEncode(const String& text);
bool httpsGet(String url, const String &bearerToken, String &responseBody, int &httpCode);
bool decompressGzipBody(uint8_t *compressedBody, size_t compressedSize, String &responseBody);
void appendBytesToString(const uint8_t *buffer, size_t length, String &target);
String getPollutantValue(const JsonArray &pollutants, const char *code);

bool queryNowWeatherSuccess = false;
bool queryFutureWeatherSuccess = false;
bool queryAirSuccess = false;

// hefeng
char charPrivateKey[65];
char charPublicKey[61];
String publicKeyMm;
String privateKeyMm;
String keyID;
String apiHost;
String projectID;

// Wifi相关
String ssid;  //WIFI名称
String pass;  //WIFI密码
String city;  // 城市
String adm; // 上级城市区划
String location; // 城市ID
String latitude; // 纬度
String longitude; // 经度
String WifiNames; // 根据搜索到的wifi生成的option字符串
// SoftAP相关
const char *APssid = "DuduClock";
IPAddress staticIP(192, 168, 1, 1);
IPAddress gateway(192, 168, 1, 254);
IPAddress subnet(255, 255, 255, 0);
WebServer server(80);
// 查询天气超时时间(ms)
int queryTimeout = 6000;
// 是否是刚启动时查询天气
bool isStartQuery = true; 
String data = "";

// 开启SoftAP进行配网
void wifiConfigBySoftAP(){
  // 开启AP模式，如果开启失败，重启系统
  startAP();
  // 扫描WiFi,并将扫描到的WiFi组成option选项字符串
  scanWiFi();
  // 启动服务器
  startServer();
  // 显示配置网络页面
  tft.pushImage(0, 0, 240, 320, QRcode);
}

// 处理服务器请求
void doClient(){
  server.handleClient();
}
// 处理404情况的函数'handleNotFound'
void handleNotFound(){
  handleRoot();//访问不存在目录则返回配置页面
}
// 处理网站根目录的访问请求
void handleRoot(){
  server.send(200,"text/html", ROOT_HTML_PAGE1 + WifiNames + ROOT_HTML_PAGE2);
}
// 提交数据后的提示页面
void handleConfigWifi(){
  //判断是否有WiFi名称
  if (server.hasArg("ssid")){
    Serial.print("获得WiFi名称:");
    ssid = server.arg("ssid");
    Serial.println(ssid);
  }else{
    Serial.println("错误, 没有发现WiFi名称");
    server.send(200, "text/html", "<meta charset='UTF-8'>错误, 没有发现WiFi名称");
    return;
  }
  //判断是否有WiFi密码
  if (server.hasArg("pass")){
    Serial.print("获得WiFi密码:");
    pass = server.arg("pass");
    Serial.println(pass);
  }else{
    Serial.println("错误, 没有发现WiFi密码");
    server.send(200, "text/html", "<meta charset='UTF-8'>错误, 没有发现WiFi密码");
    return;
  }
  //判断是否有city名称
  if (server.hasArg("city")){
    Serial.print("获得城市:");
    city = server.arg("city");
    Serial.println(city);
  }else{
    Serial.println("错误, 没有发现城市名称");
    server.send(200, "text/html", "<meta charset='UTF-8'>错误, 没有发现城市名称");
    return;
  }
  if (server.hasArg("publicKey")){
    Serial.print("publicKey:");
    publicKeyMm = server.arg("publicKey");
    Serial.println(publicKeyMm);
  }else{
    Serial.println("错误, 没有发现publicKey");
    server.send(200, "text/html", "<meta charset='UTF-8'>错误, 没有发现publicKey");
    return;
  }
  if (server.hasArg("privateKey")){
    Serial.print("privateKeyMm:");
    privateKeyMm = server.arg("privateKey");
    Serial.println(privateKeyMm);
  }else{
    Serial.println("错误, 没有发现privateKey");
    server.send(200, "text/html", "<meta charset='UTF-8'>错误, 没有发现privateKey");
    return;
  }
  if (server.hasArg("keyID")){
    Serial.print("keyID:");
    keyID = server.arg("keyID");
    Serial.println(keyID);
  }else{
    Serial.println("错误, 没有发现keyID");
    server.send(200, "text/html", "<meta charset='UTF-8'>错误, 没有发现keyID");
    return;
  }
  if (server.hasArg("apiHost")){
    Serial.print("apiHost:");
    apiHost = server.arg("apiHost");
    Serial.println(apiHost);
  }else{
    Serial.println("错误, 没有发现apiHost");
    server.send(200, "text/html", "<meta charset='UTF-8'>错误, 没有发现apiHost称");
    return;
  }
  if (server.hasArg("projectID")){
    Serial.print("projectID:");
    projectID = server.arg("projectID");
    Serial.println(projectID);
  }else{
    Serial.println("错误, 没有发现projectID");
    server.send(200, "text/html", "<meta charset='UTF-8'>错误, 没有发现projectID");
    return;
  }

  Serial.print("获得上级区划:");
  adm = server.arg("adm");
  Serial.println(adm);
  // 将信息存入nvs中
  setWiFiCity();
  setHeFeng();
  // 获得了所需要的一切信息，给客户端回复
  server.send(200, "text/html", "<meta charset='UTF-8'><style type='text/css'>body {font-size: 2rem;}</style><br/><br/>WiFi: " + ssid + "<br/>密码: " + pass + "<br/>城市: " + city + "<br/>上级区划: " + adm + "<br/>已取得相关信息,正在尝试连接,请手动关闭此页面。");
  restartSystem("即将尝试连接", false);
}

// 连接WiFi
void connectWiFi(int timeOut_s){
  delay(1500); // 让“系统启动中”字样多显示一会
  drawText("正在连接网络...");
  int connectTime = 0; //用于连接计时，如果长时间连接不成功，复位设备
  pinMode(D4,OUTPUT);
  Serial.print("正在连接网络");
  Serial.println(ssid);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    digitalWrite(D4, !digitalRead(D4));
    delay(500);
    connectTime++;
    if (connectTime > 2 * timeOut_s){ //长时间连接不上，清除nvs中保存的网络数据，并重启系统
      Serial.println("网络连接失败,即将重新开始配置网络...");
      clearWiFiCity();
      restartSystem("网络连接失败", false);
    }
  }
  digitalWrite(D4, LOW); // 连接成功后，将D4指示灯熄灭
  Serial.println("网络连接成功");
  Serial.print("本地IP： ");
  Serial.println(WiFi.localIP());
}
// 检查WiFi连接状态，如果断开了，重新连接
void checkWiFiStatus(){
  if(WiFi.status() != WL_CONNECTED){ // 网络断开了，进行重连
    Serial.println("网络断开，即将重新连接...");
    WiFi.begin(ssid, pass);
  }
}
// 启动服务器
void startServer(){
  // 当浏览器请求服务器根目录(网站首页)时调用自定义函数handleRoot处理，设置主页回调函数，必须添加第二个参数HTTP_GET，否则无法强制门户
  server.on("/", HTTP_GET, handleRoot);
  // 当浏览器请求服务器/configwifi(表单字段)目录时调用自定义函数handleConfigWifi处理
  server.on("/configwifi", HTTP_POST, handleConfigWifi);
  // 当浏览器请求的网络资源无法在服务器找到时调用自定义函数handleNotFound处理   
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("服务器启动成功！");
}
// 开启AP模式，如果开启失败，重启系统
void startAP(){
  Serial.println("开启AP模式...");
  WiFi.enableAP(true); // 使能AP模式
  //传入参数静态IP地址,网关,掩码
  WiFi.softAPConfig(staticIP, gateway, subnet);
  if (!WiFi.softAP(APssid)) {
    Serial.println("AP模式启动失败");
    ESP.restart(); // Ap模式启动失败，重启系统
  }  
  Serial.println("AP模式启动成功");
  Serial.print("IP地址: ");
  Serial.println(WiFi.softAPIP());
}
// 扫描WiFi,并将扫描到的Wifi组成option选项字符串
void scanWiFi(){
  Serial.println("开始扫描WiFi");
  int n = WiFi.scanNetworks();
  if (n){
    Serial.print("扫描到");
    Serial.print(n);
    Serial.println("个WIFI");
    WifiNames = "";
    for (size_t i = 0; i < n; i++){
      int32_t rssi = WiFi.RSSI(i);
      String signalStrength;
      if(rssi >= -35){
        signalStrength = " (信号极强)";
      }else if(rssi >= -50){
        signalStrength = " (信号强)";
      }else if(rssi >= -70){
        signalStrength = " (信号中)";
      }else{
        signalStrength = " (信号弱)";
      }
      WifiNames += "<option value='" + WiFi.SSID(i) + "'>" + WiFi.SSID(i) + signalStrength + "</option>";
      // Serial.print("WiFi的名称(SSID):");
      // Serial.println(WiFi.SSID(i));
    }
  }else{
    Serial.println("没扫描到WIFI");
  }
}

// 查询城市id
void getCityID(){
  // 计算jwt
  Serial.println("----");
  Serial.println(city);
  Serial.println(charPrivateKey);
  Serial.println(charPublicKey);
  Serial.println(keyID);
  Serial.println(projectID);
  Serial.println("----");
  String jwt = generateJWT(charPrivateKey, charPublicKey, keyID, projectID);
  // Serial.println(jwt);
  bool flag = false; // 是否成功获取到城市id的标志
  String url = "https://" + apiHost + cityURL + "?location=" + urlEncode(city) + "&adm=" + urlEncode(adm);
  int httpCode = 0;
  Serial.println("正在获取城市id");
  if (httpsGet(url, jwt, data, httpCode) && httpCode == 200) {
    StaticJsonDocument<2048> doc; //声明一个静态JsonDocument对象
    DeserializationError error = deserializeJson(doc, data); //反序列化JSON数据
    if(!error){ //检查反序列化是否成功
      //读取json节点
      String code = doc["code"].as<const char*>();
      if(code.equals("200")){
        flag = true;
        // 多结果的情况下，取第一个
        city = doc["location"][0]["name"].as<const char*>();
        location = doc["location"][0]["id"].as<const char*>();
        latitude = doc["location"][0]["lat"].as<const char*>();
        longitude = doc["location"][0]["lon"].as<const char*>();
        Serial.println("城市id :" + location);
        // 将信息存入nvs中
        setWiFiCity();
      }
    }  
  }
  if(!flag){
    Serial.print("获取城市id错误：");
    Serial.println(httpCode);
    Serial.print("城市错误，即将重启系统");
    clearWiFiCity(); // 清除配置好的信息
    restartSystem("城市名称无效", false);
  }
  Serial.println("获取成功");
}

// 查询实时天气
void getNowWeather(){
  // 计算jwt
  String jwt = generateJWT(charPrivateKey, charPublicKey, keyID, projectID);
  data = "";
  queryNowWeatherSuccess = false; // 先置为false
  String url = "https://" + apiHost + nowURL + "?location=" + location;
  int httpCode = 0;
  // Serial.println(ESP.getFreeHeap());
  Serial.println("正在获取天气数据");
  if (httpsGet(url, jwt, data, httpCode) && httpCode == 200) {
    StaticJsonDocument<2048> doc; //声明一个静态JsonDocument对象
    DeserializationError error = deserializeJson(doc, data); //反序列化JSON数据
    if(!error){ //检查反序列化是否成功
      //读取json节点
      String code = doc["code"].as<const char*>();
      if(code.equals("200")){
        queryNowWeatherSuccess = true;       
        //读取json节点
        nowWeather.text = doc["now"]["text"].as<const char*>();
        nowWeather.icon = doc["now"]["icon"].as<int>();
        nowWeather.temp = doc["now"]["temp"].as<int>();
        String feelsLike = doc["now"]["feelsLike"]; // 体感温度
        nowWeather.feelsLike = "体感温度" + feelsLike + "℃";
        String windDir = doc["now"]["windDir"];
        String windScale = doc["now"]["windScale"];
        nowWeather.win = windDir + windScale + "级";
        nowWeather.humidity = doc["now"]["humidity"].as<int>();
        String vis = doc["now"]["vis"];
        nowWeather.vis = "能见度" + vis + " KM";
        Serial.println("获取成功");
      }
    }  
  }
  if(!queryNowWeatherSuccess){
    Serial.print("请求实时天气错误：");
    Serial.println(httpCode);
    if(httpCode == 401){
      clearWiFiCity();
      Serial.print("和风天气认证失败错误401: 和风天气数据错误, 请重置配置!");
      restartSystem("和风天气401错误", false);
    }
  }
}
// 查询空气质量
void getAir(){
  // 计算jwt
  String jwt = generateJWT(charPrivateKey, charPublicKey, keyID, projectID);
  data = "";
  queryAirSuccess = false; // 先置为false
  String url = "https://" + apiHost + "/airquality/v1/current/" + latitude + "/" + longitude;
  int httpCode = 0;
  Serial.println("正在获取空气质量数据");
  if (httpsGet(url, jwt, data, httpCode) && httpCode == 200) {
    StaticJsonDocument<2048> doc; //声明一个静态JsonDocument对象
    DeserializationError error = deserializeJson(doc, data); //反序列化JSON数据
    if(!error){ //检查反序列化是否成功
      JsonArray indexes = doc["indexes"].as<JsonArray>();
      JsonArray pollutants = doc["pollutants"].as<JsonArray>();
      if(!indexes.isNull() && !pollutants.isNull() && indexes.size() > 0){
        queryAirSuccess = true;
        JsonObject firstIndex = indexes[0];
        nowWeather.air = firstIndex["aqi"].as<int>();
        nowWeather.pm10 = getPollutantValue(pollutants, "pm10");
        nowWeather.pm2p5 = getPollutantValue(pollutants, "pm2p5");
        nowWeather.no2 = getPollutantValue(pollutants, "no2");
        nowWeather.so2 = getPollutantValue(pollutants, "so2");
        nowWeather.co = getPollutantValue(pollutants, "co");
        nowWeather.o3 = getPollutantValue(pollutants, "o3");
        Serial.println("获取成功");
      }
    }  
  } 
  if(!queryAirSuccess){
    Serial.print("请求空气质量错误：");
    Serial.println(httpCode);
  }
}
// 查询未来天气，经过实况天气一环，城市名称肯定是合法的，所以无需再检验
void getFutureWeather(){
  // 计算jwt
  String jwt = generateJWT(charPrivateKey, charPublicKey, keyID, projectID);
  data = "";
  queryFutureWeatherSuccess = false; // 先置为false
  String url = "https://" + apiHost + futureURL + "?location=" + location;
  int httpCode = 0;
  Serial.println("正在获取一周天气数据");
  if (httpsGet(url, jwt, data, httpCode) && httpCode == 200) {
    StaticJsonDocument<2048> doc; //声明一个静态JsonDocument对象
    DeserializationError error = deserializeJson(doc, data); //反序列化JSON数据
    if(!error){ //检查反序列化是否成功
      //读取json节点
      String code = doc["code"].as<const char*>();
      if(code.equals("200")){
        queryFutureWeatherSuccess = true;
        //读取json节点
        futureWeather.day0wea = doc["daily"][0]["textDay"].as<const char*>();
        futureWeather.day0wea_img = doc["daily"][0]["iconDay"].as<int>();
        futureWeather.day0date = doc["daily"][0]["fxDate"].as<const char*>();
        futureWeather.day0tem_day = doc["daily"][0]["tempMax"].as<int>();
        futureWeather.day0tem_night = doc["daily"][0]["tempMin"].as<int>();

        futureWeather.day1wea = doc["daily"][1]["textDay"].as<const char*>();
        futureWeather.day1wea_img = doc["daily"][1]["iconDay"].as<int>();
        futureWeather.day1date = doc["daily"][1]["fxDate"].as<const char*>();
        futureWeather.day1tem_day = doc["daily"][1]["tempMax"].as<int>();
        futureWeather.day1tem_night = doc["daily"][1]["tempMin"].as<int>();

        futureWeather.day2wea = doc["daily"][2]["textDay"].as<const char*>();
        futureWeather.day2wea_img = doc["daily"][2]["iconDay"].as<int>();
        futureWeather.day2date = doc["daily"][2]["fxDate"].as<const char*>();
        futureWeather.day2tem_day = doc["daily"][2]["tempMax"].as<int>();
        futureWeather.day2tem_night = doc["daily"][2]["tempMin"].as<int>();

        futureWeather.day3wea = doc["daily"][3]["textDay"].as<const char*>();
        futureWeather.day3wea_img = doc["daily"][3]["iconDay"].as<int>();
        futureWeather.day3date = doc["daily"][3]["fxDate"].as<const char*>();
        futureWeather.day3tem_day = doc["daily"][3]["tempMax"].as<int>();
        futureWeather.day3tem_night = doc["daily"][3]["tempMin"].as<int>();

        futureWeather.day4wea = doc["daily"][4]["textDay"].as<const char*>();
        futureWeather.day4wea_img = doc["daily"][4]["iconDay"].as<int>();
        futureWeather.day4date = doc["daily"][4]["fxDate"].as<const char*>();
        futureWeather.day4tem_day = doc["daily"][4]["tempMax"].as<int>();
        futureWeather.day4tem_night = doc["daily"][4]["tempMin"].as<int>();

        futureWeather.day5wea = doc["daily"][5]["textDay"].as<const char*>();
        futureWeather.day5wea_img = doc["daily"][5]["iconDay"].as<int>();
        futureWeather.day5date = doc["daily"][5]["fxDate"].as<const char*>();
        futureWeather.day5tem_day = doc["daily"][5]["tempMax"].as<int>();
        futureWeather.day5tem_night = doc["daily"][5]["tempMin"].as<int>();

        futureWeather.day6wea = doc["daily"][6]["textDay"].as<const char*>();
        futureWeather.day6wea_img = doc["daily"][6]["iconDay"].as<int>();
        futureWeather.day6date = doc["daily"][6]["fxDate"].as<const char*>();
        futureWeather.day6tem_day = doc["daily"][6]["tempMax"].as<int>();
        futureWeather.day6tem_night = doc["daily"][6]["tempMin"].as<int>();

        Serial.println("获取成功");
      }
    }  
  } 
  if(!queryFutureWeatherSuccess){
    Serial.print("请求一周天气错误：");
    Serial.println(httpCode);
  }
}

// 获取NTP并同步RTC时间
void getNTPTime(){
  // 8 * 3600 东八区时间修正
  // 使用夏令时 daylightOffset_sec 就填写3600，否则就填写0；
  Serial.println("NTP对时...");
  configTime( 8 * 3600, 0, NTP1, NTP2, NTP3);
}
// 重启系统
// bool endTips  是否需要把“同步天气数据”文字的定时器任务取消
void restartSystem(String msg, bool endTips){
  if(endTips){
    //结束循环显示提示文字的定时器
    timerEnd(timerShowTips);
  }
  reflashTFT();
  for(int i = 3; i > 0; i--){
    String text = "";
    text = text + i + "秒后系统重启";
    draw2LineText(msg,text);
    delay(1000);
  }
  ESP.restart();
}

bool httpsGet(String url, const String &bearerToken, String &responseBody, int &httpCode) {
  responseBody = "";
  httpCode = 0;

  if (!url.startsWith("https://")) {
    return false;
  }

  String work = url.substring(8);
  int pathIndex = work.indexOf('/');
  String host = pathIndex >= 0 ? work.substring(0, pathIndex) : work;
  String path = pathIndex >= 0 ? work.substring(pathIndex) : "/";
  int port = 443;

  int colonIndex = host.indexOf(':');
  if (colonIndex >= 0) {
    port = host.substring(colonIndex + 1).toInt();
    host = host.substring(0, colonIndex);
  }

  NetworkClientSecure client;
  client.setInsecure();
  client.setTimeout(queryTimeout / 1000);
  if (!client.connect(host.c_str(), port)) {
    return false;
  }

  client.print(String("GET ") + path + " HTTP/1.1\r\n");
  client.print(String("Host: ") + host + "\r\n");
  client.print("User-Agent: DuduClock/2.1\r\n");
  client.print("Accept: application/json\r\n");
  client.print("Accept-Encoding: gzip\r\n");
  if (bearerToken.length() > 0) {
    client.print(String("Authorization: Bearer ") + bearerToken + "\r\n");
  }
  client.print("Connection: close\r\n\r\n");

  unsigned long start = millis();
  while (client.connected() && !client.available()) {
    if (millis() - start > (unsigned long)queryTimeout) {
      client.stop();
      return false;
    }
    delay(1);
  }

  String statusLine = client.readStringUntil('\n');
  statusLine.trim();
  if (statusLine.startsWith("HTTP/1.1 ") || statusLine.startsWith("HTTP/1.0 ")) {
    httpCode = statusLine.substring(9, 12).toInt();
  }

  bool isGzip = false;
  int contentLength = -1;
  while (client.connected()) {
    String line = client.readStringUntil('\n');
    line.trim();
    if (line.length() == 0) {
      break;
    }

    String lowerLine = line;
    lowerLine.toLowerCase();
    if (lowerLine.startsWith("content-encoding:") && lowerLine.indexOf("gzip") >= 0) {
      isGzip = true;
    } else if (lowerLine.startsWith("content-length:")) {
      contentLength = line.substring(line.indexOf(':') + 1).toInt();
    }
  }

  size_t capacity = contentLength > 0 ? (size_t)contentLength : 1024;
  uint8_t *rawBody = (uint8_t *)malloc(capacity);
  if (rawBody == NULL) {
    client.stop();
    return false;
  }

  size_t rawLength = 0;
  while (client.connected() || client.available()) {
    while (client.available()) {
      if (rawLength >= capacity) {
        size_t nextCapacity = capacity * 2;
        uint8_t *nextBuffer = (uint8_t *)realloc(rawBody, nextCapacity);
        if (nextBuffer == NULL) {
          free(rawBody);
          client.stop();
          return false;
        }
        rawBody = nextBuffer;
        capacity = nextCapacity;
      }
      rawBody[rawLength++] = (uint8_t)client.read();
    }
    delay(1);
  }
  client.stop();

  bool success = false;
  if (isGzip) {
    success = decompressGzipBody(rawBody, rawLength, responseBody);
  } else {
    appendBytesToString(rawBody, rawLength, responseBody);
    success = true;
  }

  free(rawBody);
  return success;
}

bool decompressGzipBody(uint8_t *compressedBody, size_t compressedSize, String &responseBody) {
  if (compressedBody == NULL || compressedSize == 0) {
    return false;
  }

  size_t outputCapacity = 4096;
  while (outputCapacity <= 32768) {
    uint8_t *decompressedBody = (uint8_t *)malloc(outputCapacity);
    if (decompressedBody == NULL) {
      return false;
    }

    uint32_t decompressedSize = 0;
    int result = ArduinoZlib::libmpq__decompress_zlib(
      compressedBody,
      (uint32_t)compressedSize,
      decompressedBody,
      (uint32_t)outputCapacity,
      decompressedSize
    );

    if (result >= 0 && decompressedSize > 0) {
      appendBytesToString(decompressedBody, decompressedSize, responseBody);
      free(decompressedBody);
      return true;
    }

    free(decompressedBody);
    if (result != Z_BUF_ERROR) {
      return false;
    }
    outputCapacity *= 2;
  }

  return false;
}

void appendBytesToString(const uint8_t *buffer, size_t length, String &target) {
  target.reserve(target.length() + length);
  for (size_t i = 0; i < length; i++) {
    target += (char)buffer[i];
  }
}

String getPollutantValue(const JsonArray &pollutants, const char *code) {
  if (pollutants.isNull()) {
    return "--";
  }

  for (JsonVariant pollutant : pollutants) {
    JsonObject obj = pollutant.as<JsonObject>();
    const char *pollutantCode = obj["code"];
    if (pollutantCode != NULL && strcmp(pollutantCode, code) == 0) {
      JsonVariant value = obj["concentration"]["value"];
      if (value.is<float>() || value.is<double>()) {
        return String(value.as<float>(), 1);
      }
      if (value.is<int>()) {
        return String(value.as<int>());
      }
      if (value.is<const char*>()) {
        return String(value.as<const char*>());
      }
    }
  }

  return "--";
}

String urlEncode(const String& text) {
  String encodedText = "";
  for (size_t i = 0; i < text.length(); i++) {
    char c = text[i];
    if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '-' || c == '_' || c == '.' || c == '~') {
      encodedText += c;
    } else if (c == ' ') {
      encodedText += '+';
    } else {
      encodedText += '%';
      char hex[4];
      sprintf(hex, "%02X", (uint8_t)c);
      encodedText += hex;
    }
  }
  return encodedText;
}



