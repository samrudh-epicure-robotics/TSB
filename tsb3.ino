//tsbv2
#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <PubSubClient.h>
#include <HardwareSerial.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <SoftwareSerial.h>


// Pin definitions for DS18B20 temperature sensors
#define ONE_WIRE_BUS1 26 // First sensor (hotness)
#define ONE_WIRE_BUS2 27 // second sensor (vend_temp)
#define ONE_WIRE_BUS3 18 // third sensor (cleaning_temp)

OneWire oneWire1(ONE_WIRE_BUS1);
DallasTemperature sensors1(&oneWire1); // First sensor
OneWire oneWire2(ONE_WIRE_BUS2);
DallasTemperature sensors2(&oneWire2); // Second sensor
OneWire oneWire3(ONE_WIRE_BUS3);
DallasTemperature sensors3(&oneWire3); // Third sensor

HardwareSerial mySerial(1); // Using UART1 on ESP32

//barcodecup
SoftwareSerial scannerSerial(35, 33); // TX=35, RX=33
const int scanSeconds = 1500;           // Scan timeout in seconds
int peel_not_detected=0;

//temperature_sensor
int optimal_temp = 0;
int temp_target = 80;
int temp_cutoff = 70;
int temp_monitor = 0;
unsigned long last_temp_error_time = 0;
unsigned long last_mqtt_time = 0;
bool temp_error_flag = false;


int optimal_temp2 = 0;
int temp_target2 = 75;
int temp_cutoff2 = 60;
int temp_monitor2 = 0;
unsigned long last_temp_error_time2 = 0;
unsigned long last_mqtt_time2 = 0;
bool temp_error_flag2 = false;
int coil_status=0;
int coil1_status=0;
int recipie_started=1;



// WiFi credentials
const char *ssid = "Epicure_2.4G";             
const char *password = "epicure2022";   
//const char *ssid = "Roachicon";             
//const char *password = "Roach@123$";  


// MQTT Broker settingsf
//const char *machine_id = "68ea96604fb45c0fc7074c73"; //1
const char *machine_id = "69535d8b0e9374c55fb5e943"; //2
const char *mqtt_broker = "jedfe9bd.ala.asia-southeast1.emqxsl.com";
char mqtt_topic[64];
char mqtt_tab_fb_topic[64];
char mqtt_temp_fb_topic[64];

const char *mqtt_username = "smoothiebar_server";
const char *mqtt_password = "smoothiebar@2025";
const int mqtt_port = 8883;

//int temp_monitor=0;

// WiFi and MQTT client initialization
WiFiClientSecure esp_client;
PubSubClient mqtt_client(esp_client);

const char *ca_cert = R"EOF(
-----BEGIN CERTIFICATE-----
MIIDjjCCAnagAwIBAgIQAzrx5qcRqaC7KGSxHQn65TANBgkqhkiG9w0BAQsFADBh
MQswCQYDVQQGEwJVUzEVMBMGA1UEChMMRGlnaUNlcnQgSW5jMRkwFwYDVQQLExB3
d3cuZGlnaWNlcnQuY29tMSAwHgYDVQQDExdEaWdpQ2VydCBHbG9iYWwgUm9vdCBH
MjAeFw0xMzA4MDExMjAwMDBaFw0zODAxMTUxMjAwMDBaMGExCzAJBgNVBAYTAlVT
MRUwEwYDVQQKEwxEaWdpQ2VydCBJbmMxGTAXBgNVBAsTEHd3dy5kaWdpY2VydC5j
b20xIDAeBgNVBAMTF0RpZ2lDZXJ0IEdsb2JhbCBSb290IEcyMIIBIjANBgkqhkiG
9w0BAQEFAAOCAQ8AMIIBCgKCAQEAuzfNNNx7a8myaJCtSnX/RrohCgiN9RlUyfuI
2/Ou8jqJkTx65qsGGmvPrC3oXgkkRLpimn7Wo6h+4FR1IAWsULecYxpsMNzaHxmx
1x7e/dfgy5SDN67sH0NO3Xss0r0upS/kqbitOtSZpLYl6ZtrAGCSYP9PIUkY92eQ
q2EGnI/yuum06ZIya7XzV+hdG82MHauVBJVJ8zUtluNJbd134/tJS7SsVQepj5Wz
tCO7TG1F8PapspUwtP1MVYwnSlcUfIKdzXOS0xZKBgyMUNGPHgm+F6HmIcr9g+UQ
vIOlCsRnKPZzFBQ9RnbDhxSJITRNrw9FDKZJobq7nMWxM4MphQIDAQABo0IwQDAP
BgNVHRMBAf8EBTADAQH/MA4GA1UdDwEB/wQEAwIBhjAdBgNVHQ4EFgQUTiJUIBiV
5uNu5g/6+rkS7QYXjzkwDQYJKoZIhvcNAQELBQADggEBAGBnKJRvDkhj6zHd6mcY
1Yl9PMWLSn/pvtsrF9+wX3N3KjITOYFnQoQj8kVnNeyIv/iPsGEMNKSuIEyExtv4
NeF22d+mQrvHRAiGfzZ0JFrabA0UWTW98kndth/Jsw1HKj2ZL7tcu7XUIOGZX1NG
Fdtom/DzMNU+MeKNhJ7jitralj41E6Vf8PlwUHBHQRFXGU7Aj64GxJUTFy8bJZ91
8rGOmaFvE7FBcf6IKshPECBV1/MUReXgRPTqh5Uykw7+U0b6LJ3/iyK5S9kJRaTe
pLiaWN0bfVKfjllDiIGknibVb63dDcY3fe0Dkhvld1927jyNxF1WW6LZZm6zNTfl
MrY=
-----END CERTIFICATE-----
)EOF";



uint8_t status_mqtt = 0;

#define MAX_UART_RETRY 12  // Maximum retry attempts for UART
#define TIMEOUT 2000       // Timeout in milliseconds

int check_trigger=0;

//barcode
void barcode_scan(){
   peel_not_detected=0;
    startScanSession(); // Everything runs inside this function
    delay(100);
    if(peel_not_detected==0){
      startScanSession();
    }
    if(peel_not_detected==0){
      Serial.println("cup_peeled");
         mqtt_client.publish(mqtt_tab_fb_topic, "cup_peeled");

    }
    else{
      Serial.println("cup_not_peeled");
               mqtt_client.publish(mqtt_tab_fb_topic, "cup_not_peeled");

      delay(500);
      send_uart("go_front");

    }
}


void temp_check(){
      mqtt_client.publish(mqtt_tab_fb_topic, "checking_temp");

   sensors1.requestTemperatures();
  float tempC = sensors1.getTempCByIndex(0);
  temp_error_flag = false;
  int i=0;
  while(i<5){
  if (tempC == -127.00){
    if(!temp_error_flag) {
    temp_monitor++;
    if(temp_monitor>3){
     Serial.println("stopping coil tmp error");
    Serial.println("Temperature sensor 1 error");
    delay(100);
    send_uart("coil-0");
    temp_error_flag= true;
    optimal_temp=0;
    
  }}}.
  else{
    temp_error_flag = false;
  }
  i++;
  }
  if(!temp_error_flag){ 
    temp_monitor=0;
    temp_error_flag=false;
  
  if(tempC >temp_cutoff+1 ) {
     send_uart("check_coil-0");
  }

else if(tempC<temp_cutoff){
   send_uart("check_coil-1");
}
     
}else{
  send_uart("check_coil-0");
}
}


// Configure scanner: command mode, infinite scan time, save to flash
void configureScanner() {
  // Read current value from address 0x0000
  byte readCmd[] = {0x7E, 0x00, 0x07, 0x01, 0x00, 0x00, 0x01, 0xAB, 0xCD};
  scannerSerial.write(readCmd, sizeof(readCmd));
  delay(100);
  
  String response = "";
  while (scannerSerial.available()) {
    response += (char)scannerSerial.read();
  }
  
  if (response.length() >= 5) {
    byte currentValue = (byte)response[4];
    byte newValue = (currentValue & 0xFC) | 0x01; // Command mode
    
    // Write new value to 0x0000
    byte writeModeCmd[] = {0x7E, 0x00, 0x08, 0x01, 0x00, 0x00, newValue, 0xAB, 0xCD};
    scannerSerial.write(writeModeCmd, sizeof(writeModeCmd));
    delay(100);
    while (scannerSerial.available()) scannerSerial.read(); // Flush ACK
  }
  
  // Set infinite scan time (0x00 to 0x0006)
  byte infiniteCmd[] = {0x7E, 0x00, 0x08, 0x01, 0x00, 0x06, 0x00, 0xAB, 0xCD};
  scannerSerial.write(infiniteCmd, sizeof(infiniteCmd));
  delay(100);
  while (scannerSerial.available()) scannerSerial.read();

  // Save to flash
  byte saveCmd[] = {0x7E, 0x00, 0x09, 0x01, 0x00, 0x00, 0x00, 0xDE, 0xC8};
  scannerSerial.write(saveCmd, sizeof(saveCmd));
  delay(100);
  while (scannerSerial.available()) scannerSerial.read();

  Serial.println("✅ Scanner configured: command mode + infinite scan time");
}

// Blocking scan session
void startScanSession() {
//  Serial.println("📡 Scanning for peel...");

  // Start scan command
  byte startScanCommand[] = {0x7E, 0x00, 0x08, 0x01, 0x00, 0x02, 0x01, 0xAB, 0xCD};
  scannerSerial.write(startScanCommand, sizeof(startScanCommand));
  delay(50);
  while (scannerSerial.available()) scannerSerial.read(); // flush

  unsigned long startTime = millis();
  String barcodeBuffer = "";

  while (millis() - startTime < scanSeconds ) {
    // Process incoming scanner data
    while (scannerSerial.available()) {
      char c = scannerSerial.read();
      if (c == '\r' || c == '\n') {
        if (barcodeBuffer.length() >= 6) {
//          Serial.print("📦 Complete Barcode: ");
//          Serial.println(barcodeBuffer);
//          Serial.println("✅ Peel Detected");
          peel_not_detected=1;
          stopScan();
          return; // Exit after successful scan
        }
        barcodeBuffer = "";
      } else if (isPrintable(c)) {
        barcodeBuffer += c;
      }
    }
  }

  // Timeout
//  Serial.println("❌ Peel Not Detected (Timeout)");
  stopScan();
}

// Stop scan command
void stopScan() {
  byte stopScanCommand[] = {0x7E, 0x00, 0x08, 0x01, 0x00, 0x02, 0x00, 0xAB, 0xCD};
  scannerSerial.write(stopScanCommand, sizeof(stopScanCommand));
//  Serial.println("🛑 Scanning stopped");
}

// Function to extract parameters from a command string
bool parseParams(String msg, int numParams, int params[]) {
  int startIndex = msg.indexOf('-') + 1;
  for (int i = 0; i < numParams; i++) {
    int nextDash = msg.indexOf('-', startIndex);
    if (nextDash == -1 && i < numParams - 1) {
      return false;
    }
    params[i] = msg.substring(startIndex, nextDash == -1 ? msg.length() : nextDash).toInt();
    startIndex = nextDash + 1;
  }
  return true;
}

// Send message over UART with retry logic
void send_uart(String receivedMsg) {
  receivedMsg.trim();  // normalize input
  for (int attempt = 0; attempt < MAX_UART_RETRY; attempt++) {
    Serial.println("Sending UART data: " + receivedMsg);
    mySerial.print(receivedMsg);
    mySerial.print('\n');

    unsigned long startTime = millis();
    while (millis() - startTime < TIMEOUT) {
      if (mySerial.available()) {
        String echoMsg = mySerial.readStringUntil('\n');
        echoMsg.trim();
        Serial.println("received from stm: " + echoMsg);
        if (echoMsg.equals(receivedMsg)) {
          Serial.println("sent msg to stm success: " + echoMsg);
          String success_msg = "Sent " + echoMsg+ " to stm successfully ";
          mqtt_client.publish(mqtt_tab_fb_topic, success_msg.c_str());
          return;
        }}
    }

    String feedback = "Retrying UART... Attempt " + String(attempt + 1)+ "to send "+receivedMsg;
    mqtt_client.publish(mqtt_tab_fb_topic, feedback.c_str());
    Serial.println(feedback);
  }

  Serial.println("UART error");
  mqtt_client.publish(mqtt_tab_fb_topic, "uart_error");
}

// Process incoming commands
void check_func(String msg) {
  check_trigger=1;
    if (msg.startsWith("hotness")) {
    sensors1.requestTemperatures();
    float tempC = sensors1.getTempCByIndex(0);
    if (tempC == -127.00) {
      Serial.println("Temperature sensor 1 error");
      mqtt_client.publish(mqtt_tab_fb_topic, "temp_1_error");
    } else {
      char tempString[10];
      dtostrf(tempC, 4, 2, tempString);
      mqtt_client.publish(mqtt_tab_fb_topic, tempString);
      Serial.println("Published temperature 1 (sensor 1): " + String(tempC));
    }
  }
   
  else  if (msg.startsWith("vend_temp")) {
    sensors2.requestTemperatures();
    float temp1C = sensors2.getTempCByIndex(0);
    if (temp1C == -127.00) {
      Serial.println("Temperature sensor 1 error");
      mqtt_client.publish(mqtt_tab_fb_topic, "temp_2_error");
    } else {
      char tempString1[10];
      dtostrf(temp1C, 4, 2, tempString1);
      mqtt_client.publish(mqtt_tab_fb_topic, tempString1);
      Serial.println("Published temperature 2 (sensor 2): " + String(temp1C));
    }
  }

  else  if (msg.startsWith("clean_temp")) {
    sensors3.requestTemperatures();
    float temp2C = sensors3.getTempCByIndex(0);
    if (temp2C == -127.00) {
      Serial.println("Clean Temperature sensor 2 error");
      mqtt_client.publish(mqtt_tab_fb_topic, "clean_temp_error");
    } else {
      char tempString2[10];
      dtostrf(temp2C, 4, 2, tempString2);
      mqtt_client.publish(mqtt_tab_fb_topic, tempString2);
      Serial.println("Published temperature 3 (clean temp sensor): " + String(temp2C));
    }
  }

  
    else if(msg.startsWith("coil_monitor")){
      temp_monitor=0;
    }

    else if(msg.startsWith("coil2_monitor")){
      temp_monitor2=0;
    }
    
     else if(msg.startsWith("coil_reset")){
            mqtt_client.publish(mqtt_tab_fb_topic, "resetting drinking coil");

      temp_monitor=0;
    }

    else if(msg.startsWith("clean_coil_reset")){
      mqtt_client.publish(mqtt_tab_fb_topic, "resetting cleaning coil");
      temp_monitor2=0;
    }
else if(msg.startsWith("menu_accessed")){
    send_uart("check_clean");
}
else if(msg.startsWith("check_payment")){
        recipie_started=1;

}




else if (msg.startsWith("trigger_temp")) {

        sensors1.requestTemperatures();
        sensors2.requestTemperatures();
        sensors3.requestTemperatures();

        float temp1 = sensors1.getTempCByIndex(0);
        float temp2 = sensors2.getTempCByIndex(0);
        float temp3 = sensors3.getTempCByIndex(0);

        String t1 = (temp1 == -127.00) ? "err" : String((int)temp1);
        if (temp1 == -127.00) Serial.println("Sensor 1 error");

        String t2 = (temp2 == -127.00) ? "err" : String((int)temp2);
        if (temp2 == -127.00) Serial.println("Sensor 2 error");


        String t3 = (temp3 == -127.00) ? "err" : String((int)temp3);
        if (temp3 == -127.00) Serial.println("Sensor 3 error");

        int t1Int = (temp1 == -127.00) ? -100 : (int)temp1;
        int t2Int = (temp2 == -127.00) ? -100 : (int)temp2;
        int t3Int = (temp3 == -127.00) ? -100 : (int)temp3;

//        String payload = "temp_log_coil&vm&cleaning-" + t1 + "-" + t2+"-"+t3;
//        mqtt_client.publish(mqtt_temp_fb_topic, payload.c_str());
        
        char tempBuf[64];
        snprintf(tempBuf, sizeof(tempBuf), "temp_log_coil&vm&cleaning-%d-%d-%d", t1Int, t2Int, t3Int);
        mqtt_client.publish(mqtt_temp_fb_topic, tempBuf);
//        Serial.println("Published: " + payload);

        
        recipie_started=0;
       if(temp1 != -127) {
    if(temp1 >= temp_target)  send_uart("coil-0");
    else if(temp1 <= temp_cutoff) send_uart("coil-1");
}else{
  send_uart("coil-0");
}
    if(temp3 != -127) {
    if(temp3 >= temp_target2)  send_uart("clean_coil-0");
    else if(temp3 <= temp_cutoff2) send_uart("clean_coil-1");
}else{
  send_uart("clean_coil-0");
}
     
}

else if (msg.startsWith("updatetemp")) {
        recipie_started=0;

        sensors1.requestTemperatures();
        sensors2.requestTemperatures();
        sensors3.requestTemperatures();

        float temp1 = sensors1.getTempCByIndex(0);
        float temp2 = sensors2.getTempCByIndex(0);
        float temp3 = sensors3.getTempCByIndex(0);

        String t1 = (temp1 == -127.00) ? "err" : String((int)temp1);
        if (temp1 == -127.00) Serial.println("Sensor 1 error");

        String t2 = (temp2 == -127.00) ? "err" : String((int)temp2);
        if (temp2 == -127.00) Serial.println("Sensor 2 error");


        String t3 = (temp3 == -127.00) ? "err" : String((int)temp3);
        if (temp3 == -127.00) Serial.println("Sensor 3 error");

        
        String payload = "temp_log_coil&vm&cleaning-" + t1 + "-" + t2+"-"+t3;
        mqtt_client.publish(mqtt_temp_fb_topic, payload.c_str());
        Serial.println("Published: " + payload);
        recipie_started=0;

        if(temp1!=-127){
        send_uart("coil-0");
        }

        if(temp1>87){
          send_uart("coil-0");
        }

        if(temp3!=-127){
             send_uart("clean_coil-0");
        }
      if(temp3>87){
//          recipie_started=0;
          send_uart("clean_coil-0");
        }
        
      }


      
 else if(msg.startsWith("barcode")){
        mqtt_client.publish(mqtt_tab_fb_topic, "starting_scan");

  barcode_scan();
 }
 else if(msg.startsWith("temp_check")){
  temp_check();
 }

else if(msg.startsWith("recipie_started")){
  recipie_started=1;
      mqtt_client.publish(mqtt_tab_fb_topic, "recipie_started");
 }
 else if(msg.startsWith("recipie_ended")){
  recipie_started=0;
        mqtt_client.publish(mqtt_tab_fb_topic, "recipie_ended");

 }
 
 else if(msg.startsWith("cup_picked_fb")){
   mqtt_client.publish(mqtt_tab_fb_topic, "cup_picked");
   send_uart("cup_picked");
   }

 else{
  check_trigger=2;
 }
msg="";

  }

void WiFiEvent(WiFiEvent_t event, WiFiEventInfo_t info) {
  if (event == ARDUINO_EVENT_WIFI_STA_DISCONNECTED) {
    Serial.printf("❌ Disconnected from WiFi. Reason: %d\n", info.wifi_sta_disconnected.reason);
  } else if (event == ARDUINO_EVENT_WIFI_STA_CONNECTED) {
    Serial.println("✅ WiFi connected to AP");
  } else if (event == ARDUINO_EVENT_WIFI_STA_GOT_IP) {
    Serial.print("🌐 Got IP: ");
    Serial.println(WiFi.localIP());
  }
}

void connectToWiFi() {
   WiFi.onEvent(WiFiEvent);  // Attach event handler
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  int cnt = 0;
  while (WiFi.status() != WL_CONNECTED) {
    vTaskDelay(500 / portTICK_PERIOD_MS);
    Serial.print(".");
    cnt++;
    
    if (cnt > 20) {
      Serial.println("\nFailed to connect to WiFi after maximum retries. Restarting ESP32...");
      ESP.restart();
    }
  }
  Serial.println("\nConnected to WiFi");
  mqtt_client.publish(mqtt_tab_fb_topic, "Wifi Connected");
}

void connectToMQTT() {

  mqtt_client.setServer(mqtt_broker, mqtt_port);
  mqtt_client.setCallback(mqttCallback);
  mqtt_client.setBufferSize(2048);   // increase buffer for TLS

  int retryCount = 0;
  const int maxRetries = 5;

  while (!mqtt_client.connected() && retryCount < maxRetries) {
    String client_id = "esp32-client-" + String(WiFi.macAddress());
    Serial.printf("Connecting to MQTT Broker as %s (Attempt %d/%d)...\n",
                  client_id.c_str(), retryCount + 1, maxRetries);
    if (mqtt_client.connect(client_id.c_str(), mqtt_username, mqtt_password)) {
      Serial.println("Connected to MQTT broker");
      mqtt_client.subscribe(mqtt_topic);
      if(status_mqtt==0){
      mqtt_client.publish(mqtt_tab_fb_topic, "Mqtt connected");}
        status_mqtt = 1;
      return;
    } else {
      retryCount++;
      Serial.printf("Failed to connect to MQTT broker, rc=%d\n", mqtt_client.state());
      if (retryCount < maxRetries) {
        Serial.println("Waiting 2 seconds before retry...");
        vTaskDelay(2000 / portTICK_PERIOD_MS);
      }
    }
  }
  if (retryCount >= maxRetries) {
    Serial.println("Failed to connect to MQTT after maximum retries. Restarting ESP32...");
    ESP.restart();
  }
}

void mqttCallback(char *topic, byte *payload, unsigned int length) {
  String receivedMsg;
  for (unsigned int i = 0; i < length; i++) {
    receivedMsg += (char)payload[i];
  }
  if (receivedMsg.length() > 2) {
    Serial.println("Message received on topic: " + String(topic));
    Serial.println("Message from MQTT: " + receivedMsg);
    String feedback = "Received: " + receivedMsg;
     if (!receivedMsg.startsWith("d-")) {
      if(receivedMsg.startsWith("check")){
      if(optimal_temp){
    mqtt_client.publish(mqtt_tab_fb_topic, "check");
    recipie_started=1;
    }
    else{
      mqtt_client.publish(mqtt_tab_fb_topic, "water_getting_heated");
      if(coil_status!=2){
        recipie_started=0;
        coil_status=0;
      }
    }}else{
          mqtt_client.publish(mqtt_tab_fb_topic, "check");
         delay(50);
         mqtt_client.publish(mqtt_tab_fb_topic, "check");

    }
    
    }
    if (!receivedMsg.startsWith("process")) {
      check_func(receivedMsg);
      if(check_trigger==2){
        send_uart(receivedMsg);
        check_trigger=0;
          }
  }}
}



void temperature_handler(){


  if(!recipie_started){
    
  sensors1.requestTemperatures();
  float tempC = sensors1.getTempCByIndex(0);
  if (tempC == -127.00){
    if(!temp_error_flag) {
    temp_monitor++;
    if(temp_monitor>5){
     Serial.println("stopping coil tmp error");
    Serial.println("Temperature sensor 1 error");
    delay(100);
    send_uart("coil-0");
    temp_error_flag= true;
    optimal_temp=0;
  }}}
  else{
    temp_monitor=0;
    temp_error_flag=false;
  
  if(tempC >=temp_target) {
       if(coil_status!=2){
     Serial.println("stopping coil");
     char msg[80];  // make sure the buffer is large enough
    snprintf(msg, sizeof(msg), "coil turned off, temp - %.2f", tempC);
    mqtt_client.publish(mqtt_tab_fb_topic, msg);
    mqtt_client.publish(mqtt_temp_fb_topic, msg);
     delay(100);
     send_uart("coil-0");
     coil_status=2;
    delay(100);
    temp_monitor=0;
   }
  }

else if(tempC<=temp_cutoff){
if(coil_status!=1){
    char msg[100];  // make sure the buffer is large enough
    snprintf(msg, sizeof(msg), "smoothie coil turned on, temp - %.2f", tempC);
    mqtt_client.publish(mqtt_tab_fb_topic, msg);
    mqtt_client.publish(mqtt_temp_fb_topic, msg);
    delay(500);
    send_uart("coil-1");
coil_status=1;
}}
//else{
//  if(coil_status!=3){
//    char msg[100];  // make sure the buffer is large enough
//    snprintf(msg, sizeof(msg), "smoothie coil is in proper temp - %.2f", tempC);
//    mqtt_client.publish(mqtt_tab_fb_topic, msg);
//    mqtt_client.publish(mqtt_temp_fb_topic, msg);
//    delay(500);
//    send_uart("coil-0");
//  }
//    coil_status=3;
//  }
}

if(tempC<temp_cutoff-2){
   optimal_temp=0;
}
else{
  optimal_temp=1;
}
  




sensors3.requestTemperatures();
  float tempC2 = sensors3.getTempCByIndex(0);
  if (tempC2 == -127.00){
    if(!temp_error_flag2) {
    temp_monitor2++;
    if(temp_monitor2>5){
     Serial.println("stopping clenaing coil tmp error");
    Serial.println("Temperature sensor 3 error");
    delay(100);
    send_uart("clean_coil-0");
    temp_error_flag2= true;
    optimal_temp2=0;
  }}}
  else{
    temp_monitor2=0;
    temp_error_flag2=false;
  
  if(tempC2 >temp_target2 ) {
    if(coil1_status!=2){

     Serial.println("stopping cleaning coil");
     char msg[100];  // make sure the buffer is large enough
    snprintf(msg, sizeof(msg), "cleaning coil turned off, temp - %.2f", tempC2);
    mqtt_client.publish(mqtt_tab_fb_topic, msg);
    mqtt_client.publish(mqtt_temp_fb_topic, msg);
     delay(100);
     send_uart("clean_coil-0");
    delay(100);
    temp_monitor2=0;
    coil1_status=2;
     }
  }
else if(tempC2<temp_cutoff2){
if(coil1_status!=1){

    char msg[100];  // make sure the buffer is large enough
    snprintf(msg, sizeof(msg), "cleaning coil turned on, temp - %.2f", tempC2);
    mqtt_client.publish(mqtt_tab_fb_topic, msg);
    mqtt_client.publish(mqtt_temp_fb_topic, msg);
    delay(500);
    send_uart("clean_coil-1");
   coil1_status=1;

}
}
//else{
//  if(coil1_status!=3){
//
//    char msg[100];  // make sure the buffer is large enough
//    snprintf(msg, sizeof(msg), "cleaning coil is in proper temp - %.2f", tempC2);
//    mqtt_client.publish(mqtt_tab_fb_topic, msg);
//    mqtt_client.publish(mqtt_temp_fb_topic, msg);
//    delay(500);
//    send_uart("clean_coil-0");
//   coil1_status=3;
//}}


if(tempC2<temp_cutoff2-2){
   optimal_temp2=0;
}
else{
  optimal_temp2=1;
}
  

  }

}}

void serial_handler(){
 if (Serial.available()) {
    String receivedMsg = Serial.readStringUntil('\n');
    if (receivedMsg.length() > 2) {
      Serial.println();
      Serial.println("Received from IDE: " + receivedMsg);
      check_func(receivedMsg);
      if(check_trigger==2){
        send_uart(receivedMsg);
        check_trigger=0;
      }
    }
  } 
}


void uart_handler(){
  if (mySerial.available()) {
    String c = mySerial.readStringUntil('\n');
    if (c.length() > 2) {
      Serial.println("Received from STM: " + c);
       check_func(c);
       if(check_trigger==2){
if (strncmp(c.c_str(), "fb:", 3) == 0) {
        mqtt_client.publish(mqtt_tab_fb_topic, c.substring(3).c_str());
               check_trigger=0;

       }
      }
    }
}}


void wifi_mqtt_check(){
    if (WiFi.status() != WL_CONNECTED) connectToWiFi();
  if (!mqtt_client.connected()) {
    Serial.println("MQTT connection lost, attempting to reconnect...");
    connectToMQTT();
   if (status_mqtt == 1) {
    mqtt_client.publish(mqtt_tab_fb_topic, "mqtt ReConnected");
  }}

  mqtt_client.loop();
}

void setup() {
  Serial.begin(9600);
  mySerial.begin(9600, SERIAL_8N1, 32, 15); 



  // Initialize MQTT topics
  snprintf(mqtt_topic, sizeof(mqtt_topic), "smoothiebar/%s/input", machine_id);
  snprintf(mqtt_tab_fb_topic, sizeof(mqtt_tab_fb_topic), "smoothiebar/%s/feedback", machine_id);
  snprintf(mqtt_temp_fb_topic, sizeof(mqtt_temp_fb_topic), "smoothiebar/%s/temp", machine_id);

  connectToWiFi();
esp_client.setCACert(ca_cert);
  
  connectToMQTT();

  // Initialize both temperature sensors
  sensors1.begin();
  sensors1.setResolution(9); // 9-bit resolution for sensor 1

    sensors2.begin();
  sensors2.setResolution(9); // 9-bit resolution for sensor 1

      sensors3.begin();
  sensors3.setResolution(9); // 9-bit resolution for sensor 1

  Serial.println("ESP32 MAC Address: " + WiFi.macAddress());

 scannerSerial.begin(9600);
  configureScanner();
  Serial.println("Barcode Scanner Ready");
      mqtt_client.publish(mqtt_tab_fb_topic, "void setup done");

}



void loop() {

  wifi_mqtt_check();

//  serial_handler();

  uart_handler();

  temperature_handler();
  
}