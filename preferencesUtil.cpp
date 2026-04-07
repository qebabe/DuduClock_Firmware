#include <Preferences.h>
#include "net.h"
#include "tftUtil.h"

Preferences prefs; // 声明Preferences对象

// 读取Wifi账号、密码和城市名称
void getWiFiCity(){
  prefs.begin("clock");
  ssid = prefs.getString("ssid", "");
  pass = prefs.getString("pass", "");
  city = prefs.getString("city", "");
  adm = prefs.getString("adm", "");
  location = prefs.getString("location", "");
  latitude = prefs.getString("latitude", "");
  longitude = prefs.getString("longitude", "");

  publicKeyMm = prefs.getString("publicKeyMm", PublicKey);
  if(publicKeyMm.length() == 0){
    publicKeyMm = PublicKey;
  }
  publicKeyMm.toCharArray(charPublicKey, 61);
  Serial.println("get:" + publicKeyMm);
  privateKeyMm = prefs.getString("privateKeyMm", PrivateKey);
  if(privateKeyMm.length() == 0){
    privateKeyMm = PrivateKey;
  }
  privateKeyMm.toCharArray(charPrivateKey, 65);
  Serial.println("get:" + privateKeyMm);
  keyID = prefs.getString("keyID", KeyID);
  if(keyID.length() == 0){
    keyID = KeyID;
  }
  Serial.println("get:" + keyID);
  apiHost = prefs.getString("apiHost", ApiHost);
  if(apiHost.length() == 0){
    apiHost = ApiHost;
  }
  Serial.println("get:" + apiHost);
  projectID = prefs.getString("projectID", ProjectID);
  if(projectID.length() == 0){
    projectID = ProjectID;
  }
  Serial.println("get:" + projectID);

  prefs.end();
}

// 写入Wifi账号、密码和城市名称
void setWiFiCity(){
  prefs.begin("clock");
  prefs.putString("ssid", ssid);
  prefs.putString("pass", pass);
  prefs.putString("city", city);
  prefs.putString("adm", adm);
  prefs.putString("location", location);
  prefs.putString("latitude", latitude);
  prefs.putString("longitude", longitude);
  prefs.end();
}
void setHeFeng(){
  prefs.begin("clock");
  prefs.putString("publicKeyMm", publicKeyMm);
  prefs.putString("privateKeyMm", privateKeyMm);
  prefs.putString("keyID", keyID);
  prefs.putString("apiHost", apiHost);
  prefs.putString("projectID", projectID);
  prefs.end();
}

// 清除Wifi账号、密码和城市相关信息
void clearWiFiCity(){
  prefs.begin("clock");
  prefs.remove("ssid");
  prefs.remove("pass");
  prefs.remove("city");
  prefs.remove("adm");
  prefs.remove("location");
  prefs.remove("latitude");
  prefs.remove("longitude");

  prefs.remove("publicKeyMm");
  prefs.remove("privateKeyMm");
  prefs.remove("keyID");
  prefs.remove("apiHost");
  prefs.remove("projectID");

  prefs.remove("backColor");
  prefs.end();
}

// 获取屏幕背光颜色
void getBackColor(){
  prefs.begin("clock");
  backColor = prefs.getInt("backColor",BACK_BLACK);
  prefs.end();
}

// 设置屏幕背光颜色
void setBackColor(int backColor){
  prefs.begin("clock");
  prefs.putInt("backColor",backColor);
  prefs.end();
}

// 测试用，在读取NVS之前，先写入自己的Wifi信息，免得每次浪费时间再配网
void setInfo4Test(){
  prefs.begin("clock");
  prefs.putString("ssid", "yunjiu");
  prefs.putString("pass", "610610610");
  prefs.putString("city", "烟台");
  prefs.putString("adm", "");
  prefs.putString("location", "101120501");
  prefs.putString("latitude", "37.46");
  prefs.putString("longitude", "121.45");
  prefs.putString("publicKeyMm", PublicKey);
  prefs.putString("privateKeyMm", PrivateKey);
  prefs.putString("keyID", KeyID);
  prefs.putString("apiHost", ApiHost);
  prefs.putString("projectID", ProjectID);
  prefs.end();
}
