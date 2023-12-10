#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>

#define DHTPIN 2        // Pin data DHT22 terhubung ke pin GPIO 4 pada ESP32
#define DHTTYPE DHT22   // Tipe sensor DHT yang digunakan

DHT dht(DHTPIN, DHTTYPE);

const int LDR_PIN = 15;          // Pin analog untuk sensor LDR terhubung ke pin GPIO 34 pada ESP32
const int RELAY_12V_PIN = 4;    // Pin relay untuk mengontrol kompa tafflware 12V
const int RELAY_5V_PIN = 6;     // Pin relay untuk mengontrol lampu 5V
const int SOIL_MOISTURE_PIN = 5; // Pin analog untuk sensor kelembaban tanah terhubung ke pin GPIO 14 pada ESP32

void setup() {
  Serial.begin(9600);
  pinMode(RELAY_12V_PIN, OUTPUT);
  pinMode(RELAY_5V_PIN, OUTPUT);
  
  dht.begin();
  pinMode(SOIL_MOISTURE_PIN, INPUT);  // Set pin untuk sensor kelembaban tanah sebagai input
}

void loop() {
  // Read humidity and temperature from the DHT22 sensor
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Read the LDR sensor value
  int ldrValue = analogRead(LDR_PIN);
  int brightness = map(ldrValue, 0, 4095, 0, 255);

  // Read soil moisture sensor value
  int soilMoistureValue = analogRead(SOIL_MOISTURE_PIN);

  // Check if DHT22 readings are valid
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Gagal membaca dari sensor DHT");
  } else {
    Serial.print("Kelembaban: ");
    Serial.print(humidity);
    Serial.print(" %\t Suhu: ");
    Serial.print(temperature);
    Serial.println(" °C");
  }

  // Print LDR sensor value
  Serial.print("Nilai LDR: ");
  Serial.println(ldrValue);

  // Print soil moisture sensor value
  Serial.print("Nilai Kelembaban Tanah: ");
  Serial.println(soilMoistureValue);

  // Control 12V device based on soil moisture value
  if (soilMoistureValue < 500) {
    digitalWrite(RELAY_12V_PIN, HIGH); // Turn on 12V device
  } else {
    digitalWrite(RELAY_12V_PIN, LOW); // Turn off 12V device
  }

  // Control 5V LED based on LDR sensor
  if (brightness < 100) {
    digitalWrite(RELAY_5V_PIN, HIGH); // Turn on 5V LED
  } else {
    digitalWrite(RELAY_5V_PIN, LOW); // Turn off 5V LED
  }

  delay(1000);
}
