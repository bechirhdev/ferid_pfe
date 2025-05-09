#define TINY_GSM_MODEM_SIM800
#define TINY_GSM_RX_BUFFER 1024

#include <TinyGsmClient.h>
#include <ArduinoHttpClient.h>
#include <ArduinoJson.h>


#define SerialMon Serial
#define SerialAT Serial1

#define MODEM_RST             5
#define MODEM_PWRKEY          4
#define MODEM_POWER_ON       23
#define MODEM_TX             27
#define MODEM_RX             26
#define BLUE_LED             13

#define MOTOPOMPE_PIN        32
#define VANNE1_PIN           33
#define VANNE2_PIN           34


#define BAUD_RATE            115200
#define SSL_PORT             443

const char apn[] = "internet.tn";
const char FIREBASE_HOST[] = "testlilygo-default-rtdb.firebaseio.com";
const String FIREBASE_AUTH = "BDWr2Ie3r6dBvKFIG1tT8vzlaQgE20fHHw9B33MQ";

TinyGsm modem(SerialAT);
TinyGsmClientSecure gsmClient(modem);
HttpClient httpClient(gsmClient, FIREBASE_HOST, SSL_PORT);

DynamicJsonDocument systemData(512);
DynamicJsonDocument firebaseData(512);
DynamicJsonDocument changedData(512);

bool startModem() {
  if(modem.waitForNetwork(240000L)) {
    return true;
  }
  SerialMon.print("WAITING FOR NETWORK: ");
  modem.restart();
  if(modem.waitForNetwork(240000L)) {
    SerialMon.println("SUCCESS");
    return true;
  }
  SerialMon.println("FAILED");
  return false;
  
}

bool connectGPRS() {
  if(modem.isGprsConnected()) {
    return true;
  }
  SerialMon.print("GPRS CONNECTION: ");
  modem.gprsConnect(apn);
  if(modem.isGprsConnected()) {
    SerialMon.println("SUCCESS");
    digitalWrite(BLUE_LED, HIGH);
    return true;
  }
  SerialMon.println("FAILED");
  digitalWrite(BLUE_LED, LOW);
  return false;
}

bool connectFirebase() {
  if(httpClient.connected()) {
    return true;
  }
  SerialMon.print("FIREBASE CONNECTION: ");
  httpClient.stop();
  httpClient.connect(FIREBASE_HOST, SSL_PORT);
  if(httpClient.connected()) {
    SerialMon.println("SUCCESS");
    return true;
  }
  SerialMon.println("FAILED");
  return false;
}

int readTemp() {
  return 33;
}

int readHumidite() {
  return 28;
}

int readHumiditeSol() {
  return 80;
}


bool readSystemData() {
  systemData.clear();
  systemData["temperature"] = readTemp();
  systemData["humidite"] = readHumidite();
  systemData["humiditeSol"] = readHumiditeSol();

  return true;
}

bool readFirebaseData() {
    SerialMon.print("Read Firebase Data: ");
    httpClient.get("/data.json?auth=" + FIREBASE_AUTH);
    int statusCode = httpClient.responseStatusCode();
    if(statusCode == 200) {
        DeserializationError error = deserializeJson(firebaseData, httpClient.responseBody());
        if(error) {
            SerialMon.println("Error deserialize json");
            return false;
        }
        SerialMon.println("SUCCESS");
        return true;
    }
    SerialMon.println("FAILED");
    return false;
}

bool updateFirebaseData() {
    String patchData;
    serializeJson(changedData, patchData);
    SerialMon.print("Update Firebase Data: ");
    httpClient.setHttpResponseTimeout(10000);
    httpClient.beginRequest();
    httpClient.patch("/data.json");
    httpClient.sendHeader("Content-Type", "application/json");
    httpClient.sendHeader("Content-Length", patchData.length());
    httpClient.sendHeader("auth", FIREBASE_AUTH);
    httpClient.beginBody();
    httpClient.print(patchData);  
    httpClient.endRequest();
    int statusCode = httpClient.responseStatusCode();
    if(statusCode == 200) {
        SerialMon.println("SUCCESS");
        return true;
    }
    SerialMon.println("FAILED");
    return false;
}

bool checkChangedData() {
  int motopompeStatus = digitalRead(MOTOPOMPE_PIN);
  if(firebaseData["motopompe"] == true) {
    if(motopompeStatus == LOW) {
      SerialMon.print("Start motopompe");
      digitalWrite(MOTOPOMPE_PIN, HIGH);
    }
  }
  else {
    if(motopompeStatus == HIGH) {
      SerialMon.print("Stop motopompe");
      digitalWrite(MOTOPOMPE_PIN, LOW);
    }
  }

  int vanne1Status = digitalRead(VANNE1_PIN);
  if(firebaseData["vanne1"] == true) {
    if(vanne1Status == LOW) {
      SerialMon.print("Start vanne1");
      digitalWrite(VANNE1_PIN, HIGH);
    }
  }
  else {
    if(vanne1Status == HIGH) {
      SerialMon.print("Stop vanne1");
      digitalWrite(VANNE1_PIN, LOW);
    }
  }

  int vanne2Status = digitalRead(VANNE2_PIN);
  if(firebaseData["vanne2"] == true) {
    if(vanne2Status == LOW) {
      SerialMon.print("Start vanne2");
      digitalWrite(VANNE2_PIN, HIGH);
    }
  }
  else {
    if(vanne2Status == HIGH) {
      SerialMon.print("Stop vanne2");
      digitalWrite(VANNE2_PIN, LOW);
    }
  }

  bool sensorsChanged = false; 
  changedData.clear();
  if(systemData["temperature"] != firebaseData["temperature"]) {
    changedData["temperature"] = systemData["temperature"];
    sensorsChanged = true;
  }
  if(systemData["humidite"] != firebaseData["humidite"]) {
    changedData["humidite"] = systemData["humidite"];
    sensorsChanged = true;
  }
  if(systemData["humiditeSol"] != firebaseData["humiditeSol"]) {
    changedData["humiditeSol"] = systemData["humiditeSol"];
    sensorsChanged = true;
  }
  return sensorsChanged;
}

void setup() {

  pinMode(MODEM_RST, OUTPUT);
  digitalWrite(MODEM_RST, HIGH);
  
  pinMode(MODEM_PWRKEY, OUTPUT);
  pinMode(MODEM_POWER_ON, OUTPUT);
  
  digitalWrite(MODEM_POWER_ON, HIGH);

  digitalWrite(MODEM_PWRKEY, HIGH);
  delay(100);
  digitalWrite(MODEM_PWRKEY, LOW);
  delay(1000);
  digitalWrite(MODEM_PWRKEY, HIGH);

  pinMode(BLUE_LED, OUTPUT);
  digitalWrite(BLUE_LED, LOW);

  pinMode(MOTOPOMPE_PIN, OUTPUT);
  pinMode(VANNE1_PIN, OUTPUT);
  pinMode(VANNE2_PIN, OUTPUT);
  

  SerialMon.begin(BAUD_RATE);
  delay(10);

  SerialAT.begin(BAUD_RATE, SERIAL_8N1, MODEM_RX, MODEM_TX);
}

void loop() {
  while(!startModem()) {
    delay(15000);
  }
  
  while(!connectGPRS()) {
    delay(15000);
  }

  while(!connectFirebase()) {
    delay(15000);
  }

  if(readSystemData()) {
    while(!readFirebaseData()) {
      delay(1000);
    }
    if(checkChangedData()) {
      updateFirebaseData();
    }
  }
  delay(100);
}
