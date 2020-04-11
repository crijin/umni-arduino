// For storing configurations
#include "FS.h"
#include <ArduinoJson.h>

// Wifi Libraries
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <DNSServer.h>
#include <WiFiManager.h>

//For reading battery level
ADC_MODE(ADC_VCC);
float batteryLevel = 0;

#include "HX711.h"
const int LOADCELL_DOUT_PIN = 4;
const int LOADCELL_SCK_PIN = 5;
const int CALIBRATION_FACTOR = 160000;
const int SCALE_OFFSET = 510000;

char amazonAccountId[45] = "";
char deviceId[45] = "";
char deviceName[45] = "";
bool needsConfig = true; // If set to true, this is the Arduino's first run through and needs additional configuration


void saveToConfigFile(char amazonAccountId[45], String deviceId, char deviceName[45]) {
  Serial.println("saving config");
  DynamicJsonBuffer jsonBuffer;
  JsonObject& json = jsonBuffer.createObject();
  
  json["amazonAccountId"] = amazonAccountId;
  json["deviceId"] = deviceId;
  json["deviceName"] = deviceName;

  File configFile = SPIFFS.open("/config.json", "w");
  if (!configFile) {
    Serial.println("failed to open config file for writing");
  }
  json.printTo(Serial);
  json.printTo(configFile);
  configFile.close();
}


void readConfigFile(){
  Serial.println("mounting FS...");

  if (SPIFFS.begin()) {
    Serial.println("mounted file system");
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      Serial.println("reading config file");
      
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);
        configFile.readBytes(buf.get(), size);
        
        DynamicJsonBuffer jsonBuffer;
        JsonObject& json = jsonBuffer.parseObject(buf.get());
        json.printTo(Serial);
        
        if (json.success()) {
          Serial.println("\nparsed json");
          strcpy(amazonAccountId, json["amazonAccountId"]);
          strcpy(deviceId, json["deviceId"]);
          strcpy(deviceName, json["deviceName"]);

          needsConfig = false;
        } else {
          Serial.println("failed to load json config");
        }
        needsConfig = false;
        
      }
    } else {
      Serial.println("No json config file exists");
    }
  } else {
    Serial.println("failed to mount FS");
  }
  return;
}


String httpReq(String url, String body) {
  WiFiClient client;
  HTTPClient http;
  
  http.begin(client,url);
  http.addHeader("Content-Type", "application/json");
  
  int httpCode = http.POST(body);
  return http.getString();
}


void setup() {
  Serial.begin(9600);
  Serial.println("\nStarting");
  batteryLevel = ESP.getVcc();
  
// Start Scale
  HX711 scale;
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  scale.set_scale(CALIBRATION_FACTOR);
  scale.set_offset(SCALE_OFFSET);

  readConfigFile();

//Setup WiFi network access point or connect via saved credentials
  WiFiManager wifiManager;

  if (needsConfig == true){
    wifiManager.resetSettings();
  }

  WiFiManagerParameter custom_account_id("Your Umni Home Account ID","Umni Home ID",amazonAccountId,60);
  WiFiManagerParameter custom_device_name("Device Nickname","Device Nickname",deviceName,30);

  if (needsConfig){
    wifiManager.addParameter(&custom_account_id);
    wifiManager.addParameter(&custom_device_name);
  }

  wifiManager.autoConnect("UmniScale");

  Serial.println("");
  Serial.println("Wifi Connected");

  IPAddress ip = WiFi.localIP();
  Serial.println("IP address: " + ip);
  
  while (scale.is_ready() == false) {
    Serial.println("Scale Not Ready");
  }
  
  Serial.print("Reading: ");
  float weight =  scale.get_units(100);
  Serial.print(weight, 1);  

  
  if (WiFi.status() == WL_CONNECTED){
    
    if (needsConfig == true) {

    // Sends Device Config to Firebase
      strcpy(amazonAccountId, custom_account_id.getValue());
      strcpy(deviceName, custom_device_name.getValue());
      
      String requestBody = String("{\"id\":\"") + amazonAccountId + "\",\"deviceName\":\"" + deviceName + "\",\"weight\":\"" + weight + "\"}";
      strcpy(deviceId, httpReq("http://us-central1-umnihome.cloudfunctions.net/setup", requestBody).c_str());
       
      saveToConfigFile(amazonAccountId,deviceId,deviceName);
      needsConfig = false;
    }
  }
  
//Sends Weight Log to Firebase
  String requestBody = String("{\"id\":\"") + amazonAccountId + "\",\"device\":\"" + deviceId + "\",\"weight\":\"" + weight + "\",\"battery\":\"" + batteryLevel + "\"}";
  String res = httpReq("http://us-central1-umnihome.cloudfunctions.net/weigh", requestBody);     

}


void loop() {

  ESP.deepSleep(3.6e9); //70 Minutes, the max time
//  ESP.deepSleep(2e7); // 20 seconds, for testing
}
