#include <WiFi.h>
#include <esp_wifi.h>
#include <esp_mesh.h>
#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT11

const char* ssid = "ISEP-4G";
const char* password = "keluargacemara";
mesh_addr_t rootAddress;

const int soilMoisturePin = A0;
const int mq2Pin = 33;

DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  delay(10);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Serial.println("Connected to Wi-Fi Mesh");

  dht.begin();
}

void loop() {
  int soilMoistureValue = analogRead(soilMoisturePin);
  int mq2Value = analogRead(mq2Pin);
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  Serial.print("Soil Moisture: ");
  Serial.println(soilMoistureValue);
  Serial.print("MQ-2 Value: ");
  Serial.println(mq2Value);
  Serial.print("Humidity: ");
  Serial.print(humidity);
  Serial.print(" %\t");
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.println(" °C");

  delay(5000);
}
