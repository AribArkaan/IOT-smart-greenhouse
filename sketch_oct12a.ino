#include <WiFi.h>
#include <PubSubClient.h>
#include <HTTPClient.h>

const int LDR_PIN = 35;    // Pin untuk sensor LDR
const int SOIL_PIN = 34;   // Pin untuk sensor soil
const int RELAY_SOIL_PIN = 12; // Pin untuk relay soil
const int RELAY_LAMP_PIN = 15; // Pin untuk relay lampu

const int SOIL_THRESHOLD_PERCENT = 71; // Ambang batas untuk kelembaban tanah (dalam persentase)
const int LDR_THRESHOLD_PERCENT = 49;  // Ambang batas untuk sensor LDR (dalam persentase)

WiFiClient wifiClient;
PubSubClient mqttClient(wifiClient);

const char* ssid = "Frost";
const char* password = "hahahahihihi";

const char* mqttServer = "broker.hivemq.com";
const int mqttPort = 1883;
String serverName = "https://api.thingspeak.com/update?api_key=T51F2CLSXBS296IV";
const char *mqtt_clientId = "ESP32Client";


void reconnect() {
  while (!mqttClient.connected()) {
    Serial.println("Connecting to MQTT Broker...");
    String clientId = "ESP32Client-";
    clientId += String(random(0xffff), HEX);

    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("Connected.");
    } else {
      Serial.print("Failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Retrying in 5 seconds...");
      delay(5000);
    }
  }
}

void setupMQTT() {
  mqttClient.setServer(mqttServer, mqttPort);
}

void setupRelays() {
  pinMode(RELAY_SOIL_PIN, OUTPUT);
  pinMode(RELAY_LAMP_PIN, OUTPUT);
  digitalWrite(RELAY_SOIL_PIN, LOW); // Matikan relay soil awalnya
  digitalWrite(RELAY_LAMP_PIN, LOW); // Matikan relay lampu awalnya
}

void controlRelays(int soilMoisturePercent, int ldrIntensityPercent) {
  if (soilMoisturePercent < SOIL_THRESHOLD_PERCENT) {
    // Hidupkan relay soil jika kelembaban tanah rendah
    digitalWrite(RELAY_SOIL_PIN, HIGH);
  } else {
    // Matikan relay soil jika kelembaban tanah mencukupi
    digitalWrite(RELAY_SOIL_PIN, LOW);
  }

  if (ldrIntensityPercent < LDR_THRESHOLD_PERCENT) {
    // Hidupkan relay lampu jika intensitas cahaya rendah
    digitalWrite(RELAY_LAMP_PIN, HIGH);
  } else {
    // Matikan relay lampu jika intensitas cahaya mencukupi
    digitalWrite(RELAY_LAMP_PIN, LOW);
  }
}

void sendDataToHTTP(int soilMoisture, int ldrValue) {
  HTTPClient http;
  String url = "https://iottugas.000webhostapp.com/send_data.php"; // Ganti dengan URL atau endpoint yang sesuai
  url += "?soilPercent=" + String(soilMoisture);
  url += "&ldrPercent=" + String(ldrValue);

  Serial.print("Sending GET request to: ");
  Serial.println(url);

  http.begin(url);
  int httpCode = http.GET();

  if (httpCode > 0) {
    Serial.printf("HTTP code: %d\n", httpCode);
    if (httpCode == HTTP_CODE_OK) {
      String payload = http.getString();
      Serial.println("Server response: " + payload);
    }
  } else {
    Serial.println("Failed to send HTTP request.");
  }

  http.end();
}

void setup() {
  Serial.begin(9600);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("Connected to Wi-Fi");

  setupMQTT();
  setupRelays();
}

void loop() {
  if (!mqttClient.connected())
    reconnect();

  mqttClient.loop();

  int ldrValue = analogRead(LDR_PIN);
  int ldrPercent = map(ldrValue, 0, 4095, 0, 100); // Ubah nilai sensor menjadi persentase
  ldrPercent = constrain(ldrPercent, 0, 100); // Pastikan persentase berada dalam kisaran 0-100

  char ldrString[5]; // Ukuran string disesuaikan dengan panjang persentase (maksimal 3 digit + '%' + null terminator)
  snprintf(ldrString, sizeof(ldrString), "%d%%", ldrPercent);
  Serial.print("LDR Intensity: ");
  Serial.println(ldrString);
  mqttClient.publish("greenhouse/ldr", ldrString);

  int soilMoistureValue = analogRead(SOIL_PIN);
  int soilPercent = map(soilMoistureValue, 0, 4095, 0, 100); // Ubah nilai sensor menjadi persentase
  soilPercent = constrain(soilPercent, 0, 100); // Pastikan persentase berada dalam kisaran 0-100

  char soilString[5]; // Ukuran string disesuaikan dengan panjang persentase (maksimal 3 digit + '%' + null terminator)
  snprintf(soilString, sizeof(soilString), "%d%%", soilPercent);
  Serial.print("Soil Moisture: ");
  Serial.println(soilString);
  mqttClient.publish("greenhouse/soil", soilString);

  controlRelays(soilPercent, ldrPercent); // Menggunakan persentase untuk kontrol relay

  sendDataToHTTP(soilPercent, ldrPercent);

  delay(2000);
}
